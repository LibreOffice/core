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

/*
 * CellBinding.java
 *
 * Created on 12. Mai 2004, 15:15
 */

package integration.forms;

import com.sun.star.uno.*;
import com.sun.star.util.*;
import com.sun.star.lang.*;
import com.sun.star.beans.*;
import com.sun.star.form.binding.*;
import com.sun.star.accessibility.*;
import com.sun.star.awt.XListBox;
import com.sun.star.table.CellAddress;
import com.sun.star.table.XCell;
import com.sun.star.sheet.XCellRangeData;
import com.sun.star.sheet.XCellRangeFormula;
import com.sun.star.table.CellRangeAddress;
import com.sun.star.text.XTextRange;

public class CellBinding extends complexlib.ComplexTestCase
{
    /** the test document our form layer lives in */
    private SpreadsheetDocument     m_document;
    /** our form layer */
    private FormLayer               m_formLayer;
    /** our service factory */
    private XMultiServiceFactory    m_orb;

    /** Creates a new instance of CellBinding */
    public CellBinding()
    {
    }

    public String[] getTestMethodNames()
    {
        return new String[] {
            "checkTextFieldBinding",
            "checkBooleanRadioBinding",
            "checkStringRadioBinding",
            "checkBooleanCheckBoxBinding",
            "checkStringCheckBoxBinding",
            "checkListBoxBinding",
            "checkListBoxIndexBinding"
        };
    }

    public String getTestObjectName()
    {
        return "Form Control Spreadsheet Cell Binding Test";
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
    public void before() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        m_orb = (XMultiServiceFactory)param.getMSF();
        m_document = new SpreadsheetDocument( m_orb );
        m_formLayer = new FormLayer( m_document );
    }

    /* ------------------------------------------------------------------ */
    public void after() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        closeDocument();
    }

    /* ------------------------------------------------------------------ */
    public void checkTextFieldBinding( ) throws com.sun.star.uno.Exception, java.lang.Exception
    {
        final short col = 0;
        final short row = 2;
        final String text = new String( "content" );
        final String otherText = new String( "something else" );
        final String yetAnotherText = new String( "yet another text" );

        // cretae a normal text control
        XPropertySet controlModel = m_formLayer.createControlAndShape( "DatabaseTextField", 30, 9, 30, 6 );

        // bind it to cell A1
        bindToCell( controlModel, col, row );

        // switch to alive mode
        m_document.getCurrentView().toggleFormDesignMode();

        // test the data transfer control -> cell
        simulateUserTextInput( controlModel, text );
        verifyStringCellContent( col, row, text, "A text field does not forward its user input to the cell." );

        // the same, but this time changing the control value programmatically
        controlModel.setPropertyValue( "Text", otherText );
        verifyStringCellContent( col, row, otherText, "A text field does not forward programmatic changes to the cell." );

        // the other way round: cell->control
        setCellText( col, row, yetAnotherText );
        String controlText = (String)controlModel.getPropertyValue( "Text" );
        if ( !controlText.equals( yetAnotherText ) )
            failed( "Changes in the cell are not forwarded to the text field." );
    }
    /* ------------------------------------------------------------------ */
    public void checkBooleanRadioBinding( ) throws com.sun.star.uno.Exception, java.lang.Exception
    {
        // two radio buttons
        XPropertySet primaryRadio = createRadio( 28, "radio button no. 1", "radio group", "primary" );
        XPropertySet secondaryRadio = createRadio( 33, "radio button no. 2", "radio group", "secodary" );

        // bind them
        short col = (short)0;
        short row1 = (short)6;
        short row2 = (short)7;
        bindToCell( primaryRadio, col, row1 );
        bindToCell( secondaryRadio, col, row2 );

        // check the first button
        simulateUserRadioCheck( primaryRadio );
        // check the cell content
        verifyNumericCellContent( col, row1, 1, "Radio buttons do not forward their (boolean) values to cells (1)." );
        verifyNumericCellContent( col, row2, 0, "Radio buttons do not forward their (boolean) values to cells (2)." );
        // check the second button
        simulateUserRadioCheck( secondaryRadio );
        // check the cell content
        verifyNumericCellContent( col, row1, 0, "Radio buttons do not forward their (boolean) values to cells (3)." );
        verifyNumericCellContent( col, row2, 1, "Radio buttons do not forward their (boolean) values to cells (4)." );

        // the other way round: writing values into the cell
        setCellValue( col, row1, 1.0 );
        // setting this should have checked the primary radio, which should have unchecked the secondary radio,
        // which should have been propagated to the second cell
        verifyNumericCellContent( col, row2, 0, "Changing primary cell is not propagated to the secondary cell (via the radio buttons)." );

        // setting an empty cell should result in the radio being unchecked
        setCellEmpty( col, row1 );
        if ( ((Short)primaryRadio.getPropertyValue( "State" )).shortValue() != 0 )
            failed( "Setting a cell to 'empty' does not reset the bound radio button." );
    }

    /* ------------------------------------------------------------------ */
    public void checkStringRadioBinding( ) throws com.sun.star.uno.Exception, java.lang.Exception
    {
        // two radio buttons
        XPropertySet primaryRadio = createRadio( 46, "radio button A", "radio ref group", "primary" );
        XPropertySet secondaryRadio = createRadio( 51, "radio button B", "radio ref group", "secodary" );

        // give the ref values
        String refValueA = new String( "ref value A" );
        String refValueB = new String( "ref value B" );
        primaryRadio.setPropertyValue( "RefValue", refValueA );
        secondaryRadio.setPropertyValue( "RefValue", refValueB );

        // bind them to the same cell
        short col = (short)0;
        short row = (short)10;
        bindToCell( primaryRadio, col, row );
        bindToCell( secondaryRadio, col, row );

        // checking a radio should set the respective ref value at the cell
        simulateUserRadioCheck( primaryRadio );
        verifyStringCellContent( col, row, refValueA, "A bound radio button with a reference value does not pass this value to the cell upon checking (1)." );
        simulateUserRadioCheck( secondaryRadio );
        verifyStringCellContent( col, row, refValueB, "A bound radio button with a reference value does not pass this value to the cell upon checking (2)." );

        // changing the cell should check the buttons if the cell text equals the ref value
        setCellText( col, row, "no ref value" );
        verifyRadioStates( primaryRadio, secondaryRadio, (short)0, (short)0, "Radio button not unchecked, though the bound cell value does not equal ref value." );

        setCellText( col, row, refValueA );
        verifyRadioStates( primaryRadio, secondaryRadio, (short)1, (short)0, "Radio button not properly un/checked according to the cell and ref value (1)." );

        setCellText( col, row, refValueB );
        verifyRadioStates( primaryRadio, secondaryRadio, (short)0, (short)1, "Radio button not properly un/checked according to the cell and ref value (2)." );
    }

    /* ------------------------------------------------------------------ */
    public void checkBooleanCheckBoxBinding( ) throws com.sun.star.uno.Exception, java.lang.Exception
    {
        XPropertySet checkBox = m_formLayer.createControlAndShape( "DatabaseCheckBox", 30, 59, 40, 4 );
        checkBox.setPropertyValue( "Label", "check box" );
        checkBox.setPropertyValue( "TriState", new Boolean( true ) );

        short col = (short)0;
        short row = (short)13;
        bindToCell( checkBox, col, row );

        // initialize with "not checked"
        checkBox.setPropertyValue( "State", new Short( (short)0 ) );
        verifyNumericCellContent( col, row, 0, "programmatically unchecking the check box is not propagated to the cell." );

        // first click: "not checked" -> "checked"
        simulateUserCheckBoxCheck( checkBox, (short)1 );
        verifyNumericCellContent( col, row, 1, "moving the check box state to 'checked' is not propagated to the cell." );

        // second click: "checked" -> "indetermined"
        simulateUserCheckBoxCheck( checkBox, (short)2 );
        verifyVoidCell( col, row, "propagating the 'indetermined' state to the cell does not work." );

        // third click: "indetermined" -> "not checked"
        simulateUserCheckBoxCheck( checkBox, (short)0 );
        verifyNumericCellContent( col, row, 0, "unchecking a check box via UI is not propagated to the cell." );
    }

    /* ------------------------------------------------------------------ */
    public void checkStringCheckBoxBinding( ) throws com.sun.star.uno.Exception, java.lang.Exception
    {
        String refValue = new String( "checked " );

        XPropertySet checkBox = m_formLayer.createControlAndShape( "DatabaseCheckBox", 30, 68, 40, 4 );
        checkBox.setPropertyValue( "Label", "check box with ref value" );
        checkBox.setPropertyValue( "TriState", new Boolean( true ) );
        checkBox.setPropertyValue( "RefValue", refValue );

        short col = (short)0;
        short row = (short)15;
        bindToCell( checkBox, col, row );

        // initialize with "not checked"
        checkBox.setPropertyValue( "State", new Short( (short)0 ) );
        verifyNumericCellContent( col, row, 0, "programmatically unchecking the check box is not propagated to the cell." );

        // first click: "not checked" -> "checked"
        simulateUserCheckBoxCheck( checkBox, (short)1 );
        verifyStringCellContent( col, row, refValue, "moving the check box state to 'checked' does not propagated the ref value to the cell." );

        // second click: "checked" -> "indetermined"
        simulateUserCheckBoxCheck( checkBox, (short)2 );
        verifyVoidCell( col, row, "propagating the 'indetermined' state to the cell does not work, when exchanging ref values." );

        // third click: "indetermined" -> "not checked"
        simulateUserCheckBoxCheck( checkBox, (short)0 );
        verifyStringCellContent( col, row, "", "unchecking a check box via UI does not propagated the ref value to the cell." );
    }

    /* ------------------------------------------------------------------ */
    /** verifies that a list box, which is bound via an ordinary value binding,
     *  works as expected
     */
    public void checkListBoxBinding( ) throws com.sun.star.uno.Exception, java.lang.Exception
    {
        XPropertySet listBox = m_formLayer.createControlAndShape( "DatabaseListBox", 30, 80, 40, 6 );
        listBox.setPropertyValue( "Dropdown", new Boolean( true ) );
        listBox.setPropertyValue( "StringItemList", new String[] { "Apples", "Oranges", "Peaches" } );

        short col = (short)0;
        short row = (short)18;

        // ...............................................................
        // add a list entry source which fills the list boxes list from cells in the
        // spreadsheet
        short sourceCol = (short)4;
        setCellText( sourceCol, (short)( row - 1 ), "Apples" );
        setCellText( sourceCol, (short)( row + 0 ), "Oranges" );
        setCellText( sourceCol, (short)( row + 1 ), "Peaches" );

        //setListSource( listBox, sourceCol, row, (short)( row + 2 ) );
            // TODO: this is currently prone to deadlocks

        // ...............................................................
        // bind to a cell
        bindToCell( listBox, col, row );

        // ...............................................................
        // do the tests
        listBox.setPropertyValue( "SelectedItems", new short[] { (short)0 } );
        verifyStringCellContent( col, row, "Apples", "programmatically selecting a list entry is not propagated to the cell." );

        simulateUserListBoxSelection( listBox, "Oranges" );
        verifyStringCellContent( col, row, "Oranges", "UI-selecting a list entry is not propagated to the cell." );

        setCellText( col, row, "Peaches" );
        short[] selectedItems = (short[])listBox.getPropertyValue( "SelectedItems" );
        assureEquals( "changes in the cell bound to a list box are not propagated to the list box selection",
            2, selectedItems[0] );
    }

    /* ------------------------------------------------------------------ */
    /** verifies that a list box, which is bound via a value binding exchanging the <b>index</b>
     *  of the selected entry, works as expected
     */
    public void checkListBoxIndexBinding() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        XPropertySet listBox = m_formLayer.createControlAndShape( "DatabaseListBox", 30, 94, 40, 6 );
        listBox.setPropertyValue( "Dropdown", new Boolean( true ) );
        listBox.setPropertyValue( "StringItemList", new String[] { "Pears", "Bananas", "Strawberries" } );

        short col = (short)0;
        short row = (short)21;

        // ...............................................................
        // add a list entry source which fills the list boxes list from cells in the
        // spreadsheet
        short sourceCol = (short)4;
        setCellText( sourceCol, (short)( row - 1 ), "Pears" );
        setCellText( sourceCol, (short)( row + 0 ), "Bananas" );
        setCellText( sourceCol, (short)( row + 1 ), "Strawberries" );

        //setListSource( listBox, sourceCol, row, (short)( row + 2 ) );
            // TODO: this is currently prone to deadlocks

        // ...............................................................
        // bind to a cell
        bindToCell( listBox, col, row, "com.sun.star.table.ListPositionCellBinding" );

        // ...............................................................
        // do the tests
        listBox.setPropertyValue( "SelectedItems", new short[] { (short)0 } );
        verifyNumericCellContent( col, row, 1, "programmatically selecting a list entry is not propagated (as index) to the cell." );

        simulateUserListBoxSelection( listBox, "Bananas" );
        verifyNumericCellContent( col, row, 2, "UI-selecting a list entry is not propagated (as index) to the cell." );

        setCellValue( col, row, 3 );
        short[] selectedItems = (short[])listBox.getPropertyValue( "SelectedItems" );
        assureEquals( "changes in the cell bound to a list box via list index are not propagated to the list box selection",
            2, selectedItems[0] );
    }

    /* ------------------------------------------------------------------ */
    /** verifies that the content of a given cell equals a given string
    */
    private XPropertySet createRadio( int yPos, String label, String name, String tag ) throws com.sun.star.uno.Exception, java.lang.Exception
    {
        XPropertySet radio = m_formLayer.createControlAndShape( "DatabaseRadioButton", 30, yPos, 40, 4 );
        radio.setPropertyValue( "Label", label );
        radio.setPropertyValue( "Name", name );
        radio.setPropertyValue( "Tag", tag );
        return radio;
    }

    /* ------------------------------------------------------------------ */
    /** verifies the states of two radio button
    */
    private boolean verifyRadioStates( XPropertySet radio1, XPropertySet radio2, short value1, short value2,
        String errorMessage ) throws com.sun.star.uno.Exception, java.lang.Exception
    {
        if (  ( ((Short)radio1.getPropertyValue( "State" )).shortValue() != value1 )
           || ( ((Short)radio2.getPropertyValue( "State" )).shortValue() != value2 )
           )
        {
            failed( errorMessage );
            return false;
        }
        return true;
    }

    /* ------------------------------------------------------------------ */
    /** verifies that the content of a given cell equals a given string
    */
    private boolean verifyVoidCell( short col, short row, String failErrorMessage ) throws com.sun.star.uno.Exception
    {
        XCellRangeData cell = (XCellRangeData)UnoRuntime.queryInterface( XCellRangeData.class,
            m_document.getSheet( 0 ).getCellByPosition( col, row )
        );
        Object cellContent = cell.getDataArray()[0][0];
        if ( ((com.sun.star.uno.Any)cellContent).getType().getTypeClass() != com.sun.star.uno.TypeClass.VOID )
        {
            failed( failErrorMessage );
            return false;
        }
        return true;
    }

    /* ------------------------------------------------------------------ */
    /** verifies that the content of a given cell equals a given string
    */
    private boolean verifyNumericCellContent( short col, short row, double value, String failErrorMessage ) throws com.sun.star.uno.Exception
    {
        XCell cell = (XCell)UnoRuntime.queryInterface( XCell.class,
            m_document.getSheet( 0 ).getCellByPosition( col, row )
        );
        if ( cell.getValue() != value )
        {
            failed( failErrorMessage );
            return false;
        }
        return true;
    }

    /* ------------------------------------------------------------------ */
    /** verifies that the content of a given cell equals a given string
    */
    private boolean verifyStringCellContent( short col, short row, String text, String failErrorMessage ) throws com.sun.star.uno.Exception
    {
        XTextRange cell = (XTextRange)UnoRuntime.queryInterface( XTextRange.class,
            m_document.getSheet( 0 ).getCellByPosition( col, row )
        );
        if ( !cell.getString().equals( text ) )
        {
            failed( failErrorMessage );
            return false;
        }
        return true;
    }

    /* ------------------------------------------------------------------ */
    /** sets the text of a given cell to a given string
    */
    private void setCellText( short col, short row, String text ) throws com.sun.star.uno.Exception
    {
        XTextRange cell = (XTextRange)UnoRuntime.queryInterface( XTextRange.class,
            m_document.getSheet( 0 ).getCellByPosition( col, row )
        );
        cell.setString( text );
    }

    /* ------------------------------------------------------------------ */
    /** sets a numeric value in a given cell
    */
    private void setCellValue( short col, short row, double value ) throws com.sun.star.uno.Exception
    {
        XCell cell = (XCell)UnoRuntime.queryInterface( XCell.class,
            m_document.getSheet( 0 ).getCellByPosition( col, row )
        );
        cell.setValue( value );
    }

    /* ------------------------------------------------------------------ */
    /** sets a numeric value in a given cell
    */
    private void setCellEmpty( short col, short row ) throws com.sun.star.uno.Exception
    {
        // as long as #i29130# is not fixed, we do not set the cell to "empty", but to
        // an invalid formular, which serves well for our purpose
        XCellRangeFormula cell = (XCellRangeFormula)UnoRuntime.queryInterface( XCellRangeFormula.class,
            m_document.getSheet( 0 ).getCellByPosition( col, row )
        );
        String[][] args = new String[][] { new String[] { "=INVALID_FUNCTION()" } };
        cell.setFormulaArray( args );
    }

    /* ------------------------------------------------------------------ */
    /** binds the given control model to the given cell in the first sheet,
     *  using the given service name for the binding
     */
    private void bindToCell( XPropertySet controlModel, short column, short row, String _bindingServiceName ) throws com.sun.star.uno.Exception
    {
        XBindableValue bindableModel = (XBindableValue)UnoRuntime.queryInterface( XBindableValue.class,
            controlModel
        );

        CellAddress address = new CellAddress();
        address.Column = column;
        address.Row = row;
        address.Sheet = 0;

        NamedValue[] parameters = new NamedValue[] { new NamedValue() };
        parameters[0].Name = "BoundCell";
        parameters[0].Value = address;

        XValueBinding cellBinding = (XValueBinding)UnoRuntime.queryInterface( XValueBinding.class,
            m_document.createInstanceWithArguments( _bindingServiceName, parameters )
        );

        bindableModel.setValueBinding( cellBinding );
    }

    /* ------------------------------------------------------------------ */
    /** binds the given control model to the given cell in the first sheet
    */
    private void bindToCell( XPropertySet _controlModel, short _column, short _row ) throws com.sun.star.uno.Exception
    {
        bindToCell( _controlModel, _column, _row, "com.sun.star.table.CellValueBinding" );
    }

    /* ------------------------------------------------------------------ */
    /** sets the given cell range as list entry source for the given control
    */
    private void setListSource( XPropertySet _listSink, short _sourceCol, short _rowStart, short _rowEnd ) throws com.sun.star.uno.Exception
    {
        CellRangeAddress listSourceAddress = new CellRangeAddress( (short)0, (int)_sourceCol, (int)_rowStart, (int)_sourceCol, (int)_rowEnd );
        NamedValue addressParameter = new NamedValue( "CellRange", listSourceAddress );

        XListEntrySource listSource = (XListEntrySource)UnoRuntime.queryInterface( XListEntrySource.class,
            m_document.createInstanceWithArguments( "com.sun.star.table.CellRangeListSource", new NamedValue[]{ addressParameter } )
        );
        XListEntrySink listSink = (XListEntrySink)UnoRuntime.queryInterface( XListEntrySink.class,
            _listSink );
        listSink.setListEntrySource( listSource );
    }

    /* ------------------------------------------------------------------ */
    /** simulates a user action to check a radio button
    */
    private void simulateUserRadioCheck( XPropertySet radioModel ) throws com.sun.star.uno.Exception
    {
        XAccessible accessible = (XAccessible)UnoRuntime.queryInterface(
            XAccessible.class, m_document.getCurrentView().getControl( radioModel ) );

        XAccessibleValue xValue = (XAccessibleValue)UnoRuntime.queryInterface(
            XAccessibleValue.class, accessible.getAccessibleContext() );

        Integer newValue = new Integer( 1 );
        xValue.setCurrentValue( newValue );
    }

    /* ------------------------------------------------------------------ */
    /** simulates a user action to check a radio button
    */
    private void simulateUserCheckBoxCheck( XPropertySet checkBox, short state ) throws com.sun.star.uno.Exception
    {
        XAccessible accessible = (XAccessible)UnoRuntime.queryInterface(
            XAccessible.class, m_document.getCurrentView().getControl( checkBox ) );

        XAccessibleValue xValue = (XAccessibleValue)UnoRuntime.queryInterface(
            XAccessibleValue.class, accessible.getAccessibleContext() );

        xValue.setCurrentValue( new Short( state ) );
    }

    /* ------------------------------------------------------------------ */
    /** simulates a user selecting an entry in a list box
    */
    private void simulateUserListBoxSelection( XPropertySet _listBox, String _selectEntry ) throws com.sun.star.uno.Exception
    {
        XListBox listBoxControl = (XListBox)UnoRuntime.queryInterface(
            XListBox.class, m_document.getCurrentView().getControl( _listBox ) );
        listBoxControl.selectItem( _selectEntry, true );
    }

    /* ------------------------------------------------------------------ */
    /** simulates text input into the control belonging to the given model
    */
    private void simulateUserTextInput( XPropertySet controlModel, String text ) throws com.sun.star.uno.Exception
    {
        XAccessible accessible = (XAccessible)UnoRuntime.queryInterface(
            XAccessible.class, m_document.getCurrentView().getControl( controlModel ) );

        XAccessibleContext context = accessible.getAccessibleContext();
        XServiceInfo si = (XServiceInfo)UnoRuntime.queryInterface( XServiceInfo.class,
            accessible.getAccessibleContext() );

        XAccessibleEditableText textAccess = (XAccessibleEditableText)UnoRuntime.queryInterface(
            XAccessibleEditableText.class, accessible.getAccessibleContext() );

        textAccess.setText( text );
    }
}
