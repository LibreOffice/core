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

#include "rtfimportfilter.hxx"
#include "../rtf/swparrtf.hxx"

#include <docsh.hxx>
#include <doc.hxx>
#include <unotxdoc.hxx>

#include <comphelper/mediadescriptor.hxx>
#include <unotools/ucbstreamhelper.hxx>

using namespace ::comphelper;
using namespace ::com::sun::star;

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
    SAL_INFO("sw.rtf", OSL_THIS_FUNC);

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
        if( aDescriptor[i].Name == "URL" )
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
    return OUString( IMPL_NAME_RTFIMPORT );
}

uno::Sequence< OUString > SAL_CALL RtfImport_getSupportedServiceNames() throw()
{
    const OUString aServiceName( "com.sun.star.document.ImportFilter" );
    const uno::Sequence< OUString > aSeq( &aServiceName, 1 );
    return aSeq;
}

uno::Reference< uno::XInterface > SAL_CALL RtfImport_createInstance(const uno::Reference< uno::XComponentContext > & xCtx ) throw( uno::Exception )
{
    return (cppu::OWeakObject*) new RtfImportFilter( xCtx );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
