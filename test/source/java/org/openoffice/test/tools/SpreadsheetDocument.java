/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
