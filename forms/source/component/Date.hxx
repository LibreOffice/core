/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Date.hxx,v $
 * $Revision: 1.15 $
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

#ifndef _FORMS_DATE_HXX_
#define _FORMS_DATE_HXX_

#include "EditBase.hxx"
#include "limitedformats.hxx"

//.........................................................................
namespace frm
{
//.........................................................................

//==================================================================
//= ODateModel
//==================================================================
class ODateModel
                :public OEditBaseModel
                ,public OLimitedFormats
{
    ::com::sun::star::uno::Any      m_aSaveValue;
    sal_Bool                        m_bDateTimeField;

protected:
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> _getTypes();

public:
    DECLARE_DEFAULT_LEAF_XTOR( ODateModel );

    // XPersistObject
    virtual ::rtl::OUString SAL_CALL getServiceName() throw ( ::com::sun::star::uno::RuntimeException );

    // ::com::sun::star::beans::XPropertySet
    virtual void SAL_CALL getFastPropertyValue(::com::sun::star::uno::Any& rValue, sal_Int32 nHandle ) const;
    virtual sal_Bool SAL_CALL convertFastPropertyValue(::com::sun::star::uno::Any& rConvertedValue, ::com::sun::star::uno::Any& rOldValue,
                                          sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue )
                                        throw(::com::sun::star::lang::IllegalArgumentException);
    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const ::com::sun::star::uno::Any& rValue) throw ( ::com::sun::star::uno::Exception);

    // XServiceInfo
    IMPLEMENTATION_NAME(ODateModel);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw();

    // XPropertySet
    using OEditBaseModel::getFastPropertyValue;

    // OControlModel's property handling
    virtual void describeFixedProperties(
        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property >& /* [out] */ _rProps
    ) const;

protected:
    // OBoundControlModel overridables
    virtual ::com::sun::star::uno::Any
                            translateDbColumnToControlValue( );
    virtual sal_Bool        commitControlValueToDbColumn( bool _bPostReset );

    virtual ::com::sun::star::uno::Any
                            translateControlValueToExternalValue( ) const;
    virtual ::com::sun::star::uno::Any
                            translateExternalValueToControlValue( ) const;

    virtual ::com::sun::star::uno::Any
                            translateControlValueToValidatableValue( ) const;

    virtual ::com::sun::star::uno::Any
                            getDefaultForReset() const;

    virtual void            onConnectedDbColumn( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& _rxForm );

    virtual sal_Bool        approveValueBinding( const ::com::sun::star::uno::Reference< ::com::sun::star::form::binding::XValueBinding >& _rxBinding );

protected:
    DECLARE_XCLONEABLE();

private:
    /** translates the control value (the VCL-internal integer representation of a date) into
        a UNO-Date.
    */
    void                    impl_translateControlValueToUNODate(
                                ::com::sun::star::uno::Any& _rUNOValue ) const;
};

//==================================================================
//= ODateControl
//==================================================================
class ODateControl: public OBoundControl
{
protected:
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> _getTypes();

public:
    ODateControl(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);
    DECLARE_UNO3_AGG_DEFAULTS(ODateControl, OBoundControl);

// ::com::sun::star::lang::XServiceInfo
    IMPLEMENTATION_NAME(ODateControl);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw();
};

//.........................................................................
}   // namespace frm
//.........................................................................

#endif // _FORMS_DATE_HXX_

