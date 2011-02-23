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

package org.openoffice.test.tools;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XComponent;
import com.sun.star.table.XCellRange;
import com.sun.star.container.XIndexAccess;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.uno.UnoRuntime;

/**
 * @author  frank.schoenheit@oracle.com
 */
public class SpreadsheetDocument extends OfficeDocument
{
    /** Creates a new blank spreadsheet document */
    /* ------------------------------------------------------------------ */
    public SpreadsheetDocument( XMultiServiceFactory orb ) throws com.sun.star.uno.Exception
    {
        super( orb, implLoadAsComponent( orb, "private:factory/scalc" ) );
    }

    /* ------------------------------------------------------------------ */
    public SpreadsheetDocument( XMultiServiceFactory orb, XComponent document ) throws com.sun.star.uno.Exception
    {
        super( orb, document );
    }

    /* ------------------------------------------------------------------ */
    /** returns the sheets collection
    */
    public XSpreadsheets getSheets() throws com.sun.star.uno.Exception
    {
        XSpreadsheetDocument spreadsheetDoc = UnoRuntime.queryInterface( XSpreadsheetDocument.class, getDocument() );
        return spreadsheetDoc.getSheets();
    }

    /* ------------------------------------------------------------------ */
    /** returns the sheet with the given index
    */
    public XCellRange getSheet( int index ) throws com.sun.star.uno.Exception
    {
        XIndexAccess sheets = UnoRuntime.queryInterface( XIndexAccess.class, getSheets() );
        return UnoRuntime.queryInterface( XCellRange.class, sheets.getByIndex( index ) );
    }
}
