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
    DECLARE_DEFAULT_LEAF_XTOR( OButtonModel );

    // UNO
    DECLARE_UNO3_AGG_DEFAULTS( OButtonModel, OClickableImageBaseModel )
    virtual css::uno::Any SAL_CALL queryAggregation( const css::uno::Type& _rType ) throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    css::uno::Sequence< css::uno::Type> _getTypes() SAL_OVERRIDE;

// css::lang::XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE
    { return OUString("com.sun.star.form.OButtonModel"); }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() throw(std::exception) SAL_OVERRIDE;

// css::io::XPersistObject
    virtual OUString SAL_CALL getServiceName() throw ( css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL write(const css::uno::Reference< css::io::XObjectOutputStream>& _rxOutStream) throw (css::io::IOException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL read(const css::uno::Reference< css::io::XObjectInputStream>& _rxInStream) throw (css::io::IOException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XReset
    virtual void SAL_CALL reset(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL addResetListener( const css::uno::Reference< css::form::XResetListener >& aListener ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeResetListener( const css::uno::Reference< css::form::XResetListener >& aListener ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // OControlModel's property handling
    virtual void describeFixedProperties(
        css::uno::Sequence< css::beans::Property >& /* [out] */ _rProps
    ) const SAL_OVERRIDE;

    // XPropertySet and friends
    virtual void SAL_CALL getFastPropertyValue(css::uno::Any& rValue, sal_Int32 nHandle) const SAL_OVERRIDE;
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const css::uno::Any& rValue )
                throw (css::uno::Exception, std::exception) SAL_OVERRIDE;
    virtual sal_Bool SAL_CALL convertFastPropertyValue(
                css::uno::Any& _rConvertedValue, css::uno::Any& _rOldValue, sal_Int32 _nHandle, const css::uno::Any& _rValue )
                throw (css::lang::IllegalArgumentException) SAL_OVERRIDE;
    virtual css::uno::Any getPropertyDefaultByHandle( sal_Int32 nHandle ) const SAL_OVERRIDE;

    // OComponentHelper
    virtual void SAL_CALL disposing() SAL_OVERRIDE;

protected:
    virtual css::uno::Reference< css::util::XCloneable > SAL_CALL createClone(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

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
    virtual css::uno::Sequence< css::uno::Type> _getTypes() SAL_OVERRIDE;

public:
    OButtonControl(const css::uno::Reference< css::uno::XComponentContext>& _rxFactory);
    virtual ~OButtonControl();

    // XServiceInfo
    OUString SAL_CALL getImplementationName()
        throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE
    { return OUString("com.sun.star.form.OButtonControl"); }

    virtual css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() throw(std::exception) SAL_OVERRIDE;

    // UNO binding
    DECLARE_UNO3_AGG_DEFAULTS(OButtonControl, OClickableImageBaseControl)
    virtual css::uno::Any SAL_CALL queryAggregation(const css::uno::Type& _rType) throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XActionListener
    virtual void SAL_CALL actionPerformed(const css::awt::ActionEvent& rEvent) throw ( css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XButton
    virtual void SAL_CALL addActionListener(const css::uno::Reference< css::awt::XActionListener>& _rxListener) throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL removeActionListener(const css::uno::Reference< css::awt::XActionListener>& _rxListener) throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setLabel(const OUString& Label) throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL setActionCommand(const OUString& _rCommand) throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // OComponentHelper
    virtual void SAL_CALL disposing() SAL_OVERRIDE;

    // XPropertyChangeListener
    virtual void SAL_CALL propertyChange( const css::beans::PropertyChangeEvent& evt ) throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XEventListener
    virtual void SAL_CALL disposing(const css::lang::EventObject& _rSource) throw(css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // XControl
    virtual sal_Bool SAL_CALL setModel( const css::uno::Reference< css::awt::XControlModel >& _rxModel ) throw ( css::uno::RuntimeException, std::exception ) SAL_OVERRIDE;
    void SAL_CALL setDesignMode(sal_Bool bOn) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

protected:
    // OFormNavigationHelper overriables
    virtual void    getSupportedFeatures( ::std::vector< sal_Int16 >& /* [out] */ _rFeatureIds ) SAL_OVERRIDE;
    virtual void    featureStateChanged( sal_Int16 _nFeatureId, bool _bEnabled ) SAL_OVERRIDE;
    virtual void    allFeatureStatesChanged( ) SAL_OVERRIDE;
    virtual bool    isEnabled( sal_Int16 _nFeatureId ) const SAL_OVERRIDE;

    // XDispatchProviderInterception disambiguaiton
    virtual void SAL_CALL registerDispatchProviderInterceptor( const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& Interceptor ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    virtual void SAL_CALL releaseDispatchProviderInterceptor( const css::uno::Reference< css::frame::XDispatchProviderInterceptor >& Interceptor ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    // OImageControl overridables
    virtual void    actionPerformed_Impl( bool bNotifyListener, const css::awt::MouseEvent& _rEvt ) SAL_OVERRIDE;

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
