/*************************************************************************
 *
 *  $RCSfile: Pattern.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: obo $ $Date: 2003-10-21 08:59:46 $
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
DBG_NAME( OPatternModel )
//------------------------------------------------------------------
OPatternModel::OPatternModel(const Reference<XMultiServiceFactory>& _rxFactory)
    :OEditBaseModel( _rxFactory, VCL_CONTROLMODEL_PATTERNFIELD, FRM_CONTROL_PATTERNFIELD, sal_False )
                                    // use the old control name for compytibility reasons
{
    DBG_CTOR( OPatternModel, NULL );

    m_nClassId = FormComponentType::PATTERNFIELD;
    initValueProperty( PROPERTY_TEXT, PROPERTY_ID_TEXT );
}

//------------------------------------------------------------------
OPatternModel::OPatternModel( const OPatternModel* _pOriginal, const Reference<XMultiServiceFactory>& _rxFactory )
    :OEditBaseModel( _pOriginal, _rxFactory )
{
    DBG_CTOR( OPatternModel, NULL );
}

//------------------------------------------------------------------
OPatternModel::~OPatternModel()
{
    DBG_DTOR( OPatternModel, NULL );
}

// XCloneable
//------------------------------------------------------------------------------
IMPLEMENT_DEFAULT_CLONING( OPatternModel )

// XServiceInfo
//------------------------------------------------------------------------------
StringSequence SAL_CALL OPatternModel::getSupportedServiceNames() throw()
{
    StringSequence aSupported = OBoundControlModel::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 2);

    ::rtl::OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-2] = FRM_SUN_COMPONENT_DATABASE_PATTERNFIELD;
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
    FRM_BEGIN_PROP_HELPER(11)
        // Text auf transient setzen
//      ModifyPropertyAttributes(_rAggregateProps, PROPERTY_TEXT, PropertyAttribute::TRANSIENT, 0);
        DECL_PROP1(NAME,            ::rtl::OUString,    BOUND);
        DECL_PROP2(CLASSID,         sal_Int16,          READONLY, TRANSIENT);
        DECL_PROP2(DEFAULT_TEXT,    ::rtl::OUString,    BOUND, MAYBEDEFAULT);
        DECL_BOOL_PROP1(EMPTY_IS_NULL,                  BOUND);
        DECL_PROP1(TAG,             ::rtl::OUString,    BOUND);
        DECL_PROP1(TABINDEX,        sal_Int16,          BOUND);
        DECL_PROP1(CONTROLSOURCE,   ::rtl::OUString,    BOUND);
        DECL_IFACE_PROP3(BOUNDFIELD,    XPropertySet,   BOUND,READONLY, TRANSIENT);
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
::rtl::OUString SAL_CALL OPatternModel::getServiceName() throw ( ::com::sun::star::uno::RuntimeException)
{
    return FRM_COMPONENT_PATTERNFIELD;  // old (non-sun) name for compatibility !
}

//------------------------------------------------------------------------------
sal_Bool OPatternModel::commitControlValueToDbColumn( bool _bPostReset )
{
    ::rtl::OUString sNewValue;
    m_xAggregateFastSet->getFastPropertyValue( getValuePropertyAggHandle() ) >>= sNewValue;

    if ( sNewValue != m_aSaveValue )
    {
        if ( !sNewValue.getLength() && !m_bRequired && m_bEmptyIsNull )
            m_xColumnUpdate->updateNull();
        else
        {
            try
            {
                m_xColumnUpdate->updateString( sNewValue );
            }
            catch(Exception&)
            {
                return sal_False;
            }
        }
        m_aSaveValue = sNewValue;
    }
    return sal_True;
}

// XPropertyChangeListener
//------------------------------------------------------------------------------
Any OPatternModel::translateDbColumnToControlValue()
{
    m_aSaveValue = m_xColumn->getString();
    return makeAny( m_aSaveValue );
}

// XReset
//------------------------------------------------------------------------------
Any OPatternModel::getDefaultForReset() const
{
    return makeAny( m_aDefaultText );
}

//.........................................................................
}   // namespace frm
//.........................................................................

