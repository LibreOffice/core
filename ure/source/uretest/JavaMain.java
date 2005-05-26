/*************************************************************************
 *
 *  $RCSfile: JavaMain.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: sb $ $Date: 2005-05-26 09:35:49 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2005 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package test.java.javamain;

import com.sun.star.comp.loader.FactoryHelper;
import com.sun.star.lang.XMain;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.registry.XRegistryKey;
import com.sun.star.uno.XComponentContext;
import test.types.CppTest;
import test.types.JavaTest;
import test.types.TestException;
import test.types.XTest;

public final class JavaMain implements XMain {
    public JavaMain(XComponentContext context) {
        this.context = context;
    }

    public int run(String[] arguments) {
        test(CppTest.create(context), CppTest.class.getName());
        test(JavaTest.create(context), JavaTest.class.getName());
        return 0;
    }

    public static boolean __writeRegistryServiceInfo(XRegistryKey key) {
        return
            FactoryHelper.writeRegistryServiceInfo(
                IMPLEMENTATION_NAME, SERVICE_NAME, key);
    }

    public static XSingleServiceFactory __getServiceFactory(
        String name, XMultiServiceFactory factory, XRegistryKey key)
    {
        if (name.equals(IMPLEMENTATION_NAME)) {
            return FactoryHelper.getServiceFactory(
                JavaMain.class, SERVICE_NAME, factory, key);
        } else {
            return null;
        }
    }

    private static final void test(XTest test, String name) {
        boolean ok = false;
        try {
            test.throwException();
        } catch (TestException e) {
            ok = true;
        }
        if (!ok) {
            throw new RuntimeException(name + ".throwException failed");
        }
    }

    private static final String IMPLEMENTATION_NAME
    = "test.java.javamain.Component";
    private static final String SERVICE_NAME = "test.dummy.JavaMain";

    private final XComponentContext context;
}
