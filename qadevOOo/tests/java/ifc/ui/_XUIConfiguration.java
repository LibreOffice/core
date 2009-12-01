/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: _XUIConfiguration.java,v $
 * $Revision: 1.6 $
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
* Test is <b> NOT </b> multithread compilant. <p>
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
    public static interface XUIConfigurationListenerImpl
                                    extends XUIConfigurationListener {
        public void reset();
        public void fireEvent();
        public boolean actionWasTriggered();
    }


    /**
     * try to get a listener out of the object relation
     * <CODE>XUIConfiguration.XUIConfigurationListenerImpl</CODE>
     */
    public void before() {
        xListener = (XUIConfigurationListenerImpl)tEnv.getObjRelation(
                        "XUIConfiguration.XUIConfigurationListenerImpl");
        XUIConfigurationListener l;
    }

    /**
     * adds a listener an fire an event
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
    public void after() {
        disposeEnvironment();
    }
}
