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

#include "commanddefinition.hxx"
#include "apitools.hxx"
#include "dbastrings.hrc"
#include "module_dba.hxx"
#include "services.hxx"

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <tools/debug.hxx>
#include <comphelper/sequence.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::osl;
using namespace ::comphelper;
using namespace ::cppu;

// OCommandDefinition
extern "C" void SAL_CALL createRegistryInfo_OCommandDefinition()
{
    static ::dba::OAutoRegistration< ::dbaccess::OCommandDefinition > aAutoRegistration;
}

namespace dbaccess
{

void OCommandDefinition::registerProperties()
{
    OCommandDefinition_Impl& rCommandDefinition( getCommandDefinition() );
    registerProperty(PROPERTY_COMMAND, PROPERTY_ID_COMMAND, PropertyAttribute::BOUND,
                    &rCommandDefinition.m_sCommand, cppu::UnoType<decltype(rCommandDefinition.m_sCommand)>::get());

    registerProperty(PROPERTY_ESCAPE_PROCESSING, PROPERTY_ID_ESCAPE_PROCESSING, PropertyAttribute::BOUND,
                    &rCommandDefinition.m_bEscapeProcessing, cppu::UnoType<bool>::get());

    registerProperty(PROPERTY_UPDATE_TABLENAME, PROPERTY_ID_UPDATE_TABLENAME, PropertyAttribute::BOUND,
                    &rCommandDefinition.m_sUpdateTableName, cppu::UnoType<decltype(rCommandDefinition.m_sUpdateTableName)>::get());

    registerProperty(PROPERTY_UPDATE_SCHEMANAME, PROPERTY_ID_UPDATE_SCHEMANAME, PropertyAttribute::BOUND,
                    &rCommandDefinition.m_sUpdateSchemaName, cppu::UnoType<decltype(rCommandDefinition.m_sUpdateSchemaName)>::get());

    registerProperty(PROPERTY_UPDATE_CATALOGNAME, PROPERTY_ID_UPDATE_CATALOGNAME, PropertyAttribute::BOUND,
                    &rCommandDefinition.m_sUpdateCatalogName, cppu::UnoType<decltype(rCommandDefinition.m_sUpdateCatalogName)>::get());
    registerProperty(PROPERTY_LAYOUTINFORMATION, PROPERTY_ID_LAYOUTINFORMATION, PropertyAttribute::BOUND,
                    &rCommandDefinition.m_aLayoutInformation, cppu::UnoType<decltype(rCommandDefinition.m_aLayoutInformation)>::get());
}

OCommandDefinition::OCommandDefinition(const Reference< XComponentContext >& _xORB
                                       ,const Reference< XInterface >& _rxContainer
                                       ,const TContentPtr& _pImpl)
    :OComponentDefinition(_xORB,_rxContainer,_pImpl,false)
{
    registerProperties();
}

OCommandDefinition::~OCommandDefinition()
{
}

OCommandDefinition::OCommandDefinition( const Reference< XInterface >& _rxContainer
                                       ,const OUString& _rElementName
                                       ,const Reference< XComponentContext >& _xORB
                                       ,const TContentPtr& _pImpl)
    :OComponentDefinition(_rxContainer,_rElementName,_xORB,_pImpl,false)
{
    registerProperties();
}

css::uno::Sequence<sal_Int8> OCommandDefinition::getImplementationId()
    throw (css::uno::RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
}

IMPLEMENT_GETTYPES2(OCommandDefinition,OCommandDefinition_Base,OComponentDefinition);
IMPLEMENT_FORWARD_XINTERFACE2( OCommandDefinition,OComponentDefinition,OCommandDefinition_Base)
IMPLEMENT_PROPERTYCONTAINER_DEFAULTS2(OCommandDefinition,OCommandDefinition_PROP)

OUString OCommandDefinition::getImplementationName_static(  ) throw(RuntimeException)
{
    return OUString("com.sun.star.comp.dba.OCommandDefinition");
}

OUString SAL_CALL OCommandDefinition::getImplementationName(  ) throw(RuntimeException, std::exception)
{
    return getImplementationName_static();
}

Sequence< OUString > OCommandDefinition::getSupportedServiceNames_static(  ) throw(RuntimeException)
{
    Sequence< OUString > aServices(3);
    aServices.getArray()[0] = "com.sun.star.sdb.QueryDefinition";
    aServices.getArray()[1] = "com.sun.star.sdb.CommandDefinition";
    aServices.getArray()[2] = "com.sun.star.ucb.Content";
    return aServices;
}

Sequence< OUString > SAL_CALL OCommandDefinition::getSupportedServiceNames(  ) throw(RuntimeException, std::exception)
{
    return getSupportedServiceNames_static();
}

Reference< XInterface > OCommandDefinition::Create(const Reference< XComponentContext >& _rxContext)
{
    return *(new OCommandDefinition( _rxContext, nullptr, TContentPtr( new OCommandDefinition_Impl ) ) );
}

void SAL_CALL OCommandDefinition::rename( const OUString& newName ) throw (SQLException, ElementExistException, RuntimeException, std::exception)
{
    try
    {
        sal_Int32 nHandle = PROPERTY_ID_NAME;
        osl::ClearableGuard< osl::Mutex > aGuard(m_aMutex);
        Any aOld = makeAny(m_pImpl->m_aProps.aTitle);
        aGuard.clear();
        Any aNew = makeAny(newName);
        fire(&nHandle, &aNew, &aOld, 1, sal_True );

        m_pImpl->m_aProps.aTitle = newName;
        fire(&nHandle, &aNew, &aOld, 1, sal_False );
    }
    catch(const PropertyVetoException&)
    {
        throw ElementExistException(newName,*this);
    }
}

}   // namespace dbaccess

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
