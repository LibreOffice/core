/*************************************************************************
 *
 *  $RCSfile: _XBindableValue.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2004-04-02 10:27:14 $
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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