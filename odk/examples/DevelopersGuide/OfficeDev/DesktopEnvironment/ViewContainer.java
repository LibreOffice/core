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



// __________ Imports __________

import com.sun.star.uno.UnoRuntime;

import java.lang.*;
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
 * @author     Andreas Schl&uuml;ns
 * @created    01.03.2002 08:42
 */
public class ViewContainer extends Thread
{
    // ____________________

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

    // ____________________

    /**
     * ctor
     * It's private - because nobody should create any instance
     * expect the only global one, which wil be created by ourself!
     */
    private ViewContainer()
    {
        mlViews          = new Vector();
        mlListener       = new Vector();
        mbShutdownActive = false       ;
        Runtime.getRuntime().addShutdownHook(this);
    }

    // ____________________

    /**
     * This register a new view inside this global container
     * (if it doesn't already exist).
     *
     * @param   aView       view which whish to be registered inside this container
     */
    public void addView(Object aView)
    {
        synchronized(mlViews)
        {
            if(mlViews.contains(aView)==false)
                mlViews.add(aView);
        }
    }

    // ____________________

    /**
     * This deregister a view from this global container.
     * Normaly it should be the last reference to the view
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
            if(mlViews.contains(aView)==true)
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
            if (bShutdownView==true)
                mlListener.remove(aView);
        }
        if (bShutdownView==true)
            ((IShutdownListener)aView).shutdown();

        // We use a system.exit() to finish the whole application.
        // And further we have registered THIS instance as a possible shutdown
        // hook at the runtime class. So our run() method will be called.
        // Teh our view container should be empty - but
        // our listener container can include some references.
        // These objects which to be informed then and release e.g. some
        // remote references.
        if (nViewCount<1)
        {
            boolean bNeccessary = false;
            synchronized(this)
            {
                bNeccessary = ! mbShutdownActive;
            }
            if (bNeccessary==true)
            {
                System.out.println("call exit(0)!");
                System.exit(0);
            }
        }
    }

    // ____________________

    /**
     * add/remove listener for possibe shutdown events
     */
    public void addListener( IShutdownListener rListener )
    {
        synchronized(mlListener)
        {
            if ( ! mlListener.contains(rListener) )
                mlListener.add(rListener);
        }
    }

    // ____________________

    public void removeListener( IShutdownListener rListener )
    {
        synchronized(mlListener)
        {
            if ( mlListener.contains(rListener) )
                mlListener.remove(rListener);
        }
    }

    // ____________________

    /**
     * Is called from current runtime system of the java machine
     * on shutdown. We inform all current registered listener and
     * views. They should deinitialize her internal things then.
     */
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
                try{
                    aListener = (IShutdownListener)mlListener.firstElement();
                } catch(java.util.NoSuchElementException exEmpty) {}
            }
            if (aListener==null)
                break;

            aListener.shutdown();
            // May this listener has dergeistered himself.
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

    // ____________________

    /**
     * @const   BASICNAME   it's used to create uinque names for all regieterd views
     */
    private static final String BASICNAME = "Document View ";

    // ____________________

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
    private        Vector        mlViews             ;
    private        Vector        mlListener          ;
    private        boolean       mbShutdownActive    ;
}
