/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

using System;

namespace com.sun.star.uno
{
    public interface IQueryInterface
    {
        Any queryInterface(Type type);
    }

    public static class IQueryInterfaceExtensions
    {
        public static bool implements<T>(this IQueryInterface iqi)
            where T : IQueryInterface
            => iqi is T ? true : iqi.queryInterface(typeof(T)).hasValue();

        public static T query<T>(this IQueryInterface iqi)
            where T : IQueryInterface
            => iqi is T t ? t : (T)iqi.queryInterface(typeof(T)).Value;

        public static T queryOrDefault<T>(this IQueryInterface iqi, T fallback = default)
            where T : IQueryInterface
        {
            if (iqi is T t)
                return t;
            Any result = iqi.queryInterface(typeof(T));
            return result.hasValue() ? (T)result.Value : fallback;
        }
    }
}
