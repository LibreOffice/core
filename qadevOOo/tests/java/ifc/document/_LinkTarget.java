/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _LinkTarget.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:26:11 $
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

import lib.MultiPropertyTest;

/**
* Testing <code>com.sun.star.document.LinkTarget</code>
* service properties :
* <ul>
*  <li><code> LinkDisplayName </code></li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.document.LinkTarget
*/
public class _LinkTarget extends MultiPropertyTest {

/*
    public XPropertySet oObj = null;  // oObj filled by MultiMethodTest

    public void _LinkDisplayName() {
        // check if Service is available
        XServiceInfo xInfo = (XServiceInfo)
            UnoRuntime.queryInterface(XServiceInfo.class, oObj);
        String [] sa = xInfo.getSupportedServiceNames();

        for (int i = 0; i < sa.length; i++) {
            System.out.println(sa[i]);
        }

        if ( ! xInfo.supportsService("com.sun.star.document.LinkTarget")) {
            log.println("Service not available !!!!!!!!!!!!!");
            tRes.tested("Supported", false);
        }
        //the only property is read only
        try {
            String LDN = (String) oObj.getPropertyValue("LinkDisplayName");
            tRes.tested("LinkDisplayName",LDN!=null);
        }
        catch (Exception ex) {
            log.println("Exception occured during LinkDisplayName");
            ex.printStackTrace(log);
            tRes.tested("LinkDisplayName",false);
        }

    }// EOF LinkDisplayName
*/

}

