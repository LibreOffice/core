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
    // It allows taking weak references to the object (com.sun.star.uno.XWeak),
    // using the object from StarBasic (com.sun.star.lang.XTypeProvider)
    // and explicitly controlling its lifetime (com.sun.star.lang.XComponent)
    public class WeakComponentBase : WeakBase, XComponent
    {
        private readonly List<XEventListener> _listeners;
        public WeakComponentBase() => _listeners = new List<XEventListener>();

        // XComponent
        public void dispose()
        {
            EventObject e = new EventObject(this);
            foreach (XEventListener listener in _listeners)
                listener.disposing(e);
            _listeners.Clear();
        }

        public void addEventListener(XEventListener listener) => _listeners.Add(listener);
        public void removeEventListener(XEventListener listener) => _listeners.Remove(listener);
    }
}
