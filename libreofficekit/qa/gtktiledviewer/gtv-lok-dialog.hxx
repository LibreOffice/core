/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef GTV_LOK_DIALOG_H
#define GTV_LOK_DIALOG_H

#include <gtk/gtk.h>

#include <LibreOfficeKit/LibreOfficeKitGtk.h>

G_BEGIN_DECLS

#define GTV_TYPE_LOK_DIALOG            (gtv_lok_dialog_get_type())
#define GTV_LOK_DIALOG(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), GTV_TYPE_LOK_DIALOG, GtvLokDialog))
#define GTV_IS_LOK_DIALOG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), GTV_TYPE_LOK_DIALOG))
#define GTV_LOK_DIALOG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),  GTV_TYPE_LOK_DIALOG, GtvLokDialogClass))
#define GTV_IS_LOK_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),  GTV_TYPE_LOK_DIALOG))
#define GTV_LOK_DIALOG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj),  GTV_TYPE_LOK_DIALOG, GtvLokDialogClass))

struct GtvLokDialog
{
    GtkDialog parent;
};

struct GtvLokDialogClass
{
    GtkDialogClass parentClass;
};

GType gtv_lok_dialog_get_type               (void) G_GNUC_CONST;

GtkWidget* gtv_lok_dialog_new(LOKDocView* pDocView, const gchar* dialogId);

void gtv_lok_dialog_invalidate(GtvLokDialog* dialog);

void gtv_lok_dialog_child_invalidate(GtvLokDialog* dialog);

void gtv_lok_dialog_child_close(GtvLokDialog* dialog);

G_END_DECLS

#endif /* GTV_LOK_DIALOG_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
