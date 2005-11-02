/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _DrawingDocumentDrawView.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 17:45:31 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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

