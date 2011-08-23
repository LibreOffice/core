/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
    typedef std::vector<Type_id>	ParameterTypeList;

                        OperationSignature(
                            ParameterTypeList   i_parameterTypes,  // Non const, because it will be swapped with aParameterTypes.
                            E_ConVol            i_conVol );

    bool				operator==(
                            const OperationSignature &
                                                i_rSig ) const;
    bool				operator<(
                            const OperationSignature &
                                                i_rSig ) const;

    // INQUIRY
    const ParameterTypeList &
                        Parameters() const;
    E_ConVol            ConVol() const;

    /**	Compares the signatures by length an then by ids of
        parameter types. So the result is not always human
        reconstructable.
        @return like in strcmp().
    */
    int					Compare(
                            const OperationSignature &
                                                i_rSig ) const;
  private:
    // DATA
    ParameterTypeList	aParameterTypes;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
