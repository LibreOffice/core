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

#ifndef _FORMS_RADIOBUTTON_HXX_
#define _FORMS_RADIOBUTTON_HXX_

#ifndef _FORMS_FORMCOMPONENT_HXX_
#include "FormComponent.hxx"
#endif

#ifndef _COMPHELPER_PROPERTY_MULTIPLEX_HXX_
#include <comphelper/propmultiplex.hxx>
#endif
namespace binfilter {

//.........................................................................
namespace frm
{

enum { RB_NOCHECK, RB_CHECK, RB_DONTKNOW };

//==================================================================
// ORadioButtonModel
//==================================================================
class ORadioButtonModel		:public OBoundControlModel
                            ,public OPropertyChangeListener
                            ,public ::comphelper::OAggregationArrayUsageHelper< ORadioButtonModel >
{
    ::rtl::OUString		m_sReferenceValue;	// Referenzwert zum Checken des Buttons
    sal_Int16			m_nDefaultChecked;	// Soll beim Reset gecheckt werden ?

    sal_Bool			m_bInReset : 1;

protected:
    virtual	void			_onValueChanged();
    virtual	::com::sun::star::uno::Any	_getControlValue() const;

public:
    DECLARE_DEFAULT_LEAF_XTOR( ORadioButtonModel );

    // XServiceInfo
    IMPLEMENTATION_NAME(ORadioButtonModel);
    virtual StringSequence SAL_CALL	getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    // OPropertySetHelper
    virtual void SAL_CALL getFastPropertyValue(::com::sun::star::uno::Any& rValue, sal_Int32 nHandle) const;
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast( sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue )
                throw (::com::sun::star::uno::Exception);
    virtual sal_Bool SAL_CALL convertFastPropertyValue(
                ::com::sun::star::uno::Any& _rConvertedValue, ::com::sun::star::uno::Any& _rOldValue, sal_Int32 _nHandle, const ::com::sun::star::uno::Any& _rValue )
                throw (::com::sun::star::lang::IllegalArgumentException);

    // XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo> SAL_CALL getPropertySetInfo() throw(::com::sun::star::uno::RuntimeException);
    virtual cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

    // XPersistObject
    virtual ::rtl::OUString SAL_CALL	getServiceName() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL
        write(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectOutputStream>& _rxOutStream) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL
        read(const ::com::sun::star::uno::Reference< ::com::sun::star::io::XObjectInputStream>& _rxInStream) throw(::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);

    // OPropertyChangeListener
    virtual void _propertyChanged(const ::com::sun::star::beans::PropertyChangeEvent& evt) throw(::com::sun::star::uno::RuntimeException);

    // XReset
    virtual void SAL_CALL reset() throw(::com::sun::star::uno::RuntimeException);

    // OAggregationArrayUsageHelper
    virtual void fillProperties(
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps,
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rAggregateProps
        ) const;
    IMPLEMENT_INFO_SERVICE()

protected:
    virtual void		_reset();
    virtual sal_Bool	_commit();

protected:
    void SetSiblingPropsTo(const ::rtl::OUString& rPropName, const ::com::sun::star::uno::Any& rValue);

    void implConstruct();
    DECLARE_XCLONEABLE( );
};

//==================================================================
// ORadioButtonControl
//==================================================================
class ORadioButtonControl: public OBoundControl
{
public:
    ORadioButtonControl(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);

    // XServiceInfo
    IMPLEMENTATION_NAME(ORadioButtonControl);
    virtual StringSequence SAL_CALL	getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

protected:
    // XControl
    virtual void SAL_CALL createPeer(const ::com::sun::star::uno::Reference<starawt::XToolkit>& Toolkit, const ::com::sun::star::uno::Reference<starawt::XWindowPeer>& Parent) throw (::com::sun::star::uno::RuntimeException);
};

//.........................................................................
}
//.........................................................................

}//end of namespace binfilter
#endif // _FORMS_RADIOBUTTON_HXX_

