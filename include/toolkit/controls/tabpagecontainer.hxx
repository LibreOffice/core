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
#include <com/sun/star/awt/tab/XTabPageContainerListener.hpp>
#include <com/sun/star/awt/tab/XTabPage.hpp>
#include <com/sun/star/awt/tab/XTabPageModel.hpp>
#include <toolkit/controls/unocontrolbase.hxx>
#include <toolkit/controls/unocontrolmodel.hxx>
#include <toolkit/helper/servicenames.hxx>
#include <cppuhelper/implbase.hxx>
#include <comphelper/sequence.hxx>
#include <toolkit/controls/controlmodelcontainerbase.hxx>
#include <toolkit/helper/listenermultiplexer.hxx>


//  class css::awt::tab::UnoControlTabPageContainerModel

typedef ::cppu::AggImplInheritanceHelper    <   UnoControlModel
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
    css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(css::uno::RuntimeException, std::exception) override;

public:
    UnoControlTabPageContainerModel( const css::uno::Reference< css::uno::XComponentContext >& i_factory );
    UnoControlTabPageContainerModel( const UnoControlTabPageContainerModel& rModel ) : UnoControlTabPageContainerModel_Base( rModel ),maContainerListeners( *this ) {;}

    UnoControlModel*    Clone() const override { return new UnoControlTabPageContainerModel( *this ); }

    // css::io::XPersistObject
    OUString SAL_CALL getServiceName() throw(css::uno::RuntimeException, std::exception) override;

    // css::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoControlTabPageContainerModel, UnoControlModel, "com.sun.star.awt.tab.UnoControlTabPageContainerModel" )

    // XTabPageContainerModel
    virtual css::uno::Reference< css::awt::tab::XTabPageModel > SAL_CALL createTabPage( ::sal_Int16 TabPageID ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::awt::tab::XTabPageModel > SAL_CALL loadTabPage( ::sal_Int16 TabPageID, const OUString& ResourceURL ) throw (css::uno::RuntimeException, std::exception) override;

    // XIndexContainer
    virtual void SAL_CALL insertByIndex( sal_Int32 Index, const css::uno::Any& Element )
             throw (css::lang::IllegalArgumentException, css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeByIndex( sal_Int32 Index )
             throw (css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    // XIndexReplace
     virtual void SAL_CALL replaceByIndex( sal_Int32 Index, const css::uno::Any& Element )
             throw (css::lang::IllegalArgumentException, css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

     // XIndexAccess
     virtual sal_Int32 SAL_CALL getCount() throw (css::uno::RuntimeException, std::exception) override;

     virtual css::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
             throw (css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

     // XElementAccess
     virtual css::uno::Type SAL_CALL getElementType() throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw (css::uno::RuntimeException, std::exception) override;

    // css::container::XContainer
    void SAL_CALL addContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener ) throw(css::uno::RuntimeException, std::exception) override;
};

// = UnoControlTabPageContainer

typedef ::cppu::AggImplInheritanceHelper    <   ControlContainerBase
                                            ,   css::awt::tab::XTabPageContainer
                                            >   UnoControlTabPageContainer_Base;
class UnoControlTabPageContainer : public UnoControlTabPageContainer_Base
{
public:
    UnoControlTabPageContainer( const css::uno::Reference< css::uno::XComponentContext >& rxContext );
    OUString             GetComponentServiceName() override;

    // css::lang::XComponent
    void SAL_CALL dispose(  ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XControl
    void SAL_CALL createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::tab::XTabPageContainer
    virtual ::sal_Int16 SAL_CALL getActiveTabPageID() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setActiveTabPageID( ::sal_Int16 _activetabpageid ) throw (css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int16 SAL_CALL getTabPageCount(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isTabPageActive( ::sal_Int16 tabPageIndex ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::awt::tab::XTabPage > SAL_CALL getTabPage( ::sal_Int16 tabPageIndex ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::awt::tab::XTabPage > SAL_CALL getTabPageByID( ::sal_Int16 tabPageID ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addTabPageContainerListener( const css::uno::Reference< css::awt::tab::XTabPageContainerListener >& listener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeTabPageContainerListener( const css::uno::Reference< css::awt::tab::XTabPageContainerListener >& listener ) throw (css::uno::RuntimeException, std::exception) override;

    virtual void SAL_CALL addControl( const OUString& Name, const css::uno::Reference< css::awt::XControl >& Control ) throw (css::uno::RuntimeException, std::exception) override;
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
