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

package integration.extensions;

import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.lang.XMultiServiceFactory;

import com.sun.star.frame.*;
import com.sun.star.inspection.*;
import com.sun.star.beans.*;

import integration.extensions.Frame;

public class ObjectInspector extends complexlib.ComplexTestCase
{
    private XComponentContext       m_context;
    private XMultiServiceFactory    m_orb;
    private Frame                   m_desktop;

    final private String    m_inspectorFrameName = new String( "ObjectInspector" );

    /** Creates a new instance of ValueBinding */
    public ObjectInspector()
    {
    }

    /* ------------------------------------------------------------------ */
    public String[] getTestMethodNames()
    {
        return new String[] {
            "interactiveObjectInspector"
        };
    }

    /* ------------------------------------------------------------------ */
    public String getTestObjectName()
    {
        return "Test Skeleton";
    }

    /* ------------------------------------------------------------------ */
    public void before() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        m_orb = (XMultiServiceFactory)param.getMSF();
        m_context = UnoRuntime.queryInterface( XComponentContext.class,
                UnoRuntime.queryInterface( XPropertySet.class, m_orb ).getPropertyValue( "DefaultContext" ) );
        m_desktop = new Frame( m_orb.createInstance( "com.sun.star.frame.Desktop" ) );
    }

    /* ------------------------------------------------------------------ */
    public void after() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        closeExistentInspector();
    }

    /* ------------------------------------------------------------------ */
    public void interactiveObjectInspector() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        closeExistentInspector();

        // the to-be-inspected object
        XFrame inspectee = m_desktop.getActiveFrame();

        // the inspector
        XObjectInspector inspector = createObjectInspector();

        // do inspect
        inspector.inspect( new Object[] { inspectee } );

        ConsoleWait keyWaiter = new ConsoleWait( inspector );
        keyWaiter.waitForUserInput();
    }

    /* ------------------------------------------------------------------ */
    private XObjectInspector createObjectInspector() throws com.sun.star.uno.Exception
    {
        com.sun.star.awt.XWindow floatingWindow = createFloatingWindow();

        Frame inspectorFrame = new Frame( m_orb.createInstance( "com.sun.star.frame.Frame" ) );
        inspectorFrame.setName( m_inspectorFrameName );
        inspectorFrame.initialize( floatingWindow );
        m_desktop.getFrames().append( inspectorFrame.getXFrame() );

        // handler factories:
        Object[] handlerFactories = new Object[] {
            "com.sun.star.inspection.GenericPropertyHandler",
            new ComponentFactory( ServicesHandler.class ),
            new ComponentFactory( MethodHandler.class )
        };
        // a model
        XObjectInspectorModel model = ObjectInspectorModel.createWithHandlerFactoriesAndHelpSection(
            m_context, handlerFactories, 4, 4 );

    // create the ObjectInspector
        XObjectInspector inspector = com.sun.star.inspection.ObjectInspector.createWithModel(
            m_context, model );

        // add an observer which will emit help texts
        new HelpTextProvider( inspector.getInspectorUI() );

        // plug it into the frame
        inspector.attachFrame( inspectorFrame.getXFrame() );

        // make the window visible
    floatingWindow.setVisible( true );

        // outta here
        return inspector;
    }

    /* ------------------------------------------------------------------ */
    private void closeExistentInspector()
    {
        Frame existentInspectorFrame = new Frame( m_desktop.findFrame( m_inspectorFrameName, 255 ) );
        if ( existentInspectorFrame != null )
        {
            try
            {
                existentInspectorFrame.close( true );
            }
            catch( com.sun.star.util.CloseVetoException e )
            {
                failed( "could not close the existent inspector frame" );
            }
        }
    }

    /* ------------------------------------------------------------------ */
    private com.sun.star.awt.XWindow createFloatingWindow() throws com.sun.star.uno.Exception
    {
        com.sun.star.awt.XToolkit toolkit = UnoRuntime.queryInterface(
                com.sun.star.awt.XToolkit.class, m_orb.createInstance( "com.sun.star.awt.Toolkit" ) );

        com.sun.star.awt.WindowDescriptor windowDescriptor = new com.sun.star.awt.WindowDescriptor();
        windowDescriptor.Type = com.sun.star.awt.WindowClass.TOP;
        windowDescriptor.WindowServiceName = "modelessdialog";  // "floatingwindow" would need a parent
        windowDescriptor.ParentIndex       =  -1;
        //windowDescriptor.Parent           =  null;

        windowDescriptor.Bounds              = new com.sun.star.awt.Rectangle( 500, 100,  400, 600 );
        windowDescriptor.WindowAttributes    =  com.sun.star.awt.WindowAttribute.BORDER
                                            +   com.sun.star.awt.WindowAttribute.MOVEABLE
                                            +   com.sun.star.awt.WindowAttribute.SIZEABLE
                                            +   com.sun.star.awt.WindowAttribute.CLOSEABLE
                                            +   com.sun.star.awt.VclWindowPeerAttribute.CLIPCHILDREN;

    return UnoRuntime.queryInterface( com.sun.star.awt.XWindow.class,
                toolkit.createWindow( windowDescriptor ) );
    }
}
