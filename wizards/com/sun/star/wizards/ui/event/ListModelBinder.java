/*************************************************************************
 *
 *  $RCSfile: ListModelBinder.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $  $Date: 2004-11-27 09:08:54 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 */
package com.sun.star.wizards.ui.event;

import javax.swing.ListModel;
import javax.swing.event.ListDataEvent;
import javax.swing.event.ListDataListener;

import com.sun.star.awt.XComboBox;
import com.sun.star.awt.XListBox;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.common.Helper;

/**
 * @author rpiterman

 * */
public class ListModelBinder implements ListDataListener {
    private XListBox unoList;
    private Object unoListModel;
    private ListModel listModel;
    private Renderer renderer = new Renderer() {
        public String render(Object item) {
            if (item == null)
                return "";
            else
                return item.toString();
        }
    };

    public ListModelBinder(Object unoListBox, ListModel listModel_) {
        unoList = (XListBox) UnoRuntime.queryInterface(XListBox.class, unoListBox);
        unoListModel = UnoDataAware.getModel(unoListBox);
        setListModel(listModel_);
    }

    public void setListModel(ListModel newListModel) {
        if (listModel != null)
            listModel.removeListDataListener(this);
        listModel = newListModel;
        listModel.addListDataListener(this);
    }

    /* (non-Javadoc)
     * @see javax.swing.event.ListDataListener#contentsChanged(javax.swing.event.ListDataEvent)
     */
    public void contentsChanged(ListDataEvent lde) {
        short[] selected = getSelectedItems();
        for (short i = (short) lde.getIndex0(); i <= lde.getIndex1(); i++)
            update(i);
        setSelectedItems(selected);
    }

    protected void update(short i) {
        remove(i, i);
        insert(i);
    }

    protected void remove(short i1, short i2) {
        unoList.removeItems((short) i1, (short) (i2 - i1 + 1));
    }

    protected void insert(short i) {
        unoList.addItem(getItemString(i), i);
    }

    protected String getItemString(short i) {
        return getItemString(listModel.getElementAt((int) i));
    }

    protected String getItemString(Object item) {
        return renderer.render(item);
    }

    protected short[] getSelectedItems() {
        return (short[]) Helper.getUnoPropertyValue(unoListModel, "SelectedItems");
    }

    protected void setSelectedItems(short[] selected) {
        Helper.setUnoPropertyValue(unoListModel, "SelectedItems", selected);
    }

    /* (non-Javadoc)
     * @see javax.swing.event.ListDataListener#intervalAdded(javax.swing.event.ListDataEvent)
     */
    public void intervalAdded(ListDataEvent lde) {
        //Short[] selected = getSelectedItems();
        for (short i = (short) lde.getIndex0(); i <= lde.getIndex1(); i++)
            insert(i);

        /*int insertedItems = lde.getIndex1() - lde.getIndex0() + 1;

        for (int i = 0; i<selected.length; i++)
          if (selected[i].intValue() >= lde.getIndex0())
              selected[i] = new Short((short)(selected[i].shortValue() + insertedItems));
        setSelectedItems(selected);*/
    }

    /* (non-Javadoc)
     * @see javax.swing.event.ListDataListener#intervalRemoved(javax.swing.event.ListDataEvent)
     */
    public void intervalRemoved(ListDataEvent lde) {
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

    public static interface Renderer {
        public String render(Object item);
    }

    public static void fillList(Object list, Object[] items, Renderer renderer) {
        XListBox xlist = (XListBox) UnoRuntime.queryInterface(XListBox.class, list);
        Helper.setUnoPropertyValue(UnoDataAware.getModel(list), "StringItemList", new String[] {});
        for (short i = 0; i < items.length; i++)
            if (items[i] != null)
                xlist.addItem((renderer != null ? renderer.render(items[i]) : items[i].toString()), i);
    }

    public static void fillComboBox(Object list, Object[] items, Renderer renderer) {
        XComboBox xComboBox = (XComboBox) UnoRuntime.queryInterface(XComboBox.class,list);
        Helper.setUnoPropertyValue(UnoDataAware.getModel(list), "StringItemList", new String[] {});
        for (short i = 0; i < items.length; i++)
            if (items[i] != null)
                xComboBox.addItem((renderer != null ? renderer.render(items[i]) : items[i].toString()), i);
    }

}
