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

#ifndef INCLUDED_FORMS_SOURCE_COMPONENT_BUTTON_HXX
#define INCLUDED_FORMS_SOURCE_COMPONENT_BUTTON_HXX

#include "clickableimage.hxx"
#include "togglestate.hxx"
#include "formnavigation.hxx"
#include "resettable.hxx"

#include <com/sun/star/awt/MouseEvent.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/awt/ActionEvent.hpp>
#include <com/sun/star/awt/XActionListener.hpp>
#include <com/sun/star/awt/XButton.hpp>
#include <com/sun/star/form/XReset.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>

#include <cppuhelper/implbase.hxx>

struct ImplSVEvent;

namespace frm
{

typedef ::cppu::ImplHelper  <   css::form::XReset
                            >   OButtonModel_Base;
class OButtonModel  :public OClickableImageBaseModel
                    ,public OButtonModel_Base
{
public:
    DECLARE_DEFAULT_LEAF_XTOR( OButtonModel );

    // UNO
    DECLARE_UNO3_AGG_DEFAULTS( OButtonModel, OClickableImageBaseModel )
    virtual css::uno::Any SAL_CALL queryAggregation( const css::uno::Type& _rType ) throw(css::uno::RuntimeException, std::exception) override;

    css::uno::Sequence< css::uno::Type> _getTypes() override;

// css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override
    { return OUString("com.sun.star.form.OButtonModel"); }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() throw(std::exception) override;

// css::io::XPersistObject
    virtual OUString SAL_CALL getServiceName() throw ( css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL write(const css::uno::Reference< css::io::XObjectOutputStream>& _rxOutStream) throw (css::io::IOException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL read(const css::uno::Reference< css::io::XObjectInputStream>& _rxInStream) throw (css::io::IOException, css::uno::RuntimeException, std::exception) override;

    // XReset
    virtual void SAL_CALL reset(  ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addResetListener( const css::uno::Reference< css::form::XResetListener >& aListener ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeResetListener( const css::uno::Reference< css::form::XResetListener >& aListener ) throw (css::uno::RuntimeException, std::exception) override;

    // OControlModel's property handling
    virtual void describeFixedProperties(
        css::uno::Sequence< css::beans::Property >& /* [out] */ _rProps
    ) const override;

    // XPropertySet and friends
    virtual void SAL_CALL getFastPropertyValue(css::uno::Any& rValue, sal_Int32 nHandle) const override;
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const css::uno::Any& rValue )
                throw (css::uno::Exception, std::exception) override;
    virtual sal_Bool SAL_CALL convertFastPropertyValue(
                css::uno::Any& _rConvertedValue, css::uno::Any& _rOldValue, sal_Int32 _nHandle, const css::uno::Any& _rValue )
                throw (css::lang::IllegalArgumentException) override;
    virtual css::uno::Any getPropertyDefaultByHandle( sal_Int32 nHandle ) const override;

    // OComponentHelper
    virtual void SAL_CALL disposing() override;

protected:
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone(  ) throw (css::uno::RuntimeException, std::exception) override;

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

typedef ::cppu::ImplHelper  <   css::awt::XButton
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
    virtual css::uno::Sequence< css::uno::Type> _getTypes() override;

public:
    explicit OButtonControl(const css::uno::Reference< css::uno::XComponentContext>& _rxFactory);
    virtual ~OButtonControl();

    // XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) override
    { return OUString("com.sun.star.form.OButtonControl"); }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() throw(std::exception) override;

    // UNO binding
    DECLARE_UNO3_AGG_DEFAULTS(OButtonControl, OClickableImageBaseControl)
    virtual css::uno::Any SAL_CALL queryAggregation(const css::uno::Type& _rType) throw(css::uno::RuntimeException, std::exception) override;

    // XActionListener
    virtual void SAL_CALL actionPerformed(const css::awt::ActionEvent& rEvent) throw ( css::uno::RuntimeException, std::exception) override;

    // XButton
    virtual void SAL_CALL addActionListener(const css::uno::Reference< css::awt::XActionListener>& _rxListener) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeActionListener(const css::uno::Reference< css::awt::XActionListener>& _rxListener) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setLabel(const OUString& Label) throw(css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setActionCommand(const OUString& _rCommand) throw(css::uno::RuntimeException, std::exception) override;

    // OComponentHelper
    virtual void SAL_CALL disposing() override;

    // XPropertyChangeListener
    virtual void SAL_CALL propertyChange( const css::beans::PropertyChangeEvent& evt ) throw(css::uno::RuntimeException, std::exception) override;

    // XEventListener
    virtual void SAL_CALL disposing(const css::lang::EventObject& _rSource) throw(css::uno::RuntimeException, std::exception) override;

    // XControl
    virtual sal_Bool SAL_CALL setModel( const css::uno::Reference< css::awt::XControlModel >& _rxModel ) throw ( css::uno::RuntimeException, std::exception ) override;
    void SAL_CALL setDesignMode(sal_Bool bOn) throw (css::uno::RuntimeException, std::exception) override;

protected:
    // OFormNavigationHelper overriables
    virtual void    getSupportedFeatures( ::std::vector< sal_Int16 >& /* [out] */ _rFeatureIds ) override;
    virtual void    featureStateChanged( sal_Int16 _nFeatureId, bool _bEnabled ) override;
    virtual void    allFeatureStatesChanged( ) override;
    virtual bool    isEnabled( sal_Int16 _nFeatureId ) const override;

    // XDispatchProviderInterception disambiguaiton
    virtual void SAL_CALL registerDispatchProviderInterceptor( const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& Interceptor ) throw (css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL releaseDispatchProviderInterceptor( const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& Interceptor ) throw (css::uno::RuntimeException, std::exception) override;

    // OImageControl overridables
    virtual void    actionPerformed_Impl( bool bNotifyListener, const css::awt::MouseEvent& _rEvt ) override;

private:
    DECL_LINK_TYPED( OnClick, void*, void );

    /// to be called whenever the feature URL represented by our model has potentially changed
    void        modelFeatureUrlPotentiallyChanged( );

    /// retrieves the feature id (see OFormNavigationHelper) of the TargetURL of the model.
    sal_Int16   getModelUrlFeatureId( ) const;

    /// starts or stops listening for changes in model properties we're interested in
    void        startOrStopModelPropertyListening( bool _bStart );
};


}   // namespace frm


#endif // INCLUDED_FORMS_SOURCE_COMPONENT_BUTTON_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
