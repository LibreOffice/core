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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
