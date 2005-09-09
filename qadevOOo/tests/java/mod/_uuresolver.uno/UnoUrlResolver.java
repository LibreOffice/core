/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: UnoUrlResolver.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 04:55:13 $
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

package mod._uuresolver.uno;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.Exception;
import com.sun.star.uno.XInterface;
import java.io.PrintWriter;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

public class UnoUrlResolver extends TestCase {


    protected TestEnvironment createTestEnvironment(TestParameters tParam,
            PrintWriter log) {
        XMultiServiceFactory xMSF = (XMultiServiceFactory)tParam.getMSF();

        try {
            XInterface xInt = (XInterface)xMSF.createInstance(
                    "com.sun.star.bridge.UnoUrlResolver");
            TestEnvironment tEnv = new TestEnvironment(xInt);
            Object oBFctr = xMSF.createInstance(
                                        "com.sun.star.bridge.BridgeFactory");
            tEnv.addObjRelation("BRIDGEFACTORY", oBFctr);

            Object oAcctr = xMSF.createInstance(
                                        "com.sun.star.connection.Acceptor") ;
            tEnv.addObjRelation("ACCEPTOR", oAcctr);

            int port = util.utils.getNextFreePort(20004);
            tEnv.addObjRelation("PORT", new Integer(port));

            return tEnv;
        } catch (Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Unexpected exception", e);
        }

    }
}
