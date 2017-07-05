/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef GTV_APPLICATION_WINDOW_H
#define GTV_APPLICATION_WINDOW_H

#include <gtk/gtk.h>

#include <LibreOfficeKit/LibreOfficeKitGtk.h>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

#include <string>

struct GtvRenderingArgs
{
    std::string m_aLoPath;
    std::string m_aUserProfile;
    bool m_bEnableTiledAnnotations;

    std::string m_aBackgroundColor;
    bool m_bHidePageShadow;
    bool m_bHideWhiteSpace;
};

G_BEGIN_DECLS

#define GTV_APPLICATION_WINDOW_TYPE (gtv_application_window_get_type())
G_DECLARE_FINAL_TYPE(GtvApplicationWindow, gtv_application_window, GTV, APPLICATION_WINDOW, GtkApplicationWindow);


GtvApplicationWindow* gtv_application_window_new(GtkApplication* application);

void gtv_application_window_load_document(GtvApplicationWindow* application,
                                          const GtvRenderingArgs* aArgs,
                                          const std::string& aDocPath);

LOKDocView* gtv_application_window_get_lokdocview(GtvApplicationWindow* application);

void gtv_application_window_create_view_from_window(GtvApplicationWindow* window);

G_END_DECLS

#endif /* GTV_APPLICATION_WINDOW_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
