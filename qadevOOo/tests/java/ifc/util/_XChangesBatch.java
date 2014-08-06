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
import com.sun.star.util.ElementChange;
import lib.MultiMethodTest;

import com.sun.star.util.XChangesBatch;
import lib.Status;
import lib.StatusException;

public class _XChangesBatch extends MultiMethodTest {

    public XChangesBatch oObj;
    private Object changeElement = null;
    private Object originalElement = null;
    private String elementName = null;
    private XPropertySet xProp = null;
    private XNameReplace xNameReplace = null;

    /**
     * add a change that can be committed
     */
    @Override
    protected void before() {
        changeElement = tEnv.getObjRelation("XChangesBatch.ChangeElement");
        originalElement = tEnv.getObjRelation("XChangesBatch.OriginalElement");
        elementName = (String)tEnv.getObjRelation("XChangesBatch.PropertyName");

        // to do a change, get an XPropertySet
        xProp = (XPropertySet)tEnv.getObjRelation("XChangesBatch.PropertySet");
        try {
            if (originalElement == null && xProp != null)
                originalElement = xProp.getPropertyValue(elementName);
        }
        catch(com.sun.star.uno.Exception e) {
            throw new StatusException("Could not get property '" + elementName + "'.", e);
        }

        // or get an XNameReplace
        xNameReplace = (XNameReplace)tEnv.getObjRelation("XChangesBatch.NameReplace");
        try {
            if (originalElement == null && xNameReplace != null)
                originalElement = xNameReplace.getByName(elementName);
        }
        catch(com.sun.star.uno.Exception e) {
            throw new StatusException("Could not get element by name '" + elementName + "'.", e);
        }

        if (changeElement == null || originalElement == null || elementName == null || (xProp == null && xNameReplace == null)) {
            log.println(
                (changeElement == null?"Missing property 'XChangesBatch.ChangeElement'\n":"") +
                (originalElement == null?"Missing property 'XChangesBatch.OriginalElement'\n":"") +
                (elementName == null?"Missing property 'XChangesBatch.PropertyName'\n":"") +
                (xProp == null?"Missing property 'XChangesBatch.PropertySet'":"") +
                (xNameReplace == null?"Missing property 'XChangesBatch.NameReplace'":"")
            );
            throw new StatusException("Some needed object relations are missing.", new Exception());
        }
    }

    public void _commitChanges() {
        requiredMethod("getPendingChanges()");
        try {
            log.println("Committing changes.");
            oObj.commitChanges();
        }
        catch(com.sun.star.lang.WrappedTargetException e) {
            tRes.tested("commitChanges()", Status.exception(e));
            return;
        }
        try {
            executeChange(originalElement);
        }
        catch(StatusException e) {
            tRes.tested("hasPendingChanges()", Status.exception(e));
            return;
        }

        try {
            log.println("Commit changes back.");
            oObj.commitChanges();
        }
        catch(com.sun.star.lang.WrappedTargetException e) {
            tRes.tested("commitChanges()", Status.exception(e));
            return;
        }
        tRes.tested("commitChanges()", true);
    }

    public void _getPendingChanges() {
        requiredMethod("hasPendingChanges()");
        ElementChange[]changes = oObj.getPendingChanges();
        if (changes == null) {
            log.println("Returned changes was 'null'");
            log.println("It should have been 1 change.");
            tRes.tested("getPendingChanges()", false);
        } else if (changes.length != 1) {
            int amount = changes.length;
            log.println("Found not the right number of changes: " + amount);
            log.println("It should have been 1 change.");
            for (int i=0; i<amount; i++) {
                System.out.println("Detailed Change " + i + " -> new Element: '" +
                            changes[i].Element.toString() + "'  ReplacedElement: '" +
                            changes[i].ReplacedElement.toString() + "'");
            }
            tRes.tested("getPendingChanges()", false);
        }
        else {
            boolean result = changes[0].ReplacedElement.equals(originalElement);
            result &= changes[0].Element.equals(changeElement);
            tRes.tested("getPendingChanges()", result);
        }
    }

    public void _hasPendingChanges() {
        try {
            executeChange(changeElement);
        }
        catch(StatusException e) {
            tRes.tested("hasPendingChanges()", Status.exception(e));
            return;
        }
        boolean hasPendingChanges = oObj.hasPendingChanges();
        tRes.tested("hasPendingChanges()", hasPendingChanges);
    }

    private void executeChange(Object element) throws StatusException {
        if (xProp != null) {
            try {
                xProp.setPropertyValue(elementName, element);
            }
            catch(com.sun.star.uno.Exception e) {
                throw new StatusException("Could not set property '" + elementName + "'.", e);
            }
        }
        else if (xNameReplace != null) {
            try {
                xNameReplace.replaceByName(elementName, element);
            }
            catch(com.sun.star.uno.Exception e) {
                throw new StatusException("Could not replace '" + elementName + "' by name.", e);
            }
        }
    }
}
