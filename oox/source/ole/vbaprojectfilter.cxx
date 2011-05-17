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

#include "oox/ole/vbaprojectfilter.hxx"

#include "oox/ole/vbaproject.hxx"

namespace oox {
namespace ole {

// ============================================================================

using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

using ::rtl::OUString;

// ============================================================================

VbaProjectFilterBase::VbaProjectFilterBase( const Reference< XComponentContext >& rxContext,
        const OUString& rAppName, const OUString& rStorageName ) throw( RuntimeException ) :
    BinaryFilterBase( rxContext ),
    maAppName( rAppName ),
    maStorageName( rStorageName )
{
}

bool VbaProjectFilterBase::importDocument() throw()
{
    StorageRef xVbaPrjStrg = openSubStorage( maStorageName, false );
    if( !xVbaPrjStrg || !xVbaPrjStrg->isStorage() )
        return false;

    getVbaProject().importVbaProject( *xVbaPrjStrg, getGraphicHelper() );
    return true;
}

bool VbaProjectFilterBase::exportDocument() throw()
{
    return false;
}

VbaProject* VbaProjectFilterBase::implCreateVbaProject() const
{
    return new VbaProject( getComponentContext(), getModel(), maAppName );
}

// ============================================================================

OUString SAL_CALL WordVbaProjectFilter_getImplementationName() throw()
{
    return CREATE_OUSTRING( "com.sun.star.comp.oox.WordVbaProjectFilter" );
}

Sequence< OUString > SAL_CALL WordVbaProjectFilter_getSupportedServiceNames() throw()
{
    Sequence< OUString > aSeq( 1 );
    aSeq[ 0 ] = CREATE_OUSTRING( "com.sun.star.document.ImportFilter" );
    return aSeq;
}

Reference< XInterface > SAL_CALL WordVbaProjectFilter_createInstance(
        const Reference< XComponentContext >& rxContext ) throw( Exception )
{
    return static_cast< ::cppu::OWeakObject* >( new WordVbaProjectFilter( rxContext ) );
}

// ----------------------------------------------------------------------------

WordVbaProjectFilter::WordVbaProjectFilter( const Reference< XComponentContext >& rxContext ) throw( RuntimeException ) :
    VbaProjectFilterBase( rxContext, CREATE_OUSTRING( "Writer" ), CREATE_OUSTRING( "Macros" ) )
{
}

OUString WordVbaProjectFilter::implGetImplementationName() const
{
    return WordVbaProjectFilter_getImplementationName();
}

// ============================================================================

} // namespace ole
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
