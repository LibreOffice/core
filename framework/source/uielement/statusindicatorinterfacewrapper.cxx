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
#include "precompiled_framework.hxx"

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________
#include <uielement/statusindicatorinterfacewrapper.hxx>
#include <uielement/progressbarwrapper.hxx>
#include <threadhelp/resetableguard.hxx>

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#include <vcl/svapp.hxx>
#include <vos/mutex.hxx>

using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;

namespace framework
{


StatusIndicatorInterfaceWrapper::StatusIndicatorInterfaceWrapper(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& rStatusIndicatorImpl ) :
    m_xStatusIndicatorImpl( rStatusIndicatorImpl )
{
}

StatusIndicatorInterfaceWrapper::~StatusIndicatorInterfaceWrapper()
{
}


void SAL_CALL StatusIndicatorInterfaceWrapper::start(
    const ::rtl::OUString& sText,
    sal_Int32              nRange )
throw( ::com::sun::star::uno::RuntimeException )
{
    Reference< XComponent > xComp( m_xStatusIndicatorImpl );
    if ( xComp.is() )
    {
        ProgressBarWrapper* pProgressBar = (ProgressBarWrapper*)xComp.get();
        if ( pProgressBar )
            pProgressBar->start( sText, nRange );
    }
}

void SAL_CALL StatusIndicatorInterfaceWrapper::end()
throw( ::com::sun::star::uno::RuntimeException )
{
    Reference< XComponent > xComp( m_xStatusIndicatorImpl );
    if ( xComp.is() )
    {
        ProgressBarWrapper* pProgressBar = (ProgressBarWrapper*)xComp.get();
        if ( pProgressBar )
            pProgressBar->end();
    }
}

void SAL_CALL StatusIndicatorInterfaceWrapper::reset()
throw( ::com::sun::star::uno::RuntimeException )
{
    Reference< XComponent > xComp( m_xStatusIndicatorImpl );
    if ( xComp.is() )
    {
        ProgressBarWrapper* pProgressBar = (ProgressBarWrapper*)xComp.get();
        if ( pProgressBar )
            pProgressBar->reset();
    }
}

void SAL_CALL StatusIndicatorInterfaceWrapper::setText(
    const ::rtl::OUString& sText )
throw( ::com::sun::star::uno::RuntimeException )
{
    Reference< XComponent > xComp( m_xStatusIndicatorImpl );
    if ( xComp.is() )
    {
        ProgressBarWrapper* pProgressBar = (ProgressBarWrapper*)xComp.get();
        if ( pProgressBar )
            pProgressBar->setText( sText );
    }
}

void SAL_CALL StatusIndicatorInterfaceWrapper::setValue(
    sal_Int32 nValue )
throw( ::com::sun::star::uno::RuntimeException )
{
    Reference< XComponent > xComp( m_xStatusIndicatorImpl );
    if ( xComp.is() )
    {
        ProgressBarWrapper* pProgressBar = (ProgressBarWrapper*)xComp.get();
        if ( pProgressBar )
            pProgressBar->setValue( nValue );
    }
}

} // namespace framework

