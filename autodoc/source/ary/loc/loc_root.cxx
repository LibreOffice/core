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
#include <ary/loc/loc_root.hxx>


// NOT FULLY DEFINED SERVICES


namespace ary
{
namespace loc
{


Root::Root(const csv::ploc::Path & i_path)
    :   aPath(i_path),
        sPathAsString(),
        aMyDirectory(0)
{
    StreamLock
        path_string(700);
    path_string() << i_path;
    sPathAsString = path_string().c_str();
}

Root::~Root()
{
}

void
Root::do_Accept(csv::ProcessorIfc & io_processor) const
{
    csv::CheckedCall(io_processor,*this);
}

ClassId
Root::get_AryClass() const
{
    return class_id;
}

const String &
Root::inq_LocalName() const
{
    return sPathAsString;
}

Le_id
Root::inq_ParentDirectory() const
{
    return Le_id::Null_();
}



} // namespace loc
} // namespace ary
