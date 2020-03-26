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

package com.sun.star.lib.uno.helper;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XEventListener;
import com.sun.star.lang.EventObject;
import com.sun.star.uno.Type;

/** This class can be used as the base class for UNO components. In addition to the functionality ,which
 *  is inherited from WeakBase, it implements com.sun.star.lang.XComponent.
 */
public class ComponentBase extends WeakBase implements XComponent
{
    private static final boolean DEBUG= false;
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
        // It is an error to call this method more than once.
        boolean bDoDispose= false;
        synchronized (this)
        {
            if ( ! bInDispose && ! bDisposed)
            {
                bDoDispose= true;
                bInDispose= true;
            }
        }
        // The notification occurs in an unsynchronized block in order to avoid
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
                synchronized (this)
                {
                    bDisposed= true;
                    bInDispose= false;
                }
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

    @Override
    protected void finalize() throws Throwable
    {
        if ( ! bInDispose && ! bDisposed)
            dispose();
        super.finalize();
    }
}
