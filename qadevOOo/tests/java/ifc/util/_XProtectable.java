/*************************************************************************
 *
 *  $RCSfile: _XProtectable.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Date: 2004-11-02 11:58:28 $
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
