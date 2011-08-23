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

#ifndef _FRM_IMAGE_CONTROL_HXX_
#define _FRM_IMAGE_CONTROL_HXX_

#include "FormComponent.hxx"

#ifndef _PRODUCE_HXX
#include "imgprod.hxx"
#endif

#ifndef _COM_SUN_STAR_FORM_XIMAGEPRODUCERSUPPLIER_HPP_
#include <com/sun/star/form/XImageProducerSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XMOUSELISTENER_HPP_
#include <com/sun/star/awt/XMouseListener.hpp>
#endif

#ifndef _COMPHELPER_PROPERTY_MULTIPLEX_HXX_
#include <comphelper/propmultiplex.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
namespace binfilter {

    using namespace ::comphelper;

//.........................................................................
namespace frm
{
//.........................................................................

//==================================================================
// OImageControlModel
//==================================================================
typedef ::cppu::ImplHelper2 <   ::com::sun::star::form::XImageProducerSupplier
                            ,   ::com::sun::star::awt::XImageProducer
                            >   OImageControlModel_Base;

class OImageControlModel
                :public OImageControlModel_Base
                ,public ::comphelper::OAggregationArrayUsageHelper<OImageControlModel>
                ,public OBoundControlModel
                ,public OPropertyChangeListener
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageProducer> 	m_xImageProducer;
    ImageProducer*									m_pImageProducer;
    sal_Bool										m_bReadOnly;

    OPropertyChangeMultiplexer*	m_pAggregatePropertyMultiplexer;

    // Helper functions
    void UpdateFromField();

protected:
    // UNO Anbindung
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> _getTypes();
    virtual void _onValueChanged();

    virtual sal_Bool		_approve(sal_Int32 _nColumnType);

    inline ImageProducer* GetImageProducer() { return m_pImageProducer; }

public:
    DECLARE_DEFAULT_LEAF_XTOR( OImageControlModel );

    virtual void SAL_CALL getFastPropertyValue(::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const;
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue) throw ( ::com::sun::star::uno::Exception);

    virtual sal_Bool SAL_CALL convertFastPropertyValue(::com::sun::star::uno::Any& rConvertedValue, ::com::sun::star::uno::Any& rOldValue, sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue )
        throw(::com::sun::star::lang::IllegalArgumentException);

    // UNO Anbindung
    DECLARE_UNO3_AGG_DEFAULTS(OImageControlModel, OBoundControlModel);
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation(const ::com::sun::star::uno::Type& _rType) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::form::XBoundComponent
    virtual ::com::sun::star::uno::Any _getControlValue() const;
    virtual void _reset();

// ::com::sun::star::lang::XServiceInfo
    IMPLEMENTATION_NAME(OImageControlModel);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw();

// OComponentHelper
    virtual void SAL_CALL disposing();

// OPropertyChangeListener
    virtual void _propertyChanged( const ::com::sun::star::beans::PropertyChangeEvent& ) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo> SAL_CALL getPropertySetInfo() throw(::com::sun::star::uno::RuntimeException);
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

// ::com::sun::star::io::XPersistObject
    virtual ::rtl::OUString SAL_CALL getServiceName() throw ( ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL write(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream>& _rxOutStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL read(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream>& _rxInStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

// ::com::sun::star::form::XImageProducerSupplier
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageProducer> SAL_CALL getImageProducer() throw ( ::com::sun::star::uno::RuntimeException);

// XImageProducer
    virtual void SAL_CALL addConsumer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageConsumer >& xConsumer ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeConsumer( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XImageConsumer >& xConsumer ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL startProduction(  ) throw (::com::sun::star::uno::RuntimeException);

// ::comphelper::OAggregationArrayUsageHelper
    virtual void fillProperties(
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps,
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rAggregateProps
        ) const;
    IMPLEMENT_INFO_SERVICE()

protected:
    DECLARE_XCLONEABLE();

    void implConstruct();
};

//==================================================================
// OImageControlControl
//==================================================================
class OImageControlControl : public ::com::sun::star::awt::XMouseListener,
                               public OBoundControl
{
protected:
    // UNO Anbindung
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> _getTypes();

public:
    OImageControlControl(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);

// UNO Anbindung
    DECLARE_UNO3_AGG_DEFAULTS(OImageControlControl, OBoundControl);
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation(const ::com::sun::star::uno::Type& _rType) throw(::com::sun::star::uno::RuntimeException);

// ::com::sun::star::lang::XEventListener
    virtual void SAL_CALL disposing(const ::com::sun::star::lang::EventObject& _rSource) throw(::com::sun::star::uno::RuntimeException)
        { OBoundControl::disposing(_rSource); }

// ::com::sun::star::lang::XServiceInfo
    IMPLEMENTATION_NAME(OImageControlControl);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw();

// ::com::sun::star::awt::XMouseListener
    virtual void SAL_CALL mousePressed(const ::com::sun::star::awt::MouseEvent& e) throw ( ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL mouseReleased(const ::com::sun::star::awt::MouseEvent& e) throw ( ::com::sun::star::uno::RuntimeException) { }
    virtual void SAL_CALL mouseEntered(const ::com::sun::star::awt::MouseEvent& e) throw ( ::com::sun::star::uno::RuntimeException)	{ }
    virtual void SAL_CALL mouseExited(const ::com::sun::star::awt::MouseEvent& e) throw ( ::com::sun::star::uno::RuntimeException) { }

// ::com::sun::star::awt::XControl
    virtual void SAL_CALL createPeer(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XToolkit>& _rToolkit, const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer>& _rParent) throw(::com::sun::star::uno::RuntimeException);

private:
    void	implClearGraphics();
    void	implInsertGraphics();
};

//.........................................................................
}	// namespace frm
//.........................................................................

}//end of namespace binfilter
#endif // _FRM_IMAGE_CONTROL_HXX_

