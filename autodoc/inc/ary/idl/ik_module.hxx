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



#ifndef ARY_IDL_IK_MODULE_HXX
#define ARY_IDL_IK_MODULE_HXX



// USED SERVICES
    // BASE CLASSES
#include <ary/idl/ik_ce.hxx>
    // COMPONENTS
    // PARAMETERS


namespace ary
{
namespace idl
{

class CePilot;

namespace ifc_module
{

using ifc_ce::Dyn_CeIterator;
using ifc_ce::DocText;


struct attr : public ifc_ce::attr
{
    // KORR_FUTURE
    //   This has to be changed that way, that the differencing takes place
    //   within hfi_module.cxx and not here.
    //   So the class CePilot is not needed here, etc.
    //   Too much scope pollution.
    static void         Get_AllChildrenSeparated(
                            std::vector< const CodeEntity* > & o_nestedModules,
                            std::vector< const CodeEntity* > & o_services,
                            std::vector< const CodeEntity* > & o_interfaces,
                            std::vector< const CodeEntity* > & o_structs,
                            std::vector< const CodeEntity* > & o_exceptions,
                            std::vector< const CodeEntity* > & o_enums,
                            std::vector< const CodeEntity* > & o_typedefs,
                            std::vector< const CodeEntity* > & o_constantGroups,
                            std::vector< const CodeEntity* > & o_singletons,
                            const CePilot &     i_pilot,
                            const CodeEntity &  i_ce );

    static void         Get_SubModules(
                            Dyn_CeIterator &    o_result,
                            const CodeEntity &  i_ce );
    static void         Get_Services(
                            Dyn_CeIterator &    o_result,
                            const CodeEntity &  i_ce );
    static void         Get_Interfaces(
                            Dyn_CeIterator &    o_result,
                            const CodeEntity &  i_ce );
    static void         Get_Structs(
                            Dyn_CeIterator &    o_result,
                            const CodeEntity &  i_ce );
    static void         Get_Exceptions(
                            Dyn_CeIterator &    o_result,
                            const CodeEntity &  i_ce );
    static void         Get_Enums(
                            Dyn_CeIterator &    o_result,
                            const CodeEntity &  i_ce );
    static void         Get_Typedefs(
                            Dyn_CeIterator &    o_result,
                            const CodeEntity &  i_ce );
    static void         Get_ConstantsGroups(
                            Dyn_CeIterator &    o_result,
                            const CodeEntity &  i_ce );
};

struct xref : public ifc_ce::xref
{
};

struct doc : public ifc_ce::doc
{
};

}   // namespace ifc_module

}   // namespace idl
}   // namespace ary


#endif
