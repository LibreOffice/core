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

#include <UriBindingHelper.hxx>

#include <tools/stream.hxx>
#include <unotools/streamwrap.hxx>

#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <osl/diagnose.h>
#include <rtl/uri.hxx>
#include <sal/log.hxx>

using namespace com::sun::star;

// XUriBinding

UriBindingHelper::UriBindingHelper()
{
}

UriBindingHelper::UriBindingHelper( const css::uno::Reference < css::embed::XStorage >& rxStorage )
{
    mxStorage = rxStorage;
}

void SAL_CALL UriBindingHelper::setUriBinding( const OUString& /*uri*/, const uno::Reference< io::XInputStream >&)
{
}

uno::Reference< io::XInputStream > SAL_CALL UriBindingHelper::getUriBinding( const OUString& uri )
{
    uno::Reference< io::XInputStream > xInputStream;
    if ( mxStorage.is() )
    {
        xInputStream = OpenInputStream( mxStorage, uri );
    }
    else
    {
        std::unique_ptr<SvFileStream> pStream(new SvFileStream( uri, StreamMode::READ ));
        xInputStream = new utl::OInputStreamWrapper( std::move(pStream) );
    }
    return xInputStream;
}

uno::Reference < io::XInputStream > UriBindingHelper::OpenInputStream( const uno::Reference < embed::XStorage >& rxStore, const OUString& rURI )
{
    OSL_ASSERT(!rURI.isEmpty());
    uno::Reference < io::XInputStream > xInStream;

    OUString aURI(rURI);
    // Ignore leading slash, don't attempt to open a storage with name "".
    if (aURI.startsWith("/"))
        aURI = aURI.copy(1);
    // Ignore query part of the URI.
    sal_Int32 nQueryPos = aURI.indexOf('?');
    if (nQueryPos != -1)
        aURI = aURI.copy(0, nQueryPos);


    sal_Int32 nSepPos = aURI.indexOf( '/' );
    if ( nSepPos == -1 )
    {
        // Cloning because of I can't keep all storage references open
        // MBA with think about a better API...
        const OUString sName = ::rtl::Uri::decode(
            aURI, rtl_UriDecodeStrict, rtl_UriCharClassRelSegment);
        if (sName.isEmpty() && !aURI.isEmpty())
            throw uno::Exception("Could not decode URI for stream element.", nullptr);

        uno::Reference< io::XStream > xStream;
        if (!rxStore->hasByName(sName))
            SAL_WARN("xmlsecurity.helper", "expected stream, but not found: " << sName);
        else
            xStream = rxStore->cloneStreamElement( sName );
        if ( !xStream.is() )
            throw uno::RuntimeException();
        xInStream = xStream->getInputStream();
    }
    else
    {
        const OUString aStoreName = ::rtl::Uri::decode(
            aURI.copy( 0, nSepPos ), rtl_UriDecodeStrict, rtl_UriCharClassRelSegment);
        if (aStoreName.isEmpty() && !aURI.isEmpty())
            throw uno::Exception("Could not decode URI for stream element.", nullptr);

        OUString aElement = aURI.copy( nSepPos+1 );
        uno::Reference < embed::XStorage > xSubStore = rxStore->openStorageElement( aStoreName, embed::ElementModes::READ );
        xInStream = OpenInputStream( xSubStore, aElement );
    }
    return xInStream;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
