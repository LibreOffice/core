/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MethodTypeInfo.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 13:19:10 $
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

import com.sun.star.uno.Type;

public class MethodTypeInfo extends TypeInfo
{
    protected int m_index;
    private final Type m_unoType; // @since UDK 3.2

    /**
       Create a method type info with a UNO return type that cannot
       unambiguously be represented as a Java&nbsp;1.2 type.

       @param name the name of this method; must not be <code>null</code>

       @param index the index among the direct members

       @param flags any flags (<code>ONEWAY</code>, <code>UNSIGNED</code>,
       <code>ANY</code>, <code>INTERFACE</code>)

       @param unoType the exact UNO return type; or <code>null</code> if the UNO
       type is already unambiguously represented by the Java&nbsp;1.2 type

       @since UDK 3.2
     */
    public MethodTypeInfo(String name, int index, int flags, Type unoType) {
        super(name, flags);
        m_index = index;
        m_unoType = unoType;
    }

    public MethodTypeInfo(String name, int index, int flags)
    {
        this(name, index, flags, null);
    }

    public int getIndex()
    {
        return m_index;
    }

    public boolean isReturnUnsigned()
    {
        return isUnsigned();
    }

    public boolean isOneway()
    {
        return (m_flags & TypeInfo.ONEWAY) != 0;
    }

    public boolean isConst()
    {
        return (m_flags & TypeInfo.CONST) != 0;
    }

    /**
       Get the exact UNO return type of this method type info, in case it cannot
       unambiguously be represented as a Java&nbsp;1.2 type.

       @return the exact UNO return type of this method type info, or
       <code>null</code> if the UNO type is already unambiguously represented by
       the Java&nbsp;1.2 type

       @since UDK 3.2
     */
    public final Type getUnoType() {
        return m_unoType;
    }
}


