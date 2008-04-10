/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: BasicHandlerProvider.java,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
package basicrunner;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.container.XSet;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.container.ElementExistException;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.container.NoSuchElementException;
import com.sun.star.connection.ConnectionSetupException;
import lib.TestParameters;
import share.LogWriter;
import basicrunner.basichelper.Connector;
import basicrunner.basichelper.DocumentHandler;
import basicrunner.basichelper.ThreadRunner;
import basicrunner.basichelper.AttributeList;
import basicrunner.basichelper.Filter;
import basicrunner.basichelper.DispatchProviderInterceptor;

/**
 * This class provides a BasicHandler. All classes for the communication with
 * and handling of  the BASIC tests are instantiated and inserted int the
 * MultiServiceFactory of StarOffice.
 */
public class BasicHandlerProvider {

    /** The BassicHandler **/
    static BasicHandler oHandler = null;
    /** The Connector **/
    static Connector oConnector = null;
    /** The DocumentHandler **/
    static DocumentHandler oDocumentHandler = null;
    /** The Thread Runner **/
    static ThreadRunner oThreadRunner = null;
    /** The AttributeList **/
    static AttributeList oAttributeList = null;
    /** The Filter **/
    static Filter oFilter = null;
    /** The DispatchProviderInterceptor **/
    static DispatchProviderInterceptor oCeptor = null ;
    /** The MultiServiceFactory from StarOffice **/
    static XMultiServiceFactory MSF = null;
    /** IS this a new connection or an existing one? **/
    static boolean bIsNewConnection = true;

    /**
     * Get a BasicHandler
     * @param tParam Test parameters.
     * @param log A log writer
     * @return An instance of BasicHandler
     */
    static public BasicHandler getHandler(TestParameters tParam, LogWriter log) {

        XMultiServiceFactory xMSF = (XMultiServiceFactory)tParam.getMSF();

        if (!xMSF.equals(MSF)) {
            MSF = xMSF;
            oHandler = new BasicHandler(tParam);
            oConnector = new Connector();
            oFilter = new Filter();
            oDocumentHandler = new DocumentHandler();
        oThreadRunner = new ThreadRunner(xMSF);
            oCeptor = new DispatchProviderInterceptor() ;
            oAttributeList = new AttributeList();
            XSet xMSFSet = (XSet)UnoRuntime.queryInterface(XSet.class, xMSF);

            try {
                xMSFSet.insert(oHandler);
                xMSFSet.insert(oConnector);
                xMSFSet.insert(oFilter);
                xMSFSet.insert(oDocumentHandler);
        xMSFSet.insert(oThreadRunner);
                xMSFSet.insert(oCeptor);
                xMSFSet.insert(oAttributeList);
            } catch (ElementExistException e) {
                System.out.println(e.toString());
            } catch (IllegalArgumentException e) {
                System.out.println(e.toString());
            }

            try {
                oHandler.Connect(util.utils.getFullURL((String)tParam.get("BASICBRIDGE")),
                                                               tParam, xMSF, log);
            } catch (ConnectionSetupException e) {
                System.out.println("Can't connect to BASIC !");
            }

            bIsNewConnection = true;
        } else {
            bIsNewConnection = false;
        }

        return oHandler;
    }

    /**
     * Is this a new connection?
     * @return True, if the connection did not exist before.
     */
    static public boolean isNewConnection() {
        return bIsNewConnection;
    }

    /**
     * Dispose the BasicHandler
     */
    static public void disposeHandler() {

        try {
            if (oHandler != null) {
                oHandler.dispose();
            }
            if (MSF != null) {
                XSet xMSFSet = (XSet)UnoRuntime.queryInterface(XSet.class, MSF);
                xMSFSet.remove(oHandler);
                xMSFSet.remove(oFilter);
                xMSFSet.remove(oConnector);
                xMSFSet.remove(oDocumentHandler);
        xMSFSet.remove(oThreadRunner);
                xMSFSet.remove(oAttributeList);
            }
        } catch (Exception e){
            System.out.println(e.toString());
        }

        MSF = null;
        oHandler = null;
    }
}
