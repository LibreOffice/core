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

#include "filcmd.hxx"
#include "shell.hxx"
#include "prov.hxx"

using namespace fileaccess;
using namespace com::sun::star;
using namespace com::sun::star::ucb;

#if OSL_DEBUG_LEVEL > 0
#define THROW_WHERE SAL_WHERE
#else
#define THROW_WHERE ""
#endif

XCommandInfo_impl::XCommandInfo_impl( shell* pMyShell )
    : m_pMyShell( pMyShell )
{
}

XCommandInfo_impl::~XCommandInfo_impl()
{
}


void SAL_CALL
XCommandInfo_impl::acquire(
                 void )
  throw()
{
  OWeakObject::acquire();
}


void SAL_CALL
XCommandInfo_impl::release(
    void )
  throw()
{
    OWeakObject::release();
}


uno::Any SAL_CALL
XCommandInfo_impl::queryInterface(
                    const uno::Type& rType )
  throw( uno::RuntimeException, std::exception )
{
    uno::Any aRet = cppu::queryInterface( rType,
                                          (static_cast< XCommandInfo* >(this)) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


uno::Sequence< CommandInfo > SAL_CALL
XCommandInfo_impl::getCommands(
    void )
    throw( uno::RuntimeException, std::exception )
{
    return m_pMyShell->m_sCommandInfo;
}


CommandInfo SAL_CALL
XCommandInfo_impl::getCommandInfoByName(
    const OUString& aName )
    throw( UnsupportedCommandException,
           uno::RuntimeException, std::exception)
{
    for( sal_Int32 i = 0; i < m_pMyShell->m_sCommandInfo.getLength(); i++ )
        if( m_pMyShell->m_sCommandInfo[i].Name == aName )
            return m_pMyShell->m_sCommandInfo[i];

    throw UnsupportedCommandException( THROW_WHERE );
}


CommandInfo SAL_CALL
XCommandInfo_impl::getCommandInfoByHandle(
    sal_Int32 Handle )
    throw( UnsupportedCommandException,
           uno::RuntimeException, std::exception )
{
    for( sal_Int32 i = 0; i < m_pMyShell->m_sCommandInfo.getLength(); ++i )
        if( m_pMyShell->m_sCommandInfo[i].Handle == Handle )
            return m_pMyShell->m_sCommandInfo[i];

    throw UnsupportedCommandException( THROW_WHERE );
}


sal_Bool SAL_CALL
XCommandInfo_impl::hasCommandByName(
    const OUString& aName )
    throw( uno::RuntimeException, std::exception )
{
    for( sal_Int32 i = 0; i < m_pMyShell->m_sCommandInfo.getLength(); ++i )
        if( m_pMyShell->m_sCommandInfo[i].Name == aName )
            return true;

    return false;
}


sal_Bool SAL_CALL
XCommandInfo_impl::hasCommandByHandle(
    sal_Int32 Handle )
    throw( uno::RuntimeException, std::exception )
{
    for( sal_Int32 i = 0; i < m_pMyShell->m_sCommandInfo.getLength(); ++i )
        if( m_pMyShell->m_sCommandInfo[i].Handle == Handle )
            return true;

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
