/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef GTV_COMMENTS_SIDEBAR_H
#define GTV_COMMENTS_SIDEBAR_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GTV_TYPE_COMMENTS_SIDEBAR            (gtv_comments_sidebar_get_type())
#define GTV_COMMENTS_SIDEBAR(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), GTV_TYPE_COMMENTS_SIDEBAR, GtvCommentsSidebar))
#define GTV_IS_COMMENTS_SIDEBAR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), GTV_TYPE_COMMENTS_SIDEBAR))
#define GTV_COMMENTS_SIDEBAR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),  GTV_TYPE_COMMENTS_SIDEBAR, GtvCommentsSidebarClass))
#define GTV_IS_COMMENTS_SIDEBAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),  GTV_TYPE_COMMENTS_SIDEBAR))
#define GTV_COMMENTS_SIDEBAR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj),  GTV_TYPE_COMMENTS_SIDEBAR, GtvCommentsSidebarClass))

struct GtvCommentsSidebar
{
    GtkBox parent;

    GtkWidget* viewannotationsButton;
    GtkWidget* scrolledwindow;
    GtkWidget* commentsgrid;
};

struct GtvCommentsSidebarClass
{
    GtkBoxClass parentClass;
};

GType gtv_comments_sidebar_get_type               (void) G_GNUC_CONST;

GtkWidget* gtv_comments_sidebar_new();

void gtv_comments_sidebar_view_annotations(GtvCommentsSidebar* sidebar);

G_END_DECLS

#endif /* GTV_COMMENTS_SIDEBAR_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
