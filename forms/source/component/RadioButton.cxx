/*************************************************************************
 *
 *  $RCSfile: RadioButton.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: fs $ $Date: 2001-04-02 10:28:06 $
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

#ifndef _FORMS_RADIOBUTTON_HXX_
#include "RadioButton.hxx"
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
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XVCLWINDOWPEER_HPP_
#include <com/sun/star/awt/XVclWindowPeer.hpp>
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

//==================================================================
//------------------------------------------------------------------------------
InterfaceRef SAL_CALL ORadioButtonControl_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory) throw (RuntimeException)
{
    return *(new ORadioButtonControl(_rxFactory));
}

//------------------------------------------------------------------------------
StringSequence SAL_CALL ORadioButtonControl::getSupportedServiceNames() throw(RuntimeException)
{
    StringSequence aSupported = OBoundControl::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 1);

    ::rtl::OUString* pArray = aSupported.getArray();
    pArray[aSupported.getLength()-1] = FRM_SUN_CONTROL_RADIOBUTTON;
    return aSupported;
}


//------------------------------------------------------------------
ORadioButtonControl::ORadioButtonControl(const Reference<XMultiServiceFactory>& _rxFactory)
                      :OBoundControl(_rxFactory, VCL_CONTROL_RADIOBUTTON)
{
}

//------------------------------------------------------------------
void SAL_CALL ORadioButtonControl::createPeer(const Reference<starawt::XToolkit>& _rxToolkit, const Reference<starawt::XWindowPeer>& _rxParent) throw (RuntimeException)
{
    OBoundControl::createPeer(_rxToolkit, _rxParent);

    // switch off the auto-toggle, we do this ourself ....
    // (formerly this switch-off was done in the toolkit - but the correct place is here ...)
    Reference< XVclWindowPeer >  xVclWindowPeer( getPeer(), UNO_QUERY );
    if (xVclWindowPeer.is())
        xVclWindowPeer->setProperty(::rtl::OUString::createFromAscii("AutoToggle"), ::cppu::bool2any(sal_False));
}

//==================================================================
InterfaceRef SAL_CALL ORadioButtonModel_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory) throw (RuntimeException)
{
    return *(new ORadioButtonModel(_rxFactory));
}

//------------------------------------------------------------------
ORadioButtonModel::ORadioButtonModel(const Reference<XMultiServiceFactory>& _rxFactory)
                    :OBoundControlModel(_rxFactory, VCL_CONTROLMODEL_RADIOBUTTON, FRM_CONTROL_RADIOBUTTON, sal_False)
                                    // use the old control name for compytibility reasons
                    ,OPropertyChangeListener(m_aMutex)
                    ,m_bInReset(sal_False)
{
    m_nClassId = FormComponentType::RADIOBUTTON;
    m_nDefaultChecked = RB_NOCHECK;
    m_aLabelServiceName = FRM_SUN_COMPONENT_GROUPBOX;
    m_sDataFieldConnectivityProperty = PROPERTY_STATE;


    increment(m_refCount);
    if (m_xAggregateSet.is())
    {
        OPropertyChangeMultiplexer* pMultiplexer = new OPropertyChangeMultiplexer(this, m_xAggregateSet);
        pMultiplexer->addProperty(PROPERTY_STATE);
    }
    decrement(m_refCount);
}

// XServiceInfo
//------------------------------------------------------------------------------
StringSequence SAL_CALL ORadioButtonModel::getSupportedServiceNames() throw(RuntimeException)
{
    StringSequence aSupported = OBoundControlModel::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 2);

    ::rtl::OUString* pArray = aSupported.getArray();
    pArray[aSupported.getLength()-2] = ::rtl::OUString::createFromAscii("com.sun.star.form.component.DatabaseRadioButton");
    pArray[aSupported.getLength()-1] = FRM_SUN_COMPONENT_RADIOBUTTON;
    return aSupported;
}

//------------------------------------------------------------------------------
void ORadioButtonModel::getFastPropertyValue(Any& rValue, sal_Int32 nHandle) const
{
    switch (nHandle)
    {
        case PROPERTY_ID_REFVALUE : rValue <<= m_sReferenceValue; break;
        case PROPERTY_ID_DEFAULTCHECKED : rValue <<= m_nDefaultChecked; break;
        default:
            OBoundControlModel::getFastPropertyValue(rValue, nHandle);
    }
}

//------------------------------------------------------------------------------
void ORadioButtonModel::SetSiblingPropsTo(const ::rtl::OUString& rPropName, const Any& rValue)
{
    // mein Name
    ::rtl::OUString sMyName(m_aName);

    // meine Siblings durchiterieren
    Reference<XIndexAccess> xIndexAccess(getParent(), UNO_QUERY);
    if (xIndexAccess.is())
    {
        Reference<XPropertySet> xMyProps;
        query_interface(static_cast<XWeak*>(this), xMyProps);
        ::rtl::OUString sCurrentName;
        for (sal_Int32 i=0; i<xIndexAccess->getCount(); ++i)
        {
            Reference<XPropertySet> xSiblingProperties(*(InterfaceRef*)xIndexAccess->getByIndex(i).getValue(), UNO_QUERY);
            if (!xSiblingProperties.is())
                continue;
            if (xMyProps == xSiblingProperties)
                continue;   // mich selber nicht umsetzen

            // nur wenn es ein Radio-Button ist
            if (!hasProperty(PROPERTY_CLASSID, xSiblingProperties))
                continue;
            sal_Int16 nType;
            xSiblingProperties->getPropertyValue(PROPERTY_CLASSID) >>= nType;
            if (nType != FormComponentType::RADIOBUTTON)
                continue;

            // das 'zur selben Gruppe gehoeren' wird am Namen festgemacht
            xSiblingProperties->getPropertyValue(PROPERTY_NAME) >>= sCurrentName;
            if (sCurrentName == sMyName)
                xSiblingProperties->setPropertyValue(rPropName, rValue);
        }
    }
}

//------------------------------------------------------------------------------
void ORadioButtonModel::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const Any& rValue) throw (Exception)
{
    switch (nHandle)
    {
        case PROPERTY_ID_REFVALUE :
            DBG_ASSERT(rValue.getValueType().getTypeClass() == TypeClass_STRING, "ORadioButtonModel::setFastPropertyValue_NoBroadcast : invalid type !" );
            rValue >>= m_sReferenceValue;
            break;

        case PROPERTY_ID_DEFAULTCHECKED :
            DBG_ASSERT(rValue.getValueType().getTypeClass() == TypeClass_SHORT, "ORadioButtonModel::setFastPropertyValue_NoBroadcast : invalid type !" );
            rValue >>= (sal_Int16)m_nDefaultChecked;
            _reset();
            break;

        default:
            OBoundControlModel::setFastPropertyValue_NoBroadcast(nHandle, rValue);
    }

    // if the label control changed ...
    if (nHandle == PROPERTY_ID_CONTROLLABEL)
    {   // ... forward this to our siblings
        SetSiblingPropsTo(PROPERTY_CONTROLLABEL, rValue);
    }

    // wenn sich die ControlSource-Eigenschaft geaendert hat ...
    if (nHandle == PROPERTY_ID_CONTROLSOURCE)
    {   // ... muss ich allen meinen Siblings, die in der selben RadioButton-Gruppe sind wie ich, auch die
        // neue ControlSource mitgeben
        SetSiblingPropsTo(PROPERTY_CONTROLSOURCE, rValue);
    }

    // die andere Richtung : wenn sich mein Name aendert ...
    if (nHandle == PROPERTY_ID_NAME)
    {
        // ... muss ich testen, ob ich Siblings mit dem selben Namen habe, damit ich deren ControlSource uebernehmen kann
        Reference<XIndexAccess> xIndexAccess(getParent(), UNO_QUERY);
        if (xIndexAccess.is())
        {
            ::rtl::OUString         sName;
            ::rtl::OUString         sControlSource;

            Reference<XPropertySet> xMyProps;
            query_interface(static_cast<XWeak*>(this), xMyProps);
            for (sal_Int32 i=0; i<xIndexAccess->getCount(); ++i)
            {
                Reference<XPropertySet> xSiblingProperties(*(InterfaceRef*)xIndexAccess->getByIndex(i).getValue(), UNO_QUERY);
                if (!xSiblingProperties.is())
                    continue;

                if (xMyProps == xSiblingProperties)
                    // nur wenn ich nicht mich selber gefunden habe
                    continue;

                sal_Int16 nType;
                xSiblingProperties->getPropertyValue(PROPERTY_CLASSID) >>= nType;
                if (nType != FormComponentType::RADIOBUTTON)
                    // nur Radio-Buttons
                    continue;

                xSiblingProperties->getPropertyValue(PROPERTY_NAME) >>= sName;
                // Control, das zur gleichen Gruppe gehoert ?
                if (rValue == sName)
                {
                    setPropertyValue(PROPERTY_CONTROLSOURCE, xSiblingProperties->getPropertyValue(PROPERTY_CONTROLSOURCE));
                    break;
                }
            }
        }
    }

    if (nHandle == PROPERTY_ID_DEFAULTCHECKED)
    {
        sal_Int16 nValue;
        rValue >>= nValue;
        if (1 == nValue)
        {   // bei allen Radios der selben Gruppe das 'default checked' ruecksetzen, denn wie schon der highlander wusste :
            // es kann nur einen geben.
            Any aZero;
            nValue = 0;
            aZero <<= nValue;
            SetSiblingPropsTo(PROPERTY_DEFAULTCHECKED, aZero);
        }
    }
}

//------------------------------------------------------------------------------
sal_Bool ORadioButtonModel::convertFastPropertyValue(
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
Reference<XPropertySetInfo> SAL_CALL ORadioButtonModel::getPropertySetInfo() throw(RuntimeException)
{
    Reference<XPropertySetInfo> xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//------------------------------------------------------------------------------
cppu::IPropertyArrayHelper& ORadioButtonModel::getInfoHelper()
{
    return *const_cast<ORadioButtonModel*>(this)->getArrayHelper();
}

//------------------------------------------------------------------------------
void ORadioButtonModel::fillProperties(
        Sequence< Property >& _rProps,
        Sequence< Property >& _rAggregateProps ) const
{
    FRM_BEGIN_PROP_HELPER(10)
        // the "State" property is transient, so change this
        //  ModifyPropertyAttributes(_rAggregateProps, PROPERTY_STATE, PropertyAttribute::TRANSIENT, 0);

        DECL_PROP2(CLASSID,             sal_Int16,                  READONLY, TRANSIENT);
        DECL_PROP1(REFVALUE,            ::rtl::OUString,            BOUND);
        DECL_PROP1(DEFAULTCHECKED,      sal_Int16,                  BOUND);
        DECL_PROP1(NAME,                ::rtl::OUString,            BOUND);
        DECL_PROP1(TAG,                 ::rtl::OUString,            BOUND);
        DECL_PROP1(TABINDEX,            sal_Int16,                  BOUND);
        DECL_PROP1(CONTROLSOURCE,       rtl::OUString,              BOUND);
        DECL_IFACE_PROP2(BOUNDFIELD,    XPropertySet,   READONLY, TRANSIENT);
        DECL_IFACE_PROP2(CONTROLLABEL,  XPropertySet,   BOUND, MAYBEVOID);
        DECL_PROP2(CONTROLSOURCEPROPERTY,   rtl::OUString,  READONLY, TRANSIENT);
    FRM_END_PROP_HELPER();
}

//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL ORadioButtonModel::getServiceName() throw(RuntimeException)
{
    return FRM_COMPONENT_RADIOBUTTON;   // old (non-sun) name for compatibility !
}

//------------------------------------------------------------------------------
void SAL_CALL ORadioButtonModel::write(const Reference<XObjectOutputStream>& _rxOutStream)
    throw(IOException, RuntimeException)
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
void SAL_CALL ORadioButtonModel::read(const Reference<XObjectInputStream>& _rxInStream) throw(IOException, RuntimeException)
{
    OBoundControlModel::read(_rxInStream);
    ::osl::MutexGuard aGuard(m_aMutex);

    // Version
    UINT16 nVersion = _rxInStream->readShort();

    switch (nVersion)
    {
        case 0x0001 : _rxInStream >> m_sReferenceValue; _rxInStream >> (sal_Int16)m_nDefaultChecked; break;
        case 0x0002 :
            _rxInStream >> m_sReferenceValue;
            _rxInStream >> (sal_Int16)m_nDefaultChecked;
            readHelpTextCompatibly(_rxInStream);
            break;
        case 0x0003 :
            _rxInStream >> m_sReferenceValue;
            _rxInStream >> (sal_Int16)m_nDefaultChecked;
            readHelpTextCompatibly(_rxInStream);
            readCommonProperties(_rxInStream);
            break;
        default :
            DBG_ERROR("ORadioButtonModel::read : unknown version !");
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
void ORadioButtonModel::_propertyChanged(const PropertyChangeEvent& _rEvent) throw(RuntimeException)
{
    if (_rEvent.PropertyName.equals(PROPERTY_STATE))
    {
        if (_rEvent.NewValue == (sal_Int16)1)
        {
            // wenn sich mein Status auf 'checked' geaendert hat, muss ich alle meine Siblings, die in der selben Gruppe
            // sind wie ich, entsprechend zuruecksetzen
            Any aZero;
            aZero <<= (sal_Int16)0;

            SetSiblingPropsTo(PROPERTY_STATE, aZero);
            ::osl::MutexGuard aGuard(m_aMutex);

            // as we aren't commitable we have to take care of the field we are bound to ourself
            if (m_xField.is() && !m_bInReset)
            {
                m_xField->setPropertyValue(PROPERTY_VALUE, makeAny(m_sReferenceValue));
            }
        }
    }
}

//------------------------------------------------------------------------------
void ORadioButtonModel::_onValueChanged()
{
    Any aValue;
    aValue <<= (sal_Int16)((m_xColumn->getString() == m_sReferenceValue) ? RB_CHECK : RB_NOCHECK);
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

//------------------------------------------------------------------------------
Any ORadioButtonModel::_getControlValue() const
{
    return m_xAggregateSet->getPropertyValue(PROPERTY_STATE);
}

//------------------------------------------------------------------------------
sal_Int16 ORadioButtonModel::getState( const Any& rValue )
{
    //::rtl::OUString aStrValue = DBTypeConversion::toString( rValue );
    return RB_NOCHECK;
}

//------------------------------------------------------------------------------
void ORadioButtonModel::_reset( void )
{
    Any aValue;
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
sal_Bool ORadioButtonModel::_commit()
{
    if (!m_bInReset)
        // normally we don't have a commit as we forward all state changes immediately to our field we're bound to
        return sal_True;

    // we're in reset, so this commit means "put the value into the field you're bound to"
    // 72769 - 08.02.00 - FS
    DBG_ASSERT(m_xField.is(), "ORadioButtonModel::_commit : committing while resetting, but not bound ?");
    if (m_xField.is())
    {
        try
        {
            sal_Int16 nValue;
            m_xAggregateSet->getPropertyValue(PROPERTY_STATE) >>= nValue;
            if (nValue == 1)
                m_xField->setPropertyValue(PROPERTY_VALUE, makeAny(m_sReferenceValue));
        }
        catch(Exception&)
        {
            DBG_ERROR("ORadioButtonModel::_commit : could not commit !");
        }
    }
    return sal_True;
}

//-----------------------------------------------------------------------------
void ORadioButtonModel::reset(void) throw (RuntimeException)
{
    m_bInReset = sal_True;
    OBoundControlModel::reset();
    m_bInReset = sal_False;
}

//.........................................................................
}
//.........................................................................

