/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XUIConfiguration.java,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:34:45 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

package ifc.ui;

import com.sun.star.lang.XServiceInfo;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.ui.XModuleUIConfigurationManagerSupplier;
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