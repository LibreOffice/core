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
#ifndef INCLUDED_SVX_FMGRIDIF_HXX
#define INCLUDED_SVX_FMGRIDIF_HXX

#include <svx/svxdllapi.h>

#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XContainer.hpp>
#include <com/sun/star/container/XContainerListener.hpp>
#include <com/sun/star/sdbc/XRowSetListener.hpp>
#include <com/sun/star/sdb/XRowSetSupplier.hpp>
#include <com/sun/star/form/XResetListener.hpp>
#include <com/sun/star/form/XBoundComponent.hpp>
#include <com/sun/star/form/XLoadListener.hpp>
#include <com/sun/star/form/XGridControl.hpp>
#include <com/sun/star/form/XGridPeer.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/frame/XStatusListener.hpp>
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#include <com/sun/star/view/XSelectionChangeListener.hpp>
#include <com/sun/star/util/XModeSelector.hpp>
#include <com/sun/star/util/XModifyListener.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>

#include <vcl/wintypes.hxx>
#include <toolkit/controls/unocontrol.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <comphelper/interfacecontainer4.hxx>
#include <comphelper/uno3.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/implbase10.hxx>
#include <memory>

namespace com::sun::star::beans { class XPropertySet; }
namespace cpo::uno { class XComponentContext; }

class DbGridColumn;
enum class DbGridControlNavigationBarState;

class SAL_WARN_UNUSED OWeakSubObject : public ::cppu::OWeakObject
{
protected:
    ::cppu::OWeakObject&    m_rParent;

public:
    OWeakSubObject(::cppu::OWeakObject& rParent) : m_rParent(rParent) { }

    virtual void acquire() noexcept override { m_rParent.acquire(); }
    virtual void release() noexcept override { m_rParent.release(); }
};


// FmXModifyMultiplexer

class SAL_WARN_UNUSED FmXModifyMultiplexer final : public OWeakSubObject
                            ,public ::comphelper::OInterfaceContainerHelper3<css::util::XModifyListener>
                            ,public css::util::XModifyListener
{
public:
    FmXModifyMultiplexer( ::cppu::OWeakObject& rSource, ::osl::Mutex& rMutex );
    DECLARE_UNO3_DEFAULTS(FmXModifyMultiplexer,OWeakSubObject)
    virtual cpo::uno::Any  queryInterface(const cpo::uno::Type& _rType) override;

// css::lang::XEventListener
    virtual void disposing(const css::lang::EventObject& Source) override;

// css::util::XModifyListener
    virtual void modified(const css::lang::EventObject& Source) override;
};


// FmXUpdateMultiplexer

class SAL_WARN_UNUSED FmXUpdateMultiplexer final : public OWeakSubObject,
                             public ::comphelper::OInterfaceContainerHelper3<css::form::XUpdateListener>,
                             public css::form::XUpdateListener
{
public:
    FmXUpdateMultiplexer( ::cppu::OWeakObject& rSource, ::osl::Mutex& rMutex );
    DECLARE_UNO3_DEFAULTS(FmXUpdateMultiplexer,OWeakSubObject)

    virtual cpo::uno::Any  queryInterface(const cpo::uno::Type& _rType) override;

// css::lang::XEventListener
    virtual void disposing(const css::lang::EventObject& Source) override;

// css::form::XUpdateListener
    virtual bool approveUpdate(const css::lang::EventObject &) override;
    virtual void updated(const css::lang::EventObject &) override;
};


// FmXSelectionMultiplexer

class SAL_WARN_UNUSED FmXSelectionMultiplexer final : public OWeakSubObject
                                ,public ::comphelper::OInterfaceContainerHelper3<css::view::XSelectionChangeListener>
                                ,public css::view::XSelectionChangeListener
{
public:
    FmXSelectionMultiplexer( ::cppu::OWeakObject& rSource, ::osl::Mutex& rMutex );
    DECLARE_UNO3_DEFAULTS(FmXSelectionMultiplexer, OWeakSubObject)

    virtual cpo::uno::Any  queryInterface(const cpo::uno::Type& _rType) override;

// css::lang::XEventListener
    virtual void disposing(const css::lang::EventObject& Source) override;

// css::view::XSelectionChangeListener
    virtual void selectionChanged( const css::lang::EventObject& aEvent ) override;
};


// FmXGridControlMultiplexer

class SAL_WARN_UNUSED FmXGridControlMultiplexer final : public OWeakSubObject
                                ,public ::comphelper::OInterfaceContainerHelper3<css::form::XGridControlListener>
                                ,public css::form::XGridControlListener
{
public:
    FmXGridControlMultiplexer( ::cppu::OWeakObject& rSource, ::osl::Mutex& rMutex );
    DECLARE_UNO3_DEFAULTS( FmXGridControlMultiplexer, OWeakSubObject )

    virtual cpo::uno::Any  queryInterface(const cpo::uno::Type& _rType) override;

// css::lang::XEventListener
    virtual void disposing(const css::lang::EventObject& Source) override;

// css::view::XSelectionChangeListener
    virtual void columnChanged( const css::lang::EventObject& _event ) override;
};


// FmXContainerMultiplexer

class SAL_WARN_UNUSED FmXContainerMultiplexer final : public OWeakSubObject,
                                public ::comphelper::OInterfaceContainerHelper3<css::container::XContainerListener>,
                                public css::container::XContainerListener
{
public:
    FmXContainerMultiplexer( ::cppu::OWeakObject& rSource, ::osl::Mutex& rMutex);
    DECLARE_UNO3_DEFAULTS(FmXContainerMultiplexer,OWeakSubObject)
    virtual cpo::uno::Any  queryInterface(const cpo::uno::Type& _rType) override;

// css::lang::XEventListener
    virtual void disposing(const css::lang::EventObject& Source) override;

// css::container::XContainerListener
    virtual void elementInserted(const css::container::ContainerEvent& Event) override;
    virtual void elementRemoved(const css::container::ContainerEvent& Event) override;
    virtual void elementReplaced(const css::container::ContainerEvent& Event) override;
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
class SAL_WARN_UNUSED UNLESS_MERGELIBS(SVXCORE_DLLPUBLIC) FmXGridControl  :public UnoControl
                        ,public FmXGridControl_BASE
{
    FmXModifyMultiplexer        m_aModifyListeners;
    FmXUpdateMultiplexer        m_aUpdateListeners;
    FmXContainerMultiplexer     m_aContainerListeners;
    FmXSelectionMultiplexer     m_aSelectionListeners;
    FmXGridControlMultiplexer   m_aGridControlListeners;

protected:
    bool        m_bInDraw;

    css::uno::Reference< cpo::uno::XComponentContext >    m_xContext;

public:
    FmXGridControl(const css::uno::Reference< cpo::uno::XComponentContext >&);
    virtual ~FmXGridControl() override;

    // UNO binding
    DECLARE_UNO3_AGG_DEFAULTS(FmXGridControl, UnoControl)
    virtual cpo::uno::Any  queryAggregation(const cpo::uno::Type& _rType) override;

// XTypeProvider
    virtual cpo::uno::Sequence< cpo::uno::Type> getTypes(  ) override;
    virtual cpo::uno::Sequence<sal_Int8> getImplementationId(  ) override;

// css::lang::XComponent
    virtual void dispose() override;

// css::lang::XServiceInfo
    virtual bool supportsService(const OUString& ServiceName) override;
    virtual OUString getImplementationName() override;
    virtual cpo::uno::Sequence< OUString > getSupportedServiceNames() override;

// css::awt::XControl
    virtual void createPeer(const css::uno::Reference< css::awt::XToolkit >& _rToolkit, const css::uno::Reference< css::awt::XWindowPeer >& Parent) override;
    virtual bool setModel(const css::uno::Reference< css::awt::XControlModel >& Model) override;
    virtual void setDesignMode(bool bOn) override;

// css::awt::XView
    virtual void draw( sal_Int32 x, sal_Int32 y ) override;

// css::form::XBoundComponent
    virtual void addUpdateListener(const css::uno::Reference< css::form::XUpdateListener >& l) override;
    virtual void removeUpdateListener(const css::uno::Reference< css::form::XUpdateListener >& l) override;
    virtual bool commit() override;

// css::container::XElementAccess
    virtual cpo::uno::Type getElementType(  ) override;
    virtual bool hasElements(  ) override;

// css::container::XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration >  createEnumeration() override;

// css::container::XIndexAccess
    virtual sal_Int32 getCount() override;
    virtual cpo::uno::Any getByIndex(sal_Int32 _rIndex) override;

// css::form::XGridControl
    virtual void addGridControlListener( const css::uno::Reference< css::form::XGridControlListener >& _listener ) override;
    virtual void removeGridControlListener( const css::uno::Reference< css::form::XGridControlListener >& _listener ) override;

// css::form::XGrid (base of XGridControl)
    virtual sal_Int16 getCurrentColumnPosition() override;
    virtual void setCurrentColumnPosition(sal_Int16 nPos) override;

// css::form::XGridFieldDataSupplier (base of XGridControl)
    virtual cpo::uno::Sequence< bool > queryFieldDataType( const cpo::uno::Type& xType ) override;
    virtual cpo::uno::Sequence< cpo::uno::Any > queryFieldData( sal_Int32 nRow, const cpo::uno::Type& xType ) override;

// UnoControl
    virtual OUString GetComponentServiceName() const override;

// css::util::XModifyBroadcaster
    virtual void addModifyListener(const css::uno::Reference< css::util::XModifyListener >& l) override;
    virtual void removeModifyListener(const css::uno::Reference< css::util::XModifyListener >& l) override;

// css::util::XModeSelector
    virtual void setMode(const OUString& Mode) override;
    virtual OUString getMode() override;
    virtual cpo::uno::Sequence< OUString> getSupportedModes() override;
    virtual bool supportsMode(const OUString& Mode) override;

// css::container::XContainer
    virtual void addContainerListener(const css::uno::Reference< css::container::XContainerListener >& l) override;
    virtual void removeContainerListener(const css::uno::Reference< css::container::XContainerListener >& l) override;

// css::frame::XDispatchProvider
    virtual css::uno::Reference< css::frame::XDispatch >  queryDispatch(const css::util::URL& aURL, const OUString& aTargetFrameName, sal_Int32 nSearchFlags) override;
    virtual cpo::uno::Sequence< css::uno::Reference< css::frame::XDispatch >  > queryDispatches(const cpo::uno::Sequence< css::frame::DispatchDescriptor >& aDescripts) override;

// css::frame::XDispatchProviderInterception
    virtual void registerDispatchProviderInterceptor(const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& xInterceptor) override;
    virtual void releaseDispatchProviderInterceptor(const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& xInterceptor) override;

// css::view::XSelectionSupplier
    virtual bool select( const cpo::uno::Any& aSelection ) override;
    virtual cpo::uno::Any getSelection(  ) override;
    virtual void addSelectionChangeListener( const css::uno::Reference< css::view::XSelectionChangeListener >& xListener ) override;
    virtual void removeSelectionChangeListener( const css::uno::Reference< css::view::XSelectionChangeListener >& xListener ) override;

// css::awt::XWindow
    virtual void setFocus() override;

protected:
    virtual rtl::Reference<FmXGridPeer> imp_CreatePeer(vcl::Window* pParent);
        // ImplCreatePeer would be better, but doesn't work because it's not exported
};

// FmXGridPeer -> Peer for the Gridcontrol
class FmGridControl;
class SAL_WARN_UNUSED UNLESS_MERGELIBS(SVXCORE_DLLPUBLIC) FmXGridPeer:
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
    css::uno::Reference< cpo::uno::XComponentContext >    m_xContext;
    std::mutex                                            m_aMutex;

private:
    css::uno::Reference< css::container::XIndexContainer >    m_xColumns;
    css::uno::Reference< css::sdbc::XRowSet >                 m_xCursor;
    ::comphelper::OInterfaceContainerHelper4<css::util::XModifyListener> m_aModifyListeners;
    ::comphelper::OInterfaceContainerHelper4<css::form::XUpdateListener> m_aUpdateListeners;
    ::comphelper::OInterfaceContainerHelper4<css::container::XContainerListener> m_aContainerListeners;
    ::comphelper::OInterfaceContainerHelper4<css::view::XSelectionChangeListener> m_aSelectionListeners;
    ::comphelper::OInterfaceContainerHelper4<css::form::XGridControlListener> m_aGridControlListeners;

    OUString                m_aMode;
    sal_Int32               m_nCursorListening;

    css::uno::Reference< css::frame::XDispatchProviderInterceptor >   m_xFirstDispatchInterceptor;

    bool                                m_bInterceptingDispatch;

    std::unique_ptr<bool[]>                 m_pStateCache;
        // one bool for each supported url
    std::unique_ptr<css::uno::Reference< css::frame::XDispatch >[]>   m_pDispatchers;
        // one dispatcher for each supported url
        // (I would like to have a vector here but including the stl in an exported file seems
        // very risky to me...)

    class SAL_DLLPRIVATE GridListenerDelegator;
    friend class GridListenerDelegator;
    std::unique_ptr<GridListenerDelegator>  m_pGridListener;

public:
    FmXGridPeer(const css::uno::Reference< cpo::uno::XComponentContext >&);
    virtual ~FmXGridPeer() override;

    // late constructor, to always be called after the real constructor!
    void Create(vcl::Window* pParent, WinBits nStyle);

// css::form::XGridPeer
    virtual css::uno::Reference< css::container::XIndexContainer > getColumns(  ) override;
    virtual void setColumns( const css::uno::Reference< css::container::XIndexContainer >& aColumns ) override final;

// css::lang::XComponent
    virtual void dispose() override;

// css::lang::XEventListener
    virtual void disposing(const css::lang::EventObject& Source) override;

// css::form::XBoundComponent
    virtual void addUpdateListener(const css::uno::Reference< css::form::XUpdateListener >& l) override;
    virtual void removeUpdateListener(const css::uno::Reference< css::form::XUpdateListener >& l) override;
    virtual bool commit() override;

// css::container::XElementAccess
    virtual cpo::uno::Type getElementType(  ) override;
    virtual bool hasElements(  ) override;

// css::container::XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > createEnumeration() override;

// css::container::XIndexAccess
    virtual sal_Int32 getCount() override;
    virtual cpo::uno::Any getByIndex(sal_Int32 _rIndex) override;

// css::beans::XPropertyChangeListener
    virtual void propertyChange(const css::beans::PropertyChangeEvent& evt) override;

// css::form::XLoadListener
    virtual void loaded(const css::lang::EventObject& rEvent) override;
    virtual void unloaded(const css::lang::EventObject& rEvent) override;
    virtual void unloading(const css::lang::EventObject& aEvent) override;
    virtual void reloading(const css::lang::EventObject& aEvent) override;
    virtual void reloaded(const css::lang::EventObject& aEvent) override;

// css::sdbc::XRowSetListener
    virtual void cursorMoved(const css::lang::EventObject& event) override;
    virtual void rowChanged(const css::lang::EventObject& event) override;
    virtual void rowSetChanged(const css::lang::EventObject& event) override;

// css::container::XContainerListener
    virtual void elementInserted(const css::container::ContainerEvent& Event) override;
    virtual void elementRemoved(const css::container::ContainerEvent& Event) override;
    virtual void elementReplaced(const css::container::ContainerEvent& Event) override;

// VCLXWindow
    virtual void setProperty( const OUString& PropertyName, const cpo::uno::Any& Value ) override;
    virtual cpo::uno::Any getProperty( const OUString& PropertyName ) override;

// css::form::XGridControl
    virtual void addGridControlListener( const css::uno::Reference< css::form::XGridControlListener >& _listener ) override;
    virtual void removeGridControlListener( const css::uno::Reference< css::form::XGridControlListener >& _listener ) override;

// css::form::XGrid (base of XGridControl)
    virtual sal_Int16 getCurrentColumnPosition() override;
    virtual void setCurrentColumnPosition(sal_Int16 nPos) override;

// css::form::XGridFieldDataSupplier (base of XGridControl)
    virtual cpo::uno::Sequence< bool > queryFieldDataType( const cpo::uno::Type& xType ) override;
    virtual cpo::uno::Sequence< cpo::uno::Any > queryFieldData( sal_Int32 nRow, const cpo::uno::Type& xType ) override;

// css::sdb::XRowSetSupplier
    virtual css::uno::Reference< css::sdbc::XRowSet >  getRowSet() override;
    virtual void setRowSet(const css::uno::Reference< css::sdbc::XRowSet >& xDataSource) override final;

// css::util::XModifyBroadcaster
    virtual void addModifyListener(const css::uno::Reference< css::util::XModifyListener >& l) override;
    virtual void removeModifyListener(const css::uno::Reference< css::util::XModifyListener >& l) override;

// UnoControl
    virtual void setDesignMode(bool bOn) override;
    virtual bool isDesignMode() override;

// css::view::XSelectionChangeListener
    virtual void selectionChanged(const css::lang::EventObject& aEvent) override;

    void CellModified();

// PropertyListening
    void updateGrid(const css::uno::Reference< css::sdbc::XRowSet >& _rDatabaseCursor);
    void startCursorListening();
    void stopCursorListening();

// css::util::XModeSelector
    virtual void setMode(const OUString& Mode) override;
    virtual OUString getMode() override;
    virtual cpo::uno::Sequence< OUString > getSupportedModes() override;
    virtual bool supportsMode(const OUString& Mode) override;

// css::container::XContainer
    virtual void addContainerListener(const css::uno::Reference< css::container::XContainerListener >& l) override;
    virtual void removeContainerListener(const css::uno::Reference< css::container::XContainerListener >& l) override;

    void columnVisible(DbGridColumn const * pColumn);
    void columnHidden(DbGridColumn const * pColumn);

// css::awt::XView
    virtual void draw( sal_Int32 x, sal_Int32 y ) override;

// css::frame::XDispatchProvider
    virtual css::uno::Reference< css::frame::XDispatch >  queryDispatch(const css::util::URL& aURL, const OUString& aTargetFrameName, sal_Int32 nSearchFlags) override;
    virtual cpo::uno::Sequence< css::uno::Reference< css::frame::XDispatch >  > queryDispatches(const cpo::uno::Sequence< css::frame::DispatchDescriptor >& aDescripts) override;

// css::frame::XDispatchProviderInterception
    virtual void registerDispatchProviderInterceptor(const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& xInterceptor) override;
    virtual void releaseDispatchProviderInterceptor(const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& xInterceptor) override;

// css::frame::XStatusListener
    virtual void statusChanged(const css::frame::FeatureStateEvent& Event) override;

// css::form::XResetListener
    virtual bool approveReset(const css::lang::EventObject& rEvent) override;
    virtual void resetted(const css::lang::EventObject& rEvent) override;

// css::view::XSelectionSupplier
    virtual bool select( const cpo::uno::Any& aSelection ) override;
    virtual cpo::uno::Any getSelection(  ) override;
    virtual void addSelectionChangeListener( const css::uno::Reference< css::view::XSelectionChangeListener >& xListener ) override;
    virtual void removeSelectionChangeListener( const css::uno::Reference< css::view::XSelectionChangeListener >& xListener ) override;

protected:
    virtual VclPtr<FmGridControl>  imp_CreateControl(vcl::Window* pParent, WinBits nStyle);

    static cpo::uno::Sequence< css::util::URL>&       getSupportedURLs();
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

    DECL_DLLPRIVATE_LINK(OnQueryGridSlotState, DbGridControlNavigationBarState, int);
    DECL_DLLPRIVATE_LINK(OnExecuteGridSlot, DbGridControlNavigationBarState, bool);
};


#endif // _SVX_FMGRID_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
