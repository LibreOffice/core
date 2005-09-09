/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: GlobalTransfer_Test.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 16:18:19 $
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

package com.sun.star.comp.ucb;

import complexlib.ComplexTestCase;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.task.XInteractionAbort;
import com.sun.star.task.XInteractionHandler;
import com.sun.star.task.XInteractionRequest;
import com.sun.star.task.XInteractionContinuation;
import com.sun.star.ucb.Command;
import com.sun.star.ucb.GlobalTransferCommandArgument;
import com.sun.star.ucb.NameClash;
import com.sun.star.ucb.NameClashResolveRequest;
import com.sun.star.ucb.TransferCommandOperation;
import com.sun.star.ucb.XCommandEnvironment;
import com.sun.star.ucb.XCommandProcessor;
import com.sun.star.ucb.XInteractionReplaceExistingData;
import com.sun.star.ucb.XInteractionSupplyName;
import com.sun.star.ucb.XProgressHandler;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.UnoRuntime;
import java.io.PrintWriter;

public final class GlobalTransfer_Test extends ComplexTestCase {

    static private final String fileName
        = "testcase-do-not-remove.sxw";
    static private final String httpSourceDir
        = "http://so-berlin/~webdav/";
    static private final String httpTargetDir
        = "http://so-berlin/~webdav/";
    static private final String fileSourceDir
        = "file:///d:/temp/source/";
    static private final String fileTargetDir
        = "file:///d:/temp/";

    public String getTestObjectName() {
        return getClass().getName();
    }

    public String[] getTestMethodNames() {
        return new String[] { "testNameClashASK" };
    }

    public void testNameClashASK() throws Exception {
        Object oObj = null;
        try {
           XMultiServiceFactory xMSF = (XMultiServiceFactory)param.getMSF();
           oObj
            = xMSF.createInstance( "com.sun.star.ucb.UniversalContentBroker" );
        }
        catch(com.sun.star.uno.Exception e) {
            e.printStackTrace((PrintWriter)log);

            // After this exception the test has failed and cannot continue.
            failed( "Cannot create service instance: com.sun.star.ucb." +
                    "UniversalContentBroker. message:" + e.getMessage() );
            return;
        }

        if ( oObj == null ) {
            failed( "Cannot create service instance: com.sun.star.ucb." +
                    "UniversalContentBroker");
            return;
        }

        XCommandProcessor xCmdProc
            = (XCommandProcessor)UnoRuntime.queryInterface(
                                               XCommandProcessor.class, oObj );
        assure( "UCB does not implement mandatory interface XCommandProcessor!",
                xCmdProc != null);

        ResourceCopier cp = new ResourceCopier( xCmdProc );

        try {
            cp.copyResource( httpSourceDir, fileTargetDir, fileName );
        }
        catch(com.sun.star.uno.Exception e) {
            e.printStackTrace((PrintWriter)log);

            // After this exception the test has failed and cannot continue.
            failed( "Could not copy resource:" + e.getMessage() );
        }
    }

    private final class ResourceCopier {
        private XCommandProcessor  m_cmdProc = null;
        private CommandEnvironment m_env = new CommandEnvironment();

        ResourceCopier( XCommandProcessor oCmdProc )
        {
            m_cmdProc = oCmdProc;
        }

        public void copyResource(
                String sourceDir, String targetDir, String fileName )
            throws Exception {

            GlobalTransferCommandArgument transferArg
                = new GlobalTransferCommandArgument(
                    TransferCommandOperation.COPY,
                    sourceDir + fileName,
                    targetDir,
                    "",
                    NameClash.ASK );

            Command cmd = new Command( "globalTransfer", -1, transferArg );

            m_cmdProc.execute( cmd, 0, m_env );
        }
    }

    private final class CommandEnvironment implements XCommandEnvironment {
        private final XInteractionHandler m_InteractionHandler
            = new InteractionHadler();

        public XInteractionHandler getInteractionHandler() {
            return m_InteractionHandler;
        }

        public XProgressHandler getProgressHandler() {
            // not needed for the test.
            return null;
        }
    }

    private final class InteractionHadler implements XInteractionHandler {
        public void handle( /*IN*/XInteractionRequest Request ) {

            log.println( "Interaction Handler called." );

            try {
                NameClashResolveRequest req = (NameClashResolveRequest)
                    AnyConverter.toObject(
                        NameClashResolveRequest.class, Request.getRequest() );

                log.println( "Interaction Handler: NameClashResolveRequest: "
                             + req.ClashingName );

                XInteractionContinuation[] continuations
                    = Request.getContinuations();
                for ( int i = 0; i < continuations.length; ++i ) {
/*
                    XInteractionAbort xAbort
                        = (XInteractionAbort)UnoRuntime.queryInterface(
                            XInteractionAbort.class, continuations[ i ] );
                    if ( xAbort != null ) {
                        log.println( "Interaction Handler selects: ABORT" );
                        xAbort.select();
                        return;
                    }
*/
/*
                    XInteractionReplaceExistingData xReplace
                        = (XInteractionReplaceExistingData)
                            UnoRuntime.queryInterface(
                                XInteractionReplaceExistingData.class,
                                continuations[ i ] );
                    if ( xReplace != null ) {
                        log.println( "Interaction Handler selects: REPLACE" );
                        xReplace.select();
                        return;
                    }
*/
                    XInteractionSupplyName xSupplyName
                        = (XInteractionSupplyName)
                            UnoRuntime.queryInterface(
                                XInteractionSupplyName.class,
                                continuations[ i ] );
                    if ( xSupplyName != null ) {
                        String newname = "renamed_" + req.ClashingName;
                        log.println( "Interaction Handler selects: NEW NAME: "
                                     + newname );
                        xSupplyName.setName( newname );
                        xSupplyName.select();
                        return;
                    }
                }
            }
            catch ( IllegalArgumentException e )
            {
                e.printStackTrace();
            }
        }
    }

}
