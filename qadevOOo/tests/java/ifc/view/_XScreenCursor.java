/*************************************************************************
 *
 *  $RCSfile: _XScreenCursor.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:33:12 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

