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

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <tools/debug.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/componentcontext.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::osl;
using namespace ::comphelper;
using namespace ::cppu;

namespace dbaccess
{

//==========================================================================
//= OCommandDefinition
//==========================================================================
extern "C" void SAL_CALL createRegistryInfo_OCommandDefinition()
{
    static ::dba::OAutoRegistration< OCommandDefinition > aAutoRegistration;
}

DBG_NAME(OCommandDefinition)

void OCommandDefinition::registerProperties()
{
    OCommandDefinition_Impl& rCommandDefinition( getCommandDefinition() );
    registerProperty(PROPERTY_COMMAND, PROPERTY_ID_COMMAND, PropertyAttribute::BOUND,
                    &rCommandDefinition.m_sCommand, ::getCppuType(&rCommandDefinition.m_sCommand));

    registerProperty(PROPERTY_ESCAPE_PROCESSING, PROPERTY_ID_ESCAPE_PROCESSING, PropertyAttribute::BOUND,
                    &rCommandDefinition.m_bEscapeProcessing, ::getBooleanCppuType());

    registerProperty(PROPERTY_UPDATE_TABLENAME, PROPERTY_ID_UPDATE_TABLENAME, PropertyAttribute::BOUND,
                    &rCommandDefinition.m_sUpdateTableName, ::getCppuType(&rCommandDefinition.m_sUpdateTableName));

    registerProperty(PROPERTY_UPDATE_SCHEMANAME, PROPERTY_ID_UPDATE_SCHEMANAME, PropertyAttribute::BOUND,
                    &rCommandDefinition.m_sUpdateSchemaName, ::getCppuType(&rCommandDefinition.m_sUpdateSchemaName));

    registerProperty(PROPERTY_UPDATE_CATALOGNAME, PROPERTY_ID_UPDATE_CATALOGNAME, PropertyAttribute::BOUND,
                    &rCommandDefinition.m_sUpdateCatalogName, ::getCppuType(&rCommandDefinition.m_sUpdateCatalogName));
    registerProperty(PROPERTY_LAYOUTINFORMATION, PROPERTY_ID_LAYOUTINFORMATION, PropertyAttribute::BOUND,
                    &rCommandDefinition.m_aLayoutInformation, ::getCppuType(&rCommandDefinition.m_aLayoutInformation));
}

OCommandDefinition::OCommandDefinition(const Reference< XMultiServiceFactory >& _xORB
                                       ,const Reference< XInterface >& _rxContainer
                                       ,const TContentPtr& _pImpl)
    :OComponentDefinition(_xORB,_rxContainer,_pImpl,sal_False)
{
    DBG_CTOR(OCommandDefinition, NULL);
    registerProperties();
}

OCommandDefinition::~OCommandDefinition()
{
    DBG_DTOR(OCommandDefinition, NULL);
}

OCommandDefinition::OCommandDefinition( const Reference< XInterface >& _rxContainer
                                       ,const ::rtl::OUString& _rElementName
                                       ,const Reference< XMultiServiceFactory >& _xORB
                                       ,const TContentPtr& _pImpl)
    :OComponentDefinition(_rxContainer,_rElementName,_xORB,_pImpl,sal_False)
{
    DBG_CTOR(OCommandDefinition, NULL);
    registerProperties();
}

IMPLEMENT_IMPLEMENTATION_ID(OCommandDefinition);
IMPLEMENT_GETTYPES2(OCommandDefinition,OCommandDefinition_Base,OComponentDefinition);
IMPLEMENT_FORWARD_XINTERFACE2( OCommandDefinition,OComponentDefinition,OCommandDefinition_Base)
IMPLEMENT_PROPERTYCONTAINER_DEFAULTS2(OCommandDefinition,OCommandDefinition_PROP)

OUString OCommandDefinition::getImplementationName_static(  ) throw(RuntimeException)
{
    return OUString("com.sun.star.comp.dba.OCommandDefinition");
}

::rtl::OUString SAL_CALL OCommandDefinition::getImplementationName(  ) throw(RuntimeException)
{
    return getImplementationName_static();
}

Sequence< OUString > OCommandDefinition::getSupportedServiceNames_static(  ) throw(RuntimeException)
{
    Sequence< OUString > aServices(3);
    aServices.getArray()[0] = SERVICE_SDB_QUERYDEFINITION;
    aServices.getArray()[1] = SERVICE_SDB_COMMAND_DEFINITION;
    aServices.getArray()[2] = "com.sun.star.ucb.Content";
    return aServices;
}

Sequence< ::rtl::OUString > SAL_CALL OCommandDefinition::getSupportedServiceNames(  ) throw(RuntimeException)
{
    return getSupportedServiceNames_static();
}

Reference< XInterface > OCommandDefinition::Create(const Reference< XComponentContext >& _rxContext)
{
    ::comphelper::ComponentContext aContext( _rxContext );
    return *(new OCommandDefinition( aContext.getLegacyServiceFactory(), NULL, TContentPtr( new OCommandDefinition_Impl ) ) );
}

void SAL_CALL OCommandDefinition::rename( const ::rtl::OUString& newName ) throw (SQLException, ElementExistException, RuntimeException)
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
