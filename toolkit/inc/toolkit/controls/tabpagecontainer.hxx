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

#ifndef TOOLKIT_TABPAGE_CONTAINER_HXX
#define TOOLKIT_TABPAGE_CONTAINER_HXX

#include <com/sun/star/awt/tab/XTabPageContainer.hpp>
#include <com/sun/star/awt/tab/XTabPageContainerModel.hpp>
#include <com/sun/star/awt/tab/XTabPageContainerListener.hpp>
#include <com/sun/star/awt/tab/XTabPage.hpp>
#include <com/sun/star/awt/tab/XTabPageModel.hpp>
#include <toolkit/controls/unocontrolbase.hxx>
#include <toolkit/controls/unocontrolmodel.hxx>
#include <toolkit/helper/servicenames.hxx>
#include <cppuhelper/implbase1.hxx>
#include <comphelper/sequence.hxx>
#include <toolkit/controls/controlmodelcontainerbase.hxx>
#include <toolkit/controls/unocontrolmodel.hxx>
#include <toolkit/helper/listenermultiplexer.hxx>

using namespace ::com::sun::star::uno;
//using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;

//  ------------------------------------------------------------------
//  class ::com::sun::star::awt::tab::UnoControlTabPageContainerModel
//  ------------------------------------------------------------------
typedef ::cppu::AggImplInheritanceHelper1   <   UnoControlModel
                                            ,   ::com::sun::star::awt::tab::XTabPageContainerModel
                                            >   UnoControlTabPageContainerModel_Base;
class UnoControlTabPageContainerModel : public UnoControlTabPageContainerModel_Base
{
private:
    std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::awt::tab::XTabPageModel > > m_aTabPageVector;
    ContainerListenerMultiplexer        maContainerListeners;
protected:
    ::com::sun::star::uno::Any      ImplGetDefaultValue( sal_uInt16 nPropId ) const;
    ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();
    // ::com::sun::star::beans::XMultiPropertySet
    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);

public:
    UnoControlTabPageContainerModel( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& i_factory );
    UnoControlTabPageContainerModel( const UnoControlTabPageContainerModel& rModel ) : UnoControlTabPageContainerModel_Base( rModel ),maContainerListeners( *this ) {;}

    UnoControlModel*    Clone() const { return new UnoControlTabPageContainerModel( *this ); }

    // ::com::sun::star::io::XPersistObject
    ::rtl::OUString SAL_CALL getServiceName() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoControlTabPageContainerModel, UnoControlModel, szServiceName_UnoControlTabPageContainerModel )

    // XTabPageContainerModel
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::tab::XTabPageModel > SAL_CALL createTabPage( ::sal_Int16 TabPageID ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::tab::XTabPageModel > SAL_CALL loadTabPage( ::sal_Int16 TabPageID, const ::rtl::OUString& ResourceURL ) throw (::com::sun::star::uno::RuntimeException);

    // XIndexContainer
    virtual void SAL_CALL insertByIndex( sal_Int32 Index, const ::com::sun::star::uno::Any& Element )
             throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeByIndex( sal_Int32 Index )
             throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XIndexReplace
     virtual void SAL_CALL replaceByIndex( sal_Int32 Index, const ::com::sun::star::uno::Any& Element )
             throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

     // XIndexAccess
     virtual sal_Int32 SAL_CALL getCount() throw (::com::sun::star::uno::RuntimeException);

     virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index )
             throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

     // XElementAccess
     virtual ::com::sun::star::uno::Type SAL_CALL getElementType() throw (::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::container::XContainer
    void SAL_CALL addContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    void SAL_CALL removeContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
};
// ===================================================================
// = UnoControlTabPageContainer
// ===================================================================
typedef ::cppu::AggImplInheritanceHelper1   <   ControlContainerBase
                                            ,   ::com::sun::star::awt::tab::XTabPageContainer
                                            >   UnoControlTabPageContainer_Base;
class UnoControlTabPageContainer : public UnoControlTabPageContainer_Base
{
public:
    UnoControlTabPageContainer( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& i_factory );
    ::rtl::OUString             GetComponentServiceName();

    // ::com::sun::star::lang::XComponent
    void SAL_CALL dispose(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XControl
    void SAL_CALL createPeer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& Toolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::tab::XTabPageContainer
    virtual ::sal_Int16 SAL_CALL getActiveTabPageID() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setActiveTabPageID( ::sal_Int16 _activetabpageid ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int16 SAL_CALL getTabPageCount(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL isTabPageActive( ::sal_Int16 tabPageIndex ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::tab::XTabPage > SAL_CALL getTabPage( ::sal_Int16 tabPageIndex ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::tab::XTabPage > SAL_CALL getTabPageByID( ::sal_Int16 tabPageID ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addTabPageContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tab::XTabPageContainerListener >& listener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeTabPageContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tab::XTabPageContainerListener >& listener ) throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL addControl( const ::rtl::OUString& Name, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl >& Control ) throw (::com::sun::star::uno::RuntimeException);
    // ::com::sun::star::lang::XServiceInfo
    DECLIMPL_SERVICEINFO_DERIVED( UnoControlTabPageContainer, UnoControlBase, szServiceName_UnoControlTabPageContainer )

//  using UnoControl::getPeer;
protected:
    virtual void        updateFromModel();
private:
    TabPageListenerMultiplexer  m_aTabPageListeners;
};

#endif // _TOOLKIT_TABPAGE_CONTAINER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
