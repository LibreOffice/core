/*************************************************************************
 *
 *  $RCSfile: CheckBox.cxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: obo $ $Date: 2003-10-21 08:54:43 $
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

#ifndef _FORMS_CHECKBOX_HXX_
#include "CheckBox.hxx"
#endif
#ifndef _FRM_PROPERTY_HXX_
#include "property.hxx"
#endif
#ifndef _FRM_PROPERTY_HRC_
#include "property.hrc"
#endif
#ifndef _FRM_SERVICES_HXX_
#include "services.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

//.........................................................................
namespace frm
{
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
using namespace ::drafts::com::sun::star::form;

//==================================================================
//= OCheckBoxControl
//==================================================================

//------------------------------------------------------------------
OCheckBoxControl::OCheckBoxControl(const Reference<XMultiServiceFactory>& _rxFactory)
    :OBoundControl(_rxFactory, VCL_CONTROL_CHECKBOX)
{
}

//------------------------------------------------------------------
InterfaceRef SAL_CALL OCheckBoxControl_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory) throw (RuntimeException)
{
    return *(new OCheckBoxControl(_rxFactory));
}

//------------------------------------------------------------------------------
StringSequence SAL_CALL OCheckBoxControl::getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException)
{
    StringSequence aSupported = OBoundControl::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 1);

    ::rtl::OUString* pArray = aSupported.getArray();
    pArray[aSupported.getLength()-1] = FRM_SUN_CONTROL_CHECKBOX;
    return aSupported;
}

//==================================================================
//= OCheckBoxModel
//==================================================================

//==================================================================
InterfaceRef SAL_CALL OCheckBoxModel_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory) throw (RuntimeException)
{
    return *(new OCheckBoxModel(_rxFactory));
}

//------------------------------------------------------------------
DBG_NAME( OCheckBoxModel )
//------------------------------------------------------------------
OCheckBoxModel::OCheckBoxModel(const Reference<XMultiServiceFactory>& _rxFactory)
    :OBoundControlModel( _rxFactory, VCL_CONTROLMODEL_CHECKBOX, FRM_CONTROL_CHECKBOX, sal_False, sal_True )
                    // use the old control name for compytibility reasons
{
    DBG_CTOR( OCheckBoxModel, NULL );

    m_nClassId = FormComponentType::CHECKBOX;
    m_nDefaultChecked = CB_NOCHECK;
    initValueProperty( PROPERTY_STATE, PROPERTY_ID_STATE );
}

//------------------------------------------------------------------
OCheckBoxModel::OCheckBoxModel( const OCheckBoxModel* _pOriginal, const Reference<XMultiServiceFactory>& _rxFactory )
    :OBoundControlModel( _pOriginal, _rxFactory )
{
    DBG_CTOR( OCheckBoxModel, NULL );

    m_nDefaultChecked = _pOriginal->m_nDefaultChecked;
    m_sReferenceValue = _pOriginal->m_sReferenceValue;
}

//------------------------------------------------------------------------------
OCheckBoxModel::~OCheckBoxModel()
{
    DBG_DTOR( OCheckBoxModel, NULL );
}

//------------------------------------------------------------------------------
IMPLEMENT_DEFAULT_CLONING( OCheckBoxModel )

// XServiceInfo
//------------------------------------------------------------------------------
StringSequence SAL_CALL OCheckBoxModel::getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException)
{
    StringSequence aSupported = OBoundControlModel::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 3);

    ::rtl::OUString* pArray = aSupported.getArray();
    pArray[aSupported.getLength()-3] = FRM_SUN_COMPONENT_BINDDB_CHECKBOX;
    pArray[aSupported.getLength()-2] = FRM_SUN_COMPONENT_DATABASE_CHECKBOX;
    pArray[aSupported.getLength()-1] = FRM_SUN_COMPONENT_CHECKBOX;
    return aSupported;
}

//------------------------------------------------------------------------------
void OCheckBoxModel::getFastPropertyValue(Any& _rValue, sal_Int32 _nHandle) const
{
    switch (_nHandle)
    {
        case PROPERTY_ID_REFVALUE: _rValue <<= m_sReferenceValue; break;
        case PROPERTY_ID_DEFAULTCHECKED : _rValue <<= m_nDefaultChecked; break;
        default:
            OBoundControlModel::getFastPropertyValue(_rValue, _nHandle);
    }
}

//------------------------------------------------------------------------------
void OCheckBoxModel::setFastPropertyValue_NoBroadcast(sal_Int32 _nHandle, const Any& _rValue) throw (com::sun::star::uno::Exception)
{
    switch (_nHandle)
    {
        case PROPERTY_ID_REFVALUE :
            DBG_ASSERT(_rValue.getValueType().getTypeClass() == TypeClass_STRING, "OCheckBoxModel::setFastPropertyValue_NoBroadcast : invalid type !" );
            _rValue >>= m_sReferenceValue;
            break;

        case PROPERTY_ID_DEFAULTCHECKED :
            DBG_ASSERT(_rValue.getValueType().getTypeClass() == TypeClass_SHORT, "OCheckBoxModel::setFastPropertyValue_NoBroadcast : invalid type !" );
            _rValue >>= m_nDefaultChecked;
            resetNoBroadcast();
            break;

        default:
            OBoundControlModel::setFastPropertyValue_NoBroadcast(_nHandle, _rValue);
    }
}

//------------------------------------------------------------------------------
sal_Bool OCheckBoxModel::convertFastPropertyValue(
            Any& _rConvertedValue, Any& _rOldValue, sal_Int32 _nHandle, const Any& _rValue)
            throw (IllegalArgumentException)
{
    sal_Bool bModified(sal_False);
    switch (_nHandle)
    {
        case PROPERTY_ID_REFVALUE :
            bModified = tryPropertyValue(_rConvertedValue, _rOldValue, _rValue, m_sReferenceValue);
            break;
        case PROPERTY_ID_DEFAULTCHECKED :
            bModified = tryPropertyValue(_rConvertedValue, _rOldValue, _rValue, m_nDefaultChecked);
            break;
        default:
            bModified = OBoundControlModel::convertFastPropertyValue(_rConvertedValue, _rOldValue, _nHandle, _rValue);
            break;
    }
    return bModified;
}

//------------------------------------------------------------------------------
Reference<XPropertySetInfo> SAL_CALL OCheckBoxModel::getPropertySetInfo() throw(RuntimeException)
{
    Reference<XPropertySetInfo> xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//------------------------------------------------------------------------------
cppu::IPropertyArrayHelper& OCheckBoxModel::getInfoHelper()
{
    return *const_cast<OCheckBoxModel*>(this)->getArrayHelper();
}

//------------------------------------------------------------------------------
void OCheckBoxModel::fillProperties(
        Sequence< Property >& _rProps,
        Sequence< Property >& _rAggregateProps ) const
{
    FRM_BEGIN_PROP_HELPER(10)
        // the "State" property is transient, so change this
//      ModifyPropertyAttributes(_rAggregateProps, PROPERTY_STATE, PropertyAttribute::TRANSIENT, 0);

        DECL_PROP2(CLASSID,         sal_Int16,          READONLY, TRANSIENT);
        DECL_PROP1(REFVALUE,        ::rtl::OUString,    BOUND);
        DECL_PROP1(DEFAULTCHECKED,  sal_Int16,          BOUND);
        DECL_PROP1(NAME,            rtl::OUString,      BOUND);
        DECL_PROP1(TAG,             rtl::OUString,      BOUND);
        DECL_PROP1(TABINDEX,        sal_Int16,          BOUND);
        DECL_PROP1(CONTROLSOURCE,   rtl::OUString,      BOUND);
        DECL_IFACE_PROP3(BOUNDFIELD,    XPropertySet,   BOUND,READONLY, TRANSIENT);
        DECL_IFACE_PROP2(CONTROLLABEL,  XPropertySet,   BOUND, MAYBEVOID);
        DECL_PROP2(CONTROLSOURCEPROPERTY,   rtl::OUString,  READONLY, TRANSIENT);
    FRM_END_PROP_HELPER();
}

//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OCheckBoxModel::getServiceName() throw(RuntimeException)
{
    return FRM_COMPONENT_CHECKBOX;  // old (non-sun) name for compatibility !
}

//------------------------------------------------------------------------------
void SAL_CALL OCheckBoxModel::write(const Reference<stario::XObjectOutputStream>& _rxOutStream)
    throw(stario::IOException, RuntimeException)
{
    OBoundControlModel::write(_rxOutStream);

    // Version
    _rxOutStream->writeShort(0x0003);
    // Properties
    _rxOutStream << m_sReferenceValue;
    _rxOutStream << (sal_Int16)m_nDefaultChecked;
    writeHelpTextCompatibly(_rxOutStream);
    // from version 0x0003 : common properties
    writeCommonProperties(_rxOutStream);
}

//------------------------------------------------------------------------------
void SAL_CALL OCheckBoxModel::read(const Reference<stario::XObjectInputStream>& _rxInStream) throw(stario::IOException, RuntimeException)
{
    OBoundControlModel::read(_rxInStream);
    osl::MutexGuard aGuard(m_aMutex);

    // Version
    sal_uInt16 nVersion = _rxInStream->readShort();

    switch (nVersion)
    {
        case 0x0001 : _rxInStream >> m_sReferenceValue; m_nDefaultChecked = _rxInStream->readShort(); break;
        case 0x0002 :
            _rxInStream >> m_sReferenceValue;
            _rxInStream >> m_nDefaultChecked;
            readHelpTextCompatibly(_rxInStream);
            break;
        case 0x0003 :
            _rxInStream >> m_sReferenceValue;
            _rxInStream >> m_nDefaultChecked;
            readHelpTextCompatibly(_rxInStream);
            readCommonProperties(_rxInStream);
            break;
        default :
            DBG_ERROR("OCheckBoxModel::read : unknown version !");
            m_sReferenceValue = ::rtl::OUString();
            m_nDefaultChecked = 0;
            defaultCommonProperties();
            break;
    }

    // Nach dem Lesen die Defaultwerte anzeigen
    if (m_aControlSource.getLength())
        // (not if we don't have a control source - the "State" property acts like it is persistent, then
        resetNoBroadcast();
}

//------------------------------------------------------------------------------
void OCheckBoxModel::onConnectedDbColumn( const Reference< XInterface >& _rxForm )
{
    OBoundControlModel::onConnectedDbColumn( _rxForm );
}

//------------------------------------------------------------------------------
Any OCheckBoxModel::translateDbColumnToControlValue()
{
    Any aValue;

    //////////////////////////////////////////////////////////////////
    // Wert an ControlModel setzen
    sal_Bool bValue = m_xColumn->getBoolean();
    if ( m_xColumn->wasNull() )
    {
        sal_Bool bTriState = sal_True;
        if ( m_xAggregateSet.is() )
            m_xAggregateSet->getPropertyValue( PROPERTY_TRISTATE ) >>= bTriState;
        aValue <<= (sal_Int16)( bTriState ? CB_DONTKNOW : m_nDefaultChecked );
    }
    else
        aValue <<= ( bValue ? (sal_Int16)CB_CHECK : (sal_Int16)CB_NOCHECK );

    return aValue;
}

//------------------------------------------------------------------------------
Any OCheckBoxModel::getDefaultForReset() const
{
    return makeAny( (sal_Int16)m_nDefaultChecked );
}

//-----------------------------------------------------------------------------
sal_Bool OCheckBoxModel::commitControlValueToDbColumn( bool _bPostReset )
{
    OSL_PRECOND( m_xColumnUpdate.is(), "OCheckBoxModel::commitControlValueToDbColumn: not bound!" );
    if ( m_xColumnUpdate.is() )
    {
        Any aControlValue( m_xAggregateSet->getPropertyValue( PROPERTY_STATE ) );
        try
        {
            sal_Int16 nValue = CB_DONTKNOW;
            aControlValue >>= nValue;
            switch (nValue)
            {
                case CB_DONTKNOW:
                    m_xColumnUpdate->updateNull();
                    break;
                case CB_CHECK:
                    m_xColumnUpdate->updateBoolean( sal_True );
                    break;
                case CB_NOCHECK:
                    m_xColumnUpdate->updateBoolean( sal_False );
                    break;
                default:
                    DBG_ERROR("OCheckBoxModel::commitControlValueToDbColumn: invalid value !");
            }
        }
        catch(Exception&)
        {
            DBG_ERROR("OCheckBoxModel::commitControlValueToDbColumn: could not commit !");
        }
    }
    return sal_True;
}

//-----------------------------------------------------------------------------
sal_Bool OCheckBoxModel::approveValueBinding( const Reference< XValueBinding >& _rxBinding )
{
    OSL_PRECOND( _rxBinding.is(), "OCheckBoxModel::approveValueBinding: invalid binding!" );

    // only strings are accepted for simplicity
    return  _rxBinding.is()
        &&  _rxBinding->supportsType( ::getCppuType( static_cast< sal_Bool* >( NULL ) ) );
}

//-----------------------------------------------------------------------------
Any OCheckBoxModel::translateExternalValueToControlValue( )
{
    OSL_PRECOND( m_xExternalBinding.is(), "OCheckBoxModel::commitControlValueToExternalBinding: no active binding!" );

    sal_Int16 nState = CB_DONTKNOW;
    if ( m_xExternalBinding.is() )
    {
        Any aExternalValue;
        try
        {
            aExternalValue = m_xExternalBinding->getValue( ::getCppuType( static_cast< sal_Bool* >( NULL ) ) );
        }
        catch( const IncompatibleTypesException& )
        {
            OSL_ENSURE( sal_False, "OCheckBoxModel::translateExternalValueToControlValue: caught an exception!" );
        }

        sal_Bool bState = sal_False;
        if ( aExternalValue >>= bState )
            nState = bState ? CB_CHECK : CB_NOCHECK;
    }

    return makeAny( nState );
}

//-----------------------------------------------------------------------------
Any OCheckBoxModel::translateControlValueToExternalValue( )
{
    // translate the control value into a value appropriate for the external binding
    // Basically, this means translating the INT16-State into a boolean
    Any aControlValue( m_xAggregateSet->getPropertyValue( PROPERTY_STATE ) );
    Any aExternalValue;

    sal_Int16 nControlValue = CB_DONTKNOW;
    aControlValue >>= nControlValue;

    switch( nControlValue )
    {
        case CB_CHECK:
            aExternalValue <<= (sal_Bool)sal_True;
            break;
        case CB_NOCHECK:
            aExternalValue <<= (sal_Bool)sal_False;
            break;
    }
    return aExternalValue;
}

//.........................................................................
}
//.........................................................................

