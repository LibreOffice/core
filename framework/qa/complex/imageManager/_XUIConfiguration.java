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

package complex.imageManager;


import com.sun.star.ui.XUIConfiguration;
import com.sun.star.ui.XUIConfigurationListener;
import lib.TestParameters;



public class _XUIConfiguration {


    private final TestParameters tEnv;
    private final XUIConfiguration oObj;
    private XUIConfigurationListenerImpl xListener = null;

    public interface XUIConfigurationListenerImpl
                                    extends XUIConfigurationListener {
        void reset();
        void fireEvent();
        boolean actionWasTriggered();
    }


    public _XUIConfiguration(TestParameters tEnv, XUIConfiguration oObj) {
        this.tEnv = tEnv;
        this.oObj = oObj;
    }

    public void before() {
        xListener = (XUIConfigurationListenerImpl)tEnv.get(
                        "XUIConfiguration.XUIConfigurationListenerImpl");
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

}
