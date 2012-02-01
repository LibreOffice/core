/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


// java base stuff
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
import java.util.Vector;


/**************************************************************************/
/** a helper class for operating the buttons
*/
public class ButtonOperator implements XActionListener, XFeatureInvalidation
{
    private XComponentContext   m_componentContext;
    private DocumentHelper      m_aDocument;
    private XPropertySet        m_form;
    private XFormOperations     m_formOperations;

    private Vector              m_aButtons;

    /* ------------------------------------------------------------------ */
    /** ctor
    */
    public ButtonOperator( XComponentContext xCtx, DocumentHelper aDocument, XPropertySet _form )
    {
        m_componentContext = xCtx;
        m_aDocument = aDocument;
        m_form = _form;
        m_aButtons = new Vector();
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
    /** get's the button which we operate and which is responsible for a given URL
    */
    private XPropertySet getButton( short _formFeature )
    {
        for ( int i=0; i < m_aButtons.size(); ++i )
        {
            XPropertySet button = (XPropertySet)m_aButtons.elementAt( i );
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
            if ( xButton.equals( m_aButtons.elementAt( i ) ) )
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
        XButton xButtonControl = (XButton)UnoRuntime.queryInterface( XButton.class,
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
        XPropertySet buttonModel = (XPropertySet)FLTools.getModel( aEvent.Source, XPropertySet.class );
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
        try
        {
            m_formOperations = FormOperations.createWithFormController(
                m_componentContext, m_aDocument.getCurrentView().getFormController( m_form ) );
            m_formOperations.setFeatureInvalidation( this );
            invalidateAllFeatures();
        }
        catch( final com.sun.star.uno.Exception e )
        {
        }
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
        for ( int i=0; i < m_aButtons.size(); ++i )
        {
            XPropertySet buttonModel = (XPropertySet)m_aButtons.elementAt( i );
            updateButtonState( buttonModel, getAssociatedFormFeature( buttonModel ) );
        }
    }
};

