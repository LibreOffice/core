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

using System;
using unoidl.com.sun.star.uno;
using unoidl.com.sun.star.lang;

namespace uno.util
{
    
/** An XAdapter implementation that holds a weak reference
    (System.WeakReference) to an object.
    Clients can register listeners (unoidl.com.sun.star.lang.XReference)
    which are notified when the object (the one which is kept weak) is
    being finalized.  That is, that object is being destroyed because there
    are not any hard references to it.
*/
public class WeakAdapter : XAdapter
{
    // references the XWeak implementation
    private WeakReference m_weakRef;
    // contains XReference objects registered by addReference
    private delegate void XReference_dispose();
    private XReference_dispose m_XReference_dispose;
    
    /** ctor.
        
        @param obj the object that is to be held weakly
    */
    public WeakAdapter( Object obj )
    {
        m_weakRef = new WeakReference( obj );
        m_XReference_dispose = null;
    }
    
    /** Called by the XWeak implementation (WeakBase) when it is being
        finalized.  It is only being called once.
        The registererd XReference listeners are notified. On notification
        they are  to unregister themselves. The notification is thread-safe.
        However, it is possible to add a listener during the notification
        process, which will never receive a notification.
        To prevent this, one would have to synchronize this method with 
        the addReference method. But this can result in deadlocks in a
        multithreaded environment.
    */
    internal /* non-virtual */ void referentDying()
    {
        XReference_dispose call;
        lock (this)
        {
            call = m_XReference_dispose;
            m_XReference_dispose = null;
        }
        if (null != call)
            call();
    }
    
    // XAdapter impl
    
    /** Called to obtain a hard reference o the object which is kept weakly
        by this instance.
        
        @return hard reference to the object
    */
    public Object queryAdapted()
    {
        return m_weakRef.Target;
    }
    /** Called by clients to register listener which are notified when the
        weak object is dying.
        
        @param xReference a listener
    */
    public void removeReference( XReference xReference )
    {
        lock (this)
        {
            m_XReference_dispose -=
                new XReference_dispose( xReference.dispose );
        }
    }
    /** Called by clients to unregister listeners.
        
        @param xReference a listener
    */
    public void addReference( XReference xReference )
    {
        lock (this)
        {
            m_XReference_dispose +=
                new XReference_dispose( xReference.dispose );
        }
    }
}

}
