/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;

using com.sun.star.uno;

namespace com.sun.star.uno.native
{
    internal class NativeUnoProxy : DispatchProxy
    {
        private IntPtr _bridgeHandle;
        private IntPtr _interfaceHandle;
        private IntPtr _typeHandle;
        private NetEnvironment _env;

        public string Oid { get; private set; }
        public Type Type { get; private set; }

        ~NativeUnoProxy() => _env.ReleaseProxy(this, _bridgeHandle, _interfaceHandle, _typeHandle);

        public static object CreateProxy(string oid, Type interfaceType,
            IntPtr pBridge, IntPtr pUnoInterface, IntPtr pTypeDesc, NetEnvironment env)
        {
            if (!typeof(IQueryInterface).IsAssignableFrom(interfaceType))
                throw new RuntimeException($"interfaceType must be IQueryInterface or a derived interface, was {interfaceType}", null);

            MethodInfo method = typeof(DispatchProxy).GetMethod("Create", Type.EmptyTypes);
            MethodInfo typedMethod = method.MakeGenericMethod(interfaceType, typeof(NativeUnoProxy));

            NativeUnoProxy proxy = (NativeUnoProxy)typedMethod.Invoke(null, null);
            proxy.Oid = oid;
            proxy.Type = interfaceType;
            proxy._bridgeHandle = pBridge;
            proxy._interfaceHandle = pUnoInterface;
            proxy._typeHandle = pTypeDesc;
            proxy._env = env;

            return proxy;
        }

        protected override object Invoke(MethodInfo targetMethod, object[] args)
            => _env.InvokeMethod(targetMethod, args, _bridgeHandle, _interfaceHandle, _typeHandle);

        public override bool Equals(object obj) => obj is NativeUnoProxy nup && Oid == nup.Oid;
        public override int GetHashCode() => Oid.GetHashCode();
        public override string ToString() => $"UNO Proxy {{ OID = {Oid}, Interface = {Type} }}";
    }
}
