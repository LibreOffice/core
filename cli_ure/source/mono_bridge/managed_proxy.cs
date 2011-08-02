/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

namespace com.sun.star.bridges.mono_uno /* FIXME use some uno.foo namespace ? */
{

using System;
using System.Reflection;
using System.Runtime;

using uno.Typelib;
using uno.rtl;

public unsafe class ManagedProxy
{
    Bridge bridge;
    object managedI;
    TypeDescription *unoType;
    UString* unoOid;
    string oid;
    Type type;
    IntPtr nativeProxy;

    enum MethodKind {METHOD = 0, SET, GET};

    /** The array contains MethodInfos  of the cli object. Each one reflects an
        implemented interface method of the interface for which this proxy was
        created. The MethodInfos are from the object's method and not from the
        interface type. That is, they can be used to invoke the methods. The
        order of the MethodInfo objects corresponds to the order of the
        interface methods (see member m_type). Position 0 contains the
        MethodInfo of the first method of the interface which represents the
        root of the inheritance chain. The last MethodInfo represents the last
        method of the furthest derived interface.

        The array is completely initialized in the constructor of this object.

        When the uno_DispatchMethod is called for this proxy then it receives
        a typelib_TypeDescription of the member which is either an attribute
        (setter or getter) or method. After determining the position of the
        method within the UNO interface one can use the position to obtain the
        MethodInfo of the corresponding cli method. To obtain the index for the
        m_arMethodInfos array the function position has to be decreased by 3.
        This is becaus, the cli interface does not contain the XInterface
        methods.
    */
    MethodInfo[] methodInfos;

    /** This array is similar to m_arMethodInfos but it contains the MethodInfo
        objects of the interface (not the object). When a call is made from uno
        to cli then the uno method name is compared to the cli method name. The
        cli method name can be obtained from the MethodInfo object in this
        array. The name of the actual implemented method may not be the same as
        the interface method.
    */
    MethodInfo[] interfaceMethodInfos;

    /** Maps the position of the method in the UNO interface to the position of
        the corresponding MethodInfo in m_arMethodInfos. The Uno position must
        not include the XInterface methods. For example,
        pos 0 = XInterface::queryInterface
        pos 1 = XInterface::acquire
        pos 2 = XInterface::release

        That is the real Uno position has to be deducted by 3. Then
        arUnoPosToCliPos[pos] contains the index for m_arMethodInfos.

     */
    int[] unoPosToCliPos;

    /** Count of inherited interfaces of the cli interface.
     */
    int inheritedInterfacesCount = 0;
    /** Contains the number of methods of each interface.
     */
    int[] interfaceMethodCounts;

    public unsafe ManagedProxy(Bridge bridge, object managedI,
                               TypeDescription* TD, UString* oid)
    {
        this.bridge = bridge;
        this.managedI = managedI;
        this.unoType = TD;
        TypeDescription.Acquire(this.unoType);
        this.unoOid = oid;
        UString.Acquire(this.unoOid);
        this.oid = UString.UStringToString(oid);

        if (TD != null && TD->bComplete == 0)
            TypeDescription.Complete(&TD);

        this.type = Bridge.MapUnoType(this.unoType);
        this.nativeProxy = IntPtr.Zero;
        makeMethodInfos();
    }

    ~ManagedProxy()
    {
        UString.Release(this.unoOid);
        TypeDescription.Release(this.unoType);
    }

    /** Prepares an array (m_arMethoInfos) containing MethodInfo object of the
        interface and all inherited interfaces. At index null is the first
        method of the base interface and at the last position is the last method
        of the furthest derived interface.
        If a UNO call is received then one can determine the position of the
        method (or getter or setter for an attribute) from the passed type
        information. The position minus 3 (there is no XInterface in the cli
        mapping) corresponds to the index of the cli interface method in the
        array.
    */
    void makeMethodInfos()
    {
        if (!type.IsInterface)
            return;

        MethodInfo[] thisMethods = type.GetMethods();
        // get the inherited interfaces
        Type[] inheritedIfaces = type.GetInterfaces();
        inheritedInterfacesCount = inheritedIfaces.Length;

        // array containing the number of methods for the interface
        // and its inherited interfaces
        interfaceMethodCounts = new int[inheritedInterfacesCount + 1];

        // determine the number of all interface methods, including
        // the inherited interfaces
        int methodCount = thisMethods.Length;
        foreach (Type iface in inheritedIfaces)
            methodCount += iface.GetMethods().Length;

        // array containing MethodInfos of the managed object
        methodInfos = new MethodInfo[methodCount];

        // array containing MethodInfos of the interface
        interfaceMethodInfos = new MethodInfo[methodCount];

        // array containing the mapping of UNO interface pos to pos in
        // methodInfos
        unoPosToCliPos = new int[methodCount];

        for (int i = 0; i < methodCount; ++i)
            unoPosToCliPos[i] = -1;

        // fill methodInfos with the mappings
        // !!! InterfaceMapping.TargetMethods should be MethodInfo*[] according
        // to documentation
        // but it is Type*[] instead. Bug in the framework?
        // FIXME ^ what does mono do?
        Type objType = managedI.GetType();
        try
        {
            int index = 0;
            // now get the methods from the inherited interface
            // inheritedIfaces[0] is the direct base interface
            // inheritedIfaces[n] is the furthest inherited interface
            // Start with the base interface
            for (int i = inheritedIfaces.Length - 1; i >= 0; --i)
            {
                InterfaceMapping mapInherited =
                    objType.GetInterfaceMap(inheritedIfaces[i]);

                interfaceMethodCounts[i] = mapInherited.TargetMethods.Length;
                for (int j = 0; j < interfaceMethodCounts[i]; ++j, ++index)
                {
                    methodInfos[index] = mapInherited.TargetMethods[j] as MethodInfo;
                    interfaceMethodInfos[index] =
                        mapInherited.InterfaceMethods[j] as MethodInfo;
                }
            }

            // At last come the methods of the furthest derived interface
            InterfaceMapping map = objType.GetInterfaceMap(type);
            interfaceMethodCounts[inheritedInterfacesCount] =
                map.TargetMethods.Length;
            for (int j = 0;
                 j < interfaceMethodCounts[inheritedInterfacesCount]; ++j, ++index)
            {
                methodInfos[index] = map.TargetMethods[j] as MethodInfo;
                interfaceMethodInfos[index] =
                    map.InterfaceMethods[j] as MethodInfo;
            }
        }
        catch (InvalidCastException)
        {
            // FIXME do something (can this happen, is "as" not the
            // proper translation for "__try_cast" ?
        }
    }

    /**Obtains a MethodInfo which can be used to invoke the cli object.
       Internally it maps nUnoFunctionPos to an index that is used to get the
       corresponding MethodInfo object from m_arMethoInfos. The mapping table
       is dynamically initialized. If the cli interface has no base interface
       or exactly one then the mapping table is initialized in one go at the
       first call. In all ensuing calls the MethodInfo object is immediately
       retrieved through the mapping table.

       If the interface has more then one interface in its inheritance chain,
       that is Type.GetInterfaces return more then one Type, then the mapping
       table is partially initiallized. On the first call the mappings for the
       methods of the belonging interface are created.

       The implementation assumes that the order of interface methods as
       provided by InterfaceMapping.InterfaceMethods corresponds to the order
       of methods in the interface declaration.

       @param nUnoFunctionPos
       Position of the method in the uno interface.
     */
    unsafe MethodInfo getMethodInfo(int unoFunctionPos, UString* unoMethodName, MethodKind methodKind)
    {
        MethodInfo result = null;

        // deduct 3 for XInterface methods
        unoFunctionPos -= 3;
        lock (unoPosToCliPos)
        {
            int cliPos = unoPosToCliPos[unoFunctionPos];
            if (cliPos != -1)
                return methodInfos[cliPos];

            // create the method function name
            string methodName = UString.UStringToString(unoMethodName);
            switch (methodKind)
            {
            case MethodKind.METHOD:
                break;
            case MethodKind.SET:
                methodName = "set_" + methodName;
                break;
            case MethodKind.GET:
                methodName = "get_" + methodName;
                break;
            default:
                // FIXME assert not reached
                break;
            }

            // Find the cli interface method that corresponds to the Uno method
            int indexCliMethod = -1;
            // If the cli interfaces and their methods are in the same order
            // as they were declared (inheritance chain and within the interface)
            // then unoFunctionPos should lead to the correct method. However,
            // the documentation does not say that this ordering is given.
            if (methodName == interfaceMethodInfos[unoFunctionPos].Name)
                indexCliMethod = unoFunctionPos;
            else
            {
                int methodCount = interfaceMethodInfos.Length;
                for (int i = 0; i < methodCount; ++i)
                {
                    if (interfaceMethodInfos[i].Name == methodName)
                    {
                        indexCliMethod = i;
                        break;
                    }
                }
            }

            if (indexCliMethod == -1 )
            {
                // FIXME throw some exception
                return null;
            }
            unoPosToCliPos[unoFunctionPos] = indexCliMethod;
            result = methodInfos[indexCliMethod];
        }

        return result;
    }

    void Acquire()
    {
        uno.Binary.Interface.Acquire(nativeProxy);
    }

    void Release()
    {
        uno.Binary.Interface.Release(nativeProxy);
    }

    unsafe void Dispatch(TypeDescription* memberTD, void* unoRet, void** unoArgs,
                         uno.Binary.Any** unoExc)
    {
        switch (memberTD->eTypeClass)
        {
        case TypeClass.INTERFACE_ATTRIBUTE:
        {
            int memberPos = ((InterfaceMemberTypeDescription*)memberTD)->nPosition;
            InterfaceTypeDescription* ifaceTD = (InterfaceTypeDescription*)unoType;
            int functionPos = ifaceTD->pMapMemberIndexToFunctionIndex[memberPos];

            if (unoRet != null) // is getter method
            {
                MethodInfo info = getMethodInfo(
                    functionPos,
                    ((InterfaceMemberTypeDescription*)memberTD)->pMemberName,
                    MethodKind.GET);
                bridge.CallManaged(
                    managedI, type, info,
                    ((InterfaceAttributeTypeDescription*)memberTD)->pAttributeTypeRef,
                    null, 0, // no params
                    unoRet, null, unoExc);
            }
            else // is setter method
            {
                MethodInfo info = getMethodInfo(
                    // set follows get method
                    functionPos + 1,
                    ((InterfaceMemberTypeDescription*)memberTD)->pMemberName,
                    MethodKind.SET);
                MethodParameter param;
                param.pTypeRef = ((InterfaceAttributeTypeDescription*)memberTD)->pAttributeTypeRef;
                param.bIn = 1;
                param.bOut = 0;

                bridge.CallManaged(
                    managedI, type, info,
                    null /* indicated void return */, &param, 1,
                    null, unoArgs, unoExc);
            }
            break;
        }
        case TypeClass.INTERFACE_METHOD:
        {
            int memberPos = ((InterfaceMemberTypeDescription*)memberTD)->nPosition;
            InterfaceTypeDescription* ifaceTD = (InterfaceTypeDescription*)unoType;
            int functionPos = ifaceTD->pMapMemberIndexToFunctionIndex[memberPos];

            switch (functionPos)
            {
            case 0: // queryInterface()
            {
                TypeDescription* requestedTD = null;
                // FIXME leak
         TypeDescriptionReference * argTD = *(TypeDescriptionReference **) unoArgs[0];
         if (argTD != null)
                    TypeDescriptionReference.GetDescription(&requestedTD, argTD);
                if (requestedTD == null || requestedTD->eTypeClass != TypeClass.INTERFACE)
         {
                   uno.Binary.Any.Construct((uno.Binary.Any*)unoRet, null, null, null);
            *unoExc = null;
            break;
                }

                IntPtr unoInterface = IntPtr.Zero;

                bridge.GetInterfaceFromUnoEnvironment(ref unoInterface, unoOid,
                                                      (InterfaceTypeDescription*)requestedTD);

                if (unoInterface == IntPtr.Zero)
                {
                    Type requestedType = Bridge.MapUnoType(requestedTD);
                    if (requestedType.IsInstanceOfType(managedI))
                    {
                        IntPtr unoI = bridge.MapManagedToUno(managedI, requestedTD);
                        uno.Binary.Any.Construct(
                            (uno.Binary.Any*)unoRet, &unoI, requestedTD, null);
                        uno.Binary.Interface.Release(unoI);
                    }
                    else // object does not support requested interface
                    {
                        uno.Binary.Any.Construct((uno.Binary.Any*)unoRet, null, null, null);
                    }
                    // no exception occurred
                    *unoExc = null;
                }
                else
                {
                    uno.Binary.Any.Construct((uno.Binary.Any*)unoRet, &unoInterface,
                                             requestedTD, null);
                    *unoExc = null;
                }
                break;
            }
            case 1: // acquire this proxy()
                Acquire();
                *unoExc = null;
                break;
            case 2: // release this proxy()
                Release();
                *unoExc = null;
                break;
            default: // arbitrary method call
            {
                InterfaceMethodTypeDescription* methodTD =
                    (InterfaceMethodTypeDescription*)memberTD;
                UString* methodName = ((InterfaceMemberTypeDescription*)memberTD)->pMemberName;

                MethodInfo info = getMethodInfo(functionPos, methodName, MethodKind.METHOD);

                bridge.CallManaged(
                    managedI, type, info,
                    methodTD->pReturnTypeRef, methodTD->pParams,
                    methodTD->nParams,
                    unoRet, unoArgs, unoExc);
                break;
            }
            }

            break;
        }
        default: // Cannot happen
        {
            break;// FIXME Throw an error
        }
        }
    }

    public IntPtr NativeProxy
    {
        get { return nativeProxy; }
        set { nativeProxy = value; }
    }
}

}
