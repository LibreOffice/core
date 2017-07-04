/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <gtk/gtk.h>

#include <gtv-application.hxx>
#include <gtv-application-window.hxx>

#include <string>

struct _GtvApplication
{
    GtkApplication parent;
};

struct GtvApplicationPrivate
{
    GtvRenderingArgs* m_pRenderingArgs;
};

G_DEFINE_TYPE_WITH_PRIVATE(GtvApplication, gtv_application, GTK_TYPE_APPLICATION);

static GtvApplicationPrivate*
getPrivate(GtvApplication* app)
{
    return static_cast<GtvApplicationPrivate*>(gtv_application_get_instance_private(app));
}

static void
gtv_application_open(GApplication* app, GFile** file, gint nFiles, const gchar* hint)
{
    // TODO: add some option to create a new view for existing document
    // For now, this just opens a new document
    GtvApplicationWindow* window = GTV_APPLICATION_WINDOW(gtv_application_window_new(GTK_APPLICATION(app)));
    gtk_window_present(GTK_WINDOW(window));

    GtvApplicationPrivate* priv = getPrivate(GTV_APPLICATION(app));
    gtv_application_window_load_document(window, priv->m_pRenderingArgs, std::string(g_file_get_path(file[0])));
}

static void
gtv_application_init(GtvApplication* app)
{
    static const GOptionEntry commandLineOptions[] =
    {
        { "version", 0, G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, nullptr, "Show LOkit version", nullptr },
        { "lo-path", 0, G_OPTION_FLAG_NONE, G_OPTION_ARG_STRING, nullptr, "LO path", nullptr },
        { "user-profile", 0, G_OPTION_FLAG_NONE, G_OPTION_ARG_STRING, nullptr, "User profile to use", nullptr },
        { "enable-tiled-annotations", 0, G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, nullptr, "Whether tiled annotations should be enabled", nullptr },
        { "background-color", 0, G_OPTION_FLAG_NONE, G_OPTION_ARG_STRING, nullptr, "Background color", nullptr },
        { "hide-page-shadow", 0, G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, nullptr, "Hide page shadow", nullptr },
        { "hide-whitespace", 0, G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, nullptr, "Hide whitespace", nullptr },
        { nullptr }
    };

    g_application_add_main_option_entries(G_APPLICATION(app), commandLineOptions);

    GtvApplicationPrivate* priv = getPrivate(GTV_APPLICATION(app));
    priv->m_pRenderingArgs = new GtvRenderingArgs();
}

static void
gtv_application_dispose (GObject* object)
{
    GtvApplicationPrivate* priv = getPrivate(GTV_APPLICATION(object));

    delete priv->m_pRenderingArgs;
    priv->m_pRenderingArgs = nullptr;

    G_OBJECT_CLASS (gtv_application_parent_class)->dispose (object);
}

static gint
gtv_application_handle_local_options(GApplication* app, GVariantDict* options)
{
    gint ret = -1;

    if (g_variant_dict_contains(options, "version"))
    {
        // TODO: query lokit for version here
        g_print("LOKit version:");
        ret = 1;
    }

    GtvApplicationPrivate* priv = getPrivate(GTV_APPLICATION(app));
    // This is mandatory
    if (g_variant_dict_contains(options, "lo-path"))
    {
        gchar* loPath = nullptr;
        g_variant_dict_lookup(options, "lo-path", "s", &loPath);
        if (loPath)
        {
            priv->m_pRenderingArgs->m_aLoPath = std::string(loPath);
            g_free(loPath);
        }
    }
    else
        ret = 1; // Cannot afford to continue in absense of this param

    // Optional args
    if (g_variant_dict_contains(options, "user-profile"))
    {
        gchar* userProfile = nullptr;
        g_variant_dict_lookup(options, "user-profile", "s", &userProfile);
        if (userProfile)
        {
            priv->m_pRenderingArgs->m_aUserProfile = std::string("vnd.sun.star.pathname:") + std::string(userProfile);
            g_free(userProfile);
        }
    }

    if (g_variant_dict_contains(options, "background-color"))
    {
        gchar* backgroundColor = nullptr;
        g_variant_dict_lookup(options, "background-color", "s", &backgroundColor);
        if (backgroundColor)
        {
            priv->m_pRenderingArgs->m_aBackgroundColor = std::string(backgroundColor);
            g_free(backgroundColor);
        }
    }

    if (g_variant_dict_contains(options, "enable-tiled-annotations"))
        priv->m_pRenderingArgs->m_bEnableTiledAnnotations = true;
    if (g_variant_dict_contains(options, "hide-page-shadow"))
        priv->m_pRenderingArgs->m_bHidePageShadow = true;
    if (g_variant_dict_contains(options, "hide-whitespace"))
        priv->m_pRenderingArgs->m_bHideWhiteSpace = true;

    return ret;
}

static void
gtv_application_class_init(GtvApplicationClass* klass)
{
    G_APPLICATION_CLASS(klass)->open = gtv_application_open;
    G_APPLICATION_CLASS(klass)->handle_local_options = gtv_application_handle_local_options;
    G_OBJECT_CLASS(klass)->dispose = gtv_application_dispose;
}

GtvApplication* gtv_application_new()
{
    return GTV_APPLICATION(g_object_new(GTV_APPLICATION_TYPE,
                                        "application-id", "org.libreoffice.gtktiledviewer",
                                        "flags", G_APPLICATION_HANDLES_OPEN,
                                        nullptr));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
