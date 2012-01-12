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
#include <ary/loc/loc_file.hxx>


// NOT FULLY DEFINED SERVICES



namespace ary
{
namespace loc
{

File::File( const String  &     i_sLocalName,
            Le_id               i_nParentDirectory )
    :   FileBase(i_sLocalName, i_nParentDirectory)
{
}

File::~File()
{
}

void
File::do_Accept(csv::ProcessorIfc & io_processor) const
{
    csv::CheckedCall(io_processor, *this);
}

ClassId
File::get_AryClass() const
{
    return class_id;
}



} // namespace loc
} // namespace ary
