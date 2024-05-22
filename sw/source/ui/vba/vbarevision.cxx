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
#include "vbarevision.hxx"
#include <sal/log.hxx>
#include <com/sun/star/document/XRedlinesSupplier.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include "wordvbahelper.hxx"
#include <docsh.hxx>
#include <doc.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <utility>

using namespace ::ooo::vba;
using namespace ::com::sun::star;

SwVbaRevision::SwVbaRevision( const uno::Reference< ooo::vba::XHelperInterface >& rParent, const uno::Reference< uno::XComponentContext >& rContext, uno::Reference< frame::XModel >  xModel, uno::Reference< beans::XPropertySet >  xRedlineProps ) : SwVbaRevision_BASE( rParent, rContext ), mxModel(std::move( xModel )), mxRedlineProps(std::move( xRedlineProps ))
{
}

SwVbaRevision::~SwVbaRevision()
{
}

sal_Int32 SwVbaRevision::GetPosition()
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
            SAL_INFO("sw.ui", "the redline position is " << nPos);
            break;
        }
    }
    if( nPos == -1 )
        throw uno::RuntimeException();

    return nPos;
}

void SAL_CALL
SwVbaRevision::Accept()
{
    SwDoc* pDoc = word::getDocShell( mxModel )->GetDoc();
    if( pDoc )
        pDoc->getIDocumentRedlineAccess().AcceptRedline( GetPosition(), true );
}

void SAL_CALL
SwVbaRevision::Reject( )
{
    SwDoc* pDoc = word::getDocShell( mxModel )->GetDoc();
    if( pDoc )
        pDoc->getIDocumentRedlineAccess().RejectRedline( GetPosition(), true );
}

OUString
SwVbaRevision::getServiceImplName()
{
    return u"SwVbaRevision"_ustr;
}

uno::Sequence< OUString >
SwVbaRevision::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.word.Revision"_ustr
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
