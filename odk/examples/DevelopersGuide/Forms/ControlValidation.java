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

import com.sun.star.beans.*;

public class ControlValidation extends DocumentBasedExample
{
    /** Creates a new instance of ControlValidation */
    public ControlValidation()
    {
        super( DocumentType.WRITER );
    }

    /* ------------------------------------------------------------------ */
    /* public test methods                                                */
    /* ------------------------------------------------------------------ */
    @Override
    protected void prepareDocument() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        super.prepareDocument();

        SingleControlValidation validation;
        XPropertySet focusField;

        validation = new SingleControlValidation( m_document, 5, 5, "DatabaseFormattedField", new NumericValidator() );
        focusField = validation.getInputField();
        validation.setExplanatoryText( "Please enter a number between 0 and 100, with at most 1 decimal digit" );

        validation = new SingleControlValidation( m_document, 90, 5, "DatabaseTextField", new TextValidator() );
        validation.setExplanatoryText( "Please enter a text whose length is a multiple of 3, and which does not contain the letter 'Z'" );

        validation = new SingleControlValidation( m_document, 5, 55, "DatabaseDateField", new DateValidator() );
        validation.setExplanatoryText( "Please enter a date in the current month" );
        validation.getInputField().setPropertyValue( "Dropdown", Boolean.TRUE );

        validation = new SingleControlValidation( m_document, 90, 55, "DatabaseTimeField", new TimeValidator() );
        validation.setExplanatoryText( "Please enter a time. Valid values are all full hours." );

        validation = new SingleControlValidation( m_document, 5, 110, "DatabaseCheckBox", new BooleanValidator( false ) );
        validation.setExplanatoryText( "Please check (well, or uncheck) the box. Don't leave it in indetermined state." );
        validation.getInputField().setPropertyValue( "TriState", Boolean.TRUE );

        validation = new SingleControlValidation( m_document, 90, 110, "DatabaseRadioButton", new BooleanValidator( true ), 3, 0 );
        validation.setExplanatoryText( "Please check any but the first button" );

        validation = new SingleControlValidation( m_document, 5, 165, "DatabaseListBox", new ListSelectionValidator( ), 1, 24 );
        validation.setExplanatoryText( "Please select not more than two entries." );
        validation.getInputField().setPropertyValue( "MultiSelection", Boolean.TRUE );
        validation.getInputField().setPropertyValue( "StringItemList", new String[] { "first", "second", "third", "forth", "fivth" } );

        // switch to alive mode
        m_document.getCurrentView( ).toggleFormDesignMode( );
        m_document.getCurrentView( ).grabControlFocus( focusField );

        // wait for the user telling us to exit
        waitForUserInput();
    }

    /* ------------------------------------------------------------------ */
    /** class entry point
    */
    public static void main(String argv[]) throws java.lang.Exception
    {
        ControlValidation aSample = new ControlValidation();
        aSample.run( argv );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
