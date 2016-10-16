#include "linkedlist.h"
#include "packet_plugin.h"
#include "logging.h"

#include <string.h>
#include <malloc.h>

#define TRUE 1
#define FALSE 0

/* content of this list is PACKET_PLUGIN_INFO* */
static LIST_ELEMENT* g_packet_plugin_list;
/* We need to specify the order of plugins */
static LIST_ELEMENT* g_active_packet_plugin_list;

PACKET_PLUGIN* packet_plugin_rjv3_new();

int init_packet_plugin_list() {
    PACKET_PLUGIN* (*list[])() = {
//#include "packet_pluginl_list_gen.h" TODO autogen
        packet_plugin_rjv3_new
    };
    int i = 0;
    for (; i < sizeof(list) / sizeof(PACKET_PLUGIN*); ++i) {
        PACKET_PLUGIN* (*func)() = list[i];
        insert_data(&g_packet_plugin_list, func());
    }
    return i;
}

static int plugin_name_cmp(void* to_find, void* curr) {
    return memcmp(to_find, ((PACKET_PLUGIN*)curr)->name, strlen(curr));
}

RESULT select_packet_plugin(const char* name) {
    PACKET_PLUGIN* _info;
    _info = (PACKET_PLUGIN*)lookup_data(g_packet_plugin_list, (void*)name, plugin_name_cmp);
    if (_info != NULL) {
        insert_data(&g_active_packet_plugin_list, _info);
        return SUCCESS;
    }
    return FAILURE;
}

/* 
 * I know this is silly, but is there better way to do it
 * since list_traverse takes none extra parameters?
 * Even if it takes user-defined extra params, I'd make
 * lots of useless structs to pass variable number of actual params
 * via the extra param in list_traverse - it would take twice as many
 * functions as current implementation. This is more unacceptable.
 */
#define PLUGIN ((PACKET_PLUGIN*)plugin_info)
void packet_plugin_destroy() {
    LIST_ELEMENT *plugin_info = g_active_packet_plugin_list;
    do {
        PLUGIN->destroy(PLUGIN);
    } while ((plugin_info = plugin_info->next));
}

void packet_plugin_process_cmdline_opts(int argc, char* argv[]) {
    LIST_ELEMENT *plugin_info = g_active_packet_plugin_list;
    do {
        PLUGIN->process_cmdline_opts(PLUGIN, argc, argv);
    } while ((plugin_info = plugin_info->next));
}

void packet_plugin_print_cmdline_help() {
    LIST_ELEMENT *plugin_info = g_active_packet_plugin_list;
    do {
        PLUGIN->print_cmdline_help(PLUGIN);
    } while ((plugin_info = plugin_info->next));
}

void packet_plugin_prepare_frame(ETH_EAP_FRAME* frame) {
    LIST_ELEMENT *plugin_info = g_active_packet_plugin_list;
    do {
        PLUGIN->prepare_frame(PLUGIN, frame);
    } while ((plugin_info = plugin_info->next));
}

void packet_plugin_on_frame_received(ETH_EAP_FRAME* frame) {
    LIST_ELEMENT *plugin_info = g_active_packet_plugin_list;
    do {
        PLUGIN->on_frame_received(PLUGIN, frame);
    } while ((plugin_info = plugin_info->next));
}

void packet_plugin_set_auth_round(int round) {
    LIST_ELEMENT *plugin_info = g_active_packet_plugin_list;
    do {
        PLUGIN->set_auth_round(PLUGIN, round);
    } while ((plugin_info = plugin_info->next));
}