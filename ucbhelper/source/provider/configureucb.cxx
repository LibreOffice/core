/*************************************************************************
 *
 *  $RCSfile: configureucb.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:37 $
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
#include <ucbhelper/configureucb.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_CONTENTPROVIDERSERVICEINFO2_HPP_
#include <com/sun/star/ucb/ContentProviderServiceInfo2.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_DUPLICATEPROVIDEREXCEPTION_HPP_
#include <com/sun/star/ucb/DuplicateProviderException.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTPROVIDER_HPP_
#include <com/sun/star/ucb/XContentProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTPROVIDERCONFIGURATION_HPP_
#include <com/sun/star/ucb/XContentProviderConfiguration.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTPROVIDERCONFIGURATIONMANAGER_HPP_
#include <com/sun/star/ucb/XContentProviderConfigurationManager.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCONTENTPROVIDERMANAGER_HPP_
#include <com/sun/star/ucb/XContentProviderManager.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XPARAMETERIZEDCONTENTPROVIDER_HPP_
#include <com/sun/star/ucb/XParameterizedContentProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_EXCEPTION_HPP_
#include <com/sun/star/uno/Exception.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HPP_
#include <com/sun/star/uno/RuntimeException.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _UCBHELPER_REGISTERUCB_HXX_
#include <ucbhelper/registerucb.hxx>
#endif

using namespace com::sun;
using namespace com::sun::star;

//============================================================================
//
//  configureUcb
//
//============================================================================

namespace ucb {

bool
configureUcb(
    uno::Reference< star::ucb::XContentProviderManager > const & rUcb,
    uno::Reference< lang::XMultiServiceFactory > const & rFactory,
    rtl::OUString const & rConfigurationKey)
    throw (uno::RuntimeException)
{
    if (!(rUcb.is() && rFactory.is()))
        return false;

    uno::Reference< star::ucb::XContentProviderConfigurationManager >
        xManager;
    try
    {
        xManager
            = uno::Reference<
                      star::ucb::XContentProviderConfigurationManager >(
                  rFactory->
                      createInstance(rtl::OUString::createFromAscii(
                                         "com.sun.star.ucb.Configuration")),
                  uno::UNO_QUERY);
    }
    catch (uno::RuntimeException const &) { throw; }
    catch (uno::Exception const &) {}
    if (!xManager.is())
        return false;

    uno::Reference< star::ucb::XContentProviderConfiguration >
        xConfiguration(xManager->queryContentProviderConfiguration(
                                     rConfigurationKey));
    if (!xConfiguration.is())
        return false;

    registerAtUcb(rUcb,
                  rFactory,
                  xConfiguration->
                      queryContentProviderServiceInfo(rtl::OUString()),
                  0);
    return true;
}

}


