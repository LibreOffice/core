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

#ifndef INCLUDED_TOOLKIT_AWT_VCLXTABPAGECONTAINER_HXX
#define INCLUDED_TOOLKIT_AWT_VCLXTABPAGECONTAINER_HXX

#include <toolkit/dllapi.h>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <toolkit/awt/vclxwindow.hxx>
#include <com/sun/star/awt/tab/XTabPageContainer.hpp>
#include <toolkit/helper/listenermultiplexer.hxx>
#include <cppuhelper/implbase.hxx>
#include <toolkit/awt/vclxcontainer.hxx>


typedef ::cppu::ImplInheritanceHelper  <    VCLXContainer
                                            ,   css::awt::tab::XTabPageContainer
                                            ,   css::container::XContainerListener
                                             > VCLXTabPageContainer_Base;
class VCLXTabPageContainer : public VCLXTabPageContainer_Base
{
public:
    VCLXTabPageContainer();
    virtual ~VCLXTabPageContainer();

    // css::awt::XView
    void SAL_CALL draw( sal_Int32 nX, sal_Int32 nY ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XDevice,
    css::awt::DeviceInfo SAL_CALL getInfo() throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::grid::XTabPageContainer
    virtual ::sal_Int16 SAL_CALL getActiveTabPageID() throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setActiveTabPageID( ::sal_Int16 _activetabpageid ) throw (css::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int16 SAL_CALL getTabPageCount(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isTabPageActive( ::sal_Int16 tabPageIndex ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::awt::tab::XTabPage > SAL_CALL getTabPage( ::sal_Int16 tabPageIndex ) throw (css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::awt::tab::XTabPage > SAL_CALL getTabPageByID( ::sal_Int16 tabPageID ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addTabPageContainerListener( const css::uno::Reference< css::awt::tab::XTabPageContainerListener >& listener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeTabPageContainerListener( const css::uno::Reference< css::awt::tab::XTabPageContainerListener >& listener ) throw (css::uno::RuntimeException, std::exception) override;

    static void     ImplGetPropertyIds( std::list< sal_uInt16 > &aIds );
    virtual void    GetPropertyIds( std::list< sal_uInt16 > &aIds ) override { return ImplGetPropertyIds( aIds ); }

    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL elementInserted( const css::container::ContainerEvent& Event ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL elementRemoved( const css::container::ContainerEvent& Event ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL elementReplaced( const css::container::ContainerEvent& Event ) throw (css::uno::RuntimeException, std::exception) override;

    // css::awt::XVclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) throw(css::uno::RuntimeException, std::exception) override;
protected:
    virtual void    ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;
private:
    TabPageListenerMultiplexer m_aTabPageListeners;
    ::std::vector< css::uno::Reference< css::awt::tab::XTabPage > > m_aTabPages;
};
#endif // INCLUDED_TOOLKIT_AWT_VCLXTABPAGECONTAINER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
