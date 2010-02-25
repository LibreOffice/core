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

package ifc.drawing;

import lib.MultiPropertyTest;

import com.sun.star.container.XNamed;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.uno.UnoRuntime;
import util.ValueChanger;

/**
* Testing <code>com.sun.star.drawing.DrawingDocumentDrawView</code>
* service properties :
* <ul>
*  <li><code> IsMasterPageMode</code></li>
*  <li><code> IsLayerMode</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'DrawPage'</code> (of type <code>XDrawPage</code>):
*   a draw page which will be current page. </li>
* <ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.drawing.DrawingDocumentDrawView
*/
public class _DrawingDocumentDrawView extends MultiPropertyTest {

    XDrawPage drawPage = null;
    static String test_name = "For DrawingDocumentDrawView";

    protected void before() {
        drawPage = (XDrawPage)tEnv.getObjRelation("DrawPage");
        XNamed xNamed = (XNamed)
            UnoRuntime.queryInterface(XNamed.class, drawPage);
        xNamed.setName(test_name);
    }

    /**
    * Property tester which returns new <code>XDrawPage</code> object
    * and compares draw pages.
    */
    protected PropertyTester CurPageTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue) {
            return drawPage;
        }

        protected boolean compare(Object obj1, Object obj2) {
            XNamed named1 = (XNamed)UnoRuntime.queryInterface(XNamed.class, obj1);
            XNamed named2 = (XNamed)UnoRuntime.queryInterface(XNamed.class, obj2);
            boolean res = false;

            if (named1 != null && named2 != null) {
                String name1 = named1.getName();
                String name2 = named2.getName();
                res = name1.equals(name2);
            } else {
                log.println("Interface XNamed not supported");
            }

            return res;
        }

        protected String toString(Object obj) {
            XNamed named = (XNamed)UnoRuntime.queryInterface(XNamed.class, obj);
            String res = (named == null) ? "null" : named.getName();
            return res;
        }
    } ;

    public void _CurrentPage() {
        log.println("Testing with custom Property tester");
        Object oldCurPage = null;
        try {
            oldCurPage = oObj.getPropertyValue("CurrentPage");
        } catch(com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
        } catch(com.sun.star.beans.UnknownPropertyException e) {
            e.printStackTrace(log);
        }

        testProperty("CurrentPage", CurPageTester);

        try {
            log.println("Back old current page");
            oObj.setPropertyValue("CurrentPage", oldCurPage);
        } catch(com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
        } catch(com.sun.star.beans.UnknownPropertyException e) {
            e.printStackTrace(log);
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
        } catch(com.sun.star.beans.PropertyVetoException e) {
            e.printStackTrace(log);
        }
    }

    public void _IsMasterPageMode() {
        testProperty("IsMasterPageMode");
        try {
            oObj.setPropertyValue("IsMasterPageMode", new Boolean(false));
        } catch(com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
        } catch(com.sun.star.beans.UnknownPropertyException e) {
            e.printStackTrace(log);
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
        } catch(com.sun.star.beans.PropertyVetoException e) {
            e.printStackTrace(log);
        }
    }

    public void _IsLayerMode() {
        testProperty("IsLayerMode");
        try {
            oObj.setPropertyValue("IsLayerMode", new Boolean(false));
        } catch(com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
        } catch(com.sun.star.beans.UnknownPropertyException e) {
            e.printStackTrace(log);
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
        } catch(com.sun.star.beans.PropertyVetoException e) {
            e.printStackTrace(log);
        }
    }

    public void _ZoomType() {
        Object oldValue=null;
        Object newValue=null;
        try {
            oldValue = oObj.getPropertyValue("ZoomValue");
            Object oldZoom = oObj.getPropertyValue("ZoomType");
            Object newZoom = ValueChanger.changePValue(oldZoom);
            oObj.setPropertyValue("ZoomType", newZoom);
            newValue = oObj.getPropertyValue("ZoomValue");
        } catch(com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
        } catch(com.sun.star.beans.UnknownPropertyException e) {
            e.printStackTrace(log);
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
        } catch(com.sun.star.beans.PropertyVetoException e) {
            e.printStackTrace(log);
        }
        log.println("oldZoomValue: "+oldValue);
        log.println("newZoomValue: "+newValue);
        tRes.tested("ZoomType",(!oldValue.equals(newValue)));
    }

}

