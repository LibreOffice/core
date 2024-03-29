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

#include "GroupBox.hxx"
#include <frm_strings.hxx>
#include <services.hxx>
#include <comphelper/property.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/form/FormComponentType.hpp>

namespace frm
{

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::util;
using namespace comphelper;

// OGroupBoxModel


OGroupBoxModel::OGroupBoxModel(const Reference<XComponentContext>& _rxFactory)
    :OControlModel(_rxFactory, VCL_CONTROLMODEL_GROUPBOX, VCL_CONTROL_GROUPBOX)
{
    m_nClassId = FormComponentType::GROUPBOX;
}


OGroupBoxModel::OGroupBoxModel( const OGroupBoxModel* _pOriginal, const Reference<XComponentContext>& _rxFactory )
    :OControlModel( _pOriginal, _rxFactory )
{
}

// XServiceInfo

css::uno::Sequence<OUString> SAL_CALL OGroupBoxModel::getSupportedServiceNames()
{
    css::uno::Sequence<OUString> aSupported = OControlModel::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 2);

    OUString* pArray = aSupported.getArray();
    pArray[aSupported.getLength()-2] = FRM_SUN_COMPONENT_GROUPBOX;
    pArray[aSupported.getLength()-1] = FRM_COMPONENT_GROUPBOX;
    return aSupported;
}


OGroupBoxModel::~OGroupBoxModel()
{
}


css::uno::Reference< css::util::XCloneable > SAL_CALL OGroupBoxModel::createClone()
{
    rtl::Reference<OGroupBoxModel> pClone = new OGroupBoxModel(this, getContext());
    pClone->clonedFrom(this);
    return pClone;
}


void OGroupBoxModel::describeAggregateProperties( Sequence< Property >& _rAggregateProps ) const
{
    OControlModel::describeAggregateProperties( _rAggregateProps );
    // don't want to have the TabStop property
    RemoveProperty(_rAggregateProps, PROPERTY_TABSTOP);
}


OUString SAL_CALL OGroupBoxModel::getServiceName()
{
    return FRM_COMPONENT_GROUPBOX;  // old (non-sun) name for compatibility !
}


void SAL_CALL OGroupBoxModel::write(const Reference< XObjectOutputStream>& _rxOutStream)
{
    OControlModel::write(_rxOutStream);

    // Version
    _rxOutStream->writeShort(0x0002);
    writeHelpTextCompatibly(_rxOutStream);
}


void SAL_CALL OGroupBoxModel::read(const Reference< XObjectInputStream>& _rxInStream)
{
    OControlModel::read( _rxInStream );

    // Version
    sal_uInt16 nVersion = _rxInStream->readShort();
    DBG_ASSERT(nVersion > 0, "OGroupBoxModel::read : version 0 ? this should never have been written !");

    if (nVersion == 2)
        readHelpTextCompatibly(_rxInStream);

    if (nVersion > 0x0002)
    {
        OSL_FAIL("OGroupBoxModel::read : unknown version !");
    }
};


// OGroupBoxControl

OGroupBoxControl::OGroupBoxControl(const Reference<XComponentContext>& _rxFactory)
                   :OControl(_rxFactory, VCL_CONTROL_GROUPBOX)
{
}


css::uno::Sequence<OUString> SAL_CALL OGroupBoxControl::getSupportedServiceNames()
{
    css::uno::Sequence<OUString> aSupported = OControl::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 2);

    OUString* pArray = aSupported.getArray();
    pArray[aSupported.getLength()-2] = FRM_SUN_CONTROL_GROUPBOX;
    pArray[aSupported.getLength()-1] = STARDIV_ONE_FORM_CONTROL_GROUPBOX;
    return aSupported;
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_form_OGroupBoxModel_get_implementation(css::uno::XComponentContext* component,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new frm::OGroupBoxModel(component));

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_form_OGroupBoxControl_get_implementation(css::uno::XComponentContext* component,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new frm::OGroupBoxControl(component));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
