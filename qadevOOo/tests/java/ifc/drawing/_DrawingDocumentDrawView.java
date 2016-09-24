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

    @Override
    protected void before() {
        drawPage = (XDrawPage)tEnv.getObjRelation("DrawPage");
        XNamed xNamed = UnoRuntime.queryInterface(XNamed.class, drawPage);
        xNamed.setName(test_name);
    }

    /**
    * Property tester which returns new <code>XDrawPage</code> object
    * and compares draw pages.
    */
    protected PropertyTester CurPageTester = new PropertyTester() {
        @Override
        protected Object getNewValue(String propName, Object oldValue) {
            return drawPage;
        }

        @Override
        protected boolean compare(Object obj1, Object obj2) {
            XNamed named1 = UnoRuntime.queryInterface(XNamed.class, obj1);
            XNamed named2 = UnoRuntime.queryInterface(XNamed.class, obj2);
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

        @Override
        protected String toString(Object obj) {
            XNamed named = UnoRuntime.queryInterface(XNamed.class, obj);
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
            oObj.setPropertyValue("IsMasterPageMode", Boolean.FALSE);
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
            oObj.setPropertyValue("IsLayerMode", Boolean.FALSE);
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
        if (oldValue != null)
            tRes.tested("ZoomType",(!oldValue.equals(newValue)));
    }

}

