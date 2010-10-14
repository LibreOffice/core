/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile:
 * $Revision:
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
#include "vbarevision.hxx"
#include <vbahelper/vbahelper.hxx>
#include <tools/diagnose_ex.h>
#include <com/sun/star/document/XRedlinesSupplier.hpp>
#include "wordvbahelper.hxx"
#include <docsh.hxx>
#include <doc.hxx>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaRevision::SwVbaRevision( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, const uno::Reference< frame::XModel >& xModel, const uno::Reference< beans::XPropertySet >& xRedlineProps ) throw ( uno::RuntimeException ) : SwVbaRevision_BASE( rParent, rContext ), mxModel( xModel ), mxRedlineProps( xRedlineProps )
{
}

SwVbaRevision::~SwVbaRevision()
{
}

sal_Int32 SwVbaRevision::GetPosition() throw (css::uno::RuntimeException)
{
    sal_Int32 nPos = -1;
    uno::Reference< document::XRedlinesSupplier > xRedlinesSupp( mxModel, uno::UNO_QUERY_THROW );
    uno::Reference< container::XIndexAccess > xRedlines( xRedlinesSupp->getRedlines(), uno::UNO_QUERY_THROW );
    sal_Int32 nCount = xRedlines->getCount();
    for( sal_Int32 i = 0; i < nCount; i++ )
    {
        uno::Reference< beans::XPropertySet > xProps( xRedlines->getByIndex( i ), uno::UNO_QUERY_THROW );
        if( xProps == mxRedlineProps )
        {
            nPos = i;
            OSL_TRACE(" SwVbaRevision::SwVbaRevision, the redline position is %d, ", nPos );
            break;
        }
    }
    if( nPos == -1 )
        throw uno::RuntimeException();

    return nPos;
}

void SAL_CALL
SwVbaRevision::Accept() throw ( css::uno::RuntimeException )
{
    SwDoc* pDoc = word::getDocShell( mxModel )->GetDoc();
    if( pDoc )
        pDoc->AcceptRedline( GetPosition(), sal_True );
}

void SAL_CALL
SwVbaRevision::Reject( ) throw ( css::uno::RuntimeException )
{
    SwDoc* pDoc = word::getDocShell( mxModel )->GetDoc();
    if( pDoc )
        pDoc->RejectRedline( GetPosition(), sal_True );
}

rtl::OUString&
SwVbaRevision::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("SwVbaRevision") );
    return sImplName;
}

uno::Sequence< rtl::OUString >
SwVbaRevision::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.word.Revision" ) );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
