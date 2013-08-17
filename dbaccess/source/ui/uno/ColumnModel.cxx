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
#include <com/sun/star/awt/FontRelief.hpp>
#include <com/sun/star/awt/FontEmphasisMark.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <cppuhelper/queryinterface.hxx>
#include <comphelper/extract.hxx>
#include "dbustrings.hrc"
#include "dbu_reghelper.hxx"
#include <toolkit/helper/vclunohelper.hxx>
#include <comphelper/property.hxx>

extern "C" void SAL_CALL createRegistryInfo_OColumnControlModel()
{
    static ::dbaui::OMultiInstanceAutoRegistration< ::dbaui::OColumnControlModel> aAutoRegistration;
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

DBG_NAME(OColumnControlModel)
OColumnControlModel::OColumnControlModel(const Reference<XMultiServiceFactory>& _rxFactory)
    :OPropertyContainer(m_aBHelper)
    ,OColumnControlModel_BASE(m_aMutex)
    ,m_xORB(_rxFactory)
    ,m_sDefaultControl(SERVICE_CONTROLDEFAULT)
    ,m_bEnable(sal_True)
    ,m_nBorder(0)
    ,m_nWidth(50)
{
    DBG_CTOR(OColumnControlModel,NULL);
    registerProperties();
}

OColumnControlModel::OColumnControlModel(const OColumnControlModel* _pSource,const Reference<XMultiServiceFactory>& _rxFactory)
    :OPropertyContainer(m_aBHelper)
    ,OColumnControlModel_BASE(m_aMutex)
    ,m_xORB(_rxFactory)
    ,m_sDefaultControl(_pSource->m_sDefaultControl)
    ,m_aTabStop(_pSource->m_aTabStop)
    ,m_bEnable(_pSource->m_bEnable)
    ,m_nBorder(_pSource->m_nBorder)
    ,m_nWidth(50)
{
    DBG_CTOR(OColumnControlModel,NULL);
    registerProperties();
}

OColumnControlModel::~OColumnControlModel()
{
    DBG_DTOR(OColumnControlModel,NULL);
    if ( !OColumnControlModel_BASE::rBHelper.bDisposed && !OColumnControlModel_BASE::rBHelper.bInDispose )
    {
        acquire();
        dispose();
    }
}

void OColumnControlModel::registerProperties()
{
    registerProperty( PROPERTY_ACTIVE_CONNECTION, PROPERTY_ID_ACTIVE_CONNECTION, PropertyAttribute::TRANSIENT | PropertyAttribute::BOUND,
        &m_xConnection, ::getCppuType( &m_xConnection ) );
    Any a;
    a <<= m_xColumn;
    registerProperty( PROPERTY_COLUMN, PROPERTY_ID_COLUMN, PropertyAttribute::TRANSIENT | PropertyAttribute::BOUND,
            &m_xColumn, ::getCppuType( &m_xColumn ) );

    registerMayBeVoidProperty( PROPERTY_TABSTOP, PROPERTY_ID_TABSTOP, PropertyAttribute::BOUND | PropertyAttribute::MAYBEVOID,
            &m_aTabStop, ::getCppuType( static_cast<sal_Int16*>(NULL) ) );
    registerProperty( PROPERTY_DEFAULTCONTROL, PROPERTY_ID_DEFAULTCONTROL, PropertyAttribute::BOUND,
            &m_sDefaultControl, ::getCppuType( &m_sDefaultControl ) );
    registerProperty( PROPERTY_ENABLED, PROPERTY_ID_ENABLED, PropertyAttribute::BOUND,
            &m_bEnable, ::getCppuType( &m_bEnable ) );
    registerProperty( PROPERTY_BORDER, PROPERTY_ID_BORDER, PropertyAttribute::BOUND,
        &m_nBorder, ::getCppuType( &m_nBorder ) );
    registerProperty( PROPERTY_EDIT_WIDTH, PROPERTY_ID_EDIT_WIDTH, PropertyAttribute::BOUND,
        &m_nWidth, ::getCppuType( &m_nWidth ) );
}

// XCloneable
Reference< XCloneable > SAL_CALL OColumnControlModel::createClone( ) throw (RuntimeException)
{
    return new OColumnControlModel( this, getORB() );
}

IMPLEMENT_TYPEPROVIDER2(OColumnControlModel,OColumnControlModel_BASE,comphelper::OPropertyContainer)
IMPLEMENT_PROPERTYCONTAINER_DEFAULTS(OColumnControlModel)
IMPLEMENT_SERVICE_INFO2_STATIC(OColumnControlModel,"com.sun.star.comp.dbu.OColumnControlModel","com.sun.star.awt.UnoControlModel","com.sun.star.sdb.ColumnDescriptorControlModel")
IMPLEMENT_FORWARD_REFCOUNT( OColumnControlModel, OColumnControlModel_BASE )
Any SAL_CALL OColumnControlModel::queryInterface( const Type& _rType ) throw (RuntimeException)
{
    return OColumnControlModel_BASE::queryInterface( _rType );
}

// com::sun::star::XAggregation
Any SAL_CALL OColumnControlModel::queryAggregation( const Type& rType ) throw(RuntimeException)
{
    Any aRet(OColumnControlModel_BASE::queryAggregation(rType));
    if (!aRet.hasValue())
        aRet = comphelper::OPropertyContainer::queryInterface(rType);
    return aRet;
}

OUString SAL_CALL OColumnControlModel::getServiceName() throw ( RuntimeException)
{
    return OUString();
}

void OColumnControlModel::write(const Reference<XObjectOutputStream>& /*_rxOutStream*/) throw ( ::com::sun::star::io::IOException, RuntimeException)
{
    // TODO
}

void OColumnControlModel::read(const Reference<XObjectInputStream>& /*_rxInStream*/) throw ( ::com::sun::star::io::IOException, RuntimeException)
{
    // TODO
}

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
