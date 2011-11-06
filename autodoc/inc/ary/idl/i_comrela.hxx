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



#ifndef ARY_IDL_I_COMRELA_HXX
#define ARY_IDL_I_COMRELA_HXX

// USED SERVICES
#include <ary/idl/i_types4idl.hxx>

namespace ary
{
namespace doc
{
    class OldIdlDocu;
}
}




namespace ary
{
namespace idl
{


/** Contains data for an IDL code entity related to another one like a base of
    an interface or of a service or the supported interface of a service.
*/
class CommentedRelation
{
  public:
    // LIFECYCLE

                        CommentedRelation(
                            Type_id             i_nType,
                            doc::OldIdlDocu *   i_pInfo )
                                                :   nType(i_nType),
                                                    pInfo(i_pInfo)
                                                {}
    // INQUIRY
    Type_id             Type() const            { return nType; }
    doc::OldIdlDocu *   Info() const            { return pInfo; }

  private:
    // DATA
    Type_id             nType;
    doc::OldIdlDocu *   pInfo;
};




}   // namespace idl
}   // namespace ary
#endif
