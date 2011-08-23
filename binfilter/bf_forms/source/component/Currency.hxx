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

#ifndef _FORMS_CURRENCY_HXX_
#define _FORMS_CURRENCY_HXX_

#ifndef _FORMS_EDITBASE_HXX_
#include "EditBase.hxx"
#endif
namespace binfilter {

//.........................................................................
namespace frm
{
//.........................................................................

//==================================================================
//= OCurrencyModel
//==================================================================
class OCurrencyModel
                :public OEditBaseModel
                ,public ::comphelper::OAggregationArrayUsageHelper< OCurrencyModel >
{
    ::com::sun::star::uno::Any			m_aSaveValue;

    static sal_Int32	nValueHandle;

protected:
    virtual void _onValueChanged();
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> _getTypes();

public:
    DECLARE_DEFAULT_LEAF_XTOR( OCurrencyModel );

    // starform::XBoundComponent
    virtual sal_Bool _commit();

    // ::com::sun::star::lang::XServiceInfo
    IMPLEMENTATION_NAME(OCurrencyModel);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw();

    // ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo> SAL_CALL getPropertySetInfo() throw(::com::sun::star::uno::RuntimeException);
    virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

    // ::com::sun::star::io::XPersistObject
    virtual ::rtl::OUString SAL_CALL getServiceName() throw ( ::com::sun ::star::uno::RuntimeException);

    // starform::XReset
    virtual void _reset();

    // OAggregationArrayUsageHelper
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
//= OCurrencyControl
//==================================================================
class OCurrencyControl: public OBoundControl
{
protected:
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type> _getTypes();

public:
    OCurrencyControl(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);
    // ::com::sun::star::lang::XServiceInfo
    IMPLEMENTATION_NAME(OCurrencyControl);
    virtual StringSequence SAL_CALL getSupportedServiceNames() throw();
};

//.........................................................................
}	// namespace frm
//.........................................................................

}//end of namespace binfilter
#endif // _FORMS_CURRENCY_HXX_

