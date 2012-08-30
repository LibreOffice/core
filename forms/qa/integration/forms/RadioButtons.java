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
import com.sun.star.container.*;
import com.sun.star.beans.*;
import com.sun.star.awt.XRadioButton;

import integration.forms.dbfTools;
import integration.forms.DocumentHelper;
import integration.forms.SpreadsheetDocument;

public class RadioButtons extends complexlib.ComplexTestCase
{
    private DocumentHelper          m_document;         /// our current test document
    private FormLayer               m_formLayer;        /// quick access to the form layer
    private XMultiServiceFactory    m_orb;              /// our service factory
    private XPropertySet            m_primaryForm;      /// the primary form, to be used in text documents and in the first page of spreadsheets
    private XPropertySet            m_secondaryForm;    /// the secondary form, to be used in the second page of spreadsheets

    /* ------------------------------------------------------------------ */
    public RadioButtons()
    {
    }

    /* ------------------------------------------------------------------ */
    public String[] getTestMethodNames()
    {
        return new String[] {
            "checkSingleButtons",
            "checkThreeGroups",
            "checkMultipleForms",
            "checkCalcPageSwitch"
        };
    }

    /* ------------------------------------------------------------------ */
    public String getTestObjectName()
    {
        return "Form Radio Buttons Test";
    }

    /* ------------------------------------------------------------------ */
    public void before() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        m_orb = (XMultiServiceFactory)param.getMSF();
    }

    /* ------------------------------------------------------------------ */
    private XPropertySet insertRadio( int nXPos, int nYPos, String label, String name, String refValue ) throws com.sun.star.uno.Exception, java.lang.Exception
    {
        return insertRadio( nXPos, nYPos, label, name, refValue, null );
    }

    /* ------------------------------------------------------------------ */
    private XPropertySet insertRadio( int nXPos, int nYPos, String label, String name, String refValue, XPropertySet parentForm ) throws com.sun.star.uno.Exception, java.lang.Exception
    {
        XIndexContainer parentContainer = dbfTools.queryIndexContainer( parentForm );
        XPropertySet xRadio = m_formLayer.createControlAndShape( "DatabaseRadioButton", nXPos, nYPos, 25, 6, parentContainer );
        xRadio.setPropertyValue( "Label", label );
        xRadio.setPropertyValue( "RefValue", refValue );
        xRadio.setPropertyValue( "Name", name );

        if ( null == m_primaryForm )
            m_primaryForm = (XPropertySet)dbfTools.getParent( xRadio, XPropertySet.class );

        return xRadio;
    }

    /* ------------------------------------------------------------------ */
    /** this checks whether n groups of radio buttons, consisting of only one button each,
     *  behave properly
     */
    public void checkSingleButtons() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        prepareTestStep( false );

        insertRadio( 20, 30,  "group 1", "group 1", "" );
        insertRadio( 20, 38,  "group 2", "group 2", "" );
        insertRadio( 20, 46,  "group 3", "group 3", "" );
        insertRadio( 20, 54,  "group 4", "group 4", "" );

        // switch to alive mode
        m_document.getCurrentView( ).toggleFormDesignMode( );

        checkRadio( "group 1", "" );
        verifySingleRadios( 1, 0, 0, 0 );

        checkRadio( "group 4", "" );
        verifySingleRadios( 1, 0, 0, 1 );

        checkRadio( "group 2", "" );
        verifySingleRadios( 1, 1, 0, 1 );

        checkRadio( "group 3", "" );
        verifySingleRadios( 1, 1, 1, 1 );

        cleanupTestStep();
    }

    /* ------------------------------------------------------------------ */
    /** creates three groups of radio buttons in a sample document, and checks whether they're working
     */
    public void checkThreeGroups( ) throws com.sun.star.uno.Exception, java.lang.Exception
    {
        prepareTestStep( false );

        insertRadio( 20, 30,  "group 1 (a)", "group 1", "a" );
        insertRadio( 20, 38,  "group 1 (b)", "group 1", "b" );

        insertRadio( 20, 50,  "group 2 (a)", "group 2", "a" );
        insertRadio( 20, 58,  "group 2 (b)", "group 2", "b" );

        insertRadio( 20, 70,  "group 3 (a)", "group 3", "a" );
        insertRadio( 20, 78,  "group 3 (b)", "group 3", "b" );

        // switch to alive mode
        m_document.getCurrentView( ).toggleFormDesignMode( );

        // initially, after switching to alive mode, all buttons should be unchecked
        verifySixPack( 0, 0, 0, 0, 0, 0 );

        // check one button in every group
        checkRadio( "group 1", "a" );
        checkRadio( "group 2", "b" );
        checkRadio( "group 3", "a" );
        // and verify that this worked
        verifySixPack( 1, 0, 0, 1, 1, 0 );

        // check all buttons which are currently unchecked
        checkRadio( "group 1", "b" );
        checkRadio( "group 2", "a" );
        checkRadio( "group 3", "b" );
        // this should have reset the previous checks in the respective groups
        verifySixPack( 0, 1, 1, 0, 0, 1 );

        // and back to the previous check state
        checkRadio( "group 1", "a" );
        checkRadio( "group 2", "b" );
        checkRadio( "group 3", "a" );
        verifySixPack( 1, 0, 0, 1, 1, 0 );

        cleanupTestStep();
    }

    /* ------------------------------------------------------------------ */
    /** tests whether radio buttons which belong to different forms behave properly
     */
    public void checkMultipleForms( ) throws com.sun.star.uno.Exception, java.lang.Exception
    {
        prepareTestStep( false );

        insertRadio( 20, 30,  "group 1 (a)", "group 1", "a" );
        insertRadio( 20, 38,  "group 1 (b)", "group 1", "b" );
        insertRadio( 20, 46,  "group 1 (c)", "group 1", "c" );

        m_secondaryForm = dbfTools.queryPropertySet( m_document.createSiblingForm( m_primaryForm, "secondary" ) );

        insertRadio( 70, 30,  "group 2 (a)", "group 2", "a", m_secondaryForm );
        insertRadio( 70, 38,  "group 2 (b)", "group 2", "b", m_secondaryForm );
        insertRadio( 70, 46,  "group 2 (c)", "group 2", "c", m_secondaryForm );

        // switch to alive mode
        m_document.getCurrentView( ).toggleFormDesignMode( );

        // play around with different check states
        checkRadio( "group 1", "b", m_primaryForm );
        checkRadio( "group 2", "c", m_secondaryForm );
        verifyTwoFormRadios( 0, 1, 0, 0, 0, 1 );

        checkRadio( "group 1", "c", m_primaryForm );
        verifyTwoFormRadios( 0, 0, 1, 0, 0, 1 );

        checkRadio( "group 2", "a", m_secondaryForm );
        verifyTwoFormRadios( 0, 0, 1, 1, 0, 0 );

        checkRadio( "group 1", "a", m_primaryForm );
        verifyTwoFormRadios( 1, 0, 0, 1, 0, 0 );

        checkRadio( "group 2", "b", m_secondaryForm );
        verifyTwoFormRadios( 1, 0, 0, 0, 1, 0 );

        cleanupTestStep();
    }

    /* ------------------------------------------------------------------ */
    /** tests for a special bug which we once had, where radio buttons lost their state after
     *  switching spreadsheet pages
     */
    public void checkCalcPageSwitch( ) throws com.sun.star.uno.Exception, java.lang.Exception
    {
        prepareTestStep( true );

        m_formLayer.setInsertPage( 0 );
        insertRadio( 15, 20,  "group 1 (a)", "group 1", "a" );
        insertRadio( 15, 26,  "group 1 (b)", "group 1", "b" );

        m_formLayer.setInsertPage( 1 );
        XPropertySet xRadio = insertRadio( 15, 20,  "group 2 (a)", "group 2", "a" );
                              insertRadio( 15, 26,  "group 2 (b)", "group 2", "b" );
        m_secondaryForm = (XPropertySet)dbfTools.getParent( xRadio, XPropertySet.class );

        // switch to alive mode
        SpreadsheetView view = (SpreadsheetView)m_document.getCurrentView( );
        view.toggleFormDesignMode( );
        // and do initial checking
        checkRadio( "group 1", "a", m_primaryForm );
        view.activateSheet( 1 );
        checkRadio( "group 2", "b", m_secondaryForm );

        // see whether the check states on the first page survived the page switch
        verifySheetRadios( 1, 0, 0, 1 );
        // switch back to the first sheet, and see whether the check states survived
        view.activateSheet( 0 );
        verifySheetRadios( 1, 0, 0, 1 );
        // and for completely, check again after switching to third sheet and back to the first
        view.activateSheet( 2 );
        view.activateSheet( 1 );
        verifySheetRadios( 1, 0, 0, 1 );

        cleanupTestStep();
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
        }
    }

    /* ------------------------------------------------------------------ */
    private void prepareTestStep( boolean useSpreadsheetDocument ) throws com.sun.star.uno.Exception, java.lang.Exception
    {
        m_primaryForm = null;

        m_document = useSpreadsheetDocument ? new SpreadsheetDocument( m_orb ) : DocumentHelper.blankTextDocument( m_orb );
        m_formLayer = new FormLayer( m_document );
    }

    /* ------------------------------------------------------------------ */
    private void cleanupTestStep( )
    {
        closeDocument();
    }

    /* ------------------------------------------------------------------ */
    /** checks or unchecks the radio button (in our primary form) with the given name and the given ref value
     */
    private void checkRadio( String groupName, String refValue ) throws com.sun.star.uno.Exception, java.lang.Exception
    {
        checkRadio( groupName, refValue, m_primaryForm );
    }

    /* ------------------------------------------------------------------ */
    /** checks or unchecks the radio button with the given name and the given ref value
     */
    private void checkRadio( String groupName, String refValue, XPropertySet form ) throws com.sun.star.uno.Exception, java.lang.Exception
    {
        XPropertySet xRadio = getRadioModel( groupName, refValue, form );

        XRadioButton radioButton = (XRadioButton)UnoRuntime.queryInterface(
            XRadioButton.class, m_document.getCurrentView().getControl( xRadio ) );
        radioButton.setState( true );
    }

    /* ------------------------------------------------------------------ */
    private XPropertySet getRadioModel( String name, String refValue ) throws com.sun.star.uno.Exception, java.lang.Exception
    {
        return getRadioModel( name, refValue, m_primaryForm );
    }

    /* ------------------------------------------------------------------ */
    private XPropertySet getRadioModel( String name, String refValue, XPropertySet form ) throws com.sun.star.uno.Exception, java.lang.Exception
    {
        return m_formLayer.getRadioModelByRefValue( form, name, refValue );
    }

    /* ------------------------------------------------------------------ */
    private String stateString( short[] states )
    {
        StringBuffer buf = new StringBuffer();
        for ( int i=0; i<states.length; ++i )
            buf.append( states[i] );
        return buf.toString();
    }

    /* ------------------------------------------------------------------ */
    /** verifies a number of radio buttons for their states
     */
    private boolean verifyRadios( XPropertySet[] radios, short[] expectedStates, String errorMessage ) throws com.sun.star.uno.Exception
    {
        short[] actualStates = new short[radios.length];

        // collect all current states. This is just to be able to emit them, in case of a failure
        for ( int i = 0; i<radios.length; ++i )
        {
            actualStates[i] = ((Short)radios[i].getPropertyValue( "State" )).shortValue();
        }

        // now actually check the states
        for ( int i = 0; i<radios.length; ++i )
        {
            if ( actualStates[i] != expectedStates[i] )
            {
                failed( errorMessage + " (expected: " + stateString( expectedStates ) + ", found: " + stateString( actualStates ) + ")" );
                return false;
            }
        }

        return true;
    }

    /* ------------------------------------------------------------------ */
    /** verifies the states of the 4 radio buttons from the checkSingleButtons test
     */
    private boolean verifySingleRadios( int state1, int state2, int state3, int state4 ) throws com.sun.star.uno.Exception, java.lang.Exception
    {
        XPropertySet[] radios = new XPropertySet[4];
        radios[0] = getRadioModel( "group 1", "" );
        radios[1] = getRadioModel( "group 2", "" );
        radios[2] = getRadioModel( "group 3", "" );
        radios[3] = getRadioModel( "group 4", "" );

        short[] states = new short[4];
        states[0] = (short)state1;
        states[1] = (short)state2;
        states[2] = (short)state3;
        states[3] = (short)state4;

        return verifyRadios( radios, states, "single-group radio buttons do not work!" );
    }

    /* ------------------------------------------------------------------ */
    /** verifies the states of 6 radio buttons form the checkThreeGroups test
     */
    private boolean verifySixPack( XPropertySet[] radios, String errorMessage,
        int state1, int state2, int state3, int state4, int state5, int state6 ) throws com.sun.star.uno.Exception, java.lang.Exception
    {
        short[] states = new short[6];
        states[0] = (short)state1;
        states[1] = (short)state2;
        states[2] = (short)state3;
        states[3] = (short)state4;
        states[4] = (short)state5;
        states[5] = (short)state6;

        return verifyRadios( radios, states, errorMessage );
    }

    /* ------------------------------------------------------------------ */
    /** verifies the states of 6 radio buttons
     */
    private boolean verifySixPack( int state1, int state2, int state3, int state4, int state5, int state6 ) throws com.sun.star.uno.Exception, java.lang.Exception
    {
        XPropertySet[] radios = new XPropertySet[6];
        radios[0] = getRadioModel( "group 1", "a" );
        radios[1] = getRadioModel( "group 1", "b" );
        radios[2] = getRadioModel( "group 2", "a" );
        radios[3] = getRadioModel( "group 2", "b" );
        radios[4] = getRadioModel( "group 3", "a" );
        radios[5] = getRadioModel( "group 3", "b" );

        return verifySixPack( radios, "six radio buttons, forming three different groups, do not properly work!",
            state1, state2, state3, state4, state5, state6 );
    }

    /* ------------------------------------------------------------------ */
    /** verifies the states of the 6 radio buttons in our checkMultipleForms test
     */
    private boolean verifyTwoFormRadios( int state1, int state2, int state3, int state4, int state5, int state6 ) throws com.sun.star.uno.Exception, java.lang.Exception
    {
        XPropertySet[] radios = new XPropertySet[6];
        radios[0] = getRadioModel( "group 1", "a", m_primaryForm );
        radios[1] = getRadioModel( "group 1", "b", m_primaryForm );
        radios[2] = getRadioModel( "group 1", "c", m_primaryForm );
        radios[3] = getRadioModel( "group 2", "a", m_secondaryForm );
        radios[4] = getRadioModel( "group 2", "b", m_secondaryForm );
        radios[5] = getRadioModel( "group 2", "c", m_secondaryForm );

        return verifySixPack( radios, "radio buttons on different forms do not work properly!",
            state1, state2, state3, state4, state5, state6 );
    }

    /* ------------------------------------------------------------------ */
    /** verifies the states of the 4 radio buttons in our spreadsheet document (checkCalcPageSwitch)
     */
    private boolean verifySheetRadios( int state1, int state2, int state3, int state4 ) throws com.sun.star.uno.Exception, java.lang.Exception
    {
        XPropertySet[] radios = new XPropertySet[4];
        radios[0] = getRadioModel( "group 1", "a", m_primaryForm );
        radios[1] = getRadioModel( "group 1", "b", m_primaryForm );
        radios[2] = getRadioModel( "group 2", "a", m_secondaryForm );
        radios[3] = getRadioModel( "group 2", "b", m_secondaryForm );

        short[] states = new short[4];
        states[0] = (short)state1;
        states[1] = (short)state2;
        states[2] = (short)state3;
        states[3] = (short)state4;

        return verifyRadios( radios, states, "seems some of the radio button check states didn't survive the page activation(s)!" );
    }
}

