/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package servicetests;

import com.sun.star.lang.NoSupportException;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XSingleComponentFactory;
/*import com.sun.star.uno.OptionalPropertyException;*/
/*import com.sun.star.uno.VoidPropertyException;*/
import com.sun.star.uno.XComponentContext;

public final class TestService implements XServiceInfo, XSingleComponentFactory
{
    public String getImplementationName() {
        return getClass().getName();
    }

    public boolean supportsService(String serviceName) {
        return serviceName.equals(SERVICE_NAME);
    }

    public String[] getSupportedServiceNames() {
        return new String[] { SERVICE_NAME };
    }

    public Object createInstanceWithContext(XComponentContext context)
        throws com.sun.star.uno.Exception
    {
        return new Service();
    }

    public Object createInstanceWithArgumentsAndContext(
        Object[] arguments, XComponentContext context)
        throws com.sun.star.uno.Exception
    {
        throw new NoSupportException(
            "createInstanceWithArgumentsAndContext", this);
    }

    private static final class Service implements TestService2, XTestService3 {
        public int fn1() {
            return 1;
        }

        public int getProp1() {
            return prop1;
        }

        public void setProp1(int value) {
            prop1 = value;
        }

        public int getProp2() {
            return 2;
        }

        /*public int getProp3Void() throws VoidPropertyException {
            throw new VoidPropertyException("Prop3Void", this);
        }*/

        public int getProp3Long() /*throws VoidPropertyException*/ {
            return 3;
        }

        /*public int getProp4None() throws OptionalPropertyException {
            throw new OptionalPropertyException("Prop4None", this);
        }*/

        public int getProp4Long() /*throws OptionalPropertyException*/ {
            return 4;
        }

        /*public int getProp5None()
            throws OptionalPropertyException, VoidPropertyException
        {
            throw new OptionalPropertyException("Prop4None", this);
        }*/

        /*public int getProp5Void()
            throws OptionalPropertyException, VoidPropertyException
        {
            throw new VoidPropertyException("Prop4None", this);
        }*/

        public int getProp5Long()
            /*throws OptionalPropertyException, VoidPropertyException*/
        {
            return 5;
        }

        public int getProp6() /*throws VoidPropertyException*/ {
            /*if (prop6 == null) {
                throw new VoidPropertyException("Prop6", this);
            } else {*/
                return prop6.intValue();
            /*}*/
        }

        public void setProp6(int value) {
            prop6 = new Integer(value);
        }

        /*public void clearProp6() {
            prop6 = null;
        }*/

        /*public int getProp7None()
            throws OptionalPropertyException, VoidPropertyException
        {
            throw new OptionalPropertyException("Prop7None", this);
        }*/

        /*public void setProp7None(int value) throws OptionalPropertyException {
            throw new OptionalPropertyException("Prop7None", this);
        }*/

        /*public void clearProp7None() throws OptionalPropertyException {
            throw new OptionalPropertyException("Prop7None", this);
        }*/

        public int getProp7()
            /*throws OptionalPropertyException, VoidPropertyException*/
        {
            /*if (prop7 == null) {
                throw new VoidPropertyException("Prop7", this);
            } else {*/
                return prop7.intValue();
            /*}*/
        }

        public void setProp7(int value) /*throws OptionalPropertyException*/ {
            prop7 = new Integer(value);
        }

        /*public void clearProp7() throws OptionalPropertyException {
            prop7 = null;
        }*/

        /*public int getProp8None() throws OptionalPropertyException {
            throw new OptionalPropertyException("Prop8None", this);
        }*/

        /*public void setProp8None(int value) throws OptionalPropertyException {
            throw new OptionalPropertyException("Prop8None", this);
        }*/

        public int getProp8Long() /*throws OptionalPropertyException*/ {
            return prop8;
        }

        public void setProp8Long(int value) /*throws OptionalPropertyException*/
        {
            prop8 = value;
        }

        public int fn2() {
            return 2;
        }

        public int fn3() {
            return 3;
        }

        private int prop1 = 1;
        private Integer prop6 = new Integer(6);
        private Integer prop7 = new Integer(7);
        private int prop8 = 8;
    }

    private static final String SERVICE_NAME
    = "testtools.servicetests.TestService2";
}
