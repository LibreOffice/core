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
/** is used to mark a parameterized UNO entity(i.e. struct)
    to be an instantiation of a
    template with the specified type arguments.
    
    <p>Currently only UNO structs can have type parameters.</p>

    <pre>

    [TypeParameters(new string[]{"T"})]
    struct Foo {
      [ParameterizedType("T")]
      Object member;
    }

    public interface XFoo {
       [return:TypeArguments(new string[]{typeof(char)})]
       Foo func( [TypeArguments(new string[]{typeof(char)})] Foo f);
    }
    </pre>

    @see TypeParametersAttribute
    @see ParameterizedTypeAttribute
 */
[AttributeUsage(AttributeTargets.ReturnValue
                | AttributeTargets.Parameter
                | AttributeTargets.Field, Inherited=false)]
public sealed class TypeArgumentsAttribute: System.Attribute
{
    /** initializes an instance with the specified value.

        @param parameters
        array of names representing the types.
        It must not be null.
     */
    public TypeArgumentsAttribute(Type[] arguments)
    {
        m_arguments = arguments;
    }

    public Type[] Arguments
    {
        get
        {
            return m_arguments;
        }
    }

    private Type[] m_arguments;
}

} 

