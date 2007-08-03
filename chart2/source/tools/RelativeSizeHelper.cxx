/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RelativeSizeHelper.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-08-03 12:37:01 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_chart2.hxx"
#include "RelativeSizeHelper.hxx"
#include "macros.hxx"

#include <vector>
#include <algorithm>

using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::beans;
using namespace ::std;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::makeAny;
using ::com::sun::star::uno::Exception;
using ::rtl::OUString;

namespace chart
{

// static
double RelativeSizeHelper::calculate(
    double fValue,
    const Size & rOldReferenceSize,
    const Size & rNewReferenceSize )
{
    if( rOldReferenceSize.Width <= 0 ||
        rOldReferenceSize.Height <= 0 )
        return fValue;

    return min(
        static_cast< double >( rNewReferenceSize.Width )  / static_cast< double >( rOldReferenceSize.Width ),
        static_cast< double >( rNewReferenceSize.Height ) / static_cast< double >( rOldReferenceSize.Height ))
        * fValue;
}

// static
void RelativeSizeHelper::adaptFontSizes(
    const Reference< XPropertySet > & xTargetProperties,
    const Size & rOldReferenceSize,
    const Size & rNewReferenceSize )
{
    if( ! xTargetProperties.is())
        return;

    float fFontHeight = 0;

    vector< OUString > aProperties;
    aProperties.push_back( OUString( RTL_CONSTASCII_USTRINGPARAM( "CharHeight" )));
    aProperties.push_back( OUString( RTL_CONSTASCII_USTRINGPARAM( "CharHeightAsian" )));
    aProperties.push_back( OUString( RTL_CONSTASCII_USTRINGPARAM( "CharHeightComplex" )));

    for( vector< OUString >::const_iterator aIt = aProperties.begin();
         aIt != aProperties.end(); ++aIt )
    {
        try
        {
            if( xTargetProperties->getPropertyValue( *aIt ) >>= fFontHeight )
            {
                xTargetProperties->setPropertyValue(
                    *aIt,
                    makeAny( static_cast< float >(
                                 calculate( fFontHeight, rOldReferenceSize, rNewReferenceSize ))));
            }
        }
        catch( const Exception & ex )
        {
            ASSERT_EXCEPTION( ex );
        }
    }
}

} //  namespace chart
