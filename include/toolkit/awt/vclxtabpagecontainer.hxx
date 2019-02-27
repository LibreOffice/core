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

#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/awt/tab/XTabPageContainer.hpp>
#include <cppuhelper/implbase.hxx>
#include <toolkit/helper/listenermultiplexer.hxx>
#include <toolkit/awt/vclxcontainer.hxx>


typedef cppu::ImplInheritanceHelper< VCLXContainer,
                                     css::awt::tab::XTabPageContainer,
                                     css::container::XContainerListener
                                   > VCLXTabPageContainer_Base;
class VCLXTabPageContainer : public VCLXTabPageContainer_Base
{
public:
    VCLXTabPageContainer();
    virtual ~VCLXTabPageContainer() override;

    // css::awt::XView
    void SAL_CALL draw( sal_Int32 nX, sal_Int32 nY ) override;

    // css::awt::grid::XTabPageContainer
    virtual ::sal_Int16 SAL_CALL getActiveTabPageID() override;
    virtual void SAL_CALL setActiveTabPageID( ::sal_Int16 _activetabpageid ) override;
    virtual ::sal_Int16 SAL_CALL getTabPageCount(  ) override;
    virtual sal_Bool SAL_CALL isTabPageActive( ::sal_Int16 tabPageIndex ) override;
    virtual css::uno::Reference< css::awt::tab::XTabPage > SAL_CALL getTabPage( ::sal_Int16 tabPageIndex ) override;
    virtual css::uno::Reference< css::awt::tab::XTabPage > SAL_CALL getTabPageByID( ::sal_Int16 tabPageID ) override;
    virtual void SAL_CALL addTabPageContainerListener( const css::uno::Reference< css::awt::tab::XTabPageContainerListener >& listener ) override;
    virtual void SAL_CALL removeTabPageContainerListener( const css::uno::Reference< css::awt::tab::XTabPageContainerListener >& listener ) override;

    virtual void    GetPropertyIds( std::vector< sal_uInt16 > &aIds ) override;

    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;
    virtual void SAL_CALL elementInserted( const css::container::ContainerEvent& Event ) override;
    virtual void SAL_CALL elementRemoved( const css::container::ContainerEvent& Event ) override;
    virtual void SAL_CALL elementReplaced( const css::container::ContainerEvent& Event ) override;

    // css::awt::XVclWindowPeer
    void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) override;
protected:
    virtual void    ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent ) override;
private:
    TabPageListenerMultiplexer m_aTabPageListeners;
    ::std::vector< css::uno::Reference< css::awt::tab::XTabPage > > m_aTabPages;
};
#endif // INCLUDED_TOOLKIT_AWT_VCLXTABPAGECONTAINER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
