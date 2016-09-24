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
package ifc.form;

import com.sun.star.form.XDatabaseParameterBroadcaster;
import com.sun.star.form.XDatabaseParameterListener;
import com.sun.star.sdbc.XRowSet;
import com.sun.star.uno.UnoRuntime;
import java.io.PrintWriter;
import lib.MultiMethodTest;

/**
 *
 */
public class _XDatabaseParameterBroadcaster extends MultiMethodTest {

    // oObj filled by MultiMethodTest
    public XDatabaseParameterBroadcaster oObj = null ;
    private CheckParameterListener listenerChecker = null;

    /**
     * Interface to implement so the call of the listener can be checked.
     */
    public interface CheckParameterListener extends XDatabaseParameterListener {
        /**
         * Set a log of the listener, so messages of the listener get printed
         * into the file of the interface
         */
        void setLog(PrintWriter log);
        /**
         * Return True, when the listener was called correctly.
         */
        boolean checkListener();
    }

    /**
     * Get the object relation 'ParameterListenerChecker' and
     * set the log inside of the implementation.
     */
    @Override
    protected void before() {
        listenerChecker = (CheckParameterListener)
                        tEnv.getObjRelation("ParameterListenerChecker");
        listenerChecker.setLog(log);
    }

    /**
     */
    public void _addParameterListener() {
        oObj.addParameterListener(listenerChecker);
        tRes.tested("addParameterListener()", true);
    }

    /**
     */
    public void _removeParameterListener() {
        requiredMethod("addParameterListener()");

        // trigger the action.
        try {
            XRowSet xRowSet = UnoRuntime.queryInterface(XRowSet.class, oObj);
            xRowSet.execute();
        }
        catch(com.sun.star.sdbc.SQLException e) {
            log.println("Exception in XDatabaseParameterBroadcaster test.");
            log.println("This does not let the test fail, but should be inquired.");
            e.printStackTrace(log);
        }
        // was the listener called?
        oObj.removeParameterListener(listenerChecker);
        tRes.tested("removeParameterListener()", listenerChecker.checkListener());
    }

    @Override
    protected void after() {
        disposeEnvironment();
    }
}
