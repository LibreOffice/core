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

package integration.forms;

import com.sun.star.uno.*;
import com.sun.star.util.*;
import com.sun.star.lang.*;
import com.sun.star.beans.*;
import integration.forms.DocumentHelper;
import integration.forms.NumericValidator;
import integration.forms.TextValidator;
import integration.forms.BooleanValidator;
import integration.forms.ListSelectionValidator;
import integration.forms.SingleControlValidation;

public class ControlValidation extends complexlib.ComplexTestCase implements com.sun.star.lang.XEventListener
{
    private DocumentHelper          m_document;         /// our current test document
    private XMultiServiceFactory    m_orb;              /// our service factory

    /** Creates a new instance of ControlValidation */
    public ControlValidation()
    {
    }

    public String[] getTestMethodNames()
    {
        return new String[] {
            "interactiveValidation"
        };
    }

    public String getTestObjectName()
    {
        return "Form Control Validation Test";
    }

    public static boolean isInteractiveTest()
    {
        return true;
    }

    /* ------------------------------------------------------------------ */
    /* test framework                                                     */
    /* ------------------------------------------------------------------ */
    public void before() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        m_orb = (XMultiServiceFactory)param.getMSF();
    }

    /* ------------------------------------------------------------------ */
    private void prepareTestStep( ) throws com.sun.star.uno.Exception, java.lang.Exception
    {
        m_document = DocumentHelper.blankTextDocument( m_orb );
        m_document.getDocument( ).addEventListener( this );
    }

    /* ------------------------------------------------------------------ */
    public void after()
    {
        closeDocument();
    }

    /* ------------------------------------------------------------------ */
    /** closes our document, if we have an open one
     */
    private void closeDocument()
    {
        try
        {
            // close our document
            if ( m_document != null )
            {
                XCloseable closeDoc = (XCloseable)UnoRuntime.queryInterface( XCloseable.class,
                    m_document.getDocument() );
                closeDoc.close( true );
            }
        }
        catch ( com.sun.star.uno.Exception e )
        {
            e.printStackTrace( System.err );
        }
    }

    /* ------------------------------------------------------------------ */
    /* public test methods                                                */
    /* ------------------------------------------------------------------ */
    public void interactiveValidation() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        prepareTestStep();

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
    /* internal methods                                                   */
    /* ------------------------------------------------------------------ */
    /** waits for the user to press a key (on the console where she started the java program)
            or the document to be closed by the user.
            @return
                    <TRUE/> if the user pressed a key on the console, <FALSE/> if she closed the document
    */
    protected boolean waitForUserInput() throws java.lang.Exception
    {
        synchronized (this)
        {
            integration.forms.WaitForInput aWait = new integration.forms.WaitForInput( this );
            aWait.start();
            wait();

            // if the waiter thread is done, the user pressed enter
            boolean bKeyPressed = aWait.isDone();
            if ( !bKeyPressed )
                aWait.interrupt();

            return bKeyPressed;
        }
    }

    /* ------------------------------------------------------------------ */
    /* XEventListener overridables                                        */
    /* ------------------------------------------------------------------ */
    public void disposing( com.sun.star.lang.EventObject eventObject )
    {
        if ( m_document.getDocument().equals( eventObject.Source ) )
    {
            // notify ourself that we can stop waiting for user input
            synchronized (this)
            {
                notify();
            }
        }
    }

}
