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

package ifc.util;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameReplace;
import com.sun.star.util.XChangesBatch;
import com.sun.star.util.XChangesListener;
import com.sun.star.util.XChangesNotifier;
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
        boolean bChangesOccurred = false;

        /** A change did occur
         * @param changesEvent The event.
         **/
        public void changesOccurred(com.sun.star.util.ChangesEvent changesEvent) {
            bChangesOccurred = true;
        }

        /** Disposing of the listener
         * @param eventObject The event.
         **/
        public void disposing(com.sun.star.lang.EventObject eventObject) {
            bChangesOccurred = true;
        }

        /**
         * Reset the listener
         */
        public void reset() {
            bChangesOccurred = false;
        }

        /**
         * Has the listener been called?
         * @return True, if the listener has been called.
         */
        public boolean didChangesOccur() {
            return bChangesOccurred;
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
            e.printStackTrace(log);
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
            e.printStackTrace(log);
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
                e.printStackTrace(log);
                return false;
            }
        }
        else if (xNameReplace != null) {
            try {
                xNameReplace.replaceByName(elementName, element);
            }
            catch(com.sun.star.uno.Exception e) {
                e.printStackTrace(log);
                return false;
            }
        }
        return true;
    }

}
