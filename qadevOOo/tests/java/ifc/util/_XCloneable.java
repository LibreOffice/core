/*************************************************************************
 *
 *  $RCSfile: _XCloneable.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:29:37 $
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

import lib.MultiMethodTest;

import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.XCloneable;

/**
* Testing <code>com.sun.star.util.XCloneable</code>
* interface methods :
* <ul>
*  <li><code> createClone()</code></li>
* </ul> <p>
* @see com.sun.star.util.XCloneable
*/
public class _XCloneable extends MultiMethodTest {

    // oObj filled by MultiMethodTest
    public XCloneable oObj = null ;
    protected XCloneable clone = null;

    /**
     * calls the method. <p>
     * Has <b>OK</b> status if no exception has occured. <p>
     */
    public void _createClone() {
        boolean result = true;
        clone = oObj.createClone();

        //check if the implementaionname equals
        result &= checkImplementationName(oObj,clone);

        //check ImplementationID
        result &= checkImplementationID(oObj, clone);

        tRes.tested("createClone()", result) ;
    }

    protected byte[] getImplementationID(XInterface ifc) {
        byte[] res = new byte[0];
        XTypeProvider provider = (XTypeProvider)
                    UnoRuntime.queryInterface(XTypeProvider.class, ifc);
        if (provider != null) {
            res = provider.getImplementationId();
        }
        return res;
    }

    protected boolean checkImplementationID(XInterface org, XInterface clone) {
        boolean res = getImplementationID(org).equals(
                                            getImplementationID(clone));
        if (res && getImplementationID(org).length > 0) {
            log.println("ImplementationID equals the clone has the same id as the original Object");
            log.println("------------------------------------------------------------------------");
        }
        return !res;
    }

    protected String getImplementationName(XInterface ifc) {
        String res = "";
        XServiceInfo info = (XServiceInfo)
                    UnoRuntime.queryInterface(XServiceInfo.class, ifc);
        if (info != null) {
            res = info.getImplementationName();
        }
        return res;
    }

    protected boolean checkImplementationName(XInterface org, XInterface clone) {
        boolean res = getImplementationName(org).equals(
                                            getImplementationName(clone));
        if (!res) {
            log.println("ImplementationName differs: ");
            log.println("Expected: "+getImplementationName(org));
            log.println("Gained: "+getImplementationName(clone));
            log.println("----------------------------------------");
        }
        return res;
    }

}  // finish class _XCloneable

