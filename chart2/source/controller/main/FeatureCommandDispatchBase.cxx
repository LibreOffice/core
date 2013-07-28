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

#include "FeatureCommandDispatchBase.hxx"

using namespace ::com::sun::star;

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;

namespace chart
{

FeatureCommandDispatchBase::FeatureCommandDispatchBase( const Reference< uno::XComponentContext >& rxContext )
    :CommandDispatch( rxContext )
    ,m_nFeatureId( 0 )
{
}

FeatureCommandDispatchBase::~FeatureCommandDispatchBase()
{
}

void FeatureCommandDispatchBase::initialize()
{
    CommandDispatch::initialize();
    fillSupportedFeatures();
}

bool FeatureCommandDispatchBase::isFeatureSupported( const OUString& rCommandURL )
{
    SupportedFeatures::const_iterator aIter = m_aSupportedFeatures.find( rCommandURL );
    if ( aIter != m_aSupportedFeatures.end() )
    {
        return true;
    }
    return false;
}

void FeatureCommandDispatchBase::fireStatusEvent( const OUString& rURL,
    const Reference< frame::XStatusListener >& xSingleListener /* = 0 */ )
{
    if ( rURL.isEmpty() )
    {
        SupportedFeatures::const_iterator aEnd( m_aSupportedFeatures.end() );
        for ( SupportedFeatures::const_iterator aIter( m_aSupportedFeatures.begin() ); aIter != aEnd; ++aIter )
        {
            FeatureState aFeatureState( getState( aIter->first ) );
            fireStatusEventForURL( aIter->first, aFeatureState.aState, aFeatureState.bEnabled, xSingleListener );
        }
    }
    else
    {
        FeatureState aFeatureState( getState( rURL ) );
        fireStatusEventForURL( rURL, aFeatureState.aState, aFeatureState.bEnabled, xSingleListener );
    }
}

// XDispatch
void FeatureCommandDispatchBase::dispatch( const util::URL& URL,
    const Sequence< beans::PropertyValue >& Arguments )
    throw (uno::RuntimeException)
{
    OUString aCommand( URL.Complete );
    if ( getState( aCommand ).bEnabled )
    {
        execute( aCommand, Arguments );
    }
}

void FeatureCommandDispatchBase::implDescribeSupportedFeature( const sal_Char* pAsciiCommandURL,
    sal_uInt16 nId, sal_Int16 nGroup )
{
    ControllerFeature aFeature;
    aFeature.Command = OUString::createFromAscii( pAsciiCommandURL );
    aFeature.nFeatureId = nId;
    aFeature.GroupId = nGroup;

    m_aSupportedFeatures[ aFeature.Command ] = aFeature;
}

void FeatureCommandDispatchBase::fillSupportedFeatures()
{
    describeSupportedFeatures();
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
