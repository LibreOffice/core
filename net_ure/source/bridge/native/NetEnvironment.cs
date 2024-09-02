/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

using System;
using System.Collections.Generic;
using System.Reflection;
using System.Runtime.InteropServices;

using com.sun.star.uno.helper;

namespace com.sun.star.uno.native
{
    internal class NetEnvironment
    {
        public InteropContext Context { get; }

        private readonly Marshaller _marshaller;
        private readonly WeakIndexTable _indices;
        private readonly WeakOidTypeTable _locals;
        private readonly WeakOidTypeTable _proxies;
        private readonly string _oidSuffix;
        private readonly Dictionary<int, string> _oids;

        public NetEnvironment()
        {
            Context = new InteropContext()
            {
                createProxy = CreateProxy,
                lookupObjectId = LookupOid,
                registerInterface = RegisterInterface,
                lookupInterface = LookupInterface,
                revokeInterface = RevokeInterface,
                dispatchCall = DispatchCall,
                throwError = ThrowError,
            };

            _marshaller = new Marshaller(this);
            _indices = new WeakIndexTable();
            _locals = new WeakOidTypeTable();
            _proxies = new WeakOidTypeTable();
            _oidSuffix = $";net[0];{Guid.NewGuid()}";
            _oids = new Dictionary<int, string>();
        }

        public IntPtr CreateProxy(string oid, string interfaceName,
            IntPtr pBridge, IntPtr pUnoInterface, IntPtr pTypeDesc)
        {
            Type interfaceType = TypeHelper.ParseType(interfaceName);
            object proxy = NativeUnoProxy.CreateProxy(oid, interfaceType,
                pBridge, pUnoInterface, pTypeDesc, this);
            int index = _indices.Register(proxy);
            _oids.Add(index, oid);
            _proxies.RegisterInterface(proxy, oid, interfaceType);
            return (IntPtr)index;
        }

        public string LookupOid(IntPtr pObject) => _oids[(int)pObject];

        public IntPtr RegisterInterface(IntPtr pObject, string pOid, string pInterfaceName)
        {
            Type interfaceType = TypeHelper.ParseType(pInterfaceName);
            object obj = _indices.Lookup((int)pObject);
            obj = (obj is NativeUnoProxy ? _proxies : _locals)
                .RegisterInterface(obj, pOid, interfaceType);
            return (IntPtr)_indices.Register(obj);
        }

        public IntPtr RegisterLocal(object obj, Type interfaceType)
        {
            int index = _indices.Register(obj);
            if (!_oids.ContainsKey(index))
            {
                string oid = $"{index}{_oidSuffix}";
                _oids.Add(index, oid);
            }
            _locals.RegisterInterface(obj, _oids[index], interfaceType);
            return (IntPtr)index;
        }

        public IntPtr LookupInterface(string pOid, string pInterfaceName)
        {
            Type interfaceType = TypeHelper.ParseType(pInterfaceName);
            return LookupInterface(pOid, interfaceType);
        }
        public IntPtr LookupInterface(string pOid, Type type)
        {
            object obj = _proxies.GetInterface(pOid, type)
                ?? _locals.GetInterface(pOid, type);
            return (IntPtr)_indices.Register(obj);
        }

        public void RevokeInterface(string pOid, string pInterfaceName)
        {
            Type interfaceType = TypeHelper.ParseType(pInterfaceName);
            if (!_proxies.RevokeInterface(pOid, interfaceType))
                _locals.RevokeInterface(pOid, interfaceType);
        }

        public void ThrowError(string pWhere, string pMessage)
        {
            throw new RuntimeException($"{pMessage} at {pWhere}", null);
        }

        public object GetRegisteredObject(IntPtr pIndex)
        {
            return _indices.Lookup((int)pIndex);
        }

        public unsafe void ReleaseProxy(NativeUnoProxy nup,
            IntPtr pBridge, IntPtr pUnoInterface, IntPtr pTypeDesc)
        {
            _oids.Remove(_indices.Register(nup));
            _proxies.RevokeInterface(nup.Oid, nup.Type);
            InteropMethods.releaseProxy(pBridge, pUnoInterface, pTypeDesc);
        }

        public unsafe object InvokeMethod(MethodInfo targetMethod, object[] args,
            IntPtr bridgeHandle, IntPtr interfaceHandle, IntPtr typeHandle)
        {
            ParameterInfo[] parameters = targetMethod.GetParameters();

            InteropValue* values = stackalloc InteropValue[parameters.Length + 2];
            InteropValue* retVal = &values[parameters.Length];
            InteropValue* excVal = &values[parameters.Length + 1];

            for (int i = 0; i < parameters.Length; i++)
            {
                ParameterInfo param = parameters[i];
                Type paramType = TypeHelper.RemoveReference(param.ParameterType);
                bool isInOrInOut = !(param.ParameterType.IsByRef && param.IsOut);
                if (isInOrInOut)
                    _marshaller.MarshalObject(paramType, args[i], &values[i], false);
            }

            bool error = InteropMethods.dispatchCall(
                bridgeHandle, interfaceHandle, typeHandle,
                targetMethod.Name, values, retVal, excVal) == 0;

            for (int i = 0; i < parameters.Length; i++)
            {
                ParameterInfo param = parameters[i];
                Type paramType = TypeHelper.RemoveReference(param.ParameterType);
                bool isInOutOrOut = param.ParameterType.IsByRef;
                if (isInOutOrOut)
                    _marshaller.UnmarshalObject(paramType, ref args[i], &values[i], true);
            }

            if (error)
            {
                object exception = null;
                _marshaller.UnmarshalObject(typeof(Any), ref exception, excVal, true);
                throw (Exception)((Any)exception).Value;
            }

            if (targetMethod.ReturnType != typeof(void))
            {
                object result = null;
                _marshaller.UnmarshalObject(targetMethod.ReturnType, ref result, retVal, true);
                return result;
            }

            return null;
        }

        public unsafe byte DispatchCall(IntPtr pNetInterface, string pMethodName,
            IntPtr pArgs, IntPtr pRet, IntPtr pExc)
        {
            try
            {
                int methodNameStart = pMethodName.IndexOf(':') + 2;
                int methodNameEnd = pMethodName.IndexOf(':', methodNameStart);
                if (methodNameEnd > methodNameStart)
                    pMethodName = pMethodName.Substring(methodNameStart, methodNameEnd - methodNameStart);
                else
                    pMethodName = pMethodName.Substring(methodNameStart);

                object target = _indices.Lookup((int)pNetInterface);
                if (target == null)
                    throw new RuntimeException($"{pMethodName} was called on a null or unregistered interface", null);

                Type targetType = target.GetType();
                MethodInfo targetMethod = targetType.GetMethod(pMethodName,
                    BindingFlags.Instance | BindingFlags.Public | BindingFlags.FlattenHierarchy);
                if (targetMethod == null)
                    throw new RuntimeException($"could not find method {pMethodName} on interface {target.GetType()}", null);

                ParameterInfo[] parameters = targetMethod.GetParameters();
                object[] args = new object[parameters.Length];

                for (int i = 0; i < parameters.Length; i++)
                {
                    ParameterInfo param = parameters[i];
                    Type paramType = TypeHelper.RemoveReference(param.ParameterType);
                    bool isInOrInOut = !(param.ParameterType.IsByRef && param.IsOut);
                    if (isInOrInOut)
                        _marshaller.UnmarshalObject(paramType, ref args[i], &((InteropValue*)(void*)pArgs)[i], true);
                }

                object ret = targetMethod.Invoke(target, args);

                for (int i = 0; i < parameters.Length; i++)
                {
                    ParameterInfo param = parameters[i];
                    Type paramType = TypeHelper.RemoveReference(param.ParameterType);
                    bool isInOutOrOut = param.ParameterType.IsByRef;
                    if (isInOutOrOut)
                        _marshaller.MarshalObject(paramType, args[i], &((InteropValue*)(void*)pArgs)[i], false);
                }

                if (targetMethod.ReturnType != typeof(void))
                    _marshaller.MarshalObject(targetMethod.ReturnType, ret, (InteropValue*)(void*)pRet, false);
                return 1;
            }
            catch (Exception exception)
            {
                _marshaller.MarshalObject(typeof(Any), new Any(exception.InnerException),
                    (InteropValue*)(void*)pExc, false);
                return 0;
            }
        }
    }
}
