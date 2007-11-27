/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XRequestCallback.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-27 11:43:23 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
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

package complex.toolkit.interface_tests;

import com.sun.star.awt.XRequestCallback;
import lib.MultiMethodTest;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;
import share.LogWriter;

/**
 * Testing <code>com.sun.star.awt.XRequestCallback</code>
 * interface methods :
 * <ul>
 *  <li><code> addCallback()</code></li>
 * </ul> <p>
 * @see com.sun.star.awt.XRequestCallback
 */
public class _XRequestCallback {

    private LogWriter log;

    private static final String className =
        "com.sun.star.awt.XRequestCallback" ;

    public XRequestCallback oObj = null;
    private XMultiServiceFactory xMSF;

    String text = null;


    public _XRequestCallback(XInterface object, LogWriter log, XMultiServiceFactory xMSF ) {
        oObj = (XRequestCallback)UnoRuntime.queryInterface(
                                    XRequestCallback.class, object);
        this.xMSF = xMSF;
        this.log = log;
    }


    /**
     * Calls the method and checks returned value.
     * Has OK status if returned value is equal to <code>chCount - 1</code>.
     * The following method tests are to be executed before:
     * <ul>
     *  <li> <code>addCallback()</code> </li>
     * </ul>
     */
    public boolean _addCallback() {

        boolean res = true;
        try {
            Object a = new Object();
            oObj.addCallback( null, a );
        } catch (com.sun.star.uno.RuntimeException ie) {
            res = false;
        }
        log.println("addCallback called" );

        return res;
    }

}
