/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "RelativeSizeHelper.hxx"
#include "macros.hxx"

#include <vector>
#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::beans;
using namespace ::std;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Exception;

namespace chart
{

double RelativeSizeHelper::calculate(
    double fValue,
    const awt::Size & rOldReferenceSize,
    const awt::Size & rNewReferenceSize )
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
    const awt::Size & rOldReferenceSize,
    const awt::Size & rNewReferenceSize )
{
    if( ! xTargetProperties.is())
        return;

    float fFontHeight = 0;

    vector< OUString > aProperties;
    aProperties.push_back( "CharHeight" );
    aProperties.push_back( "CharHeightAsian" );
    aProperties.push_back( "CharHeightComplex" );

    for( vector< OUString >::const_iterator aIt = aProperties.begin();
         aIt != aProperties.end(); ++aIt )
    {
        try
        {
            if( xTargetProperties->getPropertyValue( *aIt ) >>= fFontHeight )
            {
                xTargetProperties->setPropertyValue(
                    *aIt,
                    Any( static_cast< float >(
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
