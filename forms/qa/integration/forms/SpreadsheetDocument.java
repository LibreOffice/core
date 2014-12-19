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
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XComponent;
import com.sun.star.table.XCellRange;
import com.sun.star.container.XIndexAccess;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;

public class SpreadsheetDocument extends DocumentHelper
{
    /** Creates a new blank spreadsheet document */
    /* ------------------------------------------------------------------ */
    public SpreadsheetDocument( XMultiServiceFactory orb ) throws com.sun.star.uno.Exception
    {
        super( orb, implLoadAsComponent( orb, "private:factory/scalc" ) );
    }

    /* ------------------------------------------------------------------ */
    public SpreadsheetDocument( XMultiServiceFactory orb, XComponent document )
    {
        super( orb, document );
    }

    /* ------------------------------------------------------------------ */
    /** returns the sheets collection
    */
    public XSpreadsheets getSheets()
    {
        XSpreadsheetDocument spreadsheetDoc = UnoRuntime.queryInterface( XSpreadsheetDocument.class,
            getDocument()
        );
        return spreadsheetDoc.getSheets();
    }

    /* ------------------------------------------------------------------ */
    /** returns the sheet with the given index
    */
    public XCellRange getSheet( int index ) throws com.sun.star.uno.Exception
    {
        XIndexAccess sheets = UnoRuntime.queryInterface( XIndexAccess.class,
            getSheets()
        );
        return UnoRuntime.queryInterface( XCellRange.class,
            sheets.getByIndex( index )
        );
    }

}
