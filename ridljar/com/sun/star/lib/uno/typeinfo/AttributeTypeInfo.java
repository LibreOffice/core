/*************************************************************************
 *
 *  $RCSfile: AttributeTypeInfo.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2004-06-04 02:51:15 $
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
