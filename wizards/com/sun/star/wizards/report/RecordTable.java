/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RecordTable.java,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:37:42 $
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
package com.sun.star.wizards.report;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNamed;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.table.XCellRange;
import com.sun.star.table.XTableColumns;
import com.sun.star.table.XTableRows;
import com.sun.star.text.XTextTable;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.text.TextTableHandler;
import com.sun.star.wizards.text.ViewHandler;


public class RecordTable{
    String CurFieldName;
    String LabelDescription;
    public XNamed xTableName;
    public XCellRange xCellRange;
    public XTextTable xTextTable;
    private TextTableHandler oTextTableHandler;
    public XTableColumns xTableColumns;
    public XTableRows xTableRows;



    public RecordTable(TextTableHandler _oTextTableHandler){
    try{
        this.oTextTableHandler = _oTextTableHandler;
        String[] TableNames = oTextTableHandler.xTextTablesSupplier.getTextTables().getElementNames();
        XNameAccess xAllTextTables = oTextTableHandler.xTextTablesSupplier.getTextTables();
        if ((xAllTextTables.hasByName(ReportDocument.TBLRECORDSECTION)) || (xAllTextTables.hasByName(ReportDocument.COPYOFTBLRECORDSECTION))){
            Object oTable;
            if (xAllTextTables.hasByName(ReportDocument.COPYOFTBLRECORDSECTION))
                oTable = xAllTextTables.getByName(ReportDocument.COPYOFTBLRECORDSECTION);
            else
                oTable = xAllTextTables.getByName(ReportDocument.TBLRECORDSECTION);
            xTextTable = (XTextTable) UnoRuntime.queryInterface(XTextTable.class, oTable);
            xTableName = (XNamed) UnoRuntime.queryInterface(XNamed.class, xTextTable);
        }
        else{
            XIndexAccess xTableIndex = (XIndexAccess) UnoRuntime.queryInterface(XIndexAccess.class, xAllTextTables);
            xTextTable = (XTextTable) UnoRuntime.queryInterface(XTextTable.class, xTableIndex.getByIndex(xTableIndex.getCount()-1));
            xTableName = (XNamed) UnoRuntime.queryInterface(XNamed.class, xTextTable);
            xTableName.setName(ReportDocument.TBLRECORDSECTION);
        }
        xTableRows = xTextTable.getRows();
        xTableColumns = xTextTable.getColumns();
        xCellRange = (XCellRange) UnoRuntime.queryInterface(XCellRange.class, xTextTable);
    }
    catch(Exception exception){
        exception.printStackTrace(System.out);
    }}


    public void adjustOptimalTableWidths(XMultiServiceFactory _xMSF, ViewHandler oViewHandler){     // setTableColumnSeparators(){
        oTextTableHandler.adjustOptimalTableWidths(_xMSF, xTextTable);
    oViewHandler.collapseViewCursorToStart();
    }
}
