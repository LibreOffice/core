/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: CheckBox.cxx,v $
 * $Revision: 1.18 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_forms.hxx"
#include "CheckBox.hxx"
#include "property.hxx"
#ifndef _FRM_PROPERTY_HRC_
#include "property.hrc"
#endif
#include "services.hxx"
#include <tools/debug.hxx>
#include <comphelper/basicio.hxx>

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
using namespace ::com::sun::star::form::binding;

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
    :OReferenceValueComponent( _rxFactory, VCL_CONTROLMODEL_CHECKBOX, FRM_SUN_CONTROL_CHECKBOX, sal_True )
                    // use the old control name for compytibility reasons
{
    DBG_CTOR( OCheckBoxModel, NULL );

    m_nClassId = FormComponentType::CHECKBOX;
    initValueProperty( PROPERTY_STATE, PROPERTY_ID_STATE );
}

//------------------------------------------------------------------
OCheckBoxModel::OCheckBoxModel( const OCheckBoxModel* _pOriginal, const Reference<XMultiServiceFactory>& _rxFactory )
    :OReferenceValueComponent( _pOriginal, _rxFactory )
{
    DBG_CTOR( OCheckBoxModel, NULL );
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
    StringSequence aSupported = OReferenceValueComponent::getSupportedServiceNames();

    sal_Int32 nOldLen = aSupported.getLength();
    aSupported.realloc( nOldLen + 8 );
    ::rtl::OUString* pStoreTo = aSupported.getArray() + nOldLen;

    *pStoreTo++ = BINDABLE_CONTROL_MODEL;
    *pStoreTo++ = DATA_AWARE_CONTROL_MODEL;
    *pStoreTo++ = VALIDATABLE_CONTROL_MODEL;

    *pStoreTo++ = BINDABLE_DATA_AWARE_CONTROL_MODEL;
    *pStoreTo++ = VALIDATABLE_BINDABLE_CONTROL_MODEL;

    *pStoreTo++ = FRM_SUN_COMPONENT_CHECKBOX;
    *pStoreTo++ = FRM_SUN_COMPONENT_DATABASE_CHECKBOX;
    *pStoreTo++ = BINDABLE_DATABASE_CHECK_BOX;

    return aSupported;
}

//------------------------------------------------------------------------------
void OCheckBoxModel::describeFixedProperties( Sequence< Property >& _rProps ) const
{
    BEGIN_DESCRIBE_PROPERTIES( 1, OReferenceValueComponent )
        DECL_PROP1(TABINDEX,        sal_Int16,          BOUND);
    END_DESCRIBE_PROPERTIES();
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
    OReferenceValueComponent::write(_rxOutStream);

    // Version
    _rxOutStream->writeShort(0x0003);
    // Properties
    _rxOutStream << getReferenceValue();
    _rxOutStream << (sal_Int16)getDefaultChecked();
    writeHelpTextCompatibly(_rxOutStream);
    // from version 0x0003 : common properties
    writeCommonProperties(_rxOutStream);
}

//------------------------------------------------------------------------------
void SAL_CALL OCheckBoxModel::read(const Reference<stario::XObjectInputStream>& _rxInStream) throw(stario::IOException, RuntimeException)
{
    OReferenceValueComponent::read(_rxInStream);
    osl::MutexGuard aGuard(m_aMutex);

    // Version
    sal_uInt16 nVersion = _rxInStream->readShort();

    ::rtl::OUString sReferenceValue;
    sal_Int16       nDefaultChecked( 0 );
    switch ( nVersion )
    {
        case 0x0001:
            _rxInStream >> sReferenceValue;
            nDefaultChecked = _rxInStream->readShort();
            break;
        case 0x0002:
            _rxInStream >> sReferenceValue;
            _rxInStream >> nDefaultChecked;
            readHelpTextCompatibly( _rxInStream );
            break;
        case 0x0003:
            _rxInStream >> sReferenceValue;
            _rxInStream >> nDefaultChecked;
            readHelpTextCompatibly(_rxInStream);
            readCommonProperties(_rxInStream);
            break;
        default:
            DBG_ERROR("OCheckBoxModel::read : unknown version !");
            defaultCommonProperties();
            break;
    }
    setReferenceValue( sReferenceValue );
    setDefaultChecked( static_cast< CheckState >( nDefaultChecked ) );

    // Nach dem Lesen die Defaultwerte anzeigen
    if ( getControlSource().getLength() )
        // (not if we don't have a control source - the "State" property acts like it is persistent, then
        resetNoBroadcast();
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
        aValue <<= (sal_Int16)( bTriState ? STATE_DONTKNOW : getDefaultChecked() );
    }
    else
        aValue <<= (sal_Int16)( bValue ? STATE_CHECK : STATE_NOCHECK );

    return aValue;
}

//-----------------------------------------------------------------------------
sal_Bool OCheckBoxModel::commitControlValueToDbColumn( bool /*_bPostReset*/ )
{
    OSL_PRECOND( m_xColumnUpdate.is(), "OCheckBoxModel::commitControlValueToDbColumn: not bound!" );
    if ( m_xColumnUpdate.is() )
    {
        Any aControlValue( m_xAggregateSet->getPropertyValue( PROPERTY_STATE ) );
        try
        {
            sal_Int16 nValue = STATE_DONTKNOW;
            aControlValue >>= nValue;
            switch (nValue)
            {
                case STATE_DONTKNOW:
                    m_xColumnUpdate->updateNull();
                    break;
                case STATE_CHECK:
                    m_xColumnUpdate->updateBoolean( sal_True );
                    break;
                case STATE_NOCHECK:
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

//.........................................................................
}
//.........................................................................

