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



#ifndef ARY_CPP_C_OSIGNA_HXX
#define ARY_CPP_C_OSIGNA_HXX

// USED SERVICES
    // BASE CLASSES
    // OTHER
#include <ary/cpp/c_types4cpp.hxx>

namespace ary
{
namespace cpp
{
    class Gate;
}
}



namespace ary
{
namespace cpp
{


/** The signature of a C++ function. That is: parameter types and
    const/volatile modifiers.
*/
class OperationSignature
{
  public:
    typedef std::vector<Type_id>    ParameterTypeList;

                        OperationSignature(
                            ParameterTypeList   i_parameterTypes,  // Non const, because it will be swapped with aParameterTypes.
                            E_ConVol            i_conVol );

    bool                operator==(
                            const OperationSignature &
                                                i_rSig ) const;
    bool                operator<(
                            const OperationSignature &
                                                i_rSig ) const;

    // INQUIRY
    const ParameterTypeList &
                        Parameters() const;
    E_ConVol            ConVol() const;

    /** Compares the signatures by length an then by ids of
        parameter types. So the result is not always human
        reconstructable.
        @return like in strcmp().
    */
    int                 Compare(
                            const OperationSignature &
                                                i_rSig ) const;
  private:
    // DATA
    ParameterTypeList   aParameterTypes;
    E_ConVol            eConVol;
};




// IMPLEMENTATION
inline bool
OperationSignature::operator==( const OperationSignature & i_rSign ) const
{
    return Compare(i_rSign) == 0;
}

inline bool
OperationSignature::operator<( const OperationSignature & i_rSign ) const
{
    return Compare(i_rSign) < 0;
}

inline const OperationSignature::ParameterTypeList &
OperationSignature::Parameters() const
{
    return aParameterTypes;
}

inline E_ConVol
OperationSignature::ConVol() const
{
    return eConVol;
}



} // namespace cpp
} // namespace ary
#endif
