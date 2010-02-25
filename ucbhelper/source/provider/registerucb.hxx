/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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

#ifndef _UCBHELPER_REGISTERUCB_HXX_
#define _UCBHELPER_REGISTERUCB_HXX_

#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Reference.h>

namespace com { namespace sun { namespace star {
    namespace lang { class XMultiServiceFactory; }
    namespace ucb { class XContentProviderManager; }
} } }
namespace rtl { class OUString; }

namespace ucbhelper {

struct ContentProviderRegistrationInfo;

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
bool
registerAtUcb(
    com::sun::star::uno::Reference<
            com::sun::star::ucb::XContentProviderManager > const &
        rManager,
    com::sun::star::uno::Reference<
            com::sun::star::lang::XMultiServiceFactory > const &
        rServiceFactory,
    rtl::OUString const & rName,
    rtl::OUString const & rArguments,
    rtl::OUString const & rTemplate,
    ContentProviderRegistrationInfo * pInfo)
    throw (com::sun::star::uno::RuntimeException);

//============================================================================
/** Deregister a content provider from a Universal Content Broker.

    @param rManager  A content provider manager (normally, this would be a
    UCB).  May be null, which is only useful if the content provider is an
    <type>XParamterizedContentProvider</type>s.

    @param rInfo  Information about the content provider to deregister.
 */
void
deregisterFromUcb(
    com::sun::star::uno::Reference<
            com::sun::star::ucb::XContentProviderManager > const &
        rManager,
    ContentProviderRegistrationInfo const & rInfo)
    throw (com::sun::star::uno::RuntimeException);

}

#endif // _UCBHELPER_REGISTERUCB_HXX_
