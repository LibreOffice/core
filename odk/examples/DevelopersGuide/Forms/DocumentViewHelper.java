/*************************************************************************
 *
 *  $RCSfile: DocumentViewHelper.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2003-06-30 15:27:36 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright (c) 2003 by Sun Microsystems, Inc.
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

import com.sun.star.uno.*;
import com.sun.star.frame.*;
import com.sun.star.lang.*;
import com.sun.star.util.*;
import com.sun.star.awt.*;
import com.sun.star.view.*;
import com.sun.star.beans.*;
import com.sun.star.container.*;
import com.sun.star.form.*;

/**************************************************************************/
/** provides a small wrapper around a document view
*/
class DocumentViewHelper
{
    private XController         m_xController;
    protected   XMultiServiceFactory    m_xMSF;
    protected   DocumentHelper      m_aDocument;

    /* ------------------------------------------------------------------ */
    public DocumentViewHelper( XMultiServiceFactory xMSF, DocumentHelper aDocument, XController xController )
    {
        m_xMSF = xMSF;
        m_aDocument = aDocument;
        m_xController = xController;
    }

    /* ------------------------------------------------------------------ */
    /** Quick access to a given interface of the view
        @param aInterfaceClass
            the class of the interface which shall be returned
    */
    public Object get( Class aInterfaceClass )
    {
        return UnoRuntime.queryInterface( aInterfaceClass, m_xController );
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
                XURLTransformer.class, m_xMSF.createInstance( "com.sun.star.util.URLTransformer" ) );
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
        XControlModel xModel = UNO.queryControlModel( aModel );
        return getControl( xModel );
    }

    /* ------------------------------------------------------------------ */
    public Object getControl( Object aModel, Class aInterfaceClass ) throws com.sun.star.uno.Exception
    {
        XControlModel xModel = UNO.queryControlModel( aModel );
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
        XIndexContainer xForms = UNO.queryIndexContainer( m_aDocument.getFormComponentTreeRoot( ) );
        // the first form
        XIndexContainer xForm = UNO.queryIndexContainer( xForms.getByIndex( 0 ) );

        // the first control model which is no FixedText (FixedText's can't have the focus)
        for ( int i = 0; i<xForm.getCount(); ++i )
        {
            XPropertySet xControlProps = UNO.queryPropertySet( xForm.getByIndex( i ) );
            if ( FormComponentType.FIXEDTEXT != ((Short)xControlProps.getPropertyValue( "ClassId" )).shortValue() )
            {
                XControlModel xControlModel = UNO.queryControlModel( xControlProps );
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

