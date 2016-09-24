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

using System;


namespace uno
{
/** is used to mark a UNO entity to have a parameterized type.

    <p>Currently it is only applied to members of  polymorphic structs. That is structs,
    which have a type parameter list.
    </p>

    @see TypeParametersAttribute
 */
[AttributeUsage(AttributeTargets.Field, Inherited=false)]
public sealed class ParameterizedTypeAttribute: System.Attribute
{
    /** initializes an instance with the specified value.

        @param parameter
        the name of parameter from the parameter list from
        <type scope="uno.idl">TypeParametersAttribute</type>
        It must not be null.
     */
    public ParameterizedTypeAttribute(string parameter)
    {
        m_parameter = parameter;
    }

    public string Type
    {
        get
        {
            return m_parameter;
        }
    }

    private string m_parameter;
}

} 

