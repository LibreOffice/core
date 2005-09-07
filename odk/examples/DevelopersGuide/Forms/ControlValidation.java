/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ControlValidation.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:06:35 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
