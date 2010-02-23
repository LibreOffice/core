/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

using System;


namespace uno
{
/** is used to mark a UNO method to throw exceptions.

    <p>A method can be an ordinary interface method, a get or set method from
    an interface attribute, or the constructor methods of service creator
    classes. If there are no exceptions specified for a method then this
    attribute should not be applied. Because a
    <type scope="com.sun.star.uno">RuntimeException</type>
    can always be thrown it is not specified. Hence no
    <code>ExceptionAttribute</code>
    should be applied in that case.</p>
 */
[AttributeUsage(AttributeTargets.Method, Inherited=false)]
public sealed class ExceptionAttribute: System.Attribute
{
    /** initializes an instance with the specified values.

        @param raises
        array of types of Exceptions which are declared in UNOIDL.
        It must not be null.
     */
    public ExceptionAttribute(Type[] raises)
    {
        m_raises = raises;
    }

    public Type[] Raises
    {
        get
        {
            return m_raises;
        }
    }

    private Type[] m_raises;
}

} 

