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
    internal static class TypeHelper
    {
        private static List<Assembly> _assemblies;
        private static Dictionary<string, Type> _cache;

        static TypeHelper()
        {
            _assemblies = AppDomain.CurrentDomain.GetAssemblies()
                .OrderBy(a => !a.FullName.StartsWith("net_"))
                .ThenBy(a => a != Assembly.GetExecutingAssembly())
                .ToList();
            _cache = new Dictionary<string, Type>();

            // TODO: verify this works as intended
            AppDomain.CurrentDomain.AssemblyLoad += (_, e) => _assemblies.Add(e.LoadedAssembly);
        }

        public static Type RemoveReference(Type type) => type.IsByRef ? type.GetElementType() : type;

        public static Type ParseType(string encoded)
        {
            if (_cache.TryGetValue(encoded, out Type cachedType))
                return cachedType;

            string baseName = encoded;
            int arrayDims = 0;

            int genericDepth = -1;
            int genericTypeCount = 0;
            foreach (char c in encoded)
            {
                switch (c)
                {
                    case '<':
                        genericDepth++;
                        break;
                    case ',':
                        if (genericDepth == 0)
                            genericTypeCount++;
                        break;
                    case '>':
                        if (genericDepth-- == 0)
                            genericTypeCount++;
                        break;
                }
            }
            Type[] genericTypes = new Type[genericTypeCount];

            genericDepth = -1;
            int genericStart = -1;
            int genericIndex = 0;
            for (int i = 0; i < encoded.Length; i++)
            {
                char c = encoded[i];
                switch (c)
                {
                    case '<':
                        if (++genericDepth == 0)
                        {
                            baseName = encoded.Substring(0, i);
                            genericStart = i + 1;
                        }
                        break;
                    case ',':
                        if (genericDepth == 0)
                        {
                            string genericType = encoded.Substring(genericStart, i - genericStart);
                            genericTypes[genericIndex++] = ParseType(genericType);
                            genericStart = i + 1;
                        }
                        break;
                    case '>':
                        if (genericDepth-- == 0)
                        {
                            string genericType = encoded.Substring(genericStart, i - genericStart);
                            genericTypes[genericIndex++] = ParseType(genericType);
                            genericStart = i + 1;
                        }
                        break;
                    case ']':
                        if (genericDepth == 0)
                        {
                            ++arrayDims;
                        }
                        break;
                }
            }

            Type returnType = genericTypes.Length > 0
                ? LoadType($"{baseName}`{genericTypes.Length}").MakeGenericType(genericTypes)
                : LoadType(baseName);

            for (int i = 0; i < arrayDims; ++i)
                returnType = returnType.MakeArrayType();

            return _cache[encoded] = returnType;
        }

        private static Type LoadType(string name)
        {
            foreach (Assembly assembly in _assemblies)
            {
                Type t = assembly.GetType(name);
                if (t != null)
                    return t;
            }
            throw new RuntimeException($"could not load type {name}", null);
        }
    }
}
