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

#ifndef INCLUDED_TOOLKIT_CONTROLS_UNOCONTROLCONTAINER_HXX
#define INCLUDED_TOOLKIT_CONTROLS_UNOCONTROLCONTAINER_HXX


#include <com/sun/star/awt/XControlContainer.hpp>
#include <com/sun/star/awt/XUnoControlContainer.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/container/XIdentifierContainer.hpp>

#include <toolkit/controls/unocontrol.hxx>
#include <toolkit/controls/unocontrolbase.hxx>
#include <toolkit/helper/macros.hxx>
#include <toolkit/helper/servicenames.hxx>

#include <cppuhelper/implbase.hxx>

class UnoControlHolderList;


//  class UnoControlContainer

typedef ::cppu::AggImplInheritanceHelper    <   UnoControlBase
                                            ,   css::awt::XUnoControlContainer
                                            ,   css::awt::XControlContainer
                                            ,   css::container::XContainer
                                            ,   css::container::XIdentifierContainer
                                            >   UnoControlContainer_Base;

class UnoControlContainer : public UnoControlContainer_Base
{
private:
    UnoControlHolderList*                   mpControls;
    css::uno::Sequence< css::uno::Reference< css::awt::XTabController > >    maTabControllers;
    ContainerListenerMultiplexer            maCListeners;

protected:
    void                                    ImplActivateTabControllers();

public:
                UnoControlContainer();
                UnoControlContainer( const css::uno::Reference< css::awt::XWindowPeer >& xPeer );
                virtual ~UnoControlContainer() override;


    // css::lang::XComponent
    void SAL_CALL dispose() throw(css::uno::RuntimeException, std::exception) override;

    // css::lang::XEventListener
    void SAL_CALL disposing( const css::lang::EventObject& Source ) throw(css::uno::RuntimeException, std::exception) override;

    // css::container::XContainer
    void SAL_CALL addContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeContainerListener( const css::uno::Reference< css::container::XContainerListener >& xListener ) throw(css::uno::RuntimeException, std::exception) override;

    // css::container::XIdentifierContainer
    virtual ::sal_Int32 SAL_CALL insert( const css::uno::Any& aElement ) throw (css::lang::IllegalArgumentException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    // css::container::XIdentifierReplace
    virtual void SAL_CALL removeByIdentifier( ::sal_Int32 Identifier ) throw (css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL replaceByIdentifer( ::sal_Int32 Identifier, const css::uno::Any& aElement ) throw (css::lang::IllegalArgumentException, css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

    // css::container::XIdentifierAccess
    virtual css::uno::Any SAL_CALL getByIdentifier( ::sal_Int32 Identifierr ) throw (css::container::NoSuchElementException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< ::sal_Int32 > SAL_CALL getIdentifiers(  ) throw (css::uno::RuntimeException, std::exception) override;

    // css::container::XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements(  ) throw (css::uno::RuntimeException, std::exception) override;

    // css::awt::XControlContainer
    void SAL_CALL setStatusText( const OUString& StatusText ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< css::uno::Reference< css::awt::XControl > > SAL_CALL getControls(  ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Reference< css::awt::XControl > SAL_CALL getControl( const OUString& aName ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL addControl( const OUString& Name, const css::uno::Reference< css::awt::XControl >& Control ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeControl( const css::uno::Reference< css::awt::XControl >& Control ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XUnoControlContainer
    void SAL_CALL setTabControllers( const css::uno::Sequence< css::uno::Reference< css::awt::XTabController > >& TabControllers ) throw(css::uno::RuntimeException, std::exception) override;
    css::uno::Sequence< css::uno::Reference< css::awt::XTabController > > SAL_CALL getTabControllers(  ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL addTabController( const css::uno::Reference< css::awt::XTabController >& TabController ) throw(css::uno::RuntimeException, std::exception) override;
    void SAL_CALL removeTabController( const css::uno::Reference< css::awt::XTabController >& TabController ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XControl
    void SAL_CALL createPeer( const css::uno::Reference< css::awt::XToolkit >& Toolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent ) throw(css::uno::RuntimeException, std::exception) override;

    // css::awt::XWindow
    void SAL_CALL setVisible( sal_Bool Visible ) throw(css::uno::RuntimeException, std::exception) override;

    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames()
        throw (css::uno::RuntimeException, std::exception) override;

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


#endif // INCLUDED_TOOLKIT_CONTROLS_UNOCONTROLCONTAINER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
