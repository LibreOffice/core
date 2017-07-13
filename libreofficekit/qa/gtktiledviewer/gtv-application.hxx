/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef GTV_APPLICATION_H
#define GTV_APPLICATION_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GTV_TYPE_APPLICATION            (gtv_application_get_type())
#define GTV_APPLICATION(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), GTV_TYPE_APPLICATION, GtvApplication))
#define GTV_IS_APPLICATION(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), GTV_TYPE_APPLICATION))
#define GTV_APPLICATION_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass),  GTV_TYPE_APPLICATION, GtvApplicationClass))
#define GTV_IS_APPLICATION_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass),  GTV_TYPE_APPLICATION))
#define GTV_APPLICATION_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj),  GTV_TYPE_APPLICATION, GtvApplicationClass))

struct GtvApplication
{
    GtkApplication parent;
};

struct GtvApplicationClass
{
    GtkApplication parentClass;
};

GType gtv_application_get_type               (void) G_GNUC_CONST;

GtvApplication* gtv_application_new();

G_END_DECLS

#endif /* GTV_APPLICATION_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
