/*************************************************************************
 *
 *  $RCSfile: WeakReference.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jl $ $Date: 2002-04-11 15:41:03 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package com.sun.star.uno;

import com.sun.star.uno.XWeak;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XAdapter;
import com.sun.star.uno.XReference;

/** This class holds weak reference to an object. It actually holds a reference to a
    com.sun.star.XAdapter implementation and obtains a hard reference if necessary.
 */
public class WeakReference
{
    private final boolean DEBUG= false;
    private OWeakRefListener m_listener;
    // There is no default constructor. Every instance must register itself with the
    // XAdapter interface, which is done in the constructors. Assume we have this code
    // WeakReference ref= new WeakReference();
    // ref = someOtherWeakReference;
    //
    // ref would not be notified (XReference.dispose()) because it did not register
    // itself. Therefore the XAdapter would be kept aliver although this is not
    // necessary.

    /** Creates an instance of this class.
     *@param obj - another instance that is to be copied
     */
    public WeakReference(WeakReference obj)
    {
        if (obj != null)
        {
            Object weakImpl= obj.get();
            if (weakImpl != null)
            {
                XWeak weak= (XWeak) UnoRuntime.queryInterface(XWeak.class, weakImpl);
                if (weak != null)
                {
                    XAdapter adapter= (XAdapter) weak.queryAdapter();
                    if (adapter != null)
                        m_listener= new OWeakRefListener(adapter);
                }
            }
        }
    }

    /** Creates an instance of this class.
     *@param obj XWeak implementation
     */
    public WeakReference(Object obj)
    {
        XWeak weak= (XWeak) UnoRuntime.queryInterface(XWeak.class, obj);
        if (weak != null)
        {
            XAdapter adapter= (XAdapter) weak.queryAdapter();
            if (adapter != null)
                m_listener= new OWeakRefListener(adapter);
        }
    }
    /** Returns a hard reference to the object that is kept weak by this class.
     *@return a hard reference to the XWeak implementation.
     */
    public Object get()
    {
        if (m_listener != null)
            return m_listener.get();
        return null;
    }
}

/** Implementation of com.sun.star.uno.XReference for use with WeakReference.
 *  It keeps the XAdapter implementation and registers always with it. Deregistering
 *  occurs on notification by the adapter and the adapter is released.
 */
class OWeakRefListener implements XReference
{
    private final boolean DEBUG= false;
    private XAdapter m_adapter;

    /** The constructor registered this object with adapter.
     *@param adapter the XAdapter implementation.
     */
    OWeakRefListener( XAdapter adapter)
    {
        m_adapter= adapter;
        m_adapter.addReference(this);
    }
    /** Method of com.sun.star.uno.XReference. When called, it deregisteres this
     *  object with the adapter and releases the reference to it.
     */
    synchronized public void dispose()
    {
        if (m_adapter != null)
        {
            m_adapter.removeReference(this);
            m_adapter= null;
        }
    }

    /** Obtains a hard reference to the object which is kept weak by the adapter
     *  and returns it.
     *  @return hard reference to the otherwise weakly kept object.
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
