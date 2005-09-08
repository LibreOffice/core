/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XDatabaseParameterBroadcaster.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:45:47 $
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
package ifc.form;

import com.sun.star.beans.Property;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XChild;
import com.sun.star.form.XDatabaseParameterBroadcaster;
import com.sun.star.form.XDatabaseParameterListener;
import com.sun.star.form.XLoadable;
import com.sun.star.sdbc.XColumnLocate;
import com.sun.star.sdbc.XParameters;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.sdbc.XRow;
import com.sun.star.sdbc.XRowSet;
import com.sun.star.uno.UnoRuntime;
import java.io.PrintWriter;
import java.util.Properties;
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
