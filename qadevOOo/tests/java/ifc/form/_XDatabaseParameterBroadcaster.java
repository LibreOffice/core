/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: _XDatabaseParameterBroadcaster.java,v $
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
    public static interface CheckParameterListener extends XDatabaseParameterListener {
        /**
         * Set a log of the listener, so messages of the listener get printed
         * into the file of the interface
         */
        public void setLog(PrintWriter log);
        /**
         * Return True, when the listener was called correctly.
         */
        public boolean checkListener();
    }

    /**
     * Get the object relation 'ParameterListenerChecker' and
     * set the log inside of the implementation.
     */
    protected void before() {
        listenerChecker = (CheckParameterListener)
                        tEnv.getObjRelation("ParameterListenerChecker");
        listenerChecker.setLog((PrintWriter)log);
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
            XRowSet xRowSet = (XRowSet)UnoRuntime.queryInterface(XRowSet.class, oObj);
            xRowSet.execute();
        }
        catch(com.sun.star.sdbc.SQLException e) {
            log.println("Exception in XDatabaseParameterBroadcaster test.");
            log.println("This does not let the test fail, but should be inquired.");
            e.printStackTrace((PrintWriter)log);
        }
        // was the listener called?
        oObj.removeParameterListener(listenerChecker);
        tRes.tested("removeParameterListener()", listenerChecker.checkListener());
    }

    protected void after() {
        disposeEnvironment();
    }
}
