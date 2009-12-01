/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: _XUIElementFactoryRegistration.java,v $
 * $Revision: 1.5 $
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
        boolean result = true;
        try {
            oObj.deregisterFactory("private:resource/menubar/menubar", "MyOwnMenubar", "");
        }
        catch(com.sun.star.container.NoSuchElementException e) {
            result = false;
            e.printStackTrace(log);
        }
        tRes.tested("deregisterFactory()", true);
    }
}
