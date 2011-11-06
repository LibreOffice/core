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



#ifndef ARY_CPP_CA_DEF_HXX
#define ARY_CPP_CA_DEF_HXX

// USED SERVICES
    // BASE CLASSES
#include <ary/cpp/cp_def.hxx>
    // OTHER
#include "cs_def.hxx"



namespace ary
{
namespace cpp
{
    class Def_Storage;
    class RepositoryPartition;
}
}





namespace ary
{
namespace cpp
{




class DefAdmin : public DefPilot
{
  public:
    // LIFECYCLE
                        DefAdmin(
                            RepositoryPartition &
                                                io_myReposyPartition );
                        ~DefAdmin();

    // INQUIRY
    const Def_Storage & Storage() const;

    // INHERITED
    // Interface DefPilot:
    virtual Define &    Store_Define(
                            const InputContext& i_rContext,
                            const String  &     i_sName,
                            const StringVector &
                                                i_rDefinition );
    virtual Macro &     Store_Macro(
                            const InputContext& i_rContext,
                            const String  &     i_sName,
                            const StringVector &
                                                i_rParams,
                            const StringVector &
                                                i_rDefinition );
    virtual const DefineEntity &
                        Find_Def(
                            De_id               i_id ) const;
    virtual DefsResultList
                        AllDefines() const;
    virtual DefsResultList
                        AllMacros() const;

  private:
    // DATA
    Def_Storage     aStorage;
    RepositoryPartition *
                    pCppRepositoryPartition;
};




// IMPLEMENTATION
inline const Def_Storage &
DefAdmin::Storage() const
{
    return aStorage;
}



}   // namespace cpp
}   // namespace ary
#endif
