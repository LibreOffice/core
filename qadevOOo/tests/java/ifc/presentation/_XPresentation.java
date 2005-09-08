/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XPresentation.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:21:15 $
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

package ifc.presentation;

import lib.MultiMethodTest;

import com.sun.star.presentation.XPresentation;

/**
* Testing <code>com.sun.star.presentation.XPresentation</code>
* interface methods :
* <ul>
*  <li><code> start()</code></li>
*  <li><code> end()</code></li>
*  <li><code> rehearseTimings()</code></li>
* </ul> <p>
* @see com.sun.star.presentation.XPresentation
*/
public class _XPresentation extends MultiMethodTest {

    public XPresentation oObj = null;

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns. <p>
    */
    public void _start() {
        oObj.start();
        tRes.tested("start()", true);
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> start() </code> : presentation must be started before</li>
    * </ul>
    */
    public void _end() {
        requiredMethod("start()");

        oObj.end();
        tRes.tested("end()", true);
    }

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns. <p>
    */
    public void _rehearseTimings() {
        oObj.rehearseTimings();
        tRes.tested("rehearseTimings()", true);
    }

}  // finish class _XPresentation


