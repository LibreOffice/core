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

#include <precomp.h>
#include <ary/cpp/c_define.hxx>


// NOT FULLY DECLARED SERVICES
#include <prprpr.hxx>




namespace ary
{
namespace cpp
{



Define::Define( const String  &         i_name,
                const StringVector &    i_definition,
                loc::Le_id              i_declaringFile)
    :   DefineEntity(i_name, i_declaringFile),
        aDefinition(i_definition)
{
}

Define::~Define()
{
}

void
Define::do_Accept(csv::ProcessorIfc & io_processor) const
{
    csv::CheckedCall(io_processor, *this);
}

ClassId
Define::get_AryClass() const
{
    return class_id;
}

const StringVector &
Define::inq_DefinitionText() const
{
     return aDefinition;
}

}   // namespace cpp
}   // namespace ary
