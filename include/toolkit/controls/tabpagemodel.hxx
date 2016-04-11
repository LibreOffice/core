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

#ifndef INCLUDED_TOOLKIT_CONTROLS_TABPAGEMODEL_HXX
#define INCLUDED_TOOLKIT_CONTROLS_TABPAGEMODEL_HXX

#include <toolkit/controls/controlmodelcontainerbase.hxx>
#include <com/sun/star/awt/tab/XTabPageModel.hpp>
#include <com/sun/star/awt/tab/XTabPage.hpp>
#include <com/sun/star/resource/XStringResourceResolver.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <toolkit/helper/servicenames.hxx>
#include <toolkit/helper/macros.hxx>
#include <toolkit/controls/unocontrolcontainer.hxx>
#include <cppuhelper/basemutex.hxx>
#include <list>
#include <cppuhelper/implbase.hxx>

class UnoControlTabPageModel :  public ControlModelContainerBase
{
protected:
    css::uno::Any          ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper&       SAL_CALL getInfoHelper() override;
public:
    UnoControlTabPageModel( css::uno::Reference< css::uno::XComponentContext > const & i_factory);

    // css::io::XPersistObject
    OUString SAL_CALL getServiceName() throw(css::uno::RuntimeException, std::exception) override;

    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;
    // XInitialization
    virtual void SAL_CALL initialize (const css::uno::Sequence<css::uno::Any>& rArguments)
            throw (css::uno::Exception, css::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    OUString SAL_CALL getImplementationName() throw(css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;
};


//  class UnoTabPageControl

typedef ::cppu::AggImplInheritanceHelper    <   ControlContainerBase
                                            ,   css::awt::tab::XTabPage
                                            ,   css::awt::XWindowListener
                                            >   UnoControlTabPage_Base;
class UnoControlTabPage : public UnoControlTabPage_Base
{
private:
    bool            m_bWindowListener;
public:

    UnoControlTabPage( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    virtual ~UnoControlTabPage();
    OUString             GetComponentServiceName() override;

    void SAL_CALL createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL disposing( const css::lang::EventObject& Source ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL dispose() throw(css::uno::RuntimeException, std::exception) override;

     // css::awt::XWindowListener
    virtual void SAL_CALL windowResized( const css::awt::WindowEvent& e ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL windowMoved( const css::awt::WindowEvent& e ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL windowShown( const css::lang::EventObject& e ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL windowHidden( const css::lang::EventObject& e ) throw (css::uno::RuntimeException, std::exception) override;
    // css::lang::XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;
};

#endif // INCLUDED_TOOLKIT_CONTROLS_TABPAGEMODEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
