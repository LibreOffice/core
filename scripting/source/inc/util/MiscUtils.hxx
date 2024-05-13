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

#pragma once

#include <rtl/ustring.hxx>
#include <osl/diagnose.h>

#include <ucbhelper/content.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XTransientDocumentsDocumentContentFactory.hpp>
#include <com/sun/star/ucb/ContentCreationException.hpp>
#include <com/sun/star/ucb/SimpleFileAccess.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <comphelper/processfactory.hxx>

namespace sf_misc
{

class MiscUtils
{
public:

static css::uno::Sequence< OUString > allOpenTDocUrls( const  css::uno::Reference< css::uno::XComponentContext >& xCtx)
{
    css::uno::Sequence< OUString > result;
    try
    {
        if ( !xCtx.is() )
        {
            return result;
        }
        css::uno::Reference < css::ucb::XSimpleFileAccess3 > xSFA( css::ucb::SimpleFileAccess::create(xCtx) );
        result = xSFA->getFolderContents( u"vnd.sun.star.tdoc:/"_ustr, true );
    }
    catch ( css::uno::Exception& )
    {
    }
    return result;
}

static OUString xModelToTdocUrl( const css::uno::Reference< css::frame::XModel >& xModel,
                                 const css::uno::Reference< css::uno::XComponentContext >& xContext )
{
    css::uno::Reference< css::lang::XMultiComponentFactory > xMCF(
        xContext->getServiceManager() );
    css::uno::Reference<
            css::frame::XTransientDocumentsDocumentContentFactory > xDocFac;
    try
    {
        xDocFac.set(xMCF->createInstanceWithContext(
                        u"com.sun.star.frame.TransientDocumentsDocumentContentFactory"_ustr,
                        xContext ),
                    css::uno::UNO_QUERY );
    }
    catch ( css::uno::Exception const & )
    {
        // handled below
    }

    if ( xDocFac.is() )
    {
        try
        {
            css::uno::Reference< css::ucb::XContent > xContent(
                xDocFac->createDocumentContent( xModel ) );
            return xContent->getIdentifier()->getContentIdentifier();
        }
        catch ( css::lang::IllegalArgumentException const & )
        {
            OSL_FAIL( "Invalid document model!" );
        }
    }

    OSL_FAIL( "Unable to obtain URL for document model!" );
    return OUString();
}

static css::uno::Reference< css::frame::XModel > tDocUrlToModel( const OUString& url )
{
    css::uno::Any result;

    try
    {
        ::ucbhelper::Content root( url, nullptr, comphelper::getProcessComponentContext() );
        result = getUCBProperty( root, u"DocumentModel"_ustr );
    }
    catch ( css::ucb::ContentCreationException& )
    {
        // carry on, empty value will be returned
    }
    catch ( css::uno::RuntimeException& )
    {
        // carry on, empty value will be returned
    }

    css::uno::Reference< css::frame::XModel > xModel(
        result, css::uno::UNO_QUERY );

    return xModel;
}


static css::uno::Any getUCBProperty( ::ucbhelper::Content& content, OUString const & prop )
{
    css::uno::Any result;
    try
    {
        result = content.getPropertyValue( prop );
    }
    catch ( css::uno::Exception& )
    {
    }
    return result;
}

};
} // namespace sf_misc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
