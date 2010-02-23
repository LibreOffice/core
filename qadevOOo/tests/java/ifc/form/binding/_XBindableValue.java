/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
package ifc.form.binding;

import com.sun.star.form.binding.XBindableValue;
import com.sun.star.form.binding.XValueBinding;
import com.sun.star.uno.Type;

import java.util.ArrayList;

import lib.MultiMethodTest;


public class _XBindableValue extends MultiMethodTest {
    public XBindableValue oObj;
    protected XValueBinding xValueBinding = null;

    public void _getValueBinding() {
        requiredMethod("setValueBinding");

        boolean res = true;
        xValueBinding = oObj.getValueBinding();
        res &= checkValueBinding(xValueBinding);
        tRes.tested("getValueBinding()", res);
    }

    public void _setValueBinding() {
        String rightOne = "";

        try {
            oObj.setValueBinding(new MyValueBinding());
            rightOne = (String) oObj.getValueBinding().getValue(null);
        } catch (com.sun.star.form.binding.IncompatibleTypesException e) {
            e.printStackTrace();
        }

        boolean res = rightOne.equals("MyValueBinding");

        if (!res) {
            log.println("Excepted: MyValueBinding");
            log.println("getting: " + rightOne);
        }

        tRes.tested("setValueBinding()", res);
    }

    protected boolean checkValueBinding(XValueBinding xValueBinding) {
        boolean res = true;
        Type[] types = xValueBinding.getSupportedValueTypes();
        log.println("Checking: ");

        for (int i = 0; i < types.length; i++) {
            log.println("\t" + types[i].getTypeName());

            boolean localRes = xValueBinding.supportsType(types[i]);

            if (!localRes) {
                log.println("\t\tsupportsType returns false -- FAILED");
            } else {
                log.println("\t\tis supported -- OK");
            }

            res &= localRes;
        }

        return res;
    }

    class MyValueBinding implements XValueBinding {
        private Type[] TypeArray;
        private ArrayList types = new ArrayList();

        public com.sun.star.uno.Type[] getSupportedValueTypes() {
            return TypeArray;
        }

        public Object getValue(com.sun.star.uno.Type type)
            throws com.sun.star.form.binding.IncompatibleTypesException {
            return "MyValueBinding";
        }

        public void setValue(Object obj)
            throws com.sun.star.form.binding.IncompatibleTypesException,
                   com.sun.star.lang.NoSupportException {
        }

        public boolean supportsType(com.sun.star.uno.Type type) {
            types.add(type);
            TypeArray = new Type[types.size()];

            for (int i = 0; i < types.size(); i++) {
                TypeArray[i] = (Type) types.toArray()[i];
            }

            return true;
        }
    }
}