/*************************************************************************
 *
 *  $RCSfile: MethodDescription.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kr $ $Date: 2001-04-04 13:41:41 $
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


import com.sun.star.lib.uno.typeinfo.MethodTypeInfo;
import com.sun.star.lib.uno.typeinfo.ParameterTypeInfo;
import com.sun.star.lib.uno.typeinfo.TypeInfo;


public class MethodDescription {
    static private final TypeDescription __xInterface = TypeDescription.getTypeDescription(com.sun.star.uno.XInterface.class);
    static private TypeDescription __xInterfaceArray;
    static private TypeDescription __xInterfaceArrayArray;

    static {
        try {
            __xInterfaceArray = TypeDescription.getTypeDescription(Class.forName("[Lcom.sun.star.uno.XInterface;"));
            __xInterfaceArrayArray = TypeDescription.getTypeDescription(Class.forName("[[Lcom.sun.star.uno.XInterface;"));
        }
        catch(ClassNotFoundException classNotFoundException) {
            System.err.println("##### MethodDescription - ERORR:" + classNotFoundException);
        }
    }

    protected int _index;
    protected int _flags;
    protected String _name;
    protected int  _offset;
    protected TypeDescription _in_sig[];
    protected TypeDescription _out_sig[];
    protected TypeDescription _return_sig;
    protected Method _method;

    protected ParameterTypeInfo _parameterTypeInfos[];

    public MethodDescription(String name, int index, int flags) {
        _name = name;
        _flags = flags;

        _index = index;
    }

    public MethodDescription(MethodTypeInfo methodTypeInfo) {
        this(methodTypeInfo.getName(), methodTypeInfo.getIndex(), methodTypeInfo.getFlags());
    }

    public String getName() {
        return _name;
    }

    public int getIndex() {
        return _index + _offset;
    }

    public boolean isOneway() {
        return (_flags & TypeInfo.ONEWAY) != 0;
    }

    public boolean isConst() {
        return (_flags & TypeInfo.CONST) != 0;
    }

    public boolean isUnsigned()
    {
        return (_flags & TypeInfo.UNSIGNED) != 0;
    }

    public boolean isAny()
    {
        return (_flags & TypeInfo.ANY) != 0;
    }

    public boolean isInterface()
    {
        return (_flags & TypeInfo.INTERFACE) != 0;
    }


    void init(Method method, ParameterTypeInfo parameterTypeInfos[], int offset) {
        _method = method;
        _offset = offset;

        init(_method.getParameterTypes(), parameterTypeInfos, _method.getReturnType());
    }

    void init(Class signature[], ParameterTypeInfo parameterTypeInfos[], Class resultClass) {
        _parameterTypeInfos = parameterTypeInfos;

        // init _in_sig
        _in_sig = new TypeDescription[signature.length];
        for(int i = 0; i < _in_sig.length; ++i)
            _in_sig[i] = __takeCareOf_XInterface_and_java_lang_Object_as_in(_parameterTypeInfos[i], signature[i]);

        // init _out_sig
        _out_sig = new TypeDescription[signature.length];
        for(int i = 0; i < _out_sig.length; ++i)
            _out_sig[i] = __takeCareOf_XInterface_and_java_lang_Object_as_out(_parameterTypeInfos[i], signature[i]);

        // init _return_sig
        if(resultClass == Object.class && isInterface())
            if(resultClass.isArray())
                _return_sig = __xInterfaceArray;
            else
                _return_sig = __xInterface;
        else
            _return_sig = TypeDescription.getTypeDescription(resultClass);
    }

    static private TypeDescription __takeCareOf_XInterface_and_java_lang_Object_as_in(ParameterTypeInfo typeInfo, Class parameterClass) {
        TypeDescription typeDescription = TypeDescription.getTypeDescription(parameterClass);

        if(typeInfo != null) { // if there is special typeinfo, use it
            if(typeInfo.isIN()) {
                // blackdown 118 bug workaround
//                  __getFields(parameterClass);

                // see if the typeinfo says, that the parameter is an interface
                // and the parameter is not assignable to xinterface (mapping of xinterface to java.lang.Object)
                // set the parameter to class of xinterface, cause it must be assignable
                int arrayNesting = 0;
                while(parameterClass.isArray()) {
                    ++ arrayNesting;

                    parameterClass = parameterClass.getComponentType();
                }

                if(typeInfo.isInterface() && !parameterClass.isInterface())
                    for(int i = 0; i < arrayNesting; ++ i) {
                        try {
                            typeDescription = TypeDescription.getTypeDescription("[]" + typeDescription.getTypeName());
                        }
                        catch(ClassNotFoundException classNotFoundException) { // this could never happen, but anyway...
                            throw new RuntimeException(classNotFoundException.toString());
                        }
                    }
            }
            else // this is an out parameter, and must not be passed
                typeDescription = null;
        }

        return typeDescription;
    }

    static private TypeDescription __takeCareOf_XInterface_and_java_lang_Object_as_out(ParameterTypeInfo typeInfo, Class parameterClass) {
        TypeDescription typeDescription = TypeDescription.getTypeDescription(parameterClass);

        if(typeInfo != null && typeInfo.isOUT()) { // if there is special typeinfo and it is an outparameter, use it
            // blackdown 118 bug workaround
//              __getFields(parameterClass);

            // see if the typeinfo says, that the parameter is an interface
            // and the parameter is not assignable to xinterface (mapping of xinterface to java.lang.Object)
            // set the parameter to class of xinterface, cause it must be assignable
            int arrayNesting = 0;
            while(parameterClass.isArray()) {
                ++ arrayNesting;

                parameterClass = parameterClass.getComponentType();
            }

            if(typeInfo.isInterface() && !parameterClass.isInterface()) {
                typeDescription = __xInterface;

                for(int i = 0; i < arrayNesting; ++ i) {
                    try {
                        typeDescription = TypeDescription.getTypeDescription("[]" + typeDescription.getTypeName());
                    }
                    catch(ClassNotFoundException classNotFoundException) { // this could never happen, but anyway...
                        throw new RuntimeException(classNotFoundException.toString());
                    }
                }
            }
        }
        else // this is not an out parameter
            typeDescription = null;

        return typeDescription;
    }

    public TypeDescription[] getInSignature() {
        return _in_sig;
    }

    public TypeDescription[] getOutSignature() {
        return _out_sig;
    }

    public TypeDescription getReturnSig() {
        return _return_sig;
    }

    public Method getMethod() {
        return _method;
    }
}


