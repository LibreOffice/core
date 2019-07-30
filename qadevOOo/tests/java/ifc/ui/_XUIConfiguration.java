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

import com.sun.star.ui.XUIConfiguration;
import com.sun.star.ui.XUIConfigurationListener;
import lib.MultiMethodTest;

/**
* Testing <code>com.sun.star.ui.XUIConfiguration</code>
* interface methods :
* <ul>
*  <li><code> addConfigurationListener()</code></li>
*  <li><code> removeConfigurationListener()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* After test completion object environment has to be recreated.
* @see com.sun.star.ui.XUIConfiguration
*/

public class _XUIConfiguration extends MultiMethodTest {

    public XUIConfiguration oObj;
    XUIConfigurationListenerImpl xListener = null;

    /**
     * Interface for the Listener of the object relation
     * <CODE>XUIConfiguration.XUIConfigurationListenerImpl</CODE>
     * @see com.sun.star.ui.XUIConfigurationListener
     */
    public interface XUIConfigurationListenerImpl
                                    extends XUIConfigurationListener {
        void reset();
        void fireEvent();
        boolean actionWasTriggered();
    }


    /**
     * try to get a listener out of the object relation
     * <CODE>XUIConfiguration.XUIConfigurationListenerImpl</CODE>
     */
    @Override
    public void before() {
        xListener = (XUIConfigurationListenerImpl)tEnv.getObjRelation(
                        "XUIConfiguration.XUIConfigurationListenerImpl");
    }

    /**
     * adds a listener and fires an event
     * Has <B>OK</B> status if listener was called
     */
    public void _addConfigurationListener() {
        oObj.addConfigurationListener(xListener);
        xListener.fireEvent();
        tRes.tested("addConfigurationListener()", xListener.actionWasTriggered());
    }

    /**
     * removes the listener and calls an event.
     * Has <B>OK</B> status if listener is not called.
     */
    public void _removeConfigurationListener() {
        requiredMethod("addConfigurationListener()");
        oObj.removeConfigurationListener(xListener);
        xListener.reset();
        xListener.fireEvent();
        tRes.tested("removeConfigurationListener()", !xListener.actionWasTriggered());
    }

    /**
     * Dispose because the UIConfigurationManager has to be recreated
     */
    @Override
    public void after() {
        disposeEnvironment();
    }
}
