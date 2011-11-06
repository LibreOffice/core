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



#ifndef ARY_IDL_IK_ATTRIBUTE_HXX
#define ARY_IDL_IK_ATTRIBUTE_HXX
//  KORR_DEPRECATED_3.0


// USED SERVICES
    // BASE CLASSES
#include <ary/idl/ik_ce.hxx>
    // COMPONENTS
    // PARAMETERS


namespace ary
{
namespace idl
{

namespace ifc_attribute
{

using ifc_ce::DocText;
using ::ary::idl::ifc_ce::Dyn_TypeIterator;


struct attr: public ifc_ce::attr
{
    static bool         HasAnyStereotype(
                            const CodeEntity &  i_ce );
    static bool         IsReadOnly(
                            const CodeEntity &  i_ce );
    static bool         IsBound(
                            const CodeEntity &  i_ce );
    static Type_id      Type(
                            const CodeEntity &  i_ce );
    static void         Get_GetExceptions(
                            Dyn_TypeIterator &  o_result,
                            const CodeEntity &  i_ce );
    static void         Get_SetExceptions(
                            Dyn_TypeIterator &  o_result,
                            const CodeEntity &  i_ce );
};

struct xref : public ifc_ce::xref
{
};

struct doc : public ifc_ce::doc
{
};


}   // namespace ifc_attribute

}   // namespace idl
}   // namespace ary


#endif
