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

#include <vcl/svapp.hxx>
#include <comphelper/servicehelper.hxx>
#include "access.hrc"
#include <accpreview.hxx>

const sal_Char sServiceName[] = "com.sun.star.text.AccessibleTextDocumentPageView";
const sal_Char sImplementationName[] = "com.sun.star.comp.Writer.SwAccessibleDocumentPageView";

using ::com::sun::star::lang::IndexOutOfBoundsException;
using ::com::sun::star::uno::RuntimeException;
using ::com::sun::star::uno::Sequence;

// SwAccessiblePreview

SwAccessiblePreview::SwAccessiblePreview( SwAccessibleMap *pMp ) :
    SwAccessibleDocumentBase( pMp )
{
    SetName( GetResource( STR_ACCESS_DOC_NAME ) );
}

SwAccessiblePreview::~SwAccessiblePreview()
{
}

OUString SwAccessiblePreview::getImplementationName( )
    throw( RuntimeException )
{
    return OUString( sImplementationName );
}

sal_Bool SwAccessiblePreview::supportsService( const OUString& rServiceName )
    throw( RuntimeException )
{
    return rServiceName == sServiceName || rServiceName == sAccessibleServiceName;
}

Sequence<OUString> SwAccessiblePreview::getSupportedServiceNames( )
    throw( RuntimeException )
{
    Sequence<OUString> aSeq( 2 );
    aSeq[0] = OUString( sServiceName );
    aSeq[1] = OUString( sAccessibleServiceName );
    return aSeq;
}

namespace
{
    class theSwAccessiblePreviewImplementationId : public rtl::Static< UnoTunnelIdInit, theSwAccessiblePreviewImplementationId > {};
}

Sequence< sal_Int8 > SAL_CALL SwAccessiblePreview::getImplementationId()
        throw(RuntimeException)
{
    return theSwAccessiblePreviewImplementationId::get().getSeq();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
