/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
