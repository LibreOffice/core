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
            log.println("Exception occurred while method call: " + e);
            result = false ;
        }

        tRes.tested("setParentStyle()",result);
    }
} //finish class _XStyle

