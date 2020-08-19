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

#ifndef INCLUDED_UCBHELPER_REGISTERUCB_HXX
#define INCLUDED_UCBHELPER_REGISTERUCB_HXX

#include <rtl/ustring.hxx>
#include <vector>

#include <ucbhelper/ucbhelperdllapi.h>

namespace com::sun::star::uno { template <class interface_type> class Reference; }
namespace com::sun::star {
    namespace uno { class XComponentContext; }
    namespace ucb { class XContentProviderManager; }
}


namespace ucbhelper {


/** Information about a content provider, passed to
    <method>configureUcb</method>.
 */
struct ContentProviderData
{
    /** The UNO service name to use to instantiate the content provider.

        If it is the empty string, a null provider will be used (see the
        documentation of the Provider argument to
        com.sun.star.ucb.XContentProviderManager.registerContentProvider).
     */
    OUString ServiceName;

    /** The URL template to use to instantiate the content provider.
     */
    OUString URLTemplate;

    /** The arguments to use to instantiate the content provider.
     */
    OUString Arguments;
};

typedef std::vector< ContentProviderData > ContentProviderDataList;

/** Register a content provider at a Universal Content Broker.

    @param rManager  A content provider manager (normally, this would be a
    UCB).  May be null, which is only useful if the content provider is an
    XParameterizedContentProviders.

    @param rServiceFactory  A factory through which to obtain the required
    services.

    @param rName  The service name of the content provider.  If it is the empty
    string, a null provider will be used (see the documentation of the Provider
    argument to
    com.sun.star.ucb.XContentProviderManager.registerContentProvider).

    @param rArguments  Any arguments to instantiate the content provider with.

    @param rTemplate  The URL template to register the content provider on.

    @param pInfo  If not null, this output parameter is filled with
    information about the (attemptively) registered provider.

    @throws css::uno::RuntimeException
 */

UCBHELPER_DLLPUBLIC bool registerAtUcb(
    css::uno::Reference< css::ucb::XContentProviderManager > const & rManager,
    css::uno::Reference< css::uno::XComponentContext > const & rxContext,
    OUString const & rName,
    OUString const & rArguments,
    OUString const & rTemplate);

}
#endif // INCLUDED_UCBHELPER_REGISTERUCB_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
