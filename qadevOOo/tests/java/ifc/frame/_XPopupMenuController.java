/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: _XPopupMenuController.java,v $
 * $Revision: 1.5 $
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

package ifc.frame;

import com.sun.star.awt.XPopupMenu;
import com.sun.star.frame.XPopupMenuController;
import lib.MultiMethodTest;

public class _XPopupMenuController extends MultiMethodTest {
    public XPopupMenuController oObj = null;

    public void before() {

    }

    public void _setPopupMenu() {
        XPopupMenu xMenu = new PopupMenuImpl();
        oObj.setPopupMenu(xMenu);
        tRes.tested("setPopupMenu()", true);
    }

    public void _updatePopupMenu() {
        oObj.updatePopupMenu();
        tRes.tested("updatePopupMenu()", true);
    }

    private class PopupMenuImpl implements XPopupMenu {

        public void addMenuListener(com.sun.star.awt.XMenuListener xMenuListener) {
            System.out.println("addMenuListener called.");
        }

        public void checkItem(short param, boolean param1) {
            System.out.println("checkItemListener called.");
        }

        public void enableItem(short param, boolean param1) {
            System.out.println("enableItem called.");
        }

        public short execute(com.sun.star.awt.XWindowPeer xWindowPeer, com.sun.star.awt.Rectangle rectangle, short param) {
            System.out.println("execute called.");
            return 0;
        }

        public short getDefaultItem() {
            System.out.println("getDefaultItem called.");
            return 0;
        }

        public short getItemCount() {
            System.out.println("getItemCount called.");
            return 0;
        }

        public short getItemId(short param) {
            System.out.println("getItemId called.");
            return 0;
        }

        public short getItemPos(short param) {
            System.out.println("getItemPos called.");
            return 0;
        }

        public String getItemText(short param) {
            System.out.println("getItemText called.");
            return "Hi.";
        }

        public com.sun.star.awt.XPopupMenu getPopupMenu(short param) {
            System.out.println("getPopupMenu called.");
            return this;
        }

        public void insertItem(short param, String str, short param2, short param3) {
            System.out.println("insertItem called.");
        }

        public void insertSeparator(short param) {
            System.out.println("insertSeparator called.");
        }

        public boolean isItemChecked(short param) {
            System.out.println("isItemChecked called.");
            return false;
        }

        public boolean isItemEnabled(short param) {
            System.out.println("isItemEnabled called.");
            return false;
        }

        public void removeItem(short param, short param1) {
            System.out.println("removeItem called.");
        }

        public void removeMenuListener(com.sun.star.awt.XMenuListener xMenuListener) {
            System.out.println("removeMenuListener called.");
        }

        public void setDefaultItem(short param) {
            System.out.println("setDefaultItem called.");
        }

        public void setItemText(short param, String str) {
            System.out.println("setItemText called.");
        }

        public void setPopupMenu(short param, com.sun.star.awt.XPopupMenu xPopupMenu) {
            System.out.println("setPopupMenu called.");
        }
    }
}
