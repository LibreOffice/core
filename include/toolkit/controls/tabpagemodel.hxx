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
#include <cppuhelper/implbase2.hxx>

class UnoControlTabPageModel :  public ControlModelContainerBase
{
protected:
    ::com::sun::star::uno::Any          ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper&       SAL_CALL getInfoHelper() override;
public:
    UnoControlTabPageModel( ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & i_factory);

    // ::com::sun::star::io::XPersistObject
    OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    // XInitialization
    virtual void SAL_CALL initialize (const com::sun::star::uno::Sequence<com::sun::star::uno::Any>& rArguments)
            throw (com::sun::star::uno::Exception, com::sun::star::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    OUString SAL_CALL getImplementationName() throw(css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception) override;
};


//  class UnoTabPageControl

typedef ::cppu::AggImplInheritanceHelper2   <   ControlContainerBase
                                            ,   ::com::sun::star::awt::tab::XTabPage
                                            ,   ::com::sun::star::awt::XWindowListener
                                            >   UnoControlTabPage_Base;
class UnoControlTabPage : public UnoControlTabPage_Base
{
private:
    bool            m_bWindowListener;
public:

    UnoControlTabPage( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext );
    virtual ~UnoControlTabPage();
    OUString             GetComponentServiceName() override;

    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

     // ::com::sun::star::awt::XWindowListener
    virtual void SAL_CALL windowResized( const ::com::sun::star::awt::WindowEvent& e ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL windowMoved( const ::com::sun::star::awt::WindowEvent& e ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL windowShown( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL windowHidden( const ::com::sun::star::lang::EventObject& e ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    // ::com::sun::star::lang::XServiceInfo
    virtual OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    virtual sal_Bool SAL_CALL supportsService(OUString const & ServiceName)
        throw (css::uno::RuntimeException, std::exception) override;

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;
};

#endif // INCLUDED_TOOLKIT_CONTROLS_TABPAGEMODEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
