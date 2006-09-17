/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: contenthandlerfactory.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 07:36:47 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"

#include "contenthandlerfactory.hxx"
#include "querytokenizer.hxx"
#include "macros.hxx"
#include "constant.hxx"
#include "versions.hxx"

//_______________________________________________
// includes

#ifndef _COM_SUN_STAR_LANG_XINITIALIZATION_HPP_
#include <com/sun/star/lang/XInitialization.hpp>
#endif

#ifndef _COMPHELPER_ENUMHELPER_HXX_
#include <comphelper/enumhelper.hxx>
#endif

//_______________________________________________
// namespace

namespace filter{
    namespace config{

namespace css = ::com::sun::star;

//_______________________________________________
// definitions

/*-----------------------------------------------
    09.07.2003 07:43
-----------------------------------------------*/
ContentHandlerFactory::ContentHandlerFactory(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
{
    BaseContainer::init(xSMGR                                                 ,
                        ContentHandlerFactory::impl_getImplementationName()   ,
                        ContentHandlerFactory::impl_getSupportedServiceNames(),
                        FilterCache::E_CONTENTHANDLER                         );
}

/*-----------------------------------------------
    09.07.2003 07:43
-----------------------------------------------*/
ContentHandlerFactory::~ContentHandlerFactory()
{
}

/*-----------------------------------------------
    16.07.2003 13:38
-----------------------------------------------*/
css::uno::Reference< css::uno::XInterface > SAL_CALL ContentHandlerFactory::createInstance(const ::rtl::OUString& sHandler)
    throw(css::uno::Exception       ,
          css::uno::RuntimeException)
{
    return createInstanceWithArguments(sHandler, css::uno::Sequence< css::uno::Any >());
}

/*-----------------------------------------------
    16.07.2003 13:40
-----------------------------------------------*/
css::uno::Reference< css::uno::XInterface > SAL_CALL ContentHandlerFactory::createInstanceWithArguments(const ::rtl::OUString&                     sHandler  ,
                                                                                                        const css::uno::Sequence< css::uno::Any >& lArguments)
    throw(css::uno::Exception       ,
          css::uno::RuntimeException)
{
    css::uno::Reference< css::uno::XInterface > xHandler;

    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);

    ::rtl::OUString sRealHandler = sHandler;

    #ifdef _FILTER_CONFIG_MIGRATION_Q_

        /* -> TODO - HACK
            check if the given handler name realy exist ...
            Because our old implementation worked with an internal
            type name instead of a handler name. For a small migration time
            we must simulate this old feature :-( */

        if (!m_rCache->hasItem(FilterCache::E_CONTENTHANDLER, sHandler) && m_rCache->hasItem(FilterCache::E_TYPE, sHandler))
        {
            _FILTER_CONFIG_LOG_("ContentHandlerFactory::createInstanceWithArguments() ... simulate old type search functionality!\n");

            css::uno::Sequence< ::rtl::OUString > lTypes(1);
            lTypes[0] = sHandler;

            css::uno::Sequence< css::beans::NamedValue > lQuery(1);
            lQuery[0].Name    = PROPNAME_TYPES;
            lQuery[0].Value <<= lTypes;

            css::uno::Reference< css::container::XEnumeration > xSet = createSubSetEnumerationByProperties(lQuery);
            while(xSet->hasMoreElements())
            {
                ::comphelper::SequenceAsHashMap lHandlerProps(xSet->nextElement());
                if (!(lHandlerProps[PROPNAME_NAME] >>= sRealHandler))
                    continue;
            }

            // prevent outside code against NoSuchElementException!
            // But dont implement such defensive strategy for our new create handling :-)
            if (!m_rCache->hasItem(FilterCache::E_CONTENTHANDLER, sRealHandler))
                return css::uno::Reference< css::uno::XInterface>();
        }

        /* <- HACK */

    #endif // _FILTER_CONFIG_MIGRATION_Q_

    // search handler on cache
    CacheItem aHandler = m_rCache->getItem(FilterCache::E_CONTENTHANDLER, sRealHandler);

    // create service instance
    xHandler = m_xSMGR->createInstance(sRealHandler);

    // initialize filter
    css::uno::Reference< css::lang::XInitialization > xInit(xHandler, css::uno::UNO_QUERY);
    if (xInit.is())
    {
        // format: lInitData[0] = seq<PropertyValue>, which contains all configuration properties of this handler
        //         lInitData[1] = lArguments[0]
        //         ...
        //         lInitData[n] = lArguments[n-1]
        css::uno::Sequence< css::beans::PropertyValue > lConfig;
        aHandler >> lConfig;

        ::comphelper::SequenceAsVector< css::uno::Any > stlArguments(lArguments);
        stlArguments.insert(stlArguments.begin(), css::uno::makeAny(lConfig));

        css::uno::Sequence< css::uno::Any > lInitData;
        stlArguments >> lInitData;

        xInit->initialize(lInitData);
    }

    return xHandler;
    // <- SAFE
}

/*-----------------------------------------------
    09.07.2003 07:46
-----------------------------------------------*/
css::uno::Sequence< ::rtl::OUString > SAL_CALL ContentHandlerFactory::getAvailableServiceNames()
    throw(css::uno::RuntimeException)
{
    // must be the same list as ((XNameAccess*)this)->getElementNames() return!
    return getElementNames();
}

/*-----------------------------------------------
    09.07.2003 07:43
-----------------------------------------------*/
::rtl::OUString ContentHandlerFactory::impl_getImplementationName()
{
    return ::rtl::OUString::createFromAscii("com.sun.star.comp.filter.config.ContentHandlerFactory");
}

/*-----------------------------------------------
    09.07.2003 07:43
-----------------------------------------------*/
css::uno::Sequence< ::rtl::OUString > ContentHandlerFactory::impl_getSupportedServiceNames()
{
    css::uno::Sequence< ::rtl::OUString > lServiceNames(1);
    lServiceNames[0] = ::rtl::OUString::createFromAscii("com.sun.star.frame.ContentHandlerFactory");
    return lServiceNames;
}

/*-----------------------------------------------
    09.07.2003 07:43
-----------------------------------------------*/
css::uno::Reference< css::uno::XInterface > SAL_CALL ContentHandlerFactory::impl_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
{
    ContentHandlerFactory* pNew = new ContentHandlerFactory(xSMGR);
    return css::uno::Reference< css::uno::XInterface >(static_cast< css::lang::XMultiServiceFactory* >(pNew), css::uno::UNO_QUERY);
}

    } // namespace config
} // namespace filter
