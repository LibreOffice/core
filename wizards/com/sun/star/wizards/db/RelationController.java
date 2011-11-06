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


package com.sun.star.wizards.db;

import java.util.Vector;

import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.sdbc.XRow;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.wizards.common.JavaTools;
import com.sun.star.wizards.common.PropertyNames;

/**
 * @author bc93774
 *
 * TODO To change the template for this generated type comment go to
 * Window - Preferences - Java - Code Style - Code Templates
 */
public class RelationController extends CommandName
{

    private int PKTABLE_CAT = 1;
    private int PKTABLE_SCHEM = 2;
    private int PKTABLE_NAME = 3;
    private int PKCOLUMN_NAME = 4;
    private int FKTABLE_CAT = 5;
    private int FKTABLE_SCHEM = 6;
    private int FKTABLE_NAME = 7;
    private int FKCOLUMN_NAME = 8;

    public RelationController(CommandMetaData _CommandMetaData, String _CatalogName, String _SchemaName, String _TableName, boolean _baddQuotation)
    {
        super(_CommandMetaData, _CatalogName, _SchemaName, _TableName, _baddQuotation);
    }

    public RelationController(CommandMetaData _CommandMetaData, String _DisplayName)
    {
        super(_CommandMetaData, _DisplayName);
    }

    public String[] getExportedKeys()
    {
        String[] sReferencedTableNames = new String[]
        {
        };
        try
        {
            String[] sTableNames = super.getCommandMetaData().getTableNames();
            Vector aReferencedTableVector = new Vector();
            XResultSet xResultSet = super.getCommandMetaData().xDBMetaData.getExportedKeys((getCatalogName(this)), getSchemaName(), getTableName());
            XRow xRow = UnoRuntime.queryInterface(XRow.class, xResultSet);
            while (xResultSet.next())
            {
                String sForeignCatalog = xRow.getString(FKTABLE_CAT);
                String sForeignScheme = xRow.getString(FKTABLE_SCHEM);
                String sForeignTableName = xRow.getString(FKTABLE_NAME);
                CommandName oCommandName = new CommandName(getCommandMetaData(), sForeignCatalog, sForeignScheme, sForeignTableName, false);
                aReferencedTableVector.add(oCommandName.getComposedName());
            }
            sReferencedTableNames = new String[aReferencedTableVector.size()];
            aReferencedTableVector.toArray(sReferencedTableNames);
        }
        catch (SQLException e)
        {
            e.printStackTrace(System.out);
        }
        return sReferencedTableNames;
    }

    private Object getCatalogName(CommandName _oCommandName)
    {
        String sLocCatalog = _oCommandName.getCatalogName();
        if (sLocCatalog.equals(PropertyNames.EMPTY_STRING))
        {
            return null;
        }
        else
        {
            return sLocCatalog;
        }
    }

    public String[][] getImportedKeyColumns(String _sreferencedtablename)
    {
        String[][] sKeyColumnNames = new String[][]
        {
        };
        try
        {
            CommandName oLocCommandName = new CommandName(super.getCommandMetaData(), _sreferencedtablename);
            XResultSet xResultSet = super.getCommandMetaData().xDBMetaData.getImportedKeys(getCatalogName(oLocCommandName), oLocCommandName.getSchemaName(), oLocCommandName.getTableName());
            XRow xRow = UnoRuntime.queryInterface(XRow.class, xResultSet);
            boolean bleaveLoop = false;
            Vector aMasterFieldNamesVector = new Vector();
            Vector aSlaveFieldNamesVector = new Vector();
            while (xResultSet.next() && !bleaveLoop)
            {
                String sPrimaryCatalog = null;
                String sPrimarySchema = null;
                if (super.getCommandMetaData().xDBMetaData.supportsCatalogsInDataManipulation())
                {
                    sPrimaryCatalog = xRow.getString(PKTABLE_CAT);
                }
                if (super.getCommandMetaData().xDBMetaData.supportsSchemasInDataManipulation())
                {
                    sPrimarySchema = xRow.getString(PKTABLE_SCHEM);
                }
                String sPrimaryTableName = xRow.getString(PKTABLE_NAME);
                String sPrimaryColumnName = xRow.getString(PKCOLUMN_NAME);
                String sForeignColumnName = xRow.getString(FKCOLUMN_NAME);
                if (JavaTools.isSame(getTableName(), sPrimaryTableName))
                {
                    if (sPrimarySchema == null || JavaTools.isSame(getSchemaName(), sPrimarySchema))
                    {
                        if (JavaTools.isSame(getCatalogName(), sPrimaryCatalog))
                        {
                            aSlaveFieldNamesVector.add(sForeignColumnName);
                            aMasterFieldNamesVector.add(sPrimaryColumnName);
                            bleaveLoop = true;                  //Only one relation may exist between two tables...
                        }
                    }

                }
            }
            sKeyColumnNames = new String[2][aMasterFieldNamesVector.size()];
            sKeyColumnNames[0] = new String[aSlaveFieldNamesVector.size()];
            sKeyColumnNames[1] = new String[aMasterFieldNamesVector.size()];
            aSlaveFieldNamesVector.toArray(sKeyColumnNames[0]);
            aMasterFieldNamesVector.toArray(sKeyColumnNames[1]);
        }
        catch (Exception e)
        {
            e.printStackTrace(System.out);
        }
        return sKeyColumnNames;
    }
}
