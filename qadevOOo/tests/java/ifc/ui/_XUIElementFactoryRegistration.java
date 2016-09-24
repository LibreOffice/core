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

package ifc.ui;

import com.sun.star.beans.PropertyValue;
import com.sun.star.ui.XUIElementFactory;
import com.sun.star.ui.XUIElementFactoryRegistration;
import lib.MultiMethodTest;

public class _XUIElementFactoryRegistration extends MultiMethodTest {

    public XUIElementFactoryRegistration oObj;

    public void _registerFactory() {
        boolean result = true;
        try {
            oObj.registerFactory("private:resource/menubar/menubar", "MyOwnMenubar", "", "com.sun.star.comp.framework.MenuBarFactory");
        }
        catch(com.sun.star.container.ElementExistException e) {
            result = false;
            e.printStackTrace(log);
        }
        tRes.tested("registerFactory()", result);
    }

    public void _getRegisteredFactories() {
        requiredMethod("registerFactory()");
        PropertyValue[][]props = oObj.getRegisteredFactories();
        if (props == null) {
            log.println("Null was returned as PropertyValue[][]");
            props = new PropertyValue[0][0];
        }
        for(int i=0; i<props.length; i++)
            for(int j=0; j<props[i].length; j++)
                log.println("Factory: " + props[i][j].Name + "    -    " + props[i][j].Value);
        tRes.tested("getRegisteredFactories()", props.length != 0);
    }

    public void _getFactory() {
        requiredMethod("registerFactory()");
        XUIElementFactory xFactory = oObj.getFactory("private:resource/menubar/menubar", "");
        tRes.tested("getFactory()", xFactory != null);
    }

    public void _deregisterFactory() {
        executeMethod("getRegisteredFactory()");
        executeMethod("getFactory()");
        try {
            oObj.deregisterFactory("private:resource/menubar/menubar", "MyOwnMenubar", "");
        }
        catch(com.sun.star.container.NoSuchElementException e) {
            e.printStackTrace(log);
        }
        tRes.tested("deregisterFactory()", true);
    }
}
