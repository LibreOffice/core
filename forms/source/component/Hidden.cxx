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

#include "Hidden.hxx"
#include "property.hxx"
#include "property.hrc"
#include "services.hxx"
#include <tools/debug.hxx>
#include <comphelper/basicio.hxx>
#include <comphelper/processfactory.hxx>


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


OHiddenModel::OHiddenModel(const Reference<XComponentContext>& _rxFactory)
    :OControlModel(_rxFactory, OUString())
{
    m_nClassId = FormComponentType::HIDDENCONTROL;
}


OHiddenModel::OHiddenModel( const OHiddenModel* _pOriginal, const Reference<XComponentContext>& _rxFactory )
    :OControlModel( _pOriginal, _rxFactory )
{
    m_sHiddenValue = _pOriginal->m_sHiddenValue;
}


OHiddenModel::~OHiddenModel( )
{
}


IMPLEMENT_DEFAULT_CLONING( OHiddenModel )


void OHiddenModel::getFastPropertyValue(Any& _rValue, sal_Int32 _nHandle) const
{
    switch (_nHandle)
    {
        case PROPERTY_ID_HIDDEN_VALUE : _rValue <<= m_sHiddenValue; break;
        default:
            OControlModel::getFastPropertyValue(_rValue, _nHandle);
    }
}


void OHiddenModel::setFastPropertyValue_NoBroadcast(sal_Int32 _nHandle, const Any& _rValue) throw (css::uno::Exception, std::exception)
{
    switch (_nHandle)
    {
        case PROPERTY_ID_HIDDEN_VALUE :
            DBG_ASSERT(_rValue.getValueType().getTypeClass() == TypeClass_STRING, "OHiddenModel::setFastPropertyValue_NoBroadcast : invalid type !" );
            _rValue >>= m_sHiddenValue;
            break;
        default:
            OControlModel::setFastPropertyValue_NoBroadcast(_nHandle, _rValue);
    }
}


sal_Bool OHiddenModel::convertFastPropertyValue(
            Any& _rConvertedValue, Any& _rOldValue, sal_Int32 _nHandle, const Any& _rValue)
            throw (IllegalArgumentException)
{
    bool bModified(false);
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


void OHiddenModel::describeFixedProperties( Sequence< Property >& _rProps ) const
{
    BEGIN_DESCRIBE_BASE_PROPERTIES(4)
        DECL_PROP2(CLASSID,         sal_Int16,          READONLY, TRANSIENT);
        DECL_PROP1(HIDDEN_VALUE,    OUString,    BOUND);
        DECL_PROP1(NAME,            OUString,    BOUND);
        DECL_PROP1(TAG,             OUString,    BOUND);
    END_DESCRIBE_PROPERTIES();
}

// XServiceInfo

css::uno::Sequence<OUString> SAL_CALL OHiddenModel::getSupportedServiceNames() throw(css::uno::RuntimeException, std::exception)
{
    return css::uno::Sequence<OUString>{
        FRM_SUN_COMPONENT_HIDDENCONTROL, FRM_SUN_FORMCOMPONENT,
        FRM_COMPONENT_HIDDEN, FRM_COMPONENT_HIDDENCONTROL };
}


OUString SAL_CALL OHiddenModel::getServiceName() throw(RuntimeException, std::exception)
{
    return OUString(FRM_COMPONENT_HIDDEN);    // old (non-sun) name for compatibility !
}


void SAL_CALL OHiddenModel::write(const Reference<XObjectOutputStream>& _rxOutStream)
    throw(IOException, RuntimeException, std::exception)
{
    // Version
    _rxOutStream->writeShort(0x0002);

    // Value
    _rxOutStream << m_sHiddenValue;

    OControlModel::write(_rxOutStream);
}


void SAL_CALL OHiddenModel::read(const Reference<XObjectInputStream>& _rxInStream) throw(IOException, RuntimeException, std::exception)
{
    // Version
    sal_uInt16 nVersion = _rxInStream->readShort();

    // Name
    DBG_ASSERT(nVersion != 1, "OHiddenModel::read : this version is obsolete !");
    switch (nVersion)
    {
        case 1 : { OUString sDummy; _rxInStream >> sDummy; _rxInStream >> m_sHiddenValue; } break;
        case 2 : _rxInStream >> m_sHiddenValue; break;
        default : OSL_FAIL("OHiddenModel::read : unknown version !"); m_sHiddenValue.clear();
    }
    OControlModel::read(_rxInStream);
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_form_OHiddenModel_get_implementation(css::uno::XComponentContext* component,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new frm::OHiddenModel(component));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
