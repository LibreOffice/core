/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef __FRAMEWORK_UIFACTORY_WINDOWCONTENTFACTORYMANAGER_HXX_
#define __FRAMEWORK_UIFACTORY_WINDOWCONTENTFACTORYMANAGER_HXX_

/** Attention: stl headers must(!) be included at first. Otherwhise it can make trouble
               with solaris headers ...
*/
#include <vector>
#include <list>

#include <threadhelp/threadhelpbase.hxx>
#include <macros/generic.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <stdtypes.h>

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include "com/sun/star/frame/XModuleManager2.hpp"

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
        WindowContentFactoryManager( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>& rxContext );
        virtual ~WindowContentFactoryManager();

        //  XInterface, XTypeProvider, XServiceInfo
        DECLARE_XSERVICEINFO

        // XSingleComponentFactory
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstanceWithContext( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& Context ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL createInstanceWithArgumentsAndContext( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& Arguments, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& Context ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

        static void RetrieveTypeNameFromResourceURL( const ::rtl::OUString& aResourceURL, rtl::OUString& aType, rtl::OUString& aName );
    private:

        sal_Bool                                                                         m_bConfigRead;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModuleManager2 >     m_xModuleManager;
        ConfigurationAccess_FactoryManager*                                 m_pConfigAccess;
};

} // namespace framework

#endif // __FRAMEWORK_UIFACTORY_WINDOWCONTENTFACTORYMANAGER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
