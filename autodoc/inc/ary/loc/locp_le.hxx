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



#ifndef ARY_LOCP_LE_HXX
#define ARY_LOCP_LE_HXX


// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
#include <cosv/ploc.hxx>
#include <ary/loc/loc_types4loc.hxx>


namespace ary
{
namespace loc
{
    class Root;
    class Directory;
    class File;
}
}


namespace ary
{
namespace loc
{



/** Provides access to files and directories stored in the
    repository.
*/
class LocationPilot
{
  public:
    virtual             ~LocationPilot() {}

    virtual Root &      CheckIn_Root(
                            const csv::ploc::Path &
                                                i_rPath ) = 0;
    virtual File &      CheckIn_File(
                            const String  &     i_name,
                            const csv::ploc::DirectoryChain &
                                                i_subPath,
                            Le_id               i_root ) = 0;

    virtual Root &      Find_Root(
                            Le_id               i_id ) const = 0;
    virtual Directory & Find_Directory(
                            Le_id               i_id ) const = 0;
    virtual File &      Find_File(
                            Le_id               i_id ) const = 0;
};




}   // namespace loc
}   // namespace ary
#endif
