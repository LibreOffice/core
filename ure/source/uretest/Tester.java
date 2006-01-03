/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Tester.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2006-01-03 12:41:11 $
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

package test.java.tester;

import com.sun.star.uno.XComponentContext;
import test.types.CppTest;
import test.types.JavaTest;
import test.types.TestException;
import test.types.XTest;

public final class Tester {
    public static void test(XComponentContext context) {
        testService(CppTest.create(context), CppTest.class.getName());
        testService(JavaTest.create(context), JavaTest.class.getName());
    }

    private static void testService(XTest test, String name) {
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

    private Tester() {}
}
