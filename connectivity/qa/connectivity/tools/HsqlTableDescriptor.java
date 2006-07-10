/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: HsqlTableDescriptor.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 14:18:19 $
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

package connectivity.tools;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameAccess;
import com.sun.star.sdbc.ColumnValue;
import com.sun.star.sdbc.XConnection;
import com.sun.star.sdbcx.XColumnsSupplier;
import com.sun.star.sdbcx.XDataDescriptorFactory;
import com.sun.star.sdbcx.XTablesSupplier;
import com.sun.star.uno.UnoRuntime;

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

    public XPropertySet createSdbcxDescriptor( XConnection _forConnection )
    {
        XTablesSupplier suppTables = (XTablesSupplier)UnoRuntime.queryInterface(
            XTablesSupplier.class, _forConnection );
        XDataDescriptorFactory tableDescFac = (XDataDescriptorFactory)UnoRuntime.queryInterface(
            XDataDescriptorFactory.class, suppTables.getTables() );
        XPropertySet tableDesc = tableDescFac.createDataDescriptor();

        try
        {
            tableDesc.setPropertyValue( "Name", getName() );
        }
        catch ( Exception e ) { e.printStackTrace( System.err ); }

        XColumnsSupplier suppDescCols = (XColumnsSupplier)UnoRuntime.queryInterface(
            XColumnsSupplier.class, tableDesc );

        XNameAccess descColumns = suppDescCols.getColumns();
        XDataDescriptorFactory columnDescFac = (XDataDescriptorFactory)UnoRuntime.queryInterface(
            XDataDescriptorFactory.class, descColumns );

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
