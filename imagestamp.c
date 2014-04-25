#include<stdlib.h>
#include<string.h>
#include<math.h>
#include<cairo.h>
#include<gtk/gtk.h>
#include<gdk/gdkkeysyms.h>

static gchar *pattern_image;
static cairo_surface_t *background;
static cairo_surface_t *pattern;
static int width, height;
static int p_width, p_height;
static int p_x, p_y;
static gboolean clicking = FALSE;
static gchar **images;
static int image_index = 0;
GtkWidget *window;

static GOptionEntry entries[] = {
  {"pattern", 'p', 0, G_OPTION_ARG_FILENAME,
    &pattern_image, "pattern PNG file", "IMAGE"},
  {G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_STRING_ARRAY,
    &images, NULL, "IMAGES..."},
  {NULL}
};

static void load_backgound(){
  background  = cairo_image_surface_create_from_png(images[image_index]);
  width  = cairo_image_surface_get_width(background);
  height = cairo_image_surface_get_height(background);
  gtk_widget_set_size_request(window, width, height);
}

static gboolean key_pressed(GtkWidget *widget,
    GdkEventKey *event, gpointer data){
  if(event->keyval == GDK_KEY_q){
    g_debug("quit");
    gtk_main_quit();
  }
  return FALSE;
}

static gboolean do_clicking(GtkWidget *widget,
    GdkEventButton *event, gpointer data){
  if(event->button == 1){
    clicking = TRUE;
  }
  return FALSE;
}

static gboolean do_stamp(GtkWidget *widget,
    GdkEventButton *event, gpointer data){
  if(event->button != 1){
    return FALSE;
  }

  clicking = FALSE;

  gchar *p = images[image_index];
  gchar *noext_name;
  gchar *dot, *slash;
  dot = g_utf8_strrchr(p, -1, '.');
  slash = g_utf8_strrchr(p, -1, '/');
  if(dot && dot > slash + 1){
    noext_name = malloc(sizeof(char) * (dot-p+1));
    memcpy(noext_name, p, dot - p);
    noext_name[dot-p] = '\0';
  }else{
    noext_name = strdup(p);
  }
  gchar *filename = g_strdup_printf("%s_p.png", noext_name);

  cairo_t *cr;
  cr = gdk_cairo_create(widget->window);
  cairo_surface_write_to_png(cairo_get_target(cr), filename);
  cairo_destroy(cr);
  g_free(filename);

  image_index++;
  if(!images[image_index]){
    gtk_main_quit();
  }else{
    load_backgound();
  }
  return TRUE;
}

static gboolean do_move(GtkWidget *widget,
    GdkEventButton *event, gpointer data){
  if(clicking){
    return FALSE;
  }
  p_x = MAX(MIN(event->x - p_width / 2, width - p_width), 0);
  p_y = MAX(MIN(event->y - p_height / 2, height - p_height), 0);
  gtk_widget_queue_draw(widget);
  return FALSE;
}

static gboolean on_expose_event(GtkWidget *widget,
    GdkEventExpose *event, gpointer data){

  cairo_t *cr;
  cr = gdk_cairo_create(widget->window);

  cairo_set_source_surface(cr, background, 0, 0);
  cairo_paint(cr);
  cairo_set_source_surface(cr, pattern, p_x, p_y);
  cairo_paint(cr);

  cairo_destroy(cr);
  return FALSE;
}

int main(int argc, char *argv[]){
  if(!gtk_init_with_args(&argc, &argv, "", entries, NULL, NULL)){
    fprintf(stderr, "error when initilizing.\n");
    exit(2);
  }

  if(!images){
    fprintf(stderr, "no images given.\n");
    exit(1);
  }
  if(!pattern_image){
    fprintf(stderr, "no pattern image given.\n");
    exit(1);
  }

  window = gtk_dialog_new();
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_MOUSE);
  gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
  gtk_widget_set_app_paintable(window, TRUE);
  gtk_widget_set_double_buffered(window, TRUE);
  load_backgound();

  pattern  = cairo_image_surface_create_from_png(pattern_image);
  p_width  = cairo_image_surface_get_width(pattern);
  p_height = cairo_image_surface_get_height(pattern);
  p_x = (width - p_width) / 2;
  p_y = (height - p_height) / 2;

  g_signal_connect(window, "expose-event", G_CALLBACK(on_expose_event), NULL);
  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
  g_signal_connect(window, "key-press-event", G_CALLBACK(key_pressed), NULL);

  gtk_widget_add_events(window, GDK_KEY_PRESS_MASK |
      GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK |
      GDK_BUTTON_RELEASE_MASK);
  g_signal_connect(window, "button-press-event",
      G_CALLBACK(do_clicking), NULL);
  g_signal_connect(window, "button-release-event",
      G_CALLBACK(do_stamp), NULL);
  g_signal_connect(GTK_OBJECT(window), "motion_notify_event",
      G_CALLBACK(do_move), NULL);

  gtk_widget_show_all(window);
  gtk_main();
  cairo_surface_destroy(background);
  cairo_surface_destroy(pattern);

  return 0;
}
