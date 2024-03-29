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

#include <ucbhelper/contentidentifier.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::ucb;


namespace ucbhelper
{

struct ContentIdentifier_Impl
{
    OUString                          m_aContentId;
    OUString                          m_aProviderScheme;

    explicit ContentIdentifier_Impl( const OUString& rURL );
};


// ContentIdentifier_Impl Implementation.


ContentIdentifier_Impl::ContentIdentifier_Impl(const OUString& rURL )
{
    // Normalize URL scheme ( it's case insensitive ).

    // The content provider scheme is the part before the first ':'
    // within the content id.
    sal_Int32 nPos = rURL.indexOf( ':' );
    if ( nPos != -1 )
    {
        OUString aScheme( rURL.copy( 0, nPos ) );
        m_aProviderScheme = aScheme.toAsciiLowerCase();
        m_aContentId = rURL.replaceAt( 0, nPos, aScheme );
    }
}


// ContentIdentifier Implementation.


ContentIdentifier::ContentIdentifier( const OUString& rURL )
    : m_pImpl( new ContentIdentifier_Impl( rURL ) )
{
}


// virtual
ContentIdentifier::~ContentIdentifier()
{
}


// XContentIdentifier methods.


// virtual
OUString SAL_CALL ContentIdentifier::getContentIdentifier()
{
    return m_pImpl->m_aContentId;
}


// virtual
OUString SAL_CALL ContentIdentifier::getContentProviderScheme()
{
    return m_pImpl->m_aProviderScheme;
}

} /* namespace ucbhelper */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
