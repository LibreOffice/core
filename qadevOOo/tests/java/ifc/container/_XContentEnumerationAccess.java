/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XContentEnumerationAccess.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:20:17 $
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

package ifc.container;

import lib.MultiMethodTest;

import com.sun.star.container.XContentEnumerationAccess;
import com.sun.star.container.XEnumeration;

/**
* Testing <code>com.sun.star.container.XContentEnumerationAccess</code>
* interface methods :
* <ul>
*  <li><code> createContentEnumeration()</code></li>
*  <li><code> getAvailableServiceNames()</code></li>
* </ul> <p>
* @see com.sun.star.container.XContentEnumerationAccess
*/
public class _XContentEnumerationAccess extends MultiMethodTest{
    public XContentEnumerationAccess oObj = null;
    String[] serviceNames = null;

    /**
    * Retrieves service names and stores them. <p>
    * Has <b> OK </b> status if not <code>null</code>
    * value returned.
    */
    public void _getAvailableServiceNames(){
        boolean bResult = true;
        try {
            serviceNames = oObj.getAvailableServiceNames();
            bResult = serviceNames != null ;
        } catch (Exception e) {
            log.println("Exception occured. " + e);
            bResult = false;
        }
        tRes.tested("getAvailableServiceNames()", bResult);
    }

    /**
    * If at least one service available then an enumeration for
    * it created. <p>
    * Has <b> OK </b> status if no services available or enumeration
    * created for available service is not <code>null</code>.
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code>getAvailableServiceNames()</code> :
    *    to have at least one service name for enumeration to create for.</li>
    * </ul>
    */
    public void _createContentEnumeration(){
        requiredMethod("getAvailableServiceNames()");

        if (serviceNames.length == 0) {
            log.println("No service name available") ;
            tRes.tested("createContentEnumeration()", true) ;
            return ;
        }

        boolean bResult = true;

        log.println( "creating Enumeration" );
        XEnumeration oEnum = oObj.createContentEnumeration(serviceNames[0]);
        bResult &= oEnum != null;

        tRes.tested( "createContentEnumeration()",  bResult);
    }
}


