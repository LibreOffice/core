/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XUIConfiguration.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 11:10:17 $
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

package imageManager.interfaces;

import com.sun.star.lang.XServiceInfo;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.ui.XModuleUIConfigurationManagerSupplier;
import com.sun.star.ui.XUIConfiguration;
import com.sun.star.ui.XUIConfigurationListener;
import lib.TestParameters;
import share.LogWriter;


public class _XUIConfiguration {

    LogWriter log = null;
    TestParameters tEnv = null;
    public XUIConfiguration oObj;
    XUIConfigurationListenerImpl xListener = null;

    public static interface XUIConfigurationListenerImpl
                                    extends XUIConfigurationListener {
        public void reset();
        public void fireEvent();
        public boolean actionWasTriggered();
    }


    public _XUIConfiguration(LogWriter log, TestParameters tEnv, XUIConfiguration oObj) {
        this.log = log;
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
