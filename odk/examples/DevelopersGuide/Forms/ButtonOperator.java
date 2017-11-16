/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
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
// java base stuff
import java.util.ArrayList;

import com.sun.star.awt.ActionEvent;
import com.sun.star.awt.XActionListener;
import com.sun.star.awt.XButton;
import com.sun.star.beans.XPropertySet;
import com.sun.star.form.runtime.FormOperations;
import com.sun.star.form.runtime.XFeatureInvalidation;
import com.sun.star.form.runtime.XFormOperations;
import com.sun.star.lang.EventObject;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;


/**************************************************************************/
/** a helper class for operating the buttons
*/
public class ButtonOperator implements XActionListener, XFeatureInvalidation
{
    private XComponentContext   m_componentContext;
    private DocumentHelper      m_aDocument;
    private XPropertySet        m_form;
    private XFormOperations     m_formOperations;

    private ArrayList<XPropertySet>              m_aButtons;

    /* ------------------------------------------------------------------ */
    /** ctor
    */
    public ButtonOperator( XComponentContext xCtx, DocumentHelper aDocument, XPropertySet _form )
    {
        m_componentContext = xCtx;
        m_aDocument = aDocument;
        m_form = _form;
        m_aButtons = new ArrayList<XPropertySet>();
    }

    /* ------------------------------------------------------------------ */
    private short getAssociatedFormFeature( XPropertySet _buttonModel )
    {
        short formFeature = -1;
        try
        {
            formFeature = Short.valueOf( (String)_buttonModel.getPropertyValue( "Tag" ) );
        }
        catch( com.sun.star.uno.Exception e )
        {
        }
        return formFeature;
    }

    /* ------------------------------------------------------------------ */
    /** gets the button which we operate and which is responsible for a given URL
    */
    private XPropertySet getButton( short _formFeature )
    {
        for ( int i=0; i < m_aButtons.size(); ++i )
        {
            XPropertySet button = m_aButtons.get( i );
            if ( _formFeature == getAssociatedFormFeature( button ) )
                return button;
        }
        return null;
    }

    /* ------------------------------------------------------------------ */
    /** announces a button which the operator should be responsible for
    */
    private int getButtonIndex( XPropertySet xButton )
    {
        int nPos = -1;
        for ( int i=0; ( i < m_aButtons.size() ) && ( -1 == nPos ); ++i )
        {
            if ( xButton.equals( m_aButtons.get( i ) ) )
                nPos = i;
        }
        return nPos;
    }

    /* ------------------------------------------------------------------ */
    /** announces a button which the operator should be responsible for
    */
    public void addButton( XPropertySet _buttonModel, short _formFeature  ) throws java.lang.Exception
    {
        // the current view to the document
        DocumentViewHelper aCurrentView = m_aDocument.getCurrentView();

        // add a listener so we get noticed if the user presses the button
        XButton xButtonControl = UnoRuntime.queryInterface( XButton.class,
            aCurrentView.getFormControl( _buttonModel ) );
        xButtonControl.addActionListener( this );

        _buttonModel.setPropertyValue( "Tag", String.valueOf( _formFeature ) );

        // remember the button
        m_aButtons.add( _buttonModel );
    }

    /* ------------------------------------------------------------------ */
    public void revokeButton( XPropertySet xButtonModel )
    {
        int nPos = getButtonIndex( xButtonModel );
        if ( -1 < nPos )
        {
            m_aButtons.remove( nPos );
        }
    }

    /* ==================================================================
       = XActionListener
       ================================================================== */
    /* ------------------------------------------------------------------ */
    /* called when a button has been pressed
    */
    public void actionPerformed( ActionEvent aEvent ) throws com.sun.star.uno.RuntimeException
    {
        // get the model's name
        XPropertySet buttonModel = FLTools.getModel( aEvent.Source, XPropertySet.class );
        try
        {
            short formFeature = getAssociatedFormFeature( buttonModel );
            if ( formFeature != -1 )
                m_formOperations.execute( formFeature );
        }
        catch( final com.sun.star.uno.Exception e )
        {
        }
    }

    /* ------------------------------------------------------------------ */
    /* (to be) called when the form layer has been switched to alive mode
     * @todo
     *  register as listener somewhere ...
    */
    public void onFormsAlive()
    {
        m_formOperations = FormOperations.createWithFormController(
            m_componentContext, m_aDocument.getCurrentView().getFormController( m_form ) );
        m_formOperations.setFeatureInvalidation( this );
        invalidateAllFeatures();
    }

    /* ==================================================================
       = XEventListener
       ================================================================== */
    public void disposing( EventObject aEvent )
    {
        // not interested in
    }

    /* ==================================================================
       = XFeatureInvalidation
       ================================================================== */
    private void updateButtonState( XPropertySet _buttonModel, short _formFeature )
    {
        try
        {
            _buttonModel.setPropertyValue( "Enabled", m_formOperations.isEnabled( _formFeature ) );
        }
        catch( com.sun.star.uno.Exception e )
        {
        }
    }

    public void invalidateFeatures( short[] _features ) throws com.sun.star.uno.RuntimeException
    {
        for ( int i=0; i<_features.length; ++i )
        {
            XPropertySet buttonModel = getButton( _features[i] );
            if ( buttonModel != null )
                updateButtonState( buttonModel, _features[i] );
        }
    }

    public void invalidateAllFeatures() throws com.sun.star.uno.RuntimeException
    {
        for ( XPropertySet buttonModel : m_aButtons )
        {
            updateButtonState( buttonModel, getAssociatedFormFeature( buttonModel ) );
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
