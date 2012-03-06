/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _FRM_BUTTON_HXX_
#define _FRM_BUTTON_HXX_

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

//.........................................................................
namespace frm
{
//.........................................................................

//==================================================================
// OButtonModel
//==================================================================
typedef ::cppu::ImplHelper1 <   ::com::sun::star::form::XReset
                            >   OButtonModel_Base;
class OButtonModel  :public OClickableImageBaseModel
                    ,public OButtonModel_Base
{
public:
    DECLARE_DEFAULT_LEAF_XTOR( OButtonModel );

    // UNO
    DECLARE_UNO3_AGG_DEFAULTS( OButtonModel, OClickableImageBaseModel );
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& _rType ) throw(::com::sun::star::uno::RuntimeException);

    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> _getTypes();

// ::com::sun::star::lang::XServiceInfo
    IMPLEMENTATION_NAME(OButtonModel);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw();

// ::com::sun::star::io::XPersistObject
    virtual ::rtl::OUString SAL_CALL getServiceName() throw ( ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL write(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream>& _rxOutStream) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL read(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream>& _rxInStream) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

    // XReset
    virtual void SAL_CALL reset(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addResetListener( const ::com::sun::star::uno::Reference< ::com::sun::star::form::XResetListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeResetListener( const ::com::sun::star::uno::Reference< ::com::sun::star::form::XResetListener >& aListener ) throw (::com::sun::star::uno::RuntimeException);

    // OControlModel's property handling
    virtual void describeFixedProperties(
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps
    ) const;

    // XPropertySet and friends
    virtual void SAL_CALL getFastPropertyValue(::com::sun::star::uno::Any& rValue, sal_Int32 nHandle) const;
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue )
                throw (::com::sun::star::uno::Exception);
    virtual sal_Bool SAL_CALL convertFastPropertyValue(
                ::com::sun::star::uno::Any& _rConvertedValue, ::com::sun::star::uno::Any& _rOldValue, sal_Int32 _nHandle, const ::com::sun::star::uno::Any& _rValue )
                throw (::com::sun::star::lang::IllegalArgumentException);
    virtual ::com::sun::star::uno::Any getPropertyDefaultByHandle( sal_Int32 nHandle ) const;

    // OComponentHelper
    virtual void SAL_CALL disposing();

protected:
    DECLARE_XCLONEABLE();

private:
    void    impl_resetNoBroadcast_nothrow();

    using ::cppu::OPropertySetHelper::getFastPropertyValue;

private:
    ResetHelper m_aResetHelper;

    // <properties>
    ToggleState m_eDefaultState;          // the default check state
    // </properties>
protected:
    using OClickableImageBaseModel::disposing;
};

//==================================================================
// OButtonControl
//==================================================================
typedef ::cppu::ImplHelper3 <   ::com::sun::star::awt::XButton
                            ,   ::com::sun::star::awt::XActionListener
                            ,   ::com::sun::star::beans::XPropertyChangeListener
                            >   OButtonControl_BASE;

class OButtonControl    :public OButtonControl_BASE
                        ,public OClickableImageBaseControl
                        ,public OFormNavigationHelper
{
private:
    sal_uLong m_nClickEvent;
    sal_Int16   m_nTargetUrlFeatureId;
    /// caches the value of the "Enabled" property of our model
    sal_Bool    m_bEnabledByPropertyValue;

protected:

    // UNO binding
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> _getTypes();

public:
    OButtonControl(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);
    virtual ~OButtonControl();

    // XServiceInfo
    IMPLEMENTATION_NAME(OButtonControl);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw();

    // UNO binding
    DECLARE_UNO3_AGG_DEFAULTS(OButtonControl, OClickableImageBaseControl);
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation(const ::com::sun::star::uno::Type& _rType) throw(::com::sun::star::uno::RuntimeException);

    // XActionListener
    virtual void SAL_CALL actionPerformed(const ::com::sun::star::awt::ActionEvent& rEvent) throw ( ::com::sun::star::uno::RuntimeException);

    // XButton
    virtual void SAL_CALL addActionListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeActionListener(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener>& _rxListener) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setLabel(const ::rtl::OUString& Label) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setActionCommand(const ::rtl::OUString& _rCommand) throw(::com::sun::star::uno::RuntimeException);

    // OComponentHelper
    virtual void SAL_CALL disposing();

    // XPropertyChangeListener
    virtual void SAL_CALL propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& evt ) throw(::com::sun::star::uno::RuntimeException);

    // XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& _rSource) throw(::com::sun::star::uno::RuntimeException);

    // XControl
    virtual sal_Bool SAL_CALL setModel( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& _rxModel ) throw ( ::com::sun::star::uno::RuntimeException );
    void SAL_CALL setDesignMode(sal_Bool bOn) throw (::com::sun::star::uno::RuntimeException);

protected:
    // OFormNavigationHelper overriables
    virtual void    getSupportedFeatures( ::std::vector< sal_Int16 >& /* [out] */ _rFeatureIds );
    virtual void    featureStateChanged( sal_Int16 _nFeatureId, sal_Bool _bEnabled );
    virtual void    allFeatureStatesChanged( );
    virtual bool    isEnabled( sal_Int16 _nFeatureId ) const;

    // XDispatchProviderInterception disambiguaiton
    virtual void SAL_CALL registerDispatchProviderInterceptor( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor >& Interceptor ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL releaseDispatchProviderInterceptor( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor >& Interceptor ) throw (::com::sun::star::uno::RuntimeException);

    // OImageControl overridables
    virtual void    actionPerformed_Impl( sal_Bool bNotifyListener, const ::com::sun::star::awt::MouseEvent& _rEvt );

private:
    DECL_LINK( OnClick, void* );

    /** to be called whenever the feature URL represented by our model has potentially changed
    */
    void        modelFeatureUrlPotentiallyChanged( );

    /** retrieves the feature id (see OFormNavigationHelper) of the TargetURL of
        the model.
    */
    sal_Int16   getModelUrlFeatureId( ) const;

    /** starts or stops listening for changes in model properties we're interested in
    */
    void        startOrStopModelPropertyListening( bool _bStart );
};

//.........................................................................
}   // namespace frm
//.........................................................................

#endif // _FRM_BUTTON_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
