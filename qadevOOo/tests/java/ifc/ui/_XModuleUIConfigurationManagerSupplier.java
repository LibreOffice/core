/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XModuleUIConfigurationManagerSupplier.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:34:32 $
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
import lib.MultiMethodTest;

public class _XModuleUIConfigurationManagerSupplier extends MultiMethodTest {

    public XModuleUIConfigurationManagerSupplier oObj;

    public void _getUIConfigurationManager() {
        String configManagerName = null;
        String implementationName = null;
        try {
            configManagerName = (String)tEnv.getObjRelation("XModuleUIConfigurationManagerSupplier.ConfigurationManager");
            implementationName = (String)tEnv.getObjRelation("XModuleUIConfigurationManagerSupplier.ConfigManagerImplementationName");

            // get a config manager for the StartModule
            Object o = oObj.getUIConfigurationManager(configManagerName);
            XServiceInfo xServiceInfo = (XServiceInfo)UnoRuntime.queryInterface(XServiceInfo.class, o);
            String impName = xServiceInfo.getImplementationName();
            boolean result = impName.equals(implementationName);
            if (!result) {
                log.println("Returned implementation was '" + impName +
                    "' but should have been '" + implementationName + "'");
            }
            tRes.tested("getUIConfigurationManager()", result);
            return;
        }
        catch(com.sun.star.container.NoSuchElementException e) {
            log.println("Could not get a configuration manager called '" + configManagerName + "'");
            e.printStackTrace(log);
        }
        tRes.tested("getUIConfigurationManager()", false);
    }
}