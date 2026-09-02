/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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

#pragma once

#include <com/sun/star/awt/tab/XTabPageContainer.hpp>
#include <com/sun/star/awt/tab/XTabPageContainerModel.hpp>
#include <toolkit/controls/unocontrolbase.hxx>
#include <toolkit/controls/unocontrolmodel.hxx>
#include <cppuhelper/implbase1.hxx>
#include <controls/controlmodelcontainerbase.hxx>
#include <toolkit/helper/listenermultiplexer.hxx>


namespace com::sun::star::awt::tab { class XTabPageModel; }


typedef ::cppu::AggImplInheritanceHelper1   <   UnoControlModel
                                            ,   css::awt::tab::XTabPageContainerModel
                                            >   UnoControlTabPageContainerModel_Base;
class UnoControlTabPageContainerModel final : public UnoControlTabPageContainerModel_Base
{
private:
    std::vector< css::uno::Reference< css::awt::tab::XTabPageModel > > m_aTabPageVector;
    ContainerListenerMultiplexer        maContainerListeners;

    cpo::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const override;
    ::cppu::IPropertyArrayHelper& getInfoHelper() override;
    // css::beans::XMultiPropertySet
    css::uno::Reference< css::beans::XPropertySetInfo > getPropertySetInfo(  ) override;

public:
    UnoControlTabPageContainerModel( const css::uno::Reference< css::uno::XComponentContext >& i_factory );
    UnoControlTabPageContainerModel( const UnoControlTabPageContainerModel& rModel ) : UnoControlTabPageContainerModel_Base( rModel ),maContainerListeners( *this ) {}

    rtl::Reference<UnoControlModel> Clone() const override { return new UnoControlTabPageContainerModel( *this ); }

    // css::io::XPersistObject
    OUString getServiceName() override;

    // css::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoControlTabPageContainerModel, UnoControlModel, u"com.sun.star.awt.tab.UnoControlTabPageContainerModel"_ustr )

    // XTabPageContainerModel
    virtual css::uno::Reference< css::awt::tab::XTabPageModel > createTabPage( ::sal_Int16 TabPageID ) override;
    virtual css::uno::Reference< css::awt::tab::XTabPageModel > loadTabPage( ::sal_Int16 TabPageID, const OUString& ResourceURL ) override;

    // XIndexContainer
    virtual void insertByIndex( sal_Int32 Index, const cpo::uno::Any& Element ) override;
    virtual void removeByIndex( sal_Int32 Index ) override;

    // XIndexReplace
     virtual void replaceByIndex( sal_Int32 Index, const cpo::uno::Any& Element ) override;

     // XIndexAccess
     virtual sal_Int32 getCount() override;

     virtual cpo::uno::Any getByIndex( sal_Int32 Index ) override;

     // XElementAccess
     virtual cpo::uno::Type getElementType() override;
    virtual bool hasElements() override;

    // css::container::XContainer
    void addContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener ) override;
    void removeContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener ) override;
};

// = UnoControlTabPageContainer

typedef ::cppu::AggImplInheritanceHelper1   <   ControlContainerBase
                                            ,   css::awt::tab::XTabPageContainer
                                            >   UnoControlTabPageContainer_Base;
class UnoControlTabPageContainer final : public UnoControlTabPageContainer_Base
{
public:
    UnoControlTabPageContainer( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    OUString             GetComponentServiceName() const override;

    // css::lang::XComponent
    void dispose(  ) override;

    // css::awt::XControl
    void createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) override;

    // css::awt::tab::XTabPageContainer
    virtual ::sal_Int16 getActiveTabPageID() override;
    virtual void setActiveTabPageID( ::sal_Int16 _activetabpageid ) override;
    virtual ::sal_Int16 getTabPageCount(  ) override;
    virtual bool isTabPageActive( ::sal_Int16 tabPageIndex ) override;
    virtual css::uno::Reference< css::awt::tab::XTabPage > getTabPage( ::sal_Int16 tabPageIndex ) override;
    virtual css::uno::Reference< css::awt::tab::XTabPage > getTabPageByID( ::sal_Int16 tabPageID ) override;
    virtual void addTabPageContainerListener( const css::uno::Reference< css::awt::tab::XTabPageContainerListener >& listener ) override;
    virtual void removeTabPageContainerListener( const css::uno::Reference< css::awt::tab::XTabPageContainerListener >& listener ) override;

    // css::beans::XPropertiesChangeListener
    virtual void propertiesChange( const ::cpo::uno::Sequence< ::css::beans::PropertyChangeEvent >& aEvent ) override;

    virtual void addControl( const OUString& Name, const css::uno::Reference< css::awt::XControl >& Control ) override;
    // css::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoControlTabPageContainer, UnoControlBase, u"com.sun.star.awt.tab.UnoControlTabPageContainer"_ustr )

//  using UnoControl::getPeer;
private:
    virtual void        updateFromModel() override;
    TabPageListenerMultiplexer  m_aTabPageListeners;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
