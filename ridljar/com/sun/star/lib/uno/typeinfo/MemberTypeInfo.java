/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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


