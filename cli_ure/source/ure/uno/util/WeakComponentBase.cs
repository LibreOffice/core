/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/



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
