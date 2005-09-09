/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XParameterizedContentProvider.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:32:16 $
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

package ifc.ucb;

import lib.MultiMethodTest;

import com.sun.star.ucb.XContentProvider;
import com.sun.star.ucb.XParameterizedContentProvider;

/**
* Testing <code>com.sun.star.ucb.XParameterizedContentProvider</code>
* interface methods :
* <ul>
*  <li><code> registerInstance()</code></li>
*  <li><code> deregisterInstance()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.ucb.XParameterizedContentProvider
*/
public class _XParameterizedContentProvider extends MultiMethodTest {

    public static XParameterizedContentProvider oObj = null;

    /**
     * Registers an instance with some arguments. <p>
     * Has <b>OK</b> status if the method returns non <code>null</code>
     * provider.
     */
    public void _registerInstance() {
        try {
            XContentProvider CP = oObj.registerInstance(
                        "\"vnd.sun.star.pkg://file\".*",
                        "uno:pipe,name=ucb_soffice;<PIPE>;urp;UCB.Factory",
                        true);
            tRes.tested("registerInstance()",CP != null);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Exception while checking 'registerInstance'");
            e.printStackTrace(log);
            tRes.tested("registerInstance()",false);
        }
    }

    /**
     * Deregisters the instance registered before. <p>
     * Has <b>OK</b> status if the method returns non <code>null</code>
     * provider. <p>
     *
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> registerInstance() </code> : to deregister it here. </li>
     * </ul>
     */
    public void _deregisterInstance() {
        requiredMethod("registerInstance()");
        try {
            XContentProvider CP = oObj.deregisterInstance(
                        "\"vnd.sun.star.pkg://file\".*",
                        "uno:pipe,name=ucb_soffice;<PIPE>;urp;UCB.Factory");
            tRes.tested("deregisterInstance()",CP != null);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Exception while checking 'deregisterInstance'");
            e.printStackTrace(log);
            tRes.tested("deregisterInstance()",false);
        }
    }

}

