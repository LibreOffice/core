/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Button.hxx,v $
 * $Revision: 1.10 $
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
#include <com/sun/star/awt/MouseEvent.hpp>
#include <com/sun/star/lang/EventObject.hpp>
#include <com/sun/star/awt/ActionEvent.hpp>
#include <com/sun/star/awt/XActionListener.hpp>
#include <com/sun/star/awt/XButton.hpp>
#include <com/sun/star/beans/PropertyChangeEvent.hpp>
#include "formnavigation.hxx"

//.........................................................................
namespace frm
{
//.........................................................................

//==================================================================
// OButtonModel
//==================================================================
class OButtonModel
        :public OClickableImageBaseModel
{
public:
    DECLARE_DEFAULT_LEAF_XTOR( OButtonModel );

// ::com::sun::star::lang::XServiceInfo
    IMPLEMENTATION_NAME(OButtonModel);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw();

// ::com::sun::star::io::XPersistObject
    virtual ::rtl::OUString SAL_CALL getServiceName() throw ( ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL write(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream>& _rxOutStream) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL read(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream>& _rxInStream) throw (::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

    // OControlModel's property handling
    virtual void describeFixedProperties(
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps
    ) const;

protected:
    DECLARE_XCLONEABLE();
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
    sal_uInt32  m_nClickEvent;
    sal_Int32   m_nTargetUrlFeatureId;
    /// caches the value of the "Enabled" property of our model
    sal_Bool    m_bEnabledByPropertyValue;

protected:

    // UNO Anbindung
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> _getTypes();

public:
    OButtonControl(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);
    virtual ~OButtonControl();

    // XServiceInfo
    IMPLEMENTATION_NAME(OButtonControl);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw();

    // UNO Anbindung
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
    virtual void    getSupportedFeatures( ::std::vector< sal_Int32 >& /* [out] */ _rFeatureIds );
    virtual void    featureStateChanged( sal_Int32 _nFeatureId, sal_Bool _bEnabled );
    virtual void    allFeatureStatesChanged( );
    virtual bool    isEnabled( sal_Int32 _nFeatureId ) const;

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
    sal_Int32   getModelUrlFeatureId( ) const;

    /** starts or stops listening for changes in model properties we're interested in
    */
    void        startOrStopModelPropertyListening( bool _bStart );
};

//.........................................................................
}   // namespace frm
//.........................................................................

#endif // _FRM_BUTTON_HXX_

