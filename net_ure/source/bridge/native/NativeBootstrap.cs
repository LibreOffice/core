/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;

using com.sun.star.uno.native;

namespace com.sun.star.uno
{
    public static class NativeBootstrap
    {
        private static LinkedList<WeakReference<NetEnvironment>> _environments;
        static NativeBootstrap() => _environments = new LinkedList<WeakReference<NetEnvironment>>();

        public static XComponentContext bootstrap()
        {
            NetEnvironment env = new NetEnvironment();
            _environments.AddLast(new WeakReference<NetEnvironment>(env));

            IntPtr result = InteropMethods.bootstrap(env.Context);
            return (XComponentContext)env.GetRegisteredObject(result);
        }

        public static XComponentContext defaultBootstrap_InitialComponentContext(
            string iniFile = null, IDictionary<string, string> parameters = null)
        {
            iniFile = string.IsNullOrWhiteSpace(iniFile) ? null : iniFile;

            string joinedParams = string.Join("|",
                parameters?.Select(p => $"{p.Key}={p.Value}") ?? Array.Empty<string>());
            joinedParams = string.IsNullOrWhiteSpace(joinedParams) ? null : joinedParams;

            NetEnvironment env = new NetEnvironment();
            _environments.AddLast(new WeakReference<NetEnvironment>(env));

            IntPtr result = InteropMethods.defaultBootstrap_InitialComponentContext(
                iniFile, joinedParams, env.Context);
            return (XComponentContext)env.GetRegisteredObject(result);
        }
    }
}
