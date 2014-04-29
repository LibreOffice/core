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



#ifndef ARY_CPP_C_GATE_HXX
#define ARY_CPP_C_GATE_HXX


// USED SERVICES
    // BASE CLASSES
    // OTHER
#include <ary/cpp/c_types4cpp.hxx>



namespace autodoc
{
    class Options;
}
namespace ary
{
    class Entity;

namespace cpp
{
    class CodeEntity;
    class CppEntity;
    class CePilot;
    class DefPilot;
    class TypePilot;
}
namespace loc
{
    class LocationPilot;
}
}





namespace ary
{
namespace cpp
{



/** Access to all stored objcts in the repository, which are
    relevant to C++.
*/
class Gate
{
  public:
    // LIFECYCLE
    virtual             ~Gate() {}


    // OPERATIONS
    virtual void        Calculate_AllSecondaryInformation() = 0;
//                            const ::autodoc::Options &
//                                                i_options ) = 0;

    // INQUIRY
    virtual const String &
                        RepositoryTitle() const = 0;
    virtual const CodeEntity *
                        Search_RelatedCe(
                            Type_id             i_type ) const = 0;
    virtual const ::ary::cpp::CppEntity *
                        Search_Entity(
                            GlobalId            i_id ) const = 0;
    virtual uintt       Get_AlphabeticalList(
                            List_GlobalIds &    o_result,
                            const char *        i_begin,
                            const char *        i_end ) const = 0;
    virtual const CePilot &
                        Ces() const = 0;
    virtual const DefPilot &
                        Defs() const = 0;
    virtual const TypePilot &
                        Types() const = 0;
    virtual const loc::LocationPilot &
                        Locations() const = 0;

    // ACCESS
    virtual CePilot &   Ces() = 0;
    virtual DefPilot &  Defs() = 0;
    virtual TypePilot & Types() = 0;
    virtual loc::LocationPilot &
                        Locations() = 0;
};



}   // namespace cpp
}   // namespace ary
#endif
