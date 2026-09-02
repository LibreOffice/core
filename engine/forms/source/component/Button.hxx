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

#include "clickableimage.hxx"
#include <togglestate.hxx>
#include <formnavigation.hxx>
#include <resettable.hxx>

#include <com/sun/star/awt/MouseEvent.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/awt/ActionEvent.hpp>
#include <com/sun/star/awt/XActionListener.hpp>
#include <com/sun/star/awt/XButton.hpp>
#include <com/sun/star/form/XReset.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>

#include <cppuhelper/implbase1.hxx>

struct ImplSVEvent;

namespace frm
{

typedef ::cppu::ImplHelper1 <   css::form::XReset
                            >   OButtonModel_Base;
class OButtonModel  :public OClickableImageBaseModel
                    ,public OButtonModel_Base
{
public:
    OButtonModel(
        const css::uno::Reference< cpo::uno::XComponentContext>& _rxFactory
    );
    OButtonModel(
        const OButtonModel* _pOriginal,
        const css::uno::Reference< cpo::uno::XComponentContext>& _rxFactory
    );
    virtual ~OButtonModel() override;

    // UNO
    DECLARE_UNO3_AGG_DEFAULTS( OButtonModel, OClickableImageBaseModel )
    virtual cpo::uno::Any queryAggregation( const cpo::uno::Type& _rType ) override;

    cpo::uno::Sequence< cpo::uno::Type> _getTypes() override;

// css::lang::XServiceInfo
    OUString getImplementationName() override
    { return u"com.sun.star.form.OButtonModel"_ustr; }

    virtual cpo::uno::Sequence<OUString> getSupportedServiceNames() override;

// css::io::XPersistObject
    virtual OUString getServiceName() override;
    virtual void write(const css::uno::Reference< css::io::XObjectOutputStream>& _rxOutStream) override;
    virtual void read(const css::uno::Reference< css::io::XObjectInputStream>& _rxInStream) override;

    // XReset
    virtual void reset(  ) override;
    virtual void addResetListener( const css::uno::Reference< css::form::XResetListener >& aListener ) override;
    virtual void removeResetListener( const css::uno::Reference< css::form::XResetListener >& aListener ) override;

    // OControlModel's property handling
    virtual void describeFixedProperties(
        cpo::uno::Sequence< css::beans::Property >& /* [out] */ _rProps
    ) const override;

    // XPropertySet and friends
    virtual void getFastPropertyValue(cpo::uno::Any& rValue, sal_Int32 nHandle) const override;
    virtual void setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const cpo::uno::Any& rValue ) override;
    virtual bool convertFastPropertyValue(
                cpo::uno::Any& _rConvertedValue, cpo::uno::Any& _rOldValue, sal_Int32 _nHandle, const cpo::uno::Any& _rValue ) override;
    virtual cpo::uno::Any getPropertyDefaultByHandle( sal_Int32 nHandle ) const override;

    // OComponentHelper
    virtual void disposing() override;

protected:
    virtual css::uno::Reference< css::util::XCloneable > createClone(  ) override;

private:
    void    impl_resetNoBroadcast_nothrow();

    using ::cppu::OPropertySetHelper::getFastPropertyValue;

private:
    ResetHelper m_aResetHelper;

    // <properties>
    ToggleState m_eDefaultState; // the default check state
    // </properties>
protected:
    using OClickableImageBaseModel::disposing;
};


// OButtonControl

typedef ::cppu::ImplHelper3 <   css::awt::XButton
                            ,   css::awt::XActionListener
                            ,   css::beans::XPropertyChangeListener
                            >   OButtonControl_BASE;

class OButtonControl    :public OButtonControl_BASE
                        ,public OClickableImageBaseControl
                        ,public OFormNavigationHelper
{
private:
    ImplSVEvent * m_nClickEvent;
    sal_Int16   m_nTargetUrlFeatureId;
    /// caches the value of the "Enabled" property of our model
    bool    m_bEnabledByPropertyValue;

protected:

    // UNO binding
    virtual cpo::uno::Sequence< cpo::uno::Type> _getTypes() override;

public:
    explicit OButtonControl(const css::uno::Reference< cpo::uno::XComponentContext>& _rxFactory);
    virtual ~OButtonControl() override;

    // XServiceInfo
    OUString getImplementationName() override
    { return u"com.sun.star.form.OButtonControl"_ustr; }

    virtual cpo::uno::Sequence<OUString> getSupportedServiceNames() override;

    // UNO binding
    DECLARE_UNO3_AGG_DEFAULTS(OButtonControl, OClickableImageBaseControl)
    virtual cpo::uno::Any queryAggregation(const cpo::uno::Type& _rType) override;

    // XActionListener
    virtual void actionPerformed(const css::awt::ActionEvent& rEvent) override;

    // XButton
    virtual void addActionListener(const css::uno::Reference< css::awt::XActionListener>& _rxListener) override;
    virtual void removeActionListener(const css::uno::Reference< css::awt::XActionListener>& _rxListener) override;
    virtual void setLabel(const OUString& Label) override;
    virtual void setActionCommand(const OUString& _rCommand) override;

    // OComponentHelper
    virtual void disposing() override;

    // XPropertyChangeListener
    virtual void propertyChange( const css::beans::PropertyChangeEvent& evt ) override;

    // XEventListener
    virtual void disposing(const css::lang::EventObject& _rSource) override;

    // XControl
    virtual bool setModel( const css::uno::Reference< css::awt::XControlModel >& _rxModel ) override;
    void setDesignMode(bool bOn) override;

protected:
    // OFormNavigationHelper overriables
    virtual void    getSupportedFeatures( ::std::vector< sal_Int16 >& /* [out] */ _rFeatureIds ) override;
    virtual void    featureStateChanged( sal_Int16 _nFeatureId, bool _bEnabled ) override;
    virtual void    allFeatureStatesChanged( ) override;
    virtual bool    isEnabled( sal_Int16 _nFeatureId ) const override;

    // XDispatchProviderInterception disambiguation
    virtual void registerDispatchProviderInterceptor( const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& Interceptor ) override;
    virtual void releaseDispatchProviderInterceptor( const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& Interceptor ) override;

    // OImageControl overridables
    virtual void    actionPerformed_Impl( bool bNotifyListener, const css::awt::MouseEvent& _rEvt ) override;

private:
    DECL_LINK( OnClick, void*, void );

    /// to be called whenever the feature URL represented by our model has potentially changed
    void        modelFeatureUrlPotentiallyChanged( );

    /// retrieves the feature id (see OFormNavigationHelper) of the TargetURL of the model.
    sal_Int16   getModelUrlFeatureId( ) const;

    /// starts or stops listening for changes in model properties we're interested in
    void        startOrStopModelPropertyListening( bool _bStart );
};


}   // namespace frm

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
