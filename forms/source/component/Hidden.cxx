/*************************************************************************
 *
 *  $RCSfile: Hidden.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:29:05 $
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

#ifndef _FORMS_HIDDEN_HXX_
#include "Hidden.hxx"
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

//------------------------------------------------------------------
InterfaceRef SAL_CALL OHiddenModel_CreateInstance(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory) throw (staruno::RuntimeException)
{
    return *(new OHiddenModel(_rxFactory));
}

//------------------------------------------------------------------
OHiddenModel::OHiddenModel(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory)
    :OControlModel(_rxFactory, ::rtl::OUString())
{
    m_nClassId = starform::FormComponentType::HIDDENCONTROL;
}

//------------------------------------------------------------------------------
void OHiddenModel::getFastPropertyValue(staruno::Any& _rValue, sal_Int32 _nHandle) const
{
    switch (_nHandle)
    {
        case PROPERTY_ID_HIDDEN_VALUE : _rValue <<= m_sHiddenValue; break;
        default:
            OControlModel::getFastPropertyValue(_rValue, _nHandle);
    }
}

//------------------------------------------------------------------------------
void OHiddenModel::setFastPropertyValue_NoBroadcast(sal_Int32 _nHandle, const staruno::Any& _rValue) throw (com::sun::star::uno::Exception)
{
    switch (_nHandle)
    {
        case PROPERTY_ID_HIDDEN_VALUE :
            DBG_ASSERT(_rValue.getValueType().getTypeClass() == staruno::TypeClass_STRING, "OHiddenModel::setFastPropertyValue_NoBroadcast : invalid type !" );
            _rValue >>= m_sHiddenValue;
            break;
        default:
            OControlModel::setFastPropertyValue_NoBroadcast(_nHandle, _rValue);
    }
}

//------------------------------------------------------------------------------
sal_Bool OHiddenModel::convertFastPropertyValue(
            staruno::Any& _rConvertedValue, staruno::Any& _rOldValue, sal_Int32 _nHandle, const staruno::Any& _rValue)
            throw (starlang::IllegalArgumentException)
{
    sal_Bool bModified(sal_False);
    switch (_nHandle)
    {
        case PROPERTY_ID_HIDDEN_VALUE :
            bModified = tryPropertyValue(_rConvertedValue, _rOldValue, _rValue, m_sHiddenValue);
            break;
        default:
            bModified = OControlModel::convertFastPropertyValue(_rConvertedValue, _rOldValue, _nHandle, _rValue);
            break;
    }
    return bModified;
}

//------------------------------------------------------------------------------
staruno::Reference<starbeans::XPropertySetInfo> SAL_CALL OHiddenModel::getPropertySetInfo() throw(staruno::RuntimeException)
{
    staruno::Reference<starbeans::XPropertySetInfo> xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//------------------------------------------------------------------------------
cppu::IPropertyArrayHelper& OHiddenModel::getInfoHelper()
{
    return *const_cast<OHiddenModel*>(this)->getArrayHelper();
}

//------------------------------------------------------------------------------
void OHiddenModel::fillProperties(
        staruno::Sequence< starbeans::Property >& _rProps,
        staruno::Sequence< starbeans::Property >& _rAggregateProps ) const
{
    BEGIN_AGGREGATION_PROPERTY_HELPER(4, m_xAggregateSet)
        DECL_PROP2(CLASSID,         sal_Int16,          READONLY, TRANSIENT);
        DECL_PROP1(HIDDEN_VALUE,    ::rtl::OUString,    BOUND);
        DECL_PROP1(NAME,            ::rtl::OUString,    BOUND);
        DECL_PROP1(TAG,             ::rtl::OUString,    BOUND);
    END_AGGREGATION_PROPERTY_HELPER();
}

// XServiceInfo
//------------------------------------------------------------------------------
StringSequence SAL_CALL OHiddenModel::getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException)
{
    StringSequence aSupported = OControlModel::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 2);

    ::rtl::OUString* pArray = aSupported.getArray();
    pArray[aSupported.getLength()-2] = ::rtl::OUString::createFromAscii("com.sun.star.form.FormComponent");
    pArray[aSupported.getLength()-1] = FRM_SUN_COMPONENT_HIDDENCONTROL;
    return aSupported;
}

//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OHiddenModel::getServiceName() throw(staruno::RuntimeException)
{
    return FRM_COMPONENT_HIDDEN;    // old (non-sun) name for compatibility !
}

//------------------------------------------------------------------------------
void SAL_CALL OHiddenModel::write(const staruno::Reference<stario::XObjectOutputStream>& _rxOutStream)
    throw(stario::IOException, staruno::RuntimeException)
{
    // Version
    _rxOutStream->writeShort(0x0002);

    // Wert
    _rxOutStream << m_sHiddenValue;

    OControlModel::write(_rxOutStream);
}

//------------------------------------------------------------------------------
void SAL_CALL OHiddenModel::read(const staruno::Reference<stario::XObjectInputStream>& _rxInStream) throw(stario::IOException, staruno::RuntimeException)
{
    // Version
    UINT16 nVersion = _rxInStream->readShort();

    // Name
    DBG_ASSERT(nVersion != 1, "OHiddenModel::read : this version is obsolete !");
    switch (nVersion)
    {
        case 1 : { ::rtl::OUString sDummy; _rxInStream >> sDummy; _rxInStream >> m_sHiddenValue; } break;
        case 2 : _rxInStream >> m_sHiddenValue; break;
        default : DBG_ERROR("OHiddenModel::read : unknown version !"); m_sHiddenValue = ::rtl::OUString();
    }
    OControlModel::read(_rxInStream);
}

//.........................................................................
}
//.........................................................................

