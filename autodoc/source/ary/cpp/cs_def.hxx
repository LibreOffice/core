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



#ifndef ARY_CPP_CS_DE_HXX
#define ARY_CPP_CS_DE_HXX



// USED SERVICES
    // BASE CLASSES
#include <store/s_storage.hxx>
    // OTHER
#include <ary/cpp/c_de.hxx>
#include <ary/cpp/c_traits.hxx>
#include <sortedids.hxx>



namespace ary
{
namespace cpp
{




/** The data base for all ->ary::cpp::Type objects.
*/
class Def_Storage : public ::ary::stg::Storage<DefineEntity>
{
  public:
    typedef SortedIds<Def_Compare>      Index;

    // LIFECYCLE
                        Def_Storage();
    virtual             ~Def_Storage();

    De_id               Store_Define(
                            DYN DefineEntity &  pass_de );
    De_id               Store_Macro(
                            DYN DefineEntity &  pass_de );

    const Index &       DefineIndex() const     { return aDefines; }
    const Index &       MacroIndex() const      { return aMacros; }

    Index &             DefineIndex()           { return aDefines; }
    Index &             MacroIndex()            { return aMacros; }

    static Def_Storage &
                        Instance_()             { csv_assert(pInstance_ != 0);
                                                  return *pInstance_; }
  private:
    // DATA
    Index               aDefines;
    Index               aMacros;


    static Def_Storage *
                        pInstance_;
};




namespace predefined
{

enum E_DefineEntity
{
    // 0 is always unused with repository storages.
    de_MAX = 1
};

}   // namespace predefined






}   // namespace cpp
}   // namespace ary
#endif
