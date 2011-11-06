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



#ifndef ARY_CPP_C_REPOSYPART_HXX
#define ARY_CPP_C_REPOSYPART_HXX



// BASE CLASSES
#include <cpp_internalgate.hxx>

namespace ary
{
namespace cpp
{
    class CeAdmin;
    class DefAdmin;
    class TypeAdmin;
}
}




namespace ary
{
namespace cpp
{



/** The C++ partition of the repository.
*/
class RepositoryPartition : public InternalGate
{
  public:
                        RepositoryPartition(
                            RepositoryCenter &  i_reposyImpl );
    virtual             ~RepositoryPartition();

    // INHERITED
        // Interface Gate:
    virtual void        Calculate_AllSecondaryInformation();
//                            const ::autodoc::Options &
//                                                i_options );
    virtual const String &
                        RepositoryTitle() const;
    virtual const CodeEntity *
                        Search_RelatedCe(
                            Type_id             i_type ) const;
    virtual const ::ary::cpp::CppEntity *
                        Search_Entity(
                            GlobalId            i_id ) const;
    virtual uintt       Get_AlphabeticalList(
                            List_GlobalIds &    o_result,
                            const char *        i_begin,
                            const char *        i_end ) const;
    virtual const CePilot &
                        Ces() const;
    virtual const DefPilot &
                        Defs() const;
    virtual const TypePilot &
                        Types() const;
    virtual const loc::LocationPilot &
                        Locations() const;
    virtual CePilot &   Ces();
    virtual DefPilot &  Defs();
    virtual TypePilot & Types();
    virtual loc::LocationPilot &
                        Locations();
  private:
    // Locals
    void                Connect_AllTypes_2_TheirRelated_CodeEntites();

    // DATA
    RepositoryCenter *  pRepositoryCenter;

    Dyn<CeAdmin>        pCes;
    Dyn<TypeAdmin>      pTypes;
    Dyn<DefAdmin>       pDefs;
    Dyn<loc::LocationPilot>
                        pLocations;
};




}   //  namespace cpp
}   //  namespace ary
#endif
