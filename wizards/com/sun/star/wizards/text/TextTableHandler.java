/*************************************************************************
*
*  $RCSfile: TextTableHandler.java,v $
*
*  $Revision: 1.2 $
*
*  last change: $Author: kz $ $Date: 2004-05-19 12:49:37 $
*
*  The Contents of this file are made available subject to the terms of
*  either of the following licenses
*
*         - GNU Lesser General Public License Version 2.1
*         - Sun Industry Standards Source License Version 1.1
*
*  Sun Microsystems Inc., October, 2000
*
*  GNU Lesser General Public License Version 2.1
*  =============================================
*  Copyright 2000 by Sun Microsystems, Inc.
*  901 San Antonio Road, Palo Alto, CA 94303, USA
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Lesser General Public
*  License version 2.1, as published by the Free Software Foundation.
*
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*  Lesser General Public License for more details.
*
*  You should have received a copy of the GNU Lesser General Public
*  License along with this library; if not, write to the Free Software
*  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
*  MA  02111-1307  USA
*
*
*  Sun Industry Standards Source License Version 1.1
*  =================================================
*  The contents of this file are subject to the Sun Industry Standards
*  Source License Version 1.1 (the "License"); You may not use this file
*  except in compliance with the License. You may obtain a copy of the
*  License at http://www.openoffice.org/license.html.
*
*  Software provided under this License is provided on an "AS IS" basis,
*  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
*  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
*  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
*  See the License for the specific provisions governing your rights and
*  obligations concerning the Software.
*
*  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
*
*  Copyright: 2000 by Sun Microsystems, Inc.
*
*  All Rights Reserved.
*
*  Contributor(s): _______________________________________
*
*/

package com.sun.star.wizards.text;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNamed;
import com.sun.star.frame.XFrame;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.style.BreakType;
import com.sun.star.table.XCellRange;
import com.sun.star.text.XSimpleText;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextTable;
import com.sun.star.text.XTextTablesSupplier;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.XNumberFormats;
import com.sun.star.util.XNumberFormatsSupplier;
import com.sun.star.view.XSelectionSupplier;
import com.sun.star.wizards.common.Desktop;
import com.sun.star.wizards.common.Helper;


public class TextTableHandler {
    public XTextTablesSupplier xTextTablesSupplier;
    public XMultiServiceFactory xMSFDoc;
    public XTextDocument xTextDocument;
    public XNumberFormats NumberFormats;
    public XSimpleText xSimpleText;


    /** Creates a new instance of TextTableHandler */
    public TextTableHandler(XMultiServiceFactory xMSF, XTextDocument xTextDocument) {
        this.xMSFDoc = xMSF;
        this.xTextDocument = xTextDocument;
        xTextTablesSupplier = (XTextTablesSupplier) UnoRuntime.queryInterface(XTextTablesSupplier.class, xTextDocument);
        XNumberFormatsSupplier xNumberFormatsSupplier = (XNumberFormatsSupplier) UnoRuntime.queryInterface(XNumberFormatsSupplier.class, xTextDocument);
        NumberFormats = xNumberFormatsSupplier.getNumberFormats();
        xSimpleText = (XSimpleText) UnoRuntime.queryInterface(XSimpleText.class, xTextDocument.getText());

    }

    public com.sun.star.text.XTextTable getlastTextTable() {
        try {
            XIndexAccess xAllTextTables = (XIndexAccess) UnoRuntime.queryInterface(XIndexAccess.class, xTextTablesSupplier.getTextTables());
            int MaxIndex = xAllTextTables.getCount() - 1;
            Object oTable = xAllTextTables.getByIndex(MaxIndex);
            XTextTable xTextTable = (XTextTable) UnoRuntime.queryInterface(XTextTable.class, oTable);
            return xTextTable;
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
            return null;
        }
    }

    public void insertTextTable(com.sun.star.text.XTextCursor xTextCursor) {
        try {
            com.sun.star.uno.XInterface xTextTable = (XInterface) xMSFDoc.createInstance("com.sun.star.text.TextTable");
            XTextContent xTextContentTable = (XTextContent) UnoRuntime.queryInterface(XTextContent.class, xTextTable);
            if (xTextCursor == null) {
                xTextCursor = xTextDocument.getText().createTextCursor();
                xTextCursor.gotoEnd(false);
            }
            xTextCursor.getText().insertTextContent(xTextCursor, xTextContentTable, false);
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
    }

    public void removeAllTextTables() {
        try {
            Object oTextTable;
            com.sun.star.text.XText xText = xTextDocument.getText();
            XIndexAccess xAllTextTables = (XIndexAccess) UnoRuntime.queryInterface(XIndexAccess.class, xTextTablesSupplier.getTextTables());
            int TextTableCount = xAllTextTables.getCount();
            for (int i = TextTableCount - 1; i >= 0; i--) {
                oTextTable = xAllTextTables.getByIndex(i);
                XTextContent xTextContentTextTable = (XTextContent) UnoRuntime.queryInterface(XTextContent.class, oTextTable);
                xText.removeTextContent(xTextContentTextTable);
            }
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
    }

    public void removeLastTextTable() {
        try {
            XIndexAccess xAllTextTables = (XIndexAccess) UnoRuntime.queryInterface(XIndexAccess.class, xTextTablesSupplier.getTextTables());
            Object oTextTable = xAllTextTables.getByIndex(xAllTextTables.getCount() - 1);
            XTextContent xTextContentTable = (XTextContent) UnoRuntime.queryInterface(XTextContent.class, oTextTable);
            xTextDocument.getText().removeTextContent(xTextContentTable);
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
    }

    public void removeTextTablebyName(String TableName) {
        try {
            com.sun.star.container.XNameAccess xAllTextTables = xTextTablesSupplier.getTextTables();
            if (xAllTextTables.hasByName(TableName) == true) {
                Object oTextTable = xAllTextTables.getByName(TableName);
                XTextContent xTextContentoTextTable = (XTextContent) UnoRuntime.queryInterface(XTextContent.class, oTextTable);
                xTextDocument.getText().removeTextContent(xTextContentoTextTable);
            }
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
    }

    public void renameTextTable(String OldTableName, String NewTableName) {
        try {
            XNameAccess xTextTableNames = xTextTablesSupplier.getTextTables();
            if (xTextTableNames.hasByName(OldTableName)) {
                Object oTextTable = xTextTableNames.getByName(OldTableName);
                XNamed xTextTableName = (com.sun.star.container.XNamed) UnoRuntime.queryInterface(XNamed.class, oTextTable);
                xTextTableName.setName(NewTableName);
            }
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
    }

    public static BreakType resetBreakTypeofTextTable(Object oTextTable) {
        BreakType CorrBreakValue = null;
        //  BreakType BreakValue = (BreakType) com.sun.star.wizards.common.Helper.getUnoStructValue(oTextTable, "BreakType");
        //  if (BreakValue.equals(BreakType.NONE) == false){
        //      CorrBreakValue = BreakValue;
        Helper.setUnoPropertyValue(oTextTable, "BreakType", BreakType.NONE);
        //  }
        return BreakType.NONE;
    }


    public void adjustOptimalTableWidths(XMultiServiceFactory _xMSF, XTextTable xTextTable){        // setTableColumnSeparators(){
    try{
        XFrame xFrame = this.xTextDocument.getCurrentController().getFrame();
        int ColCount = xTextTable.getColumns().getCount();
        XCellRange xCellRange = (XCellRange) UnoRuntime.queryInterface(XCellRange.class, xTextTable);
        XCellRange xLocCellRange = xCellRange.getCellRangeByPosition(0, 0, ColCount-1, 1);
        short iHoriOrient =  AnyConverter.toShort(Helper.getUnoPropertyValue(xTextTable, "HoriOrient"));
        XSelectionSupplier xSelection = (XSelectionSupplier) UnoRuntime.queryInterface(XSelectionSupplier.class, xTextDocument.getCurrentController());
        xSelection.select(xLocCellRange);
        Desktop.dispatchURL(_xMSF, ".Uno:DistributeColumns", xFrame);
        Desktop.dispatchURL(_xMSF, ".Uno:SetOptimalColumnWidth", xFrame);
        Helper.setUnoPropertyValue(xTextTable, "HoriOrient", new Short(iHoriOrient));
    }
    catch( Exception exception ){
        exception.printStackTrace(System.out);
    }}
}
