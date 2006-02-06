/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: HsqlColumnDescriptor.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2006-02-06 16:43:01 $
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
    public String Name;
    public String TypeName;
    public boolean NotNull;
    public boolean PrimaryKey;

    public HsqlColumnDescriptor( String _Name, String _TypeName )
    {
        Name = _Name;
        TypeName = _TypeName;
        NotNull = false;
        PrimaryKey = false;
    }

    public HsqlColumnDescriptor( String _Name, String _TypeName, boolean _NotNull )
    {
        Name = _Name;
        TypeName = _TypeName;
        NotNull = _NotNull;
        PrimaryKey = false;
    }

    public HsqlColumnDescriptor( String _Name, String _TypeName, boolean _NotNull, boolean _PrimaryKey )
    {
        Name = _Name;
        TypeName = _TypeName;
        NotNull = _NotNull;
        PrimaryKey = _PrimaryKey;
    }
};
