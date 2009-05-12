/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ObjectInspector.java,v $
 * $Revision: 1.4 $
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
        m_context = (XComponentContext)UnoRuntime.queryInterface( XComponentContext.class,
                ((XPropertySet)UnoRuntime.queryInterface( XPropertySet.class, m_orb )).getPropertyValue( "DefaultContext" ) );
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
        com.sun.star.awt.XToolkit toolkit = (com.sun.star.awt.XToolkit)UnoRuntime.queryInterface(
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

    return (com.sun.star.awt.XWindow)UnoRuntime.queryInterface( com.sun.star.awt.XWindow.class,
                toolkit.createWindow( windowDescriptor ) );
    }
}
