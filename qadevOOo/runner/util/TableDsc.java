/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package util;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.text.XTextTable;
/**
 * the class TableDsc
 */
public class TableDsc extends InstDescr {

    private int rows = 0;
    private int columns = 0;
    private static final String ifcName = "com.sun.star.text.XTextTable";
    private static final String service = "com.sun.star.text.TextTable";

    public TableDsc() {
        initTable();
    }

    public TableDsc( int nRows, int nColumns ) {
        rows = nRows;
        columns = nColumns;
        initTable();
    }

    @Override
    public String getName() {
        return null;
    }
    @Override
    public String getIfcName() {
        return ifcName;
    }
    @Override
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
    @Override
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
