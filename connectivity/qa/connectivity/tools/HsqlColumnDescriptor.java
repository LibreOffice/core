/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: HsqlColumnDescriptor.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 14:17:55 $
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

/** is a very simply and rudimentary descriptor of table columns, for creating HSQLDB tables
 */
public class HsqlColumnDescriptor
{
    private String Name;
    private String TypeName;
    private boolean Required;
    private boolean PrimaryKey;
    private String ForeignTable;
    private String ForeignColumn;

    public final String getName() { return Name; }
    public final String getTypeName() { return TypeName; }
    public final boolean isRequired() { return Required; }
    public final boolean isPrimaryKey() { return PrimaryKey; }

    public final boolean isForeignKey() { return ( ForeignTable.length() != 0 ) && ( ForeignColumn.length() != 0 ); }
    public final String getForeignTable() { return ForeignTable; }
    public final String getForeignColumn() { return ForeignColumn; }

    /// determines that a column is required, i.e. not nullable
    public final static int REQUIRED    = 1;
    /// determines that a column is part of the primary key of its table
    public final static int PRIMARY     = 2;

    public HsqlColumnDescriptor( String _Name, String _TypeName )
    {
        Name = _Name;
        TypeName = _TypeName;
        Required = false;
        PrimaryKey = false;
        ForeignTable = "";
        ForeignColumn = "";
    }

    public HsqlColumnDescriptor( String _Name, String _TypeName, int _Flags )
    {
        Name = _Name;
        TypeName = _TypeName;
        Required = ( _Flags & REQUIRED ) != 0;
        PrimaryKey = ( _Flags & PRIMARY ) != 0;
        ForeignTable = "";
        ForeignColumn = "";
    }

    public HsqlColumnDescriptor( String _Name, String _TypeName, int _Flags, String _ForeignTable, String _ForeignColumn )
    {
        Name = _Name;
        TypeName = _TypeName;
        Required = ( _Flags & REQUIRED ) != 0;
        PrimaryKey = ( _Flags & PRIMARY ) != 0;
        ForeignTable = _ForeignTable;
        ForeignColumn = _ForeignColumn;
    }
};
