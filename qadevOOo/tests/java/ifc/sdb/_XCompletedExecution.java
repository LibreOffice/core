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
    public static interface CheckInteractionHandler extends XInteractionHandler {
        /**
         * Set a log of the listener, so messages of the listener get printed
         * into the file of the interface
         */
        public void setLog(PrintWriter log);
        /**
         * Return True, when the listener was called correctly.
         */
        public boolean checkInteractionHandler();
    }

    protected void before() {
        checkHandler = (CheckInteractionHandler)
                        tEnv.getObjRelation("InteractionHandlerChecker");
        if (checkHandler == null)
            throw new StatusException("Missing object relation 'InteractionHandlerChecker'", new Exception());
        checkHandler.setLog((PrintWriter)log);
    }


    public void _executeWithCompletion() {
        try {
            oObj.executeWithCompletion(checkHandler);
        }
        catch(com.sun.star.sdbc.SQLException e) {
            e.printStackTrace((PrintWriter)log);
            tRes.tested("executeWithCompletion()", false);
            return;
        }

        tRes.tested("executeWithCompletion()", checkHandler.checkInteractionHandler());
    }

    protected void after() {
        disposeEnvironment();
    }
}
