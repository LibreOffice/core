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
