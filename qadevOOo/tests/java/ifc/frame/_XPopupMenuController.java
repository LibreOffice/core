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

import com.sun.star.graphic.XGraphic;
import com.sun.star.awt.XPopupMenu;
import com.sun.star.awt.KeyEvent;
import com.sun.star.frame.XPopupMenuController;
import lib.MultiMethodTest;

public class _XPopupMenuController extends MultiMethodTest {
    public XPopupMenuController oObj = null;

    @Override
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

    private static class PopupMenuImpl implements XPopupMenu {

        public void addMenuListener(com.sun.star.awt.XMenuListener xMenuListener) {
            System.out.println("addMenuListener called.");
        }

        public void checkItem(short param, boolean param1) {
            System.out.println("checkItemListener called.");
        }

        public void enableItem(short param, boolean param1) {
            System.out.println("enableItem called.");
        }

        public short execute(com.sun.star.awt.XWindowPeer xWindowPeer, com.sun.star.awt.Rectangle pos, short param) {
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

        public XGraphic getItemImage(short param ) {
            System.out.println("getItemImage called.");
            return null;
        }

        public void setItemImage(short param, XGraphic param1, boolean param2 ) {
            System.out.println("setItemImage called.");
        }

        public KeyEvent getAcceleratorKeyEvent(short param ) {
            System.out.println("getAcceleratorKeyEvent called.");
            return new KeyEvent();
        }

        public void setAcceleratorKeyEvent(short param, KeyEvent param1 ) {
            System.out.println("setAcceleratorKeyEvent called.");
        }

        public void endExecute() {
            System.out.println("endExecute called.");
        }

        public boolean isInExecute() {
            System.out.println("isInExecute called.");
            return false;
        }

        public boolean isPopupMenu() {
            System.out.println("isPopupMenu called.");
            return true;
        }

        public String getTipHelpText(short param ) {
            System.out.println("getTipHelpText called.");
            return null;
        }

        public void setTipHelpText(short param, String param1 ) {
            System.out.println("setTipHelpText called.");
        }

        public String getHelpText(short param ) {
            System.out.println("getHelpText called.");
            return null;
        }

        public void setHelpText(short param, String param1 ) {
            System.out.println("setHelpText called.");
        }

        public String getHelpCommand(short param ) {
            System.out.println("getHelpCommand called.");
            return null;
        }

        public void setHelpCommand(short param, String param1 ) {
            System.out.println("setHelpCommand called.");
        }

        public String getCommand(short param ) {
            System.out.println("getCommand called.");
            return null;
        }

        public void setCommand(short param, String param1 ) {
            System.out.println("setCommand called.");
        }

        public void enableAutoMnemonics(boolean param ) {
            System.out.println("enableAutoMnemonics called.");
        }

        public void hideDisabledEntries(boolean param ) {
            System.out.println("hideDisabledEntries called.");
        }

        public com.sun.star.awt.MenuItemType getItemType(short param ) {
            System.out.println("getItemType called.");
            return com.sun.star.awt.MenuItemType.DONTKNOW;
        }

        public void clear() {
            System.out.println("clear called.");
        }
    }
}
