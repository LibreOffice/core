/*************************************************************************
 *
 *  $RCSfile: CheckBox.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:29:04 $
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

//==================================================================
//= OCheckBoxControl
//==================================================================

//------------------------------------------------------------------
OCheckBoxControl::OCheckBoxControl(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory)
    :OBoundControl(_rxFactory, VCL_CONTROL_CHECKBOX)
{
}

//------------------------------------------------------------------
InterfaceRef SAL_CALL OCheckBoxControl_CreateInstance(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory) throw (staruno::RuntimeException)
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
InterfaceRef SAL_CALL OCheckBoxModel_CreateInstance(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory) throw (staruno::RuntimeException)
{
    return *(new OCheckBoxModel(_rxFactory));
}

//------------------------------------------------------------------
OCheckBoxModel::OCheckBoxModel(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory)
                 :OBoundControlModel(_rxFactory, VCL_CONTROLMODEL_CHECKBOX, FRM_CONTROL_CHECKBOX, sal_False)
                                    // use the old control name for compytibility reasons
                 ,OPropertyChangeListener(m_aMutex)
                 ,m_bInReset(sal_False)
{
    m_nClassId = starform::FormComponentType::CHECKBOX;
    m_nDefaultChecked = CB_NOCHECK;
    m_sDataFieldConnectivityProperty = PROPERTY_STATE;

    increment(m_refCount);
    if (m_xAggregateSet.is())
    {
        OPropertyChangeMultiplexer* pMultiplexer = new OPropertyChangeMultiplexer(this, m_xAggregateSet);
        pMultiplexer->addProperty(PROPERTY_STATE);
    }
    decrement(m_refCount);
}

//------------------------------------------------------------------------------
void OCheckBoxModel::_propertyChanged(const starbeans::PropertyChangeEvent& _rEvent) throw(staruno::RuntimeException)
{
    // as we aren't commitable we have to take care of the field we are bound to ourself
    osl::MutexGuard aGuard(m_aMutex);
    if (m_xField.is() && !m_bInReset)
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
StringSequence SAL_CALL OCheckBoxModel::getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException)
{
    StringSequence aSupported = OBoundControlModel::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 2);

    ::rtl::OUString* pArray = aSupported.getArray();
    pArray[aSupported.getLength()-2] = ::rtl::OUString::createFromAscii("com.sun.star.form.component.DatabaseCheckBox");
    pArray[aSupported.getLength()-1] = FRM_SUN_COMPONENT_CHECKBOX;
    return aSupported;
}

//------------------------------------------------------------------------------
void OCheckBoxModel::getFastPropertyValue(staruno::Any& _rValue, sal_Int32 _nHandle) const
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
void OCheckBoxModel::setFastPropertyValue_NoBroadcast(sal_Int32 _nHandle, const staruno::Any& _rValue) throw (com::sun::star::uno::Exception)
{
    switch (_nHandle)
    {
        case PROPERTY_ID_REFVALUE :
            DBG_ASSERT(_rValue.getValueType().getTypeClass() == staruno::TypeClass_STRING, "OCheckBoxModel::setFastPropertyValue_NoBroadcast : invalid type !" );
            _rValue >>= m_sReferenceValue;
            break;

        case PROPERTY_ID_DEFAULTCHECKED :
            DBG_ASSERT(_rValue.getValueType().getTypeClass() == staruno::TypeClass_SHORT, "OCheckBoxModel::setFastPropertyValue_NoBroadcast : invalid type !" );
            _rValue >>= m_nDefaultChecked;
            _reset();
            break;

        default:
            OBoundControlModel::setFastPropertyValue_NoBroadcast(_nHandle, _rValue);
    }
}

//------------------------------------------------------------------------------
sal_Bool OCheckBoxModel::convertFastPropertyValue(
            staruno::Any& _rConvertedValue, staruno::Any& _rOldValue, sal_Int32 _nHandle, const staruno::Any& _rValue)
            throw (starlang::IllegalArgumentException)
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
staruno::Reference<starbeans::XPropertySetInfo> SAL_CALL OCheckBoxModel::getPropertySetInfo() throw(staruno::RuntimeException)
{
    staruno::Reference<starbeans::XPropertySetInfo> xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//------------------------------------------------------------------------------
cppu::IPropertyArrayHelper& OCheckBoxModel::getInfoHelper()
{
    return *const_cast<OCheckBoxModel*>(this)->getArrayHelper();
}

//------------------------------------------------------------------------------
void OCheckBoxModel::fillProperties(
        staruno::Sequence< starbeans::Property >& _rProps,
        staruno::Sequence< starbeans::Property >& _rAggregateProps ) const
{
    FRM_BEGIN_PROP_HELPER(11)
        // the "State" property is transient, so change this
//      ModifyPropertyAttributes(_rAggregateProps, PROPERTY_STATE, starbeans::PropertyAttribute::TRANSIENT, 0);

        DECL_PROP2(CLASSID,         sal_Int16,          READONLY, TRANSIENT);
        DECL_PROP1(REFVALUE,        ::rtl::OUString,    BOUND);
        DECL_PROP1(DEFAULTCHECKED,  sal_Int16,          BOUND);
        DECL_PROP1(NAME,            rtl::OUString,      BOUND);
        DECL_PROP1(TAG,             rtl::OUString,      BOUND);
        DECL_PROP1(TABINDEX,        sal_Int16,          BOUND);
        DECL_PROP1(CONTROLSOURCE,   rtl::OUString,      BOUND);
        DECL_PROP1(HELPTEXT,        rtl::OUString,      BOUND);
        DECL_IFACE_PROP2(BOUNDFIELD,    starbeans::XPropertySet,    READONLY, TRANSIENT);
        DECL_IFACE_PROP2(CONTROLLABEL,  starbeans::XPropertySet,    BOUND, MAYBEVOID);
        DECL_PROP2(CONTROLSOURCEPROPERTY,   rtl::OUString,  READONLY, TRANSIENT);
    FRM_END_PROP_HELPER();
}

//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OCheckBoxModel::getServiceName() throw(staruno::RuntimeException)
{
    return FRM_COMPONENT_CHECKBOX;  // old (non-sun) name for compatibility !
}

//------------------------------------------------------------------------------
void SAL_CALL OCheckBoxModel::write(const staruno::Reference<stario::XObjectOutputStream>& _rxOutStream)
    throw(stario::IOException, staruno::RuntimeException)
{
    OBoundControlModel::write(_rxOutStream);

    // Version
    _rxOutStream->writeShort(0x0003);
    // Properties
    _rxOutStream << m_sReferenceValue;
    _rxOutStream << (sal_Int16)m_nDefaultChecked;
    _rxOutStream << m_aHelpText;
    // from version 0x0003 : common properties
    writeCommonProperties(_rxOutStream);
}

//------------------------------------------------------------------------------
void SAL_CALL OCheckBoxModel::read(const staruno::Reference<stario::XObjectInputStream>& _rxInStream) throw(stario::IOException, staruno::RuntimeException)
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
            _rxInStream >> (sal_Int16)m_nDefaultChecked;
            _rxInStream >> m_aHelpText;
            break;
        case 0x0003 :
            _rxInStream >> m_sReferenceValue;
            _rxInStream >> (sal_Int16)m_nDefaultChecked;
            _rxInStream >> m_aHelpText;
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
void OCheckBoxModel::_loaded(const starlang::EventObject& rEvent)
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
        staruno::Any aValue;
        if (m_xColumn->getBoolean())
            aValue <<= (sal_Int16)CB_CHECK;
        else if (m_xColumn->wasNull())
        {
            sal_Bool bTriState;
            m_xAggregateSet->getPropertyValue(PROPERTY_TRISTATE) >>= bTriState;
            aValue <<= (sal_Int16)(bTriState ? CB_DONTKNOW : m_nDefaultChecked);
        }
        else
            aValue <<= (sal_Int16)CB_NOCHECK;
        m_bInReset = sal_True;
        {   // release our mutex once (it's acquired in the calling method !), as setting aggregate properties
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
staruno::Any OCheckBoxModel::_getControlValue() const
{
    return m_xAggregateSet->getPropertyValue(PROPERTY_STATE);
}

//------------------------------------------------------------------------------
void OCheckBoxModel::_reset( void )
{
    staruno::Any aValue;
    aValue <<= (sal_Int16)m_nDefaultChecked;
    {   // release our mutex once (it's acquired in the calling method !), as setting aggregate properties
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
    DBG_ASSERT(m_xField.is(), "OCheckBoxModel::_commit : committing while resetting, but not bound ?");
    if (m_xField.is())
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
        catch(...)
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

