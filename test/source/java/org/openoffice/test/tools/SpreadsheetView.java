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
