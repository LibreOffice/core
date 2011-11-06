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



#ifndef COSV_STR_TYPES_HXX
#define COSV_STR_TYPES_HXX


namespace csv
{

/** Provides some generally used constants.
*/
struct str
{
  public:
    typedef ::size_t    position;
    typedef ::size_t    size;

    enum constants
    {
        npos = position(-1),
        maxsize = size(-1)
    };

    enum insert_mode
    {
        overwrite = 0,
        insert = 1
    };
};


/** Is used for string comparisons.

    @collab String
    @collab various csv::compare(...) functions
*/
class CharOrder_Table
{
  public:
    /** @precond
        Parameter i_pCharWeightsArray
        must have size of 256.
    */
                        CharOrder_Table(
                            const int *         i_pCharWeightsArray );

    /** @return the weight of the char i_c.
        @precond
        Even with unusual implementations, where char has more than 8 bit,
        there must be true: 0 <= i_c < 256.
    */
    int                 operator()(
                            char                i_c ) const;
  private:
    int                 cWeights[256];
};


// IMPLEMENTATION

inline int
CharOrder_Table::operator()( char i_c ) const
    { return cWeights[ UINT8(i_c) ]; }



}   // namespace csv

#endif
