/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

import com.sun.star.uno.*;
import com.sun.star.util.*;
import com.sun.star.lang.*;
import com.sun.star.accessibility.*;
import com.sun.star.container.*;
import com.sun.star.beans.*;
import com.sun.star.form.binding.*;
import com.sun.star.form.validation.*;

import integration.forms.DocumentHelper;

public class SingleControlValidation implements XFormComponentValidityListener
{
    private DocumentHelper          m_document;         /// our current test document
    private FormLayer               m_formLayer;        /// quick access to the form layer
    private XMultiServiceFactory    m_orb;              /// our service factory

    private XPropertySet            m_inputField;
    private XPropertySet            m_inputLabel;
    private XPropertySet            m_statusField;
    private XPropertySet            m_explanationField;
    private XValidator              m_validator;

    /* ------------------------------------------------------------------ */
    public SingleControlValidation( DocumentHelper document, int columnPos, int rowPos, String formComponentService, XValidator validator )
    {
        m_document = document;
        m_validator = validator;
        m_formLayer = new FormLayer( m_document );
        createControls( columnPos, rowPos, formComponentService, 1, 0 );
    }

    /* ------------------------------------------------------------------ */
    public SingleControlValidation( DocumentHelper document, int columnPos, int rowPos, String formComponentService, XValidator validator, int controlCount, int controlHeight )
    {
        m_document = document;
        m_validator = validator;
        m_formLayer = new FormLayer( m_document );
        createControls( columnPos, rowPos, formComponentService, controlCount, controlHeight );
    }

    /* ------------------------------------------------------------------ */
    public XPropertySet getInputField()
    {
        return m_inputField;
    }

    /* ------------------------------------------------------------------ */
    public void setExplanatoryText( String text )
    {
        try
        {
            m_inputLabel.setPropertyValue( "Label", text );
        }
        catch( com.sun.star.uno.Exception e )
        {
            e.printStackTrace( System.err );
        }
    }

    /* ------------------------------------------------------------------ */
    private void createControls( int columnPos, int rowPos, String formComponentService, int controlCount, int controlHeight )
    {
        try
        {
            m_inputLabel = m_formLayer.createControlAndShape( "FixedText", columnPos, rowPos, 70, 12, null );
            m_inputLabel.setPropertyValue( "MultiLine", new Boolean( true ) );

            com.sun.star.awt.FontDescriptor font = (com.sun.star.awt.FontDescriptor)m_inputLabel.getPropertyValue( "FontDescriptor" );
            font.Weight = com.sun.star.awt.FontWeight.BOLD;
            m_inputLabel.setPropertyValue( "FontDescriptor", font );

            if ( controlHeight == 0 )
                controlHeight = 6;

            int controlPos = rowPos + 12;
            XPropertySet[] controls = new XPropertySet[ controlCount ];
            for ( int i = 0; i < controlCount; ++i, controlPos += controlHeight )
            {
                controls[ i ] = m_formLayer.createControlAndShape( formComponentService, columnPos, controlPos, 25, controlHeight, null );
                controls[ i ].setPropertyValue( "Name", formComponentService );
                controls[ i ].setPropertyValue( "Tag", String.valueOf( i ) );

                if ( controls[ i ].getPropertySetInfo().hasPropertyByName( "Border" ) )
                    controls[ i ].setPropertyValue( "Border", new Short( (short)2 ) );

                XValidatableFormComponent xComp = (XValidatableFormComponent)UnoRuntime.queryInterface( XValidatableFormComponent.class,
                    controls[ i ] );
                xComp.addFormComponentValidityListener( this );
            }
            m_inputField = controls[ 0 ];

            // ----------------------------------
            controlPos += 4;
            XPropertySet xLabel = m_formLayer.createControlAndShape( "FixedText", columnPos, controlPos, 70, 4, null );
            xLabel.setPropertyValue( "Label", new String( "Status:" ) );
            controlPos += 4;
            m_statusField = m_formLayer.createControlAndShape( "FixedText", columnPos, controlPos, 70, 4, null );
            m_statusField.setPropertyValue( "Label", new String( "" ) );

            // ----------------------------------
            controlPos += 6;
            xLabel = m_formLayer.createControlAndShape( "FixedText", columnPos, controlPos, 70, 4, null );
            xLabel.setPropertyValue( "Label", new String( "Explanation for invalidity:" ) );
            controlPos += 4;
            m_explanationField = m_formLayer.createControlAndShape( "FixedText", columnPos, controlPos, 70, 4, null );
            m_explanationField.setPropertyValue( "Label", new String( "" ) );

            XValidatable xValidatable = (XValidatable)UnoRuntime.queryInterface( XValidatable.class, m_inputField );
            xValidatable.setValidator( m_validator );
        }
        catch( java.lang.Exception e  )
        {
            e.printStackTrace( System.out );
        }
    }

    /* ------------------------------------------------------------------ */
    /* XEventListener overridables                                        */
    /* ------------------------------------------------------------------ */
    public void disposing( com.sun.star.lang.EventObject eventObject )
    {
        // not interested in
    }

    /* ------------------------------------------------------------------ */
    /* XFormComponentValidityListener overridables                        */
    /* ------------------------------------------------------------------ */
    public void componentValidityChanged( com.sun.star.lang.EventObject eventObject )
    {
        try
        {
            if ( m_inputField.equals( eventObject.Source ) )
            {
                XValidatableFormComponent xComp = (XValidatableFormComponent)UnoRuntime.queryInterface( XValidatableFormComponent.class,
                    eventObject.Source );
                // the current value
                Object value = xComp.getCurrentValue();

                // the current validity flag
                boolean isValid = xComp.isValid();

                m_statusField.setPropertyValue("Label", isValid ? "valid" : "invalid" );
                m_statusField.setPropertyValue( "TextColor", new Integer( isValid ? 0x008000 : 0x800000 ) );

                String validityMessage = "";
                if ( !isValid )
                    validityMessage = m_validator.explainInvalid( value );
                m_explanationField.setPropertyValue( "Label", validityMessage );
            }
        }
        catch( com.sun.star.uno.Exception e )
        {
            e.printStackTrace( System.out );
        }
    }

}
