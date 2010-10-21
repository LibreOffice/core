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
package complex.tdoc;

import com.sun.star.beans.XPropertyContainer;
import share.LogWriter;

/**
 *
 * @author  sg128468
 */
public class _XPropertyContainer {
    public XPropertyContainer oObj = null;
    public LogWriter log = null;

    public boolean _addProperty() {
        boolean result = true;
        // add illegal property
        try {
            oObj.addProperty("MyIllegalProperty", (short)0, null);
        }
        catch(com.sun.star.beans.PropertyExistException e) {
            e.printStackTrace((java.io.PrintWriter)log);
            result = false;
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace((java.io.PrintWriter)log);
            log.println("'IllegalArgument' Unexpected but correct.");
        }
        catch(com.sun.star.beans.IllegalTypeException e) {
            log.println("'IllegalType' Correctly thrown");
        }
        // add valid property
        try {
            oObj.addProperty("MyLegalProperty", (short)0, "Just a value");
        }
        catch(com.sun.star.beans.PropertyExistException e) {
            e.printStackTrace((java.io.PrintWriter)log);
            result = false;
        }
        catch(com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace((java.io.PrintWriter)log);
            result = false;
        }
        catch(com.sun.star.beans.IllegalTypeException e) {
            e.printStackTrace((java.io.PrintWriter)log);
            result = false;
        }
        return result;
    }

    public boolean _removeProperty() {
        boolean result = true;
        try {
            oObj.removeProperty("MyIllegalProperty");
        }
        catch(com.sun.star.beans.UnknownPropertyException e) {
            log.println("'UnknownProperty' Correctly thrown");
        }
        catch(com.sun.star.beans.NotRemoveableException e) {
            e.printStackTrace((java.io.PrintWriter)log);
            result = false;
        }
        try {
            oObj.removeProperty("MyLegalProperty");
        }
        catch(com.sun.star.beans.UnknownPropertyException e) {
            e.printStackTrace((java.io.PrintWriter)log);
            result = false;
        }
        catch(com.sun.star.beans.NotRemoveableException e) {
            e.printStackTrace((java.io.PrintWriter)log);
            result = false;
        }
        return result;
    }

}
