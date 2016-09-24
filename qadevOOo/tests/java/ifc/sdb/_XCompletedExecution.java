/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
package ifc.sdb;

import com.sun.star.sdb.XCompletedExecution;
import com.sun.star.task.XInteractionHandler;
import java.io.PrintWriter;
import lib.MultiMethodTest;
import lib.StatusException;

/**
 *
 */
public class _XCompletedExecution extends MultiMethodTest {

    // oObj filled by MultiMethodTest
    public XCompletedExecution oObj = null ;

    private CheckInteractionHandler checkHandler = null;
    /**
     * Interface to implement so the call of the listener can be checked.
     */
    public interface CheckInteractionHandler extends XInteractionHandler {
        /**
         * Set a log of the listener, so messages of the listener get printed
         * into the file of the interface
         */
        void setLog(PrintWriter log);
        /**
         * Return True, when the listener was called correctly.
         */
        boolean checkInteractionHandler();
    }

    @Override
    protected void before() {
        checkHandler = (CheckInteractionHandler)
                        tEnv.getObjRelation("InteractionHandlerChecker");
        if (checkHandler == null)
            throw new StatusException("Missing object relation 'InteractionHandlerChecker'", new Exception());
        checkHandler.setLog(log);
    }


    public void _executeWithCompletion() {
        try {
            oObj.executeWithCompletion(checkHandler);
        }
        catch(com.sun.star.sdbc.SQLException e) {
            e.printStackTrace(log);
            tRes.tested("executeWithCompletion()", false);
            return;
        }

        tRes.tested("executeWithCompletion()", checkHandler.checkInteractionHandler());
    }

    @Override
    protected void after() {
        disposeEnvironment();
    }
}
