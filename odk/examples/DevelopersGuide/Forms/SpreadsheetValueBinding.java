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

import com.sun.star.uno.UnoRuntime;

import com.sun.star.beans.XPropertySet;
import com.sun.star.table.XCellRange;
import com.sun.star.text.XTextRange;
import com.sun.star.form.binding.XValueBinding;
import com.sun.star.form.binding.XBindableValue;
import com.sun.star.form.binding.XListEntrySource;
import com.sun.star.form.binding.XListEntrySink;

public class SpreadsheetValueBinding extends DocumentBasedExample
{
    /** Creates a new instance of SpreadsheetValueBinding */
    public SpreadsheetValueBinding()
    {
        super( DocumentType.CALC );
    }

    /* ------------------------------------------------------------------ */
    protected void prepareDocument() throws com.sun.star.uno.Exception, java.lang.Exception
    {
        super.prepareDocument();

        SpreadsheetDocument document = (SpreadsheetDocument)m_document;

        final short sheet = (short)0;
        final short exchangeColumn = (short)1;  // B
        final short exchangeRow = (short)1;     // 2
        final Integer backColor = new Integer( 0x00E0E0E0 );

        // ----------------------------------------------------------------------
        // a numeric control
        XPropertySet numericControl = m_formLayer.insertControlLine( "NumericField",
            "enter a value", "", 30 );
        numericControl.setPropertyValue( "ValueMin", new Short( (short)1 ) );
        numericControl.setPropertyValue( "ValueMax", new Short( (short)5 ) );
        numericControl.setPropertyValue( "Value", new Short( (short)1 ) );
        numericControl.setPropertyValue( "DecimalAccuracy", new Short( (short)0 ) );
        numericControl.setPropertyValue( "Spin", new Boolean( true ) );

        // bind the control model to cell B2
        implBind( numericControl, document.createCellBinding( sheet, exchangeColumn, exchangeRow ) );

        // ----------------------------------------------------------------------
        // insert a list box
        XPropertySet listBox = m_formLayer.insertControlLine( "ListBox",
            "select  an entry", "", 2, 40, 20 );
        listBox.setPropertyValue( "Dropdown", new Boolean( false ) );

        // a list binding for cell range C1-C5
        final short listSourceSheet = (short)1;
        final short column = (short)0;
        final short topRow = (short)0;
        final short bottomRow = (short)4;
        XListEntrySource entrySource = document.createListEntrySource(
            listSourceSheet, column, topRow, bottomRow );

        // bind it to the list box
        XListEntrySink consumer = (XListEntrySink)UnoRuntime.queryInterface(
            XListEntrySink.class, listBox );
        consumer.setListEntrySource( entrySource );

        // and also put the list selection index into cell B2
        implBind( listBox, document.createListIndexBinding( sheet, exchangeColumn, exchangeRow ) );

        // ----------------------------------------------------------------------
        // fill the cells which we just bound the listbox to
        XCellRange exchangeSheet = document.getSheet( listSourceSheet );
        String[] listContent = new String[] { "first", "second", "third", "forth", "fivth" };
        for ( short row = topRow; row <= bottomRow; ++row )
        {
            XTextRange cellText = (XTextRange)UnoRuntime.queryInterface(
                XTextRange.class, exchangeSheet.getCellByPosition( column, row ) );
            cellText.setString( listContent[row] );
        }

        // some coloring
        XPropertySet exchangeCell = UNO.queryPropertySet(
            document.getSheet( sheet ).getCellByPosition( exchangeColumn, exchangeRow )
        );
        exchangeCell.setPropertyValue( "CellBackColor", backColor );
        numericControl.setPropertyValue( "BackgroundColor", backColor );
        listBox.setPropertyValue( "BackgroundColor", backColor );
    }

    /* ------------------------------------------------------------------ */
    private void implBind( XPropertySet controlModel, XValueBinding binding ) throws com.sun.star.form.binding.IncompatibleTypesException
    {
        XBindableValue bindable = (XBindableValue)UnoRuntime.queryInterface(
            XBindableValue.class, controlModel
        );
        bindable.setValueBinding( binding );
    }

    /* ------------------------------------------------------------------ */
    /** class entry point
    */
    public static void main(String argv[]) throws java.lang.Exception
    {
        SpreadsheetValueBinding aSample = new SpreadsheetValueBinding();
        aSample.run( argv );
    }
 }
