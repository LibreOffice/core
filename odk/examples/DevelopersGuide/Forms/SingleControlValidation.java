/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

import com.sun.star.uno.*;
import com.sun.star.beans.*;
import com.sun.star.form.validation.*;

public class SingleControlValidation implements XFormComponentValidityListener
{
    private DocumentHelper          m_document;         /// our current test document
    private FormLayer               m_formLayer;        /// quick access to the form layer

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
            m_inputLabel.setPropertyValue( "MultiLine", Boolean.TRUE );

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
                    controls[ i ].setPropertyValue( "Border", Short.valueOf( (short)2 ) );

                XValidatableFormComponent xComp = UnoRuntime.queryInterface( XValidatableFormComponent.class,
                    controls[ i ] );
                xComp.addFormComponentValidityListener( this );
            }
            m_inputField = controls[ 0 ];


            controlPos += 4;
            XPropertySet xLabel = m_formLayer.createControlAndShape( "FixedText", columnPos, controlPos, 70, 4, null );
            xLabel.setPropertyValue( "Label", "Status:" );
            controlPos += 4;
            m_statusField = m_formLayer.createControlAndShape( "FixedText", columnPos, controlPos, 70, 4, null );
            m_statusField.setPropertyValue( "Label", "" );


            controlPos += 6;
            xLabel = m_formLayer.createControlAndShape( "FixedText", columnPos, controlPos, 70, 4, null );
            xLabel.setPropertyValue( "Label", "Explanation for invalidity:" );
            controlPos += 4;
            m_explanationField = m_formLayer.createControlAndShape( "FixedText", columnPos, controlPos, 70, 4, null );
            m_explanationField.setPropertyValue( "Label", "" );

            XValidatable xValidatable = UnoRuntime.queryInterface( XValidatable.class, m_inputField );
            xValidatable.setValidator( m_validator );
        }
        catch( java.lang.Exception e  )
        {
            e.printStackTrace( System.err );
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
                XValidatableFormComponent xComp = UnoRuntime.queryInterface( XValidatableFormComponent.class,
                    eventObject.Source );
                // the current value
                Object value = xComp.getCurrentValue();

                // the current validity flag
                boolean isValid = xComp.isValid();

                m_statusField.setPropertyValue("Label", isValid ? "valid" : "invalid" );
                m_statusField.setPropertyValue( "TextColor", Integer.valueOf( isValid ? 0x008000 : 0x800000 ) );

                String validityMessage = "";
                if ( !isValid )
                    validityMessage = m_validator.explainInvalid( value );
                m_explanationField.setPropertyValue( "Label", validityMessage );
            }
        }
        catch( com.sun.star.uno.Exception e )
        {
            e.printStackTrace( System.err );
        }
    }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
