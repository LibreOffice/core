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
                            ,public css::util::XModifyListener
{
public:
    FmXModifyMultiplexer( ::cppu::OWeakObject& rSource, ::osl::Mutex& rMutex );
    DECLARE_UNO3_DEFAULTS(FmXModifyMultiplexer,OWeakSubObject)
    virtual css::uno::Any  SAL_CALL queryInterface(const css::uno::Type& _rType) throw (css::uno::RuntimeException, std::exception) override;

// css::lang::XEventListener
    virtual void SAL_CALL disposing(const css::lang::EventObject& Source) throw(css::uno::RuntimeException, std::exception) override;

// css::util::XModifyListener
    virtual void SAL_CALL modified(const css::lang::EventObject& Source) throw(css::uno::RuntimeException, std::exception) override;

// resolve ambiguity : both OWeakObject and OInterfaceContainerHelper have these memory operators
    void * SAL_CALL operator new( size_t size ) throw() { return OWeakSubObject::operator new(size); }
    void SAL_CALL operator delete( void * p ) throw() { OWeakSubObject::operator delete(p); }
};


// FmXUpdateMultiplexer

class SAL_WARN_UNUSED FmXUpdateMultiplexer : public OWeakSubObject,
                             public ::cppu::OInterfaceContainerHelper,
                             public css::form::XUpdateListener
{
public:
    FmXUpdateMultiplexer( ::cppu::OWeakObject& rSource, ::osl::Mutex& rMutex );
    DECLARE_UNO3_DEFAULTS(FmXUpdateMultiplexer,OWeakSubObject)

    virtual css::uno::Any  SAL_CALL queryInterface(const css::uno::Type& _rType) throw (css::uno::RuntimeException, std::exception) override;

// css::lang::XEventListener
    virtual void SAL_CALL disposing(const css::lang::EventObject& Source) throw(css::uno::RuntimeException, std::exception) override;

// css::form::XUpdateListener
    virtual sal_Bool SAL_CALL approveUpdate(const css::lang::EventObject &) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL updated(const css::lang::EventObject &) throw(css::uno::RuntimeException, std::exception) override;

// resolve ambiguity : both OWeakObject and OInterfaceContainerHelper have these memory operators
    void * SAL_CALL operator new( size_t size ) throw() { return OWeakSubObject::operator new(size); }
    void SAL_CALL operator delete( void * p ) throw() { OWeakSubObject::operator delete(p); }
};


// FmXSelectionMultiplexer

class SAL_WARN_UNUSED FmXSelectionMultiplexer   :public OWeakSubObject
                                ,public ::cppu::OInterfaceContainerHelper
                                ,public css::view::XSelectionChangeListener
{
public:
    FmXSelectionMultiplexer( ::cppu::OWeakObject& rSource, ::osl::Mutex& rMutex );
    DECLARE_UNO3_DEFAULTS(FmXSelectionMultiplexer, OWeakSubObject)

    virtual css::uno::Any  SAL_CALL queryInterface(const css::uno::Type& _rType) throw (css::uno::RuntimeException, std::exception) override;

// css::lang::XEventListener
    virtual void SAL_CALL disposing(const css::lang::EventObject& Source) throw(css::uno::RuntimeException, std::exception) override;

// css::view::XSelectionChangeListener
    virtual void SAL_CALL selectionChanged( const css::lang::EventObject& aEvent ) throw (css::uno::RuntimeException, std::exception) override;

// resolve ambiguity : both OWeakObject and OInterfaceContainerHelper have these memory operators
    void * SAL_CALL operator new( size_t size ) throw() { return OWeakSubObject::operator new(size); }
    void SAL_CALL operator delete( void * p ) throw() { OWeakSubObject::operator delete(p); }
};


// FmXGridControlMultiplexer

class SAL_WARN_UNUSED FmXGridControlMultiplexer :public OWeakSubObject
                                ,public ::cppu::OInterfaceContainerHelper
                                ,public css::form::XGridControlListener
{
public:
    FmXGridControlMultiplexer( ::cppu::OWeakObject& rSource, ::osl::Mutex& rMutex );
    DECLARE_UNO3_DEFAULTS( FmXGridControlMultiplexer, OWeakSubObject )

    virtual css::uno::Any  SAL_CALL queryInterface(const css::uno::Type& _rType) throw (css::uno::RuntimeException, std::exception) override;

// css::lang::XEventListener
    virtual void SAL_CALL disposing(const css::lang::EventObject& Source) throw(css::uno::RuntimeException, std::exception) override;

// css::view::XSelectionChangeListener
    virtual void SAL_CALL columnChanged( const css::lang::EventObject& _event ) throw (css::uno::RuntimeException, std::exception) override;

// resolve ambiguity : both OWeakObject and OInterfaceContainerHelper have these memory operators
    void * SAL_CALL operator new( size_t size ) throw() { return OWeakSubObject::operator new(size); }
    void SAL_CALL operator delete( void * p ) throw() { OWeakSubObject::operator delete(p); }
};


// FmXContainerMultiplexer

class SAL_WARN_UNUSED FmXContainerMultiplexer : public OWeakSubObject,
                                public ::cppu::OInterfaceContainerHelper,
                                public css::container::XContainerListener
{
public:
    FmXContainerMultiplexer( ::cppu::OWeakObject& rSource, ::osl::Mutex& rMutex);
    DECLARE_UNO3_DEFAULTS(FmXContainerMultiplexer,OWeakSubObject)
    virtual css::uno::Any  SAL_CALL queryInterface(const css::uno::Type& _rType) throw (css::uno::RuntimeException, std::exception) override;

// css::lang::XEventListener
    virtual void SAL_CALL disposing(const css::lang::EventObject& Source) throw(css::uno::RuntimeException, std::exception) override;

// css::container::XContainerListener
    virtual void SAL_CALL elementInserted(const css::container::ContainerEvent& Event) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL elementRemoved(const css::container::ContainerEvent& Event) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL elementReplaced(const css::container::ContainerEvent& Event) throw(css::uno::RuntimeException, std::exception) override;

// resolve ambiguity : both OWeakObject and OInterfaceContainerHelper have these memory operators
    void * SAL_CALL operator new( size_t size ) throw() { return OWeakSubObject::operator new(size); }
    void SAL_CALL operator delete( void * p ) throw() { OWeakSubObject::operator delete(p); }
};


// FmXGridControl

typedef ::cppu::ImplHelper10<   css::form::XBoundComponent,
                                css::form::XGridControl,
                                css::util::XModifyBroadcaster,
                                css::container::XIndexAccess,
                                css::container::XEnumerationAccess,
                                css::util::XModeSelector,
                                css::container::XContainer,
                                css::frame::XDispatchProvider,
                                css::frame::XDispatchProviderInterception,
                                css::view::XSelectionSupplier
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
    bool        m_bInDraw;

    css::uno::Reference< css::uno::XComponentContext >    m_xContext;

public:
    FmXGridControl(const css::uno::Reference< css::uno::XComponentContext >&);
    virtual ~FmXGridControl();

    // UNO connection
    DECLARE_UNO3_AGG_DEFAULTS(FmXGridControl, UnoControl)
    virtual css::uno::Any  SAL_CALL queryAggregation(const css::uno::Type& _rType) throw (css::uno::RuntimeException, std::exception) override;

// XTypeProvider
    virtual css::uno::Sequence< css::uno::Type> SAL_CALL getTypes(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence<sal_Int8> SAL_CALL getImplementationId(  ) throw(css::uno::RuntimeException, std::exception) override;

// css::lang::XComponent
    virtual void SAL_CALL dispose() throw(css::uno::RuntimeException, std::exception) override;

// css::lang::XServiceInfo
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) throw(std::exception) override;
    virtual OUString SAL_CALL getImplementationName() throw(std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(std::exception) override;

// css::awt::XControl
    virtual void SAL_CALL createPeer(const css::uno::Reference< css::awt::XToolkit >& _rToolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent) throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL setModel(const css::uno::Reference< css::awt::XControlModel >& Model) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setDesignMode(sal_Bool bOn) throw(css::uno::RuntimeException, std::exception) override;

// css::awt::XView
    virtual void SAL_CALL draw( sal_Int32 x, sal_Int32 y ) throw(css::uno::RuntimeException, std::exception) override;

// css::form::XBoundComponent
    virtual void SAL_CALL addUpdateListener(const css::uno::Reference< css::form::XUpdateListener >& l) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeUpdateListener(const css::uno::Reference< css::form::XUpdateListener >& l) throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL commit() throw(css::uno::RuntimeException, std::exception) override;

// css::container::XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements(  ) throw(css::uno::RuntimeException, std::exception) override;

// css::container::XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration >  SAL_CALL createEnumeration() throw(css::uno::RuntimeException, std::exception) override;

// css::container::XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getByIndex(sal_Int32 _rIndex) throw(css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

// css::form::XGridControl
    virtual void SAL_CALL addGridControlListener( const css::uno::Reference< css::form::XGridControlListener >& _listener ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeGridControlListener( const css::uno::Reference< css::form::XGridControlListener >& _listener ) throw(css::uno::RuntimeException, std::exception) override;

// css::form::XGrid (base of XGridControl)
    virtual sal_Int16 SAL_CALL getCurrentColumnPosition() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setCurrentColumnPosition(sal_Int16 nPos) throw(css::uno::RuntimeException, std::exception) override;

// css::form::XGridFieldDataSupplier (base of XGridControl)
    virtual css::uno::Sequence< sal_Bool > SAL_CALL queryFieldDataType( const css::uno::Type& xType ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::uno::Any > SAL_CALL queryFieldData( sal_Int32 nRow, const css::uno::Type& xType ) throw(css::uno::RuntimeException, std::exception) override;

// UnoControl
    virtual OUString GetComponentServiceName() override;

// css::util::XModifyBroadcaster
    virtual void SAL_CALL addModifyListener(const css::uno::Reference< css::util::XModifyListener >& l) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeModifyListener(const css::uno::Reference< css::util::XModifyListener >& l) throw(css::uno::RuntimeException, std::exception) override;

// css::util::XModeSelector
    virtual void SAL_CALL setMode(const OUString& Mode) throw(css::lang::NoSupportException, css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getMode() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString> SAL_CALL getSupportedModes() throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsMode(const OUString& Mode) throw(css::uno::RuntimeException, std::exception) override;

// css::container::XContainer
    virtual void SAL_CALL addContainerListener(const css::uno::Reference< css::container::XContainerListener >& l) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeContainerListener(const css::uno::Reference< css::container::XContainerListener >& l) throw(css::uno::RuntimeException, std::exception) override;

// css::frame::XDispatchProvider
    virtual css::uno::Reference< css::frame::XDispatch >  SAL_CALL queryDispatch(const css::util::URL& aURL, const OUString& aTargetFrameName, sal_Int32 nSearchFlags) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::uno::Reference< css::frame::XDispatch >  > SAL_CALL queryDispatches(const css::uno::Sequence< css::frame::DispatchDescriptor >& aDescripts) throw(css::uno::RuntimeException, std::exception) override;

// css::frame::XDispatchProviderInterception
    virtual void SAL_CALL registerDispatchProviderInterceptor(const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& xInterceptor) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL releaseDispatchProviderInterceptor(const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& xInterceptor) throw(css::uno::RuntimeException, std::exception) override;

// css::view::XSelectionSupplier
    virtual sal_Bool SAL_CALL select( const css::uno::Any& aSelection ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getSelection(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addSelectionChangeListener( const css::uno::Reference< css::view::XSelectionChangeListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeSelectionChangeListener( const css::uno::Reference< css::view::XSelectionChangeListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;

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

// css::lang::XUnoTunnel
    static const css::uno::Sequence< sal_Int8 >&   getUnoTunnelImplementationId() throw();
    static FmXGridPeer*                                         getImplementation( const css::uno::Reference< css::uno::XInterface >& _rxIFace ) throw();
    sal_Int64                                                   SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& _rIdentifier ) throw(css::uno::RuntimeException, std::exception) override;

// css::form::XGridPeer
    virtual css::uno::Reference< css::container::XIndexContainer > SAL_CALL getColumns(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setColumns( const css::uno::Reference< css::container::XIndexContainer >& aColumns ) throw(css::uno::RuntimeException, std::exception) override;

// css::lang::XComponent
    virtual void SAL_CALL dispose() throw(css::uno::RuntimeException, std::exception) override;

// css::lang::XEventListener
    virtual void SAL_CALL disposing(const css::lang::EventObject& Source) throw(css::uno::RuntimeException, std::exception) override;

// css::form::XBoundComponent
    virtual void SAL_CALL addUpdateListener(const css::uno::Reference< css::form::XUpdateListener >& l) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeUpdateListener(const css::uno::Reference< css::form::XUpdateListener >& l) throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL commit() throw(css::uno::RuntimeException, std::exception) override;

// css::container::XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements(  ) throw(css::uno::RuntimeException, std::exception) override;

// css::container::XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() throw(css::uno::RuntimeException, std::exception) override;

// css::container::XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getByIndex(sal_Int32 _rIndex) throw(css::lang::IndexOutOfBoundsException, css::lang::WrappedTargetException, css::uno::RuntimeException, std::exception) override;

// css::beans::XPropertyChangeListener
    virtual void SAL_CALL SAL_CALL propertyChange(const css::beans::PropertyChangeEvent& evt) throw(css::uno::RuntimeException, std::exception) override;

// css::form::XLoadListener
    virtual void SAL_CALL loaded(const css::lang::EventObject& rEvent) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL unloaded(const css::lang::EventObject& rEvent) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL unloading(const css::lang::EventObject& aEvent) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL reloading(const css::lang::EventObject& aEvent) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL reloaded(const css::lang::EventObject& aEvent) throw(css::uno::RuntimeException, std::exception) override;

// css::sdbc::XRowSetListener
    virtual void SAL_CALL cursorMoved(const css::lang::EventObject& event) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL rowChanged(const css::lang::EventObject& event) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL rowSetChanged(const css::lang::EventObject& event) throw(css::uno::RuntimeException, std::exception) override;

// css::container::XContainerListener
    virtual void SAL_CALL elementInserted(const css::container::ContainerEvent& Event) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL elementRemoved(const css::container::ContainerEvent& Event) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL elementReplaced(const css::container::ContainerEvent& Event) throw(css::uno::RuntimeException, std::exception) override;

// VCLXWindow
    virtual void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleContext >
                    CreateAccessibleContext() override;

// css::form::XGridControl
    virtual void SAL_CALL addGridControlListener( const css::uno::Reference< css::form::XGridControlListener >& _listener ) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeGridControlListener( const css::uno::Reference< css::form::XGridControlListener >& _listener ) throw(css::uno::RuntimeException, std::exception) override;

// css::form::XGrid (base of XGridControl)
    virtual sal_Int16 SAL_CALL getCurrentColumnPosition() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setCurrentColumnPosition(sal_Int16 nPos) throw(css::uno::RuntimeException, std::exception) override;

// css::form::XGridFieldDataSupplier (base of XGridControl)
    virtual css::uno::Sequence< sal_Bool > SAL_CALL queryFieldDataType( const css::uno::Type& xType ) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::uno::Any > SAL_CALL queryFieldData( sal_Int32 nRow, const css::uno::Type& xType ) throw(css::uno::RuntimeException, std::exception) override;

// css::sdb::XRowSetSupplier
    virtual css::uno::Reference< css::sdbc::XRowSet >  SAL_CALL getRowSet() throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setRowSet(const css::uno::Reference< css::sdbc::XRowSet >& xDataSource) throw(css::uno::RuntimeException, std::exception) override;

// css::util::XModifyBroadcaster
    virtual void SAL_CALL addModifyListener(const css::uno::Reference< css::util::XModifyListener >& l) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeModifyListener(const css::uno::Reference< css::util::XModifyListener >& l) throw(css::uno::RuntimeException, std::exception) override;

// UnoControl
    virtual void SAL_CALL SAL_CALL setDesignMode(sal_Bool bOn) throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isDesignMode() throw(css::uno::RuntimeException, std::exception) override;

// css::view::XSelectionChangeListener
    virtual void SAL_CALL selectionChanged(const css::lang::EventObject& aEvent) throw(css::uno::RuntimeException, std::exception) override;

    void CellModified();

// PropertyListening
    void updateGrid(const css::uno::Reference< css::sdbc::XRowSet >& _rDatabaseCursor);
    void startCursorListening();
    void stopCursorListening();

// css::util::XModeSelector
    virtual void SAL_CALL setMode(const OUString& Mode) throw(css::lang::NoSupportException, css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getMode() throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedModes() throw(css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsMode(const OUString& Mode) throw(css::uno::RuntimeException, std::exception) override;

// css::container::XContainer
    virtual void SAL_CALL addContainerListener(const css::uno::Reference< css::container::XContainerListener >& l) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeContainerListener(const css::uno::Reference< css::container::XContainerListener >& l) throw(css::uno::RuntimeException, std::exception) override;

    void columnVisible(DbGridColumn* pColumn);
    void columnHidden(DbGridColumn* pColumn);

// css::awt::XView
    virtual void SAL_CALL draw( sal_Int32 x, sal_Int32 y ) throw(css::uno::RuntimeException, std::exception) override;

// css::frame::XDispatchProvider
    virtual css::uno::Reference< css::frame::XDispatch >  SAL_CALL queryDispatch(const css::util::URL& aURL, const OUString& aTargetFrameName, sal_Int32 nSearchFlags) throw(css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< css::uno::Reference< css::frame::XDispatch >  > SAL_CALL queryDispatches(const css::uno::Sequence< css::frame::DispatchDescriptor >& aDescripts) throw(css::uno::RuntimeException, std::exception) override;

// css::frame::XDispatchProviderInterception
    virtual void SAL_CALL registerDispatchProviderInterceptor(const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& xInterceptor) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL releaseDispatchProviderInterceptor(const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& xInterceptor) throw(css::uno::RuntimeException, std::exception) override;

// css::frame::XStatusListener
    virtual void SAL_CALL statusChanged(const css::frame::FeatureStateEvent& Event) throw(css::uno::RuntimeException, std::exception) override;

// css::form::XResetListener
    virtual sal_Bool SAL_CALL approveReset(const css::lang::EventObject& rEvent) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL resetted(const css::lang::EventObject& rEvent) throw(css::uno::RuntimeException, std::exception) override;

// css::view::XSelectionSupplier
    virtual sal_Bool SAL_CALL select( const css::uno::Any& aSelection ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getSelection(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addSelectionChangeListener( const css::uno::Reference< css::view::XSelectionChangeListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeSelectionChangeListener( const css::uno::Reference< css::view::XSelectionChangeListener >& xListener ) throw (css::uno::RuntimeException, std::exception) override;

protected:
    virtual VclPtr<FmGridControl>  imp_CreateControl(vcl::Window* pParent, WinBits nStyle);

    static css::uno::Sequence< css::util::URL>&       getSupportedURLs();
    static css::uno::Sequence<sal_uInt16>& getSupportedGridSlots();
    void    ConnectToDispatcher();
    void    DisConnectFromDispatcher();
    void    UpdateDispatches(); // will connect if not already connected and just update else

    /** If a derived class wants to listen at some column properties, it doesn't have
        to override all methods affecting columns (setColumns, elementInserted, elementRemoved ...)
        Instead it may use addColumnListeners and removeColumnListeners which are called in all
        the cases.
    */
    void addColumnListeners(const css::uno::Reference< css::beans::XPropertySet >& xCol);
    void removeColumnListeners(const css::uno::Reference< css::beans::XPropertySet >& xCol);

    void selectionChanged();
    void columnChanged();

    DECL_LINK_TYPED(OnQueryGridSlotState, sal_uInt16, int);
    DECL_LINK_TYPED(OnExecuteGridSlot, sal_uInt16, bool);
};



#endif // _SVX_FMGRID_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
