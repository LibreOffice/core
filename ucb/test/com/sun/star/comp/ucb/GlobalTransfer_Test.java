/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
