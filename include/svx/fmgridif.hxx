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
#ifndef INCLUDED_SVX_FMGRIDIF_HXX
#define INCLUDED_SVX_FMGRIDIF_HXX

#include <svx/svxdllapi.h>

#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/sdbc/XRowSetListener.hpp>
#include <com/sun/star/sdb/XRowSetSupplier.hpp>
#include <com/sun/star/form/XReset.hpp>
#include <com/sun/star/form/XBoundComponent.hpp>
#include <com/sun/star/form/XLoadListener.hpp>
#include <com/sun/star/form/XGridControl.hpp>
#include <com/sun/star/form/XGridPeer.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#include <com/sun/star/view/XSelectionChangeListener.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XModeSelector.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>

#include <tools/wintypes.hxx>
#include <toolkit/controls/unocontrol.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <comphelper/uno3.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/implbase10.hxx>

class DbGridColumn;

class SAL_WARN_UNUSED OWeakSubObject : public ::cppu::OWeakObject
{
protected:
    ::cppu::OWeakObject&    m_rParent;

public:
    OWeakSubObject(::cppu::OWeakObject& rParent) : m_rParent(rParent) { }

    virtual void SAL_CALL acquire() throw() override { m_rParent.acquire(); }
    virtual void SAL_CALL release() throw() override { m_rParent.release(); }
};


// FmXModifyMultiplexer

class SAL_WARN_UNUSED FmXModifyMultiplexer  :public OWeakSubObject
                            ,public ::cppu::OInterfaceContainerHelper
                            ,public ::com::sun::star::util::XModifyListener
{
public:
    FmXModifyMultiplexer( ::cppu::OWeakObject& rSource, ::osl::Mutex& rMutex );
    DECLARE_UNO3_DEFAULTS(FmXModifyMultiplexer,OWeakSubObject)
    virtual ::com::sun::star::uno::Any  SAL_CALL queryInterface(const ::com::sun::star::uno::Type& _rType) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

// ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

// ::com::sun::star::util::XModifyListener
    virtual void SAL_CALL modified(const ::com::sun::star::lang::EventObject& Source) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

// resolve ambiguity : both OWeakObject and OInterfaceContainerHelper have these memory operators
    void * SAL_CALL operator new( size_t size ) throw() { return OWeakSubObject::operator new(size); }
    void SAL_CALL operator delete( void * p ) throw() { OWeakSubObject::operator delete(p); }
};


// FmXUpdateMultiplexer

class SAL_WARN_UNUSED FmXUpdateMultiplexer : public OWeakSubObject,
                             public ::cppu::OInterfaceContainerHelper,
                             public ::com::sun::star::form::XUpdateListener
{
public:
    FmXUpdateMultiplexer( ::cppu::OWeakObject& rSource, ::osl::Mutex& rMutex );
    DECLARE_UNO3_DEFAULTS(FmXUpdateMultiplexer,OWeakSubObject)

    virtual ::com::sun::star::uno::Any  SAL_CALL queryInterface(const ::com::sun::star::uno::Type& _rType) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

// ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

// ::com::sun::star::form::XUpdateListener
    virtual sal_Bool SAL_CALL approveUpdate(const ::com::sun::star::lang::EventObject &) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL updated(const ::com::sun::star::lang::EventObject &) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

// resolve ambiguity : both OWeakObject and OInterfaceContainerHelper have these memory operators
    void * SAL_CALL operator new( size_t size ) throw() { return OWeakSubObject::operator new(size); }
    void SAL_CALL operator delete( void * p ) throw() { OWeakSubObject::operator delete(p); }
};


// FmXSelectionMultiplexer

class SAL_WARN_UNUSED FmXSelectionMultiplexer   :public OWeakSubObject
                                ,public ::cppu::OInterfaceContainerHelper
                                ,public ::com::sun::star::view::XSelectionChangeListener
{
public:
    FmXSelectionMultiplexer( ::cppu::OWeakObject& rSource, ::osl::Mutex& rMutex );
    DECLARE_UNO3_DEFAULTS(FmXSelectionMultiplexer, OWeakSubObject)

    virtual ::com::sun::star::uno::Any  SAL_CALL queryInterface(const ::com::sun::star::uno::Type& _rType) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

// ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

// ::com::sun::star::view::XSelectionChangeListener
    virtual void SAL_CALL selectionChanged( const ::com::sun::star::lang::EventObject& aEvent ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

// resolve ambiguity : both OWeakObject and OInterfaceContainerHelper have these memory operators
    void * SAL_CALL operator new( size_t size ) throw() { return OWeakSubObject::operator new(size); }
    void SAL_CALL operator delete( void * p ) throw() { OWeakSubObject::operator delete(p); }
};


// FmXGridControlMultiplexer

class SAL_WARN_UNUSED FmXGridControlMultiplexer :public OWeakSubObject
                                ,public ::cppu::OInterfaceContainerHelper
                                ,public ::com::sun::star::form::XGridControlListener
{
public:
    FmXGridControlMultiplexer( ::cppu::OWeakObject& rSource, ::osl::Mutex& rMutex );
    DECLARE_UNO3_DEFAULTS( FmXGridControlMultiplexer, OWeakSubObject )

    virtual ::com::sun::star::uno::Any  SAL_CALL queryInterface(const ::com::sun::star::uno::Type& _rType) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

// ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

// ::com::sun::star::view::XSelectionChangeListener
    virtual void SAL_CALL columnChanged( const ::com::sun::star::lang::EventObject& _event ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

// resolve ambiguity : both OWeakObject and OInterfaceContainerHelper have these memory operators
    void * SAL_CALL operator new( size_t size ) throw() { return OWeakSubObject::operator new(size); }
    void SAL_CALL operator delete( void * p ) throw() { OWeakSubObject::operator delete(p); }
};


// FmXContainerMultiplexer

class SAL_WARN_UNUSED FmXContainerMultiplexer : public OWeakSubObject,
                                public ::cppu::OInterfaceContainerHelper,
                                public ::com::sun::star::container::XContainerListener
{
public:
    FmXContainerMultiplexer( ::cppu::OWeakObject& rSource, ::osl::Mutex& rMutex);
    DECLARE_UNO3_DEFAULTS(FmXContainerMultiplexer,OWeakSubObject)
    virtual ::com::sun::star::uno::Any  SAL_CALL queryInterface(const ::com::sun::star::uno::Type& _rType) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

// ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

// ::com::sun::star::container::XContainerListener
    virtual void SAL_CALL elementInserted(const ::com::sun::star::container::ContainerEvent& Event) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL elementRemoved(const ::com::sun::star::container::ContainerEvent& Event) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL elementReplaced(const ::com::sun::star::container::ContainerEvent& Event) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

// resolve ambiguity : both OWeakObject and OInterfaceContainerHelper have these memory operators
    void * SAL_CALL operator new( size_t size ) throw() { return OWeakSubObject::operator new(size); }
    void SAL_CALL operator delete( void * p ) throw() { OWeakSubObject::operator delete(p); }
};


// FmXGridControl

typedef ::cppu::ImplHelper10<   ::com::sun::star::form::XBoundComponent,
                                ::com::sun::star::form::XGridControl,
                                ::com::sun::star::util::XModifyBroadcaster,
                                ::com::sun::star::container::XIndexAccess,
                                ::com::sun::star::container::XEnumerationAccess,
                                ::com::sun::star::util::XModeSelector,
                                ::com::sun::star::container::XContainer,
                                ::com::sun::star::frame::XDispatchProvider,
                                ::com::sun::star::frame::XDispatchProviderInterception,
                                ::com::sun::star::view::XSelectionSupplier
                            >   FmXGridControl_BASE;

class FmXGridPeer;
class SVX_DLLPUBLIC SAL_WARN_UNUSED FmXGridControl  :public UnoControl
                        ,public FmXGridControl_BASE
{
    FmXModifyMultiplexer        m_aModifyListeners;
    FmXUpdateMultiplexer        m_aUpdateListeners;
    FmXContainerMultiplexer     m_aContainerListeners;
    FmXSelectionMultiplexer     m_aSelectionListeners;
    FmXGridControlMultiplexer   m_aGridControlListeners;

protected:
    sal_uInt16  m_nPeerCreationLevel;
    bool        m_bInDraw;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >    m_xContext;

public:
    FmXGridControl(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >&);
    virtual ~FmXGridControl();

    // UNO connection
    DECLARE_UNO3_AGG_DEFAULTS(FmXGridControl, UnoControl)
    virtual ::com::sun::star::uno::Any  SAL_CALL queryAggregation(const ::com::sun::star::uno::Type& _rType) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

// XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence<sal_Int8> SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

// ::com::sun::star::lang::XComponent
    virtual void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

// ::com::sun::star::lang::XServiceInfo
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw(std::exception) override;
    virtual OUString SAL_CALL getImplementationName() throw(std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(std::exception) override;

// ::com::sun::star::awt::XControl
    virtual void SAL_CALL createPeer(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit >& _rToolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >& Parent) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL setModel(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& Model) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDesignMode(sal_Bool bOn) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

// ::com::sun::star::awt::XView
    virtual void SAL_CALL draw( sal_Int32 x, sal_Int32 y ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

// ::com::sun::star::form::XBoundComponent
    virtual void SAL_CALL addUpdateListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XUpdateListener >& l) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeUpdateListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XUpdateListener >& l) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL commit() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

// ::com::sun::star::container::XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

// ::com::sun::star::container::XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration >  SAL_CALL createEnumeration() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

// ::com::sun::star::container::XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex(sal_Int32 _rIndex) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;

// ::com::sun::star::form::XGridControl
    virtual void SAL_CALL addGridControlListener( const ::com::sun::star::uno::Reference< ::com::sun::star::form::XGridControlListener >& _listener ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeGridControlListener( const ::com::sun::star::uno::Reference< ::com::sun::star::form::XGridControlListener >& _listener ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

// ::com::sun::star::form::XGrid (base of XGridControl)
    virtual sal_Int16 SAL_CALL getCurrentColumnPosition() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setCurrentColumnPosition(sal_Int16 nPos) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

// ::com::sun::star::form::XGridFieldDataSupplier (base of XGridControl)
    virtual ::com::sun::star::uno::Sequence< sal_Bool > SAL_CALL queryFieldDataType( const ::com::sun::star::uno::Type& xType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL queryFieldData( sal_Int32 nRow, const ::com::sun::star::uno::Type& xType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

// UnoControl
    virtual OUString GetComponentServiceName() override;

// ::com::sun::star::util::XModifyBroadcaster
    virtual void SAL_CALL addModifyListener(const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& l) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeModifyListener(const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& l) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

// ::com::sun::star::util::XModeSelector
    virtual void SAL_CALL setMode(const OUString& Mode) throw(::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getMode() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString> SAL_CALL getSupportedModes() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsMode(const OUString& Mode) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

// ::com::sun::star::container::XContainer
    virtual void SAL_CALL addContainerListener(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& l) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeContainerListener(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& l) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

// ::com::sun::star::frame::XDispatchProvider
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >  SAL_CALL queryDispatch(const ::com::sun::star::util::URL& aURL, const OUString& aTargetFrameName, sal_Int32 nSearchFlags) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >  > SAL_CALL queryDispatches(const ::com::sun::star::uno::Sequence< ::com::sun::star::frame::DispatchDescriptor >& aDescripts) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

// ::com::sun::star::frame::XDispatchProviderInterception
    virtual void SAL_CALL registerDispatchProviderInterceptor(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor >& xInterceptor) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL releaseDispatchProviderInterceptor(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor >& xInterceptor) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

// ::com::sun::star::view::XSelectionSupplier
    virtual sal_Bool SAL_CALL select( const ::com::sun::star::uno::Any& aSelection ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getSelection(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addSelectionChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeSelectionChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

protected:
    virtual FmXGridPeer*    imp_CreatePeer(vcl::Window* pParent);
        // ImplCreatePeer would be better, but doesn't work because it's not exported

};


// FmXGridPeer -> Peer for the Gridcontrol

class FmGridControl;
class SVX_DLLPUBLIC SAL_WARN_UNUSED FmXGridPeer:
    public cppu::ImplInheritanceHelper<
        VCLXWindow,
        css::form::XGridPeer,
        css::form::XBoundComponent,
        css::form::XGridControl,
        css::sdb::XRowSetSupplier,
        css::util::XModifyBroadcaster,
        css::beans::XPropertyChangeListener,
        css::container::XContainerListener,
        css::sdbc::XRowSetListener,
        css::form::XLoadListener,
        css::view::XSelectionChangeListener,
        css::container::XIndexAccess,
        css::container::XEnumerationAccess,
        css::util::XModeSelector,
        css::container::XContainer,
        css::frame::XStatusListener,
        css::frame::XDispatchProvider,
        css::frame::XDispatchProviderInterception,
        css::form::XResetListener,
        css::view::XSelectionSupplier>
{
    css::uno::Reference< css::container::XIndexContainer >    m_xColumns;
    css::uno::Reference< css::sdbc::XRowSet >                 m_xCursor;
    ::cppu::OInterfaceContainerHelper       m_aModifyListeners,
                                            m_aUpdateListeners,
                                            m_aContainerListeners,
                                            m_aSelectionListeners,
                                            m_aGridControlListeners;

    OUString                m_aMode;
    sal_Int32               m_nCursorListening;

    css::uno::Reference< css::frame::XDispatchProviderInterceptor >   m_xFirstDispatchInterceptor;

    bool                                m_bInterceptingDispatch;

    bool*                               m_pStateCache;
        // one bool for each supported url
    css::uno::Reference< css::frame::XDispatch > *                    m_pDispatchers;
        // one dispatcher for each supported url
        // (I would like to have a vector here but including the stl in an exported file seems
        // very risky to me ....)

    class GridListenerDelegator;
    friend class GridListenerDelegator;
    GridListenerDelegator*  m_pGridListener;

protected:
    css::uno::Reference< css::uno::XComponentContext >    m_xContext;
    ::osl::Mutex                                              m_aMutex;

public:
    FmXGridPeer(const css::uno::Reference< css::uno::XComponentContext >&);
    virtual ~FmXGridPeer();

    // spaeter Constructor, immer nach dem realen Constructor zu rufen !
    void Create(vcl::Window* pParent, WinBits nStyle);

// ::com::sun::star::lang::XUnoTunnel
    static const ::com::sun::star::uno::Sequence< sal_Int8 >&   getUnoTunnelImplementationId() throw();
    static FmXGridPeer*                                         getImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxIFace ) throw();
    sal_Int64                                                   SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& _rIdentifier ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

// ::com::sun::star::form::XGridPeer
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer > SAL_CALL getColumns(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setColumns( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >& aColumns ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

// ::com::sun::star::lang::XComponent
    virtual void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

// ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& Source) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

// ::com::sun::star::form::XBoundComponent
    virtual void SAL_CALL addUpdateListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XUpdateListener >& l) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeUpdateListener(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XUpdateListener >& l) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL commit() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

// ::com::sun::star::container::XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

// ::com::sun::star::container::XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL createEnumeration() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

// ::com::sun::star::container::XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex(sal_Int32 _rIndex) throw(::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;

// ::com::sun::star::beans::XPropertyChangeListener
    virtual void SAL_CALL SAL_CALL propertyChange(const ::com::sun::star::beans::PropertyChangeEvent& evt) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

// ::com::sun::star::form::XLoadListener
    virtual void SAL_CALL loaded(const ::com::sun::star::lang::EventObject& rEvent) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL unloaded(const ::com::sun::star::lang::EventObject& rEvent) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL unloading(const ::com::sun::star::lang::EventObject& aEvent) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL reloading(const ::com::sun::star::lang::EventObject& aEvent) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL reloaded(const ::com::sun::star::lang::EventObject& aEvent) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

// ::com::sun::star::sdbc::XRowSetListener
    virtual void SAL_CALL cursorMoved(const ::com::sun::star::lang::EventObject& event) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL rowChanged(const ::com::sun::star::lang::EventObject& event) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL rowSetChanged(const ::com::sun::star::lang::EventObject& event) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

// ::com::sun::star::container::XContainerListener
    virtual void SAL_CALL elementInserted(const ::com::sun::star::container::ContainerEvent& Event) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL elementRemoved(const ::com::sun::star::container::ContainerEvent& Event) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL elementReplaced(const ::com::sun::star::container::ContainerEvent& Event) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

// VCLXWindow
    virtual void SAL_CALL setProperty( const OUString& PropertyName, const ::com::sun::star::uno::Any& Value ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessibleContext >
                    CreateAccessibleContext() override;

// ::com::sun::star::form::XGridControl
    virtual void SAL_CALL addGridControlListener( const ::com::sun::star::uno::Reference< ::com::sun::star::form::XGridControlListener >& _listener ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeGridControlListener( const ::com::sun::star::uno::Reference< ::com::sun::star::form::XGridControlListener >& _listener ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

// ::com::sun::star::form::XGrid (base of XGridControl)
    virtual sal_Int16 SAL_CALL getCurrentColumnPosition() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setCurrentColumnPosition(sal_Int16 nPos) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

// ::com::sun::star::form::XGridFieldDataSupplier (base of XGridControl)
    virtual ::com::sun::star::uno::Sequence< sal_Bool > SAL_CALL queryFieldDataType( const ::com::sun::star::uno::Type& xType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL queryFieldData( sal_Int32 nRow, const ::com::sun::star::uno::Type& xType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

// ::com::sun::star::sdb::XRowSetSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >  SAL_CALL getRowSet() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setRowSet(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& xDataSource) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

// ::com::sun::star::util::XModifyBroadcaster
    virtual void SAL_CALL addModifyListener(const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& l) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeModifyListener(const ::com::sun::star::uno::Reference< ::com::sun::star::util::XModifyListener >& l) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

// UnoControl
    virtual void SAL_CALL SAL_CALL setDesignMode(sal_Bool bOn) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isDesignMode() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

// ::com::sun::star::view::XSelectionChangeListener
    virtual void SAL_CALL selectionChanged(const ::com::sun::star::lang::EventObject& aEvent) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    void CellModified();

// PropertyListening
    void updateGrid(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet >& _rDatabaseCursor);
    void startCursorListening();
    void stopCursorListening();

// ::com::sun::star::util::XModeSelector
    virtual void SAL_CALL setMode(const OUString& Mode) throw(::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getMode() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedModes() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsMode(const OUString& Mode) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

// ::com::sun::star::container::XContainer
    virtual void SAL_CALL addContainerListener(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& l) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeContainerListener(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XContainerListener >& l) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    void columnVisible(DbGridColumn* pColumn);
    void columnHidden(DbGridColumn* pColumn);

// ::com::sun::star::awt::XView
    virtual void SAL_CALL draw( sal_Int32 x, sal_Int32 y ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

// ::com::sun::star::frame::XDispatchProvider
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >  SAL_CALL queryDispatch(const ::com::sun::star::util::URL& aURL, const OUString& aTargetFrameName, sal_Int32 nSearchFlags) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >  > SAL_CALL queryDispatches(const ::com::sun::star::uno::Sequence< ::com::sun::star::frame::DispatchDescriptor >& aDescripts) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

// ::com::sun::star::frame::XDispatchProviderInterception
    virtual void SAL_CALL registerDispatchProviderInterceptor(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor >& xInterceptor) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL releaseDispatchProviderInterceptor(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor >& xInterceptor) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

// ::com::sun::star::frame::XStatusListener
    virtual void SAL_CALL statusChanged(const ::com::sun::star::frame::FeatureStateEvent& Event) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

// ::com::sun::star::form::XResetListener
    virtual sal_Bool SAL_CALL approveReset(const ::com::sun::star::lang::EventObject& rEvent) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL resetted(const ::com::sun::star::lang::EventObject& rEvent) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

// ::com::sun::star::view::XSelectionSupplier
    virtual sal_Bool SAL_CALL select( const ::com::sun::star::uno::Any& aSelection ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getSelection(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addSelectionChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeSelectionChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

protected:
    virtual VclPtr<FmGridControl>  imp_CreateControl(vcl::Window* pParent, WinBits nStyle);

    static ::com::sun::star::uno::Sequence< ::com::sun::star::util::URL>&       getSupportedURLs();
    static ::com::sun::star::uno::Sequence<sal_uInt16>& getSupportedGridSlots();
    void    ConnectToDispatcher();
    void    DisConnectFromDispatcher();
    void    UpdateDispatches(); // will connect if not already connected and just update else

    /** If a derived class wants to listen at some column properties, it doesn't have
        to override all methods affecting columns (setColumns, elementInserted, elementRemoved ...)
        Instead it may use addColumnListeners and removeColumnListeners which are called in all
        the cases.
    */
    void addColumnListeners(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xCol);
    void removeColumnListeners(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& xCol);

    void selectionChanged();
    void columnChanged();

    DECL_LINK_TYPED(OnQueryGridSlotState, sal_uInt16, int);
    DECL_LINK_TYPED(OnExecuteGridSlot, sal_uInt16, bool);
};



#endif // _SVX_FMGRID_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
