/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DisposeNotifier.java,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
     * adding a listener to an already disposed notifer, the listener must
     * eventually receive a <code>notifyDispose</code> callback.</p>
     *
     * @param listener a dispose listener, to be notified when this object is
     * disposed; must not be <code>null</code>
     */
    void addDisposeListener(DisposeListener listener);
}
