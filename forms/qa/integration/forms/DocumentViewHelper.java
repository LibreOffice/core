/*************************************************************************
 *
 *  $RCSfile: DocumentViewHelper.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2003-10-21 08:52:54 $
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
    public Object get( Class aInterfaceClass )
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
        XController xController = (XController)get( XController.class );
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
    public XDispatch getDispatcher( String sURL ) throws java.lang.Exception
    {
        URL[] aURL = new URL[] { new URL() };
        aURL[0].Complete = sURL;
        return getDispatcher( aURL );
    }

    /* ------------------------------------------------------------------ */
    /** retrieves a control within the current view of a document
        @param xModel
            specifies the control model which's control should be located
        @return
            the control tied to the model
    */
    public XControl getControl( XControlModel xModel ) throws com.sun.star.uno.Exception
    {
        // the current view of the document
        XControlAccess xCtrlAcc = (XControlAccess)get( XControlAccess.class );
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
        // get a dispatcher for the toggle URL
        URL[] aToggleURL = new URL[] { new URL() };
        aToggleURL[0].Complete = new String( ".uno:SwitchControlDesignMode" );
        XDispatch xDispatcher = getDispatcher( aToggleURL );

        // dispatch the URL - this will result in toggling the mode
        PropertyValue[] aDummyArgs = new PropertyValue[] { };
        xDispatcher.dispatch( aToggleURL[0], aDummyArgs );
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
        XIndexContainer xForms = dbfTools.queryXIndexContainer( m_document.getFormComponentTreeRoot( ) );
        // the first form
        XIndexContainer xForm = dbfTools.queryXIndexContainer( xForms.getByIndex( 0 ) );

        // the first control model which is no FixedText (FixedText's can't have the focus)
        for ( int i = 0; i<xForm.getCount(); ++i )
        {
            XPropertySet xControlProps = dbfTools.queryXPropertySet( xForm.getByIndex( i ) );
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

