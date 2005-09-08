/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XStandaloneDocumentInfo.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:29:52 $
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

package ifc.document;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.beans.XPropertySet;
import com.sun.star.document.XStandaloneDocumentInfo;
import com.sun.star.io.IOException;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.document.XStandaloneDocumentInfo</code>
* interface methods. <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'DOCURL'</code> (of type <code>String</code>):
*   URL of document which info is loaded.</li>
* <ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.document.XStandaloneDocumentInfo
*/
public class _XStandaloneDocumentInfo extends MultiMethodTest {

    public XStandaloneDocumentInfo oObj = null;
    String url = null;

    protected void before() {
        url = (String)tEnv.getObjRelation("DOCURL");
        if (url == null) {
            throw new StatusException
                (Status.failed("Relation 'DOCURL' not found"));
        }
    }

    String oldProp = null;
    String newProp = null;
    /**
    * Sets new value of the property 'Author' and calls the method. <p>
    * Has <b> OK </b> status if no exception occured.
    */
    public void _storeIntoURL() {
        try {
            oObj.loadFromURL(url);
            XPropertySet propSet = (XPropertySet)
                UnoRuntime.queryInterface(XPropertySet.class, oObj);
            oldProp = (String)propSet.getPropertyValue("Author");
            newProp = oldProp + "_";
            propSet.setPropertyValue("Author", newProp);

            oObj.storeIntoURL(url);
            tRes.tested("storeIntoURL()", true);
        } catch (IOException e) {
            log.println("Couldn't store to " + url
                    + " : " + e.getMessage());
            e.printStackTrace(log);
            tRes.tested("storeIntoURL()", false);
        } catch(com.sun.star.lang.WrappedTargetException e) {
            log.println("Couldn't get/set property 'Author':" + e);
            tRes.tested("storeIntoURL()", false);
        } catch(com.sun.star.beans.UnknownPropertyException e) {
            log.println("Couldn't get/set property 'Author':" + e);
            tRes.tested("storeIntoURL()", false);
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println("Couldn't get/set property 'Author':" + e);
            tRes.tested("storeIntoURL()", false);
        } catch(com.sun.star.beans.PropertyVetoException e) {
            log.println("Couldn't get/set property 'Author':" + e);
            tRes.tested("storeIntoURL()", false);
        }
    }

    /**
    * Calls the method and checks value of the property 'Author'. <p>
    * Has <b> OK </b> status if no exception occured and value of the property
    * 'Author' is equal to value that was set in the method
    * <code>storeIntoURL</code>.
    */
    public void _loadFromURL() {
        requiredMethod("storeIntoURL()");
        try {
            oObj.loadFromURL(url);
            XPropertySet propSet = (XPropertySet)
                UnoRuntime.queryInterface(XPropertySet.class, oObj);
            String resProp = (String)propSet.getPropertyValue("Author");
            log.println("Was: '" + oldProp + "',Set: '" + newProp +
                        "', New: " + resProp + "'");
            tRes.tested("loadFromURL()", resProp.equals(newProp) );
        } catch (IOException e) {
            log.println("Couldn't load from " + url
                    + " : " + e.getMessage());
            e.printStackTrace(log);
            tRes.tested("loadFromURL()", false);
        } catch(com.sun.star.lang.WrappedTargetException e) {
            log.println("Couldn't get/set property 'Author':" + e);
            tRes.tested("loadFromURL()", false);
        } catch(com.sun.star.beans.UnknownPropertyException e) {
            log.println("Couldn't get/set property 'Author':" + e);
            tRes.tested("loadFromURL()", false);
        }
    }

}  // finish class _XStandaloneDocumentInfo

