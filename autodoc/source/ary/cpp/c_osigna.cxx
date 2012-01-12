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

#include <precomp.h>
#include <ary/cpp/c_osigna.hxx>


// NOT FULLY DEFINED SERVICES


namespace ary
{
namespace cpp
{


OperationSignature::OperationSignature( std::vector<Type_id>    i_parameterTypes,
                                        E_ConVol                i_conVol )
    :   aParameterTypes(i_parameterTypes),
        eConVol(i_conVol)
{
}

int
OperationSignature::Compare( const OperationSignature & i_rSig ) const
{
    if ( aParameterTypes.size() < i_rSig.aParameterTypes.size() )
        return -1;
    else if ( i_rSig.aParameterTypes.size() < aParameterTypes.size() )
        return 1;

    ParameterTypeList::const_iterator iMe = aParameterTypes.begin();
    ParameterTypeList::const_iterator iOther = i_rSig.aParameterTypes.begin();
    for ( ; iMe != aParameterTypes.end(); ++iMe, ++iOther )
    {
        if ( *iMe < *iOther )
            return -1;
        else if ( *iOther < *iMe )
            return 1;
    }

    if ( eConVol < i_rSig.eConVol )
        return -1;
    else if ( eConVol != i_rSig.eConVol )
        return 1;

    return 0;
}


} // namespace cpp
} // namespace ary
