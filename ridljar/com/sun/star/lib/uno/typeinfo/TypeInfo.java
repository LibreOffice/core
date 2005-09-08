/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TypeInfo.java,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 13:19:47 $
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
package com.sun.star.lib.uno.typeinfo;


/** Defines a class to describe additional type information.
 */
public class TypeInfo
{
    public static final int IN          = 0x00000001;
    public static final int OUT         = 0x00000002;
    public static final int UNSIGNED    = 0x00000004;
    public static final int READONLY    = 0x00000008;
    public static final int ONEWAY      = 0x00000010;
    public static final int CONST       = 0x00000020;
    public static final int ANY         = 0x00000040;
    public static final int INTERFACE   = 0x00000080;

    /**
       Marks an extended attribute of an interface type as bound.

       <p>Only used in the <code>flags</code> argument of the
       <code>AttributeTypeInfo</code> constructors.</p>

       @since UDK 3.2
     */
    public static final int BOUND = 0x00000100;

    protected int       m_flags;
    protected String    m_name;

    public TypeInfo(String name, int flags)
    {
        m_name = name;
        m_flags = flags;
    }

    public String getName()
    {
        return m_name;
    }

    public int getFlags() {
        return m_flags;
    }

    public boolean isUnsigned()
    {
        return (m_flags & TypeInfo.UNSIGNED) != 0;
    }

    public boolean isAny()
    {
        return (m_flags & TypeInfo.ANY) != 0;
    }

    public boolean isInterface()
    {
        return (m_flags & TypeInfo.INTERFACE) != 0;
    }
}
