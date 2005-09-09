/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XTextField.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:25:53 $
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

package ifc.text;

import lib.MultiMethodTest;

import com.sun.star.text.XTextField;

/**
 * Testing <code>com.sun.star.text.XTextField</code>
 * interface methods :
 * <ul>
 *  <li><code> getPresentation()</code></li>
 * </ul> <p>
 * Test is multithread compilant. <p>
 * @see com.sun.star.text.XTextField
 */
public class _XTextField extends MultiMethodTest{
    public XTextField oObj = null;

    /**
     * Calls the method with <code>true</code> and <code>false</code>
     * parameter. <p>
     * Has <b>OK</b> status if in both cases not <code>null</code>
     * value returned.
     */
    public void _getPresentation() {

        boolean result = true;

        // begin test here
        log.println("getting presentetion with bShowCommand flag...");
        result &= oObj.getPresentation(true) != null;
        log.println("getting presentetion without bShowCommand flag...");
        result &= oObj.getPresentation(false) != null;

        tRes.tested( "getPresentation()", result );

     } // end getPresentation()
}

