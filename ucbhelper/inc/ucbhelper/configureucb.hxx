/*************************************************************************
 *
 *  $RCSfile: configureucb.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:36 $
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

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

namespace com { namespace sun { namespace star {
    namespace lang { class XMultiServiceFactory; }
    namespace ucb { class XContentProviderManager; }
    namespace uno { class RuntimeException; }
} } }
namespace rtl { class OUString; }

//============================================================================
/** The keys under which the com.sun.star.ucb.Configuration service's
    com::sun::star::ucb::XContentProviderConfigurationManager stores the
    various com::sun::star::ucb::XContentProviderConfigurations.
 */
#define UCBHELPER_CONFIGURATION_KEY_STANDARD "ContentProviderServices"
#define UCBHELPER_CONFIGURATION_KEY_LOCAL "LocalContentProviderServices"

//============================================================================
namespace ucb {

/** Configure a (newly instantiated) Universal Content Broker.

    @descr  This function tries to register at the given UCB all the content
    provider services supplied by the given
    com::sun::star::ucb::XContentProviderConfiguration.

    @param rUcb  A (newly intantiated) Universal Content Broker.

    @param rFactory  A factory through which to obtain the
    com.sun.star.ucb.Configuration service and the various content provider
    services.

    @param rConfigurationKey  The key under which the given
    com.sun.star.ucb.Configuration service's
    com::sun::star::ucb::XContentProviderConfigurationManager stores the
    desired com::sun::star::ucb::XContentProviderConfiguration.

    @return  True if the UCB has successfuly been configured (though not all
    content providers have necessarily been registered due to conflicts with
    already registered providers).
 */
bool
configureUcb(
    com::sun::star::uno::Reference<
            com::sun::star::ucb::XContentProviderManager > const &
        rUcb,
    com::sun::star::uno::Reference<
            com::sun::star::lang::XMultiServiceFactory > const &
        rFactory,
    rtl::OUString const & rConfigurationKey)
    throw (com::sun::star::uno::RuntimeException);

}

#endif // _UCBHELPER_CONFIGUREUCB_HXX_

