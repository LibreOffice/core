/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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