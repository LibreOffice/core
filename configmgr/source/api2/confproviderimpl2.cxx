/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: confproviderimpl2.cxx,v $
 *
 *  $Revision: 1.31 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:12:10 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include <stdio.h>
#include "confproviderimpl2.hxx"

#ifndef CONFIGMGR_ROOTTREE_HXX_
#include "roottree.hxx"
#endif
#ifndef CONFIGMGR_CONFIGNODE_HXX_
#include "noderef.hxx"
#endif
#ifndef CONFIGMGR_API_TREEACCESS_HXX_
#include "apitreeaccess.hxx"
#endif
#ifndef CONFIGMGR_API_OBJECTREGISTRY_HXX_
#include "objectregistry.hxx"
#endif
#ifndef CONFIGMGR_CONFIGEXCEPT_HXX_
#include "configexcept.hxx"
#endif
#ifndef CONFIGMGR_API_PROVIDER2_HXX_
#include "confprovider2.hxx"
#endif
#ifndef CONFIGMGR_TREEPROVIDER_HXX
#include "treeprovider.hxx"
#endif

#ifndef _CONFIGMGR_TRACER_HXX_
#include "tracer.hxx"
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

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
    OConfigurationProviderImpl::OConfigurationProviderImpl(OConfigurationProvider* _pProvider, CreationContext const & xContext)
                               :OProviderImpl(_pProvider, xContext)
    {
    }
    //--------------------------------------------------------------------------
    uno::Reference<uno::XInterface>  OConfigurationProviderImpl::createReadAccess( uno::Sequence<uno::Any> const& aArgs)
        CFG_UNO_THROW_ALL(  )
    {
        CFG_TRACE_INFO("config provider: going to create a read access instance");

        // extract the args
        OUString sPath;
        sal_Int32 nLevels;

        RequestOptions aOptions = getDefaultOptions();

        OProviderImpl::FactoryArguments::extractArgs(aArgs, sPath, nLevels, aOptions);

        CFG_TRACE_INFO_NI("config provider: node accessor extracted from the args is %s", OUSTRING2ASCII(sPath));
        CFG_TRACE_INFO_NI("config provider: level depth extracted from the args is %i", nLevels);

        // create the access object
        uno::Reference< uno::XInterface > xReturn;

        NodeElement* pElement = buildReadAccess(sPath, aOptions, nLevels);
        if (pElement != 0)
        {
            xReturn = pElement->getUnoInstance();
            if (xReturn.is())
                // acquired once by buildReadAccess
                xReturn->release();
        }

        return xReturn;
    }

    //-----------------------------------------------------------------------------------
    uno::Reference<uno::XInterface> OConfigurationProviderImpl::createUpdateAccess( uno::Sequence<uno::Any> const& aArgs)
        CFG_UNO_THROW_ALL(  )
    {
        CFG_TRACE_INFO("config provider: going to create an update access instance");

        // extract the args
        OUString sPath;
        sal_Int32 nLevels;

        RequestOptions aOptions = getDefaultOptions();

        OProviderImpl::FactoryArguments::extractArgs(aArgs, sPath, nLevels, aOptions);

        CFG_TRACE_INFO_NI("config provider: node accessor extracted from the args is %s", OUSTRING2ASCII(sPath));
        CFG_TRACE_INFO_NI("config provider: level depth extracted from the args is %i", nLevels);

        // create the access object
        uno::Reference< uno::XInterface > xReturn;

        NodeElement* pElement = buildUpdateAccess(sPath, aOptions, nLevels);
        if (pElement != 0)
        {
            xReturn = pElement->getUnoInstance();
            if (xReturn.is())
                // acquired once by buildReadAccess
                xReturn->release();
        }

        return xReturn;
    }
} // namespace configmgr


