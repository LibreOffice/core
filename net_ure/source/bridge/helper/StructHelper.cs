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
using System.Reflection;

namespace com.sun.star.uno.helper
{
    internal static class StructHelper
    {
        private static Dictionary<Type, (Func<object[], object>, Type[])> _ctorCache;
        private static Dictionary<Type, (Action<object, object[]>, Type[])> _dtorCache;
        static StructHelper()
        {
            _ctorCache = new Dictionary<Type, (Func<object[], object>, Type[])>();
            _dtorCache = new Dictionary<Type, (Action<object, object[]>, Type[])>();
        }

        public static (Func<object[], object>, Type[]) GetConstructor(Type type)
        {
            if (_ctorCache.TryGetValue(type, out (Func<object[], object>, Type[]) cached))
                return cached;

            ConstructorInfo ctor = type.GetConstructors().First(c => c.GetParameters().Length > 0);

            Func<object[], object> func = args => ctor.Invoke(args);
            Type[] paramTypes = ctor.GetParameters()
                .Select(p => TypeHelper.RemoveReference(p.ParameterType))
                .ToArray();

            return _ctorCache[type] = (func, paramTypes);
        }

        public static (Action<object, object[]>, Type[]) GetDeconstructor(Type type)
        {
            if (_dtorCache.TryGetValue(type, out (Action<object, object[]>, Type[]) cached))
                return cached;

            MethodInfo dtor = type.GetMethod("Deconstruct", BindingFlags.Instance | BindingFlags.Public);

            Action<object, object[]> action = (target, args) => dtor.Invoke(target, args);
            Type[] paramTypes = dtor.GetParameters()
                .Select(p => TypeHelper.RemoveReference(p.ParameterType))
                .ToArray();

            return _dtorCache[type] = (action, paramTypes);
        }
    }
}
