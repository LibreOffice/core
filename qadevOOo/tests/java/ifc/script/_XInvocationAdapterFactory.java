/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XInvocationAdapterFactory.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:23:25 $
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

package ifc.script;

import lib.MultiMethodTest;
import lib.StatusException;

import com.sun.star.io.XInputStream;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.script.XInvocation;
import com.sun.star.script.XInvocationAdapterFactory;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Testing <code>com.sun.star.script.XInvocationAdapterFactory</code>
* interface methods :
* <ul>
*  <li><code> createAdapter()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.script.XInvocationAdapterFactory
*/
public class _XInvocationAdapterFactory extends MultiMethodTest {

    /**
     * oObj filled by MultiMethodTest
     */
    public XInvocationAdapterFactory oObj = null;

    /**
    * First an invocation object of <code>com.sun.star.io.Pipe</code>
    * instance is created using <code>com.sun.star.script.Invocation
    * </code> service. Then trying to create an adapter of this
    * invocation for <code>com.sun.star.io.XInputStream</code>
    * interface. <p>
    * Has <b>OK</b> status if the adapter returned is successfully
    * queried for <code>com.sun.star.io.XInputStream</code>
    * interface.
    * @see com.sun.star.script.Invocation
    * @see com.sun.star.script.XInvocation
    * @see com.sun.star.io.Pipe
    */
    public void _createAdapter() {
        XInvocation xInv = null ;
        XMultiServiceFactory xMSF = null;
        try {
            xMSF = (XMultiServiceFactory)tParam.getMSF();
            Object[] args = {xMSF.createInstance
                ("com.sun.star.io.Pipe")};

            Object oInvFac = xMSF.createInstance
                ("com.sun.star.script.Invocation") ;

            XSingleServiceFactory xInvFac = (XSingleServiceFactory) UnoRuntime.
                queryInterface(XSingleServiceFactory.class, oInvFac) ;

            Object oInv = xInvFac.createInstanceWithArguments(args) ;

            xInv = (XInvocation) UnoRuntime.queryInterface
                (XInvocation.class, oInv) ;

        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log) ;
            throw new StatusException("Cann't create invocation for object", e) ;
        }

        XInterface xInStr = null ;

        Object adp = oObj.createAdapter(xInv,
                new Type(XInputStream.class)) ;

        xInStr = (XInterface) UnoRuntime.queryInterface
                (XInputStream.class, adp) ;


        if (xInStr != null)
            tRes.tested("createAdapter()", true) ;
        else {
            log.println("Adapter created doesn't implement required interface") ;
            tRes.tested("createAdapter()", false) ;
        }
    }
}

