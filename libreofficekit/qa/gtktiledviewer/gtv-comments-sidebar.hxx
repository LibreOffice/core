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

#include <boost/property_tree/json_parser.hpp>

G_BEGIN_DECLS

struct _GtvCommentsSidebar
{
    GtkBox parent;

    GtkWidget* viewannotationsButton;
    GtkWidget* scrolledwindow;
    GtkWidget* commentsgrid;
};

#define GTV_COMMENTS_SIDEBAR_TYPE (gtv_comments_sidebar_get_type())
G_DECLARE_FINAL_TYPE(GtvCommentsSidebar, gtv_comments_sidebar, GTV, COMMENTS_SIDEBAR, GtkBox);

GtkWidget* gtv_comments_sidebar_new();

void gtv_comments_sidebar_view_annotations(GtvCommentsSidebar* sidebar);

G_END_DECLS

#endif /* GTV_COMMENTS_SIDEBAR_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
