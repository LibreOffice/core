/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 * Copyright 2010 Miklos Vajna.
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

#include <rtfexportfilter.hxx>
#include <rtfexport.hxx>
#include <rtfimportfilter.hxx>

#include <docsh.hxx>
#include <doc.hxx>
#include <pam.hxx>
#include <unotxdoc.hxx>

#include <cppuhelper/factory.hxx>
#include <comphelper/mediadescriptor.hxx>
#include <unotools/ucbstreamhelper.hxx>

using namespace ::comphelper;
using namespace ::com::sun::star;
using ::rtl::OUString;

RtfExportFilter::RtfExportFilter( const uno::Reference< uno::XComponentContext >& xCtx) :
    m_xCtx( xCtx )
{
}

RtfExportFilter::~RtfExportFilter()
{
}

sal_Bool RtfExportFilter::filter( const uno::Sequence< beans::PropertyValue >& aDescriptor )
    throw (uno::RuntimeException)
{
    OSL_TRACE("%s", OSL_THIS_FUNC);

    MediaDescriptor aMediaDesc = aDescriptor;
    ::uno::Reference< io::XStream > xStream =
        aMediaDesc.getUnpackedValueOrDefault( MediaDescriptor::PROP_STREAMFOROUTPUT(), uno::Reference< io::XStream >() );
    m_pStream = utl::UcbStreamHelper::CreateStream( xStream, sal_True );
    m_aWriter.SetStream(m_pStream);

    // get SwDoc*
    uno::Reference< uno::XInterface > xIfc( m_xSrcDoc, uno::UNO_QUERY );
    SwXTextDocument *pTxtDoc = dynamic_cast< SwXTextDocument * >( xIfc.get() );
    if ( !pTxtDoc ) {
        return sal_False;
    }

    SwDoc *pDoc = pTxtDoc->GetDocShell()->GetDoc();
    if ( !pDoc ) {
        return sal_False;
    }

    // get SwPaM*
    // we get SwPaM for the entire document; copy&paste is handled internally, not via UNO
    SwPaM aPam( pDoc->GetNodes().GetEndOfContent() );
    aPam.SetMark();
    aPam.Move( fnMoveBackward, fnGoDoc );

    SwPaM *pCurPam = new SwPaM( *aPam.End(), *aPam.Start() );

    // export the document
    // (in a separate block so that it's destructed before the commit)
    {
        RtfExport aExport( this, pDoc, pCurPam, &aPam, NULL );
        aExport.ExportDocument( true );
    }

    // delete the pCurPam
    if ( pCurPam )
    {
        while ( pCurPam->GetNext() != pCurPam )
            delete pCurPam->GetNext();
        delete pCurPam;
    }
    delete m_pStream;

    return sal_True;
}


void RtfExportFilter::cancel(  ) throw (uno::RuntimeException)
{
}

void RtfExportFilter::setSourceDocument( const uno::Reference< lang::XComponent >& xDoc )
    throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    m_xSrcDoc = xDoc;
}

//////////////////////////////////////////////////////////////////////////
// UNO helpers
//////////////////////////////////////////////////////////////////////////

OUString RtfExport_getImplementationName()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( IMPL_NAME_RTFEXPORT ) );
}

uno::Sequence< OUString > SAL_CALL RtfExport_getSupportedServiceNames() throw()
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.ExportFilter" ) );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL RtfExport_createInstance(const uno::Reference< uno::XComponentContext > & xCtx ) throw( uno::Exception )
{
    return (cppu::OWeakObject*) new RtfExportFilter( xCtx );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
