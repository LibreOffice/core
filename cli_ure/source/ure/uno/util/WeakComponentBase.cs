/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WeakComponentBase.cs,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 02:03:15 $
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
using unoidl.com.sun.star.lang;

namespace uno.util
{

/** This class can be used as a base class for UNO objects.
    It implements the capability to be kept weakly
    (unoidl.com.sun.star.uno.XWeak) and it implements
    unoidl.com.sun.star.lang.XTypeProvider which is necessary for
    using the object from StarBasic.
    In addition, it implements the interface
    unoidl.com.sun.star.lang.XComponent to be disposed explicitly.
*/
public class WeakComponentBase : WeakBase, XComponent
{
    private delegate void t_disposing( EventObject evt );
    private t_disposing m_disposing = null;
    private bool m_inDispose = false;
    private bool m_disposed = false;

    /** Indicates whether object is alrady disposed.
        
        @return
                true, if object has been disposed
    */
    protected bool isDisposed()
    {
        lock (this)
        {
            return m_disposed;
        }
    }

    /** Checks whether this object is disposed and throws a DisposedException
        if it is already disposed.
    */
    protected void checkUnDisposed()
    {
        if (! isDisposed())
        {
            throw new unoidl.com.sun.star.lang.DisposedException(
                "object already disposed!", this );
        }
    }
    
    ~WeakComponentBase()
    {
        bool doDispose;
        lock (this)
        {
            doDispose = (!m_inDispose && !m_disposed);
        }
        if (doDispose)
        {
            dispose();
        }
    }
    
    /** Override to perform extra clean-up work. Provided for subclasses.
        It is called during dispose()
    */
    protected void preDisposing()
    {
    }
    
    /** Override to become notified right before the disposing action is
        performed.
    */
    protected void postDisposing()
    {
    }
    
    // XComponent impl
    /** This method is called by the owner of this object to explicitly
        dispose it.  This implementation of dispose() first notifies this object
        via preDisposing(), then  all registered event listeners and
        finally this object again calling postDisposing().
    */
    public void dispose()
    {
        // Determine in a thread-safe way if this is the first call to this
        // method.  Only then we proceed with the notification of event
        // listeners.  It is an error to call this method more then once.
        bool doDispose = false;
        t_disposing call = null;
        lock (this)
        {
            if (! m_inDispose && !m_disposed)
            {
                call = m_disposing;
                m_disposing = null;
                m_inDispose = true;
                doDispose = true;
            }
        }
        // The notification occures in an unsynchronized block in order to avoid
        // deadlocks if one of the listeners calls back in a different thread on
        // a synchronized method which uses the same object.
        if (doDispose)
        {
            try
            {
                // call sub class
                preDisposing();
                // send disposing notifications to listeners
                if (null != call)
                {
                    EventObject evt = new EventObject( this );
                    call( evt );
                }
                // call sub class
                postDisposing();
            }
            finally
            {
                // finally makes sure that the flags are set ensuring
                // that this function is only called once.
                m_disposed = true;
                m_inDispose = false;
            }
        }
        else
        {
            // in a multithreaded environment, it can't be avoided,
            // that dispose is called twice.
            // However this condition is traced, because it MAY indicate an
            // error.
#if DEBUG
            Console.WriteLine(
                "WeakComponentBase.dispose() - dispose called twice" );
#endif
//             Debug.Fail( "WeakComponentBase.dispose() - dispose called twice" );
        }
    }
    /** Registers an event listener being notified when this object is disposed.

        @param xListener event listener
    */
    public void addEventListener( XEventListener xListener )
    {
        bool add;
        lock (this)
        {
            add = (! m_inDispose && !m_disposed);
            if (add)
                m_disposing += new t_disposing( xListener.disposing );
        }
        if (! add)
            xListener.disposing( new EventObject( this ) );
    }
    /** Revokes an event listener from being notified when this object
        is disposed.
        
        @param xListener event listener
    */
    public void removeEventListener( XEventListener xListener )
    {
        lock (this)
        {
            if (! m_inDispose && !m_disposed)
                m_disposing -= new t_disposing( xListener.disposing );
        }
    }
}

}
