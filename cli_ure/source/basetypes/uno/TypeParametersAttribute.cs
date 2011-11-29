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



using System;


namespace uno
{
/** is used to mark a UNO entity to have type parameters.

    <p>Currently it is only applied with polymorphic structs. That is structs,
    which have a type parameter list.
    </p>
 */
[AttributeUsage(AttributeTargets.Class, Inherited=false)]
public sealed class TypeParametersAttribute: System.Attribute
{
    /** initializes an instance with the specified value.

        @param parameters
        array of names representing the types.
        It must not be null.
     */
    public TypeParametersAttribute(string[] parameters)
    {
        m_parameters = parameters;
    }

    public string[] Parameters
    {
        get
        {
            return m_parameters;
        }
    }

    private string[] m_parameters;
}

} 

