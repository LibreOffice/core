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

bool FeatureCommandDispatchBase::isFeatureSupported( const ::rtl::OUString& rCommandURL )
{
    SupportedFeatures::const_iterator aIter = m_aSupportedFeatures.find( rCommandURL );
    if ( aIter != m_aSupportedFeatures.end() )
    {
        return true;
    }
    return false;
}

void FeatureCommandDispatchBase::fireStatusEvent( const ::rtl::OUString& rURL,
    const Reference< frame::XStatusListener >& xSingleListener /* = 0 */ )
{
    if ( rURL.getLength() == 0 )
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
    ::rtl::OUString aCommand( URL.Complete );
    if ( getState( aCommand ).bEnabled )
    {
        execute( aCommand, Arguments );
    }
}

void FeatureCommandDispatchBase::implDescribeSupportedFeature( const sal_Char* pAsciiCommandURL,
    sal_uInt16 nId, sal_Int16 nGroup )
{
    ControllerFeature aFeature;
    aFeature.Command = ::rtl::OUString::createFromAscii( pAsciiCommandURL );
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
