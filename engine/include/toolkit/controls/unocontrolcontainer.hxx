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

#include <toolkit/dllapi.h>
#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/awt/XUnoControlContainer.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/container/XIdentifierContainer.hpp>

#include <toolkit/controls/unocontrolbase.hxx>

#include <cppuhelper/implbase4.hxx>
#include <memory>

class UnoControlHolderList;



typedef ::cppu::AggImplInheritanceHelper4   <   UnoControlBase
                                            ,   css::awt::XUnoControlContainer
                                            ,   css::awt::XControlContainer
                                            ,   css::container::XContainer
                                            ,   css::container::XIdentifierContainer
                                            >   UnoControlContainer_Base;

class TOOLKIT_DLLPUBLIC UnoControlContainer : public UnoControlContainer_Base
{
private:
    std::unique_ptr<UnoControlHolderList>   mpControls;
    cpo::uno::Sequence< css::uno::Reference< css::awt::XTabController > >    maTabControllers;
    ContainerListenerMultiplexer            maCListeners;

protected:
    void                                    ImplActivateTabControllers();

public:
                UnoControlContainer();
                UnoControlContainer( const css::uno::Reference< css::awt::XVclWindowPeer >& xPeer );
                virtual ~UnoControlContainer() override;


    // css::lang::XComponent
    void dispose() override;

    // css::lang::XEventListener
    void disposing( const css::lang::EventObject& Source ) override;

    // css::container::XContainer
    void addContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener ) override;
    void removeContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener ) override;

    // css::container::XIdentifierContainer
    virtual ::sal_Int32 insert( const cpo::uno::Any& aElement ) override;

    // css::container::XIdentifierReplace
    virtual void removeByIdentifier( ::sal_Int32 Identifier ) override;
    virtual void replaceByIdentifer( ::sal_Int32 Identifier, const cpo::uno::Any& aElement ) override;

    // css::container::XIdentifierAccess
    virtual cpo::uno::Any getByIdentifier( ::sal_Int32 Identifier ) override;
    virtual cpo::uno::Sequence< ::sal_Int32 > getIdentifiers(  ) override;

    // css::container::XElementAccess
    virtual cpo::uno::Type getElementType(  ) override;
    virtual bool hasElements(  ) override;

    // css::awt::XControlContainer
    void setStatusText( const OUString& StatusText ) override;
    cpo::uno::Sequence< css::uno::Reference< css::awt::XControl > > getControls(  ) override;
    css::uno::Reference< css::awt::XControl > getControl( const OUString& aName ) override;
    void addControl( const OUString& Name, const css::uno::Reference< css::awt::XControl >& Control ) override;
    void removeControl( const css::uno::Reference< css::awt::XControl >& Control ) override;

    // css::awt::XUnoControlContainer
    void setTabControllers( const cpo::uno::Sequence< css::uno::Reference< css::awt::XTabController > >& TabControllers ) override;
    cpo::uno::Sequence< css::uno::Reference< css::awt::XTabController > > getTabControllers(  ) override;
    void addTabController( const css::uno::Reference< css::awt::XTabController >& TabController ) override;
    void removeTabController( const css::uno::Reference< css::awt::XTabController >& TabController ) override;

    // css::awt::XControl
    void createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) override;

    // css::awt::XWindow
    void setVisible( bool Visible ) override;

    OUString getImplementationName() override;

    cpo::uno::Sequence<OUString> getSupportedServiceNames() override;

protected:
    virtual void PrepareWindowDescriptor( css::awt::WindowDescriptor& rDesc ) override;
    virtual void removingControl( const css::uno::Reference< css::awt::XControl >& _rxControl );
    virtual void addingControl( const css::uno::Reference< css::awt::XControl >& _rxControl );

    /** ensures that the given control has a peer, if necessary and possible
        @param _rxControl
            an ->XControl which has just been inserted into the container. Must not be <NULL/>.
        @precond
            our mutex is locked
    */
    virtual void    impl_createControlPeerIfNecessary(
        const css::uno::Reference< css::awt::XControl >& _rxControl
    );
private:
    /** adds the control to the container, does necessary notifications, and the like
        @param _rxControl
            the control to add. Must not be <NULL/>
        @param _pName
            Pointer to a name for the control. Might be <NULL/>, in this case an automatic name is generated
        @return
            the ID of the newly added control
    */
    sal_Int32 impl_addControl(
        const css::uno::Reference< css::awt::XControl >& _rxControl,
        const OUString* _pName = nullptr
    );

    /** removes the given control from the container, including necessary notifications and the like
        @param  _nId
            the ID of the control to remove
        @param  _rxControl
            the control itself. Must be the one which is stored under the given ID. This parameter could also be
            obtained inside the method, but callers usually have obtained it, anyway.
    */
    void      impl_removeControl(
        sal_Int32 _nId,
        const css::uno::Reference< css::awt::XControl >& _rxControl
    );

};


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
