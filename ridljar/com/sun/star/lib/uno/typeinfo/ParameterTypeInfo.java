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
package com.sun.star.lib.uno.typeinfo;

import com.sun.star.uno.Type;

public class ParameterTypeInfo extends TypeInfo
{
    protected int       m_index;
    protected String    m_methodName;
    private final Type m_unoType; // @since UDK 3.2

    /**
       Create a parameter type info with a UNO type that cannot unambiguously be
       represented as a Java&nbsp;1.2 type.

       @param name the name of this parameter; must not be <code>null</code>

       @param methodName the name of the method; must not be <code>null</code>

       @param index the index among the parameters

       @param flags any flags (<code>IN</code>, <code>OUT</code>,
       <code>UNSIGNED</code>, <code>ANY</code>, <code>INTERFACE</code>)

       @param unoType the exact UNO type; or <code>null</code> if the UNO type
       is already unambiguously represented by the Java&nbsp;1.2 type

       @since UDK 3.2
     */
    public ParameterTypeInfo(
        String name, String methodName, int index, int flags, Type unoType)
    {
        super(name, flags);
        m_index = index;
        m_methodName = methodName;
        m_unoType = unoType;
    }

    public ParameterTypeInfo(String name, String methodName, int index, int flags)
    {
        this(name, methodName, index, flags, null);
    }

    public String getMethodName()
    {
        return m_methodName;
    }

    public int getIndex()
    {
        return m_index;
    }

    public boolean isIN()
    {
        return ((m_flags & TypeInfo.IN) != 0 ||
                (m_flags & (TypeInfo.IN | TypeInfo.OUT)) == 0); // nothing set => IN
    }

    public boolean isOUT()
    {
        return (m_flags & TypeInfo.OUT) != 0;
    }

    public boolean isINOUT()
    {
        return (m_flags & (TypeInfo.IN | TypeInfo.OUT)) == (TypeInfo.IN | TypeInfo.OUT);
    }

    /**
       Get the exact UNO type of this parameter type info, in case it cannot
       unambiguously be represented as a Java&nbsp;1.2 type.

       <p>If this is an out or in&ndash;out parameter, the UNO type must be a
       sequence type, taking into account that such a parameter is represented
       in Java as a parameter of array type.</p>

       @return the exact UNO type of this parameter type info, or
       <code>null</code> if the UNO type is already unambiguously represented by
       the Java&nbsp;1.2 type

       @since UDK 3.2
     */
    public final Type getUnoType() {
        return m_unoType;
    }
}


