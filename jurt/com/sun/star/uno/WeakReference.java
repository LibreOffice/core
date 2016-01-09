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

package com.sun.star.uno;

/**
 * This class holds weak reference to an object.
 *
 * <p>It actually holds a reference to a <code>com.sun.star.XAdapter</code>
 * implementation and obtains a hard reference if necessary.
 */
public class WeakReference
{
    private OWeakRefListener m_listener;
    // There is no default constructor. Every instance must register itself with the
    // XAdapter interface, which is done in the constructors. Assume we have this code
    // WeakReference ref= new WeakReference();
    // ref = someOtherWeakReference;
    //
    // ref would not be notified (XReference.dispose()) because it did not register
    // itself. Therefore the XAdapter would be kept alive although this is not
    // necessary.

    /**
     * Creates an instance of this class.
     *
     * @param obj another instance that is to be copied.
     */
    public WeakReference(WeakReference obj)
    {
        if (obj == null) {
            return;
        }
        Object weakImpl = obj.get();
        if (weakImpl == null) {
            return;
        }
        XWeak weak = UnoRuntime.queryInterface(XWeak.class, weakImpl);
        if (weak != null) {
            XAdapter adapter = weak.queryAdapter();
            if (adapter != null)
                m_listener = new OWeakRefListener(adapter);
        }
    }

    /**
     * Creates an instance of this class.
     *
     * @param obj XWeak implementation.
     */
    public WeakReference(Object obj)
    {
        XWeak weak= UnoRuntime.queryInterface(XWeak.class, obj);
        if (weak != null)
        {
            XAdapter adapter= weak.queryAdapter();
            if (adapter != null)
                m_listener= new OWeakRefListener(adapter);
        }
    }

    /**
     * Returns a hard reference to the object that is kept weak by this class.
     *
     * @return a hard reference to the XWeak implementation.
     */
    public Object get()
    {
        if (m_listener != null)
            return m_listener.get();
        return null;
    }
}

/**
 * Implementation of com.sun.star.uno.XReference for use with WeakReference.
 *
 * <p>It keeps the XAdapter implementation and registers always with it.
 * Deregistering occurs on notification by the adapter and the adapter is
 * released.</p>
 */
class OWeakRefListener implements XReference
{
    private XAdapter m_adapter;

    /**
     * The constructor registered this object with adapter.
     *
     * @param adapter the XAdapter implementation.
     */
    OWeakRefListener( XAdapter adapter)
    {
        m_adapter= adapter;
        m_adapter.addReference(this);
    }

    /**
     * Method of <code>com.sun.star.uno.XReference</code>.
     *
     * <p>When called, it deregisters this object with the adapter and releases
     * the reference to it.</p>
     */
    synchronized public void dispose()
    {
        if (m_adapter != null)
        {
            m_adapter.removeReference(this);
            m_adapter= null;
        }
    }

    /**
     * Obtains a hard reference to the object which is kept weak by the adapter
     * and returns it.
     *
     * @return hard reference to the otherwise weakly kept object.
     */
    synchronized Object get()
    {
        Object retVal= null;
        if (m_adapter != null)
        {
            retVal= m_adapter.queryAdapted();
            if (retVal == null)
            {
                // If this object registered as listener with XAdapter while it was notifying
                // the listeners then this object might not have been notified. If queryAdapted
                // returned null then the weak kept object is dead and the listeners have already
                // been notified. And we missed it.
                m_adapter.removeReference(this);
                m_adapter= null;
            }
        }
        return retVal;
    }
}
