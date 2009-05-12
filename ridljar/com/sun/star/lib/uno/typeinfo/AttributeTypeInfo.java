/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: AttributeTypeInfo.java,v $
 * $Revision: 1.5 $
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

import com.sun.star.uno.Type;

public class AttributeTypeInfo extends TypeInfo
{
    protected int m_index;
    private final Type m_unoType; // @since UDK 3.2

    /**
       Create an attribute type info with a UNO type that cannot unambiguously
       be represented as a Java&nbsp;1.2 type.

       @param name the name of this attribute; must not be <code>null</code>

       @param index the index among the direct members

       @param flags any flags (<code>READONLY</code>, <code>BOUND</code>,
       <code>UNSIGNED</code>, <code>ANY</code>, <code>INTERFACE</code>)

       @param unoType the exact UNO type; or <code>null</code> if the UNO type
       is already unambiguously represented by the Java&nbsp;1.2 type

       @since UDK 3.2
     */
    public AttributeTypeInfo(String name, int index, int flags, Type unoType) {
        super(name, flags);
        m_index = index;
        m_unoType = unoType;
    }

    public AttributeTypeInfo(String name, int index, int flags)
    {
        this(name, index, flags, null);
    }

    public int getIndex()
    {
        return m_index;
    }

    public boolean isReadOnly()
    {
        return (m_flags & TypeInfo.READONLY) != 0;
    }

    /**
       Returns the status of the 'bound' flag.

       @since UDK 3.2
     */
    public final boolean isBound() {
        return (m_flags & TypeInfo.BOUND) != 0;
    }

    /**
       Get the exact UNO type of this attribute type info, in case it cannot
       unambiguously be represented as a Java&nbsp;1.2 type.

       @return the exact UNO type of this attribute type info, or
       <code>null</code> if the UNO type is already unambiguously represented by
       the Java&nbsp;1.2 type

       @since UDK 3.2
     */
    public final Type getUnoType() {
        return m_unoType;
    }
}
