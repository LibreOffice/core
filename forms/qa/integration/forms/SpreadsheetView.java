/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SpreadsheetView.java,v $
 * $Revision: 1.4 $
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
/*
 * SpreadsheetView.java
 *
 * Created on 2. Oktober 2003, 14:02
 */

package integration.forms;

import com.sun.star.uno.*;
import com.sun.star.lang.*;
import com.sun.star.frame.*;
import com.sun.star.sheet.*;
import com.sun.star.container.*;

import integration.forms.DocumentViewHelper;
import integration.forms.DocumentHelper;

/**
 *
 * @author  fs93730
 */
public class SpreadsheetView extends integration.forms.DocumentViewHelper
{

    /** Creates a new instance of SpreadsheetView */
    public SpreadsheetView( XMultiServiceFactory orb, DocumentHelper document, XController controller )
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
            XSpreadsheetDocument doc = (XSpreadsheetDocument)UnoRuntime.queryInterface(
                XSpreadsheetDocument.class, getDocument().getDocument() );
            XIndexAccess sheets = (XIndexAccess)UnoRuntime.queryInterface(
                XIndexAccess.class, doc.getSheets() );

            XSpreadsheet sheet = (XSpreadsheet)UnoRuntime.queryInterface(
                XSpreadsheet.class, sheets.getByIndex( sheetIndex ) );

            // activate
            XSpreadsheetView view = (XSpreadsheetView)UnoRuntime.queryInterface(
                XSpreadsheetView.class, getController() );
            view.setActiveSheet( sheet );
        }
        catch( com.sun.star.uno.Exception e )
        {
        }
    }
}
