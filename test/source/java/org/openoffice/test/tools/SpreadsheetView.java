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

import com.sun.star.container.XIndexAccess;
import com.sun.star.frame.XController;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheetView;
import com.sun.star.uno.UnoRuntime;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * @author frank.schoenheit@oracle.com
 */
public class SpreadsheetView extends OfficeDocumentView
{

    /** Creates a new instance of SpreadsheetView */
    public SpreadsheetView( XMultiServiceFactory orb, OfficeDocument document, XController controller )
    {
        super( orb, document, controller );
    }

    /** activates the sheet with the given index
     */
    void activateSheet( int sheetIndex )
    {
        try
        {
            // get the sheet to activate
            XSpreadsheetDocument doc = UnoRuntime.queryInterface( XSpreadsheetDocument.class, getDocument().getDocument() );
            XIndexAccess sheets = UnoRuntime.queryInterface( XIndexAccess.class, doc.getSheets() );

            XSpreadsheet sheet = UnoRuntime.queryInterface( XSpreadsheet.class, sheets.getByIndex( sheetIndex ) );

            // activate
            XSpreadsheetView view = UnoRuntime.queryInterface( XSpreadsheetView.class, getController() );
            view.setActiveSheet( sheet );
        }
        catch( com.sun.star.uno.Exception e )
        {
            Logger.getLogger( SpreadsheetView.class.getName() ).log( Level.SEVERE, "unable to activate the given sheet", e );
        }
    }
}
