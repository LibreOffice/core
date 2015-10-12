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

#ifndef INCLUDED_SD_SOURCE_UI_INC_DRAWCONTROLLER_HXX
#define INCLUDED_SD_SOURCE_UI_INC_DRAWCONTROLLER_HXX

#include "ViewShell.hxx"

#include <osl/mutex.hxx>
#include <cppuhelper/propshlp.hxx>
#include <sfx2/sfxbasecontroller.hxx>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/view/XFormLayerAccess.hpp>
#include <com/sun/star/drawing/XDrawSubController.hpp>
#include <com/sun/star/drawing/XDrawView.hpp>
#include <com/sun/star/drawing/framework/XConfigurationController.hpp>
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/drawing/framework/ModuleController.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <comphelper/uno3.hxx>
#include <cppuhelper/implbase.hxx>
#include <tools/weakbase.hxx>
#include <memory>
#include <vector>
#include <com/sun/star/drawing/XLayer.hpp>

namespace sd {

typedef ::cppu::ImplInheritanceHelper <
    SfxBaseController,
    ::com::sun::star::view::XSelectionSupplier,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::drawing::XDrawView,
    ::com::sun::star::view::XSelectionChangeListener,
    ::com::sun::star::view::XFormLayerAccess,
    ::com::sun::star::drawing::framework::XControllerManager,
    ::com::sun::star::lang::XUnoTunnel
    > DrawControllerInterfaceBase;

class BroadcastHelperOwner
{
public:
    BroadcastHelperOwner (::osl::Mutex& rMutex) : maBroadcastHelper(rMutex) {};
    ::cppu::OBroadcastHelper maBroadcastHelper;
};

class DrawSubController;
class ViewShellBase;
class ViewShell;

/** The DrawController is the UNO controller for Impress and Draw.  It
    relies objects that implement the DrawSubController interface for view
    specific behaviour.  The life time of the DrawController is roughly that
    of ViewShellBase but note that the DrawController can (in the case of a
    reload) outlive the ViewShellBase.

    The implementation of the XControllerManager interface is not yet in its
    final form.
*/
class DrawController
    : public DrawControllerInterfaceBase,
      private BroadcastHelperOwner,
      public ::cppu::OPropertySetHelper
{
public:
    enum PropertyHandle {
        PROPERTY_WORKAREA = 0,
        PROPERTY_SUB_CONTROLLER = 1,
        PROPERTY_CURRENTPAGE = 2,
        PROPERTY_MASTERPAGEMODE = 3,
        PROPERTY_LAYERMODE = 4,
        PROPERTY_ACTIVE_LAYER = 5,
        PROPERTY_ZOOMTYPE = 6,
        PROPERTY_ZOOMVALUE = 7,
        PROPERTY_VIEWOFFSET = 8,
        PROPERTY_DRAWVIEWMODE = 9
        ,PROPERTY_UPDATEACC = 10
        ,PROPERTY_PAGE_CHANGE = 11
    };

    /** Create a new DrawController object for the given ViewShellBase.
    */
    DrawController (ViewShellBase& rBase) throw();

    virtual ~DrawController() throw();

    /** Replace the currently used sub controller with the given one.  This
        new sub controller is used from now on for the view (that is the
        main view shell to be precise) specific tasks.  Call this method
        with a suitable sub controller whenever the view shell in the center
        pane is exchanged.
        @param pSubController
            The ViewShell specific sub controller or NULL when (temporarily
            while switching to another one) there is no ViewShell displayed
            in the center pane.
    */
    void SetSubController (
        const css::uno::Reference<css::drawing::XDrawSubController>& rxSubController);

    /** Call this method when the VisArea has changed.
    */
    void FireVisAreaChanged (const Rectangle& rVisArea) throw();

    /** Call this method when the selection has changed.
    */
    void FireSelectionChangeListener() throw();

    /** Call this method when the edit mode has changed.
    */
    void FireChangeEditMode (bool bMasterPageMode) throw();

    /** Call this method when the layer mode has changed.
    */
    void FireChangeLayerMode (bool bLayerMode) throw();

    /** Call this method when there is a new current page.
    */
    void FireSwitchCurrentPage (SdPage* pCurrentPage) throw();

    /** Broadcast a sidebar context change that is caused by a view
        switch.
    */
    void BroadcastContextChange() const;
    void NotifyAccUpdate();
    void fireChangeLayer( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XLayer>* pCurrentLayer ) throw();
    // change the parameter to int
    //void fireSwitchCurrentPage( String pageName) throw();
    void fireSwitchCurrentPage( sal_Int32 pageIndex) throw();
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XLayer>* mpCurrentLayer;
    bool IsDisposing() const { return mbDisposing; }

    /** Return a pointer to the ViewShellBase object that the DrawController
        is connected to.
        @return
            The returned pointer is <NULL/> after a call to
            ReleaseViewShellBase().
    */
    ViewShellBase* GetViewShellBase() { return mpBase;}

    /** This method is typically called from the destructor of ViewShellBase
        to tell the DrawController that it and its members must not access
        the ViewShellBase anymore.
        After this call the DrawController is semi-disposed.
    */
    void ReleaseViewShellBase();

    static const ::com::sun::star::uno::Sequence<sal_Int8>& getUnoTunnelId();

    DECLARE_XINTERFACE()
    DECLARE_XTYPEPROVIDER()

    // XComponent
    virtual void SAL_CALL dispose() throw( ::com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XController
    virtual sal_Bool SAL_CALL suspend( sal_Bool Suspend ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XSelectionSupplier
    virtual sal_Bool SAL_CALL select( const ::com::sun::star::uno::Any& aSelection ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getSelection(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addSelectionChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionChangeListener >& xListener ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeSelectionChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionChangeListener >& xListener ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // XFormLayerAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::form::runtime::XFormController > SAL_CALL getFormController( const ::com::sun::star::uno::Reference< ::com::sun::star::form::XForm >& Form ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isFormDesignMode(  ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setFormDesignMode( sal_Bool DesignMode ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XControlAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControl > SAL_CALL getControl( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& xModel ) throw (::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    // XDrawView
    virtual void SAL_CALL
        setCurrentPage (
            const ::com::sun::star::uno::Reference<
            ::com::sun::star::drawing::XDrawPage >& xPage)
        throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::drawing::XDrawPage > SAL_CALL
        getCurrentPage()
        throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // lang::XEventListener
    virtual void SAL_CALL
        disposing (const ::com::sun::star::lang::EventObject& rEventObject)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // view::XSelectionChangeListener
    virtual void  SAL_CALL
        selectionChanged (const ::com::sun::star::lang::EventObject& rEvent)
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XControllerManager

    virtual css::uno::Reference<css::drawing::framework::XConfigurationController> SAL_CALL
        getConfigurationController()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual css::uno::Reference<css::drawing::framework::XModuleController> SAL_CALL
        getModuleController()
        throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // XUnoTunnel

    virtual sal_Int64 SAL_CALL getSomething (const com::sun::star::uno::Sequence<sal_Int8>& rId)
        throw (com::sun::star::uno::RuntimeException, std::exception) override;

protected:
    /** This method must return the name to index table. This table
        contains all property names and types of this object.
     */
    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;

    static void FillPropertyTable (
        ::std::vector< ::com::sun::star::beans::Property>& rProperties);

    /**
     * The same as getFastProperyValue, but return the value through
     * rValue and nHandle is always valid.
     */
    virtual void SAL_CALL getFastPropertyValue(
        ::com::sun::star::uno::Any& rValue,
        sal_Int32 nHandle ) const override;

    /** Convert the value rValue and return the result in rConvertedValue and the
        old value in rOldValue.
        After this call the vetoable listeners are notified.

        @param rConvertedValue
            The converted value. Only set if return is true.
        @param rOldValue
            The old value. Only set if return is true.
        @param nHandle
            The handle of the proberty.
        @return
            <TRUE/> if the value is converted successfully.
        @throws IllegalArgumentException
     */
    virtual sal_Bool SAL_CALL convertFastPropertyValue(
        ::com::sun::star::uno::Any & rConvertedValue,
        ::com::sun::star::uno::Any & rOldValue,
        sal_Int32 nHandle,
        const ::com::sun::star::uno::Any& rValue )
        throw (::com::sun::star::lang::IllegalArgumentException) override;

    /** The same as setFastProperyValue, but no exception is thrown and nHandle
        is always valid. You must not broadcast the changes in this method.
     */
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
        sal_Int32 nHandle,
        const ::com::sun::star::uno::Any& rValue )
        throw (::com::sun::star::uno::Exception, std::exception) override;

    /** When the called object has been disposed already this method throws
        a Disposed exception and does not return.
    */
    void ThrowIfDisposed() const
        throw (::com::sun::star::lang::DisposedException);

    using cppu::OPropertySetHelper::disposing;
    using cppu::OPropertySetHelper::getFastPropertyValue;

private:
    const ::com::sun::star::uno::Type m_aSelectionTypeIdentifier;

    /** This pointer to the ViewShellBase can be NULL (after a call to
        ReleaseViewShellBase()).
    */
    ViewShellBase* mpBase;

    Rectangle maLastVisArea;
    ::tools::WeakReference<SdrPage> mpCurrentPage;
    bool mbMasterPageMode;
    bool mbLayerMode;

    /** This flag indicates whether the called DrawController is being
        disposed or already has been disposed.
    */
    bool mbDisposing;

    ::std::unique_ptr< ::cppu::IPropertyArrayHelper> mpPropertyArrayHelper;

    /** The current sub controller.  May be NULL.
    */
    css::uno::Reference<css::drawing::XDrawSubController> mxSubController;

    css::uno::Reference<
        css::drawing::framework::XConfigurationController> mxConfigurationController;
    css::uno::Reference<
        css::drawing::framework::XModuleController> mxModuleController;

    /** Send an event to all relevant property listeners that a
        property has changed its value.  The fire() method of the
        OPropertySetHelper is wrapped by this method to handle
        exceptions thrown by called listeners.
    */
    void FirePropertyChange (
        sal_Int32 nHandle,
        const ::com::sun::star::uno::Any& rNewValue,
        const ::com::sun::star::uno::Any& rOldValue);

    void ProvideFrameworkControllers();
    void DisposeFrameworkControllers();
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
