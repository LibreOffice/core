/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SpreadsheetView.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:10:24 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
/*
 * SpreadsheetView.java
 *
 * Created on 2. Oktober 2003, 14:02
 */

import com.sun.star.uno.*;
import com.sun.star.lang.*;
import com.sun.star.frame.*;
import com.sun.star.sheet.*;
import com.sun.star.container.*;

/**
 *
 * @author  fs93730
 */
public class SpreadsheetView extends DocumentViewHelper
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
