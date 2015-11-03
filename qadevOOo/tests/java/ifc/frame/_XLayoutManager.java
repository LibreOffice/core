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

import com.sun.star.awt.Point;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XLayoutManager;
import com.sun.star.awt.Rectangle;
import com.sun.star.awt.Size;
import com.sun.star.ui.DockingArea;
import com.sun.star.ui.XUIElement;
import lib.MultiMethodTest;
import util.utils;

public class _XLayoutManager extends MultiMethodTest {
    public XLayoutManager oObj = null;

    private XFrame xFrame = null;
    private static final String sElementName = "private:resource/menubar/myMenu";

    @Override
    public void before() {
        xFrame = (XFrame)tEnv.getObjRelation("XLayoutManager.Frame");
    }

    public void _attachFrame() {
        oObj.attachFrame(xFrame);
        tRes.tested("attachFrame()", true);
    }

    public void _createElement() {
        requiredMethod("attachFrame()");
        oObj.createElement(sElementName);
        tRes.tested("createElement()", true);
    }

    public void _destroyElement() {
        requiredMethod("getElement()");
        requiredMethod("getElements()");
        oObj.destroyElement(sElementName);
        tRes.tested("destroyElement()", true);
    }

    public void _doLayout() {
        oObj.doLayout();
        tRes.tested("doLayout()", true);
    }

    public void _dockWindow() {
        requiredMethod("createElement()");
        Point p = new Point();
        p.X = 0;
        p.Y = 0;
        oObj.dockWindow(sElementName, DockingArea.DOCKINGAREA_TOP, p);
        tRes.tested("dockWindow()", true);
    }

    public void _floatWindow() {
        requiredMethod("createElement()");
        oObj.floatWindow(sElementName);
        tRes.tested("floatWindow()", true);
    }

    public void _getCurrentDockingArea() {
        requiredMethod("attachFrame()");
        Rectangle rect = oObj.getCurrentDockingArea();
        tRes.tested("getCurrentDockingArea()", rect != null);
    }

    public void _getElement() {
        requiredMethod("createElement()");
        oObj.getElement(sElementName);
        tRes.tested("getElement()", true);
    }

    public void _getElementPos() {
        Point p = oObj.getElementPos(sElementName);
        tRes.tested("getElementPos()", p != null);
    }

    public void _getElementSize() {
        Size s = oObj.getElementSize(sElementName);
        tRes.tested("getElementSize()", s != null);
    }

    public void _getElements() {
        requiredMethod("createElement()");
        XUIElement[] xElements = oObj.getElements();
        for (int i=0; i<xElements.length; i++) {
            Object o = xElements[i].getRealInterface();
            log.println("Element " + i + ": " + o.toString() + "         " + utils.getImplName(o));
        }
        tRes.tested("getElements()", true);
    }

    public void _hideElement() {
        oObj.hideElement(sElementName);
        tRes.tested("hideElement()", true);
    }

    public void _isElementDocked() {
        oObj.isElementDocked(sElementName);
        tRes.tested("isElementDocked()", true);
    }

    public void _isElementFloating() {
        oObj.isElementFloating(sElementName);
        tRes.tested("isElementFloating()", true);
    }

    public void _isElementVisible() {
        oObj.isElementVisible(sElementName);
        tRes.tested("isElementVisible()", true);
    }

    public void _lock() {
        oObj.lock();
        tRes.tested("lock()", true);
    }

    public void _reset() {
        oObj.reset();
        tRes.tested("reset()", true);
    }

    public void _setDockingAreaAcceptor() {
        oObj.setDockingAreaAcceptor(null);
        tRes.tested("setDockingAreaAcceptor()", true);
    }

    public void _setElementPos() {
        requiredMethod("createElement()");
        Point p = new Point();
        p.X = 0;
        p.Y = 150;
        oObj.setElementPos(sElementName, p);
        tRes.tested("setElementPos()", true);
    }

    public void _setElementPosSize() {
        requiredMethod("createElement()");
        Point p = new Point();
        p.X = 150;
        p.Y = 0;
        Size size = new Size();
        size.Height = 50;
        size.Width = 80;
        oObj.setElementPosSize(sElementName, p, size);
        tRes.tested("setElementPosSize()", true);
    }

    public void _setElementSize() {
        requiredMethod("createElement()");
        Size size = new Size();
        size.Height = 80;
        size.Width = 50;
        oObj.setElementSize(sElementName, size);
        tRes.tested("setElementSize()", true);
    }

    public void _showElement() {
        requiredMethod("createElement()");
        oObj.showElement(sElementName);
        tRes.tested("showElement()", true);

    }

    public void _unlock() {
        requiredMethod("lock()");
        oObj.unlock();
        tRes.tested("unlock()", true);
    }

}
