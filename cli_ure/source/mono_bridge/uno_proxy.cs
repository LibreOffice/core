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

using System;
using System.Collections;
using System.Diagnostics;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Runtime.Remoting;
using System.Runtime.Remoting.Messaging;
using System.Runtime.Remoting.Proxies;

using uno.Binary;
using uno.Typelib;
using uno.rtl;

namespace com.sun.star.bridges.mono_uno /* FIXME use some uno.foo namespace ? */
{

public unsafe class UnoInterfaceInfo
{
    public IntPtr UnoInterface; // wrapped interface
    public Type Type; // mapped type
    public com.sun.star.bridges.mono_uno.Bridge Bridge;
    public InterfaceTypeDescription *TypeDesc;

    public UnoInterfaceInfo(com.sun.star.bridges.mono_uno.Bridge bridge,
                            IntPtr unoInterface,
                            InterfaceTypeDescription *td)
    {
        Bridge = bridge;
        UnoInterface = unoInterface;
        Type = Bridge.MapUnoType((TypeDescription *)td);
        uno.Binary.Interface.Acquire(UnoInterface);
        TypeDesc = td;
        InterfaceTypeDescription.Acquire(TypeDesc);

        fixed (InterfaceTypeDescription **ppTypeDesc = &TypeDesc)
            if (((TypeDescription *)TypeDesc)->bComplete == 0 &&
                TypeDescription.Complete((TypeDescription **)ppTypeDesc))
            {
                // FIXME throw a uno runtime exception
            }
    }

    ~UnoInterfaceInfo()
    {
        Bridge.RevokeFromUnoEnvironment(UnoInterface);
        uno.Binary.Interface.Release(UnoInterface);
        InterfaceTypeDescription.Release(TypeDesc);
    }
}

public unsafe class UnoInterfaceProxy: RealProxy, IRemotingTypeInfo
{
    /** used for IRemotingTypeInfo.TypeName
     */
    string typeName = "System.Object";

    /** The list is filled with UnoInterfaceInfo objects. The list can only
        grow and elements are never changed. If an element was added it
        must not be changed!
    */
    ArrayList interfaces = new ArrayList(10); // of UnoInterfaceInfo

    /** The list is filled with additional UnoInterfaceProxy object due
        to aggregation via bridges.  Though the latter is strongly
        discouraged, this has to be supported.
    */
    ArrayList additionalProxies = new ArrayList();

    Bridge bridge;
    string oid;

    private unsafe UnoInterfaceProxy(Bridge bridge, IntPtr unoInterface,
                                     InterfaceTypeDescription *TD, string oid)
        : base(typeof(MarshalByRefObject)) // FIXME is there a better type?
    {
        this.bridge = bridge;
        this.oid = oid;
        AddUnoInterface(unoInterface, TD);
    }

    ~UnoInterfaceProxy()
    {
        // FIXME should revokeInterface from environment, but can't
        // access managed string oid any longer.
    }

    public static unsafe object Create(Bridge bridge, IntPtr unoInterface,
                                       InterfaceTypeDescription *TD, string oid)
    {
        UnoInterfaceProxy realProxy = new UnoInterfaceProxy(bridge, unoInterface,
                                                            TD, oid);
        object proxy = realProxy.GetTransparentProxy();
        bridge.RegisterWithCliEnvironment(proxy, oid);
        return proxy;
    }

    // RealProxy members
    public unsafe override IMessage Invoke(IMessage request)
    {
        IMethodCallMessage callmsg = (IMethodCallMessage)request;

        // Find out which UNO interface is being called
        string typeName = callmsg.TypeName;
        typeName = typeName.Substring(0, typeName.IndexOf(','));

        // Special Handling for System.Object methods
        if (typeName.IndexOf("System.Object") != -1)
        {
            return InvokeObjectMethod(request);
        }

        Type typeBeingCalled = Bridge.LoadCliType(typeName);
        UnoInterfaceInfo info = FindInfo(typeBeingCalled);

        Trace.Assert(info != null);

        string methodName = callmsg.MethodName;
        TypeDescriptionReference **ppAllMembers =
        info.TypeDesc->ppAllMembers;
        int numMembers = info.TypeDesc->nAllMembers;
        for (int i = numMembers - 1; i >= 0; --i)
        {
            TypeDescriptionReference *memberTD = *(ppAllMembers + i);

            // FIXME do without string conversion?
            string memberTypeName = UString.UStringToString(memberTD->pTypeName);
            // check methodName against fully qualified memberTypeName
            // of memberTD; memberTypeName is of the form
            //  <name> "::" <methodName> *(":@" <idx> "," <idx> ":" <name>)

            int offset = memberTypeName.IndexOf(':') + 2;
            int remainder = memberTypeName.Length - offset;
            if (memberTD->eTypeClass == TypeClass.INTERFACE_METHOD)
            {
                if ((methodName.Length == remainder ||
                     (methodName.Length < remainder &&
                      memberTypeName[offset + methodName.Length] == ':')) &&
                    String.Compare(memberTypeName, offset,
                                   methodName, 0, methodName.Length) == 0)
                {
                    TypeDescription *methodTD = null;
            // FIXME release it
            TypeDescriptionReference.GetDescription(&methodTD, memberTD);

                    uno.Any exception;
                    uno.Any result =
                        bridge.CallUno(info.UnoInterface,
                                       methodTD,
                                       ((InterfaceMethodTypeDescription *)methodTD)->pReturnTypeRef,
                                       ((InterfaceMethodTypeDescription *)methodTD)->nParams,
                       ((InterfaceMethodTypeDescription *)methodTD)->pParams,
                                       callmsg.Args,
                                       /* FIXME this is an implementation detail,
                                          documented on MSDN, but still an implementation
                                          detail. cli_uno does the same */
                                       (System.Type[])callmsg.MethodSignature,
                                       out exception);
                    return ConstructReturnMessage(result, callmsg.Args,
                           (InterfaceMethodTypeDescription *)methodTD,
                                                  callmsg, exception);
                }
            }
            else // INTERFACE_ATTRIBUTE
            {
                if (methodName.Length > 4 &&
                    (methodName.Length - 4 == remainder ||
                     (methodName.Length - 4 <  remainder &&
                      memberTypeName[offset + methodName.Length - 4] == ':')) &&
                    methodName[1] == 'e' && methodName[2] == 't' &&
                    String.Compare(memberTypeName, offset,
                                   methodName, 4, methodName.Length - 4) == 0)
                {
                    InterfaceAttributeTypeDescription *attributeTD = null;
            // FIXME release it
                    TypeDescriptionReference.GetDescription( (TypeDescription **)&attributeTD,
                                                             memberTD );
                    uno.Any exception;
                    uno.Any result;

                    if (methodName[0] == 'g') // "get"
                    {
                        result = bridge.CallUno(info.UnoInterface,
                                                (TypeDescription *)attributeTD,
                                                attributeTD->pAttributeTypeRef,
                                                0, null, null, null,
                                                out exception);
                        return ConstructReturnMessage(result, null, null,
                                                      callmsg, exception);
                    }
                    else if (methodName[0] == 's') // "set"
                    {
                        if (attributeTD->bReadOnly != 0)
                            /* FIXME should we generate an exception? */
                            return ConstructReturnMessage(uno.Any.VOID, null, null,
                                                          callmsg, uno.Any.VOID);

                        MethodParameter param;
                        param.pTypeRef = attributeTD->pAttributeTypeRef;
                        param.bIn = 1;
                        param.bOut = 0;

                        result =
                            bridge.CallUno(info.UnoInterface,
                                           (TypeDescription *)attributeTD,
                                           TypeDescription.VoidType,
                                           1, &param,
                                           callmsg.Args, null, /* FIXME ??? from cli_uno */
                                           out exception);
                        return ConstructReturnMessage(uno.Any.VOID, null, null,
                                                      callmsg, exception);
                    }
                    break;
                }
            }
        }
        // FIXME check if the message of the exception is not crippled

        // the thing that should not be... no method info found!
        // FIXME throw unoidl.com.sun.star.uno.RuntimeException

        return null;
    }

    // IRemotingTypeInfo members
    public string TypeName
    {
        get { return typeName; }
        set { typeName = value; }
    }

    public unsafe bool CanCastTo(Type fromType, object o)
    {
        if (fromType == typeof(Object))
        return true;

        lock (this) {
            if (FindInfo(fromType) != null)
                // type is already in our list of Interfaces
                return true;

            // queryInterface for the required type
            // there is always a least one interface in our list
            UnoInterfaceInfo info = (UnoInterfaceInfo)interfaces[0];
            // ppAllMembers[0] corresponds to queryInterface
            TypeDescription *queryI = null;

            TypeDescriptionReference.GetDescription( // FIXME release it when you're done
                &queryI, *(info.TypeDesc->ppAllMembers));

            object[] args = new object[] { fromType };
            uno.Any exception;

            uno.Any result = bridge.CallUno(info.UnoInterface,
                                            queryI,
                                            ((InterfaceMethodTypeDescription *)queryI)->pReturnTypeRef,
                                            1, ((InterfaceMethodTypeDescription *)queryI)->pParams, args, null,
                                            out exception);

            // queryInterface doesn't throw exceptions.

            if (result.Type != typeof(void)) // result has a value
            {
                if (FindInfo(fromType) != null)
                {
                    // the proxy supports the requested interface now
                    return true;
                }

                // via aggregation: it is possible that queryInterface() returns
                //                  and interface with a different oid.
                //                  That way, this type is supported for the CLI
                //                  interpreter (CanCastTo() returns true)
                object obj = result.Value;
                if (RemotingServices.IsTransparentProxy(obj))
                {
                    UnoInterfaceProxy proxy =
                        (UnoInterfaceProxy)RemotingServices.GetRealProxy(obj);
                    additionalProxies.Add(proxy);
                    return true;
                }
            }
        }
        return false;
    }

    // internals
    public unsafe void AddUnoInterface(IntPtr unoInterface, InterfaceTypeDescription *TD)
    {
        lock (this)
        {
            foreach (UnoInterfaceInfo info in interfaces)
            {
                if (InterfaceTypeDescription.Equal(info.TypeDesc, TD))
                    return;
            }
            // This proxy does not contain the unoInterface. Add it.
            bridge.RegisterWithUnoEnvironment(ref unoInterface,
                                              oid, TD);
            interfaces.Add(new UnoInterfaceInfo(bridge, unoInterface, TD));
        }
    }

    UnoInterfaceInfo FindInfo(Type type)
    {
        foreach (UnoInterfaceInfo info in interfaces)
        {
            if (type.IsAssignableFrom(info.Type))
                return info;
        }
        foreach (UnoInterfaceProxy proxy in additionalProxies)
        {
            UnoInterfaceInfo info = proxy.FindInfo(type);
            if (info != null)
                return info;
        }
        return null;
    }

    static Type MapUnoType(TypeDescription *TD)
    {
        return MapUnoType(TD->pWeakRef);
    }

    static Type MapUnoType(TypeDescriptionReference *TD)
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
            result = Bridge.LoadCliType(TD->pTypeName);
            break;
        case TypeClass.INTERFACE:
            // special handling for XInterface, since it does not exist in cli.
            if (UString.UStringToString(TD->pTypeName) == "com.sun.star.uno.XInterface")
                result = typeof(object);
            else
                result = Bridge.LoadCliType(TD->pTypeName);
            break;
        case TypeClass.SEQUENCE:
        {
            // FIXME do something with TD here?
            TypeDescriptionReference *elementTDRef =
                ((IndirectTypeDescription *)TD)->pType;
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
                result = Bridge.LoadCliType(TD->pTypeName);
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

    IMessage InvokeObjectMethod(IMessage request)
    {
        IMethodMessage methodmsg = (IMethodMessage)request;
        object ret;
        switch (methodmsg.MethodName)
        {
        case "Equals":
            ret = false;

            if (RemotingServices.IsTransparentProxy(methodmsg.Args[0]))
            {
                UnoInterfaceProxy unoProxy =
                    RemotingServices.GetRealProxy(methodmsg.Args[0]) as UnoInterfaceProxy;

                if (unoProxy != null)
                {
                    ret = oid.Equals(unoProxy.Oid);
                    break;
                }
            }
            break;
        case "GetHashCode":
            ret = oid.GetHashCode();
            break;
        case "GetType":
            ret = typeof(System.Object);
            break;
        case "ToString":
            ret = String.Format("Uno object proxy. OID: {0}", oid);
            break;
        default:
            // Cannot happen
            ret = null;
            break;
        }

        return new ReturnMessage(ret, new object[0], 0,
                                 methodmsg.LogicalCallContext,
                                 (IMethodCallMessage)methodmsg);
    }

    public string Oid {
        get { return oid; }
    }

    IMessage ConstructReturnMessage(uno.Any result, object[] args,
                                    InterfaceMethodTypeDescription *methodTD,
                                    IMethodCallMessage callmsg, uno.Any exception)
    {
        if (exception.hasValue())
        {
            throw (System.Exception)exception.Value;
        }
        else
        {
            if (args != null)
            {
                object[] outArgs = new object[methodTD->nParams];
                int numOutArgs = 0;
                for (int i = 0; i < methodTD->nParams; ++i)
                {
                    if (methodTD->pParams[i].bOut == 1)
                    {
                        outArgs[i] = args[i];
                        ++numOutArgs;
                    }
                }
                return new ReturnMessage(result.Value, outArgs, numOutArgs,
                                         callmsg.LogicalCallContext,
                                         callmsg);
            }
            else
            {
                return new ReturnMessage(result.Value, null, 0,
                                         callmsg.LogicalCallContext,
                                         callmsg);
            }
        }
    }
}

}
