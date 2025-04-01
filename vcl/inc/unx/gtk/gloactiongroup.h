/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <glib-object.h>
#include <glib.h>

G_BEGIN_DECLS

                          (g_lo_action_group_get_type ())
(inst)                         (G_TYPE_CHECK_INSTANCE_CAST ((inst),                     \
                                                         G_TYPE_LO_ACTION_GROUP, GLOActionGroup))
(inst)                      (G_TYPE_CHECK_INSTANCE_TYPE ((inst),                     \
                                                         G_TYPE_LO_ACTION_GROUP))

struct GLOActionGroupPrivate;

struct GLOActionGroup
{
    /*< private >*/
    GObject parent_instance;

    GLOActionGroupPrivate *priv;
};

struct GLOActionGroupClass
{
    /*< private >*/
    GObjectClass parent_class;

    /*< private >*/
    gpointer padding[12];
};

GType               g_lo_action_group_get_type              (void) G_GNUC_CONST;

GLOActionGroup *    g_lo_action_group_new                   (void);

void                g_lo_action_group_set_top_menu          (GLOActionGroup     *group,
                                                             gpointer           top_menu);

void                g_lo_action_group_insert                (GLOActionGroup     *group,
                                                             const gchar        *action_name,
                                                             gint                item_id,
                                                             gboolean            submenu);

void                g_lo_action_group_insert_stateful       (GLOActionGroup     *group,
                                                             const gchar        *action_name,
                                                             gint                item_id,
                                                             gboolean            submenu,
                                                             const GVariantType *parameter_type,
                                                             const GVariantType *state_type,
                                                             GVariant           *state_hint,
                                                             GVariant           *state);

void                g_lo_action_group_set_action_enabled    (GLOActionGroup     *group,
                                                             const gchar        *action_name,
                                                             gboolean            enabled);

void                g_lo_action_group_remove                (GLOActionGroup     *group,
                                                             const gchar        *action_name);

void                g_lo_action_group_clear                 (GLOActionGroup     *group);

G_END_DECLS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
