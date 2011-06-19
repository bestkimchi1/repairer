#include <gtk/gtk.h>

#include "nautilus-filename-repairer-i18n.h"
#include "encoding-dialog.h"

#define ENCODINGS_DIALOG_UI PKGDATADIR "/encoding-dialog.ui"

enum {
    ENCODING_COLUMN_ENCODING,
    ENCODING_NUM_COLUMNS
};

static const char* encoding_list[] = {
    "ISO-8859-1",          /* Western */
    "ISO-8859-2",          /* Central European */
    "ISO-8859-3",          /* South European */
    "ISO-8859-4",          /* Baltic */
    "ISO-8859-5",          /* Cyrillic */
    "ISO-8859-6",          /* Arabic */
    "ISO-8859-7",          /* Greek */
    "ISO-8859-8",          /* Hebrew */
    "ISO-8859-9",          /* Turkish */
    "ISO-8859-10",         /* Nordic */
    "ISO-8859-11",         /* Thai */
    "ISO-8859-13",         /* Baltic */
    "ISO-8859-14",         /* Celtic */
    "ISO-8859-15",         /* Western */
    "ISO-8859-16",         /* Eastern European */

    "EUC-CN",              /* Chinese Simplified */
    "EUC-JP",              /* Japanese */
    "EUC-KR",              /* Korean */
    "EUC-TW",              /* Chinese Traditional */
};

static GtkListStore*
get_encoding_list_model()
{
    GtkListStore* store;
    GtkTreeIter iter;
    int i;
    int n;

    store = gtk_list_store_new(ENCODING_NUM_COLUMNS, G_TYPE_STRING); 

    i = 0;
    n = G_N_ELEMENTS(encoding_list);
    for (i = 0; i < n; i++) {
	gtk_list_store_append(store, &iter);
	gtk_list_store_set(store, &iter,
		ENCODING_COLUMN_ENCODING, encoding_list[i],
		-1);
    }

    return store;
}

static char*
encoding_dialog_get_encoding(GtkDialog* dialog)
{
    GtkComboBox* combobox;

    combobox = g_object_get_data(G_OBJECT(dialog), "encoding_entry");

    return gtk_combo_box_get_active_text(combobox);
}

GtkDialog*
encoding_dialog_new(GtkWindow* parent)
{
    GtkBuilder* builder;
    GObject* object;
    GtkDialog* dialog;
    GtkComboBoxEntry* entry;
    GtkTreeModel* model;

    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, ENCODINGS_DIALOG_UI, NULL);

    object = gtk_builder_get_object(builder, "encoding_dialog");
    if (object == NULL)
	return NULL;

    dialog = GTK_DIALOG(object);
    gtk_window_set_transient_for(GTK_WINDOW(dialog), parent);

    object = gtk_builder_get_object(builder, "encoding_name_entry");
    if (object == NULL) {
	return dialog;
    }

    model = GTK_TREE_MODEL(get_encoding_list_model());
    entry = GTK_COMBO_BOX_ENTRY(object);
    gtk_combo_box_set_model(GTK_COMBO_BOX(entry), model);
    gtk_combo_box_entry_set_text_column(entry, ENCODING_COLUMN_ENCODING);
    g_object_set_data(G_OBJECT(dialog), "encoding_entry", entry);

    g_object_unref(builder);

    return dialog;
}

static gboolean
is_valid_encoding(const char* encoding)
{
    GIConv cd;

    if (encoding == NULL || encoding[0] == '\0')
	return FALSE;

    cd = g_iconv_open("UTF-8", encoding);
    if (cd == (GIConv)-1) {
        return FALSE;
    }
    g_iconv_close(cd);

    return TRUE;
}

void
encoding_dialog_show_error_message(GtkDialog* dialog, const char* encoding)
{
    GtkWidget* message;

    if (encoding == NULL || encoding[0] == '\0')
	return;

    message = gtk_message_dialog_new_with_markup(GTK_WINDOW(dialog),
	    GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
	    GTK_MESSAGE_ERROR,
	    GTK_BUTTONS_CLOSE,
	    _("<span size=\"larger\">The selected \"%s\" is invalid encoding.</span>\n"
	      "<span size=\"larger\">Please choose another one.</span>"),
	    encoding);
    gtk_dialog_run(GTK_DIALOG(message));
    gtk_widget_destroy(message);
}

char*
encoding_dialog_run(GtkDialog* dialog)
{
    char* encoding;
    gint response;

run:
    response = gtk_dialog_run(dialog);

    if (response == GTK_RESPONSE_OK) {
	encoding = encoding_dialog_get_encoding(dialog);
	if (is_valid_encoding(encoding)) {
	    return encoding;
	} else {
	    encoding_dialog_show_error_message(dialog, encoding);
	    g_free(encoding);
	    goto run;
	}
    }

    return NULL;
}
