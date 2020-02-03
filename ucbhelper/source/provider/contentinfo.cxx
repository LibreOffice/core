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

/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/ucb/UnsupportedCommandException.hpp>
#include <com/sun/star/ucb/XPersistentPropertySet.hpp>
#include <com/sun/star/ucb/XCommandInfo.hpp>

#include <cppuhelper/queryinterface.hxx>
#include <osl/mutex.hxx>
#include <ucbhelper/contenthelper.hxx>
#include "contentinfo.hxx"
#include <ucbhelper/macros.hxx>

using namespace com::sun::star;


// PropertySetInfo Implementation.


namespace ucbhelper {

PropertySetInfo::PropertySetInfo(
    const uno::Reference< css::ucb::XCommandEnvironment >& rxEnv,
    ContentImplHelper* pContent )
: m_xEnv( rxEnv ),
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
    if ( !m_pProps )
    {
        osl::MutexGuard aGuard( m_aMutex );
        if ( !m_pProps )
        {

            // Get info for core ( native) properties.


            try
            {
                uno::Sequence< beans::Property > aProps
                    = m_pContent->getProperties( m_xEnv );
                m_pProps.reset(new uno::Sequence< beans::Property >( aProps ));
            }
            catch ( uno::RuntimeException const & )
            {
                throw;
            }
            catch ( uno::Exception const & )
            {
                m_pProps.reset(new uno::Sequence< beans::Property >( 0 ));
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
                        sal_Int32 nPos = m_pProps->getLength();
                        m_pProps->realloc( nPos + nAddProps );

                        std::copy(rAddProps.begin(), rAddProps.end(),
                                  std::next(m_pProps->begin(), nPos));
                    }
                }
            }
        }
    }
    return *m_pProps;
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
    osl::MutexGuard aGuard( m_aMutex );
    m_pProps.reset();
}


bool PropertySetInfo::queryProperty(
    const OUString& rName, beans::Property& rProp )
{
    osl::MutexGuard aGuard( m_aMutex );

    getProperties();

    const beans::Property* pProps = m_pProps->getConstArray();
    sal_Int32 nCount = m_pProps->getLength();
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
    const uno::Reference< css::ucb::XCommandEnvironment >& rxEnv,
    ContentImplHelper* pContent )
: m_xEnv( rxEnv ),
  m_pContent( pContent )
{
}


// virtual
CommandProcessorInfo::~CommandProcessorInfo()
{
}


// XCommandInfo methods.


// virtual
uno::Sequence< css::ucb::CommandInfo > SAL_CALL
CommandProcessorInfo::getCommands()
{
    if ( !m_pCommands )
    {
        osl::MutexGuard aGuard( m_aMutex );
        if ( !m_pCommands )
        {

            // Get info for commands.


            try
            {
                uno::Sequence< css::ucb::CommandInfo > aCmds
                    = m_pContent->getCommands( m_xEnv );
                m_pCommands.reset(new uno::Sequence< css::ucb::CommandInfo >( aCmds ));
            }
            catch ( uno::RuntimeException const & )
            {
                throw;
            }
            catch ( uno::Exception const & )
            {
                m_pCommands.reset(new uno::Sequence< css::ucb::CommandInfo >( 0 ));
            }
        }
    }
    return *m_pCommands;
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
    osl::MutexGuard aGuard( m_aMutex );
    m_pCommands.reset();
}


bool CommandProcessorInfo::queryCommand(
    const OUString& rName,
    css::ucb::CommandInfo& rCommand )
{
    osl::MutexGuard aGuard( m_aMutex );

    getCommands();

    const css::ucb::CommandInfo* pCommands
        = m_pCommands->getConstArray();
    sal_Int32 nCount = m_pCommands->getLength();
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
    osl::MutexGuard aGuard( m_aMutex );

    getCommands();

    const css::ucb::CommandInfo* pCommands = m_pCommands->getConstArray();
    sal_Int32 nCount = m_pCommands->getLength();
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
