/*************************************************************************
 *
 *  $RCSfile: ViewContainer.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2003-06-10 10:31:38 $
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

package OfficeDev.samples.DesktopEnvironment;

// __________ Imports __________

// interfaces

// helper
import com.sun.star.uno.UnoRuntime;

// UI classes

// events

// base classes
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
     * Neccessary for terminate(9 functionality to be able
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
     * (if it doesnt already exist).
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
        // It's neccessary to guarantee his dead ...
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
        // These objects wich to be informed then and release e.g. some
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
     *                              We supress it by using this variable!
     */
    public  static boolean       mbInplace   = false ;
    private static ViewContainer maSingleton = null  ;
    private        Vector        mlViews             ;
    private        Vector        mlListener          ;
    private        boolean       mbShutdownActive    ;
}
