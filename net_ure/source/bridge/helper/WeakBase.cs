/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Linq;

using com.sun.star.uno;
using com.sun.star.lang;

namespace com.sun.star.uno.helper
{
    // This class can be used as a helper base class for UNO interfaces.
    // It allows taking weak references to the object (com.sun.star.uno.XWeak)
    // and using the object from StarBasic (com.sun.star.lang.XTypeProvider).
    public class WeakBase : IQueryInterface, XWeak, XTypeProvider
    {
        private WeakAdapter _adapter;
        ~WeakBase() => _adapter?.RaiseTargetDestructing();

        private static readonly ConcurrentDictionary<Type, Type[]> _providedTypes;
        static WeakBase() => _providedTypes = new ConcurrentDictionary<Type, Type[]>();

        // IQueryInterface
        public Any queryInterface(Type type)
            => type.IsAssignableFrom(GetType()) ? new Any(type, this) : Any.VOID;

        // XWeak
        public XAdapter queryAdapter()
        {
            if (_adapter == null)
                _adapter = new WeakAdapter(this);
            return _adapter;
        }

        // XTypeProvider
        public Type[] getTypes()
        {
            return _providedTypes.GetOrAdd(GetType(),
                type => type.GetInterfaces()
                    .Where(i => typeof(IQueryInterface).IsAssignableFrom(i))
                    .ToArray()
            );
        }

        public sbyte[] getImplementationId() => Array.Empty<sbyte>();

        // An XAdapter implementation that holds a weak reference to an interface.
        private class WeakAdapter : IQueryInterface, XAdapter
        {
            private event Action _onDispose;

            private WeakReference<IQueryInterface> _weakRef;
            public WeakAdapter(IQueryInterface obj)
                => _weakRef = new WeakReference<IQueryInterface>(obj);

            public void RaiseTargetDestructing() => _onDispose?.Invoke();

            // IQueryInterface
            public Any queryInterface(Type type)
                => type.IsAssignableFrom(GetType()) ? new Any(type, this) : Any.VOID;

            // XAdapter
            public IQueryInterface queryAdapted()
                => _weakRef.TryGetTarget(out IQueryInterface target) ? target : null;

            public void addReference(XReference reference) => _onDispose += reference.dispose;
            public void removeReference(XReference reference) => _onDispose -= reference.dispose;
        }
    }
}
