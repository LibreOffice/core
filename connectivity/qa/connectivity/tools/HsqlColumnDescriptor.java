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
