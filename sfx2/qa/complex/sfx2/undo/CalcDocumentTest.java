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

package complex.sfx2.undo;

import org.openoffice.test.tools.SpreadsheetDocument;
import com.sun.star.table.XCellRange;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.table.XCell;
import com.sun.star.uno.UnoRuntime;
import org.openoffice.test.tools.DocumentType;
import static org.junit.Assert.*;

/**
 * implements the {@link DocumentTest} interface on top of a spreadsheet document
 */
public class CalcDocumentTest extends DocumentTestBase
{
    public CalcDocumentTest( final XMultiServiceFactory i_orb ) throws Exception
    {
        super( i_orb, DocumentType.CALC );
    }

    public String getDocumentDescription()
    {
        return "spreadsheet document";
    }

    public void initializeDocument() throws com.sun.star.uno.Exception
    {
        final XCell cellA1 = getCellA1();
        cellA1.setValue( INIT_VALUE );
        assertEquals( "initializing the cell value didn't work", cellA1.getValue(), INIT_VALUE, 0 );

        XCellRange range = UnoRuntime.queryInterface( XCellRange.class,
            ((SpreadsheetDocument)m_document).getSheet(0) );

        for ( int i=0; i<12; ++i )
        {
            XCell cell = range.getCellByPosition( 1, i );
            cell.setFormula( "" );
        }
    }

    public void doSingleModification() throws com.sun.star.uno.Exception
    {
        final XCell cellA1 = getCellA1();
        assertEquals( "initial cell value not as expected", INIT_VALUE, cellA1.getValue(), 0 );
        cellA1.setValue( MODIFIED_VALUE );
        assertEquals( "modified cell value not as expected", MODIFIED_VALUE, cellA1.getValue(), 0 );
    }

    public void verifyInitialDocumentState() throws com.sun.star.uno.Exception
    {
        final XCell cellA1 = getCellA1();
        assertEquals( "cell A1 doesn't have its initial value", INIT_VALUE, cellA1.getValue(), 0 );

        XCellRange range = UnoRuntime.queryInterface( XCellRange.class,
            ((SpreadsheetDocument)m_document).getSheet(0) );
        for ( int i=0; i<12; ++i )
        {
            final XCell cell = range.getCellByPosition( 1, i );
            assertEquals( "Cell B" + (i+1) + " not having its initial value (an empty string)", "", cell.getFormula() );
        }
    }

    public void verifySingleModificationDocumentState() throws com.sun.star.uno.Exception
    {
        final XCell cellA1 = getCellA1();
        assertEquals( "cell A1 doesn't have the value which we gave it", MODIFIED_VALUE, cellA1.getValue(), 0 );
    }

    public int doMultipleModifications() throws com.sun.star.uno.Exception
    {
        XCellRange range = UnoRuntime.queryInterface( XCellRange.class,
            ((SpreadsheetDocument)m_document).getSheet(0) );

        final String[] months = new String[] {
            "January", "February", "March", "April", "May", "June", "July", "August",
            "September", "October", "November", "December" };
        for ( int i=0; i<12; ++i )
        {
            final XCell cell = range.getCellByPosition( 1, i );
            cell.setFormula( months[i] );
        }
        return 12;
    }

    private XCell getCellA1() throws com.sun.star.uno.Exception
    {
        XCellRange range = UnoRuntime.queryInterface( XCellRange.class,
            ((SpreadsheetDocument)m_document).getSheet(0) );
        return range.getCellByPosition( 0, 0 );
    }

    private static final double INIT_VALUE = 100.0;
    private static final double MODIFIED_VALUE = 200.0;
}
