/*************************************************************************
 *
 *  $RCSfile: registerucb.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: sb $ $Date: 2000-11-09 13:23:55 $
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

#ifndef _UCBHELPER_REGISTERUCB_HXX_
#define _UCBHELPER_REGISTERUCB_HXX_

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

namespace com { namespace sun { namespace star {
    namespace lang { class XMultiServiceFactory; }
    namespace ucb { class XContentProviderManager; }
    namespace uno { class RuntimeException; }
} } }
namespace rtl { class OUString; }

namespace ucb {

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
void
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
