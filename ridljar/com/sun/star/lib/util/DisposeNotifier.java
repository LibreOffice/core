/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DisposeNotifier.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 13:20:36 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
