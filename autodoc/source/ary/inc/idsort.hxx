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



#ifndef ARY_IDSORT_HXX
#define ARY_IDSORT_HXX


/** A compare function that sorts ids of repository entities in the same
    storage.

    @see ::ary::SortedIds
*/
template<class COMPARE>
struct IdSorter
{
    bool                operator()(
                            typename COMPARE::id_type
                                                i_1,
                            typename COMPARE::id_type
                                                i_2 ) const
                            { return COMPARE::Lesser_(
                                        COMPARE::KeyOf_(COMPARE::EntityOf_(i_1)),
                                        COMPARE::KeyOf_(COMPARE::EntityOf_(i_2)) );
                            }
};


#endif
