/*************************************************************************
 *
 *  $RCSfile: Numeric.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:29:06 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _FORMS_NUMERIC_HXX_
#include "Numeric.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

//.........................................................................
namespace frm
{
//.........................................................................

//==================================================================
// ONumericControl
//==================================================================

//------------------------------------------------------------------
ONumericControl::ONumericControl(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory)
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
InterfaceRef SAL_CALL ONumericControl_CreateInstance(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory)
{
    return *(new ONumericControl(_rxFactory));
}

//------------------------------------------------------------------------------
staruno::Sequence<staruno::Type> ONumericControl::_getTypes()
{
    return OBoundControl::_getTypes();
}

//==================================================================
// ONumericModel
//==================================================================
sal_Int32   ONumericModel::nValueHandle = -1;
//------------------------------------------------------------------
InterfaceRef SAL_CALL ONumericModel_CreateInstance(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory)
{
    return *(new ONumericModel(_rxFactory));
}

//------------------------------------------------------------------------------
staruno::Sequence<staruno::Type> ONumericModel::_getTypes()
{
    return OEditBaseModel::_getTypes();
}

//------------------------------------------------------------------
ONumericModel::ONumericModel(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory)
                :OEditBaseModel(_rxFactory, VCL_CONTROLMODEL_NUMERICFIELD, FRM_CONTROL_NUMERICFIELD)
                                    // use the old control name for compytibility reasons
{
    m_nClassId = starform::FormComponentType::NUMERICFIELD;
    m_sDataFieldConnectivityProperty = PROPERTY_VALUE;
    if (ONumericModel::nValueHandle == -1)
        ONumericModel::nValueHandle = getOriginalHandle(PROPERTY_ID_VALUE);
}

//------------------------------------------------------------------
ONumericModel::~ONumericModel()
{
}

// starlang::XServiceInfo
//------------------------------------------------------------------------------
StringSequence ONumericModel::getSupportedServiceNames() throw()
{
    StringSequence aSupported = OBoundControlModel::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 2);

    ::rtl::OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-2] = ::rtl::OUString::createFromAscii("com.sun.star.form.component.starform");
    pArray[aSupported.getLength()-1] = FRM_SUN_COMPONENT_NUMERICFIELD;
    return aSupported;
}

//------------------------------------------------------------------------------
staruno::Reference<starbeans::XPropertySetInfo> SAL_CALL ONumericModel::getPropertySetInfo() throw( staruno::RuntimeException )
{
    staruno::Reference<starbeans::XPropertySetInfo>  xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//------------------------------------------------------------------------------
void ONumericModel::fillProperties(
        staruno::Sequence< starbeans::Property >& _rProps,
        staruno::Sequence< starbeans::Property >& _rAggregateProps ) const
{
    FRM_BEGIN_PROP_HELPER(10)
        // Value auf transient setzen
//      ModifyPropertyAttributes(_rAggregateProps, PROPERTY_VALUE, starbeans::PropertyAttribute::TRANSIENT, 0);

        DECL_PROP2(CLASSID,         sal_Int16,          READONLY, TRANSIENT);
        DECL_PROP3(DEFAULT_VALUE,   double,             BOUND, MAYBEDEFAULT, MAYBEVOID);
        DECL_PROP1(NAME,            ::rtl::OUString,    BOUND);
        DECL_PROP1(TAG,             ::rtl::OUString,    BOUND);
        DECL_PROP1(TABINDEX,        sal_Int16,          BOUND);
        DECL_PROP1(CONTROLSOURCE,   ::rtl::OUString,    BOUND);
        DECL_PROP1(HELPTEXT,        ::rtl::OUString,    BOUND);
        DECL_IFACE_PROP2(BOUNDFIELD,        starbeans::XPropertySet,READONLY, TRANSIENT);
        DECL_IFACE_PROP2(CONTROLLABEL,      starbeans::XPropertySet,    BOUND, MAYBEVOID);
        DECL_PROP2(CONTROLSOURCEPROPERTY,   rtl::OUString,  READONLY, TRANSIENT);
    FRM_END_PROP_HELPER();
}

//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& ONumericModel::getInfoHelper()
{
    return *const_cast<ONumericModel*>(this)->getArrayHelper();
}

//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL ONumericModel::getServiceName()
{
    return FRM_COMPONENT_NUMERICFIELD;  // old (non-sun) name for compatibility !
}

// starform::XBoundComponent
//------------------------------------------------------------------------------
sal_Bool ONumericModel::_commit()
{
    staruno::Any aNewValue = m_xAggregateFastSet->getFastPropertyValue( ONumericModel::nValueHandle );
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
            catch(...)
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

    {   // release our mutex once (it's acquired in the calling method !), as setting aggregate properties
        // may cause any uno controls belonging to us to lock the solar mutex, which is potentially dangerous with
        // our own mutex locked
        // FS - 72451 - 31.01.00
        MutexRelease aRelease(m_aMutex);
        m_xAggregateFastSet->setFastPropertyValue(ONumericModel::nValueHandle, m_aSaveValue);
    }
}

// starform::XReset
//------------------------------------------------------------------------------
void ONumericModel::_reset()
{
    staruno::Any aValue;
    if (m_aDefault.getValueType().getTypeClass() == staruno::TypeClass_DOUBLE)
        aValue = m_aDefault;

    {   // release our mutex once (it's acquired in the calling method !), as setting aggregate properties
        // may cause any uno controls belonging to us to lock the solar mutex, which is potentially dangerous with
        // our own mutex locked
        // FS - 72451 - 31.01.00
        MutexRelease aRelease(m_aMutex);
        m_xAggregateFastSet->setFastPropertyValue(ONumericModel::nValueHandle, aValue);
    }
}

//.........................................................................
}   // namespace frm
//.........................................................................

