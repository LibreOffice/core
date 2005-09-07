/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: c_osigna.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:32:31 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


#include <precomp.h>
#include <ary/cpp/c_osigna.hxx>


// NOT FULLY DEFINED SERVICES


namespace ary
{
namespace cpp
{


OperationSignature::OperationSignature()
    :   // aParameterTypes,
        eConVol(CONVOL_none)
{
}

OperationSignature::OperationSignature( std::vector<Tid> & i_rParameterTypes,
                                        E_ConVol           i_eConVol )
    :   // aParameterTypes,
        eConVol(i_eConVol)
{
    std::swap( aParameterTypes, i_rParameterTypes );
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


