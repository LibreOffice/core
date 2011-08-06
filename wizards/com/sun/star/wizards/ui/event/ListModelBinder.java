/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
package com.sun.star.wizards.ui.event;

import javax.swing.ListModel;
import javax.swing.event.ListDataEvent;
import javax.swing.event.ListDataListener;

import com.sun.star.awt.XComboBox;
import com.sun.star.awt.XListBox;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.common.Helper;
import com.sun.star.wizards.common.PropertyNames;

/**
 * @author rpiterman

 * */
public class ListModelBinder implements ListDataListener
{

    private XListBox unoList;
    private Object unoListModel;
    private ListModel listModel;
    private Renderer renderer = new Renderer()
    {

        public String render(Object item)
        {
            if (item == null)
            {
                return PropertyNames.EMPTY_STRING;
            }
            else
            {
                return item.toString();
            }
        }
    };

    public ListModelBinder(Object unoListBox, ListModel listModel_)
    {
        unoList = UnoRuntime.queryInterface(XListBox.class, unoListBox);
        unoListModel = UnoDataAware.getModel(unoListBox);
        setListModel(listModel_);
    }

    public void setListModel(ListModel newListModel)
    {
        if (listModel != null)
        {
            listModel.removeListDataListener(this);
        }
        listModel = newListModel;
        listModel.addListDataListener(this);
    }

    /* (non-Javadoc)
     * @see javax.swing.event.ListDataListener#contentsChanged(javax.swing.event.ListDataEvent)
     */
    public void contentsChanged(ListDataEvent lde)
    {
        short[] selected = getSelectedItems();
        for (short i = (short) lde.getIndex0(); i <= lde.getIndex1(); i++)
        {
            update(i);
        }
        setSelectedItems(selected);
    }

    protected void update(short i)
    {
        remove(i, i);
        insert(i);
    }

    protected void remove(short i1, short i2)
    {
        unoList.removeItems(i1, (short) (i2 - i1 + 1));
    }

    protected void insert(short i)
    {
        unoList.addItem(getItemString(i), i);
    }

    protected String getItemString(short i)
    {
        return getItemString(listModel.getElementAt((int) i));
    }

    protected String getItemString(Object item)
    {
        return renderer.render(item);
    }

    protected short[] getSelectedItems()
    {
        return (short[]) Helper.getUnoPropertyValue(unoListModel, PropertyNames.SELECTED_ITEMS);
    }

    protected void setSelectedItems(short[] selected)
    {
        Helper.setUnoPropertyValue(unoListModel, PropertyNames.SELECTED_ITEMS, selected);
    }

    /* (non-Javadoc)
     * @see javax.swing.event.ListDataListener#intervalAdded(javax.swing.event.ListDataEvent)
     */
    public void intervalAdded(ListDataEvent lde)
    {
        //Short[] selected = getSelectedItems();
        for (short i = (short) lde.getIndex0(); i <= lde.getIndex1(); i++)
        {
            insert(i);

        /*int insertedItems = lde.getIndex1() - lde.getIndex0() + 1;

        for (int i = 0; i<selected.length; i++)
        if (selected[i].intValue() >= lde.getIndex0())
        selected[i] = new Short((short)(selected[i].shortValue() + insertedItems));
        setSelectedItems(selected);*/
        }
    }

    /* (non-Javadoc)
     * @see javax.swing.event.ListDataListener#intervalRemoved(javax.swing.event.ListDataEvent)
     */
    public void intervalRemoved(ListDataEvent lde)
    {
        //Short[] selected = getSelectedItems();

        remove((short) lde.getIndex0(), (short) lde.getIndex1());

    /*int removed = 0;
    for (int i = 0; i<selected.length; i++) {
    short s = selected[i].shortValue();
    if (s>=lde.getIndex0() && s<==lde.getIndex1()) {
    selected[i] = null;
    removed++;
    }
    }

    Short[] newSelected = (removed > 0 ? new Short[selected.length - removed] : selected;
    if (removed>0)

    if (selected[i].intValue() >= lde.getIndex0())
     */
    }

    public static interface Renderer
    {

        public String render(Object item);
    }

    public static void fillList(Object list, Object[] items, Renderer renderer)
    {
        XListBox xlist = UnoRuntime.queryInterface(XListBox.class, list);
        Helper.setUnoPropertyValue(UnoDataAware.getModel(list), PropertyNames.STRING_ITEM_LIST, new String[]
                {
                });
        for (short i = 0; i < items.length; i++)
        {
            if (items[i] != null)
            {
                xlist.addItem((renderer != null ? renderer.render(items[i]) : items[i].toString()), i);
            }
        }
    }

    public static void fillComboBox(Object list, Object[] items, Renderer renderer)
    {
        XComboBox xComboBox = UnoRuntime.queryInterface(XComboBox.class, list);
        Helper.setUnoPropertyValue(UnoDataAware.getModel(list), PropertyNames.STRING_ITEM_LIST, new String[]
                {
                });
        for (short i = 0; i < items.length; i++)
        {
            if (items[i] != null)
            {
                xComboBox.addItem((renderer != null ? renderer.render(items[i]) : items[i].toString()), i);
            }
        }
    }
}
