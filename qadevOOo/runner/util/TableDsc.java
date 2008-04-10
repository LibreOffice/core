/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: TableDsc.java,v $
 * $Revision: 1.3 $
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

package util;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextTable;
/**
 * the class TableDsc
 */
public class TableDsc extends InstDescr {

    private int rows = 0;
    private int columns = 0;
    private String name = null;
    final String ifcName = "com.sun.star.text.XTextTable";
    final String service = "com.sun.star.text.TextTable";

    public TableDsc() {
        initTable();
    }

    public TableDsc( int nRows, int nColumns ) {
        rows = nRows;
        columns = nColumns;
        initTable();
    }

    public TableDsc( String TableName, int nRows, int nColumns ) {
        name = TableName;
        rows = nRows;
        columns = nColumns;
        initTable();
    }
    public String getName() {
        return name;
    }
    public String getIfcName() {
        return ifcName;
    }
    public String getService() {
        return service;
    }

    private void initTable() {
        try {
               ifcClass = Class.forName( ifcName );
        }
        catch( ClassNotFoundException cnfE ) {
        }
    }
    public XInterface createInstance( XMultiServiceFactory docMSF ) {
        Object SrvObj = null;
        try {
            SrvObj = docMSF.createInstance( service );
        }
        catch( com.sun.star.uno.Exception cssuE ){
        }

        XTextTable TT = (XTextTable)UnoRuntime.queryInterface(
                                                        ifcClass, SrvObj );

        if ( rows > 0 && columns > 0 )  {
            TT.initialize( rows, columns );
        }

        return TT;

    }
}