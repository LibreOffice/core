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
using System.Runtime.InteropServices;
using System.Runtime.Remoting;
using System.Text;
using uno.Binary;
using uno.rtl;
using uno.Typelib;

public unsafe class Bridge
{
    static IntPtr unoEnvironment;
    static cli_uno.Cli_environment managedEnvironment;

    Bridge(IntPtr unoEnv)
    {
        unoEnvironment = unoEnv;
        managedEnvironment = new cli_uno.Cli_environment();
    }

    public unsafe IntPtr MapManagedToUno(object managedData, TypeDescription *td)
    {
        IntPtr result = new IntPtr(null);

        // get oid from managed environment
        string oid = cli_uno.Cli_environment.getObjectIdentifier(managedData);

        UString* unoOid = null;
        UString.NewFromString(&unoOid, oid);
        uno.Binary.Environment.GetRegisteredInterface(unoEnvironment,
                                                      ref result,
                                                      unoOid,
                                                      (InterfaceTypeDescription *)td);
        if (result == IntPtr.Zero)
            lock (typeof(cli_uno.Cli_environment))
            {
                uno.Binary.Environment.GetRegisteredInterface(unoEnvironment,
                                                              ref result,
                                                              unoOid,
                                                              (InterfaceTypeDescription *)td);
                if (result == IntPtr.Zero)
                    result = CreateManagedProxy(managedData, td, unoOid);
            }

        UString.Release(unoOid);

        return result;
    }

    public unsafe object MapUnoToManaged(IntPtr unoInterface, InterfaceTypeDescription *iTD)
    {
        object result = null;

        UString* oidPtr = null;
        uno.Binary.Environment.GetObjectIdentifier(unoEnvironment, &oidPtr, unoInterface);

        // See if the interface was already mapped
        Type ifaceType = MapUnoType((TypeDescription *)iTD);
        string oid = UString.UStringToString(oidPtr);
        // the string is owned by unoEnvironment
        oidPtr = null;

        lock (managedEnvironment)
        {
            result = managedEnvironment.getRegisteredInterface(oid, ifaceType);
            if (result != null)
            {
                // There is already an registered object. It can either be a proxy
                // for the UNO object or a real cli object. In the first case we
                // tell the proxy that it shall also represent the current UNO
                // interface. If it already does that, then it does nothing
                if (RemotingServices.IsTransparentProxy(result))
                {
                    UnoInterfaceProxy p = (UnoInterfaceProxy)RemotingServices.GetRealProxy(result);
                    p.AddUnoInterface(unoInterface, iTD);
                }
            }
            else
            {
                result = UnoInterfaceProxy.Create(this, unoInterface, iTD, oid);
            }
        }

        return result;
    }

    // FIXME convert cli types to expected types, e.g a long to a short where the uno type
    // is a sal_Int16. This could be necessary if a scripting language (typeless) is used
    // @param assign the uno_data has to be destructed (in/out args)
    unsafe void MapToUno(void *unoData, object managedData,
                         // FIXME it's a TypeDescriptionReference
                         TypeDescription *type, bool assign)
    {
        // FIXME acquire the TypeDescription?
        // FIXME assert that all the type equivalences in the comments hold
        switch (type->eTypeClass)
        {
        case TypeClass.VOID:
            break;
        case TypeClass.CHAR:
            *(ushort *)unoData = (char)managedData; // sal_Unicode = ushort
            break;
        case TypeClass.BOOLEAN:
            *(byte *)unoData = (bool)managedData ? (byte)1 : (byte)0; // sal_Bool = byte
            break;
        case TypeClass.BYTE:
            *(byte *)unoData = (byte)managedData; // sal_Int8 = byte
            break;
        case TypeClass.SHORT:
            *(short *)unoData = (short)managedData; // sal_Int16 = short
            break;
        case TypeClass.UNSIGNED_SHORT:
            *(ushort *)unoData = (ushort)managedData; // sal_uInt16 = ushort
            break;
        case TypeClass.LONG:
            *(int *)unoData = (int)managedData; // sal_Int32 = int
            break;
        case TypeClass.UNSIGNED_LONG:
            *(uint *)unoData = (uint)managedData; // sal_uInt32 = uint
            break;
        case TypeClass.HYPER:
            *(long *)unoData = (long)managedData; // sal_Int64 = long
            break;
        case TypeClass.UNSIGNED_HYPER:
            *(ulong *)unoData = (ulong)managedData; // sal_uInt64 = ulong
            break;
        case TypeClass.FLOAT:
            *(float *)unoData = (float)managedData; // C++ float = C# float
            break;
        case TypeClass.DOUBLE:
            *(double *)unoData = (double)managedData; // C++ double = C# double
            break;
        case TypeClass.STRING:
        {
            if (assign && *(UString **)unoData != null)
                UString.Release(*(UString **)unoData);

            *(UString **)unoData = null;
            if (managedData == null)
            {
                UString.New((UString **)unoData);
            }
            else
            {
                string s = (string)managedData;
                UString.NewFromString((UString **)unoData, s);
            }
        }
        break;
        case TypeClass.TYPE:
            if (assign)
                TypeDescriptionReference.Release(*(TypeDescriptionReference **)unoData);

            *(TypeDescriptionReference **)unoData = MapManagedType((Type)managedData);
            break;
        case TypeClass.ANY:
        {
            uno.Binary.Any *binAny = (uno.Binary.Any *)unoData;

            if (assign)
                uno.Binary.Any.Destroy(binAny, null);

            if (managedData == null)
            {
                uno.Binary.Any.Construct(binAny, null, null, null);
                break;
            }

            uno.Any any = (uno.Any)managedData;
            TypeDescription *valueTD = (TypeDescription *)MapManagedType(any.Type);

            // if there's enough room in void *pReserved, store the value in the pointer
            binAny->pData = &binAny->pReserved; // this triggers a bug in mcs < 1.1.4
            switch (valueTD->eTypeClass)
            {
            case TypeClass.VOID:
                break;
            case TypeClass.CHAR:
                *(ushort *)binAny->pData = (char)any.Value;
                break;
            case TypeClass.BOOLEAN:
                *(byte *)binAny->pData = (bool)any.Value ? (byte)1 : (byte)0;
                break;
            case TypeClass.BYTE:
                *(byte *)binAny->pData = (byte)any.Value;
                break;
            case TypeClass.SHORT:
                *(short *)binAny->pData = (short)any.Value;
                break;
            case TypeClass.UNSIGNED_SHORT:
                *(ushort *)binAny->pData = (ushort)any.Value;
                break;
            case TypeClass.LONG:
                *(int *)binAny->pData = (int)any.Value;
                break;
            case TypeClass.UNSIGNED_LONG:
                *(uint *)binAny->pData = (uint)any.Value;
                break;
            case TypeClass.HYPER:
                if (sizeof(long) > sizeof(void *))
                    binAny->pData = uno.rtl.Mem.Allocate(sizeof(long));

                *(long *)binAny->pData = (long)any.Value;
                break;
            case TypeClass.UNSIGNED_HYPER:
                if (sizeof(ulong) > sizeof(void *))
                    binAny->pData = uno.rtl.Mem.Allocate(sizeof(ulong));

                *(ulong *)binAny->pData = (ulong)any.Value;
                break;
            case TypeClass.FLOAT:
                if (sizeof(float) > sizeof(void *)) // FIXME can this happen?
                    binAny->pData = uno.rtl.Mem.Allocate(sizeof(float));

                *(float *)binAny->pData = (float)any.Value;
                break;
            case TypeClass.DOUBLE:
                if (sizeof(double) > sizeof(void *))
                    binAny->pData = uno.rtl.Mem.Allocate(sizeof(double));

                *(double *)binAny->pData = (double)any.Value;
                break;
            case TypeClass.STRING:
                // string anies are used so often, that we handle them
                // separately, to be a little faster than with an
                // extra MapToUno call

                // the Any was already destroyed, can't contain a
                // valid string that we could leak here.
                *(UString **)binAny->pData = null;

                if (managedData == null)
                {
                    UString.New((UString **)binAny->pData);
                }
                else
                {
                    string s = (string)any.Value;
                    UString.NewFromString((UString **)binAny->pData, s);
                }
                // the string is owned by the Any
                break;
            case TypeClass.ENUM:
                *(int *) binAny->pData = System.Convert.ToInt32(any.Value);
                break;
            case TypeClass.TYPE:
            case TypeClass.SEQUENCE:
            case TypeClass.INTERFACE:
                binAny->pReserved = null;
                MapToUno(binAny->pData, any.Value, valueTD, false /* no assign */);
                break;
            case TypeClass.STRUCT:
            case TypeClass.EXCEPTION:
                binAny->pData = uno.rtl.Mem.Allocate(valueTD->nSize);
                MapToUno(binAny->pData, any.Value, valueTD, false /* no assign */);
                break;
            default:
                // FIXME
                throw new Exception();
            }
            binAny->pType = (TypeDescriptionReference *)valueTD;
            TypeDescriptionReference.Acquire(binAny->pType);
        }
        break;
        case TypeClass.ENUM:
            *(int *)unoData = System.Convert.ToInt32(managedData);
            break;
        case TypeClass.STRUCT:
        case TypeClass.EXCEPTION:
        {
            TypeDescription *td = null; // FIXME leak
            TypeDescriptionReference.GetDescription(&td, (TypeDescriptionReference *)type);
            CompoundTypeDescription *compTD = (CompoundTypeDescription *)td;
            StructTypeDescription *structTD = null;

            if (type->eTypeClass == TypeClass.STRUCT)
                structTD = (StructTypeDescription *)type;

            if (((TypeDescription *)compTD)->bComplete == 0)
                TypeDescription.Complete((TypeDescription **)&compTD);

            int members = compTD->nMembers;
            Type managedType = null;
            if (managedData != null)
                managedType = managedData.GetType();

            if (compTD->pBaseTypeDescription != null)
                MapToUno(unoData, managedData,
                         (TypeDescription *)((TypeDescription *)compTD->pBaseTypeDescription)->pWeakRef,
                         assign);

            TypeDescriptionReference *memberType = null;
            for (int i = 0; i < members; ++i)
            {
                memberType = compTD->ppTypeRefs[i];

                object val = null;
                if (managedData != null)
                {
                    string fieldName = UString.UStringToString(compTD->ppMemberNames[i]);
                    FieldInfo fieldInfo = managedType.GetField(fieldName);
                    // special case for Exception.Message property
                    // The com.sun.star.uno.Exception.Message field is mapped to the
                    // System.Exception property. Type.GetField("Message") returns null
                    if (fieldInfo == null &&
                        UString.UStringToString(type->pTypeName) == "com.sun.star.uno.Exception")
                    {
                        // get ExceptionMessage property
                        if (fieldName == "Message")
                        {
                            PropertyInfo propInfo = managedType.GetProperty(fieldName);
                            val = propInfo.GetValue(managedData, null);
                        } // FIXME  else throw exception
                    }
                    else if (fieldInfo != null)
                    {
                        val = fieldInfo.GetValue(managedData);
                    } // FIXME else throw exception
                }

                void *p = (byte *)unoData + compTD->pMemberOffsets[i];
                // When using polymorphic structs then the parameterized members can be null.
                // Then we set a default value.
                bool useDefault = ((structTD != null &&
                                    structTD->pParameterizedTypes != null &&
                                    structTD->pParameterizedTypes[i] == 1 &&
                                    val == null) ||
                                   managedData == null);
                switch (memberType->eTypeClass)
                {
                case TypeClass.CHAR:
                    if (useDefault)
                        *(ushort *)p = 0;
                    else
                        *(ushort *)p = (char)val;
                    break;
                case TypeClass.BOOLEAN:
                    if (useDefault)
                        *(byte *)p = (byte)0;
                    else
                        *(byte *)p = (bool)val ? (byte)1 : (byte)0;
                    break;
                case TypeClass.BYTE:
                    if (useDefault)
                        *(byte *)p = (byte)0;
                    else
                        *(byte *)p = (byte)val;
                    break;
                case TypeClass.SHORT:
                    if (useDefault)
                        *(short *)p = (short)0;
                    else
                        *(short *)p = (short)val;
                    break;
                case TypeClass.UNSIGNED_SHORT:
                    if (useDefault)
                        *(ushort *)p = (ushort)0;
                    else
                        *(ushort *)p = (ushort)val;
                    break;
                case TypeClass.LONG:
                    if (useDefault)
                        *(int *)p = 0;
                    else
                        *(int *)p = (int)val;
                    break;
                case TypeClass.UNSIGNED_LONG:
                    if (useDefault)
                        *(uint *)p = (uint)0;
                    else
                        *(uint *)p = (uint)val;
                    break;
                case TypeClass.HYPER:
                    if (useDefault)
                        *(long *)p = (long)0;
                    else
                        *(long *)p = (long)val;
                    break;
                case TypeClass.UNSIGNED_HYPER:
                    if (useDefault)
                        *(ulong *)p = (ulong)0;
                    else
                        *(ulong *)p = (ulong)val;
                    break;
                case TypeClass.FLOAT:
                    if (useDefault)
                        *(float *)p = 0.0F;
                    else
                        *(float *)p = (float)val;
                    break;
                case TypeClass.DOUBLE:
                    if (useDefault)
                        *(double *)p = 0.0;
                    else
                        *(double *)p = (double)val;
                    break;
                default:
                    // FIXME enum should be converted here
                    MapToUno(p, val, (TypeDescription *)memberType, assign);
                    break;
                }
            }
            // FIXME exception handling
        }
        break;
        case TypeClass.SEQUENCE:
        {
            TypeDescription *td = null; // FIXME
            TypeDescriptionReference.GetDescription(&td, (TypeDescriptionReference *)type);
            TypeDescriptionReference *elementType =
                ((IndirectTypeDescription *)td)->pType;

            SequencePtr seq = new SequencePtr();

            if (managedData != null)
            {
                Array array = (Array)managedData;
                int length = array.GetLength(0);

                switch (elementType->eTypeClass)
                {
                case TypeClass.CHAR:
                    seq = SequencePtr.Allocate(length, sizeof(ushort));
                    Marshal.Copy((char [])managedData, 0, seq.elementsPtr, length);
                    break;
/*                case TypeClass.BOOLEAN:
                    // FIXME bool vs. byte ???
                    seq = SequencePtr.Allocate(length, sizeof(byte));
                    Marshal.Copy((byte [])managedData, 0, seq.elementsPtr, length);
                    break;*/
                case TypeClass.BYTE:
                    seq = SequencePtr.Allocate(length, sizeof(byte));
                    Marshal.Copy((byte [])managedData, 0, seq.elementsPtr, length);
                    break;
                case TypeClass.SHORT:
                    seq = SequencePtr.Allocate(length, sizeof(short));
                    Marshal.Copy((short [])managedData, 0, seq.elementsPtr, length);
                    break;
/*                case TypeClass.UNSIGNED_SHORT:
                    seq = SequencePtr.Allocate(length, sizeof(ushort));
                    Marshal.Copy((short [])managedData, 0, seq.elementsPtr, length);
                    break; */
                case TypeClass.LONG:
                    seq = SequencePtr.Allocate(length, sizeof(int));
                    Marshal.Copy((int [])managedData, 0, seq.elementsPtr, length);
                    break;
/*                case TypeClass.UNSIGNED_LONG:
                    seq = SequencePtr.Allocate(length, sizeof(uint));
                    Marshal.Copy((int [])managedData, 0, seq.elementsPtr, length);
                    break; */
                case TypeClass.HYPER:
                    seq = SequencePtr.Allocate(length, sizeof(long));
                    Marshal.Copy((long [])managedData, 0, seq.elementsPtr, length);
                    break;
/*                case TypeClass.UNSIGNED_HYPER:
                    seq = SequencePtr.Allocate(length, sizeof(ulong));
                    Marshal.Copy((long [])managedData, 0, seq.elementsPtr, length);
                    break; */
                case TypeClass.FLOAT:
                    seq = SequencePtr.Allocate(length, sizeof(float));
                    Marshal.Copy((float [])managedData, 0, seq.elementsPtr, length);
                    break;
                case TypeClass.DOUBLE:
                    seq = SequencePtr.Allocate(length, sizeof(double));
                    Marshal.Copy((double [])managedData, 0, seq.elementsPtr, length);
                    break;
                case TypeClass.STRING:
                {
                    seq = SequencePtr.Allocate(length, sizeof(void *));
                    string[] stringArray = (string [])managedData;
                    for (int i = 0; i < length; ++i)
                    {
                        UString** pStr = ((UString** )seq.elementsPtr) + i;
                        *pStr = null;
                        UString.NewFromString(pStr, stringArray[i]);
                        // string ownership goes to callee
                    }
                }
                break;
                case TypeClass.ENUM:
                    seq = SequencePtr.Allocate(length, sizeof(int));
                    for (int i = 0; i < length; ++i)
                        ((int *)seq.elementsPtr)[i] =
                            Convert.ToInt32(array.GetValue(i));
                    break;
                case TypeClass.TYPE:
                case TypeClass.ANY:
                case TypeClass.STRUCT:
                case TypeClass.EXCEPTION:
                case TypeClass.SEQUENCE:
                case TypeClass.INTERFACE:
        // FIXME: surely we can do better for this lot [!] - sign problems ...
                case TypeClass.BOOLEAN:
                case TypeClass.UNSIGNED_SHORT:
         case TypeClass.UNSIGNED_LONG:
                case TypeClass.UNSIGNED_HYPER:
                {
                    seq = SequencePtr.Allocate(
                        length, ((TypeDescription *)elementType)->nSize);

                    for (int i = 0; i < length; ++i)
                    {
                        void *p =
                            (byte *)seq.elementsPtr +
                            i * ((TypeDescription *)elementType)->nSize;
                        object elementData = ((Array)managedData).GetValue(i);
                        MapToUno(p, elementData,
                                 (TypeDescription *)((TypeDescription *)elementType)->pWeakRef,
                                 false /* no assign */);
                    }
                    // FIXME exception handling
                }
                break;
                default:
                    break; // FIXME throw some exception
                }
            }
            else
            {
                seq = SequencePtr.Allocate(0, sizeof(int));
            }
            *(SequencePtr *)unoData = seq;
        }
        break;
        case TypeClass.INTERFACE:
        {
            if (assign && *(void **)unoData != null)
                uno.Binary.Interface.Release(new IntPtr(*(void **)unoData));

            if (managedData == null)
                *(void **)unoData = null;
            else
            {
                TypeDescription *td = null; // FIXME leak
                TypeDescriptionReference.GetDescription(&td, (TypeDescriptionReference *)type);
                *(void **)unoData = MapManagedToUno(managedData, td).ToPointer();
            }
            break;
        }
        default:
            // FIXME throw some exception
            break;
        }
    }

    unsafe void MapToManaged(ref object managedData, void *unoData,
                             TypeDescriptionReference *type, Type info, bool dontCreateObj)
    {
        switch (type->eTypeClass)
        {
        case TypeClass.CHAR:
            managedData = (char)*(ushort *)unoData;
            break;
        case TypeClass.BOOLEAN:
            managedData = (*(byte *)unoData != 0);
            break;
        case TypeClass.BYTE:
            managedData = *(byte *)unoData;
            break;
        case TypeClass.SHORT:
            managedData = *(short *)unoData;
            break;
        case TypeClass.UNSIGNED_SHORT:
            managedData = *(ushort *)unoData;
            break;
        case TypeClass.LONG:
            managedData = *(int *)unoData;
            break;
        case TypeClass.UNSIGNED_LONG:
            managedData = *(uint *)unoData;
            break;
        case TypeClass.HYPER:
            managedData = *(long *)unoData;
            break;
        case TypeClass.UNSIGNED_HYPER:
            managedData = *(ulong *)unoData;
            break;
        case TypeClass.FLOAT:
            managedData = *(float *)unoData;
            break;
        case TypeClass.DOUBLE:
            managedData = *(double *)unoData;
            break;
        case TypeClass.STRING:
            managedData = UString.UStringToString(*(UString **)unoData);
            break;
        case TypeClass.TYPE:
            managedData = MapUnoType(*(TypeDescriptionReference **)unoData);
            break;
        case TypeClass.ANY:
        {
            uno.Binary.Any *binAny = (uno.Binary.Any *)unoData;
            if (binAny->pType->eTypeClass != TypeClass.VOID)
            {
                object value = null;
                MapToManaged(ref value, binAny->pData, binAny->pType, null, false);
                managedData = new uno.Any(MapUnoType(binAny->pType), value);
            }
            else
            {
                managedData = uno.Any.VOID;
            }
            break;
        }
        case TypeClass.ENUM:
            if (info != null)
                managedData = Enum.ToObject(
                    info.GetElementType(), *(int *)unoData);
            else
                managedData = Enum.ToObject(
                    MapUnoType(type), *(int *)unoData);
            break;
        case TypeClass.STRUCT:
        case TypeClass.EXCEPTION:
        {
            TypeDescription *td = null; // FIXME leak
            TypeDescriptionReference.GetDescription(&td, (TypeDescriptionReference *)type);
            CompoundTypeDescription *compTD = (CompoundTypeDescription *)td;

            if (((TypeDescription *)compTD)->bComplete == 0)
                TypeDescription.Complete((TypeDescription **)&compTD);

            // create the type
            Type managedType = LoadCliType(td->pTypeName);

            // detect if we recursivly convert inherited
            // structures. If this point is reached because of a
            // recursive call during converting a struct then we must
            // not create a new object rather we use the one in
            // cli_data argument.
            object managedObject;
            if (dontCreateObj)
                managedObject = managedData;
            else
            {
                // Special handling for Exception conversion. We must
                // call constructor System::Exception to pass the
                // message string
                if (typeof(unoidl.com.sun.star.uno.Exception).IsAssignableFrom(managedType))
                {
                    // We need to get the Message field. Therefore we
                    // must obtain the offset from the
                    // typedescription. The base interface of all
                    // exceptions is com::sun::star::uno::Exception
                    // which contains the message
                    CompoundTypeDescription *pCTD = compTD;
                    while (pCTD->pBaseTypeDescription != null)
                        pCTD = pCTD->pBaseTypeDescription;

                    int pos = -1;
                    for (int i = 0; i < pCTD->nMembers; ++i)
                    {
                        if (UString.UStringToString(pCTD->ppMemberNames[i]) == "Message")
                        {
                            pos = i;
                            break;
                        }
                    }

                    int offset = pCTD->pMemberOffsets[pos];
                    // With the offset within the exception we can get
                    // the message string
                    string message = UString.UStringToString(
                        (*(UString **)((byte *)unoData + offset)));
                    // We need to find a constructor for the exception
                    // that takes the message string.  We assume that
                    // the first argument is the message string
                    ConstructorInfo[] ctorInfos = managedType.GetConstructors();
                    ConstructorInfo ctorInfo = null;
                    // Constructor must at least have 2 params for the base
                    // unoidl.com.sun.star.uno.Exception (String, Object);
                    int numArgs = -1;
                    foreach (ConstructorInfo ci in ctorInfos)
                    {
                        numArgs = ci.GetParameters().Length;
                        if (numArgs < 2)
                            continue;
                        ctorInfo = ci;
                        break;
                    }

                    // Prepare parameters for constructor
                    object[] args = new object[numArgs];
                    // only initialize the first argument with the
                    // message. All unoidl.<Foo Exception>s are
                    // autogenerated, we know that this is safe.
                    args[0] = message;
                    managedObject = ctorInfo.Invoke(args);
                }
                else
                    managedObject = Activator.CreateInstance(managedType);
            }

            TypeDescriptionReference **memberTypeRefs = compTD->ppTypeRefs;
            int *memberOffsets = compTD->pMemberOffsets;

            if (compTD->pBaseTypeDescription != null)
            {
                // convert inherited struct
                // cliObj is passed inout (args in_param, out_param are true), hence the passed
                // cliObj is used by the callee instead of a newly created struct
                MapToManaged(ref managedObject,  unoData,
                             ((TypeDescription *)compTD->pBaseTypeDescription)->pWeakRef,
                             null,
                             true);
            }
            for (int i = compTD->nMembers - 1; i >= 0; --i)
            {
                TypeDescriptionReference *memberType = memberTypeRefs[i];
                string memberName = UString.UStringToString(compTD->ppMemberNames[i]);
                FieldInfo fieldInfo = managedType.GetField(memberName);
                // special case for Exception.Message. The field has already been
                // set while constructing cli object
                if (fieldInfo == null &&
                    UString.UStringToString(type->pTypeName) == "com.sun.star.uno.Exception")
                    continue;

                void *p = (byte *)unoData + memberOffsets[i];
                switch (memberType->eTypeClass)
                {
                case TypeClass.CHAR:
                    fieldInfo.SetValue(managedObject, (char)*(short *)p);
                    break;
                case TypeClass.BOOLEAN:
                    fieldInfo.SetValue(managedObject, (*(byte *)p) != 0);
                    break;
                case TypeClass.BYTE:
                    fieldInfo.SetValue(managedObject, *(byte *)p);
                    break;
                case TypeClass.SHORT:
                    fieldInfo.SetValue(managedObject, *(short *)p);
                    break;
                case TypeClass.UNSIGNED_SHORT:
                    fieldInfo.SetValue(managedObject, *(ushort *)p);
                    break;
                case TypeClass.LONG:
                    fieldInfo.SetValue(managedObject, *(int *)p);
                    break;
                case TypeClass.UNSIGNED_LONG:
                    fieldInfo.SetValue(managedObject, *(uint *)p);
                    break;
                case TypeClass.HYPER:
                    fieldInfo.SetValue(managedObject, *(long *)p);
                    break;
                case TypeClass.UNSIGNED_HYPER:
                    fieldInfo.SetValue(managedObject, *(ulong *)p);
                    break;
                case TypeClass.FLOAT:
                    fieldInfo.SetValue(managedObject, *(float *)p);
                    break;
                case TypeClass.DOUBLE:
                    fieldInfo.SetValue(managedObject, *(double *)p);
                    break;
                default:
                {
                    object managedValue = null;
                    MapToManaged(ref managedValue, p, memberType, null, false);
                    fieldInfo.SetValue(managedObject, managedValue);
                    break;
                }
                }
            }
            managedData = managedObject;
            break;
        }
        case TypeClass.SEQUENCE:
        {
            SequencePtr seq = *(SequencePtr *)unoData;
            int length = seq.nElements;

            TypeDescription *td = null; // FIXME leak
            TypeDescriptionReference.GetDescription(&td, (TypeDescriptionReference *)type);
            TypeDescriptionReference *elementType = ((IndirectTypeDescription *)td)->pType;

            switch (elementType->eTypeClass)
            {
            case TypeClass.CHAR:
            {
                char[] array = new char[length];
                Marshal.Copy(seq.elementsPtr, array, 0, length);
                managedData = array;
                break;
            }
            case TypeClass.BOOLEAN:
            {
                bool[] array = new bool[length];
                byte *source = (byte *)seq.elementsPtr;
                if (length > 0) fixed (bool *arrayPtr = array)
                {
                    bool *dest = arrayPtr;
                    for (int i = 0; i < length; ++i)
                        *dest++ = (*source++ != 0);
                }
                managedData = array;
                break;
            }
            case TypeClass.BYTE:
            {
                byte[] array = new byte[length];
                byte *source = (byte *)seq.elementsPtr;
                if (length > 0) fixed (byte *arrayPtr = array)
                {
                    byte *dest = arrayPtr;
                    for (int i = 0; i < length; ++i)
                        *dest++ = *source++;
                }
                managedData = array;
                break;
            }
            case TypeClass.SHORT:
            {
                short[] array = new short[length];
                Marshal.Copy(seq.elementsPtr, array, 0, length);
                managedData = array;
                break;
            }
            case TypeClass.UNSIGNED_SHORT:
            {
                ushort[] array = new ushort[length];
                ushort *source = (ushort *)seq.elementsPtr;
                if (length > 0) fixed (ushort *arrayPtr = array)
                {
                    ushort *dest = arrayPtr;
                    for (int i = 0; i < length; ++i)
                        *dest++ = *source++;
                }
                managedData = array;
                break;
            }
            case TypeClass.LONG:
            {
                int[] array = new int[length];
                Marshal.Copy(seq.elementsPtr, array, 0, length);
                managedData = array;
                break;
            }
            case TypeClass.UNSIGNED_LONG:
            {
                uint[] array = new uint[length];
                uint *source = (uint *)seq.elementsPtr;
                if (length > 0) fixed (uint *arrayPtr = array)
                {
                    uint *dest = arrayPtr;
                    for (int i = 0; i < length; ++i)
                        *dest++ = *source++;
                }
                managedData = array;
                break;
            }
            case TypeClass.HYPER:
            {
                long[] array = new long[length];
                Marshal.Copy(seq.elementsPtr, array, 0, length);
                managedData = array;
                break;
            }
            case TypeClass.UNSIGNED_HYPER:
            {
                ulong[] array = new ulong[length];
                ulong *source = (ulong *)seq.elementsPtr;
                if (length > 0) fixed (ulong *arrayPtr = array)
                {
                    ulong *dest = arrayPtr;
                    for (int i = 0; i < length; ++i)
                        *dest++ = *source++;
                }
                managedData = array;
                break;
            }
            case TypeClass.FLOAT:
            {
                float[] array = new float[length];
                Marshal.Copy(seq.elementsPtr, array, 0, length);
                managedData = array;
                break;
            }
            case TypeClass.DOUBLE:
            {
                double[] array = new double[length];
                Marshal.Copy(seq.elementsPtr, array, 0, length);
                managedData = array;
                break;
            }
            case TypeClass.STRING:
            {
                string[] array = new string[length];
                for (int i = 0; i < length; ++i)
                {
                    UString *us  = ((UString **)seq.elementsPtr)[i];
                    array[i] = UString.UStringToString(us);
                }
                managedData = array;
                break;
            }
            case TypeClass.TYPE:
            {
                Type[] array = new Type[length];
                for (int i = 0; i < length; ++i)
                    array[i] = MapUnoType(((TypeDescriptionReference **)
                                           seq.elementsPtr)[i]);
                managedData = array;
                break;
            }
            case TypeClass.ANY:
            {
                uno.Any[] array = new uno.Any[length];
                uno.Binary.Any *binAny = (uno.Binary.Any *)seq.elementsPtr;
                for (int i = 0; i < length; ++i)
                {
                    object any = new uno.Any();
                    MapToManaged(ref any, (void **)(binAny + i),
                                 (TypeDescriptionReference *)elementType,
                                 null, false);
                    array[i] = (uno.Any)any;
                }
                managedData = array;
                break;
            }
            case TypeClass.ENUM:
            {
                Type enumType = null;
                if (info != null)
                {
                    enumType = info.GetElementType();
                    // enumType is EnumType[], get EnumType
                    enumType = enumType.GetElementType();
                }
                else
                    enumType = MapUnoType(elementType);

                Array array = Array.CreateInstance(enumType, length);
                for (int i = 0; i < length; ++i)
                {
                    array.SetValue(Enum.ToObject(enumType,
                                                 ((int *)seq.elementsPtr)[i]),
                                   i);
                }
                managedData = array;
                break;
            }
            case TypeClass.STRUCT:
            case TypeClass.EXCEPTION:
            {
                Array array = Array.CreateInstance(MapUnoType(elementType), length);
                if (length > 0)
                {
                    // FIXME check this
                    byte *p = (byte *)seq.elementsPtr;
                    int size = ((TypeDescription *)elementType)->nSize;
                    for (int i = 0; i < length; ++i)
                    {
                        object val = null;
                        MapToManaged(ref val, p + (size * i), elementType, null, false);
                        array.SetValue(val, i);
                    }
                }
                managedData = array;
                break;
            }
            // FIXME verify (says cli_data.cxx)
            case TypeClass.SEQUENCE:
            {
                Array array = Array.CreateInstance(
                    MapUnoType(elementType), length);
                if (length > 0)
                {
                    SequencePtr *elements = (SequencePtr *)seq.elementsPtr;
                    for (int i = 0; i < length; ++i)
                    {
                        object val = null;
                        MapToManaged(ref val, elements + i, elementType, null, false);
                        array.SetValue(val, i);
                    }
                }
                managedData = array;
                break;
            }
            case TypeClass.INTERFACE:
            {
                Type ifaceType = MapUnoType(elementType);
                Array array = Array.CreateInstance(ifaceType, length);

                byte *p = (byte *)seq.elementsPtr;
                int size = ((TypeDescription *)elementType)->nSize;
                for (int i = 0; i < length; ++i)
                {
                    object val = null;
                    MapToManaged(ref val, p + (size * i), elementType, null, false);
                    array.SetValue(val, i);
                }
                managedData = array;
                break;
            }
            default:
            {
                // FIXME throw some exception
                break;
            }
            }
            break;
        }
        case TypeClass.INTERFACE:
        {
            IntPtr unoI = new IntPtr(*(void **)unoData);
            if (unoI != IntPtr.Zero)
            {
                TypeDescription *td = null; // FIXME leak
                TypeDescriptionReference.GetDescription(&td, type);
                managedData = MapUnoToManaged(unoI, (InterfaceTypeDescription *)td);
            }
            else
                managedData = null;
            break;
        }
        default:
        {
            // FIXME throw some exception
            break;
        }
        }
    }

    public static Type MapUnoType(TypeDescription *TD)
    {
        return MapUnoType(TD->pWeakRef);
    }

    public static Type MapUnoType(TypeDescriptionReference *TD)
    {
        Type result;

        switch(TD->eTypeClass)
        {
        case TypeClass.VOID:
            result = typeof(void);
            break;
        case TypeClass.CHAR:
            result = typeof(char);
            break;
        case TypeClass.BOOLEAN:
            result = typeof(bool);
            break;
        case TypeClass.BYTE:
            result = typeof(byte);
            break;
        case TypeClass.SHORT:
            result = typeof(short);
            break;
        case TypeClass.UNSIGNED_SHORT:
            result = typeof(ushort);
            break;
        case TypeClass.LONG:
            result = typeof(int);
            break;
        case TypeClass.UNSIGNED_LONG:
            result = typeof(uint);
            break;
        case TypeClass.HYPER:
            result = typeof(long);
            break;
        case TypeClass.UNSIGNED_HYPER:
            result = typeof(ulong);
            break;
        case TypeClass.FLOAT:
            result = typeof(float);
            break;
        case TypeClass.DOUBLE:
            result = typeof(double);
            break;
        case TypeClass.STRING:
            result = typeof(string);
            break;
        case TypeClass.TYPE:
            result = typeof(Type);
            break;
        case TypeClass.ANY:
            result = typeof(uno.Any);
            break;
        case TypeClass.ENUM:
        case TypeClass.STRUCT:
        case TypeClass.EXCEPTION:
            result = LoadCliType(TD->pTypeName);
            break;
        case TypeClass.INTERFACE:
            // special handling for XInterface, since it does not exist in cli.
            if (UString.UStringToString(TD->pTypeName) == "com.sun.star.uno.XInterface")
                result = typeof(object);
            else
                result = LoadCliType(TD->pTypeName);
            break;
        case TypeClass.SEQUENCE:
        {
            TypeDescription *seqType = null; // FIXME leak
            TypeDescriptionReference.GetDescription(&seqType, TD);

            // FIXME do something with TD here?
            TypeDescriptionReference *elementTDRef =
                ((IndirectTypeDescription *)seqType)->pType;

            switch (elementTDRef->eTypeClass)
            {
            case TypeClass.CHAR:
                result = Type.GetType("System.Char[]");
                break;
            case TypeClass.BOOLEAN:
                result = Type.GetType("System.Boolean[]");
                break;
            case TypeClass.BYTE:
                result = Type.GetType("System.Byte[]");
                break;
            case TypeClass.SHORT:
                result = Type.GetType("System.Int16[]");
                break;
            case TypeClass.UNSIGNED_SHORT:
                result = Type.GetType("System.UInt16[]");
                break;
            case TypeClass.LONG:
                result = Type.GetType("System.Int32[]");
                break;
            case TypeClass.UNSIGNED_LONG:
                result = Type.GetType("System.UInt32[]");
                break;
            case TypeClass.HYPER:
                result = Type.GetType("System.Int64[]");
                break;
            case TypeClass.UNSIGNED_HYPER:
                result = Type.GetType("System.UInt64[]");
                break;
            case TypeClass.FLOAT:
                result = Type.GetType("System.Single[]");
                break;
            case TypeClass.DOUBLE:
                result = Type.GetType("System.Double[]");
                break;
            case TypeClass.STRING:
                result = Type.GetType("System.String[]");
                break;
            case TypeClass.TYPE:
                result = Type.GetType("System.Type[]");
                break;
            case TypeClass.ANY:
            case TypeClass.ENUM:
            case TypeClass.EXCEPTION:
            case TypeClass.STRUCT:
            case TypeClass.INTERFACE:
            case TypeClass.SEQUENCE:
                result = LoadCliType(TD->pTypeName);
                break;
            default:
                // FIXME can't happen
                result = null;
                break;
            }
            break;
        }
        default:
            // FIXME can't happen
            result = null;
            break;
        }
        return result;
    }

    public static Type LoadCliType(UString* unoName)
    {
        return LoadCliType(MapUnoTypeName(UString.UStringToString(unoName)));
    }

    public static Type LoadCliType(string unoName)
    {
        Type result = null;
        bool isPolymorphic = false;

        string loadName = unoName;
        int index = unoName.IndexOf('<');
        if (index != -1)
        {
            loadName = unoName.Substring(0, index);
            isPolymorphic = true;
        }

        result = Type.GetType(loadName + ",cli_uretypes");

        if (result == null)
            result = Type.GetType(loadName + ",cli_basetypes");

        if (result == null)
            result = Type.GetType(loadName, false);

        if (result == null)
        {
            foreach (Assembly a in AppDomain.CurrentDomain.GetAssemblies())
            {
                result = a.GetType(loadName, false);
                if (result != null)
                    break;
            }
        }

        if (result == null)
            // FIXME don't use generic Exception type
            throw new Exception("A type could not be loaded: " + loadName);

        if (isPolymorphic)
            result = uno.PolymorphicType.GetType(result, unoName);

        return result;
    }

    static TypeDescriptionReference *MapManagedType(Type managedType)
    {
        TypeDescriptionReference *result = null;
        if (managedType == null)
        {
            result = *TypeDescriptionReference.GetByTypeClass(TypeClass.VOID);
            TypeDescriptionReference.Acquire(result);
            return result;
        }

        // check for Enum first,
        // because otherwise case System.TypeCode.Int32 applies
        if (managedType.IsEnum)
        {
            UString* unoTypeName = MapManagedTypeName(managedType.FullName);
            TypeDescriptionReference.New(&result, TypeClass.ENUM, unoTypeName);
            TypeDescriptionReference.Acquire(result);
        }
        else
        {
            switch (System.Type.GetTypeCode(managedType))
            {
            case System.TypeCode.Boolean:
                result = *TypeDescriptionReference.GetByTypeClass(TypeClass.BOOLEAN);
                TypeDescriptionReference.Acquire(result);
                break;
            case System.TypeCode.Char:
                result = *TypeDescriptionReference.GetByTypeClass(TypeClass.CHAR);
                TypeDescriptionReference.Acquire(result);
                break;
            case System.TypeCode.Byte:
                result = *TypeDescriptionReference.GetByTypeClass(TypeClass.BYTE);
                TypeDescriptionReference.Acquire(result);
                break;
            case System.TypeCode.Int16:
                result = *TypeDescriptionReference.GetByTypeClass(TypeClass.SHORT);
                TypeDescriptionReference.Acquire(result);
                break;
            case System.TypeCode.Int32:
                result = *TypeDescriptionReference.GetByTypeClass(TypeClass.LONG);
                TypeDescriptionReference.Acquire(result);
                break;
            case System.TypeCode.Int64:
                result = *TypeDescriptionReference.GetByTypeClass(TypeClass.HYPER);
                TypeDescriptionReference.Acquire(result);
                break;
            case System.TypeCode.UInt16:
                result = *TypeDescriptionReference.GetByTypeClass(TypeClass.UNSIGNED_SHORT);
                TypeDescriptionReference.Acquire(result);
                break;
            case System.TypeCode.UInt32:
                result = *TypeDescriptionReference.GetByTypeClass(TypeClass.UNSIGNED_LONG);
                TypeDescriptionReference.Acquire(result);
                break;
            case System.TypeCode.UInt64:
                result = *TypeDescriptionReference.GetByTypeClass(TypeClass.UNSIGNED_HYPER);
                TypeDescriptionReference.Acquire(result);
                break;
            case System.TypeCode.Single:
                result = *TypeDescriptionReference.GetByTypeClass(TypeClass.FLOAT);
                TypeDescriptionReference.Acquire(result);
                break;
            case System.TypeCode.Double:
                result = *TypeDescriptionReference.GetByTypeClass(TypeClass.DOUBLE);
                TypeDescriptionReference.Acquire(result);
                break;
            case System.TypeCode.String:
                result = *TypeDescriptionReference.GetByTypeClass(TypeClass.STRING);
                TypeDescriptionReference.Acquire(result);
                break;
            }
        }

        if (result == null)
        {
            string managedTypeName = managedType.FullName;
            if (managedTypeName == "System.Void")
            {
                result = *TypeDescriptionReference.GetByTypeClass(TypeClass.VOID);
                TypeDescriptionReference.Acquire(result);
            }
            else if (managedTypeName == "System.Type")
            {
                result = *TypeDescriptionReference.GetByTypeClass(TypeClass.TYPE);
                TypeDescriptionReference.Acquire(result);
            }
            else if (managedTypeName == "uno.Any")
            {
                result = *TypeDescriptionReference.GetByTypeClass(TypeClass.ANY);
                TypeDescriptionReference.Acquire(result);
            }
            else
            {
                UString* unoTypeName;

                uno.PolymorphicType poly = managedType as uno.PolymorphicType;
                if (poly != null)
                    unoTypeName = MapManagedTypeName(poly.PolymorphicName);
                else
                    unoTypeName = MapManagedTypeName(managedTypeName);

                TypeDescription *td = null;
                TypeDescription.GetByName(&td, unoTypeName);
                if (td != null)
                {
                    result = td->pWeakRef;
                    TypeDescriptionReference.Acquire(result);
                    TypeDescription.Release(td);
                }
            }
        }

        if (result == null)
        {
            // FIXME - quite probably we should throw an exception here instead.
            result = *TypeDescriptionReference.GetByTypeClass(TypeClass.VOID);
            TypeDescriptionReference.Acquire(result);
        }

        return result;
    }

    static string MapUnoTypeName(string typeName)
    {
        StringBuilder buf = new StringBuilder();

        // determine if the type is a sequence and its dimensions
        int dims = 0;
        if (typeName[0] == '[')
        {
            int index = 1;
            while (true)
            {
                if (typeName[index++] == ']')
                    ++dims;
                if (typeName[index++] != '[')
                    break;
            }
            typeName = typeName.Substring(index - 1);
        }

        switch (typeName)
        {
        case "boolean":
            buf.Append("System.Boolean");
            break;
        case "char":
            buf.Append("System.Char");
            break;
        case "byte":
            buf.Append("System.Byte");
            break;
        case "short":
            buf.Append("System.Int16");
            break;
        case "unsigned short":
            buf.Append("System.UInt16");
            break;
        case "long":
            buf.Append("System.Int32");
            break;
        case "unsigned long":
            buf.Append("System.UInt32");
            break;
        case "hyper":
            buf.Append("System.Int64");
            break;
        case "unsigned hyper":
            buf.Append("System.UInt64");
            break;
        case "float":
            buf.Append("System.Single");
            break;
        case "double":
            buf.Append("System.Double");
            break;
        case "string":
            buf.Append("System.String");
            break;
        case "void":
            buf.Append("System.Void");
            break;
        case "type":
            buf.Append("System.Type");
            break;
        case "com.sun.star.uno.XInterface":
            buf.Append("System.Object");
            break;
        case "any":
            buf.Append("uno.Any");
            break;
        default:
            // put "unoidl." at the beginning
            buf.Append("unoidl.");
            // for polymorphic struct types remove the brackets, e.g. mystruct<bool> -> mystruct
            buf.Append(MapUnoPolymorphicName(typeName));
            break;
        }

        // append []
        for ( ; dims > 0; --dims)
            buf.Append("[]");

        return buf.ToString();
    }

    /** For example, there is a uno type
        com.sun.star.Foo<char, long>.
        The values in the type list
        are uno types and are replaced by cli types, such as System.Char,
        System.Int32, etc.
    */
    static string MapUnoPolymorphicName(string unoName)
    {
        int startIndex = unoName.LastIndexOf('<');
        if (startIndex == -1)
            return unoName;

        // get the type list within < and >
        int endIndex = unoName.LastIndexOf('>');
        string list = unoName.Substring(startIndex + 1, endIndex - startIndex - 1);

        // parse the type list and replace the types with the corresponding CLI types
        char[] delimiters = new char[] { ',' };
        string[] unoTypes = list.Split(delimiters);

        StringBuilder builder = new StringBuilder(unoName.Substring(0, startIndex + 1));

        int typeCount = unoTypes.Length;
        for (int i = 0; i < typeCount; ++i)
            builder.Append(MapUnoTypeName(unoTypes[i]));

        builder.Append('>');
        return builder.ToString();
    }

    static UString* MapManagedTypeName(string typeName)
    {
        int dims = 0;
        int index = 0;

        if ((index = typeName.IndexOf("[]")) > 0)
        {
            dims = 1;

            int curIndex = index;
            while ((curIndex + 2) < typeName.Length &&
                   (curIndex = typeName.IndexOf("[]", curIndex + 2)) > 0)
                ++dims;

            // get the element name by removing the brackets
            typeName = typeName.Substring(0, index);
        }

        StringBuilder buf = new StringBuilder(256);
        for (; dims > 0; --dims)
            buf.Append("[]");

        switch (typeName)
        {
        case "System.Boolean":
            buf.Append("boolean");
            break;
        case "System.Char":
            buf.Append("char");
            break;
        case "System.Byte":
            buf.Append("byte");
            break;
        case "System.Int16":
            buf.Append("short");
            break;
        case "System.UInt16":
            buf.Append("unsigned short");
            break;
        case "System.Int32":
            buf.Append("long");
            break;
        case "System.UInt32":
            buf.Append("unsigned long");
            break;
        case "System.Int64":
            buf.Append("hyper");
            break;
        case "System.UInt64":
            buf.Append("unsigned hyper");
            break;
        case "System.Single":
            buf.Append("float");
            break;
        case "System.Double":
            buf.Append("double");
            break;
        case "System.String":
            buf.Append("string");
            break;
        case "System.Void":
            buf.Append("void");
            break;
        case "System.Type":
            buf.Append("type");
            break;
        case "System.Object":
            buf.Append("com.sun.star.uno.XInterface");
            break;
        case "uno.Any":
            buf.Append("any");
            break;
        default:
        {
            string name = MapManagedPolymorphicName(typeName);
            int i = name.IndexOf('.');
            buf.Append(name.Substring(i + 1));
            break;
        }
        }

        UString *rtl_uString = null;
        UString.NewFromStringBuilder(&rtl_uString, buf);
        return rtl_uString;
    }

    static string MapManagedPolymorphicName(string unoName)
    {
        int startIndex = unoName.LastIndexOf('<');
        if (startIndex == -1)
            return unoName;

        // get the type list withing < and >
        int endIndex = unoName.LastIndexOf('>');
        string list = unoName.Substring(startIndex + 1, endIndex - startIndex - 1);

        // parse the type list and replace the types with the corresponding CLI types
        char[] delimiters = new char[] { ',' };
        string[] unoTypes = list.Split(delimiters);

        StringBuilder builder = new StringBuilder(unoName.Substring(0, startIndex + 1));

        int typeCount = unoTypes.Length;
        for (int i = 0; i < typeCount; ++i)
            builder.Append(UString.UStringToString(MapManagedTypeName(unoTypes[i])));
        builder.Append('>');
        return builder.ToString();
    }

    [StructLayout(LayoutKind.Explicit)]
    private unsafe struct largest
    {
        [FieldOffset(0)] long n;
        [FieldOffset(0)] double d;
        [FieldOffset(0)] void *p;
        [FieldOffset(0)] uno.Binary.Any a;
    }

    // FIXME args[i] must be of same type as return value
    public unsafe uno.Any CallUno(IntPtr unoInterface, TypeDescription *memberTD,
                                  TypeDescriptionReference *returnType, int nParams,
                                  MethodParameter *parameters, object[] args,
                                  Type[] argTypes, out uno.Any exception)
    {
        int returnSize = sizeof(largest);

        if (returnType != null &&
            (returnType->eTypeClass == TypeClass.STRUCT ||
             returnType->eTypeClass == TypeClass.EXCEPTION))
        {
            // FIXME leak
            TypeDescription *td = null;
            TypeDescriptionReference.GetDescription(&td, returnType);

            if (td->nSize > returnSize)
                returnSize = td->nSize;
        }

        // Prepare memory that contains all converted arguments and
        // return values. The memory block contains first pointers to
        // the arguments which are in the same block For example, 2
        // arguments, 1 ret.
        //
        //      | Pointer
        //      | Pointer
        //      | Return value
        //      | Arg 1
        //      | Arg 2
        //
        // If an argument is larger then struct largest, such as some
        // structures, then the pointer points to an extra block of
        // memory. The same goes for a big return value.
        // FIXME the last sentence is bullshit. Get it deleted from cli_uno ;)
        byte *mem = stackalloc byte[nParams * sizeof(void *) +
                                    returnSize +
                                    nParams * sizeof(largest)];

        // array of pointers to args
        void **unoArgPtrs = (void **)mem;

        // Return Value
        void *unoRetPtr = null;
        largest *unoArgs = (largest *)(unoArgPtrs + nParams);
        if (memberTD->eTypeClass != TypeClass.INTERFACE_ATTRIBUTE || nParams != 1)
    {
            // If an attribute is set, then unoRet must be null, e.g. void setAttribute(int)
        unoRetPtr = (void *)unoArgs;
        unoArgs = (largest *)((byte *)unoRetPtr + returnSize);
    }

        for (int i = 0; i < nParams; ++i)
        {
            // FIXME it's a TypeDescriptionReference
            TypeDescription *type = (TypeDescription *)parameters[i].pTypeRef;

            unoArgPtrs[i] = unoArgs + i;
            if ((type->eTypeClass == TypeClass.STRUCT ||
                 type->eTypeClass == TypeClass.EXCEPTION) &&
                (type->nSize > sizeof(largest)))
            {
                // stackalloc is only allowed in initializers
                byte *bigArgPtr = stackalloc byte[type->nSize];

                unoArgPtrs[i] = bigArgPtr;
            }

            if (parameters[i].bIn != 0)
            {
                // FIXME error handling
                MapToUno(unoArgPtrs[i], args[i], type, false /* no assign */);
            }
        }

        uno.Binary.Any unoExceptionHolder;
        uno.Binary.Any *unoExc = &unoExceptionHolder;

        // call binary uno
        uno.Binary.Interface.Dispatch(
            unoInterface, memberTD, unoRetPtr, unoArgPtrs, &unoExc);

        if (unoExc == null)
        {
            exception = uno.Any.VOID;

            // convert out args, destroy uno args
            for (int i = 0; i < nParams; ++i)
            {
                // FIXME it's a TypeDescriptionReference
                TypeDescription *type = (TypeDescription *)parameters[i].pTypeRef;

                if (parameters[i].bOut != 0)
                {
                    // FIXME error handling
                    MapToManaged(ref args[i], unoArgPtrs[i], parameters[i].pTypeRef,
                                 argTypes != null ? argTypes[i] : null, false);
                }

                // cleanup args
                if (type->eTypeClass < TypeClass.DOUBLE &&
                    type->eTypeClass != TypeClass.ENUM) // no need to destroy these
                    uno.Binary.Data.Destroy(unoArgPtrs[i], type, null);
            }

            if (returnType != null && returnType->eTypeClass != TypeClass.VOID)
            {
                // convert uno return value
                object result = null;
                // FIXME error handling
                MapToManaged(ref result, unoRetPtr, returnType, null, false);
                uno.Binary.Data.Destroy(unoRetPtr, (TypeDescription *)returnType, null);
                return new uno.Any(MapUnoType(returnType), result); // FIXME is this correct?
            }

            return uno.Any.VOID;
        }
        else // exception occured
        {
            for (int i = 0; i < nParams; ++i)
                if (parameters[i].bIn != 0)
                    uno.Binary.Data.Destroy(unoArgPtrs[i], (TypeDescription *)parameters[i].pTypeRef, null);

            // FIXME needs uno.Any vs System.Object clarification
            object exc = null;
            MapToManaged(ref exc, unoExceptionHolder.pData,
                         unoExceptionHolder.pType, null, false);
            exception = new uno.Any(MapUnoType(unoExceptionHolder.pType), exc);
            return uno.Any.VOID;
        }

        // FIXME error handling
    }

    // FIXME rename, to say what it does, not how it does this
    public void RegisterWithCliEnvironment(object managedI, string oid)
    {
        managedEnvironment.registerInterface(managedI, oid);
    }

    public void RegisterWithCliEnvironment(object managedI, string oid, Type type)
    {
        managedEnvironment.registerInterface(managedI, oid, type);
    }

    public void RegisterWithUnoEnvironment(ref IntPtr unoInterface, string oid, InterfaceTypeDescription *TD)
    {
        UString *unoOid = null;
        UString.NewFromString(&unoOid, oid);

        uno.Binary.Environment.RegisterInterface(unoEnvironment, ref unoInterface, unoOid, TD);

        UString.Release(unoOid);
    }

    public void GetInterfaceFromUnoEnvironment(ref IntPtr unoInterface, UString* unoOid, InterfaceTypeDescription* TD)
    {
        uno.Binary.Environment.GetRegisteredInterface(unoEnvironment, ref unoInterface, unoOid, TD);
    }

    public void RevokeFromUnoEnvironment(IntPtr unoInterface)
    {
        throw new NotImplementedException();
    }

    public unsafe IntPtr CreateManagedProxy(object managedInterface,
                                            TypeDescription* td,
                                            UString* oid)
    {
        // register original interface
        RegisterWithCliEnvironment(managedInterface,
                                   UString.UStringToString(oid),
                                   Bridge.MapUnoType(td));

        ManagedProxy proxy = new ManagedProxy(this, managedInterface, td, oid);
        GCHandle gchandle = GCHandle.Alloc(proxy);

        // create binary uno uno_Interface and register proxy with target environment
        IntPtr unoI = CreateBinaryProxyAndRegister(unoEnvironment, (IntPtr)gchandle,
                                                   oid, td);

        proxy.NativeProxy = unoI;
        return unoI;
    }

    [ DllImport("cli_uno", EntryPoint="cli_uno_environment_createMonoProxyAndRegister") ]
    public static unsafe extern IntPtr CreateBinaryProxyAndRegister(
        IntPtr unoEnvironment,
        IntPtr monoProxy,
        /* UString */ void* oid,
        /* InterfaceTypeDescription */ void* td);

    public unsafe void CallManaged(object managedI, Type ifaceType, MethodInfo method,
                                   TypeDescriptionReference* returnType,
                                   MethodParameter* parameters, int nParams, void* unoRet,
                                   void** unoArgs, uno.Binary.Any** unoExc)
    {
        object[] args = new object[nParams];
        for (int i = 0; i < nParams; ++i)
            if (parameters[i].bIn != 0)
                MapToManaged(ref args[i], unoArgs[i], parameters[i].pTypeRef, null, false);

        object invocationResult = null;
        try
        {
            invocationResult = method.Invoke(managedI, args);
        }
        catch (TargetInvocationException e)
        {
            Exception exc = e.InnerException;
            TypeDescription* td = null;
            // FIXME leak
            TypeDescriptionReference.GetDescription(&td, MapManagedType(exc.GetType()));
            void* memExc = uno.rtl.Mem.Allocate(td->nSize);
            MapToUno(memExc, exc, /* FIXME !!!*/ (TypeDescription*)td->pWeakRef, false);
            (*unoExc)->pType = td->pWeakRef;
            (*unoExc)->pData = memExc;
            return;
        }
        catch (Exception e)
        {
            // FIXME
        }

        // convert out, in/out params
        for (int i = 0; i < nParams; ++i)
        {
            if (parameters[i].bOut != 0)
            {
                MapToUno(
                    unoArgs[i], args[i], /* FIXME !!! */(TypeDescription*)parameters[i].pTypeRef,
                    parameters[i].bIn != 0 /* assign if inout */);
                // FIXME error handling
            }
        }

        // return value
        if (returnType != null)
            MapToUno(unoRet, invocationResult, /* FIXME !!! */(TypeDescription*)returnType, false /* no assign */);

        // no exception occurred
        *unoExc = null;
    }
}

}
