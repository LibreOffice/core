/*************************************************************************
 *
 *  $RCSfile: _XChangesBatch.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-12-11 11:48:49 $
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

import com.sun.star.beans.Property;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XProperty;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XHierarchicalName;
import com.sun.star.container.XHierarchicalNameAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNameReplace;
import com.sun.star.document.XTypeDetection;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.ElementChange;
import lib.MultiMethodTest;

import com.sun.star.util.XChangesBatch;
import java.io.PrintWriter;
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
                changeElement == null?"Missing property 'XChangesBatch.ChangeElement'\n":"" +
                originalElement == null?"Missing property 'XChangesBatch.OriginalElement'\n":"" +
                elementName == null?"Missing property 'XChangesBatch.PropertyName'\n":"" +
                xProp == null?"Missing property 'XChangesBatch.PropertySet'":"" +
                xNameReplace == null?"Missing property 'XChangesBatch.NameReplace'":""
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
