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

#ifndef _UCBHELPER_CONFIGUREUCB_HXX_
#define _UCBHELPER_CONFIGUREUCB_HXX_

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/ustring.hxx>

#include <vector>
#include "ucbhelper/ucbhelperdllapi.h"

namespace com { namespace sun { namespace star {
    namespace lang { class XMultiServiceFactory; }
    namespace ucb {
        class XContentProvider;
        class XContentProviderManager;
    }
    namespace uno { class Any; }
} } }

namespace ucbhelper {

//============================================================================
/** Information about a registered content provider, passed from
    <method>configureUcb</method> to <method>unconfigureUcb</method> (and from
    <method>registerAtUcb</method> to <method>deregisterFromUcb</method>).
 */
struct ContentProviderRegistrationInfo
{
    /** The registered content provider (or null if registration failed).
     */
    com::sun::star::uno::Reference< com::sun::star::ucb::XContentProvider >
        m_xProvider;

    /** The arguments the content provider was instantiated with.
     */
    rtl::OUString m_aArguments;

    /** The URL template the content provider is registered on.
     */
    rtl::OUString m_aTemplate;
};
    //@@@ if registerucb.hxx were exported, too, this might better reside in
    // there...

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
    rtl::OUString ServiceName;

    /** The URL template to use to instanciate the content provider.
     */
    rtl::OUString URLTemplate;

    /** The arguments to use to instanciate the content provider.
     */
    rtl::OUString Arguments;

    ContentProviderData() {};
    ContentProviderData( const rtl::OUString & rService,
                         const rtl::OUString & rTemplate,
                         const rtl::OUString & rArgs )
    : ServiceName( rService ), URLTemplate( rTemplate ), Arguments( rArgs ) {}
};

typedef std::vector< ContentProviderData > ContentProviderDataList;

//============================================================================
/** Configure a (newly instantiated) Universal Content Broker.

    @descr  This function tries to register at the given content provider
    manager all the content provider services listed under a given key in the
    configuration database.

    @param rManager  A content provider manager (normally, this would be a
    newly intantiated UCB).

    @param rServiceFactory  A service factory through which to obtain the
    various services required.

    @param rData  A list containing the data for the content providers for
    the UCB to configure.

    @pInfos  If not null, an entry will be added to this vector for every
    content provider that is registered (sucessfully or not).

    @return  True if the UCB has successfuly been configured (though not all
    content providers have necessarily been registered due to individual
    problems).
 */
bool
configureUcb(
    com::sun::star::uno::Reference<
            com::sun::star::ucb::XContentProviderManager > const &
        rManager,
    com::sun::star::uno::Reference<
            com::sun::star::lang::XMultiServiceFactory > const &
        rServiceFactory,
    ContentProviderDataList const & rData,
    ContentProviderRegistrationInfoList * pInfos)
    throw (com::sun::star::uno::RuntimeException);


//============================================================================
/** Configure a (newly instantiated) Universal Content Broker.

    @descr  This function tries to register at the given content provider
    manager all the content provider services listed under a given key in the
    configuration database.

    @param rManager  A content provider manager (normally, this would be a
    newly intantiated UCB).

    @param rServiceFactory  A service factory through which to obtain the
    various services required.

    @param rArguments  A sequence of at least two strings: the primary and
    secondary key addressing a chosen UCB configuration in the configuration
    database.  The sequence can be longer, in which case the excess elements
    must be strings that form key/value pairs.  These key/value pairs will be
    used to replace placeholders in the data from the configuration database
    with (dynamic) values.  This is a (rather unstructured) sequence of
    <type>Any<type/>s, since normally this parameter will simply be forwarded
    by the UCB's <method>initialize<method/> method, which has a parameter of
    the same type.

    @pInfos  If not null, an entry will be added to this vector for every
    content provider that is registered (sucessfully or not).

    @return  True if the UCB has successfuly been configured (though not all
    content providers have necessarily been registered due to individual
    problems).
 */
UCBHELPER_DLLPUBLIC bool
configureUcb(
    com::sun::star::uno::Reference<
            com::sun::star::ucb::XContentProviderManager > const &
        rManager,
    com::sun::star::uno::Reference<
            com::sun::star::lang::XMultiServiceFactory > const &
        rServiceFactory,
    com::sun::star::uno::Sequence< com::sun::star::uno::Any > const &
        rArguments,
    std::vector< ContentProviderRegistrationInfo > * pInfos)
    throw (com::sun::star::uno::RuntimeException);

//============================================================================
/** Undo the configuration of a Universal Content Broker.

    @descr  This function is the reverse of <method>configureUcb</method>.

    @param rManager  A content provider manager.

    @param rInfos  Information about all the registered content providers.
 */
void
unconfigureUcb(
    com::sun::star::uno::Reference<
            com::sun::star::ucb::XContentProviderManager > const &
        rManager,
    std::vector< ContentProviderRegistrationInfo > const & rInfos)
    throw (com::sun::star::uno::RuntimeException);

}

#endif // _UCBHELPER_CONFIGUREUCB_HXX_
