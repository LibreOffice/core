/*************************************************************************
 *
 *  $RCSfile: MethodDescription.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kr $ $Date: 2001-05-08 09:34:17 $
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


import com.sun.star.uno.IMethodDescription;
import com.sun.star.uno.ITypeDescription;


import com.sun.star.lib.uno.typeinfo.MethodTypeInfo;
import com.sun.star.lib.uno.typeinfo.ParameterTypeInfo;
import com.sun.star.lib.uno.typeinfo.TypeInfo;


public class MethodDescription extends MethodTypeInfo implements IMethodDescription {
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

    protected int  _offset;
    protected TypeDescription _in_sig[];
    protected TypeDescription _out_sig[];
    protected TypeDescription _return_sig;
    protected Method _method;

    protected ParameterTypeInfo _parameterTypeInfos[];

      public MethodDescription(String name, int index, int flags) {
        super(name, index, flags);
    }

    public MethodDescription(MethodTypeInfo methodTypeInfo) {
        this(methodTypeInfo.getName(), methodTypeInfo.getIndex(), methodTypeInfo.getFlags());
    }

    public int getIndex() {
        return super.getIndex() + _offset;
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
            if(_parameterTypeInfos[i] == null || _parameterTypeInfos[i].isIN())
                _in_sig[i] = TypeDescription.getTypeDescription(_parameterTypeInfos[i], signature[i]);

        // init _out_sig
        _out_sig = new TypeDescription[signature.length];
        for(int i = 0; i < _out_sig.length; ++i)
            if(_parameterTypeInfos[i] != null && _parameterTypeInfos[i].isOUT())
                _out_sig[i] = TypeDescription.getTypeDescription(_parameterTypeInfos[i], signature[i]);

        // init _return_sig
        _return_sig = TypeDescription.getTypeDescription(this, resultClass);
    }

    public ITypeDescription[] getInSignature() {
        return _in_sig;
    }

    public ITypeDescription[] getOutSignature() {
        return _out_sig;
    }

    public ITypeDescription getReturnSignature() {
        return _return_sig;
    }

    public Method getMethod() {
        return _method;
    }
}


