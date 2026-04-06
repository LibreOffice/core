/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef GTV_KIT_DIALOG_H
#define GTV_KIT_DIALOG_H

#include <gtk/gtk.h>

#include <COKit/COKitGtk.h>

G_BEGIN_DECLS

#define GTV_TYPE_KIT_DIALOG            (gtv_kit_dialog_get_type())
#define GTV_KIT_DIALOG(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), GTV_TYPE_KIT_DIALOG, GtvKitDialog))
#define GTV_IS_KIT_DIALOG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), GTV_TYPE_KIT_DIALOG))
#define GTV_KIT_DIALOG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),  GTV_TYPE_KIT_DIALOG, GtvKitDialogClass))
#define GTV_IS_KIT_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),  GTV_TYPE_KIT_DIALOG))
#define GTV_KIT_DIALOG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj),  GTV_TYPE_KIT_DIALOG, GtvKitDialogClass))

struct GtvKitDialog
{
    GtkDialog parent;
};

struct GtvKitDialogClass
{
    GtkDialogClass parentClass;
};

GType gtv_kit_dialog_get_type               (void) G_GNUC_CONST;

GtkWidget* gtv_kit_dialog_new(KitDocumentView* pDocView, guint dialogId, guint width, guint height);

void gtv_kit_dialog_invalidate(GtvKitDialog* dialog, const GdkRectangle& aRectangle);

void gtv_kit_dialog_child_create(GtvKitDialog* dialog, guint childId, guint nX, guint nY, guint width, guint height);

void gtv_kit_dialog_child_invalidate(GtvKitDialog* dialog);

void gtv_kit_dialog_child_close(GtvKitDialog* dialog);

gboolean gtv_kit_dialog_is_parent_of(GtvKitDialog* dialog, guint childId);

G_END_DECLS

#endif /* GTV_KIT_DIALOG_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
