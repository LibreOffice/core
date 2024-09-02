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
using System.Threading;

namespace com.sun.star.uno.native
{
    internal class WeakOidTypeTable
    {
        private readonly Dictionary<string, Level1Entry> _level1Map;
        private readonly List<(string, Type)> _cleanupEntries;
        private readonly object _lock;

        private int _count;
        private int _updateOps;

        public WeakOidTypeTable()
        {
            _level1Map = new Dictionary<string, Level1Entry>();
            _cleanupEntries = new List<(string, Type)>();
            _lock = new object();

            _count = 0;
            _updateOps = 0;
        }

        public object RegisterInterface(object obj, string oid, Type type)
        {
            lock (_lock)
            {
                PurgeStaleEntries();

                if (!_level1Map.ContainsKey(oid))
                    _level1Map[oid] = new Level1Entry();

                Level1Entry level1 = _level1Map[oid];
                if (level1.TryGetCompatible(type, out Level2Entry level2))
                {
                    if (level2.TryGetTarget(out object o))
                    {
                        level2.Acquire();
                        return o;
                    }
                }

                level1[type] = new Level2Entry(obj);
                ++_count;
                ++_updateOps;

                return obj;
            }
        }

        public bool RevokeInterface(string oid, Type type)
        {
            lock (_lock)
            {
                Level2Entry level2 = null;

                if (_level1Map.TryGetValue(oid, out Level1Entry level1))
                {
                    if (level1.TryGetCompatible(type, out level2))
                    {
                        if (level2.Release())
                        {
                            level1.Remove(type);
                            if (level1.Count == 0)
                                _level1Map.Remove(oid);

                            --_count;
                            ++_updateOps;
                        }
                    }
                }

                PurgeStaleEntries();

                return level2 != null;
            }
        }

        public Object GetInterface(string oid, Type type)
        {
            lock (_lock)
            {
                if (_level1Map.TryGetValue(oid, out Level1Entry level1))
                    if (level1.TryGetCompatible(type, out Level2Entry level2))
                        if (level2.TryGetTarget(out object obj))
                            return obj;
                return null;
            }
        }

        public void Clear()
        {
            lock (_lock)
            {
                _level1Map.Clear();
                _updateOps = 0;
            }
        }

        private void PurgeStaleEntries()
        {
            if (_updateOps >= _count)
            {
                _updateOps = 0;

                foreach (string oid in _level1Map.Keys)
                {
                    Level1Entry level1 = _level1Map[oid];
                    foreach (Type type in level1.Keys)
                    {
                        Level2Entry level2 = level1[type];
                        if (!level2.TryGetTarget(out _))
                        {
                            _cleanupEntries.Add((oid, type));
                            --_count;
                        }
                    }
                }

                foreach ((string oid, Type type) in _cleanupEntries)
                {
                    _level1Map[oid].Remove(type);
                    if (_level1Map[oid].Count == 0)
                        _level1Map.Remove(oid);
                }
                _cleanupEntries.Clear();
            }
        }

        private class Level1Entry : Dictionary<Type, Level2Entry>
        {
            public bool TryGetCompatible(Type type, out Level2Entry level2)
            {
                if (TryGetValue(type, out level2))
                    return true;

                foreach (Type stored in Keys)
                {
                    if (type.IsAssignableFrom(stored))
                    {
                        level2 = this[stored];
                        return true;
                    }
                }

                level2 = null;
                return false;
            }
        }

        private class Level2Entry
        {
            private readonly WeakReference<object> _ref;
            private int _count;

            public Level2Entry(object obj)
            {
                _ref = new WeakReference<object>(obj);
                _count = 1;
            }

            public bool TryGetTarget(out object obj) => _ref.TryGetTarget(out obj);

            public void Acquire() => ++_count;
            public bool Release() => --_count == 0;
        }
    }
}
