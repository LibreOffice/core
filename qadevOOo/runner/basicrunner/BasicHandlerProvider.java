/*************************************************************************
 *
 *  $RCSfile: BasicHandlerProvider.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-05-27 12:01:08 $
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
