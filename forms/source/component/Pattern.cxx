/*************************************************************************
 *
 *  $RCSfile: Pattern.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: oj $ $Date: 2000-11-23 08:48:15 $
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

#ifndef _FORMS_PATTERN_HXX_
#include "Pattern.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

//.........................................................................
namespace frm
{
//.........................................................................
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;

//==================================================================
// OPatternControl
//==================================================================
//------------------------------------------------------------------
OPatternControl::OPatternControl(const Reference<XMultiServiceFactory>& _rxFactory)
    :OBoundControl(_rxFactory, VCL_CONTROL_PATTERNFIELD)
{
}

//------------------------------------------------------------------
InterfaceRef SAL_CALL OPatternControl_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory)
{
    return *(new OPatternControl(_rxFactory));
}

//------------------------------------------------------------------------------
Sequence<Type> OPatternControl::_getTypes()
{
    return OBoundControl::_getTypes();
}

//------------------------------------------------------------------------------
StringSequence OPatternControl::getSupportedServiceNames() throw()
{
    StringSequence aSupported = OBoundControl::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 1);

    ::rtl::OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-1] = FRM_SUN_CONTROL_PATTERNFIELD;
    return aSupported;
}

//==================================================================
// OPatternModel
//==================================================================
sal_Int32   OPatternModel::nTextHandle = -1;

//------------------------------------------------------------------
InterfaceRef SAL_CALL OPatternModel_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory)
{
    return *(new OPatternModel(_rxFactory));
}

//------------------------------------------------------------------------------
Sequence<Type> OPatternModel::_getTypes()
{
    return OEditBaseModel::_getTypes();
}

//------------------------------------------------------------------
OPatternModel::OPatternModel(const Reference<XMultiServiceFactory>& _rxFactory)
    :OEditBaseModel(_rxFactory, VCL_CONTROLMODEL_PATTERNFIELD, FRM_CONTROL_PATTERNFIELD)
                                    // use the old control name for compytibility reasons
{
    m_nClassId = FormComponentType::PATTERNFIELD;
    m_sDataFieldConnectivityProperty = PROPERTY_TEXT;
    if (OPatternModel::nTextHandle == -1)
        OPatternModel::nTextHandle = getOriginalHandle(PROPERTY_ID_TEXT);
}

//------------------------------------------------------------------
OPatternModel::~OPatternModel()
{
}

// XServiceInfo
//------------------------------------------------------------------------------
StringSequence SAL_CALL OPatternModel::getSupportedServiceNames() throw()
{
    StringSequence aSupported = OBoundControlModel::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 2);

    ::rtl::OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-2] = ::rtl::OUString::createFromAscii("com.sun.star.form.component.DatabasePatternField");
    pArray[aSupported.getLength()-1] = FRM_SUN_COMPONENT_PATTERNFIELD;
    return aSupported;
}


//------------------------------------------------------------------------------
Reference<XPropertySetInfo> SAL_CALL OPatternModel::getPropertySetInfo() throw( RuntimeException )
{
    Reference<XPropertySetInfo>  xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//------------------------------------------------------------------------------
void OPatternModel::fillProperties(
        Sequence< Property >& _rProps,
        Sequence< Property >& _rAggregateProps ) const
{
    FRM_BEGIN_PROP_HELPER(12)
        // Text auf transient setzen
//      ModifyPropertyAttributes(_rAggregateProps, PROPERTY_TEXT, PropertyAttribute::TRANSIENT, 0);
        DECL_PROP1(NAME,            ::rtl::OUString,    BOUND);
        DECL_PROP2(CLASSID,         sal_Int16,          READONLY, TRANSIENT);
        DECL_PROP2(DEFAULT_TEXT,    ::rtl::OUString,    BOUND, MAYBEDEFAULT);
        DECL_BOOL_PROP1(EMPTY_IS_NULL,                  BOUND);
        DECL_PROP1(TAG,             ::rtl::OUString,    BOUND);
        DECL_PROP1(TABINDEX,        sal_Int16,          BOUND);
        DECL_PROP1(CONTROLSOURCE,   ::rtl::OUString,    BOUND);
        DECL_PROP1(HELPTEXT,        ::rtl::OUString,    BOUND);
        DECL_IFACE_PROP2(BOUNDFIELD,    XPropertySet,   READONLY, TRANSIENT);
        DECL_PROP2(FILTERPROPOSAL,  sal_Bool,           BOUND, MAYBEDEFAULT);
        DECL_IFACE_PROP2(CONTROLLABEL,  XPropertySet,   BOUND, MAYBEVOID);
        DECL_PROP2(CONTROLSOURCEPROPERTY,   rtl::OUString,  READONLY, TRANSIENT);
    FRM_END_PROP_HELPER();
}

//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& OPatternModel::getInfoHelper()
{
    return *const_cast<OPatternModel*>(this)->getArrayHelper();
}

//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OPatternModel::getServiceName()
{
    return FRM_COMPONENT_PATTERNFIELD;  // old (non-sun) name for compatibility !
}

// XBoundComponent
//------------------------------------------------------------------------------
sal_Bool OPatternModel::_commit()
{
    ::rtl::OUString aNewValue = getString(m_xAggregateFastSet->getFastPropertyValue( OPatternModel::nTextHandle ));
    if (aNewValue != m_aSaveValue)
    {
        if (!aNewValue.len() && !m_bRequired && m_bEmptyIsNull)
            m_xColumnUpdate->updateNull();
        else
        {
            try
            {
                m_xColumnUpdate->updateString(aNewValue);
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

// XPropertyChangeListener
//------------------------------------------------------------------------------
void OPatternModel::_onValueChanged()
{
    m_aSaveValue = m_xColumn->getString();
    {   // release our mutex once (it's acquired in the calling method !), as setting aggregate properties
        // may cause any uno controls belonging to us to lock the solar mutex, which is potentially dangerous with
        // our own mutex locked
        // FS - 72451 - 31.01.00
        MutexRelease aRelease(m_aMutex);
        m_xAggregateFastSet->setFastPropertyValue(OPatternModel::nTextHandle, makeAny(m_aSaveValue));
    }
}

// XReset
//------------------------------------------------------------------------------
void OPatternModel::_reset( void )
{
    {   // release our mutex once (it's acquired in the calling method !), as setting aggregate properties
        // may cause any uno controls belonging to us to lock the solar mutex, which is potentially dangerous with
        // our own mutex locked
        // FS - 72451 - 31.01.00
        MutexRelease aRelease(m_aMutex);
        m_xAggregateFastSet->setFastPropertyValue(OPatternModel::nTextHandle, makeAny(m_aDefaultText));
    }
}

//.........................................................................
}   // namespace frm
//.........................................................................

