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

#include "rtfimportfilter.hxx"
#include "../rtf/swparrtf.hxx"

#include <docsh.hxx>
#include <doc.hxx>
#include <pam.hxx>
#include <unotxdoc.hxx>
#include <swerror.h>

#include <cppuhelper/factory.hxx>
#include <comphelper/mediadescriptor.hxx>
#include <unotools/ucbstreamhelper.hxx>

#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>

using namespace ::comphelper;
using namespace ::com::sun::star;
using ::rtl::OUString;
using rtl::OUStringToOString;

RtfImportFilter::RtfImportFilter( const uno::Reference< uno::XComponentContext > &xCtx ) :
    m_xCtx( xCtx )
{
}

RtfImportFilter::~RtfImportFilter()
{
}

sal_Bool RtfImportFilter::filter( const uno::Sequence< beans::PropertyValue >& aDescriptor )
    throw (uno::RuntimeException)
{
    OSL_TRACE("%s", OSL_THIS_FUNC);

    MediaDescriptor aMediaDesc = aDescriptor;
    ::uno::Reference< io::XInputStream > xInputStream =
        aMediaDesc.getUnpackedValueOrDefault( MediaDescriptor::PROP_INPUTSTREAM(), uno::Reference< io::XInputStream >() );
    SvStream* pStream = utl::UcbStreamHelper::CreateStream( xInputStream, sal_True );
    if (!pStream)
        return sal_False;

    // get SwDoc*
    uno::Reference< uno::XInterface > xIfc( m_xDstDoc, uno::UNO_QUERY );
    SwXTextDocument *pTxtDoc = dynamic_cast< SwXTextDocument * >( xIfc.get() );
    if (!pTxtDoc)
        return sal_False;
    SwDoc *pDoc = pTxtDoc->GetDocShell()->GetDoc();
    if (!pDoc)
        return sal_False;

    // get SwPaM*
    // NEEDSWORK should we care about partial imports? For now we just import
    // the whole document
    SwPaM aPam( pDoc->GetNodes().GetEndOfContent() );
    aPam.SetMark();
    aPam.Move( fnMoveBackward, fnGoDoc );
    SwPaM *pCurPam = new SwPaM( *aPam.End(), *aPam.Start() );

    String aURL;
    OUString sTemp;
    for ( sal_Int32 i = 0; i < aDescriptor.getLength(); i++ )
    {
        if( aDescriptor[i].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("URL")) )
        {
            aDescriptor[i].Value >>= sTemp;
            aURL = sTemp;
        }
    }

    RtfReader aReader;
    sal_Bool bRet = aReader.Read(pStream, *pDoc, aURL, *pCurPam) == 0;
    delete pStream;
    return bRet;
}


void RtfImportFilter::cancel(  ) throw (uno::RuntimeException)
{
}

void RtfImportFilter::setTargetDocument( const uno::Reference< lang::XComponent >& xDoc )
    throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    m_xDstDoc = xDoc;
}

//////////////////////////////////////////////////////////////////////////
// UNO helpers
//////////////////////////////////////////////////////////////////////////

OUString RtfImport_getImplementationName()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM( IMPL_NAME_RTFIMPORT ) );
}

uno::Sequence< OUString > SAL_CALL RtfImport_getSupportedServiceNames() throw()
{
    const OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.document.ImportFilter" ) );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL RtfImport_createInstance(const uno::Reference< uno::XComponentContext > & xCtx ) throw( uno::Exception )
{
    return (cppu::OWeakObject*) new RtfImportFilter( xCtx );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
