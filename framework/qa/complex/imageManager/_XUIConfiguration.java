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

package complex.imageManager;


import com.sun.star.ui.XUIConfiguration;
import com.sun.star.ui.XUIConfigurationListener;
import lib.TestParameters;



public class _XUIConfiguration {


    TestParameters tEnv = null;
    public XUIConfiguration oObj;
    XUIConfigurationListenerImpl xListener = null;

    public static interface XUIConfigurationListenerImpl
                                    extends XUIConfigurationListener {
        public void reset();
        public void fireEvent();
        public boolean actionWasTriggered();
    }


    public _XUIConfiguration(TestParameters tEnv, XUIConfiguration oObj) {
        this.tEnv = tEnv;
        this.oObj = oObj;
    }

    public void before() {
        xListener = (XUIConfigurationListenerImpl)tEnv.get(
                        "XUIConfiguration.XUIConfigurationListenerImpl");
        XUIConfigurationListener l;
    }

    public boolean _addConfigurationListener() {
        oObj.addConfigurationListener(xListener);
        xListener.fireEvent();
        return xListener.actionWasTriggered();
    }

    public boolean _removeConfigurationListener() {
        oObj.removeConfigurationListener(xListener);
        xListener.reset();
        xListener.fireEvent();
        return !xListener.actionWasTriggered();
    }

    /**
     * Dispose because the UIConfigurationManager has to be recreated
     */
    public void after() {
//        disposeEnvironment();
    }
}
