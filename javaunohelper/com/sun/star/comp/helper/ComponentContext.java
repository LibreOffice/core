/*************************************************************************
 *
 *  $RCSfile: ComponentContext.java,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-26 12:23:46 $
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
package com.sun.star.comp.helper;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.Any;

import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XSingleComponentFactory;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XEventListener;
import com.sun.star.lang.EventObject;

import java.util.Hashtable;
import java.util.Enumeration;
import java.util.Vector;


//==================================================================================================
class Disposer implements XEventListener
{
    private XComponent m_xComp;

    //----------------------------------------------------------------------------------------------
    Disposer( XComponent xComp )
    {
        m_xComp = xComp;
    }
    //______________________________________________________________________________________________
    public void disposing( EventObject Source )
    {
        m_xComp.dispose();
    }
}

/** Component context implementation.
*/
public class ComponentContext implements XComponentContext, XComponent
{
    private static final boolean DEBUG = false;
    private static final String SMGR_NAME = "/singletons/com.sun.star.lang.theServiceManager";
    private static final String TDMGR_NAME = "/singletons/com.sun.star.reflection.theTypeDescriptionManager";

    private Hashtable m_table;
    private XComponentContext m_xDelegate;

    private XMultiComponentFactory m_xSMgr;
    private boolean m_bDisposeSMgr;

    private Vector m_eventListener;

    /** Ctor to create a component context passing a hashtable for values and a delegator
        reference. Entries of the passed hashtable are either direct values or
        ComponentContextEntry objects.

        @param table
               entries
        @param xDelegate
               if values are not found, request is delegated to this object
    */
    public ComponentContext( Hashtable table, XComponentContext xDelegate )
    {
        m_eventListener = new Vector();
        m_table = table;
        m_xDelegate = xDelegate;
        m_xSMgr = null;
        m_bDisposeSMgr = false;

        Object o = table.get( SMGR_NAME );
        if (o != null)
        {
            if (o instanceof ComponentContextEntry)
            {
                o = ((ComponentContextEntry)o).m_value;
            }
            m_xSMgr = (XMultiComponentFactory)UnoRuntime.queryInterface(
                XMultiComponentFactory.class, o );
        }
        if (m_xSMgr != null)
        {
            m_bDisposeSMgr = true;
        }
        else if (m_xDelegate != null)
        {
            m_xSMgr = m_xDelegate.getServiceManager();
        }

        // listen for delegate
        XComponent xComp = (XComponent)UnoRuntime.queryInterface(
            XComponent.class, m_xDelegate );
        if (xComp != null)
        {
            xComp.addEventListener( new Disposer( this ) );
        }
    }

    // XComponentContext impl
    //______________________________________________________________________________________________
    public Object getValueByName( String rName )
    {
        Object o = m_table.get( rName );
        if (o != null)
        {
            if (o instanceof ComponentContextEntry)
            {
                ComponentContextEntry entry = (ComponentContextEntry)o;
                if (entry.m_lateInit != null)
                {
                    Object xInstance = null;

                    try
                    {
                        String serviceName = (String)entry.m_lateInit;
                        if (serviceName != null)
                        {
                            if (m_xSMgr != null)
                            {
                                xInstance = m_xSMgr.createInstanceWithContext( serviceName, this );
                            }
                            else
                            {
                                if (DEBUG)
                                    System.err.println( "### no service manager instance for late init of singleton instance \"" + rName + "\"!" );
                            }
                        }
                        else
                        {
                            XSingleComponentFactory xCompFac =
                                (XSingleComponentFactory)UnoRuntime.queryInterface(
                                    XSingleComponentFactory.class, entry.m_lateInit );
                            if (xCompFac != null)
                            {
                                xInstance = xCompFac.createInstanceWithContext( this );
                            }
                            else
                            {
                                if (DEBUG)
                                    System.err.println( "### neither service name nor service factory given for late init of singleton instance \"" + rName + "\"!" );
                            }
                        }
                    }
                    catch (com.sun.star.uno.Exception exc)
                    {
                        if (DEBUG)
                            System.err.println( "### exception occured on late init of singleton instance \"" + rName + "\": " + exc.getMessage() );
                    }

                    if (xInstance != null)
                    {
                        synchronized (entry)
                        {
                            if (entry.m_lateInit != null)
                            {
                                entry.m_value = xInstance;
                                entry.m_lateInit = null;
                            }
                            else // inited in the meantime
                            {
                                // dispose fresh service instance
                                XComponent xComp = (XComponent)UnoRuntime.queryInterface(
                                    XComponent.class, xInstance );
                                if (xComp != null)
                                {
                                    xComp.dispose();
                                }
                            }
                        }
                    }
                    else
                    {
                        if (DEBUG)
                            System.err.println( "### failed late init of singleton instance \"" + rName + "\"!" );
                    }
                }
                return entry.m_value;
            }
            else // direct value in map
            {
                return o;
            }
        }
        else if (m_xDelegate != null)
        {
            return m_xDelegate.getValueByName( rName );
        }
        else
        {
            return Any.VOID;
        }
    }
    //______________________________________________________________________________________________
    public XMultiComponentFactory getServiceManager()
    {
        return m_xSMgr;
    }

    // XComponent impl
    //______________________________________________________________________________________________
    public void dispose()
    {
        if (DEBUG)
            System.err.print( "> disposing context " + this );

        // fire events
        EventObject evt = new EventObject( this );
        Enumeration eventListener = m_eventListener.elements();
        while (eventListener.hasMoreElements())
        {
            XEventListener listener = (XEventListener)eventListener.nextElement();
            listener.disposing( evt );
        }
        m_eventListener.removeAllElements();

        XComponent tdmgr = null;
        // dispose values, then service manager, then typdescription manager
        Enumeration keys = m_table.keys();
        while (keys.hasMoreElements())
        {
            String name = (String)keys.nextElement();
            if (! name.equals( SMGR_NAME ))
            {
                Object o = m_table.get( name );
                if (o instanceof ComponentContextEntry)
                {
                    o = ((ComponentContextEntry)o).m_value;
                }

                XComponent xComp = (XComponent)UnoRuntime.queryInterface( XComponent.class, o );
                if (xComp != null)
                {
                    if (name.equals( TDMGR_NAME ))
                    {
                        tdmgr = xComp;
                    }
                    else
                    {
                        xComp.dispose();
                    }
                }
            }
        }
        m_table.clear();

        // smgr
        if (m_bDisposeSMgr)
        {
            XComponent xComp = (XComponent)UnoRuntime.queryInterface(
                XComponent.class, m_xSMgr );
            if (xComp != null)
            {
                xComp.dispose();
            }
        }
        m_xSMgr = null;

        // tdmgr
        if (tdmgr != null)
        {
            tdmgr.dispose();
        }

        if (DEBUG)
            System.err.println( "... finished" );
    }
    //______________________________________________________________________________________________
    public void addEventListener( XEventListener xListener )
    {
        if (xListener == null)
            throw new com.sun.star.uno.RuntimeException( "Listener must not be null" );
          if (m_eventListener.contains( xListener ))
              throw new com.sun.star.uno.RuntimeException( "Listener already registred." );

           m_eventListener.addElement( xListener );
    }
    //______________________________________________________________________________________________
    public void removeEventListener( XEventListener xListener )
    {
        if (xListener == null)
            throw new com.sun.star.uno.RuntimeException( "Listener must not be null" );
          if (! m_eventListener.contains( xListener ))
              throw new com.sun.star.uno.RuntimeException( "Listener is not registered." );

        m_eventListener.removeElement( xListener );
    }
}
