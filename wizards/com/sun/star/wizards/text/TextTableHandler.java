/*************************************************************************
*
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TextTableHandler.java,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2006-12-01 16:32:58 $
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
 ************************************************************************/package com.sun.star.wizards.text;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNamed;
import com.sun.star.frame.XFrame;
import com.sun.star.lang.Locale;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.style.BreakType;
import com.sun.star.table.XCellRange;
import com.sun.star.text.XSimpleText;
import com.sun.star.text.XText;
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
import com.sun.star.wizards.common.NumberFormatter;


public class TextTableHandler {
    public XTextTablesSupplier xTextTablesSupplier;
    public XMultiServiceFactory xMSFDoc;
    public XTextDocument xTextDocument;
    public XSimpleText xSimpleText;
    private XText xText;
    private NumberFormatter oNumberFormatter;
    private Locale aCharLocale;



    /** Creates a new instance of TextTableHandler */
    public TextTableHandler(XMultiServiceFactory xMSF, XTextDocument xTextDocument) {
    try {
        this.xMSFDoc = xMSF;
        this.xTextDocument = xTextDocument;
        xText = xTextDocument.getText();
        xTextTablesSupplier = (XTextTablesSupplier) UnoRuntime.queryInterface(XTextTablesSupplier.class, xTextDocument);
        xSimpleText = (XSimpleText) UnoRuntime.queryInterface(XSimpleText.class, xTextDocument.getText());
        XNumberFormatsSupplier xNumberFormatsSupplier = (XNumberFormatsSupplier) UnoRuntime.queryInterface(XNumberFormatsSupplier.class, xTextDocument);
        aCharLocale = (Locale) Helper.getUnoStructValue((Object) xTextDocument, "CharLocale");
        oNumberFormatter = new NumberFormatter(xNumberFormatsSupplier, aCharLocale );
    } catch (java.lang.Exception e) {
        e.printStackTrace(System.out);
    }
}

    public NumberFormatter getNumberFormatter(){
        return oNumberFormatter;
    }


    public XTextTable getByName(String _sTableName){
        XTextTable xTextTable = null;
    try{
        XNameAccess xAllTextTables = xTextTablesSupplier.getTextTables();
        if (xAllTextTables.hasByName(_sTableName)){
            Object oTable = xAllTextTables.getByName(_sTableName);
            xTextTable = (XTextTable) UnoRuntime.queryInterface(XTextTable.class, oTable);
        }
    } catch (Exception exception) {
        exception.printStackTrace(System.out);
    }
        return xTextTable;
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
            XIndexAccess xAllTextTables = (XIndexAccess) UnoRuntime.queryInterface(XIndexAccess.class, xTextTablesSupplier.getTextTables());
            int TextTableCount = xAllTextTables.getCount();
            for (int i = TextTableCount - 1; i >= 0; i--) {
                removeTextTable(xAllTextTables.getByIndex(i));
            }
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
    }

    public void removeLastTextTable() {
        try {
            XIndexAccess xAllTextTables = (XIndexAccess) UnoRuntime.queryInterface(XIndexAccess.class, xTextTablesSupplier.getTextTables());
            Object oTextTable = xAllTextTables.getByIndex(xAllTextTables.getCount() - 1);
            removeTextTable(oTextTable);
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
    }


    public void removeTextTable(Object oTextTable){
    try {
        XTextContent xTextContentTable = (XTextContent) UnoRuntime.queryInterface(XTextContent.class, oTextTable);
        xTextDocument.getText().removeTextContent(xTextContentTable);
    } catch (Exception exception) {
        exception.printStackTrace(System.out);
    }}


    public void removeTextTablebyName(String TableName) {
        try {
            XNameAccess xAllTextTables = xTextTablesSupplier.getTextTables();
            if (xAllTextTables.hasByName(TableName) == true) {
                removeTextTable(xAllTextTables.getByName(TableName));
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
                XNamed xTextTableName = (XNamed) UnoRuntime.queryInterface(XNamed.class, oTextTable);
                xTextTableName.setName(NewTableName);
            }
        } catch (Exception exception) {
            exception.printStackTrace(System.out);
        }
    }

    public static BreakType resetBreakTypeofTextTable(Object oTextTable) {
        BreakType CorrBreakValue = null;
        BreakType BreakValue = (BreakType) com.sun.star.wizards.common.Helper.getUnoStructValue(oTextTable, "BreakType");
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
