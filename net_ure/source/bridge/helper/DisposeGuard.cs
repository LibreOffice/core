/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

using System;

using com.sun.star.lang;

namespace com.sun.star.uno.helper
{
    // This class can be used to auto-dispose UNO components 
    // from managed code with a using block/statement.
    public class DisposeGuard : IDisposable
    {
        private readonly XComponent _component;
        public DisposeGuard(XComponent component) => _component = component;

        public void Dispose() => _component?.dispose();
    }

}
