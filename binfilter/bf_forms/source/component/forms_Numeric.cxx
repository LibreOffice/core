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

#ifndef _FORMS_NUMERIC_HXX_
#include "Numeric.hxx"
#endif


#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif

#ifndef _FRM_SERVICES_HXX_
#include "services.hxx"
#endif

#ifndef _COM_SUN_STAR_FORM_FORMCOMPONENTTYPE_HPP_
#include <com/sun/star/form/FormComponentType.hpp>
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
// ONumericControl
//==================================================================

//------------------------------------------------------------------
ONumericControl::ONumericControl(const Reference<XMultiServiceFactory>& _rxFactory)
    :OBoundControl(_rxFactory, VCL_CONTROL_NUMERICFIELD)
{
}

//------------------------------------------------------------------------------
StringSequence ONumericControl::getSupportedServiceNames() throw()
{
    StringSequence aSupported = OBoundControl::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 1);

    ::rtl::OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-1] = FRM_SUN_CONTROL_NUMERICFIELD;
    return aSupported;
}


//------------------------------------------------------------------
InterfaceRef SAL_CALL ONumericControl_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory)
{
    return *(new ONumericControl(_rxFactory));
}

//------------------------------------------------------------------------------
Sequence<Type> ONumericControl::_getTypes()
{
    return OBoundControl::_getTypes();
}

//==================================================================
// ONumericModel
//==================================================================
sal_Int32	ONumericModel::nValueHandle = -1;
//------------------------------------------------------------------
InterfaceRef SAL_CALL ONumericModel_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory)
{
    return *(new ONumericModel(_rxFactory));
}

//------------------------------------------------------------------------------
Sequence<Type> ONumericModel::_getTypes()
{
    return OEditBaseModel::_getTypes();
}

//------------------------------------------------------------------
DBG_NAME( ONumericModel )
//------------------------------------------------------------------
ONumericModel::ONumericModel(const Reference<XMultiServiceFactory>& _rxFactory)
                :OEditBaseModel(_rxFactory, VCL_CONTROLMODEL_NUMERICFIELD, FRM_CONTROL_NUMERICFIELD)
                                    // use the old control name for compytibility reasons
{
    DBG_CTOR( ONumericModel, NULL );

    m_nClassId = FormComponentType::NUMERICFIELD;
    m_sDataFieldConnectivityProperty = PROPERTY_VALUE;
    if (ONumericModel::nValueHandle == -1)
        ONumericModel::nValueHandle = getOriginalHandle(PROPERTY_ID_VALUE);
}

//------------------------------------------------------------------
ONumericModel::ONumericModel( const ONumericModel* _pOriginal, const Reference<XMultiServiceFactory>& _rxFactory )
    :OEditBaseModel( _pOriginal, _rxFactory )
{
    DBG_CTOR( ONumericModel, NULL );
}

//------------------------------------------------------------------
ONumericModel::~ONumericModel()
{
    DBG_DTOR( ONumericModel, NULL );
}

// XCloneable
//------------------------------------------------------------------------------
IMPLEMENT_DEFAULT_CLONING( ONumericModel )

// XServiceInfo
//------------------------------------------------------------------------------
StringSequence ONumericModel::getSupportedServiceNames() throw()
{
    StringSequence aSupported = OBoundControlModel::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 2);

    ::rtl::OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-2] = FRM_SUN_COMPONENT_DATABASE_NUMERICFIELD;
    pArray[aSupported.getLength()-1] = FRM_SUN_COMPONENT_NUMERICFIELD;
    return aSupported;
}

//------------------------------------------------------------------------------
Reference<XPropertySetInfo> SAL_CALL ONumericModel::getPropertySetInfo() throw( RuntimeException )
{
    Reference<XPropertySetInfo>  xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//------------------------------------------------------------------------------
void ONumericModel::fillProperties(
        Sequence< Property >& _rProps,
        Sequence< Property >& _rAggregateProps ) const
{
    FRM_BEGIN_PROP_HELPER(9)
        // Value auf transient setzen
//		ModifyPropertyAttributes(_rAggregateProps, PROPERTY_VALUE, PropertyAttribute::TRANSIENT, 0);

        DECL_PROP2(CLASSID,			sal_Int16,			READONLY, TRANSIENT);
        DECL_PROP3(DEFAULT_VALUE,	double,				BOUND, MAYBEDEFAULT, MAYBEVOID);
        DECL_PROP1(NAME,			::rtl::OUString,	BOUND);
        DECL_PROP1(TAG,				::rtl::OUString,	BOUND);
        DECL_PROP1(TABINDEX,		sal_Int16,			BOUND);
        DECL_PROP1(CONTROLSOURCE,	::rtl::OUString,	BOUND);
        DECL_IFACE_PROP3(BOUNDFIELD,		XPropertySet,BOUND,READONLY, TRANSIENT);
        DECL_IFACE_PROP2(CONTROLLABEL,		XPropertySet,	BOUND, MAYBEVOID);
        DECL_PROP2(CONTROLSOURCEPROPERTY,	rtl::OUString,	READONLY, TRANSIENT);
    FRM_END_PROP_HELPER();
}

//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& ONumericModel::getInfoHelper()
{
    return *const_cast<ONumericModel*>(this)->getArrayHelper();
}

//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL ONumericModel::getServiceName() throw ( ::com::sun::star::uno::RuntimeException)
{
    return FRM_COMPONENT_NUMERICFIELD;	// old (non-sun) name for compatibility !
}

// XBoundComponent
//------------------------------------------------------------------------------
sal_Bool ONumericModel::_commit()
{
    Any aNewValue = m_xAggregateFastSet->getFastPropertyValue( ONumericModel::nValueHandle );
    if (!compare(aNewValue, m_aSaveValue))
    {
        if (!aNewValue.hasValue())
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
void ONumericModel::_onValueChanged()
{
    m_aSaveValue <<= (double)m_xColumn->getDouble();
    if (m_xColumn->wasNull())
        m_aSaveValue.clear();

    {	// release our mutex once (it's acquired in the calling method !), as setting aggregate properties
        // may cause any uno controls belonging to us to lock the solar mutex, which is potentially dangerous with
        // our own mutex locked
        // FS - 72451 - 31.01.00
        MutexRelease aRelease(m_aMutex);
        m_xAggregateFastSet->setFastPropertyValue(ONumericModel::nValueHandle, m_aSaveValue);
    }
}

// XReset
//------------------------------------------------------------------------------
void ONumericModel::_reset()
{
    Any aValue;
    if (m_aDefault.getValueType().getTypeClass() == TypeClass_DOUBLE)
        aValue = m_aDefault;

    {	// release our mutex once (it's acquired in the calling method !), as setting aggregate properties
        // may cause any uno controls belonging to us to lock the solar mutex, which is potentially dangerous with
        // our own mutex locked
        // FS - 72451 - 31.01.00
        MutexRelease aRelease(m_aMutex);
        m_xAggregateFastSet->setFastPropertyValue(ONumericModel::nValueHandle, aValue);
    }
}

//.........................................................................
}	// namespace frm
//.........................................................................

}
