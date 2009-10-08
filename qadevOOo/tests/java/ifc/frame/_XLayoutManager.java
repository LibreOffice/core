/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: _XLayoutManager.java,v $
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
    private String sElementName = "private:resource/menubar/myMenu";

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
        XUIElement xElement = oObj.getElement(sElementName);
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
        tRes.tested("getElements()", xElements != null);
    }

    public void _hideElement() {
        boolean hidden = oObj.hideElement(sElementName);
        tRes.tested("hideElement()", true);
    }

    public void _isElementDocked() {
        boolean docked = oObj.isElementDocked(sElementName);
        tRes.tested("isElementDocked()", true);
    }

    public void _isElementFloating() {
        boolean docked = oObj.isElementFloating(sElementName);
        tRes.tested("isElementFloating()", true);
    }

    public void _isElementVisible() {
        boolean docked = oObj.isElementVisible(sElementName);
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
        boolean shown = oObj.showElement(sElementName);
        tRes.tested("showElement()", true);

    }

    public void _unlock() {
        requiredMethod("lock()");
        oObj.unlock();
        tRes.tested("unlock()", true);
    }

}
