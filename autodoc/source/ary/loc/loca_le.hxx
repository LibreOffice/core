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



#ifndef ARY_LOC_LOCA_LE_HXX
#define ARY_LOC_LOCA_LE_HXX

// BASE CLASSES
#include <ary/loc/locp_le.hxx>

namespace ary
{
namespace loc
{
    class Le_Storage;
}
}




namespace ary
{
namespace loc
{


/** Provides access to files and directories stored in the
    repository.
*/
class LocationAdmin : public LocationPilot
{
  public:
                        LocationAdmin();
    virtual              ~LocationAdmin();

    // INHERITED
        // Interface LocationPilot:
    virtual Root &      CheckIn_Root(
                            const csv::ploc::Path &
                                                i_rPath );
    virtual File &      CheckIn_File(
                            const String  &     i_name,
                            const csv::ploc::DirectoryChain &
                                                i_subPath,
                            Le_id               i_root );

    virtual Root &      Find_Root(
                            Le_id               i_id ) const;
    virtual Directory & Find_Directory(
                            Le_id               i_id ) const;
    virtual File &      Find_File(
                            Le_id               i_id ) const;
  private:
    // Locals
    Le_Storage &        Storage() const;
    Directory &         CheckIn_Directory(
                            Directory &         io_parent,
                            const String &      i_name );
    Directory &         CheckIn_Directories(
                            Directory &         io_root,
                            StringVector::const_iterator
                                                i_beginSubPath,
                            StringVector::const_iterator
                                                i_endSubPath );
    // DATA
    Dyn<Le_Storage>     pStorage;
};




}   // namespace loc
}   // namespace ary
#endif
