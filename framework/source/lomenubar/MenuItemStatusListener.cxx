/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#include "MenuItemStatusListener.hxx"
#include "MenuItemInfo.hxx"

#include <com/sun/star/frame/status/Visibility.hpp>

using com::sun::star::frame::status::Visibility;

MenuItemStatusListener::MenuItemStatusListener (FrameHelper *helper)
    : m_helper(helper)
{
    OSL_ENSURE(m_helper, "FrameHelper cannot be NULL");
}

void SAL_CALL
MenuItemStatusListener::statusChanged(const FeatureStateEvent& Event)
  throw (RuntimeException)
{
    sal_Bool            isChecked;
    Visibility          visible;
    OUString url = Event.FeatureURL.Complete;
    OUString oULabel;

    gchar* c_url = g_utf16_to_utf8 (url.getStr(),
                                    url.getLength(),
                                    NULL, NULL, NULL);

    GHashTable *commandsInfo = m_helper->getCommandsInfo ();
    MenuItemInfo *info = (MenuItemInfo*)g_hash_table_lookup (commandsInfo, (gpointer)c_url);
    if (!info)
    {
        info = new MenuItemInfo ();
        g_hash_table_insert (commandsInfo, c_url, info);

        //Set the default label
        oULabel = m_helper->getLabelFromCommandURL(url);
        // Replace tilde with underscore for Dbusmenu Alt accelerators
        oULabel = oULabel.replace ((sal_Unicode)0x007e, (sal_Unicode)0x005f);

        // GLib behaves better than OUStringToOString wrt encoding transformation
        gchar* label = g_utf16_to_utf8 (oULabel.getStr(),
                                        oULabel.getLength(),
                                        NULL, NULL, NULL);
        info->setLabel (label);
        g_free (label);
    }
    else
    {
        //Since we're not introducing it in the hash table, we get rid of this string
        g_free (c_url);
    }

    //We set the enabled/disabled state
    info->setEnabled ((gboolean)Event.IsEnabled);

    //We find out what the new state is by casting

    //For some reason, URLs can slip through as labels, we make sure
    //this doesn't happen.
    if ((Event.State >>= oULabel) &&
        !oULabel.match ("private:", 0) &&
        !oULabel.match (".uno:", 0)    &&
        !oULabel.match ("slot:", 0)    &&
        !oULabel.match ("service:", 0) &&
        !oULabel.match (".cmd:", 0)    &&
        !oULabel.match ("macro:///", 0))
    {
        oULabel = oULabel.replace ((sal_Unicode)0x007e, (sal_Unicode)0x005f);
        gchar* label = g_utf16_to_utf8 (oULabel.getStr(),
                                        oULabel.getLength(),
                                        NULL, NULL, NULL);
        info->setLabel (label);
        g_free (label);
    }
    else if (Event.State >>= isChecked)
    {
        info->setCheckState (isChecked);
    }
    else if (Event.State >>= visible)
    {
        info->setVisible (visible.bVisible);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
