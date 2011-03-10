/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
            XRow xRow = (XRow) UnoRuntime.queryInterface(XRow.class, xResultSet);
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
            XRow xRow = (XRow) UnoRuntime.queryInterface(XRow.class, xResultSet);
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
