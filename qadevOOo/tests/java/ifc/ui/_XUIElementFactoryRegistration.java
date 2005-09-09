/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XUIElementFactoryRegistration.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:36:07 $
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

import com.sun.star.beans.PropertyValue;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.ui.XUIElement;
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
