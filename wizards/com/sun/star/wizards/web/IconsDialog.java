/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
package com.sun.star.wizards.web;

import javax.swing.ListModel;

import com.sun.star.awt.Size;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.wizards.common.ConfigSet;
import com.sun.star.wizards.common.FileAccess;
import com.sun.star.wizards.common.PropertyNames;
import com.sun.star.wizards.ui.ImageList;
import com.sun.star.wizards.web.data.CGIconSet;

/**
 * The dialog class for choosing an icon set.
 * This class simulates a model, though it does not functions really as one,
 * since it does not cast events.
 * It also implements the ImageList.ImageRenderer interface, to handle
 * its own objects. 
 */
public class IconsDialog extends ImageListDialog implements ImageList.IImageRenderer, ListModel
{

    private ConfigSet set;
    String htmlexpDirectory;
    /**
     * this icons filename prefixes are used to display the icons.
     */
    private String[] icons = new String[]
    {
        "firs", "prev", "next", "last", "nav", "text", "up", "down"
    };
    private Integer[] objects;

    /**
     * @param xmsf
     * @param set_ the configuration set of the supported
     * icon sets.
     */
    public IconsDialog(XMultiServiceFactory xmsf,
            ConfigSet set_,
            WebWizardDialogResources resources)
            throws Exception
    {
        super(xmsf, WWHID.HID_IS, new String[]
                {
                    resources.resIconsDialog,
                    resources.resIconsDialogCaption,
                    resources.resOK,
                    resources.resCancel,
                    resources.resHelp,
                    resources.resDeselect,
                    resources.resOther,
                    resources.resCounter
                });

        htmlexpDirectory = FileAccess.getOfficePath(xmsf, "Gallery", "share", PropertyNames.EMPTY_STRING);
        set = set_;
        objects = new Integer[set.getSize() * icons.length];
        for (int i = 0; i < objects.length; i++)
        {
            objects[i] = new Integer(i);
        }
        il.setListModel(this);
        il.setRenderer(this);
        il.setRows(4);
        il.setCols(8);
        il.setImageSize(new Size(20, 20));
        il.setShowButtons(false);
        il.setRowSelect(true);
        il.scaleImages = Boolean.FALSE;

        showDeselectButton = true;
        showOtherButton = false;

        build();
    }

    public String getIconset()
    {
        if (getSelected() == null)
        {
            return null;
        }
        else
        {
            return (String) set.getKey(((Number) getSelected()).intValue() / icons.length);
        }
    }

    public void setIconset(String iconset)
    {
        int icon = set.getIndexOf(set.getElement(iconset)) * icons.length;
        this.setSelected(icon >= 0 ? objects[icon] : null);
    }

    public synchronized void addListDataListener(javax.swing.event.ListDataListener listener)
    {
    }

    public synchronized void removeListDataListener(javax.swing.event.ListDataListener listener)
    {
    }

    /* (non-Javadoc)
     * @see javax.swing.ListModel#getSize()
     */
    public int getSize()
    {
        return set.getSize() * icons.length;
    }
    /* (non-Javadoc)
     * @see javax.swing.ListModel#getElementAt(int)
     */

    public Object getElementAt(int arg0)
    {
        return objects[arg0];
    }

    /* (non-Javadoc)
     * @see com.sun.star.wizards.ui.ImageList.ImageRenderer#getImageUrls(java.lang.Object)
     */
    public Object[] getImageUrls(Object listItem)
    {
        int i = ((Number) listItem).intValue();
        int iset = getIconsetNum(i);
        int icon = getIconNum(i);
        String[] sRetUrls = new String[2];
        sRetUrls[0] = htmlexpDirectory + "/htmlexpo/" +
                getIconsetPref(iset) +
                icons[icon] +
                getIconsetPostfix(iset);
        sRetUrls[1] = sRetUrls[0];
        return sRetUrls;
    }
    /* (non-Javadoc)
     * @see com.sun.star.wizards.common.Renderer#render(java.lang.Object)
     */

    public String render(Object object)
    {
        if (object == null)
        {
            return PropertyNames.EMPTY_STRING;
        }
        int i = ((Number) object).intValue();
        int iset = getIconsetNum(i);
        return getIconset(iset).cp_Name;
    }

    private int getIconsetNum(int i)
    {
        return i / icons.length;
    }

    private int getIconNum(int i)
    {
        return i % icons.length;
    }

    private String getIconsetPref(int iconset)
    {
        return getIconset(iconset).cp_FNPrefix;
    }

    private String getIconsetPostfix(int iconset)
    {
        return getIconset(iconset).cp_FNPostfix;
    }

    private CGIconSet getIconset(int i)
    {
        return (CGIconSet) set.getElementAt(i);
    }
}
