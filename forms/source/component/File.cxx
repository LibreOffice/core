/*************************************************************************
 *
 *  $RCSfile: File.cxx,v $
 *
 *  $Revision: 1.4 $
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


#ifndef _FORMS_FILE_HXX_
#include "File.hxx"
#endif

#include <com/sun/star/form/FormComponentType.hpp>

#ifndef _FRM_PROPERTY_HRC_
#include "property.hrc"
#endif
#ifndef _FRM_SERVICES_HXX_
#include "services.hxx"
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COMPHELPER_CONTAINER_HXX_
#include <comphelper/container.hxx>
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
StringSequence  OFileControlModel::getSupportedServiceNames() throw(RuntimeException)
{
    StringSequence aSupported = OControlModel::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 1);

    ::rtl::OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-1] = FRM_SUN_COMPONENT_FILECONTROL;
    return aSupported;
}

//------------------------------------------------------------------
OFileControlModel::OFileControlModel(const Reference<XMultiServiceFactory>& _rxFactory)
                    :OControlModel(_rxFactory, VCL_CONTROLMODEL_FILECONTROL)
                    ,m_aResetListeners(m_aMutex)
{
    m_nClassId = FormComponentType::FILECONTROL;
}

//------------------------------------------------------------------
OFileControlModel::~OFileControlModel()
{
    if (!OComponentHelper::rBHelper.bDisposed)
    {
        acquire();
        dispose();
    }
}

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
void OFileControlModel::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle, const Any& rValue)
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
        DECL_PROP2(CLASSID,         sal_Int16,          READONLY, TRANSIENT);
        DECL_PROP1(DEFAULT_TEXT,    ::rtl::OUString,    BOUND);
        DECL_PROP1(NAME,            ::rtl::OUString,    BOUND);
        DECL_PROP1(TAG,             ::rtl::OUString,    BOUND);
        DECL_PROP1(TABINDEX,        sal_Int16,          BOUND);

        // in den agregierten Properties muss ich noch PROPERTY_ID_TEXT auf transient setzen ...
//      ModifyPropertyAttributes(_rAggregateProps, PROPERTY_TEXT, PropertyAttribute::TRANSIENT, 0);
    FRM_END_PROP_HELPER();
}

//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& OFileControlModel::getInfoHelper()
{
    return *const_cast<OFileControlModel*>(this)->getArrayHelper();
}

//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OFileControlModel::getServiceName()
{
    return FRM_COMPONENT_FILECONTROL;   // old (non-sun) name for compatibility !
}

//------------------------------------------------------------------------------
void OFileControlModel::write(const Reference<stario::XObjectOutputStream>& _rxOutStream)
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
void OFileControlModel::read(const Reference<stario::XObjectInputStream>& _rxInStream)
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
//  _reset();
}

//-----------------------------------------------------------------------------
void SAL_CALL OFileControlModel::reset()
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
        NOTIFY_LISTENERS(m_aResetListeners, XResetListener, resetted, aEvt);
    }
}

//-----------------------------------------------------------------------------
void OFileControlModel::addResetListener(const Reference<XResetListener>& _rxListener)
{
    m_aResetListeners.addInterface(_rxListener);
}

//-----------------------------------------------------------------------------
void OFileControlModel::removeResetListener(const Reference<XResetListener>& _rxListener)
{
    m_aResetListeners.removeInterface(_rxListener);
}

//------------------------------------------------------------------------------
void OFileControlModel::_reset()
{
    {   // release our mutex once (it's acquired in the calling method !), as setting aggregate properties
        // may cause any uno controls belonging to us to lock the solar mutex, which is potentially dangerous with
        // our own mutex locked
        // FS - 72451 - 31.01.00
        MutexRelease aRelease(m_aMutex);
        m_xAggregateSet->setPropertyValue(PROPERTY_TEXT, makeAny(m_sDefaultValue));
    }
}

//.........................................................................
}   // namespace frm
//.........................................................................

