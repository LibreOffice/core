/*************************************************************************
 *
 *  $RCSfile: MemberTypeInfo.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 02:51:45 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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


