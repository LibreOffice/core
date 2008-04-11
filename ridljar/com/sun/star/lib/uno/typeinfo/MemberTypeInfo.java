/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: MemberTypeInfo.java,v $
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

public class MemberTypeInfo extends TypeInfo
{
    int m_index;
    private final Type m_unoType; // @since UDK 3.2
    private final int m_typeParameterIndex; // @since UDK 3.2

    /**
       Create a member type info with a UNO type that cannot unambiguously be
       represented as a Java&nbsp;1.2 type.

       @param name the name of this member; must not be <code>null</code>

       @param index the index among the direct members

       @param flags any flags (<code>UNSIGNED</code>, <code>ANY</code>,
       <code>INTERFACE</code>, <code>TYPE_PARAMETER</code>)

       @param unoType the exact UNO type; or <code>null</code> if the UNO type
       is already unambiguously represented by the Java&nbsp;1.2 type

       @param typeParameterIndex the index of the type parameter that determines
       the type of this parameterized member; or <code>-1</code> if this member
       is of an explicit type, or is the member of a plain struct type

       @since UDK 3.2
     */
    public MemberTypeInfo(
        String name, int index, int flags, Type unoType, int typeParameterIndex)
    {
        super(name, flags);
        m_index = index;
        m_unoType = unoType;
        m_typeParameterIndex = typeParameterIndex;
    }

    public MemberTypeInfo(String name, int index, int flags )
    {
        this(name, index, flags, null, -1);
    }

    public int getIndex()
    {
        return m_index;
    }

    /**
       Get the exact UNO type of this member type info, in case it cannot
       unambiguously be represented as a Java&nbsp;1.2 type.

       @return the exact UNO type of this member type info, or <code>null</code>
       if the UNO type is already unambiguously represented by the Java&nbsp;1.2
       type

       @since UDK 3.2
     */
    public final Type getUnoType() {
        return m_unoType;
    }

    /**
       Returns the index of the type parameter that determines the parameterized
       type of this member.

       @return the index of the type parameter that determines the type of this
       parameterized member; if this member is of an explicit type, or is the
       member of a plain struct type, <code>-1</code> is returned

       @since UDK 3.2
     */
    public final int getTypeParameterIndex() {
        return m_typeParameterIndex;
    }
}


