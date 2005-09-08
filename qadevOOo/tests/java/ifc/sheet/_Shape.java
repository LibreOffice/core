/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _Shape.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:37:14 $
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
package ifc.sheet;

import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.table.XCell;
import com.sun.star.uno.UnoRuntime;

import lib.MultiPropertyTest;


public class _Shape extends MultiPropertyTest {
    public void _Anchor() {
        XSpreadsheetDocument xSheetDoc = (XSpreadsheetDocument) UnoRuntime.queryInterface(
                                                 XSpreadsheetDocument.class,
                                                 tEnv.getObjRelation(
                                                         "DOCUMENT"));
        String[] sheetNames = xSheetDoc.getSheets().getElementNames();
        XSpreadsheet xSheet = null;
        XCell xCell = null;

        try {
            xSheet = (XSpreadsheet) UnoRuntime.queryInterface(
                             XSpreadsheet.class,
                             xSheetDoc.getSheets().getByName(sheetNames[0]));
            xCell = xSheet.getCellByPosition(0, 0);
        } catch (com.sun.star.container.NoSuchElementException e) {
            e.printStackTrace();
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace();
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace();
        }

        testProperty("Anchor", xSheet, xCell);
    }
}