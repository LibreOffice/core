/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef GTV_MAIN_TOOLBAR_H
#define GTV_MAIN_TOOLBAR_H

#include <gtk/gtk.h>

#include <string>

#include <LibreOfficeKit/LibreOfficeKitEnums.h>

#define GTV_TYPE_MAIN_TOOLBAR            (gtv_main_toolbar_get_type())
#define GTV_MAIN_TOOLBAR(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), GTV_TYPE_MAIN_TOOLBAR, GtvMainToolbar))
#define GTV_IS_MAIN_TOOLBAR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), GTV_TYPE_MAIN_TOOLBAR))
#define GTV_MAIN_TOOLBAR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),  GTV_TYPE_MAIN_TOOLBAR, GtvMainToolbarClass))
#define GTV_IS_MAIN_TOOLBAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),  GTV_TYPE_MAIN_TOOLBAR))
#define GTV_MAIN_TOOLBAR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj),  GTV_TYPE_MAIN_TOOLBAR, GtvMainToolbarClass))

struct GtvMainToolbar
{
    GtkBox parent;

    GtkWidget* m_pAddressbar;
    GtkWidget* m_pFormulabar;
};

struct GtvMainToolbarClass
{
    GtkBoxClass parentClass;
};

GType gtv_main_toolbar_get_type               (void) G_GNUC_CONST;

GtkWidget* gtv_main_toolbar_new();

GtkContainer* gtv_main_toolbar_get_first_toolbar(GtvMainToolbar* toolbar);

GtkContainer* gtv_main_toolbar_get_second_toolbar(GtvMainToolbar* toolbar);

void gtv_main_toolbar_set_sensitive_internal(GtvMainToolbar* toolbar, GtkToolItem* pItem, bool isSensitive);

/// Use internal sensitivity map to set actual widget's sensitiveness
void gtv_main_toolbar_set_edit(GtvMainToolbar* toolbar, gboolean bEdit);

void gtv_main_toolbar_doc_loaded(GtvMainToolbar* toolbar, LibreOfficeKitDocumentType eDocType, bool bEditMode);

void gtv_main_toolbar_add_recent_uno(GtvMainToolbar* toolbar, const std::string& rUnoCmdStr);

const std::string gtv_main_toolbar_get_recent_uno_args(GtvMainToolbar* toolbar, const std::string& rUnoCmd);

#endif /* GTV_MAIN_TOOLBAR_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
