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



#ifndef UDM_LUXENUM_HXX
#define UDM_LUXENUM_HXX



// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
#include <map>
#include <algorithm>


namespace lux
{

typedef std::map< intt, String  > EnumValueMap;


template <class DIFF>
class Enum // : public Template_Base
{
  public:
    // TYPES
    typedef Enum< DIFF >    self;

    // LIFECYCLE
                        Enum(
                            DIFF                i_nValue,
                            const char *        i_sText )
                                                :   nValue(i_nValue) { Values_()[nValue] = i_sText;
                                                                       // Sequence_().insert(
                                                                       //       std::lower_bound( Sequence_().begin(), Sequence_().end(), i_nValue ),
                                                                       //       i_nValue );
                                                                     }
                        Enum(
                            DIFF                i_nValue )
                                                :   nValue(i_nValue) { ; }
                        Enum(
                            intt                i_nValue = 0 )
                                                :   nValue(i_nValue) { if ( NOT CheckIntt(i_nValue) ) { csv_assert(false); } }
                        Enum(
                            const self &        i_rEnum )
                                                :   nValue(i_rEnum.nValue) {;}

    self &              operator=(
                            DIFF                i_nValue )
                                                { nValue = i_nValue; return *this; }
    self &              operator=(
                            intt                i_nValue )
                                                { if ( CheckIntt(i_nValue) ) {nValue = DIFF(i_nValue);}
                                                  else {csv_assert(false);} return *this; }
    self &              operator=(
                            const self &        i_rEnum )
                                                { nValue = i_rEnum.nValue; return *this; }
                        operator DIFF() const   { return DIFF(nValue); }

    DIFF                operator()() const      { return nValue; }
    const String  &     Text() const            { return Values_()[nValue]; }

  private:
    static EnumValueMap &
                        Values_();
    bool                CheckIntt(
                            intt                i_nNumber )
                                                { return Values_().find(i_nNumber) != Values_().end(); }
    // DATA
    intt                nValue;
};




}   // namespace lux
#endif

