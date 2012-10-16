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

#include <uifactory/statusbarfactory.hxx>
#include <uifactory/menubarfactory.hxx>

#include <uielement/statusbarwrapper.hxx>
#include <threadhelp/resetableguard.hxx>

#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>

#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>

#include <vcl/svapp.hxx>
#include <rtl/ustrbuf.hxx>

//_________________________________________________________________________________________________________________
//  Defines
//_________________________________________________________________________________________________________________

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;
using namespace ::com::sun::star::ui;

namespace framework
{

//*****************************************************************************************************************
//  XInterface, XTypeProvider, XServiceInfo
//*****************************************************************************************************************
DEFINE_XSERVICEINFO_ONEINSTANCESERVICE  (   StatusBarFactory                                ,
                                            ::cppu::OWeakObject                             ,
                                            SERVICENAME_STATUSBARFACTORY                    ,
                                            IMPLEMENTATIONNAME_STATUSBARFACTORY
                                        )

DEFINE_INIT_SERVICE                     (   StatusBarFactory, {} )

StatusBarFactory::StatusBarFactory( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xServiceManager ) :
    MenuBarFactory( xServiceManager,true )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "StatusBarFactory::StatusBarFactory" );
}

// XUIElementFactory
Reference< XUIElement > SAL_CALL StatusBarFactory::createUIElement(
    const ::rtl::OUString& ResourceURL,
    const Sequence< PropertyValue >& Args )
throw ( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "framework", "Ocke.Janssen@sun.com", "StatusBarFactory::createUIElement" );
    // SAFE
    ResetableGuard aLock( m_aLock );
    StatusBarWrapper* pWrapper = new StatusBarWrapper( m_xServiceManager );
    Reference< ::com::sun::star::ui::XUIElement > xMenuBar( (OWeakObject *)pWrapper, UNO_QUERY );
    Reference< ::com::sun::star::frame::XModuleManager2 > xModuleManager = m_xModuleManager;
    aLock.unlock();
    MenuBarFactory::CreateUIElement(ResourceURL,Args,NULL,"private:resource/statusbar/",xMenuBar,xModuleManager,m_xServiceManager);
    return xMenuBar;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
