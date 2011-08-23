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
OperationSignature::Compare( const OperationSignature &	i_rSig ) const
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
