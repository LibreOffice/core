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
/** is used to mark UNO interface attributes as &quot;bound&quot.

    <p>It corresponds to <const
    scope="com::sun::star::beans">PropertyAttribute::BOUND</const>.
    </p>
    UNO interface attributes are mapped to CLI properties.
 */
[AttributeUsage(AttributeTargets.Property, Inherited=false)]
public sealed class BoundAttribute: System.Attribute
{
}

}

