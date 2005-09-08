/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XCompletedExecution.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:26:14 $
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
