/*************************************************************************
 *
 *  $RCSfile: c_osigna.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:18 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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


