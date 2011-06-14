/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
 /*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"
#include "filcmd.hxx"
#include "shell.hxx"
#include "prov.hxx"


using namespace fileaccess;
using namespace com::sun::star;
using namespace com::sun::star::ucb;


XCommandInfo_impl::XCommandInfo_impl( shell* pMyShell )
    : m_pMyShell( pMyShell ),
      m_xProvider( pMyShell->m_pProvider )
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
  throw( uno::RuntimeException )
{
    uno::Any aRet = cppu::queryInterface( rType,
                                          SAL_STATIC_CAST( XCommandInfo*,this) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


uno::Sequence< CommandInfo > SAL_CALL
XCommandInfo_impl::getCommands(
    void )
    throw( uno::RuntimeException )
{
    return m_pMyShell->m_sCommandInfo;
}


CommandInfo SAL_CALL
XCommandInfo_impl::getCommandInfoByName(
    const rtl::OUString& aName )
    throw( UnsupportedCommandException,
           uno::RuntimeException)
{
    for( sal_Int32 i = 0; i < m_pMyShell->m_sCommandInfo.getLength(); i++ )
        if( m_pMyShell->m_sCommandInfo[i].Name == aName )
            return m_pMyShell->m_sCommandInfo[i];

    throw UnsupportedCommandException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );
}


CommandInfo SAL_CALL
XCommandInfo_impl::getCommandInfoByHandle(
    sal_Int32 Handle )
    throw( UnsupportedCommandException,
           uno::RuntimeException )
{
    for( sal_Int32 i = 0; i < m_pMyShell->m_sCommandInfo.getLength(); ++i )
        if( m_pMyShell->m_sCommandInfo[i].Handle == Handle )
            return m_pMyShell->m_sCommandInfo[i];

    throw UnsupportedCommandException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ), uno::Reference< uno::XInterface >() );
}


sal_Bool SAL_CALL
XCommandInfo_impl::hasCommandByName(
    const rtl::OUString& aName )
    throw( uno::RuntimeException )
{
    for( sal_Int32 i = 0; i < m_pMyShell->m_sCommandInfo.getLength(); ++i )
        if( m_pMyShell->m_sCommandInfo[i].Name == aName )
            return true;

    return false;
}


sal_Bool SAL_CALL
XCommandInfo_impl::hasCommandByHandle(
    sal_Int32 Handle )
    throw( uno::RuntimeException )
{
    for( sal_Int32 i = 0; i < m_pMyShell->m_sCommandInfo.getLength(); ++i )
        if( m_pMyShell->m_sCommandInfo[i].Handle == Handle )
            return true;

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
