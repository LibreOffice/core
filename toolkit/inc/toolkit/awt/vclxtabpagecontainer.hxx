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

#ifndef _TOOLKIT_AWT_VCLXTABPAGECONTAINER_HXX_
#define _TOOLKIT_AWT_VCLXTABPAGECONTAINER_HXX_

#include <toolkit/dllapi.h>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include "toolkit/awt/vclxwindow.hxx"
#include <com/sun/star/awt/tab/XTabPageContainer.hpp>
#include <toolkit/helper/listenermultiplexer.hxx>
#include <cppuhelper/implbase2.hxx>
#include <toolkit/awt/vclxcontainer.hxx>

//  ----------------------------------------------------
typedef ::cppu::ImplInheritanceHelper2 <    VCLXContainer
                                            ,   ::com::sun::star::awt::tab::XTabPageContainer
                                            ,   ::com::sun::star::container::XContainerListener
                                             > VCLXTabPageContainer_Base;
class VCLXTabPageContainer : public VCLXTabPageContainer_Base
{
public:
    VCLXTabPageContainer();
    ~VCLXTabPageContainer();

    // ::com::sun::star::awt::XView
    void SAL_CALL draw( sal_Int32 nX, sal_Int32 nY ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XDevice,
    ::com::sun::star::awt::DeviceInfo SAL_CALL getInfo() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::grid::XTabPageContainer
    virtual ::sal_Int16 SAL_CALL getActiveTabPageID() throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setActiveTabPageID( ::sal_Int16 _activetabpageid ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int16 SAL_CALL getTabPageCount(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL isTabPageActive( ::sal_Int16 tabPageIndex ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::tab::XTabPage > SAL_CALL getTabPage( ::sal_Int16 tabPageIndex ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::tab::XTabPage > SAL_CALL getTabPageByID( ::sal_Int16 tabPageID ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addTabPageContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tab::XTabPageContainerListener >& listener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeTabPageContainerListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::tab::XTabPageContainerListener >& listener ) throw (::com::sun::star::uno::RuntimeException);

    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds ) { return ImplGetPropertyIds( aIds ); }

    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& Event ) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::awt::XVclWindowPeer
    void SAL_CALL setProperty( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException);
protected:
    virtual void    ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent );
private:
    TabPageListenerMultiplexer m_aTabPageListeners;
    ::std::vector< ::com::sun::star::uno::Reference< ::com::sun::star::awt::tab::XTabPage > > m_aTabPages;
};
#endif // _TOOLKIT_AWT_VCLXTABPAGEMODEL_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
