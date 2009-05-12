/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: TypeInfo.java,v $
 * $Revision: 1.6 $
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
