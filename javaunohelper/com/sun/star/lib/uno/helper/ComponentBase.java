/*************************************************************************
 *
 *  $RCSfile: ComponentBase.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jl $ $Date: 2002-04-16 15:23:08 $
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
    public MultiTypeInterfaceContainer listenerContainer;
    private boolean bDisposed= false;
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
    protected void disposing()
    {
    }
    /** Override to become notified right before the disposing action is performed.
     */
    protected void inDispose()
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
            if ( bDisposed == false)
            {
                bDoDispose= true;
                bDisposed= true;
            }
        }
        // The notification occures in an unsynchronized block in order to avoid
        // deadlocks if one of the listeners calls back in a different thread on
        // a synchronized method which uses the same object.
        if (bDoDispose)
        {
            inDispose();
            listenerContainer.disposeAndClear(new EventObject(this));
            //notify subclasses that disposing is in progress
            disposing();
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
            if (bDisposed == true)
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
        if (bDisposed == false)
            dispose();
        super.finalize();
    }
}
