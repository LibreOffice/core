/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ComponentBase.java,v $
 * $Revision: 1.5 $
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

package com.sun.star.lib.uno.helper;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XEventListener;
import com.sun.star.lang.EventObject;
import java.util.List;
import java.util.Collections;
import java.util.LinkedList;
import com.sun.star.uno.Type;

/** This class can be used as the base class for UNO components. In addition to the functionality ,which
 *  is inherited from WeakBase, it implements com.sun.star.lang.XComponent.
 */
public class ComponentBase extends WeakBase implements XComponent
{
    private final boolean DEBUG= false;
    protected MultiTypeInterfaceContainer listenerContainer;
    protected boolean bInDispose= false;
    protected boolean bDisposed= false;
    static final Type  EVT_LISTENER_TYPE= new Type(XEventListener.class);


    /** Creates a new instance of CompBase */
    public ComponentBase()
    {
        super();
        listenerContainer= new MultiTypeInterfaceContainer();
    }

    /** Override to perform extra clean-up work. Provided for subclasses. It is
        called during dispose()
     */
    protected void preDisposing()
    {
    }
    /** Override to become notified right before the disposing action is performed.
     */
    protected void postDisposing()
    {
    }


    /** Method of XComponent. It is called by the owning client when the component is not needed
     *  anymore. The registered listeners are notified that this method has been called.
     */
    public void dispose()
    {
        // Determine in a thread-safe way if this is the first call to this method.
        // Only then we proceed with the notification of event listeners.
        // It is an error to call this method more then once.
        boolean bDoDispose= false;
        synchronized (this)
        {
            if ( ! bInDispose && ! bDisposed)
            {
                bDoDispose= true;
                bInDispose= true;
            }
        }
        // The notification occures in an unsynchronized block in order to avoid
        // deadlocks if one of the listeners calls back in a different thread on
        // a synchronized method which uses the same object.
        if (bDoDispose)
        {
            try
            {
                preDisposing();
                listenerContainer.disposeAndClear(new EventObject(this));
                //notify subclasses that disposing is in progress
                postDisposing();
            }
            finally
            {
                // finally makes sure that the  flags are set even if a RuntimeException is thrown.
                // That ensures that this function is only called once.
                bDisposed= true;
                bInDispose= false;
            }
        }
        else
        {
            // in a multithreaded environment, it can't be avoided, that dispose is called twice.
            // However this condition is traced, because it MAY indicate an error.
            if (DEBUG)
                System.out.println("OComponentHelper::dispose() - dispose called twice" );
        }
    }

    /** Method of XComponent.
     */
    public void removeEventListener(XEventListener xEventListener)
    {
        listenerContainer.removeInterface( EVT_LISTENER_TYPE, xEventListener);
    }

    public void addEventListener(XEventListener listener)
    {
        boolean bDoDispose= false;
        synchronized (this)
        {
            if (bDisposed || bInDispose)
                bDoDispose= true;
            else
               listenerContainer.addInterface(EVT_LISTENER_TYPE, listener);
        }
        if (bDoDispose )
        {
            listener.disposing( new EventObject(this));
        }
    }

    protected void finalize() throws Throwable
    {
        if ( ! bInDispose && ! bDisposed)
            dispose();
        super.finalize();
    }
}
