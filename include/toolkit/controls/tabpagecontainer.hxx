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

#ifndef INCLUDED_TOOLKIT_CONTROLS_TABPAGECONTAINER_HXX
#define INCLUDED_TOOLKIT_CONTROLS_TABPAGECONTAINER_HXX

#include <com/sun/star/awt/tab/XTabPageContainer.hpp>
#include <com/sun/star/awt/tab/XTabPageContainerModel.hpp>
#include <toolkit/controls/unocontrolbase.hxx>
#include <toolkit/controls/unocontrolmodel.hxx>
#include <cppuhelper/implbase1.hxx>
#include <toolkit/controls/controlmodelcontainerbase.hxx>
#include <toolkit/helper/listenermultiplexer.hxx>


namespace com { namespace sun { namespace star { namespace awt { namespace tab { class XTabPage; } } } } }
namespace com { namespace sun { namespace star { namespace awt { namespace tab { class XTabPageContainerListener; } } } } }
namespace com { namespace sun { namespace star { namespace awt { namespace tab { class XTabPageModel; } } } } }

//  class css::awt::tab::UnoControlTabPageContainerModel

typedef ::cppu::AggImplInheritanceHelper1   <   UnoControlModel
                                            ,   css::awt::tab::XTabPageContainerModel
                                            >   UnoControlTabPageContainerModel_Base;
class UnoControlTabPageContainerModel : public UnoControlTabPageContainerModel_Base
{
private:
    std::vector< css::uno::Reference< css::awt::tab::XTabPageModel > > m_aTabPageVector;
    ContainerListenerMultiplexer        maContainerListeners;
protected:
    css::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;
    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;

public:
    UnoControlTabPageContainerModel( const css::uno::Reference< css::uno::XComponentContext >& i_factory );
    UnoControlTabPageContainerModel( const UnoControlTabPageContainerModel& rModel ) : UnoControlTabPageContainerModel_Base( rModel ),maContainerListeners( *this ) {}

    rtl::Reference<UnoControlModel> Clone() const override { return new UnoControlTabPageContainerModel( *this ); }

    // css::io::XPersistObject
    OUString SAL_CALL getServiceName() override;

    // css::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoControlTabPageContainerModel, UnoControlModel, "com.sun.star.awt.tab.UnoControlTabPageContainerModel" )

    // XTabPageContainerModel
    virtual css::uno::Reference< css::awt::tab::XTabPageModel > SAL_CALL createTabPage( ::sal_Int16 TabPageID ) override;
    virtual css::uno::Reference< css::awt::tab::XTabPageModel > SAL_CALL loadTabPage( ::sal_Int16 TabPageID, const OUString& ResourceURL ) override;

    // XIndexContainer
    virtual void SAL_CALL insertByIndex( sal_Int32 Index, const css::uno::Any& Element ) override;
    virtual void SAL_CALL removeByIndex( sal_Int32 Index ) override;

    // XIndexReplace
     virtual void SAL_CALL replaceByIndex( sal_Int32 Index, const css::uno::Any& Element ) override;

     // XIndexAccess
     virtual sal_Int32 SAL_CALL getCount() override;

     virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) override;

     // XElementAccess
     virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

    // css::container::XContainer
    void SAL_CALL addContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener ) override;
    void SAL_CALL removeContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener ) override;
};

// = UnoControlTabPageContainer

typedef ::cppu::AggImplInheritanceHelper1   <   ControlContainerBase
                                            ,   css::awt::tab::XTabPageContainer
                                            >   UnoControlTabPageContainer_Base;
class UnoControlTabPageContainer : public UnoControlTabPageContainer_Base
{
public:
    UnoControlTabPageContainer( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    OUString             GetComponentServiceName() override;

    // css::lang::XComponent
    void SAL_CALL dispose(  ) override;

    // css::awt::XControl
    void SAL_CALL createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) override;

    // css::awt::tab::XTabPageContainer
    virtual ::sal_Int16 SAL_CALL getActiveTabPageID() override;
    virtual void SAL_CALL setActiveTabPageID( ::sal_Int16 _activetabpageid ) override;
    virtual ::sal_Int16 SAL_CALL getTabPageCount(  ) override;
    virtual sal_Bool SAL_CALL isTabPageActive( ::sal_Int16 tabPageIndex ) override;
    virtual css::uno::Reference< css::awt::tab::XTabPage > SAL_CALL getTabPage( ::sal_Int16 tabPageIndex ) override;
    virtual css::uno::Reference< css::awt::tab::XTabPage > SAL_CALL getTabPageByID( ::sal_Int16 tabPageID ) override;
    virtual void SAL_CALL addTabPageContainerListener( const css::uno::Reference< css::awt::tab::XTabPageContainerListener >& listener ) override;
    virtual void SAL_CALL removeTabPageContainerListener( const css::uno::Reference< css::awt::tab::XTabPageContainerListener >& listener ) override;

    virtual void SAL_CALL addControl( const OUString& Name, const css::uno::Reference< css::awt::XControl >& Control ) override;
    // css::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoControlTabPageContainer, UnoControlBase, "com.sun.star.awt.tab.UnoControlTabPageContainer" )

//  using UnoControl::getPeer;
protected:
    virtual void        updateFromModel() override;
private:
    TabPageListenerMultiplexer  m_aTabPageListeners;
};

#endif // _ INCLUDED_TOOLKIT_CONTROLS_TABPAGECONTAINER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
