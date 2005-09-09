/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XStyle.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:03:28 $
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

package ifc.style;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.style.XStyle;

/**
* Testing <code>com.sun.star.style.XStyle</code>
* interface methods :
* <ul>
*  <li><code> isUserDefined()</code></li>
*  <li><code> isInUse()</code></li>
*  <li><code> getParentStyle()</code></li>
*  <li><code> setParentStyle()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'PoolStyle'</code> (of type <code>XStyle</code>):
*   some style from the SOffice collection (not user defined) </li>
* <ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.style.XStyle
*/
public class _XStyle extends MultiMethodTest {

    public XStyle oObj = null;
    XStyle oMyStyle = null;

    /**
    * Retrieves object relations.
    * @throws StatusException If one of relations not found.
    */
    public void before() {
        oMyStyle = (XStyle) tEnv.getObjRelation("PoolStyle");
        if (oMyStyle == null) throw new StatusException
            (Status.failed("Relation not found")) ;
    }

    /**
    * Gets the parent style . <p>
    * Has <b> OK </b> status if the name of style returned is
    * equal to the name of style which was set before. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> setParentStyle() </code> : to set the parent style </li>
    * </ul>
    */
    public void _getParentStyle() {
        requiredMethod("setParentStyle()");
        tRes.tested("getParentStyle()",
            oObj.getParentStyle().equals(oMyStyle.getName()));
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method sreturns <code>true</code>. <p>
    */
    public void _isInUse() {
        tRes.tested("isInUse()",oObj.isInUse());
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method returns <code>true</code>. <p>
    */
    public void _isUserDefined() {
        tRes.tested("isUserDefined()",
            oObj.isUserDefined() && !oMyStyle.isUserDefined() );
    }

    /**
    * Sets the style name which was passed as relation. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    */
    public void _setParentStyle() {
        boolean result = true ;
        try {
            oObj.setParentStyle(oMyStyle.getName());
        } catch (com.sun.star.container.NoSuchElementException e) {
            log.println("Exception occured while method call: " + e);
            result = false ;
        }

        tRes.tested("setParentStyle()",result);
    }
} //finish class _XStyle

