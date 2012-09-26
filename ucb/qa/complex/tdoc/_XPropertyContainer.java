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
package complex.tdoc;

import com.sun.star.beans.XPropertyContainer;
import share.LogWriter;

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
