/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WeakAdapter.cs,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 02:02:45 $
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
