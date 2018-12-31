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
#include <com/sun/star/beans/XPropertySet.hpp>
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
#include <memory>

class DbGridColumn;
enum class DbGridControlNavigationBarState;

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
                            ,public ::comphelper::OInterfaceContainerHelper2
                            ,public css::util::XModifyListener
{
public:
    FmXModifyMultiplexer( ::cppu::OWeakObject& rSource, ::osl::Mutex& rMutex );
    DECLARE_UNO3_DEFAULTS(FmXModifyMultiplexer,OWeakSubObject)
    virtual css::uno::Any  SAL_CALL queryInterface(const css::uno::Type& _rType) override;

// css::lang::XEventListener
    virtual void SAL_CALL disposing(const css::lang::EventObject& Source) override;

// css::util::XModifyListener
    virtual void SAL_CALL modified(const css::lang::EventObject& Source) override;

// resolve ambiguity : both OWeakObject and OInterfaceContainerHelper2 have these memory operators
    using OWeakSubObject::operator new;
    using OWeakSubObject::operator delete;
};


// FmXUpdateMultiplexer

class SAL_WARN_UNUSED FmXUpdateMultiplexer : public OWeakSubObject,
                             public ::comphelper::OInterfaceContainerHelper2,
                             public css::form::XUpdateListener
{
public:
    FmXUpdateMultiplexer( ::cppu::OWeakObject& rSource, ::osl::Mutex& rMutex );
    DECLARE_UNO3_DEFAULTS(FmXUpdateMultiplexer,OWeakSubObject)

    virtual css::uno::Any  SAL_CALL queryInterface(const css::uno::Type& _rType) override;

// css::lang::XEventListener
    virtual void SAL_CALL disposing(const css::lang::EventObject& Source) override;

// css::form::XUpdateListener
    virtual sal_Bool SAL_CALL approveUpdate(const css::lang::EventObject &) override;
    virtual void SAL_CALL updated(const css::lang::EventObject &) override;

// resolve ambiguity : both OWeakObject and OInterfaceContainerHelper2 have these memory operators
    using OWeakSubObject::operator new;
    using OWeakSubObject::operator delete;
};


// FmXSelectionMultiplexer

class SAL_WARN_UNUSED FmXSelectionMultiplexer   :public OWeakSubObject
                                ,public ::comphelper::OInterfaceContainerHelper2
                                ,public css::view::XSelectionChangeListener
{
public:
    FmXSelectionMultiplexer( ::cppu::OWeakObject& rSource, ::osl::Mutex& rMutex );
    DECLARE_UNO3_DEFAULTS(FmXSelectionMultiplexer, OWeakSubObject)

    virtual css::uno::Any  SAL_CALL queryInterface(const css::uno::Type& _rType) override;

// css::lang::XEventListener
    virtual void SAL_CALL disposing(const css::lang::EventObject& Source) override;

// css::view::XSelectionChangeListener
    virtual void SAL_CALL selectionChanged( const css::lang::EventObject& aEvent ) override;

// resolve ambiguity : both OWeakObject and OInterfaceContainerHelper2 have these memory operators
    using OWeakSubObject::operator new;
    using OWeakSubObject::operator delete;
};


// FmXGridControlMultiplexer

class SAL_WARN_UNUSED FmXGridControlMultiplexer :public OWeakSubObject
                                ,public ::comphelper::OInterfaceContainerHelper2
                                ,public css::form::XGridControlListener
{
public:
    FmXGridControlMultiplexer( ::cppu::OWeakObject& rSource, ::osl::Mutex& rMutex );
    DECLARE_UNO3_DEFAULTS( FmXGridControlMultiplexer, OWeakSubObject )

    virtual css::uno::Any  SAL_CALL queryInterface(const css::uno::Type& _rType) override;

// css::lang::XEventListener
    virtual void SAL_CALL disposing(const css::lang::EventObject& Source) override;

// css::view::XSelectionChangeListener
    virtual void SAL_CALL columnChanged( const css::lang::EventObject& _event ) override;

// resolve ambiguity : both OWeakObject and OInterfaceContainerHelper2 have these memory operators
    using OWeakSubObject::operator new;
    using OWeakSubObject::operator delete;
};


// FmXContainerMultiplexer

class SAL_WARN_UNUSED FmXContainerMultiplexer : public OWeakSubObject,
                                public ::comphelper::OInterfaceContainerHelper2,
                                public css::container::XContainerListener
{
public:
    FmXContainerMultiplexer( ::cppu::OWeakObject& rSource, ::osl::Mutex& rMutex);
    DECLARE_UNO3_DEFAULTS(FmXContainerMultiplexer,OWeakSubObject)
    virtual css::uno::Any  SAL_CALL queryInterface(const css::uno::Type& _rType) override;

// css::lang::XEventListener
    virtual void SAL_CALL disposing(const css::lang::EventObject& Source) override;

// css::container::XContainerListener
    virtual void SAL_CALL elementInserted(const css::container::ContainerEvent& Event) override;
    virtual void SAL_CALL elementRemoved(const css::container::ContainerEvent& Event) override;
    virtual void SAL_CALL elementReplaced(const css::container::ContainerEvent& Event) override;

// resolve ambiguity : both OWeakObject and OInterfaceContainerHelper2 have these memory operators
    using OWeakSubObject::operator new;
    using OWeakSubObject::operator delete;
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
class SAL_WARN_UNUSED SVX_DLLPUBLIC FmXGridControl  :public UnoControl
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
    virtual ~FmXGridControl() override;

    // UNO binding
    DECLARE_UNO3_AGG_DEFAULTS(FmXGridControl, UnoControl)
    virtual css::uno::Any  SAL_CALL queryAggregation(const css::uno::Type& _rType) override;

// XTypeProvider
    virtual css::uno::Sequence< css::uno::Type> SAL_CALL getTypes(  ) override;
    virtual css::uno::Sequence<sal_Int8> SAL_CALL getImplementationId(  ) override;

// css::lang::XComponent
    virtual void SAL_CALL dispose() override;

// css::lang::XServiceInfo
    virtual sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;
    virtual OUString SAL_CALL getImplementationName() override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

// css::awt::XControl
    virtual void SAL_CALL createPeer(const css::uno::Reference< css::awt::XToolkit >& _rToolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent) override;
    virtual sal_Bool SAL_CALL setModel(const css::uno::Reference< css::awt::XControlModel >& Model) override;
    virtual void SAL_CALL setDesignMode(sal_Bool bOn) override;

// css::awt::XView
    virtual void SAL_CALL draw( sal_Int32 x, sal_Int32 y ) override;

// css::form::XBoundComponent
    virtual void SAL_CALL addUpdateListener(const css::uno::Reference< css::form::XUpdateListener >& l) override;
    virtual void SAL_CALL removeUpdateListener(const css::uno::Reference< css::form::XUpdateListener >& l) override;
    virtual sal_Bool SAL_CALL commit() override;

// css::container::XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) override;
    virtual sal_Bool SAL_CALL hasElements(  ) override;

// css::container::XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration >  SAL_CALL createEnumeration() override;

// css::container::XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override;
    virtual css::uno::Any SAL_CALL getByIndex(sal_Int32 _rIndex) override;

// css::form::XGridControl
    virtual void SAL_CALL addGridControlListener( const css::uno::Reference< css::form::XGridControlListener >& _listener ) override;
    virtual void SAL_CALL removeGridControlListener( const css::uno::Reference< css::form::XGridControlListener >& _listener ) override;

// css::form::XGrid (base of XGridControl)
    virtual sal_Int16 SAL_CALL getCurrentColumnPosition() override;
    virtual void SAL_CALL setCurrentColumnPosition(sal_Int16 nPos) override;

// css::form::XGridFieldDataSupplier (base of XGridControl)
    virtual css::uno::Sequence< sal_Bool > SAL_CALL queryFieldDataType( const css::uno::Type& xType ) override;
    virtual css::uno::Sequence< css::uno::Any > SAL_CALL queryFieldData( sal_Int32 nRow, const css::uno::Type& xType ) override;

// UnoControl
    virtual OUString GetComponentServiceName() override;

// css::util::XModifyBroadcaster
    virtual void SAL_CALL addModifyListener(const css::uno::Reference< css::util::XModifyListener >& l) override;
    virtual void SAL_CALL removeModifyListener(const css::uno::Reference< css::util::XModifyListener >& l) override;

// css::util::XModeSelector
    virtual void SAL_CALL setMode(const OUString& Mode) override;
    virtual OUString SAL_CALL getMode() override;
    virtual css::uno::Sequence< OUString> SAL_CALL getSupportedModes() override;
    virtual sal_Bool SAL_CALL supportsMode(const OUString& Mode) override;

// css::container::XContainer
    virtual void SAL_CALL addContainerListener(const css::uno::Reference< css::container::XContainerListener >& l) override;
    virtual void SAL_CALL removeContainerListener(const css::uno::Reference< css::container::XContainerListener >& l) override;

// css::frame::XDispatchProvider
    virtual css::uno::Reference< css::frame::XDispatch >  SAL_CALL queryDispatch(const css::util::URL& aURL, const OUString& aTargetFrameName, sal_Int32 nSearchFlags) override;
    virtual css::uno::Sequence< css::uno::Reference< css::frame::XDispatch >  > SAL_CALL queryDispatches(const css::uno::Sequence< css::frame::DispatchDescriptor >& aDescripts) override;

// css::frame::XDispatchProviderInterception
    virtual void SAL_CALL registerDispatchProviderInterceptor(const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& xInterceptor) override;
    virtual void SAL_CALL releaseDispatchProviderInterceptor(const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& xInterceptor) override;

// css::view::XSelectionSupplier
    virtual sal_Bool SAL_CALL select( const css::uno::Any& aSelection ) override;
    virtual css::uno::Any SAL_CALL getSelection(  ) override;
    virtual void SAL_CALL addSelectionChangeListener( const css::uno::Reference< css::view::XSelectionChangeListener >& xListener ) override;
    virtual void SAL_CALL removeSelectionChangeListener( const css::uno::Reference< css::view::XSelectionChangeListener >& xListener ) override;

protected:
    virtual FmXGridPeer*    imp_CreatePeer(vcl::Window* pParent);
        // ImplCreatePeer would be better, but doesn't work because it's not exported

};


// FmXGridPeer -> Peer for the Gridcontrol

class FmGridControl;
class SAL_WARN_UNUSED SVX_DLLPUBLIC FmXGridPeer:
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
protected:
    css::uno::Reference< css::uno::XComponentContext >    m_xContext;
    ::osl::Mutex                                          m_aMutex;

private:
    css::uno::Reference< css::container::XIndexContainer >    m_xColumns;
    css::uno::Reference< css::sdbc::XRowSet >                 m_xCursor;
    ::comphelper::OInterfaceContainerHelper2       m_aModifyListeners,
                                            m_aUpdateListeners,
                                            m_aContainerListeners,
                                            m_aSelectionListeners,
                                            m_aGridControlListeners;

    OUString                m_aMode;
    sal_Int32               m_nCursorListening;

    css::uno::Reference< css::frame::XDispatchProviderInterceptor >   m_xFirstDispatchInterceptor;

    bool                                m_bInterceptingDispatch;

    std::unique_ptr<bool[]>                 m_pStateCache;
        // one bool for each supported url
    std::unique_ptr<css::uno::Reference< css::frame::XDispatch >[]>   m_pDispatchers;
        // one dispatcher for each supported url
        // (I would like to have a vector here but including the stl in an exported file seems
        // very risky to me ....)

    class GridListenerDelegator;
    friend class GridListenerDelegator;
    std::unique_ptr<GridListenerDelegator>  m_pGridListener;

public:
    FmXGridPeer(const css::uno::Reference< css::uno::XComponentContext >&);
    virtual ~FmXGridPeer() override;

    // late constructor, to always be called after the real constructor!
    void Create(vcl::Window* pParent, WinBits nStyle);

// css::lang::XUnoTunnel
    static const css::uno::Sequence< sal_Int8 >&   getUnoTunnelImplementationId() throw();
    static FmXGridPeer*                                         getImplementation( const css::uno::Reference< css::uno::XInterface >& _rxIFace ) throw();
    sal_Int64                                                   SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& _rIdentifier ) override;

// css::form::XGridPeer
    virtual css::uno::Reference< css::container::XIndexContainer > SAL_CALL getColumns(  ) override;
    virtual void SAL_CALL setColumns( const css::uno::Reference< css::container::XIndexContainer >& aColumns ) override;

// css::lang::XComponent
    virtual void SAL_CALL dispose() override;

// css::lang::XEventListener
    virtual void SAL_CALL disposing(const css::lang::EventObject& Source) override;

// css::form::XBoundComponent
    virtual void SAL_CALL addUpdateListener(const css::uno::Reference< css::form::XUpdateListener >& l) override;
    virtual void SAL_CALL removeUpdateListener(const css::uno::Reference< css::form::XUpdateListener >& l) override;
    virtual sal_Bool SAL_CALL commit() override;

// css::container::XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) override;
    virtual sal_Bool SAL_CALL hasElements(  ) override;

// css::container::XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration() override;

// css::container::XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() override;
    virtual css::uno::Any SAL_CALL getByIndex(sal_Int32 _rIndex) override;

// css::beans::XPropertyChangeListener
    virtual void SAL_CALL propertyChange(const css::beans::PropertyChangeEvent& evt) override;

// css::form::XLoadListener
    virtual void SAL_CALL loaded(const css::lang::EventObject& rEvent) override;
    virtual void SAL_CALL unloaded(const css::lang::EventObject& rEvent) override;
    virtual void SAL_CALL unloading(const css::lang::EventObject& aEvent) override;
    virtual void SAL_CALL reloading(const css::lang::EventObject& aEvent) override;
    virtual void SAL_CALL reloaded(const css::lang::EventObject& aEvent) override;

// css::sdbc::XRowSetListener
    virtual void SAL_CALL cursorMoved(const css::lang::EventObject& event) override;
    virtual void SAL_CALL rowChanged(const css::lang::EventObject& event) override;
    virtual void SAL_CALL rowSetChanged(const css::lang::EventObject& event) override;

// css::container::XContainerListener
    virtual void SAL_CALL elementInserted(const css::container::ContainerEvent& Event) override;
    virtual void SAL_CALL elementRemoved(const css::container::ContainerEvent& Event) override;
    virtual void SAL_CALL elementReplaced(const css::container::ContainerEvent& Event) override;

// VCLXWindow
    virtual void SAL_CALL setProperty( const OUString& PropertyName, const css::uno::Any& Value ) override;
    virtual css::uno::Any SAL_CALL getProperty( const OUString& PropertyName ) override;
    virtual css::uno::Reference< css::accessibility::XAccessibleContext >
                    CreateAccessibleContext() override;

// css::form::XGridControl
    virtual void SAL_CALL addGridControlListener( const css::uno::Reference< css::form::XGridControlListener >& _listener ) override;
    virtual void SAL_CALL removeGridControlListener( const css::uno::Reference< css::form::XGridControlListener >& _listener ) override;

// css::form::XGrid (base of XGridControl)
    virtual sal_Int16 SAL_CALL getCurrentColumnPosition() override;
    virtual void SAL_CALL setCurrentColumnPosition(sal_Int16 nPos) override;

// css::form::XGridFieldDataSupplier (base of XGridControl)
    virtual css::uno::Sequence< sal_Bool > SAL_CALL queryFieldDataType( const css::uno::Type& xType ) override;
    virtual css::uno::Sequence< css::uno::Any > SAL_CALL queryFieldData( sal_Int32 nRow, const css::uno::Type& xType ) override;

// css::sdb::XRowSetSupplier
    virtual css::uno::Reference< css::sdbc::XRowSet >  SAL_CALL getRowSet() override;
    virtual void SAL_CALL setRowSet(const css::uno::Reference< css::sdbc::XRowSet >& xDataSource) override;

// css::util::XModifyBroadcaster
    virtual void SAL_CALL addModifyListener(const css::uno::Reference< css::util::XModifyListener >& l) override;
    virtual void SAL_CALL removeModifyListener(const css::uno::Reference< css::util::XModifyListener >& l) override;

// UnoControl
    virtual void SAL_CALL setDesignMode(sal_Bool bOn) override;
    virtual sal_Bool SAL_CALL isDesignMode() override;

// css::view::XSelectionChangeListener
    virtual void SAL_CALL selectionChanged(const css::lang::EventObject& aEvent) override;

    void CellModified();

// PropertyListening
    void updateGrid(const css::uno::Reference< css::sdbc::XRowSet >& _rDatabaseCursor);
    void startCursorListening();
    void stopCursorListening();

// css::util::XModeSelector
    virtual void SAL_CALL setMode(const OUString& Mode) override;
    virtual OUString SAL_CALL getMode() override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedModes() override;
    virtual sal_Bool SAL_CALL supportsMode(const OUString& Mode) override;

// css::container::XContainer
    virtual void SAL_CALL addContainerListener(const css::uno::Reference< css::container::XContainerListener >& l) override;
    virtual void SAL_CALL removeContainerListener(const css::uno::Reference< css::container::XContainerListener >& l) override;

    void columnVisible(DbGridColumn const * pColumn);
    void columnHidden(DbGridColumn const * pColumn);

// css::awt::XView
    virtual void SAL_CALL draw( sal_Int32 x, sal_Int32 y ) override;

// css::frame::XDispatchProvider
    virtual css::uno::Reference< css::frame::XDispatch >  SAL_CALL queryDispatch(const css::util::URL& aURL, const OUString& aTargetFrameName, sal_Int32 nSearchFlags) override;
    virtual css::uno::Sequence< css::uno::Reference< css::frame::XDispatch >  > SAL_CALL queryDispatches(const css::uno::Sequence< css::frame::DispatchDescriptor >& aDescripts) override;

// css::frame::XDispatchProviderInterception
    virtual void SAL_CALL registerDispatchProviderInterceptor(const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& xInterceptor) override;
    virtual void SAL_CALL releaseDispatchProviderInterceptor(const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& xInterceptor) override;

// css::frame::XStatusListener
    virtual void SAL_CALL statusChanged(const css::frame::FeatureStateEvent& Event) override;

// css::form::XResetListener
    virtual sal_Bool SAL_CALL approveReset(const css::lang::EventObject& rEvent) override;
    virtual void SAL_CALL resetted(const css::lang::EventObject& rEvent) override;

// css::view::XSelectionSupplier
    virtual sal_Bool SAL_CALL select( const css::uno::Any& aSelection ) override;
    virtual css::uno::Any SAL_CALL getSelection(  ) override;
    virtual void SAL_CALL addSelectionChangeListener( const css::uno::Reference< css::view::XSelectionChangeListener >& xListener ) override;
    virtual void SAL_CALL removeSelectionChangeListener( const css::uno::Reference< css::view::XSelectionChangeListener >& xListener ) override;

protected:
    virtual VclPtr<FmGridControl>  imp_CreateControl(vcl::Window* pParent, WinBits nStyle);

    static css::uno::Sequence< css::util::URL>&       getSupportedURLs();
    static const std::vector<DbGridControlNavigationBarState>& getSupportedGridSlots();
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

    DECL_LINK(OnQueryGridSlotState, DbGridControlNavigationBarState, int);
    DECL_LINK(OnExecuteGridSlot, DbGridControlNavigationBarState, bool);
};


#endif // _SVX_FMGRID_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
