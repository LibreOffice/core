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
package com.sun.star.comp.helper;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.Any;

import com.sun.star.uno.DeploymentException;
import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XSingleComponentFactory;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XEventListener;
import com.sun.star.lang.EventObject;

import java.util.Hashtable;
import java.util.Iterator;
import java.util.ArrayList;


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

    private Hashtable<String,Object> m_table;
    private XComponentContext m_xDelegate;

    private XMultiComponentFactory m_xSMgr;
    private boolean m_bDisposeSMgr;

    private ArrayList<XEventListener> m_eventListener;

    /** Ctor to create a component context passing a hashtable for values and a delegator
        reference. Entries of the passed hashtable are either direct values or
        ComponentContextEntry objects.

        @param table
               entries
        @param xDelegate
               if values are not found, request is delegated to this object
    */
    public ComponentContext( Hashtable<String,Object> table, XComponentContext xDelegate )
    {
        m_eventListener = new ArrayList<XEventListener>();
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
            m_xSMgr = UnoRuntime.queryInterface(
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
        XComponent xComp = UnoRuntime.queryInterface(
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
                                UnoRuntime.queryInterface(
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
                            System.err.println( "### exception occurred on late init of singleton instance \"" + rName + "\": " + exc.getMessage() );
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
                                XComponent xComp = UnoRuntime.queryInterface(
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
        if (m_xSMgr == null)
        {
            throw new DeploymentException(
                "null component context service manager" );
        }
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
        Iterator<XEventListener> eventListener = m_eventListener.iterator();
        while (eventListener.hasNext())
        {
            XEventListener listener = eventListener.next();
            listener.disposing( evt );
        }
        m_eventListener.clear();

        XComponent tdmgr = null;
        // dispose values, then service manager, then typdescription manager
        Iterator<String> keys = m_table.keySet().iterator();
        while (keys.hasNext())
        {
            String name = keys.next();
            if (! name.equals( SMGR_NAME ))
            {
                Object o = m_table.get( name );
                if (o instanceof ComponentContextEntry)
                {
                    o = ((ComponentContextEntry)o).m_value;
                }

                XComponent xComp = UnoRuntime.queryInterface( XComponent.class, o );
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
            XComponent xComp = UnoRuntime.queryInterface(
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

           m_eventListener.add( xListener );
    }
    //______________________________________________________________________________________________
    public void removeEventListener( XEventListener xListener )
    {
        if (xListener == null)
            throw new com.sun.star.uno.RuntimeException( "Listener must not be null" );
          if (! m_eventListener.contains( xListener ))
              throw new com.sun.star.uno.RuntimeException( "Listener is not registered." );

        m_eventListener.remove( xListener );
    }
}
