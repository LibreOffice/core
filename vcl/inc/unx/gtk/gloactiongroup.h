#ifndef GLOACTIONGROUP_H
#define GLOACTIONGROUP_H

#include <gio/gio.h>

G_BEGIN_DECLS

#define G_TYPE_LO_ACTION_GROUP                          (g_lo_action_group_get_type ())
#define G_LO_ACTION_GROUP(inst)                         (G_TYPE_CHECK_INSTANCE_CAST ((inst),                     \
                                                         G_TYPE_LO_ACTION_GROUP, GLOActionGroup))
#define G_LO_ACTION_GROUP_CLASS(klass)                  (G_TYPE_CHECK_CLASS_CAST ((klass),                       \
                                                         G_TYPE_LO_ACTION_GROUP, GLOActionGroupClass))
#define G_IS_LO_ACTION_GROUP(inst)                      (G_TYPE_CHECK_INSTANCE_TYPE ((inst),                     \
                                                         G_TYPE_LO_ACTION_GROUP))
#define G_IS_LO_ACTION_GROUP_CLASS(klass)               (G_TYPE_CHECK_CLASS_TYPE ((klass),                       \
                                                         G_TYPE_LO_ACTION_GROUP))
#define G_LO_ACTION_GROUP_GET_CLASS(inst)               (G_TYPE_INSTANCE_GET_CLASS ((inst),                      \
                                                         G_TYPE_LO_ACTION_GROUP, GLOActionGroupClass))

typedef struct _GLOActionGroupPrivate                   GLOActionGroupPrivate;
typedef struct _GLOActionGroupClass                     GLOActionGroupClass;
typedef struct _GLOActionGroup                          GLOActionGroup;

struct _GLOActionGroup
{
    /*< private >*/
    GObject parent_instance;

    GLOActionGroupPrivate *priv;
};

struct _GLOActionGroupClass
{
    /*< private >*/
    GObjectClass parent_class;

    /*< private >*/
    gpointer padding[12];
};

GType               g_lo_action_group_get_type              (void) G_GNUC_CONST;

GLOActionGroup *    g_lo_action_group_new                   (void);

void                g_lo_action_group_insert                (GLOActionGroup *group,
                                                             const gchar    *action_name,
                                                             gpointer        action_info);

void                g_lo_action_group_set_action_enabled    (GLOActionGroup *group,
                                                             const gchar    *action_name,
                                                             gboolean        enabled);

void                g_lo_action_group_remove                (GLOActionGroup *group,
                                                             const gchar    *action_name);

void                g_lo_action_group_clear                 (GLOActionGroup *group);

void                g_lo_action_group_merge                 (GLOActionGroup *input_group,
                                                             GLOActionGroup *output_group);

G_END_DECLS

#endif // GLOACTIONGROUP_H
