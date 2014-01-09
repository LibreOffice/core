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

#include <uielement/toolbarwrapper.hxx>
#include <threadhelp/resetableguard.hxx>

#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/ui/XModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>

#include <cppuhelper/weak.hxx>
#include <vcl/svapp.hxx>
#include <rtl/ref.hxx>
#include <rtl/ustrbuf.hxx>

#include <macros/xserviceinfo.hxx>
#include <services.h>
#include <uifactory/menubarfactory.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::frame;
using namespace com::sun::star::beans;
using namespace com::sun::star::util;
using namespace ::com::sun::star::ui;
using namespace framework;

namespace {

class ToolBoxFactory :  public MenuBarFactory
{
    public:
        ToolBoxFactory( const css::uno::Reference< css::uno::XComponentContext >& xContext );

        //  XInterface, XTypeProvider, XServiceInfo
        DECLARE_XSERVICEINFO

        // XUIElementFactory
        virtual css::uno::Reference< css::ui::XUIElement > SAL_CALL createUIElement( const OUString& ResourceURL, const css::uno::Sequence< css::beans::PropertyValue >& Args ) throw ( css::container::NoSuchElementException, css::lang::IllegalArgumentException, css::uno::RuntimeException );
};

DEFINE_XSERVICEINFO_ONEINSTANCESERVICE_2(   ToolBoxFactory                                  ,
                                            ::cppu::OWeakObject                             ,
                                            SERVICENAME_TOOLBARFACTORY                      ,
                                            IMPLEMENTATIONNAME_TOOLBARFACTORY
                                        )

DEFINE_INIT_SERVICE                     (   ToolBoxFactory, {} )

ToolBoxFactory::ToolBoxFactory( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext ) :
    MenuBarFactory( xContext,true )
{
}

// XUIElementFactory
Reference< XUIElement > SAL_CALL ToolBoxFactory::createUIElement(
    const OUString& ResourceURL,
    const Sequence< PropertyValue >& Args )
throw ( ::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException )
{
    ResetableGuard aLock( m_aLock );
    ToolBarWrapper* pWrapper = new ToolBarWrapper( m_xContext );
    Reference< ::com::sun::star::ui::XUIElement > xMenuBar( (OWeakObject *)pWrapper, UNO_QUERY );
    Reference< ::com::sun::star::frame::XModuleManager2 > xModuleManager = m_xModuleManager;
    aLock.unlock();
    CreateUIElement(ResourceURL, Args, "PopupMode", "private:resource/toolbar/", xMenuBar, xModuleManager, m_xContext);
    return xMenuBar;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_framework_ToolBarFactory_get_implementation(
        css::uno::XComponentContext * context,
        uno_Sequence * arguments)
{
    assert(arguments != 0 && arguments->nElements == 0); (void) arguments;
    rtl::Reference<ToolBoxFactory> x(new ToolBoxFactory(context));
    x->acquire();
    return static_cast<cppu::OWeakObject *>(x.get());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
