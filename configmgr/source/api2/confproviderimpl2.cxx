/*************************************************************************
 *
 *  $RCSfile: confproviderimpl2.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: lla $ $Date: 2000-11-13 13:14:44 $
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

#include <stdio.h>
#include "confproviderimpl2.hxx"
#include "roottree.hxx"
#include "noderef.hxx"
#include "apitreeaccess.hxx"
#include "objectregistry.hxx"

#ifndef CONFIGMGR_API_PROVIDER2_HXX_
#include "confprovider2.hxx"
#endif
#ifndef CONFIGMGR_MODULE_HXX_
#include "configmodule.hxx"
#endif

#ifndef _CONFIGMGR_TREECACHE_HXX_
#include "treecache.hxx"
#endif
#ifndef _CONFIGMGR_TRACER_HXX_
#include "tracer.hxx"
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

#include "hashhelper.hxx"

namespace configmgr
{
    namespace css   = ::com::sun::star;
    namespace uno   = css::uno;
    namespace beans = css::beans;

    using ::rtl::OUString;

    using configapi::NodeElement;
    using configuration::RootTree;

    //=============================================================================
    //= OConfigurationProviderImpl
    //=============================================================================
    //-----------------------------------------------------------------------------------
    OConfigurationProviderImpl::OConfigurationProviderImpl(OConfigurationProvider* _pProvider, IConfigSession* _pSession, const uno::Reference< script::XTypeConverter >& _rxTypeConverter)
                               :OProviderImpl(_pProvider, _pSession, _rxTypeConverter)
    {
    }
    //--------------------------------------------------------------------------
    uno::Reference<uno::XInterface>  OConfigurationProviderImpl::createReadAccess( uno::Sequence<uno::Any> const& aArgs)
        throw (uno::Exception, uno::RuntimeException)
    {
        CFG_TRACE_INFO("config provider: going to create a read access instance");

        // extract the args
        ::rtl::OUString sUser, sPath, sLocale;
        sal_Int32 nLevels;
        OProviderImpl::FactoryArguments::extractArgs(aArgs, sPath, sUser, sLocale, nLevels);
        ::rtl::OUString sNodeAccessor = IConfigSession::composeNodeAccessor(sPath, sUser);

        m_pConfiguration->setOptions(getOptions());
        getOptions()->add("Locale", sLocale);

        CFG_TRACE_INFO_NI("config provider: node accessor extracted from the args is %s", OUSTRING2ASCII(sNodeAccessor));
        CFG_TRACE_INFO_NI("config provider: level depth extracted from the args is %i", nLevels);

        // create the access object
        uno::Reference< uno::XInterface > xReturn;
        if (aArgs.getLength() != 0)
        {
            NodeElement* pElement = buildReadAccess(sNodeAccessor, nLevels);
            if (pElement != 0)
            {
                xReturn = pElement->getUnoInstance();
                if (xReturn.is())
                    // acquired once by buildReadAccess
                    xReturn->release();
            }
        }
        return xReturn;
    }


    //-----------------------------------------------------------------------------------
    uno::Reference<uno::XInterface> OConfigurationProviderImpl::createUpdateAccess( uno::Sequence<uno::Any> const& aArgs)
        throw (uno::Exception, uno::RuntimeException)
    {
        CFG_TRACE_INFO("config provider: going to create an update access instance");

        // extract the args
        sal_Int32 nLevels;
        ::rtl::OUString sUser, sPath, sLocale;
        OProviderImpl::FactoryArguments::extractArgs(aArgs, sPath, sUser, sLocale, nLevels);
        ::rtl::OUString sNodeAccessor = IConfigSession::composeNodeAccessor(sPath, sUser);

        CFG_TRACE_INFO_NI("config provider: node accessor extracted from the args is %s", OUSTRING2ASCII(sNodeAccessor));
        CFG_TRACE_INFO_NI("config provider: level depth extracted from the args is %i", nLevels);

        m_pConfiguration->setOptions(getOptions());
        getOptions()->add("Locale", sLocale);

        // create the access object
        uno::Reference< uno::XInterface > xReturn;
        if (aArgs.getLength() != 0)
        {
            NodeElement* pElement = buildUpdateAccess(sNodeAccessor, nLevels);
            if (pElement != 0)
            {
                xReturn = pElement->getUnoInstance();
                if (xReturn.is())
                    // acquired once by buildReadAccess
                    xReturn->release();
            }
        }
        return xReturn;
    }
} // namespace configmgr


