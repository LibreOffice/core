/*************************************************************************
 *
 *  $RCSfile: TypeDescription_Test.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: kr $ $Date: 2001-09-11 15:57:18 $
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
package com.sun.star.lib.uno.typedesc;


import java.lang.reflect.Method;
import java.util.Vector;


import com.sun.star.uno.Any;
import com.sun.star.uno.IFieldDescription;
import com.sun.star.uno.IMethodDescription;
import com.sun.star.uno.ITypeDescription;
import com.sun.star.uno.Type;
import com.sun.star.uno.TypeClass;
import com.sun.star.uno.XInterface;


public class TypeDescription_Test {
//      static public TypeDescription getTypeDescription(String typeName) throws ClassNotFoundException {
//      static public ITypeDescription getTypeDescription(Type type) throws ClassNotFoundException {
//      static public TypeDescription getTypeDescription(TypeClass typeClass) {
//      static public TypeDescription getTypeDescription(TypeInfo typeInfo, Class zClass) {
//          TypeDescription.isTypeClassSimple(TypeClass typeClass) {

    static class MethodSignature {
        boolean _oneway;
        boolean _buildIn;
        ITypeDescription _inSig[];
        ITypeDescription _outSig[];
        ITypeDescription _retSig;

        MethodSignature(boolean oneway, boolean buildin, ITypeDescription inSig[], ITypeDescription outSig[], ITypeDescription retSig) {
            _oneway = oneway;
            _buildIn = buildin;
            _inSig = inSig;
            _outSig = outSig;
            _retSig = retSig;
        }

        boolean test(Vector notpassed, int index, IMethodDescription iMethodDescription) {
            boolean passed = true;

            boolean tmp_passed = iMethodDescription.isOneway() == _oneway;
            passed = passed && tmp_passed;
            if(!tmp_passed)
                notpassed.addElement("isOneway");

            int mindex = iMethodDescription.getIndex();
            tmp_passed = mindex == index;
            passed = passed && tmp_passed;
            if(!tmp_passed) {
                notpassed.addElement("getIndex - " + mindex);
//                  throw new RuntimeException("blabla");
            }

            ITypeDescription inSig[] = iMethodDescription.getInSignature();
            tmp_passed = inSig.length == _inSig.length;
            if(tmp_passed)
                for(int i = 0; i < inSig.length; ++ i)
                    tmp_passed = tmp_passed && inSig[i].equals(_inSig[i]);
            passed = passed && tmp_passed;
            if(!tmp_passed)
                notpassed.addElement("getInSignature - ");

            ITypeDescription outSig[] = iMethodDescription.getOutSignature();
            tmp_passed = outSig.length == _outSig.length;
            if(tmp_passed)
                for(int i = 0; i < outSig.length; ++ i)
                    tmp_passed = tmp_passed && (outSig[i] == _outSig[i] || outSig[i].equals(_outSig[i]));
            passed = passed && tmp_passed;
            if(!tmp_passed)
                notpassed.addElement("getOutSignature - ");

            ITypeDescription retSig = iMethodDescription.getReturnSignature();
            tmp_passed = retSig.equals(_retSig);
            passed = passed && tmp_passed;
            if(!tmp_passed)
                notpassed.addElement("getReturnSignature - ");

            Method method = iMethodDescription.getMethod();
            tmp_passed = (method != null) ^ _buildIn;
            passed = passed && tmp_passed;
            if(!tmp_passed)
                notpassed.addElement("getMethod - " + method);

            return passed;
        }
    }


    static class TypeSignature {
        TypeSignature _superType;
        MethodSignature _methodSignatures[];
        String _method_names[];
        int _method_offset;

        TypeSignature _fieldSignatures[];
        String _field_names[];
        int _field_offset;

        TypeSignature(TypeSignature superType, String names[], MethodSignature methodSignatures[], String field_names[], TypeSignature fieldSignatures[]) {
            _superType = superType;
            _method_names = names;
            _methodSignatures = methodSignatures;

            _fieldSignatures = fieldSignatures;
            _field_names = field_names;

            if(_superType != null) {
                _method_offset = _superType._method_offset + _superType._method_names.length;
                _field_offset = _superType._field_offset + _superType._field_names.length;
            }
        }

        public boolean test(Vector notpassed, Object names[], ITypeDescription iTypeDescription) throws Exception {
            boolean passed = true;

            String comment = "TypeSignature.test(" + iTypeDescription + ") ";
            System.err.println("TypeDescription_Test.test_TypeSignature - doing tests...");

            ITypeDescription superType = iTypeDescription.getSuperType();
            boolean tmp_passed = superType == null || (names.length == 6 && _superType.test(notpassed, (Object [])names[5], superType));
            passed = passed && tmp_passed;
            if(!tmp_passed)
                notpassed.addElement(comment + "- getSuperType - " + superType);

            IMethodDescription iMethodDescriptions[] = iTypeDescription.getMethodDescriptions();
            tmp_passed = iMethodDescriptions != null;
            tmp_passed = tmp_passed && _methodSignatures.length == iMethodDescriptions.length;
            if(tmp_passed)
                for(int i = 0; i < _methodSignatures.length; ++ i) {
                    boolean tmp_tmp_passed = _methodSignatures[i].test(notpassed, i + _method_offset, iMethodDescriptions[i]);

                    tmp_passed = tmp_passed && tmp_tmp_passed;
                    if(!tmp_tmp_passed)
                        notpassed.addElement(comment + "- getMethodDescriptions - index:" + i);
                }
            else
                notpassed.addElement(comment + "- getMethodDescriptions - size:" + iMethodDescriptions.length);
            passed = passed && tmp_passed;


            for(int i = 0; i < _method_names.length; ++ i) {
                IMethodDescription iMethodDescription = iTypeDescription.getMethodDescription(i + _method_offset);
                tmp_passed = iMethodDescription != null && _methodSignatures[i].test(notpassed, i + _method_offset, iMethodDescription);
                passed = passed && tmp_passed;
                if(!tmp_passed)
                    notpassed.addElement(comment + "- getMethodDescription(int " + (i + _method_offset) + ") - " + iMethodDescription);
            }

            for(int i = 0; i < _method_names.length; ++ i) {
                IMethodDescription iMethodDescription  = iTypeDescription.getMethodDescription(_method_names[i]);
                tmp_passed = iMethodDescription != null && _methodSignatures[i].test(notpassed, i + _method_offset, iMethodDescription);
                passed = passed && tmp_passed;
                if(!tmp_passed)
                    notpassed.addElement(comment + "- getMethodDescription(String " + _method_names[i] + ")");
            }

            IFieldDescription iFieldDescriptions[] = iTypeDescription.getFieldDescriptions();
            tmp_passed = iFieldDescriptions != null && iFieldDescriptions.length == _fieldSignatures.length;
            if(tmp_passed)
                for(int i = 0; i < _fieldSignatures.length; ++ i) {
                    boolean tmp_tmp_passed = _fieldSignatures[i].test(notpassed, ((Object [][])names[4])[i], iFieldDescriptions[i].getTypeDescription());

                    tmp_passed = tmp_passed && tmp_tmp_passed;
                    if(!tmp_tmp_passed)
                        notpassed.addElement(comment + "- getFieldDescriptions - index:" + i);
                }
            else
                notpassed.addElement(comment + "- getFieldDescriptions - size:" + iFieldDescriptions.length);
            passed = passed && tmp_passed;


            TypeClass typeClass = iTypeDescription.getTypeClass();
            tmp_passed = typeClass == names[3];
            passed = passed && tmp_passed;
            if(!tmp_passed)
                notpassed.addElement(comment + "- getTypeClass");

            ITypeDescription componentType = iTypeDescription.getComponentType();
            tmp_passed = componentType == null;
            passed = passed && tmp_passed;
            if(!tmp_passed)
                notpassed.addElement(comment + "- getComponentType");

            String typeName = iTypeDescription.getTypeName();
            tmp_passed = typeName.equals(names[0]);
            passed = passed && tmp_passed;
            if(!tmp_passed)
                notpassed.addElement(comment + "- getTypeName");

            String arrayTypeName = iTypeDescription.getArrayTypeName();
            tmp_passed = arrayTypeName.equals(names[1]);
            passed = passed && tmp_passed;
            if(!tmp_passed)
                notpassed.addElement(comment + "- getArrayTypeName - " + names[1]);

            Class xzClass = iTypeDescription.getZClass();
            tmp_passed = xzClass == names[2];
                passed = passed && tmp_passed;
            if(!tmp_passed)
                notpassed.addElement(comment + "- getZClass");

            tmp_passed = iTypeDescription.equals(TypeDescription.getTypeDescription((String)names[0]));
            passed = passed && tmp_passed;
            if(!tmp_passed)
                notpassed.addElement(comment + "- equals");


            return passed;
        }
    }

    static ITypeDescription __stringTD = TypeDescription.getTypeDescription(String.class);
    static ITypeDescription __xinterfaceTD = TypeDescription.getTypeDescription(XInterface.class);
    static ITypeDescription __voidTD = TypeDescription.getTypeDescription(Void.class);
    static ITypeDescription __typeTD = TypeDescription.getTypeDescription(Type.class);
    static ITypeDescription __anyTD = TypeDescription.getTypeDescription(Any.class);


    static MethodSignature __sig_sync_buildin_itype__any = new MethodSignature(false, true, new ITypeDescription[]{__typeTD}, new ITypeDescription[1], __anyTD);
    static MethodSignature __sig_async_buildin__void = new MethodSignature(true, true, new ITypeDescription[]{}, new ITypeDescription[0], __voidTD);

    static TypeSignature __itf_xinterface = new TypeSignature(null,
                                                              new String[]{"queryInterface", "acquire", "release"},
                                                              new MethodSignature[]{__sig_sync_buildin_itype__any, __sig_async_buildin__void, __sig_async_buildin__void},
                                                              new String[0],
                                                              new TypeSignature[0]);


    static MethodSignature __sig_sync_addon_istr__xifc = new MethodSignature(false, false, new ITypeDescription[]{__stringTD}, new ITypeDescription[1], __xinterfaceTD);
    static MethodSignature __sig_sync_addon_istr_ixifc__void = new MethodSignature(false, false, new ITypeDescription[]{__stringTD, __xinterfaceTD}, new ITypeDescription[2], __voidTD);
    static MethodSignature __sig_sync_addon_istr__void = new MethodSignature(false, false, new ITypeDescription[]{__stringTD}, new ITypeDescription[1], __voidTD);

    static TypeSignature __itf_xnaming_service = new TypeSignature(__itf_xinterface,
                                                                   new String[]{"getRegisteredObject", "registerObject", "revokeObject"},
                                                                   new MethodSignature[]{__sig_sync_addon_istr__xifc, __sig_sync_addon_istr_ixifc__void, __sig_sync_addon_istr__void},
                                                                   new String[0],
                                                                   new TypeSignature[0]);



    static TypeSignature __emptySig = new TypeSignature(null,
                                                        new String[0],
                                                        new MethodSignature[0],
                                                        new String[0],
                                                        new TypeSignature[0]);


    static TypeSignature __exception = new TypeSignature(null,
                                                         new String[0],
                                                         new MethodSignature[0],
                                                         new String[]{"Message", "Context"},
                                                         new TypeSignature[] {__emptySig, __itf_xinterface});



    public static boolean test(Vector notpassed) throws Exception {
        boolean passed = true;

        boolean tmp_passed = true;

        Object string_cc[] = new Object[]{"string", "[Ljava.lang.String;", java.lang.String.class, TypeClass.STRING};
        Object byte_cc[] = new Object[]{"byte", "[B", byte.class, TypeClass.BYTE};

        Object xinterface_cc[] = new Object[]{"com.sun.star.uno.XInterface", "[Lcom.sun.star.uno.XInterface;", com.sun.star.uno.XInterface.class, TypeClass.INTERFACE};

        tmp_passed = __emptySig.test(notpassed, byte_cc, TypeDescription.getTypeDescription("byte"));
        passed = passed && tmp_passed;

        tmp_passed = __emptySig.test(notpassed, string_cc, TypeDescription.getTypeDescription("string"));
        passed = passed && tmp_passed;


        tmp_passed = __exception.test(notpassed, new Object[]{"com.sun.star.uno.Exception",
                                                              "[Lcom.sun.star.uno.Exception;",
                                                              com.sun.star.uno.Exception.class,
                                                              TypeClass.EXCEPTION,
                                                              new Object[]{string_cc, xinterface_cc}}, TypeDescription.getTypeDescription("com.sun.star.uno.Exception"));
        passed = passed && tmp_passed;

        tmp_passed = __emptySig.test(notpassed, new Object[]{"com.sun.star.uno.TypeClass",
                                                             "[Lcom.sun.star.uno.TypeClass;",
                                                             com.sun.star.uno.TypeClass.class,
                                                             TypeClass.ENUM}, TypeDescription.getTypeDescription("com.sun.star.uno.TypeClass"));
        passed = passed && tmp_passed;

        tmp_passed = __itf_xinterface.test(notpassed, xinterface_cc, TypeDescription.getTypeDescription(XInterface.class));
        passed = passed && tmp_passed;

        tmp_passed = __itf_xnaming_service.test(notpassed, new Object[]{"com.sun.star.uno.XNamingService",
                                                                        "[Lcom.sun.star.uno.XNamingService;",
                                                                        com.sun.star.uno.XNamingService.class,
                                                                        TypeClass.INTERFACE,
                                                                        null,
                                                                        xinterface_cc}, TypeDescription.getTypeDescription(com.sun.star.uno.XNamingService.class));
        passed = passed && tmp_passed;

        return passed;
    }

    static public void main(String args[]) throws Exception {
        Vector notpassed = new Vector();

        boolean passed = test(notpassed);

        System.err.println("tests passed? " + passed);

        for(int i = 0; i < notpassed.size(); ++ i)
            System.err.println("not passed:" + notpassed.elementAt(i));
    }
}
