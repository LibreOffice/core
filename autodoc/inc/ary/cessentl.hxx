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



#ifndef ARY_CESSENTL_HXX
#define ARY_CESSENTL_HXX
//  KORR_DEPRECATED_3.0


// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
#include <ary/cpp/c_types4cpp.hxx>
#include <ary/loc/loc_types4loc.hxx>



namespace ary
{
namespace cpp
{


class CeEssentials
{   // Non inline functions are implemented in ceworker.cxx .
  public:
    // LIFECYCLE
                        CeEssentials();
                        CeEssentials(
                            const String  &     i_sLocalName,
                            Ce_id               i_nOwner,
                            loc::Le_id          i_nLocation );
                        ~CeEssentials();
    // INQUIRY
    const String  &     LocalName() const;
    Ce_id               Owner() const;
    loc::Le_id          Location() const;

    // ACCESS
  private:
    String              sLocalName;
    Ce_id               nOwner;
    loc::Le_id          nLocation;
};



// IMPLEMENTATION
inline const String     &
CeEssentials::LocalName() const
    { return sLocalName; }
inline Ce_id
CeEssentials::Owner() const
    { return nOwner; }
inline loc::Le_id
CeEssentials::Location() const
    { return nLocation; }




}   // namespace cpp
}   // namespace ary
#endif
