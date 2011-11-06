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



#ifndef ARY_IDL_IK_SERVICE_HXX
#define ARY_IDL_IK_SERVICE_HXX



// USED SERVICES
    // BASE CLASSES
#include <ary/idl/i_param.hxx>
#include <ary/idl/ik_ce.hxx>
    // COMPONENTS
    // PARAMETERS


namespace ary
{
namespace idl
{



namespace ifc_function
{

using ::ary::idl::ifc_ce::Dyn_CeIterator;
using ::ary::idl::ifc_ce::Dyn_TypeIterator;
using ::ary::idl::ifc_ce::DocText;


struct attr: public ifc_ce::attr
{
    static Type_id      ReturnType(
                            const CodeEntity &  i_ce );
    static bool         IsOneway(
                            const CodeEntity &  i_ce );
    static bool         HasEllipse(
                            const CodeEntity &  i_ce );
    static void         Get_Parameters(
                            Dyn_StdConstIterator<ary::idl::Parameter> &
                                                o_result,
                            const CodeEntity &  i_ce );
    static void         Get_Exceptions(
                            Dyn_TypeIterator &  o_result,
                            const CodeEntity &  i_ce );
};

struct xref : public ifc_ce::xref
{
};

struct doc : public ifc_ce::doc
{
//  aStateMachine.AddToken( "@param",   nTok_at_param,       A_nAtTagDefStatus, finAtTag );
//  aStateMachine.AddToken( "@throws",  nTok_at_throws,      A_nAtTagDefStatus, finAtTag );
//  aStateMachine.AddToken( "@exception",
//  aStateMachine.AddToken( "@return",  nTok_at_return,      A_nAtTagDefStatus, finAtTag );
//  aStateMachine.AddToken( "@returns", nTok_at_return,      A_nAtTagDefStatus, finAtTag );
};


}   // namespace ifc_function



}   // namespace idl
}   // namespace ary


#endif

