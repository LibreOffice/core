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



#ifndef ARY_LOC_LOCS_LE_HXX
#define ARY_LOC_LOCS_LE_HXX

// BASE CLASSES
#include <store/s_storage.hxx>
// USED SERVICES
#include <cosv/tpl/tpltools.hxx>
#include <ary/loc/loc_le.hxx>
#include <ary/loc/loc_root.hxx>
#include <sortedids.hxx>




namespace ary
{
namespace loc
{


/** The data base for all ->ary::cpp::CodeEntity objects.
*/
class Le_Storage : public ::ary::stg::Storage<LocationEntity>
{
  public:
    typedef SortedIds<Le_Compare>           Index;

                        Le_Storage();
    virtual             ~Le_Storage();

    const Index &       RootIndex() const         { return aRoots; }
    Index &             RootIndex()               { return aRoots; }

    static Le_Storage & Instance_()               { csv_assert(pInstance_ != 0);
                                                    return *pInstance_; }
  private:
    // DATA
    Index               aRoots;

    static Le_Storage * pInstance_;
};




namespace predefined
{

enum E_LocationEntity
{
    le_MAX = 1
};

}   // namespace predefined




}   // namespace cpp
}   // namespace ary
#endif
