/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package com.sun.star.lib.util;

/**
 * Optional interface to be implemented by objects stored in a
 * <code>WeakMap</code>.
 *
 * @see WeakMap
 */
public interface DisposeNotifier {
    /**
     * Adds a dispose listener, to be notified when this object is disposed.
     *
     * <p>It is unspecified what happens when the same listener is added
     * multiple times.</p>
     *
     * <p>It is unspecified exactly when the <code>notifyDispose</code> callback
     * is fired: immediately before the notifier is disposed, while it is in the
     * process of disposing, or some time after it got disposed.  But even if
     * adding a listener to an already disposed notifier, the listener must
     * eventually receive a <code>notifyDispose</code> callback.</p>
     *
     * @param listener a dispose listener, to be notified when this object is
     * disposed; must not be <code>null</code>
     */
    void addDisposeListener(DisposeListener listener);
}
