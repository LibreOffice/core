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
#include <ary/cessentl.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/cpp/c_ce.hxx>
#include <ary/doc/d_oldcppdocu.hxx>


namespace ary
{
namespace cpp
{


CeEssentials::CeEssentials()
    :   sLocalName(),
        nOwner(0),
        nLocation(0)
{
}

CeEssentials::CeEssentials( const String  &     i_sLocalName,
                            Cid                 i_nOwner,
                            loc::Le_id          i_nLocation )
    :   sLocalName(i_sLocalName),
        nOwner(i_nOwner),
        nLocation(i_nLocation)
{
}

CeEssentials::~CeEssentials()
{
}



inline bool
IsInternal(const doc::Documentation & i_doc)
{
    const ary::doc::OldCppDocu *
        docu = dynamic_cast< const ary::doc::OldCppDocu* >(i_doc.Data());
    if (docu != 0)
        return docu->IsInternal();
    return false;
}


bool
CodeEntity::IsVisible() const
{
    // KORR_FUTURE:   Improve the whole handling of internal and visibility.
    return bIsVisible && NOT IsInternal(Docu());
}



}   // namespace cpp
}   // namespace ary
