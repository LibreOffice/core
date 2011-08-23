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

#ifndef _FORMS_CHECKBOX_HXX_
#include "CheckBox.hxx"
#endif
#ifndef _FRM_PROPERTY_HRC_
#include "property.hrc"
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

#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif

namespace binfilter {

//.........................................................................
namespace frm
{
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
StringSequence SAL_CALL	OCheckBoxControl::getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException)
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
    :OBoundControlModel(_rxFactory, VCL_CONTROLMODEL_CHECKBOX, FRM_CONTROL_CHECKBOX, sal_False, sal_False)
                    // use the old control name for compytibility reasons
    ,OPropertyChangeListener(m_aMutex)
    ,m_bInReset(sal_False)
    ,m_pAggregatePropertyMultiplexer(NULL)
{
    DBG_CTOR( OCheckBoxModel, NULL );
    implConstruct();

    m_nClassId = FormComponentType::CHECKBOX;
    m_nDefaultChecked = CB_NOCHECK;
    m_sDataFieldConnectivityProperty = PROPERTY_STATE;
}

//------------------------------------------------------------------
OCheckBoxModel::OCheckBoxModel( const OCheckBoxModel* _pOriginal, const Reference<XMultiServiceFactory>& _rxFactory )
    :OBoundControlModel( _pOriginal, _rxFactory, sal_False, sal_False )
    ,OPropertyChangeListener( m_aMutex )
    ,m_bInReset( sal_False )
    ,m_pAggregatePropertyMultiplexer( NULL )
{
    DBG_CTOR( OCheckBoxModel, NULL );
    implConstruct();

    m_nDefaultChecked = _pOriginal->m_nDefaultChecked;
    m_sReferenceValue = _pOriginal->m_sReferenceValue;
}

//------------------------------------------------------------------------------
void OCheckBoxModel::implConstruct()
{
    increment( m_refCount );
    if ( m_xAggregateSet.is() )
    {
        m_pAggregatePropertyMultiplexer = new OPropertyChangeMultiplexer( this, m_xAggregateSet, sal_False );
        m_pAggregatePropertyMultiplexer->acquire();
        m_pAggregatePropertyMultiplexer->addProperty( PROPERTY_STATE );
    }
    decrement( m_refCount );

    doSetDelegator();
}

//------------------------------------------------------------------------------
OCheckBoxModel::~OCheckBoxModel()
{
    doResetDelegator();

    if (m_pAggregatePropertyMultiplexer)
    {
        m_pAggregatePropertyMultiplexer->dispose();
        m_pAggregatePropertyMultiplexer->release();
        m_pAggregatePropertyMultiplexer = NULL;
    }
    DBG_DTOR( OCheckBoxModel, NULL );
}

//------------------------------------------------------------------------------
IMPLEMENT_DEFAULT_CLONING( OCheckBoxModel )

//------------------------------------------------------------------------------
void SAL_CALL OCheckBoxModel::disposing()
{
    if (m_pAggregatePropertyMultiplexer)
        m_pAggregatePropertyMultiplexer->dispose();

    OBoundControlModel::disposing();
}

//------------------------------------------------------------------------------
void OCheckBoxModel::_propertyChanged(const PropertyChangeEvent& _rEvent) throw(RuntimeException)
{
    // as we aren't commitable we have to take care of the field we are bound to ourself
    osl::MutexGuard aGuard(m_aMutex);
    if (getField().is() && !m_bInReset)
    {

        sal_uInt16 nState;
        _rEvent.NewValue >>= nState;
        switch (nState)
        {
            case CB_DONTKNOW:
                m_xColumnUpdate->updateNull();
                break;
            case CB_CHECK:
                m_xColumnUpdate->updateBoolean(sal_True);
                break;
            case CB_NOCHECK:
                m_xColumnUpdate->updateBoolean(sal_False);
                break;
            default:
                DBG_ERROR("OCheckBoxModel::_commit : invalid value !");
        }
    }
}


// XServiceInfo
//------------------------------------------------------------------------------
StringSequence SAL_CALL	OCheckBoxModel::getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException)
{
    StringSequence aSupported = OBoundControlModel::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 2);

    ::rtl::OUString* pArray = aSupported.getArray();
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
            _reset();
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
//		ModifyPropertyAttributes(_rAggregateProps, PROPERTY_STATE, PropertyAttribute::TRANSIENT, 0);

        DECL_PROP2(CLASSID,			sal_Int16,			READONLY, TRANSIENT);
        DECL_PROP1(REFVALUE,		::rtl::OUString,	BOUND);
        DECL_PROP1(DEFAULTCHECKED,	sal_Int16,			BOUND);
        DECL_PROP1(NAME,			rtl::OUString,		BOUND);
        DECL_PROP1(TAG,				rtl::OUString,		BOUND);
        DECL_PROP1(TABINDEX,		sal_Int16,			BOUND);
        DECL_PROP1(CONTROLSOURCE,	rtl::OUString,		BOUND);
        DECL_IFACE_PROP3(BOUNDFIELD,	XPropertySet,	BOUND,READONLY, TRANSIENT);
        DECL_IFACE_PROP2(CONTROLLABEL,	XPropertySet,	BOUND, MAYBEVOID);
        DECL_PROP2(CONTROLSOURCEPROPERTY,	rtl::OUString,	READONLY, TRANSIENT);
    FRM_END_PROP_HELPER();
}

//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OCheckBoxModel::getServiceName() throw(RuntimeException)
{
    return FRM_COMPONENT_CHECKBOX;	// old (non-sun) name for compatibility !
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
        _reset();
}

//------------------------------------------------------------------------------
void OCheckBoxModel::_loaded(const EventObject& rEvent)
{
    OBoundControlModel::_loaded(rEvent);
}

//------------------------------------------------------------------------------
void OCheckBoxModel::_onValueChanged()
{
    //////////////////////////////////////////////////////////////////
    // Wert an ControlModel setzen
    if (m_xAggregateSet.is())
    {
        Any aValue;
        sal_Bool bValue = m_xColumn->getBoolean();
        if (m_xColumn->wasNull())
        {
            sal_Bool bTriState;
            m_xAggregateSet->getPropertyValue(PROPERTY_TRISTATE) >>= bTriState;
            aValue <<= (sal_Int16)(bTriState ? CB_DONTKNOW : m_nDefaultChecked);
        }
        else
            aValue <<= ( bValue ? (sal_Int16)CB_CHECK : (sal_Int16)CB_NOCHECK );
        m_bInReset = sal_True;
        {	// release our mutex once (it's acquired in the calling method !), as setting aggregate properties
            // may cause any uno controls belonging to us to lock the solar mutex, which is potentially dangerous with
            // our own mutex locked
            // FS - 72451 - 31.01.00
            MutexRelease aRelease(m_aMutex);
            m_xAggregateSet->setPropertyValue(PROPERTY_STATE, aValue);
        }
        m_bInReset = sal_False;
    }
}

//------------------------------------------------------------------------------
Any OCheckBoxModel::_getControlValue() const
{
    return m_xAggregateSet->getPropertyValue(PROPERTY_STATE);
}

//------------------------------------------------------------------------------
void OCheckBoxModel::_reset( void )
{
    Any aValue;
    aValue <<= (sal_Int16)m_nDefaultChecked;
    {	// release our mutex once (it's acquired in the calling method !), as setting aggregate properties
        // may cause any uno controls belonging to us to lock the solar mutex, which is potentially dangerous with
        // our own mutex locked
        // FS - 72451 - 31.01.00
        MutexRelease aRelease(m_aMutex);
        m_xAggregateSet->setPropertyValue(PROPERTY_STATE, aValue);
    }
}

//-----------------------------------------------------------------------------
sal_Bool OCheckBoxModel::_commit()
{
    if (!m_bInReset)
        // normally we don't have a commit as we forward all state changes immediately to our field we're bound to
        return sal_True;

    // we're in reset, so this commit means "put the value into the field you're bound to"
    // 72769 - 08.02.00 - FS
    DBG_ASSERT(getField().is(), "OCheckBoxModel::_commit : committing while resetting, but not bound ?");
    if (getField().is())
    {
        try
        {
            sal_Int16 nValue;
            m_xAggregateSet->getPropertyValue(PROPERTY_STATE) >>= nValue;
            switch (nValue)
            {
                case CB_DONTKNOW:
                    m_xColumnUpdate->updateNull();
                    break;
                case CB_CHECK:
                    m_xColumnUpdate->updateBoolean(sal_True);
                    break;
                case CB_NOCHECK:
                    m_xColumnUpdate->updateBoolean(sal_False);
                    break;
                default:
                    DBG_ERROR("OCheckBoxModel::_commit : invalid value !");
            }
        }
        catch(Exception&)
        {
            DBG_ERROR("OCheckBoxModel::_commit : could not commit !");
        }
    }
    return sal_True;
}

//-----------------------------------------------------------------------------
void OCheckBoxModel::reset(void) throw (com::sun::star::uno::RuntimeException)
{
    m_bInReset = sal_True;
    OBoundControlModel::reset();
    m_bInReset = sal_False;
}

//.........................................................................
}
//.........................................................................

}
