/*************************************************************************
 *
 *  $RCSfile: configureucb.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: sb $ $Date: 2000-11-09 13:16:05 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _UCBHELPER_CONFIGUREUCB_HXX_
#define _UCBHELPER_CONFIGUREUCB_HXX_

#include <vector>

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

namespace com { namespace sun { namespace star {
    namespace lang { class XMultiServiceFactory; }
    namespace ucb {
        class XContentProvider;
        class XContentProviderManager;
    }
    namespace uno {
        class Any;
        class RuntimeException;
    }
} } }

namespace ucb {

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
bool
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
