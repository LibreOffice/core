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

package ifc.view;

import lib.MultiMethodTest;

import com.sun.star.view.XScreenCursor;


/**
 * Testing <code>com.sun.star.view.XScreenCursor</code>
 * interface methods :
 * <ul>
 *  <li><code> screenDown()</code></li>
 *  <li><code> screenUp()</code></li>
 * </ul> <p>
 *
 * Uses test parameter <b><code>soapi.test.hidewindows</code></b>.
 * Methods of this interface works with view representation,
 * thereby their calls have no effect when document is opened
 * in hidden mode. <p>
 *
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.view.XScreenCursor
 */
public class _XScreenCursor extends MultiMethodTest {

    // oObj filled by MultiMethodTest

    public XScreenCursor oObj = null;

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method returns
     * <code>true</code> value or if document is
     * opened in hidden mode.
     */
    public void _screenDown(){
        log.println("test for screenDown() ");
        tRes.tested("screenDown()", oObj.screenDown() );
    }

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method returns
     * <code>true</code> value or if document is
     * opened in hidden mode.
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> screenDown() </code> : by default view is on the
     *  top of pag, thereby it's needed first to moved down. </li>
     * </ul>
     */
    public void _screenUp(){
        requiredMethod("screenDown()") ;

        log.println("test for screenUp() ");
        tRes.tested("screenUp()", oObj.screenUp() );
    }

}  // finish class _XScreenCursor

