/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XPropertySetInfo.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:06:20 $
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

package ifc.beans;

import lib.MultiMethodTest;

import com.sun.star.beans.Property;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.XPropertySetInfo;

/**
* Testing <code>com.sun.star.beans.XPropertySetInfo</code>
* interface methods :
* <ul>
*  <li><code>getProperties()</code></li>
*  <li><code>getPropertyByName()</code></li>
*  <li><code>hasPropertyByName()</code></li>
* </ul>
* @see com.sun.star.beans.XPropertySetInfo
*/
public class _XPropertySetInfo extends MultiMethodTest {

    public XPropertySetInfo oObj = null;// oObj filled by MultiMethodTest

    public Property IsThere = null;

    /**
    * Test calls the method and stores one of the properties.<p>
    * Has <b> OK </b> status if the method successfully returns
    * value that isn't null.<p>
    */
    public void _getProperties() {
        Property[] properties = oObj.getProperties();
        IsThere = properties[0];
        tRes.tested("getProperties()", ( properties != null ));
        return;
    }

    /**
    * Test calls the method with property name that certainly present
    * in the property set and again calls the method with property name
    * that certainly doesn't present in the property set.<p>
    * Has <b> OK </b> status if the method in one case successfully
    * returns value that isn't null and no exceptions were thrown and
    * in other case exception was thrown.<p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code>getProperties()</code> : to have a property that certainly
    *  present in the property set</li>
    * </ul>
    */
    public void _getPropertyByName() {
        requiredMethod("getProperties()");
        boolean result;
        try {
            Property prop  = oObj.getPropertyByName(IsThere.Name);
            result = (prop != null);
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            log.println("Exception occurred while testing" +
                                " getPropertyByName with existing property");
            e.printStackTrace(log);
            result = false;
        }

        try {
            oObj.getPropertyByName("Jupp");
            log.println("No Exception thrown while testing"+
                                " getPropertyByName with non existing property");
            result = false;
        }
        catch (UnknownPropertyException e) {
            result = true;
        }
        tRes.tested("getPropertyByName()", result);
        return;
    }

    /**
    * Test calls the method with property name that certainly present
    * in the property set and again calls the method with property name
    * that certainly doesn't present in the property set.<p>
    * Has <b> OK </b> status if the method successfully returns true in
    * one case and false in other case.<p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code>getProperties()</code> : to have a property that certainly
    *  present in the property set</li>
    * </ul>
    */
    public void _hasPropertyByName() {
        requiredMethod("getProperties()");
        tRes.tested("hasPropertyByName()",
        (
            (oObj.hasPropertyByName(IsThere.Name)) &&
            (!oObj.hasPropertyByName("Jupp")) )
        );
    }

}    /// finish class XPropertySetInfo


