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

#include <cppuhelper/propshlp.hxx>
#include <sfx2/sfxbasecontroller.hxx>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/view/XFormLayerAccess.hpp>
#include <com/sun/star/drawing/XDrawView.hpp>
#include <com/sun/star/drawing/framework/XControllerManager.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <comphelper/uno3.hxx>
#include <cppuhelper/implbase.hxx>
#include <tools/weakbase.h>
#include <memory>
#include <vector>

namespace com { namespace sun { namespace star { namespace drawing { class XDrawSubController; } } } }
namespace com { namespace sun { namespace star { namespace drawing { namespace framework { class XConfigurationController; } } } } }
namespace com { namespace sun { namespace star { namespace drawing { namespace framework { class XModuleController; } } } } }
namespace com { namespace sun { namespace star { namespace drawing { class XLayer; } } } }
namespace osl { class Mutex; }

class SdPage;

namespace sd {

typedef ::cppu::ImplInheritanceHelper <
    SfxBaseController,
    css::view::XSelectionSupplier,
    css::lang::XServiceInfo,
    css::drawing::XDrawView,
    css::view::XSelectionChangeListener,
    css::view::XFormLayerAccess,
    css::drawing::framework::XControllerManager,
    css::lang::XUnoTunnel
    > DrawControllerInterfaceBase;

class BroadcastHelperOwner
{
public:
    explicit BroadcastHelperOwner (::osl::Mutex& rMutex) : maBroadcastHelper(rMutex) {};
    ::cppu::OBroadcastHelper maBroadcastHelper;
};

class ViewShellBase;

/** The DrawController is the UNO controller for Impress and Draw.  It
    relies objects that implement the DrawSubController interface for view
    specific behaviour.  The life time of the DrawController is roughly that
    of ViewShellBase but note that the DrawController can (in the case of a
    reload) outlive the ViewShellBase.

    The implementation of the XControllerManager interface is not yet in its
    final form.
*/
class DrawController final
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
    explicit DrawController (ViewShellBase& rBase) throw();

    virtual ~DrawController() throw() override;

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
    void FireVisAreaChanged (const ::tools::Rectangle& rVisArea) throw();

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
    void fireChangeLayer( css::uno::Reference< css::drawing::XLayer>* pCurrentLayer ) throw();
    // change the parameter to int
    //void fireSwitchCurrentPage( String pageName) throw();
    void fireSwitchCurrentPage( sal_Int32 pageIndex) throw();
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

    static const css::uno::Sequence<sal_Int8>& getUnoTunnelId();

    DECLARE_XINTERFACE()
    DECLARE_XTYPEPROVIDER()

    // XComponent
    virtual void SAL_CALL dispose() override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;

    // XController
    virtual sal_Bool SAL_CALL suspend( sal_Bool Suspend ) override;

    // XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // XSelectionSupplier
    virtual sal_Bool SAL_CALL select( const css::uno::Any& aSelection ) override;
    virtual css::uno::Any SAL_CALL getSelection(  ) override;
    virtual void SAL_CALL addSelectionChangeListener( const css::uno::Reference< css::view::XSelectionChangeListener >& xListener ) override;
    virtual void SAL_CALL removeSelectionChangeListener( const css::uno::Reference< css::view::XSelectionChangeListener >& xListener ) override;

    // XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) override;

    // XFormLayerAccess
    virtual css::uno::Reference< css::form::runtime::XFormController > SAL_CALL getFormController( const css::uno::Reference< css::form::XForm >& Form ) override;
    virtual sal_Bool SAL_CALL isFormDesignMode(  ) override;
    virtual void SAL_CALL setFormDesignMode( sal_Bool DesignMode ) override;

    // XControlAccess
    virtual css::uno::Reference< css::awt::XControl > SAL_CALL getControl( const css::uno::Reference< css::awt::XControlModel >& xModel ) override;

    // XDrawView
    virtual void SAL_CALL
        setCurrentPage (
            const css::uno::Reference<
            css::drawing::XDrawPage >& xPage) override;

    virtual css::uno::Reference<
        css::drawing::XDrawPage > SAL_CALL
        getCurrentPage() override;

    // lang::XEventListener
    virtual void SAL_CALL
        disposing (const css::lang::EventObject& rEventObject) override;

    // view::XSelectionChangeListener
    virtual void  SAL_CALL
        selectionChanged (const css::lang::EventObject& rEvent) override;

    // XControllerManager

    virtual css::uno::Reference<css::drawing::framework::XConfigurationController> SAL_CALL
        getConfigurationController() override;

    virtual css::uno::Reference<css::drawing::framework::XModuleController> SAL_CALL
        getModuleController() override;

    // XUnoTunnel

    virtual sal_Int64 SAL_CALL getSomething (const css::uno::Sequence<sal_Int8>& rId) override;

private:
    /** This method must return the name to index table. This table
        contains all property names and types of this object.
     */
    virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;

    static void FillPropertyTable (
        ::std::vector< css::beans::Property>& rProperties);

    /**
     * The same as getFastProperyValue, but return the value through
     * rValue and nHandle is always valid.
     */
    virtual void SAL_CALL getFastPropertyValue(
        css::uno::Any& rValue,
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
        css::uno::Any & rConvertedValue,
        css::uno::Any & rOldValue,
        sal_Int32 nHandle,
        const css::uno::Any& rValue ) override;

    /** The same as setFastProperyValue, but no exception is thrown and nHandle
        is always valid. You must not broadcast the changes in this method.
     */
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
        sal_Int32 nHandle,
        const css::uno::Any& rValue ) override;

    /** When the called object has been disposed already this method throws
        a Disposed exception and does not return.

        @throws css::lang::DisposedException
    */
    void ThrowIfDisposed() const;

    using cppu::OPropertySetHelper::disposing;
    using cppu::OPropertySetHelper::getFastPropertyValue;

    css::uno::Reference< css::drawing::XLayer>* mpCurrentLayer;

    const css::uno::Type m_aSelectionTypeIdentifier;

    /** This pointer to the ViewShellBase can be NULL (after a call to
        ReleaseViewShellBase()).
    */
    ViewShellBase* mpBase;

    ::tools::Rectangle maLastVisArea;
    ::tools::WeakReference<SdPage> mpCurrentPage;
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
        const css::uno::Any& rNewValue,
        const css::uno::Any& rOldValue);

    void ProvideFrameworkControllers();
    void DisposeFrameworkControllers();
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
