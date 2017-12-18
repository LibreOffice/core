/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

// __________ Imports __________

import java.util.*;

// __________ Implementation __________

/**
 * It's implement a static container which hold
 * all opened documents and her views alive.
 * It's possible to register/deregister such views,
 * to get information about these and it provides
 * some global functionality - like termination of
 * this demo application.
 *
 */
public class ViewContainer extends Thread
{


    /**
     * provides a singleton view container
     * Necessary for terminate(9 functionality to be able
     * to call Runtime.runFinilization().
     *
     * @return a reference to the singleton ViewContainer instance
     */
    public static synchronized ViewContainer getGlobalContainer()
    {
        if (maSingleton==null)
            maSingleton=new ViewContainer();
        return maSingleton;
    }



    /**
     * ctor
     * It's private - because nobody should create any instance
     * expect the only global one, which will be created by ourself!
     */
    private ViewContainer()
    {
        mlViews          = new ArrayList<Object>();
        mlListener       = new ArrayList<IShutdownListener>();
        mbShutdownActive = false       ;
        Runtime.getRuntime().addShutdownHook(this);
    }



    /**
     * This register a new view inside this global container
     * (if it doesn't already exist).
     *
     * @param   aView       view which wish to be registered inside this container
     */
    public void addView(Object aView)
    {
        synchronized(mlViews)
        {
            if(!mlViews.contains(aView))
                mlViews.add(aView);
        }
    }



    /**
     * This deregister a view from this global container.
     * Normally it should be the last reference to the view
     * and her finalize() method should be called.
     * If last view will be closed here - we terminate these
     * java application too. Because there is no further
     * visible frame anymore.
     *
     * @param aView
     *          view object which wish to be deregistered
     */
    public void removeView(Object aView)
    {
        int nViewCount = 0;
        synchronized(mlViews)
        {
            if(mlViews.contains(aView))
                mlViews.remove(aView);

            nViewCount = mlViews.size();

            if (nViewCount<1)
                mlViews = null;
        }
        // If this view is a registered shutdown listener on this view container
        // too, we must call his interface and forget him as possible listener.
        // It's necessary to guarantee his dead ...
        boolean bShutdownView = false;
        synchronized(mlListener)
        {
            bShutdownView = mlListener.contains(aView);
            if (bShutdownView)
                mlListener.remove(aView);
        }
        if (bShutdownView)
            ((IShutdownListener)aView).shutdown();

        // We use a system.exit() to finish the whole application.
        // And further we have registered THIS instance as a possible shutdown
        // hook at the runtime class. So our run() method will be called.
        // Our view container should be empty - but
        // our listener container can include some references.
        // These objects will be informed then and release e.g. some
        // remote references.
        if (nViewCount<1)
        {
            boolean bNecessary = false;
            synchronized(this)
            {
                bNecessary = ! mbShutdownActive;
            }
            if (bNecessary)
            {
                System.out.println("call exit(0)!");
                System.exit(0);
            }
        }
    }



    /**
     * add/remove listener for possible shutdown events
     */
    public void addListener( IShutdownListener rListener )
    {
        synchronized(mlListener)
        {
            if ( ! mlListener.contains(rListener) )
                mlListener.add(rListener);
        }
    }



    private void removeListener( IShutdownListener rListener )
    {
        synchronized(mlListener)
        {
            if ( mlListener.contains(rListener) )
                mlListener.remove(rListener);
        }
    }



    /**
     * Is called from current runtime system of the java machine
     * on shutdown. We inform all current registered listener and
     * views. They should deinitialize her internal things then.
     */
    @Override
    public void run()
    {
        synchronized(this)
        {
            if (mbShutdownActive)
                return;
            mbShutdownActive=true;
        }

        while( true )
        {
            IShutdownListener aListener = null;
            synchronized(mlListener)
            {
                if (!mlListener.isEmpty())
                    aListener = mlListener.get(0);
            }
            if (aListener==null)
                break;

            aListener.shutdown();
            // May this listener has deregistered himself.
            // But if not we must do it for him. Our own
            // method "removeListener()" ignore requests for
            // already gone listener objects.
            removeListener(aListener);
        }

        if (mlViews!=null)
        {
            synchronized(mlViews)
            {
                mlViews.clear();
                mlViews = null;
            }
        }

        if (mlListener!=null)
        {
            synchronized(mlListener)
            {
                mlListener.clear();
                mlListener = null;
            }
        }
    }



    /**
     * @member  mbInplace           indicates using of inplace office frames instead of outplace ones
     * @member  maSingleton         singleton instance of this view container
     * @member  mlViews             list of all currently registered document views
     * @member  mlListener          list of all currently registered shutdown listener
     * @member  mbShutdownActive    if this shutdown hook already was started it's not a good idea to
     *                              call System.exit() again for other conditions.
     *                              We suppress it by using this variable!
     */
    public  static boolean       mbInplace   = false ;
    private static ViewContainer maSingleton = null  ;
    private        ArrayList<Object>        mlViews             ;
    private        ArrayList<IShutdownListener>        mlListener          ;
    private        boolean       mbShutdownActive    ;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
