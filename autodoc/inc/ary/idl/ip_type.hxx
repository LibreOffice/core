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



#ifndef ARY_IDL_IP_TYPE_HXX
#define ARY_IDL_IP_TYPE_HXX

// USED SERVICES
#include <ary/idl/i_types4idl.hxx>




namespace ary
{
    class QualifiedName;

namespace idl
{
    class Type;
    class ExplicitNameRoom;


/** Access point to all {->Type}s in IDL.
*/
class TypePilot
{
  public:
    // LIFECYCLE
    virtual             ~TypePilot() {}

    // OPERATIONS
    virtual const Type &
                        CheckIn_Type(
                            QualifiedName &     i_rFullName,
                            uintt               i_nSequenceCount,
                            Ce_id               i_nModuleOfOccurrence,
                            const std::vector<Type_id> *
                                                i_templateParameters ) = 0;
    // INQUIRY
    virtual const Type &
                        Find_Type(
                            Type_id             i_nType ) const = 0;
    virtual String      Search_LocalNameOf(
                            Type_id             i_nType ) const = 0;
    virtual Ce_id       Search_CeRelatedTo(
                            Type_id             i_nType ) const = 0;
    virtual const ExplicitNameRoom &
                        Find_XNameRoom(
                            Type_id             i_nType ) const = 0;
    virtual bool        IsBuiltInOrRelated(
                            const Type &        i_rType ) const = 0;
};




}   // namespace idl
}   // namespace ary
#endif
