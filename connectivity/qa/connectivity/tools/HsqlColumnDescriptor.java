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
}
