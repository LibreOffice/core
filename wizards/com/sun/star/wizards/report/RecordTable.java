/*************************************************************************
 *
 *  RCSfile: RecordTable.java,v
 *
 *  Revision: 1.0
 *
 *  last change: Author: bc  Date: 08.03.2004 12:57:38
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright (c) 2003 by Sun Microsystems, Inc.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/
package com.sun.star.wizards.report;

import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNamed;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.table.XCellRange;
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


    public RecordTable(TextTableHandler _oTextTableHandler){
    try{
        this.oTextTableHandler = _oTextTableHandler;
        String[] TableNames = oTextTableHandler.xTextTablesSupplier.getTextTables().getElementNames();
        XNameAccess xAllTextTables = oTextTableHandler.xTextTablesSupplier.getTextTables();
        if (xAllTextTables.hasByName("Tbl_RecordSection") == false){
            XIndexAccess xTableIndex = (XIndexAccess) UnoRuntime.queryInterface(XIndexAccess.class, xAllTextTables);
            xTextTable = (XTextTable) xTableIndex.getByIndex(xTableIndex.getCount()-1);
            xTableName = (XNamed) UnoRuntime.queryInterface(XNamed.class, xTextTable);
            xTableName.setName("Tbl_RecordSection");
        }
        else{
            Object oTable = xAllTextTables.getByName("Tbl_RecordSection");
            xTextTable = (XTextTable) UnoRuntime.queryInterface(XTextTable.class, oTable);
            xTableName = (XNamed) UnoRuntime.queryInterface(XNamed.class, xTextTable);
        }
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
