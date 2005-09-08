/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XDocumentInfoSupplier.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:27:25 $
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
package ifc.document;

import com.sun.star.beans.XPropertySet;
import com.sun.star.document.XDocumentInfo;
import com.sun.star.document.XDocumentInfoSupplier;
import com.sun.star.uno.UnoRuntime;
import lib.MultiMethodTest;

/**
 * test the XDocumentInfoSupplier interface.
 */
public class _XDocumentInfoSupplier extends MultiMethodTest {
    public XDocumentInfoSupplier oObj = null;

    public void _getDocumentInfo() {
        // returns a com.sun.star.document.DocumentInfo
        XDocumentInfo info = oObj.getDocumentInfo();
        XPropertySet xPropertySet = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, info);
        String title = null;
        try {
            title = (String)xPropertySet.getPropertyValue("Title");
        }
        catch(Exception e) {
            e.printStackTrace((java.io.PrintWriter)log);
            title = null;
        }
        log.println("Got document title: " + title);
        tRes.tested("getDocumentInfo()", title != null);
    }
}
