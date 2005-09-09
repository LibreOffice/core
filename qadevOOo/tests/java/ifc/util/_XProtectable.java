/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XProtectable.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:43:08 $
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
package ifc.util;

import com.sun.star.util.XProtectable;
import lib.MultiMethodTest;

/**
 * Check the XProtectable interface.
 */
public class _XProtectable extends MultiMethodTest {
    public XProtectable oObj = null;
    String sPassWord = "TopSecret";

    /**
     * Check, if the sheet is protected.
     * Has OK status, if this is the case.
     */
    public void _isProtected() {
        requiredMethod("protect()");
        boolean result = oObj.isProtected();
        tRes.tested("isProtected()", result);
    }

    /**
     * Protect the sheet
     */
    public void _protect() {
        oObj.protect(sPassWord);
        tRes.tested("protect()", true);
    }

    /**
     * Unprotect with wrong password, see if it's still protected.
     * Unprotect with correct password, see if it's unprotected.
     */
    public void _unprotect() {
        requiredMethod("isProtected()");
        boolean result = true;
        try {
            oObj.unprotect("WrongPassword");
//            result = false;
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println("Correct Exception thrown.");
        }
        // just check if it's still protected
        result &= oObj.isProtected();
        try {
            oObj.unprotect(sPassWord);
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println("Wrong Exception thrown: password is correct.");
            result = false;
        }
        result &= !oObj.isProtected();
        tRes.tested("unprotect()", result);
    }

}
