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
import com.sun.star.lang.XComponent;
import com.sun.star.table.XCellRange;
import com.sun.star.table.CellAddress;
import com.sun.star.table.CellRangeAddress;
import com.sun.star.container.XIndexAccess;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.beans.NamedValue;

/**
 *
 * @author  fs93730
 */
public class SpreadsheetDocument extends DocumentHelper
{
    /** Creates a new blank spreadsheet document */
    public SpreadsheetDocument( XComponentContext xCtx ) throws com.sun.star.uno.Exception
    {
        super( xCtx, implCreateBlankDocument( xCtx, "private:factory/scalc" ) );
    }

    public SpreadsheetDocument( XComponentContext xCtx, XComponent document ) throws com.sun.star.uno.Exception
    {
        super( xCtx, document );
    }

    public XCellRange getSheet( int index ) throws com.sun.star.uno.Exception
    {
        XSpreadsheetDocument spreadsheetDoc = (XSpreadsheetDocument)UnoRuntime.queryInterface( XSpreadsheetDocument.class,
            m_documentComponent
        );
        XIndexAccess sheets = (XIndexAccess)UnoRuntime.queryInterface( XIndexAccess.class,
            spreadsheetDoc.getSheets()
        );
        return (XCellRange)UnoRuntime.queryInterface( XCellRange.class,
            sheets.getByIndex( index )
        );
    }

    /** creates a value binding for a given cell
    */
    public com.sun.star.form.binding.XValueBinding createCellBinding( short sheet, short column, short row )
    {
        return createCellBinding( sheet, column, row, false );
    }

    /** creates a value binding which can be used to exchange a list box selection <em>index</em> with a cell
    */
    public com.sun.star.form.binding.XValueBinding createListIndexBinding( short sheet, short column, short row )
    {
        return createCellBinding( sheet, column, row, true );
    }

    /** creates a value binding for a given cell, with or without support for integer value exchange
    */
    private com.sun.star.form.binding.XValueBinding createCellBinding( short sheet, short column, short row, boolean supportIntegerValues )
    {
        com.sun.star.form.binding.XValueBinding cellBinding = null;
        try
        {
            CellAddress address = new CellAddress( sheet, column, row );
            Object[] initParam = new Object[] { new NamedValue( "BoundCell", address ) };
            cellBinding = (com.sun.star.form.binding.XValueBinding)UnoRuntime.queryInterface(
                com.sun.star.form.binding.XValueBinding.class,
                createInstanceWithArguments(
                    supportIntegerValues ? "com.sun.star.table.ListPositionCellBinding"
                                         : "com.sun.star.table.CellValueBinding",
                    initParam
                )
            );
        }
        catch( com.sun.star.uno.Exception e )
        {
            System.err.println( e );
            e.printStackTrace( System.err );
        }
        return cellBinding;
    }

    /** creates a source of list entries associated with a (one-column) cell range
    */
    public com.sun.star.form.binding.XListEntrySource createListEntrySource( short sheet, short column,
        short topRow, short bottomRow )
    {
        com.sun.star.form.binding.XListEntrySource entrySource = null;
        try
        {
            CellRangeAddress rangeAddress = new CellRangeAddress( sheet, column,
                topRow, column, bottomRow );
            Object[] initParam = new Object[] { new NamedValue( "CellRange", rangeAddress ) };
            entrySource = (com.sun.star.form.binding.XListEntrySource)UnoRuntime.queryInterface(
                com.sun.star.form.binding.XListEntrySource.class,
                createInstanceWithArguments(
                    "com.sun.star.table.CellRangeListSource", initParam ) );
        }
        catch( com.sun.star.uno.Exception e )
        {
            System.err.println( e );
            e.printStackTrace( System.err );
        }
        return entrySource;
    }
}
