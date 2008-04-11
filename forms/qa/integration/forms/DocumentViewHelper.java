/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DocumentViewHelper.java,v $
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
package integration.forms;

/**************************************************************************/
import com.sun.star.uno.*;
import com.sun.star.frame.*;
import com.sun.star.lang.*;
import com.sun.star.util.*;
import com.sun.star.awt.*;
import com.sun.star.view.*;
import com.sun.star.beans.*;
import com.sun.star.container.*;
import com.sun.star.form.*;

import integration.forms.DocumentHelper;

/**************************************************************************/
/** provides a small wrapper around a document view
*/
class DocumentViewHelper
{
    private     XMultiServiceFactory    m_orb;
    private     XController             m_controller;
    private     DocumentHelper      m_document;

    /* ------------------------------------------------------------------ */
    final protected XController getController()
    {
        return m_controller;
    }

    /* ------------------------------------------------------------------ */
    final protected DocumentHelper getDocument()
    {
        return m_document;
    }

    /* ------------------------------------------------------------------ */
    public DocumentViewHelper( XMultiServiceFactory orb, DocumentHelper document, XController controller )
    {
        m_orb = orb;
        m_document = document;
        m_controller = controller;
    }

    /* ------------------------------------------------------------------ */
    /** Quick access to a given interface of the view
        @param aInterfaceClass
                the class of the interface which shall be returned
    */
    public Object query( Class aInterfaceClass )
    {
        return UnoRuntime.queryInterface( aInterfaceClass, m_controller );
    }

    /* ------------------------------------------------------------------ */
    /** retrieves a dispatcher for the given URL, obtained at the current view of the document
        @param aURL
            a one-element array. The first element must contain a valid
            <member scope="com.sun.star.util">URL::Complete</member> value. Upon return, the URL is correctly
            parsed.
        @return
            the dispatcher for the URL in question
    */
    public XDispatch getDispatcher( URL[] aURL ) throws java.lang.Exception
    {
        XDispatch xReturn = null;

        // go get the current view
        XController xController = (XController)query( XController.class );
        // go get the dispatch provider of it's frame
        XDispatchProvider xProvider = (XDispatchProvider)UnoRuntime.queryInterface(
            XDispatchProvider.class, xController.getFrame() );
        if ( null != xProvider )
        {
            // need an URLTransformer
            XURLTransformer xTransformer = (XURLTransformer)UnoRuntime.queryInterface(
                    XURLTransformer.class, m_orb.createInstance( "com.sun.star.util.URLTransformer" ) );
            xTransformer.parseStrict( aURL );

            xReturn = xProvider.queryDispatch( aURL[0], new String( ), 0 );
        }
        return xReturn;
    }

    /* ------------------------------------------------------------------ */
    /** retrieves a dispatcher for the given URL, obtained at the current view of the document
    */
    public XDispatch getDispatcher( String url ) throws java.lang.Exception
    {
        URL[] aURL = new URL[] { new URL() };
        aURL[0].Complete = url;
        return getDispatcher( aURL );
    }

    /* ------------------------------------------------------------------ */
    /** dispatches the given URL into the view, if there's a dispatcher for it

        @return
            <TRUE/> if the URL was successfully dispatched
    */
    public boolean dispatch( String url ) throws java.lang.Exception
    {
        URL[] completeURL = new URL[] { new URL() };
        completeURL[0].Complete = url;
        XDispatch dispatcher = getDispatcher( completeURL );
        if ( dispatcher == null )
            return false;

        PropertyValue[] aDummyArgs = new PropertyValue[] { };
        dispatcher.dispatch( completeURL[0], aDummyArgs );
        return true;
    }

    /* ------------------------------------------------------------------ */
    /** retrieves a control within the current view of a document
        @param xModel
            specifies the control model whose control should be located
        @return
            the control tied to the model
    */
    public XControl getControl( XControlModel xModel ) throws com.sun.star.uno.Exception
    {
        // the current view of the document
        XControlAccess xCtrlAcc = (XControlAccess)query( XControlAccess.class );
        // delegate the task of looking for the control
        return xCtrlAcc.getControl( xModel );
    }

    /* ------------------------------------------------------------------ */
    public XControl getControl( Object aModel ) throws com.sun.star.uno.Exception
    {
        XControlModel xModel = (XControlModel)UnoRuntime.queryInterface( XControlModel.class, aModel );
        return getControl( xModel );
    }

    /* ------------------------------------------------------------------ */
    public Object getControl( Object aModel, Class aInterfaceClass ) throws com.sun.star.uno.Exception
    {
        XControlModel xModel = (XControlModel)UnoRuntime.queryInterface( XControlModel.class, aModel );
        return UnoRuntime.queryInterface( aInterfaceClass, getControl( xModel ) );
    }

    /* ------------------------------------------------------------------ */
    /** toggles the design mode of the form layer of active view of our sample document
    */
    protected void toggleFormDesignMode( ) throws java.lang.Exception
    {
        dispatch( ".uno:SwitchControlDesignMode" );
    }

    /* ------------------------------------------------------------------ */
    /** sets the focus to a specific control
        @param xModel
            a control model. The focus is set to that control which is part of our view
            and associated with the given model.
    */
    public void grabControlFocus( Object xModel ) throws com.sun.star.uno.Exception
    {
        // look for the control from the current view which belongs to the model
        XControl xControl = getControl( xModel );

        // the focus can be set to an XWindow only
        XWindow xControlWindow = (XWindow)UnoRuntime.queryInterface( XWindow.class,
            xControl );

        // grab the focus
        xControlWindow.setFocus();
    }

    /* ------------------------------------------------------------------ */
    /** sets the focus to the first control
    */
    protected void grabControlFocus( ) throws java.lang.Exception
    {
        // the forms container of our document
        XIndexContainer xForms = dbfTools.queryIndexContainer( m_document.getFormComponentTreeRoot( ) );
        // the first form
        XIndexContainer xForm = dbfTools.queryIndexContainer( xForms.getByIndex( 0 ) );

        // the first control model which is no FixedText (FixedText's can't have the focus)
        for ( int i = 0; i<xForm.getCount(); ++i )
        {
            XPropertySet xControlProps = dbfTools.queryPropertySet( xForm.getByIndex( i ) );
            if ( FormComponentType.FIXEDTEXT != ((Short)xControlProps.getPropertyValue( "ClassId" )).shortValue() )
            {
                XControlModel xControlModel = (XControlModel)UnoRuntime.queryInterface(
                    XControlModel.class, xControlProps );
                // set the focus to this control
                grabControlFocus( xControlModel );
                // outta here
                break;
            }
        }

        // Note that we simply took the first control model from the hierarchy. This does state nothing
        // about the location of the respective control in the view. A control model is tied to a control
        // shape, and the shapes are where the geometry information such as position and size is hung up.
        // So you could easily have a document where the first control model is bound to a shape which
        // has a greater ordinate than any other control model.
    }
};

