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

#ifndef _UCBHELPER_REGISTERUCB_HXX_
#define _UCBHELPER_REGISTERUCB_HXX_

#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <com/sun/star/ucb/XContentProviderManager.hpp>
#include <vector>

#include "ucbhelper/ucbhelperdllapi.h"

namespace com { namespace sun { namespace star {
    namespace lang { class XMultiServiceFactory; }
} } }


namespace ucbhelper {

//============================================================================
/** Information about a registered content provider.
 */
struct ContentProviderRegistrationInfo
{
    /** The registered content provider (or null if registration failed).
     */
    com::sun::star::uno::Reference< com::sun::star::ucb::XContentProvider >
        m_xProvider;

    /** The arguments the content provider was instantiated with.
     */
    OUString m_aArguments;

    /** The URL template the content provider is registered on.
     */
    OUString m_aTemplate;
};

typedef std::vector< ContentProviderRegistrationInfo >
                                    ContentProviderRegistrationInfoList;

//============================================================================
/** Information about a content provider, passed to
    <method>configureUcb</method>.
 */
struct ContentProviderData
{
    /** The UNO service name to use to instanciate the content provider.
     */
    OUString ServiceName;

    /** The URL template to use to instanciate the content provider.
     */
    OUString URLTemplate;

    /** The arguments to use to instanciate the content provider.
     */
    OUString Arguments;

    ContentProviderData() {};
    ContentProviderData( const OUString & rService,
                         const OUString & rTemplate,
                         const OUString & rArgs )
    : ServiceName( rService ), URLTemplate( rTemplate ), Arguments( rArgs ) {}
};

typedef std::vector< ContentProviderData > ContentProviderDataList;
//============================================================================
/** Register a content provider at a Universal Content Broker.

    @param rManager  A content provider manager (normally, this would be a
    UCB).  May be null, which is only useful if the content provider is an
    <type>XParamterizedContentProvider</type>s.

    @param rServiceFactory  A factory through which to obtain the required
    services.

    @param rName  The service name of the content provider.

    @param rArguments  Any arguments to instantiate the content provider with.

    @param rTemplate  The URL template to register the content provider on.

    @param pInfo  If not null, this output parameter is filled with
    information about the (atemptively) registered provider.
 */

UCBHELPER_DLLPUBLIC bool registerAtUcb(
    com::sun::star::uno::Reference<
            com::sun::star::ucb::XContentProviderManager > const &
        rManager,
    com::sun::star::uno::Reference<
            com::sun::star::lang::XMultiServiceFactory > const &
        rServiceFactory,
    OUString const & rName,
    OUString const & rArguments,
    OUString const & rTemplate,
    ContentProviderRegistrationInfo * pInfo)
    throw (com::sun::star::uno::RuntimeException);

}
#endif // _UCBHELPER_REGISTERUCB_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
