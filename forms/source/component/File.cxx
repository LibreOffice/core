/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "File.hxx"

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/form/FormComponentType.hpp>

#include <property.hxx>
#include <services.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <tools/debug.hxx>
#include <comphelper/basicio.hxx>
#include <comphelper/property.hxx>

using namespace comphelper;


namespace frm
{

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;


Sequence<Type> OFileControlModel::_getTypes()
{
    static Sequence<Type> const aTypes =
        concatSequences(OControlModel::_getTypes(), Sequence<Type>{ cppu::UnoType<XReset>::get() });
    return aTypes;
}


// XServiceInfo

css::uno::Sequence<OUString>  OFileControlModel::getSupportedServiceNames()
{
    css::uno::Sequence<OUString> aSupported = OControlModel::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 2);

    OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-2] = FRM_SUN_COMPONENT_FILECONTROL;
    pArray[aSupported.getLength()-1] = FRM_COMPONENT_FILECONTROL;
    return aSupported;
}


OFileControlModel::OFileControlModel(const Reference<XComponentContext>& _rxFactory)
                    :OControlModel(_rxFactory, VCL_CONTROLMODEL_FILECONTROL)
                    ,m_aResetListeners(m_aMutex)
{
    m_nClassId = FormComponentType::FILECONTROL;
}


OFileControlModel::OFileControlModel( const OFileControlModel* _pOriginal, const Reference<XComponentContext>& _rxFactory )
    :OControlModel( _pOriginal, _rxFactory )
    ,m_aResetListeners( m_aMutex )
{

    m_sDefaultValue = _pOriginal->m_sDefaultValue;
}


OFileControlModel::~OFileControlModel()
{
    if (!OComponentHelper::rBHelper.bDisposed)
    {
        acquire();
        dispose();
    }
}


css::uno::Reference< css::util::XCloneable > SAL_CALL OFileControlModel::createClone()
{
    rtl::Reference<OFileControlModel> pClone = new OFileControlModel(this, getContext());
    pClone->clonedFrom(this);
    return pClone;
}


Any SAL_CALL OFileControlModel::queryAggregation(const Type& _rType)
{
    Any aReturn = OControlModel::queryAggregation(_rType);
    if (!aReturn.hasValue())
        aReturn = ::cppu::queryInterface(_rType
            ,static_cast<XReset*>(this)
        );

    return aReturn;
}

// OComponentHelper

void OFileControlModel::disposing()
{
    OControlModel::disposing();

    EventObject aEvt(static_cast<XWeak*>(this));
    m_aResetListeners.disposeAndClear(aEvt);
}


Any OFileControlModel::getPropertyDefaultByHandle( sal_Int32 _nHandle ) const
{
    switch ( _nHandle )
    {
        case PROPERTY_ID_DEFAULT_TEXT:
            return Any( OUString() );
    }
    return OControlModel::getPropertyDefaultByHandle( _nHandle );
}


void OFileControlModel::getFastPropertyValue(Any& rValue, sal_Int32 nHandle) const
{
    switch (nHandle)
    {
        case PROPERTY_ID_DEFAULT_TEXT : rValue <<= m_sDefaultValue; break;
        default:
            OControlModel::getFastPropertyValue(rValue, nHandle);
    }
}


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


sal_Bool OFileControlModel::convertFastPropertyValue(Any& rConvertedValue, Any& rOldValue, sal_Int32 nHandle, const Any& rValue)
{
    switch (nHandle)
    {
        case PROPERTY_ID_DEFAULT_TEXT :
            return tryPropertyValue(rConvertedValue, rOldValue, rValue, m_sDefaultValue);
        default:
            return OControlModel::convertFastPropertyValue(rConvertedValue, rOldValue, nHandle, rValue);
    }
}


void OFileControlModel::describeFixedProperties( Sequence< Property >& _rProps ) const
{
    OControlModel::describeFixedProperties( _rProps );
    sal_Int32 nOldCount = _rProps.getLength();
    _rProps.realloc( nOldCount + 2);
    css::beans::Property* pProperties = _rProps.getArray() + nOldCount;
    *pProperties++ = css::beans::Property(PROPERTY_DEFAULT_TEXT, PROPERTY_ID_DEFAULT_TEXT, cppu::UnoType<OUString>::get(), css::beans::PropertyAttribute::BOUND);
    *pProperties++ = css::beans::Property(PROPERTY_TABINDEX, PROPERTY_ID_TABINDEX, cppu::UnoType<sal_Int16>::get(), css::beans::PropertyAttribute::BOUND);
    DBG_ASSERT( pProperties == _rProps.getArray() + _rProps.getLength(), "<...>::describeFixedProperties/getInfoHelper: forgot to adjust the count ?");
}


OUString SAL_CALL OFileControlModel::getServiceName()
{
    return FRM_COMPONENT_FILECONTROL;   // old (non-sun) name for compatibility !
}


void OFileControlModel::write(const Reference<css::io::XObjectOutputStream>& _rxOutStream)
{
    OControlModel::write(_rxOutStream);

    ::osl::MutexGuard aGuard(m_aMutex);

    // Version
    _rxOutStream->writeShort(0x0002);
    // Default value
    _rxOutStream << m_sDefaultValue;
    writeHelpTextCompatibly(_rxOutStream);
}


void OFileControlModel::read(const Reference<css::io::XObjectInputStream>& _rxInStream)
{
    OControlModel::read(_rxInStream);
    ::osl::MutexGuard aGuard(m_aMutex);

    // Version
    sal_uInt16 nVersion = _rxInStream->readShort();
    // Default value
    switch (nVersion)
    {
        case 1:
            _rxInStream >> m_sDefaultValue; break;
        case 2:
            _rxInStream >> m_sDefaultValue;
            readHelpTextCompatibly(_rxInStream);
            break;
        default:
            OSL_FAIL("OFileControlModel::read : unknown version !");
            m_sDefaultValue.clear();
    }

    // Display default values after read
//  _reset();
}


void SAL_CALL OFileControlModel::reset()
{
    ::comphelper::OInterfaceIteratorHelper3 aIter(m_aResetListeners);
    EventObject aEvt(static_cast<XWeak*>(this));
    bool bContinue = true;
    while (aIter.hasMoreElements() && bContinue)
        bContinue = aIter.next()->approveReset(aEvt);

    if (bContinue)
    {
        // don't lock our mutex as setting aggregate properties
        // may cause any uno controls belonging to us to lock the solar mutex, which is potentially dangerous with
        // our own mutex locked
        m_xAggregateSet->setPropertyValue(PROPERTY_TEXT, Any(m_sDefaultValue));
        m_aResetListeners.notifyEach( &XResetListener::resetted, aEvt );
    }
}


void OFileControlModel::addResetListener(const Reference<XResetListener>& _rxListener)
{
    m_aResetListeners.addInterface(_rxListener);
}


void OFileControlModel::removeResetListener(const Reference<XResetListener>& _rxListener)
{
    m_aResetListeners.removeInterface(_rxListener);
}


}   // namespace frm

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_form_OFileControlModel_get_implementation(css::uno::XComponentContext* component,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new frm::OFileControlModel(component));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
