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

import com.sun.star.uno.*;
import com.sun.star.util.*;
import com.sun.star.lang.*;
import com.sun.star.accessibility.*;
import com.sun.star.container.*;
import com.sun.star.beans.*;
import com.sun.star.form.binding.*;

/**
 *
 * @author  fs@openoffice.org
 */
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
        validation.getInputField().setPropertyValue( "Dropdown", new Boolean( true ) );

        validation = new SingleControlValidation( m_document, 90, 55, "DatabaseTimeField", new TimeValidator() );
        validation.setExplanatoryText( "Please enter a time. Valid values are all full hours." );

        validation = new SingleControlValidation( m_document, 5, 110, "DatabaseCheckBox", new BooleanValidator( false ) );
        validation.setExplanatoryText( "Please check (well, or uncheck) the box. Don't leave it in indetermined state." );
        validation.getInputField().setPropertyValue( "TriState", new Boolean( true ) );

        validation = new SingleControlValidation( m_document, 90, 110, "DatabaseRadioButton", new BooleanValidator( true ), 3, 0 );
        validation.setExplanatoryText( "Please check any but the first button" );

        validation = new SingleControlValidation( m_document, 5, 165, "DatabaseListBox", new ListSelectionValidator( ), 1, 24 );
        validation.setExplanatoryText( "Please select not more than two entries." );
        validation.getInputField().setPropertyValue( "MultiSelection", new Boolean( true ) );
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
