/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef __FRAMEWORK_UIFACTORY_WINDOWCONTENTFACTORYMANAGER_HXX_
#define __FRAMEWORK_UIFACTORY_WINDOWCONTENTFACTORYMANAGER_HXX_

/** Attention: stl headers must(!) be included at first. Otherwhise it can make trouble
               with solaris headers ...
*/
#include <vector>
#include <list>

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________
#include <threadhelp/threadhelpbase.hxx>
#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <stdtypes.h>

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include "com/sun/star/frame/XModuleManager.hpp"

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________
#include <cppuhelper/implbase2.hxx>
#include <rtl/ustring.hxx>

namespace framework
{

class ConfigurationAccess_FactoryManager;
class WindowContentFactoryManager : private ThreadHelpBase                                        , // Struct for right initalization of mutex member! Must be first of baseclasses.
                                    public ::cppu::WeakImplHelper2< com::sun::star::lang::XServiceInfo                    ,
                                                             com::sun::star::lang::XSingleComponentFactory>
{
    public:
        WindowContentFactoryManager( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager );
        virtual ~WindowContentFactoryManager();

        //  XInterface, XTypeProvider, XServiceInfo
        DECLARE_XSERVICEINFO

        // XSingleComponentFactory
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstanceWithContext( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& Context ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstanceWithArgumentsAndContext( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Arguments, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& Context ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

        static void RetrieveTypeNameFromResourceURL( const ::rtl::OUString& aResourceURL, rtl::OUString& aType, rtl::OUString& aName );
    private:

        sal_Bool                                                                         m_bConfigRead;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xServiceManager;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModuleManager >      m_xModuleManager;
        ConfigurationAccess_FactoryManager*                                 m_pConfigAccess;
};

} // namespace framework

#endif // __FRAMEWORK_UIFACTORY_WINDOWCONTENTFACTORYMANAGER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
