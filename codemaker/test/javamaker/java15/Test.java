/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Test.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 02:21:16 $
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

package test.codemaker.javamaker.java15;

import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.uno.DeploymentException;
import com.sun.star.uno.XComponentContext;
import complexlib.ComplexTestCase;

public final class Test extends ComplexTestCase {
    public String[] getTestMethodNames() {
        return new String[] {
            "testPlainPolyStruct", "testBooleanPolyStruct", "testStruct",
            "testService" };
    }

    public void testPlainPolyStruct() {
        PolyStruct s = new PolyStruct();
        assure(s.member1 == null);
        assure(s.member2 == 0);
        s = new PolyStruct("ABC", 5);
        assure(s.member1.equals("ABC"));
        assure(s.member2 == 5);
    }

    public void testBooleanPolyStruct() {
        PolyStruct<Boolean,Object> s = new PolyStruct<Boolean,Object>();
        assure(s.member1 == null);
        assure(s.member2 == 0);
        s = new PolyStruct<Boolean,Object>(true, 5);
        assure(s.member1 == true);
        assure(s.member2 == 5);
    }

    public void testStruct() {
        Struct s = new Struct();
        assure(s.member.member1 == null);
        assure(s.member.member2 == 0);
        s = new Struct(
            new PolyStruct<PolyStruct<boolean[], Object>, Integer>(
                new PolyStruct<boolean[], Object>(new boolean[] { true }, 3),
                4));
        assure(s.member.member1.member1.length == 1);
        assure(s.member.member1.member1[0] == true);
        assure(s.member.member1.member2 == 3);
        assure(s.member.member2 == 4);
    }

    public void testService() {
        XComponentContext context = new XComponentContext() {
                public Object getValueByName(String name) {
                    return null;
                }

                public XMultiComponentFactory getServiceManager() {
                    return null;
                }
            };
        try {
            Service.create(context);
            failed();
        } catch (DeploymentException e) {}
        try {
            Service.create(
                context, false, (byte) 1, (short) 2, Integer.valueOf(4));
            failed();
        } catch (DeploymentException e) {}
    }

    private static final class Ifc implements XIfc {
        public void f1(PolyStruct<Integer, Integer> arg) {}

        public void f2(PolyStruct<Object, Object> arg) {}
    }
}
