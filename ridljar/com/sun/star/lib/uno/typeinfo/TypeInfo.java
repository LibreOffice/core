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
