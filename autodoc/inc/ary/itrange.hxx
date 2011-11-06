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



#ifndef ARY_ITRANGE_HXX
#define ARY_ITRANGE_HXX
//  KORR_DEPRECATED_3.0


// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
#include <utility>




namespace ary
{

template <typename ITER>
class IteratorRange
{
  public:
                        IteratorRange(
                            ITER                i_begin,
                            ITER                i_end )
                                                :   itCurrent(i_begin),
                                                    itEnd(i_end)
                                                {}
                        IteratorRange(
                            std::pair<ITER,ITER>
                                                i_range )
                                                :   itCurrent(i_range.first),
                                                    itEnd(i_range.second)
                                                {}

                        operator bool() const   { return itCurrent != itEnd; }
    IteratorRange &     operator++()            { ++itCurrent; return *this; }

    ITER                cur() const             { return itCurrent; }
    ITER                end() const             { return itEnd; }


  private:
    // DATA
    ITER                itCurrent;
    ITER                itEnd;
};





}   // namespace ary
#endif
