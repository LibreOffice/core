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

#ifndef _COMPHELPER_PROPERTY_ARRAY_HELPER_HXX_
#include <comphelper/proparrhlp.hxx>
#endif

#ifndef _FORMS_CURRENCY_HXX_
#include "Currency.hxx"
#endif

#ifndef INCLUDED_SVTOOLS_SYSLOCALE_HXX 
#include <bf_svtools/syslocale.hxx>
#endif

#ifndef _FRM_SERVICES_HXX_
#include "services.hxx"
#endif

#ifndef _COM_SUN_STAR_FORM_FORMCOMPONENTTYPE_HPP_
#include <com/sun/star/form/FormComponentType.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif

namespace binfilter {

//.........................................................................
namespace frm
{
//.........................................................................
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
//using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;

//==================================================================
// OCurrencyControl
//==================================================================
//------------------------------------------------------------------
OCurrencyControl::OCurrencyControl(const Reference<XMultiServiceFactory>& _rxFactory)
    :OBoundControl(_rxFactory, VCL_CONTROL_CURRENCYFIELD)
{
}

//------------------------------------------------------------------
InterfaceRef SAL_CALL OCurrencyControl_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory)
{
    return *(new OCurrencyControl(_rxFactory));
}

//------------------------------------------------------------------------------
Sequence<Type> OCurrencyControl::_getTypes()
{
    return OBoundControl::_getTypes();
}

//------------------------------------------------------------------------------
StringSequence SAL_CALL OCurrencyControl::getSupportedServiceNames() throw()
{
    StringSequence aSupported = OBoundControl::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 1);

    ::rtl::OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-1] = FRM_SUN_CONTROL_CURRENCYFIELD;
    return aSupported;
}

//==================================================================
// OCurrencyModel
//==================================================================
sal_Int32	OCurrencyModel::nValueHandle = -1;

//------------------------------------------------------------------
InterfaceRef SAL_CALL OCurrencyModel_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory)
{
    return *(new OCurrencyModel(_rxFactory));
}

//------------------------------------------------------------------------------
Sequence<Type> OCurrencyModel::_getTypes()
{
    return OEditBaseModel::_getTypes();
}

//------------------------------------------------------------------
void OCurrencyModel::implConstruct()
{
    if (m_xAggregateSet.is())
    {
        try
        {
            // get the system international informations
            const LocaleDataWrapper& aLocaleInfo = SvtSysLocale().GetLocaleData();

            ::rtl::OUString sCurrencySymbol;
            sal_Bool bPrependCurrencySymbol;
            switch ( aLocaleInfo.getCurrPositiveFormat() )
            {
                case 0:	// $1
                    sCurrencySymbol = String(aLocaleInfo.getCurrSymbol());
                    bPrependCurrencySymbol = sal_True;
                    break;
                case 1:	// 1$
                    sCurrencySymbol = String(aLocaleInfo.getCurrSymbol());
                    bPrependCurrencySymbol = sal_False;
                    break;
                case 2:	// $ 1
                    sCurrencySymbol = ::rtl::OUString(String(aLocaleInfo.getCurrSymbol())) + ::rtl::OUString::createFromAscii(" ");
                    bPrependCurrencySymbol = sal_True;
                    break;
                case 3: // 1 $
                    sCurrencySymbol = ::rtl::OUString::createFromAscii(" ") + ::rtl::OUString(String(aLocaleInfo.getCurrSymbol()));
                    bPrependCurrencySymbol = sal_False;
                    break;
            }
            if (sCurrencySymbol.getLength())
            {
                m_xAggregateSet->setPropertyValue(PROPERTY_CURRENCYSYMBOL, makeAny(sCurrencySymbol));
                m_xAggregateSet->setPropertyValue(PROPERTY_CURRSYM_POSITION, makeAny(bPrependCurrencySymbol));
            }
        }
        catch(Exception&)
        {
            DBG_ERROR( "OCurrencyModel::implConstruct: caught an exception while initializing the aggregate!" );
        }
    }
}

//------------------------------------------------------------------
DBG_NAME( OCurrencyModel )
//------------------------------------------------------------------
OCurrencyModel::OCurrencyModel(const Reference<XMultiServiceFactory>& _rxFactory)
    :OEditBaseModel(_rxFactory, VCL_CONTROLMODEL_CURRENCYFIELD, FRM_CONTROL_CURRENCYFIELD)
                                    // use the old control name for compytibility reasons
{
    DBG_CTOR( OCurrencyModel, NULL );

    m_nClassId = FormComponentType::CURRENCYFIELD;
    m_sDataFieldConnectivityProperty = PROPERTY_VALUE;
    if (OCurrencyModel::nValueHandle == -1)
        OCurrencyModel::nValueHandle = getOriginalHandle(PROPERTY_ID_VALUE);

    implConstruct();
}

//------------------------------------------------------------------
OCurrencyModel::OCurrencyModel( const OCurrencyModel* _pOriginal, const Reference<XMultiServiceFactory>& _rxFactory )
    :OEditBaseModel( _pOriginal, _rxFactory )
{
    DBG_CTOR( OCurrencyModel, NULL );
    implConstruct();
}

//------------------------------------------------------------------
OCurrencyModel::~OCurrencyModel()
{
    DBG_DTOR( OCurrencyModel, NULL );
}

// XCloneable
//------------------------------------------------------------------------------
IMPLEMENT_DEFAULT_CLONING( OCurrencyModel )

// XServiceInfo
//------------------------------------------------------------------------------
StringSequence SAL_CALL OCurrencyModel::getSupportedServiceNames() throw()
{
    StringSequence aSupported = OBoundControlModel::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 2);

    ::rtl::OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-2] = FRM_SUN_COMPONENT_DATABASE_CURRENCYFIELD;
    pArray[aSupported.getLength()-1] = FRM_SUN_COMPONENT_CURRENCYFIELD;
    return aSupported;
}

//------------------------------------------------------------------------------
Reference<XPropertySetInfo> SAL_CALL OCurrencyModel::getPropertySetInfo() throw( RuntimeException )
{
    Reference<XPropertySetInfo>  xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//------------------------------------------------------------------------------
void OCurrencyModel::fillProperties(
        Sequence< Property >& _rProps,
        Sequence< Property >& _rAggregateProps ) const
{
    FRM_BEGIN_PROP_HELPER(9)
        // Value auf transient setzen
//		ModifyPropertyAttributes(_rAggregateProps, PROPERTY_VALUE, PropertyAttribute::TRANSIENT, 0);

        DECL_PROP2(CLASSID,		sal_Int16,					READONLY, TRANSIENT);
        DECL_PROP3(DEFAULT_VALUE,		double,				BOUND, MAYBEDEFAULT, MAYBEVOID);
        DECL_PROP1(NAME,		::rtl::OUString,			BOUND);
        DECL_PROP1(TAG,		::rtl::OUString,				BOUND);
        DECL_PROP1(TABINDEX,		sal_Int16,				BOUND);
        DECL_PROP1(CONTROLSOURCE,		::rtl::OUString,	BOUND);
        DECL_IFACE_PROP3(BOUNDFIELD,		XPropertySet,	BOUND,READONLY, TRANSIENT);
        DECL_IFACE_PROP2(CONTROLLABEL,		XPropertySet,	BOUND, MAYBEVOID);
        DECL_PROP2(CONTROLSOURCEPROPERTY,	rtl::OUString,	READONLY, TRANSIENT);
    FRM_END_PROP_HELPER();
}

//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& OCurrencyModel::getInfoHelper()
{
    return *const_cast<OCurrencyModel*>(this)->getArrayHelper();
}

//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OCurrencyModel::getServiceName() throw ( ::com::sun::star::uno::RuntimeException)
{
    return FRM_COMPONENT_CURRENCYFIELD;	// old (non-sun) name for compatibility !
}

// XBoundComponent
//------------------------------------------------------------------------------
sal_Bool OCurrencyModel::_commit()
{
    Any aNewValue = m_xAggregateFastSet->getFastPropertyValue( OCurrencyModel::nValueHandle );
    if (!compare(aNewValue, m_aSaveValue))
    {
        if (aNewValue.getValueType().getTypeClass() == TypeClass_VOID)
            m_xColumnUpdate->updateNull();
        else
        {
            try
            {
                m_xColumnUpdate->updateDouble(getDouble(aNewValue));
            }
            catch(Exception&)
            {
                return sal_False;
            }
        }
        m_aSaveValue = aNewValue;
    }
    return sal_True;
}

//------------------------------------------------------------------------------
void OCurrencyModel::_onValueChanged()
{
    m_aSaveValue <<= m_xColumn->getDouble();
    if (m_xColumn->wasNull())
        m_aSaveValue.clear();
    {	// release our mutex once (it's acquired in the calling method !), as setting aggregate properties
        // may cause any uno controls belonging to us to lock the solar mutex, which is potentially dangerous with
        // our own mutex locked
        // FS - 72451 - 31.01.00
        MutexRelease aRelease(m_aMutex);
        m_xAggregateFastSet->setFastPropertyValue(OCurrencyModel::nValueHandle, m_aSaveValue);
    }
}

// XReset
//------------------------------------------------------------------------------
void OCurrencyModel::_reset( void )
{
    Any aValue;
    if (m_aDefault.getValueType().getTypeClass() == TypeClass_DOUBLE)
        aValue = m_aDefault;

    {	// release our mutex once (it's acquired in the calling method !), as setting aggregate properties
        // may cause any uno controls belonging to us to lock the solar mutex, which is potentially dangerous with
        // our own mutex locked
        // FS - 72451 - 31.01.00
        MutexRelease aRelease(m_aMutex);
        m_xAggregateFastSet->setFastPropertyValue(OCurrencyModel::nValueHandle, aValue);
    }
}

//.........................................................................
}	// namespace frm
//.........................................................................

}
