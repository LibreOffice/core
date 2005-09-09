/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: LocalServiceTest.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 12:28:28 $
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

package testtools.servicetests;

import com.sun.star.comp.helper.Bootstrap;
import com.sun.star.container.XSet;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;

public final class LocalServiceTest extends TestBase {
    protected TestServiceFactory getTestServiceFactory() throws Exception {
        return new TestServiceFactory() {
                public Object get() throws Exception {
                    XComponentContext context
                        = Bootstrap.createInitialComponentContext(null);
                    XMultiComponentFactory serviceManager
                        = context.getServiceManager();
                    ((XSet) UnoRuntime.queryInterface(
                        XSet.class, serviceManager)).insert(new TestService());
                    return serviceManager.createInstanceWithContext(
                        "testtools.servicetests.TestService2", context);
                }

                public void dispose() throws Exception {}
            };
    }
}
