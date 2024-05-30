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
    public class Any
    {
        public static readonly Any VOID = new Any(typeof(void), null);

        public Type Type { get; private set; }
        public Object Value { get; private set; }

        public Any(Type type, object value) => setValue(type, value);
        public static Any with<T>(T value) => new Any(typeof(T), value);

        public bool hasValue() => Type != typeof(void);

        public void setValue(Type type, object value)
        {
            if (type is null)
                throw new ArgumentNullException(nameof(type), "Type of Any cannot be null.");

            if (type == typeof(Any))
                throw new ArgumentException("Any object cannot be nested inside another Any.");

            if (value is null && type != typeof(void))
                throw new ArgumentException("Value of Any can only be null if Type is void." +
                    " Perhaps you want Any.VOID?");

            Type = type;
            Value = value;
        }

        public bool equals(Any obj) => Type == obj.Type && Value == obj.Value;

        public override bool Equals(object obj) => (obj is Any other) && equals(other);
        public override int GetHashCode() => (Type, Value).GetHashCode();
        public override string ToString() => $"uno.Any {{ Type = {Type}, Value = {Value ?? "Null"} }}";
    }
}
