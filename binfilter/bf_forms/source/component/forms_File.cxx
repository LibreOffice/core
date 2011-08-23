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

#ifndef _FORMS_FILE_HXX_
#include "File.hxx"
#endif

#ifndef _COM_SUN_STAR_FORM_FORMCOMPONENTTYPE_HPP_
#include <com/sun/star/form/FormComponentType.hpp>
#endif

#ifndef _FRM_PROPERTY_HRC_
#include "property.hrc"
#endif
#ifndef _FRM_SERVICES_HXX_
#include "services.hxx"
#endif

#ifndef _COMPHELPER_CONTAINER_HXX_
#include <comphelper/container.hxx>
#endif

#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
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

//------------------------------------------------------------------
InterfaceRef SAL_CALL OFileControlModel_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory)
{
    return *(new OFileControlModel(_rxFactory));
}

//------------------------------------------------------------------------------
Sequence<Type> OFileControlModel::_getTypes()
{
    static Sequence<Type> aTypes;
    if (!aTypes.getLength())
    {
        // my base class
        Sequence<Type> aBaseClassTypes = OControlModel::_getTypes();

        Sequence<Type> aOwnTypes(1);
        Type* pOwnTypes = aOwnTypes.getArray();
        pOwnTypes[0] = getCppuType((Reference<XReset>*)NULL);

        aTypes = concatSequences(aBaseClassTypes, aOwnTypes);
    }
    return aTypes;
}


// XServiceInfo
//------------------------------------------------------------------------------
StringSequence	OFileControlModel::getSupportedServiceNames() throw(RuntimeException)
{
    StringSequence aSupported = OControlModel::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 1);

    ::rtl::OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-1] = FRM_SUN_COMPONENT_FILECONTROL;
    return aSupported;
}

//------------------------------------------------------------------
DBG_NAME( OFileControlModel )
//------------------------------------------------------------------
OFileControlModel::OFileControlModel(const Reference<XMultiServiceFactory>& _rxFactory)
                    :OControlModel(_rxFactory, VCL_CONTROLMODEL_FILECONTROL)
                    ,m_aResetListeners(m_aMutex)
{
    DBG_CTOR( OFileControlModel, NULL );
    m_nClassId = FormComponentType::FILECONTROL;
}

//------------------------------------------------------------------
OFileControlModel::OFileControlModel( const OFileControlModel* _pOriginal, const Reference<XMultiServiceFactory>& _rxFactory )
    :OControlModel( _pOriginal, _rxFactory )
    ,m_aResetListeners( m_aMutex )
{
    DBG_CTOR( OFileControlModel, NULL );

    m_sDefaultValue = _pOriginal->m_sDefaultValue;
}

//------------------------------------------------------------------
OFileControlModel::~OFileControlModel()
{
    if (!OComponentHelper::rBHelper.bDisposed)
    {
        acquire();
        dispose();
    }
    DBG_DTOR( OFileControlModel, NULL );
}

//------------------------------------------------------------------------------
IMPLEMENT_DEFAULT_CLONING( OFileControlModel )

//------------------------------------------------------------------------------
Any SAL_CALL OFileControlModel::queryAggregation(const Type& _rType) throw (RuntimeException)
{
    Any aReturn = OControlModel::queryAggregation(_rType);
    if (!aReturn.hasValue())
        aReturn = ::cppu::queryInterface(_rType
            ,static_cast<XReset*>(this)
        );

    return aReturn;
}

// OComponentHelper
//-----------------------------------------------------------------------------
void OFileControlModel::disposing()
{
    OControlModel::disposing();

    EventObject aEvt(static_cast<XWeak*>(this));
    m_aResetListeners.disposeAndClear(aEvt);
}

//------------------------------------------------------------------------------
void OFileControlModel::getFastPropertyValue(Any& rValue, sal_Int32 nHandle) const
{
    switch (nHandle)
    {
        case PROPERTY_ID_DEFAULT_TEXT : rValue <<= m_sDefaultValue; break;
        default:
            OControlModel::getFastPropertyValue(rValue, nHandle);
    }
}

//------------------------------------------------------------------------------
void OFileControlModel::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const Any& rValue) throw ( ::com::sun::star::uno::Exception)
{
    switch (nHandle)
    {
        case PROPERTY_ID_DEFAULT_TEXT :
            DBG_ASSERT(rValue.getValueType().getTypeClass() == TypeClass_STRING, "OFileControlModel::setFastPropertyValue_NoBroadcast : invalid type !" );
            rValue >>= m_sDefaultValue;
            break;
        default:
            OControlModel::setFastPropertyValue_NoBroadcast(nHandle, rValue);
    }
}

//------------------------------------------------------------------------------
sal_Bool OFileControlModel::convertFastPropertyValue(Any& rConvertedValue, Any& rOldValue, sal_Int32 nHandle, const Any& rValue)
                            throw( IllegalArgumentException )
{
    switch (nHandle)
    {
        case PROPERTY_ID_DEFAULT_TEXT :
            return tryPropertyValue(rConvertedValue, rOldValue, rValue, m_sDefaultValue);
        default:
            return OControlModel::convertFastPropertyValue(rConvertedValue, rOldValue, nHandle, rValue);
    }
}

//------------------------------------------------------------------------------
Reference<XPropertySetInfo> SAL_CALL OFileControlModel::getPropertySetInfo() throw( RuntimeException )
{
    Reference<XPropertySetInfo>  xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

//------------------------------------------------------------------------------
void OFileControlModel::fillProperties(
        Sequence< Property >& _rProps,
        Sequence< Property >& _rAggregateProps ) const
{
    FRM_BEGIN_PROP_HELPER(5)
        DECL_PROP2(CLASSID,			sal_Int16,			READONLY, TRANSIENT);
        DECL_PROP1(DEFAULT_TEXT,	::rtl::OUString,	BOUND);
        DECL_PROP1(NAME,			::rtl::OUString,	BOUND);
        DECL_PROP1(TAG,				::rtl::OUString,	BOUND);
        DECL_PROP1(TABINDEX,		sal_Int16,			BOUND);

        // in den agregierten Properties muss ich noch PROPERTY_ID_TEXT auf transient setzen ...
//		ModifyPropertyAttributes(_rAggregateProps, PROPERTY_TEXT, PropertyAttribute::TRANSIENT, 0);
    FRM_END_PROP_HELPER();
}

//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& OFileControlModel::getInfoHelper()
{
    return *const_cast<OFileControlModel*>(this)->getArrayHelper();
}

//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OFileControlModel::getServiceName() throw ( ::com::sun::star::uno::RuntimeException)
{
    return FRM_COMPONENT_FILECONTROL;	// old (non-sun) name for compatibility !
}

//------------------------------------------------------------------------------
void OFileControlModel::write(const Reference<stario::XObjectOutputStream>& _rxOutStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    OControlModel::write(_rxOutStream);

    ::osl::MutexGuard aGuard(m_aMutex);

    // Version
    _rxOutStream->writeShort(0x0002);
    // Default-Wert
    _rxOutStream << m_sDefaultValue;
    writeHelpTextCompatibly(_rxOutStream);
}

//------------------------------------------------------------------------------
void OFileControlModel::read(const Reference<stario::XObjectInputStream>& _rxInStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    OControlModel::read(_rxInStream);
    ::osl::MutexGuard aGuard(m_aMutex);

    // Version
    sal_uInt16 nVersion = _rxInStream->readShort();
    // Default-Wert
    switch (nVersion)
    {
        case 1:
            _rxInStream >> m_sDefaultValue; break;
        case 2:
            _rxInStream >> m_sDefaultValue;
            readHelpTextCompatibly(_rxInStream);
            break;
        default:
            DBG_ERROR("OFileControlModel::read : unknown version !");
            m_sDefaultValue = ::rtl::OUString();
    }

    // Nach dem Lesen die Defaultwerte anzeigen
//	_reset();
}

//-----------------------------------------------------------------------------
void SAL_CALL OFileControlModel::reset() throw ( ::com::sun::star::uno::RuntimeException)
{
    ::cppu::OInterfaceIteratorHelper aIter(m_aResetListeners);
    EventObject aEvt(static_cast<XWeak*>(this));
    sal_Bool bContinue = sal_True;
    while (aIter.hasMoreElements() && bContinue)
        bContinue =((XResetListener*)aIter.next())->approveReset(aEvt);

    if (bContinue)
    {
        {
            // Wenn Models threadSave
            ::osl::MutexGuard aGuard(m_aMutex);
            _reset();
        }
        m_aResetListeners.notifyEach(&XResetListener::resetted, aEvt);
    }
}

//-----------------------------------------------------------------------------
void OFileControlModel::addResetListener(const Reference<XResetListener>& _rxListener) throw ( ::com::sun::star::uno::RuntimeException)
{
    m_aResetListeners.addInterface(_rxListener);
}

//-----------------------------------------------------------------------------
void OFileControlModel::removeResetListener(const Reference<XResetListener>& _rxListener) throw ( ::com::sun::star::uno::RuntimeException)
{
    m_aResetListeners.removeInterface(_rxListener);
}

//------------------------------------------------------------------------------
void OFileControlModel::_reset()
{
    {	// release our mutex once (it's acquired in the calling method !), as setting aggregate properties
        // may cause any uno controls belonging to us to lock the solar mutex, which is potentially dangerous with
        // our own mutex locked
        // FS - 72451 - 31.01.00
        MutexRelease aRelease(m_aMutex);
        m_xAggregateSet->setPropertyValue(PROPERTY_TEXT, makeAny(m_sDefaultValue));
    }
}

//.........................................................................
}	// namespace frm
//.........................................................................

}
