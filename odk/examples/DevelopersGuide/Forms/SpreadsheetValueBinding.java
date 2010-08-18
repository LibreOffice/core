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

import com.sun.star.uno.UnoRuntime;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.beans.XPropertySet;
import com.sun.star.table.XCellRange;
import com.sun.star.table.XCell;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XText;
import com.sun.star.text.XTextTable;
import com.sun.star.text.XTextCursor;
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
