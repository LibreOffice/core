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

package connectivity.tools;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameAccess;
import com.sun.star.sdbc.ColumnValue;
import com.sun.star.sdbcx.XColumnsSupplier;
import com.sun.star.sdbcx.XDataDescriptorFactory;
import com.sun.star.sdbcx.XTablesSupplier;
import com.sun.star.uno.UnoRuntime;
import connectivity.tools.sdb.Connection;

/** is a very simply descriptor of a HSQL table, to be used with a HsqlDatabase.createTable method
 */
public class HsqlTableDescriptor
{
    private String                  m_name;
    private HsqlColumnDescriptor[]  m_columns;

    /** Creates a new instance of HsqlTableDescriptor */
    public HsqlTableDescriptor( String _name, HsqlColumnDescriptor[] _columns )
    {
        m_name = _name;
        m_columns = _columns;
    }

    /** returns the name of the table
     */
    public String getName()
    {
        return m_name;
    }

    /** returns the set of column descriptors for the table
     */
    public HsqlColumnDescriptor[] getColumns()
    {
        return m_columns;
    }

    public XPropertySet createSdbcxDescriptor( Connection _forConnection )
    {
        XTablesSupplier suppTables = UnoRuntime.queryInterface( XTablesSupplier.class, _forConnection.getXConnection() );
        XDataDescriptorFactory tableDescFac = UnoRuntime.queryInterface( XDataDescriptorFactory.class, suppTables.getTables() );
        XPropertySet tableDesc = tableDescFac.createDataDescriptor();

        try
        {
            tableDesc.setPropertyValue( "Name", getName() );
        }
        catch ( Exception e ) { e.printStackTrace( System.err ); }

        XColumnsSupplier suppDescCols = UnoRuntime.queryInterface( XColumnsSupplier.class, tableDesc );

        XNameAccess descColumns = suppDescCols.getColumns();
        XDataDescriptorFactory columnDescFac = UnoRuntime.queryInterface( XDataDescriptorFactory.class, descColumns );

        HsqlColumnDescriptor[] myColumns = getColumns();
        for ( int i = 0; i < myColumns.length; ++i )
        {
            XPropertySet columnDesc = columnDescFac.createDataDescriptor();
            try
            {
                columnDesc.setPropertyValue( "Name", myColumns[i].getName() );
                columnDesc.setPropertyValue( "IsNullable", new Integer( myColumns[i].isRequired() ? ColumnValue.NO_NULLS : ColumnValue.NULLABLE) );
                columnDesc.setPropertyValue( "TypeName", myColumns[i].getTypeName() );
                if ( myColumns[i].isPrimaryKey() || myColumns[i].isForeignKey() )
                    // not yet implemented
                    throw new java.lang.UnsupportedOperationException("creating a primary or foreign key via SDBCX not yet implemented" );
            }
            catch( com.sun.star.uno.Exception e ) { e.printStackTrace( System.err ); }
        }

        return tableDesc;
    }
}
