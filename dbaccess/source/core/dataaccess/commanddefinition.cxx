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

#include <com/sun/star/container/ElementExistException.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>

#include <tools/debug.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;

namespace dbaccess
{

void OCommandDefinition::registerProperties()
{
    OCommandDefinition_Impl& rCommandDefinition = dynamic_cast< OCommandDefinition_Impl& >( *m_pImpl.get() );
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
{
    return css::uno::Sequence<sal_Int8>();
}

IMPLEMENT_GETTYPES2(OCommandDefinition,OCommandDefinition_Base,OComponentDefinition);
IMPLEMENT_FORWARD_XINTERFACE2( OCommandDefinition,OComponentDefinition,OCommandDefinition_Base)
IMPLEMENT_PROPERTYCONTAINER_DEFAULTS2(OCommandDefinition,OCommandDefinition_PROP)

OUString SAL_CALL OCommandDefinition::getImplementationName()
{
    return OUString("com.sun.star.comp.dba.OCommandDefinition");
}

css::uno::Sequence<OUString> SAL_CALL OCommandDefinition::getSupportedServiceNames()
{
    return {
        "com.sun.star.sdb.QueryDefinition",
        "com.sun.star.sdb.CommandDefinition",
        "com.sun.star.ucb.Content"
    };
}

void SAL_CALL OCommandDefinition::rename( const OUString& newName )
{
    try
    {
        sal_Int32 nHandle = PROPERTY_ID_NAME;
        osl::ClearableGuard< osl::Mutex > aGuard(m_aMutex);
        Any aOld = makeAny(m_pImpl->m_aProps.aTitle);
        aGuard.clear();
        Any aNew = makeAny(newName);
        fire(&nHandle, &aNew, &aOld, 1, true );

        m_pImpl->m_aProps.aTitle = newName;
        fire(&nHandle, &aNew, &aOld, 1, false );
    }
    catch(const PropertyVetoException&)
    {
        throw ElementExistException(newName,*this);
    }
}

}   // namespace dbaccess

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_comp_dba_OCommandDefinition(css::uno::XComponentContext* context,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new dbaccess::OCommandDefinition(
            context, nullptr, dbaccess::TContentPtr( new dbaccess::OCommandDefinition_Impl )));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
