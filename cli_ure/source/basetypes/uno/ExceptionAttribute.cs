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

