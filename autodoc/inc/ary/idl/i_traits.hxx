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



#ifndef ARY_IDL_I_TRAITS_HXX
#define ARY_IDL_I_TRAITS_HXX

// USED SERVICES
#include <ary/idl/i_types4idl.hxx>




namespace ary
{
namespace idl
{


/** Basic traits for derivd ones of ->CodeEntity.
*/
struct Ce_Traits
{
    typedef CodeEntity          entity_base_type;
    typedef Ce_id               id_type;

    static entity_base_type &
                        EntityOf_(
                            id_type             i_id );
};


/** An instance of COMPARE for ->::ary::SortedIds<>.

    @see ::ary::SortedIds<>
*/
struct Ce_Compare : public Ce_Traits
{
    typedef  String             key_type;

    static const key_type &
                        KeyOf_(
                            const entity_base_type &
                                                i_entity );
    static bool         Lesser_(
                            const key_type &    i_1,
                            const key_type &    i_2 );
};




}   // namespace idl
}   // namespace ary
#endif
