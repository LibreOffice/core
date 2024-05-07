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

#include "ColumnModel.hxx"
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <stringconstants.hxx>
#include <strings.hxx>

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_dbu_OColumnControlModel_get_implementation(
    css::uno::XComponentContext* , css::uno::Sequence<css::uno::Any> const& )
{
    return cppu::acquire(new ::dbaui::OColumnControlModel());
}

namespace dbaui
{
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;

OColumnControlModel::OColumnControlModel()
    :OPropertyContainer(m_aBHelper)
    ,OColumnControlModel_BASE(m_aMutex)
    ,m_sDefaultControl(SERVICE_CONTROLDEFAULT)
    ,m_bEnable(true)
    ,m_nBorder(0)
    ,m_nWidth(50)
{
    registerProperties();
}

OColumnControlModel::OColumnControlModel(const OColumnControlModel* _pSource)
    :OPropertyContainer(m_aBHelper)
    ,OColumnControlModel_BASE(m_aMutex)
    ,m_sDefaultControl(_pSource->m_sDefaultControl)
    ,m_aTabStop(_pSource->m_aTabStop)
    ,m_bEnable(_pSource->m_bEnable)
    ,m_nBorder(_pSource->m_nBorder)
    ,m_nWidth(50)
{
    registerProperties();
}

OColumnControlModel::~OColumnControlModel()
{
    if ( !OColumnControlModel_BASE::rBHelper.bDisposed && !OColumnControlModel_BASE::rBHelper.bInDispose )
    {
        acquire();
        dispose();
    }
}

void OColumnControlModel::registerProperties()
{
    registerProperty( PROPERTY_ACTIVE_CONNECTION, PROPERTY_ID_ACTIVE_CONNECTION, PropertyAttribute::TRANSIENT | PropertyAttribute::BOUND,
        &m_xConnection, cppu::UnoType<decltype(m_xConnection)>::get() );
    Any a;
    a <<= m_xColumn;
    registerProperty( PROPERTY_COLUMN, PROPERTY_ID_COLUMN, PropertyAttribute::TRANSIENT | PropertyAttribute::BOUND,
            &m_xColumn, cppu::UnoType<decltype(m_xColumn)>::get() );

    registerMayBeVoidProperty( PROPERTY_TABSTOP, PROPERTY_ID_TABSTOP, PropertyAttribute::BOUND | PropertyAttribute::MAYBEVOID,
            &m_aTabStop, ::cppu::UnoType<sal_Int16>::get() );
    registerProperty( PROPERTY_DEFAULTCONTROL, PROPERTY_ID_DEFAULTCONTROL, PropertyAttribute::BOUND,
            &m_sDefaultControl, cppu::UnoType<decltype(m_sDefaultControl)>::get() );
    registerProperty( PROPERTY_ENABLED, PROPERTY_ID_ENABLED, PropertyAttribute::BOUND,
            &m_bEnable, cppu::UnoType<decltype(m_bEnable)>::get() );
    registerProperty( PROPERTY_BORDER, PROPERTY_ID_BORDER, PropertyAttribute::BOUND,
        &m_nBorder, cppu::UnoType<decltype(m_nBorder)>::get() );
    registerProperty( PROPERTY_EDIT_WIDTH, PROPERTY_ID_EDIT_WIDTH, PropertyAttribute::BOUND,
        &m_nWidth, cppu::UnoType<decltype(m_nWidth)>::get() );
}

// XCloneable
Reference< XCloneable > SAL_CALL OColumnControlModel::createClone( )
{
    return new OColumnControlModel( this );
}

css::uno::Sequence<sal_Int8> OColumnControlModel::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

css::uno::Sequence< css::uno::Type > OColumnControlModel::getTypes()
{
    return  ::comphelper::concatSequences(
        OColumnControlModel_BASE::getTypes( ),
        OPropertyContainer::getTypes( )
    );
}
css::uno::Reference< css::beans::XPropertySetInfo >  SAL_CALL OColumnControlModel::getPropertySetInfo()
{
    Reference< XPropertySetInfo > xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}
::cppu::IPropertyArrayHelper& OColumnControlModel::getInfoHelper()
{
    return *OColumnControlModel::getArrayHelper();
}
::cppu::IPropertyArrayHelper* OColumnControlModel::createArrayHelper( ) const
{
    css::uno::Sequence< css::beans::Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}

OUString SAL_CALL OColumnControlModel::getImplementationName()
{
    return u"com.sun.star.comp.dbu.OColumnControlModel"_ustr;
}
sal_Bool SAL_CALL OColumnControlModel::supportsService(const OUString& _rServiceName)
    {
        const css::uno::Sequence< OUString > aSupported(getSupportedServiceNames());
        for (const OUString& s : aSupported)
            if (s == _rServiceName)
                return true;

        return false;
    }
css::uno::Sequence< OUString > SAL_CALL OColumnControlModel::getSupportedServiceNames()
{
    return { u"com.sun.star.awt.UnoControlModel"_ustr,u"com.sun.star.sdb.ColumnDescriptorControlModel"_ustr };
}
IMPLEMENT_FORWARD_REFCOUNT( OColumnControlModel, OColumnControlModel_BASE )
Any SAL_CALL OColumnControlModel::queryInterface( const Type& _rType )
{
    return OColumnControlModel_BASE::queryInterface( _rType );
}

// css::XAggregation
Any SAL_CALL OColumnControlModel::queryAggregation( const Type& rType )
{
    Any aRet(OColumnControlModel_BASE::queryAggregation(rType));
    if (!aRet.hasValue())
        aRet = comphelper::OPropertyContainer::queryInterface(rType);
    return aRet;
}

OUString SAL_CALL OColumnControlModel::getServiceName()
{
    return OUString();
}

void OColumnControlModel::write(const Reference<XObjectOutputStream>& /*_rxOutStream*/)
{
    // TODO
}

void OColumnControlModel::read(const Reference<XObjectInputStream>& /*_rxInStream*/)
{
    // TODO
}

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
