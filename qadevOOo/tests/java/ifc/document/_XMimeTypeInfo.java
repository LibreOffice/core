/*************************************************************************
 *
 *  $RCSfile: _XMimeTypeInfo.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:27:21 $
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

package ifc.document;


import lib.MultiMethodTest;

import com.sun.star.document.XMimeTypeInfo;
import com.sun.star.lang.XComponent;

/**
 * Testing <code>com.sun.star.document.XMimeTypeInfo</code>
 * interface methods :
 * <ul>
 *  <li><code> supportsMimeType()</code></li>
 *  <li><code> getSupportedMimeTypeNames()</code></li>
 * </ul> <p>
 *
 * @see com.sun.star.document.XMimeTypeInfo
 */
public class _XMimeTypeInfo extends MultiMethodTest {

    public XMimeTypeInfo oObj = null;
    public XComponent source = null ;
    public String[] smi = null;

    /**
    * Gets supported types and stores them. <p>
    * Has <b> OK </b> status if at least one type exists.
    */
    public void _getSupportedMimeTypeNames() {
        smi = oObj.getSupportedMimeTypeNames();
        tRes.tested("getSupportedMimeTypeNames()", smi.length>0) ;
    }

    /**
     * Calls the method for one supported type retrieved by
     * <code>getSupportedMimeTypeNames</code> method and for
     * bad type. <p>
     *
     * Has <b> OK </b> status if <code>true</code> returned for
     * supported type and <code>false</code> for bad type.
     *
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> getSupportedMimeTypeNames </code> : to have a list of
     *    supported types. </li>
     * </ul>
     */
    public void _supportsMimeType() {
        requiredMethod("getSupportedMimeTypeNames()");
        boolean pos = false;
        pos = oObj.supportsMimeType(smi[0]);
        if (!pos) {
            log.println("Method returns false for existing MimeType");
        }
        boolean neg = true;
        neg = oObj.supportsMimeType("NoRealMimeType");
        if (neg) {
            log.println("Method returns true for non existing MimeType");
        }
        tRes.tested("supportsMimeType()", (pos && !neg)) ;
    }
}


