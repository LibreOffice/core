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



#ifndef ARY_CPP_CS_CE_HXX
#define ARY_CPP_CS_CE_HXX



// USED SERVICES
    // BASE CLASSES
#include <store/s_storage.hxx>
    // OTHER
#include <ary/cpp/c_ce.hxx>
#include <ary/cpp/c_traits.hxx>
#include <sortedids.hxx>



namespace ary
{
namespace cpp
{


/** The data base for all ->ary::cpp::CodeEntity objects.
*/
class Ce_Storage : public ::ary::stg::Storage<CodeEntity>
{
  public:
    typedef SortedIds<Ce_Compare>       Index;

                        Ce_Storage();
    virtual             ~Ce_Storage();

    Ce_id               Store_Type(
                            DYN CodeEntity &    pass_ce );
    Ce_id               Store_Operation(
                            DYN CodeEntity &    pass_ce );
    Ce_id               Store_Datum(
                            DYN CodeEntity &    pass_ce );

    const Index &       TypeIndex() const           { return aTypes; }
    const Index &       OperationIndex() const      { return aOperations; }
    const Index &       DataIndex() const           { return aData; }

    Index &             TypeIndex()                 { return aTypes; }
    Index &             OperationIndex()            { return aOperations; }
    Index &             DataIndex()                 { return aData; }

    static Ce_Storage & Instance_()                 { csv_assert(pInstance_ != 0);
                                                      return *pInstance_; }
  private:
    // DATA
    Index               aTypes;
    Index               aOperations;
    Index               aData;

    static Ce_Storage * pInstance_;
};




namespace predefined
{

enum E_CodeEntity
{
    ce_GlobalNamespace = 1,
    ce_MAX
};

}   // namespace predefined





}   // namespace cpp
}   // namespace ary
#endif
