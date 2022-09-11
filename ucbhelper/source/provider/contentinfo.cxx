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

#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/ucb/UnsupportedCommandException.hpp>
#include <com/sun/star/ucb/XPersistentPropertySet.hpp>
#include <com/sun/star/ucb/XCommandInfo.hpp>

#include <ucbhelper/contenthelper.hxx>
#include <utility>
#include "contentinfo.hxx"

using namespace com::sun::star;


// PropertySetInfo Implementation.


namespace ucbhelper {

PropertySetInfo::PropertySetInfo(
    uno::Reference< css::ucb::XCommandEnvironment > xEnv,
    ContentImplHelper* pContent )
: m_xEnv(std::move( xEnv )),
  m_pContent( pContent )
{
}


// virtual
PropertySetInfo::~PropertySetInfo()
{
}


// XPropertySetInfo methods.


// virtual
uno::Sequence< beans::Property > SAL_CALL PropertySetInfo::getProperties()
{
    std::unique_lock aGuard( m_aMutex );
    return getPropertiesImpl();
}

const uno::Sequence< beans::Property > & PropertySetInfo::getPropertiesImpl()
{
    if ( m_xProps )
        return *m_xProps;

    // Get info for core ( native) properties.

    try
    {
        m_xProps = m_pContent->getProperties( m_xEnv );
    }
    catch ( uno::RuntimeException const & )
    {
        throw;
    }
    catch ( uno::Exception const & )
    {
        m_xProps.emplace();
    }

    // Get info for additional properties.

    uno::Reference< css::ucb::XPersistentPropertySet >
        xSet ( m_pContent->getAdditionalPropertySet( false ) );

    if ( xSet.is() )
    {
        // Get property set info.
        uno::Reference< beans::XPropertySetInfo > xInfo(
            xSet->getPropertySetInfo() );
        if ( xInfo.is() )
        {
            const uno::Sequence< beans::Property >& rAddProps
                = xInfo->getProperties();
            sal_Int32 nAddProps = rAddProps.getLength();
            if ( nAddProps > 0 )
            {
                sal_Int32 nPos = m_xProps->getLength();
                m_xProps->realloc( nPos + nAddProps );

                std::copy(rAddProps.begin(), rAddProps.end(),
                          std::next(m_xProps->getArray(), nPos));
            }
        }
    }
    return *m_xProps;
}


// virtual
beans::Property SAL_CALL PropertySetInfo::getPropertyByName(
        const OUString& aName )
{
    beans::Property aProp;
    if ( queryProperty( aName, aProp ) )
        return aProp;

    throw beans::UnknownPropertyException(aName);
}


// virtual
sal_Bool SAL_CALL PropertySetInfo::hasPropertyByName(
        const OUString& Name )
{
    beans::Property aProp;
    return queryProperty( Name, aProp );
}


// Non-Interface methods.


void PropertySetInfo::reset()
{
    std::unique_lock aGuard( m_aMutex );
    m_xProps.reset();
}


bool PropertySetInfo::queryProperty(
    std::u16string_view rName, beans::Property& rProp )
{
    std::unique_lock aGuard( m_aMutex );

    getPropertiesImpl();

    const beans::Property* pProps = m_xProps->getConstArray();
    sal_Int32 nCount = m_xProps->getLength();
    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        const beans::Property& rCurrProp = pProps[ n ];
        if ( rCurrProp.Name == rName )
        {
            rProp = rCurrProp;
            return true;
        }
    }

    return false;
}


// CommandProcessorInfo Implementation.


CommandProcessorInfo::CommandProcessorInfo(
    uno::Reference< css::ucb::XCommandEnvironment > xEnv,
    ContentImplHelper* pContent )
: m_xEnv(std::move( xEnv )),
  m_pContent( pContent )
{
}


// virtual
CommandProcessorInfo::~CommandProcessorInfo()
{
}


// XCommandInfo methods.


// virtual
uno::Sequence< css::ucb::CommandInfo > SAL_CALL CommandProcessorInfo::getCommands()
{
    std::unique_lock aGuard( m_aMutex );
    return getCommandsImpl();
}

const uno::Sequence< css::ucb::CommandInfo > & CommandProcessorInfo::getCommandsImpl()
{
    if ( m_xCommands )
        return *m_xCommands;

    // Get info for commands.

    try
    {
        m_xCommands = m_pContent->getCommands( m_xEnv );
    }
    catch ( uno::RuntimeException const & )
    {
        throw;
    }
    catch ( uno::Exception const & )
    {
        m_xCommands.emplace();
    }
    return *m_xCommands;
}


// virtual
css::ucb::CommandInfo SAL_CALL
CommandProcessorInfo::getCommandInfoByName(
        const OUString& Name )
{
    css::ucb::CommandInfo aInfo;
    if ( queryCommand( Name, aInfo ) )
        return aInfo;

    throw css::ucb::UnsupportedCommandException();
}


// virtual
css::ucb::CommandInfo SAL_CALL
CommandProcessorInfo::getCommandInfoByHandle( sal_Int32 Handle )
{
    css::ucb::CommandInfo aInfo;
    if ( queryCommand( Handle, aInfo ) )
        return aInfo;

    throw css::ucb::UnsupportedCommandException();
}


// virtual
sal_Bool SAL_CALL CommandProcessorInfo::hasCommandByName(
       const OUString& Name )
{
    css::ucb::CommandInfo aInfo;
    return queryCommand( Name, aInfo );
}


// virtual
sal_Bool SAL_CALL CommandProcessorInfo::hasCommandByHandle( sal_Int32 Handle )
{
    css::ucb::CommandInfo aInfo;
    return queryCommand( Handle, aInfo );
}


// Non-Interface methods.


void CommandProcessorInfo::reset()
{
    std::unique_lock aGuard( m_aMutex );
    m_xCommands.reset();
}


bool CommandProcessorInfo::queryCommand(
    std::u16string_view rName,
    css::ucb::CommandInfo& rCommand )
{
    std::unique_lock aGuard( m_aMutex );

    getCommandsImpl();

    const css::ucb::CommandInfo* pCommands
        = m_xCommands->getConstArray();
    sal_Int32 nCount = m_xCommands->getLength();
    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        const css::ucb::CommandInfo& rCurrCommand = pCommands[ n ];
        if ( rCurrCommand.Name == rName )
        {
            rCommand = rCurrCommand;
            return true;
        }
    }

    return false;
}


bool CommandProcessorInfo::queryCommand(
    sal_Int32 nHandle,
    css::ucb::CommandInfo& rCommand )
{
    std::unique_lock aGuard( m_aMutex );

    getCommandsImpl();

    const css::ucb::CommandInfo* pCommands = m_xCommands->getConstArray();
    sal_Int32 nCount = m_xCommands->getLength();
    for ( sal_Int32 n = 0; n < nCount; ++n )
    {
        const css::ucb::CommandInfo& rCurrCommand = pCommands[ n ];
        if ( rCurrCommand.Handle == nHandle )
        {
            rCommand = rCurrCommand;
            return true;
        }
    }

    return false;
}

} // namespace ucbhelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
