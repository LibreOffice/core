/*************************************************************************
 *
 *  $RCSfile: frameloaderfactory.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-01-28 15:15:29 $
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

#include "frameloaderfactory.hxx"
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
FrameLoaderFactory::FrameLoaderFactory(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
{
    BaseContainer::init(xSMGR                                              ,
                        FrameLoaderFactory::impl_getImplementationName()   ,
                        FrameLoaderFactory::impl_getSupportedServiceNames(),
                        FilterCache::E_FRAMELOADER                         );
}

/*-----------------------------------------------
    09.07.2003 07:43
-----------------------------------------------*/
FrameLoaderFactory::~FrameLoaderFactory()
{
}

/*-----------------------------------------------
    16.07.2003 13:37
-----------------------------------------------*/
css::uno::Reference< css::uno::XInterface > SAL_CALL FrameLoaderFactory::createInstance(const ::rtl::OUString& sLoader)
    throw(css::uno::Exception       ,
          css::uno::RuntimeException)
{
    return createInstanceWithArguments(sLoader, css::uno::Sequence< css::uno::Any >());
}

/*-----------------------------------------------
    17.07.2003 09:00
-----------------------------------------------*/
css::uno::Reference< css::uno::XInterface > SAL_CALL FrameLoaderFactory::createInstanceWithArguments(const ::rtl::OUString&                     sLoader  ,
                                                                                                     const css::uno::Sequence< css::uno::Any >& lArguments)
    throw(css::uno::Exception       ,
          css::uno::RuntimeException)
{
    // SAFE ->
    ::osl::ResettableMutexGuard aLock(m_aLock);

    ::rtl::OUString sRealLoader = sLoader;

    #ifdef _FILTER_CONFIG_MIGRATION_Q_

        /* -> TODO - HACK
            check if the given loader name realy exist ...
            Because our old implementation worked with an internal
            type name instead of a loader name. For a small migration time
            we must simulate this old feature :-( */

        if (!m_rCache->hasItem(FilterCache::E_FRAMELOADER, sLoader) && m_rCache->hasItem(FilterCache::E_TYPE, sLoader))
        {
            _FILTER_CONFIG_LOG_("FrameLoaderFactory::createInstanceWithArguments() ... simulate old type search functionality!\n");

            css::uno::Sequence< ::rtl::OUString > lTypes(1);
            lTypes[0] = sLoader;

            css::uno::Sequence< css::beans::NamedValue > lQuery(1);
            lQuery[0].Name    = PROPNAME_TYPES;
            lQuery[0].Value <<= lTypes;

            css::uno::Reference< css::container::XEnumeration > xSet = createSubSetEnumerationByProperties(lQuery);
            while(xSet->hasMoreElements())
            {
                ::comphelper::SequenceAsHashMap lLoaderProps(xSet->nextElement());
                if (!(lLoaderProps[PROPNAME_NAME] >>= sRealLoader))
                    continue;
            }

            // prevent outside code against NoSuchElementException!
            // But dont implement such defensive strategy for our new create handling :-)
            if (!m_rCache->hasItem(FilterCache::E_FRAMELOADER, sRealLoader))
                return css::uno::Reference< css::uno::XInterface>();
        }

        /* <- HACK */

    #endif // _FILTER_CONFIG_MIGRATION_Q_

    // search loader on cache
    CacheItem aLoader = m_rCache->getItem(m_eType, sRealLoader);

    // create service instance
    css::uno::Reference< css::uno::XInterface > xLoader = m_xSMGR->createInstance(sRealLoader);

    // initialize filter
    css::uno::Reference< css::lang::XInitialization > xInit(xLoader, css::uno::UNO_QUERY);
    if (xInit.is())
    {
        // format: lInitData[0] = seq<PropertyValue>, which contains all configuration properties of this loader
        //         lInitData[1] = lArguments[0]
        //         ...
        //         lInitData[n] = lArguments[n-1]
        css::uno::Sequence< css::beans::PropertyValue > lConfig;
        aLoader >> lConfig;

        ::comphelper::SequenceAsVector< css::uno::Any > stlArguments(lArguments);
        stlArguments.insert(stlArguments.begin(), css::uno::makeAny(lConfig));

        css::uno::Sequence< css::uno::Any > lInitData;
        stlArguments >> lInitData;

        xInit->initialize(lInitData);
    }

    return xLoader;
    // <- SAFE
}

/*-----------------------------------------------
    09.07.2003 07:46
-----------------------------------------------*/
css::uno::Sequence< ::rtl::OUString > SAL_CALL FrameLoaderFactory::getAvailableServiceNames()
    throw(css::uno::RuntimeException)
{
    // must be the same list as ((XNameAccess*)this)->getElementNames() return!
    return getElementNames();
}

/*-----------------------------------------------
    09.07.2003 07:43
-----------------------------------------------*/
::rtl::OUString FrameLoaderFactory::impl_getImplementationName()
{
    return ::rtl::OUString::createFromAscii("com.sun.star.comp.filter.config.FrameLoaderFactory");
}

/*-----------------------------------------------
    09.07.2003 07:43
-----------------------------------------------*/
css::uno::Sequence< ::rtl::OUString > FrameLoaderFactory::impl_getSupportedServiceNames()
{
    css::uno::Sequence< ::rtl::OUString > lServiceNames(1);
    lServiceNames[0] = ::rtl::OUString::createFromAscii("com.sun.star.frame.FrameLoaderFactory");
    return lServiceNames;
}

/*-----------------------------------------------
    09.07.2003 07:43
-----------------------------------------------*/
css::uno::Reference< css::uno::XInterface > SAL_CALL FrameLoaderFactory::impl_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR)
{
    FrameLoaderFactory* pNew = new FrameLoaderFactory(xSMGR);
    return css::uno::Reference< css::uno::XInterface >(static_cast< css::lang::XMultiServiceFactory* >(pNew), css::uno::UNO_QUERY);
}

    } // namespace config
} // namespace filter
