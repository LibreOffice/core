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
using System.Runtime.CompilerServices;
using System.Threading;

namespace com.sun.star.uno.native
{
    internal class WeakIndexTable
    {
        private readonly Dictionary<int, WeakReference<object>> _objectByIndex;
        private readonly Dictionary<WeakReference<object>, int> _indexByObject;
        private readonly List<int> _cleanupIndices;
        private readonly object _lock;

        private int _counter;
        private int _updateOps;

        public WeakIndexTable()
        {
            _objectByIndex = new Dictionary<int, WeakReference<object>>();
            _indexByObject = new Dictionary<WeakReference<object>, int>(new WeakReferenceComparer());
            _cleanupIndices = new List<int>();
            _lock = new object();

            _counter = 1; // index 0 is reserved for null
            _updateOps = 0;
        }

        public object Lookup(int index)
        {
            if (index == 0)
                return null;

            if (_objectByIndex.TryGetValue(index, out WeakReference<object> weakRef))
                if (weakRef.TryGetTarget(out object obj))
                    return obj;

            throw new RuntimeException($"no object was found at index {index}", null);
        }

        public int Register(object obj)
        {
            if (obj == null)
                return 0;

            WeakReference<object> weakRef = new WeakReference<object>(obj);
            if (_indexByObject.TryGetValue(weakRef, out int index))
                return index;

            lock (_lock)
            {
                _objectByIndex.Add(_counter, weakRef);
                _indexByObject.Add(weakRef, _counter);

                _updateOps++;
                AutoClean();

                return _counter++;
            }
        }

        private void AutoClean()
        {
            if (_updateOps >= _objectByIndex.Count)
            {
                _updateOps = 0;
                foreach (int key in _objectByIndex.Keys)
                {
                    WeakReference<object> weakRef = _objectByIndex[key];
                    if (!weakRef.TryGetTarget(out _))
                        _cleanupIndices.Add(key);
                }

                foreach (int key in _cleanupIndices)
                    _objectByIndex.Remove(key);
                _cleanupIndices.Clear();
            }
        }

        private class WeakReferenceComparer : IEqualityComparer<WeakReference<object>>
        {
            public bool Equals(WeakReference<object> x, WeakReference<object> y)
            {
                if (x.TryGetTarget(out object first))
                    if (y.TryGetTarget(out object second))
                        return ReferenceEquals(first, second);
                return false;
            }

            public int GetHashCode(WeakReference<object> obj)
                => obj.TryGetTarget(out object target) ? target.GetHashCode() : 0;
        }
    }
}
