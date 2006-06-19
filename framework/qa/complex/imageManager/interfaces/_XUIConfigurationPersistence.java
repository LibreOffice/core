/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XUIConfigurationPersistence.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 11:10:29 $
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

import com.sun.star.embed.XStorage;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.ui.XModuleUIConfigurationManagerSupplier;
import com.sun.star.ui.XUIConfigurationPersistence;
import lib.TestParameters;
import share.LogWriter;


public class _XUIConfigurationPersistence {

    LogWriter log = null;
    TestParameters tEnv = null;
    public XUIConfigurationPersistence oObj;
    private XStorage xStore = null;

    public _XUIConfigurationPersistence(LogWriter log, TestParameters tEnv, XUIConfigurationPersistence oObj) {
        this.log = log;
        this.tEnv = tEnv;
        this.oObj = oObj;
    }

    public void before() {
        xStore = (XStorage)tEnv.get("XUIConfigurationStorage.Storage");
    }

    public boolean _reload() {
        try {
            oObj.reload();
        }
        catch(com.sun.star.uno.Exception e) {
            e.printStackTrace((java.io.PrintWriter)log);
        }
        return true;
    }

    public boolean _store() {
        try {
            oObj.store();
        }
        catch(com.sun.star.uno.Exception e) {
            e.printStackTrace((java.io.PrintWriter)log);
        }
        return true;
    }

    public boolean _storeToStorage() {
        boolean result = true;
        try {
            oObj.storeToStorage(xStore);
        }
        catch(com.sun.star.uno.Exception e) {
            result = false;
            e.printStackTrace((java.io.PrintWriter)log);
        }
        return result;
    }

    public boolean _isModified() {
        return !oObj.isModified();
    }

    public boolean _isReadOnly() {
        return  !oObj.isReadOnly();
    }

}
