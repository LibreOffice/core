/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XComponentContext.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:39:05 $
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
package ifc.uno;

import lib.MultiMethodTest;
import util.ValueComparer;

import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;


public class _XComponentContext extends MultiMethodTest {
    public XComponentContext oObj;
    protected XMultiComponentFactory byValue = null;
    protected XMultiComponentFactory directly = null;

    public void _getServiceManager() {
        log.println("getting manager");

        directly = oObj.getServiceManager();
        String[] names = directly.getAvailableServiceNames();
        boolean res = true;

        for (int i = 0; i < names.length; i++) {
            try {
                if (names[i].equals("com.sun.star.i18n.ConversionDictionary_ko")) continue;
                if (names[i].equals("com.sun.star.i18n.TextConversion_ko")) continue;
                log.println("try to instanciate found servicename " +
                            names[i]);
                directly.createInstanceWithContext(names[i], oObj);
                log.println("worked .... ok");
                res &= true;
            } catch (com.sun.star.uno.Exception e) {
                log.println("Exception occured " + e.getMessage());
                res &= false;
            }
        }

        tRes.tested("getServiceManager()", res);
    }

    public void _getValueByName() {
        requiredMethod("getServiceManager()");

        Object value = oObj.getValueByName(
                               "/singletons/com.sun.star.lang.theServiceManager");
        byValue = (XMultiComponentFactory) UnoRuntime.queryInterface(
                          XMultiComponentFactory.class, value);

        String[] vNames = byValue.getAvailableServiceNames();
        String[] dNames = directly.getAvailableServiceNames();

        boolean res = ValueComparer.equalValue(byValue, directly);
        tRes.tested("getValueByName()", res);
    }
}