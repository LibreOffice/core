/*************************************************************************
 *
 *  $RCSfile: _XChangesNotifier.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-12-11 11:49:03 $
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

package ifc.util;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameReplace;
import com.sun.star.util.XChangesBatch;
import com.sun.star.util.XChangesListener;
import com.sun.star.util.XChangesNotifier;
import java.io.PrintWriter;
import lib.StatusException;
import lib.MultiMethodTest;

/**
 * Test the XChangesNotifier interface. To produce some changes,
 * XChangesBatch is used.
 * @see com.sun.star.util.XChangesNotifier
 * @see com.sun.star.util.XChangesBatch
 */
public class _XChangesNotifier extends MultiMethodTest {

    public XChangesNotifier oObj = null;
    private XChangesBatch xBatch = null;
    private Object changeElement = null;
    private Object originalElement = null;
    private String elementName = null;
    private XPropertySet xProp = null;
    private XNameReplace xNameReplace = null;
    private _XChangesNotifier.MyChangesListener xListener = null;

    /**
     * Own implementation of the XChangesListener interface
     * @see com.sun.star.util.XChangesListener
     */
    private static class MyChangesListener implements XChangesListener {
        /** Just lo a call of the listener **/
        boolean bChangesOccured = false;

        /** A change did occur
         * @param changesEvent The event.
         **/
        public void changesOccurred(com.sun.star.util.ChangesEvent changesEvent) {
            bChangesOccured = true;
        }

        /** Disposing of the listener
         * @param eventObject The event.
         **/
        public void disposing(com.sun.star.lang.EventObject eventObject) {
            bChangesOccured = true;
        }

        /**
         * Reset the listener
         */
        public void reset() {
            bChangesOccured = false;
        }

        /**
         * Has the listener been called?
         * @return True, if the listener has been called.
         */
        public boolean didChangesOccur() {
            return bChangesOccured;
        }
    }

    /**
     * Before the test: get the 'XChangesNotifier.ChangesBatch' object relation
     * and create the listener.
     */
    protected void before() {
        xBatch = (XChangesBatch)tEnv.getObjRelation("XChangesNotifier.ChangesBatch");
        changeElement = tEnv.getObjRelation("XChangesNotifier.ChangeElement");
        originalElement = tEnv.getObjRelation("XChangesNotifier.OriginalElement");
        elementName = (String)tEnv.getObjRelation("XChangesNotifier.PropertyName");

        xProp = (XPropertySet)tEnv.getObjRelation("XChangesNotifier.PropertySet");
        try {
            if (originalElement == null && xProp != null)
                originalElement = xProp.getPropertyValue(elementName);
        }
        catch(com.sun.star.uno.Exception e) {
            throw new StatusException("Could not get property '" + elementName + "'.", e);
        }

        // or get an XNameReplace
        xNameReplace = (XNameReplace)tEnv.getObjRelation("XChangesNotifier.NameReplace");
        try {
            if (originalElement == null && xNameReplace != null)
                originalElement = xNameReplace.getByName(elementName);
        }
        catch(com.sun.star.uno.Exception e) {
            throw new StatusException("Could not get element by name '" + elementName + "'.", e);
        }

        if (changeElement == null || originalElement == null || elementName == null || (xProp == null && xNameReplace == null) || xBatch == null) {
            log.println(
                changeElement == null?"Missing property 'XChangesNotifier.ChangeElement'\n":"" +
                originalElement == null?"Missing property 'XChangesNotifier.OriginalElement'\n":"" +
                elementName == null?"Missing property 'XChangesNotifier.PropertyName'\n":"" +
                xProp == null?"Missing property 'XChangesNotifier.PropertySet'":"" +
                xNameReplace == null?"Missing property 'XChangesNotifier.NameReplace'":"" +
                xBatch == null?"Missing property 'XChangesNotifier.ChangesBatch'":""
            );
            throw new StatusException("Some needed object relations are missing.", new Exception());
        }

        xListener = new _XChangesNotifier.MyChangesListener();
    }

    /** test addChangesListener **/
    public void _addChangesListener() {
        oObj.addChangesListener(xListener);
        tRes.tested("addChangesListener()", true);
    }

    /** test removeChangesListener **/
    public void _removeChangesListener() {
        requiredMethod("addChangesListener()");
        boolean result = true;
        result &= commitChanges();
        result &= xListener.didChangesOccur();
        if (!result)
            log.println("Listener has not been called.");
        oObj.removeChangesListener(xListener);
        xListener.reset();
        result &= redoChanges();
        boolean result2 = xListener.didChangesOccur();
        if (result2)
            log.println("Removed listener has been called.");

        tRes.tested("removeChangesListener()", result && !result2);
    }

    /**
     * Commit a change, using an implementation of the XChangesBatch interface.
     * @return true, if changing worked.
     */
    private boolean commitChanges() {
        if (!executeChange(changeElement)) return false;
        if (!xBatch.hasPendingChanges()) return false;
        try {
            xBatch.commitChanges();
        }
        catch(com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace((PrintWriter)log);
            return false;
        }
        return true;
    }

    /**
     * Redo the change, using an implementation of the XChangesBatch interface.
     * @return true, if changing worked.
     */
    private boolean redoChanges() {
        if (!executeChange(originalElement)) return false;
        if (!xBatch.hasPendingChanges()) return false;
        try {
            xBatch.commitChanges();
        }
        catch(com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace((PrintWriter)log);
            return false;
        }
        return true;
    }

    /**
     * Execute the change, use XPropertySet or XNameReplace
     * @return False, if changing did throw an exception.
     */
    private boolean executeChange(Object element) throws StatusException {
        if (xProp != null) {
            try {
                xProp.setPropertyValue(elementName, element);
            }
            catch(com.sun.star.uno.Exception e) {
                e.printStackTrace((PrintWriter)log);
                return false;
            }
        }
        else if (xNameReplace != null) {
            try {
                xNameReplace.replaceByName(elementName, element);
            }
            catch(com.sun.star.uno.Exception e) {
                e.printStackTrace((PrintWriter)log);
                return false;
            }
        }
        return true;
    }

}
