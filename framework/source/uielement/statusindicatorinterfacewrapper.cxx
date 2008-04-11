/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: statusindicatorinterfacewrapper.cxx,v $
 * $Revision: 1.5 $
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

using namespace rtl;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;

namespace framework
{

//*****************************************************************************************************************
//  XInterface, XTypeProvider
//*****************************************************************************************************************
DEFINE_XINTERFACE_2     (   StatusIndicatorInterfaceWrapper                                 ,
                            cppu::OWeakObject                                               ,
                            DIRECT_INTERFACE( ::com::sun::star::lang::XTypeProvider         ),
                            DIRECT_INTERFACE( ::com::sun::star::task::XStatusIndicator      )
                        )

DEFINE_XTYPEPROVIDER_2  (   StatusIndicatorInterfaceWrapper             ,
                            ::com::sun::star::lang::XTypeProvider       ,
                            ::com::sun::star::task::XStatusIndicator
                        )

StatusIndicatorInterfaceWrapper::StatusIndicatorInterfaceWrapper(
    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent >& rStatusIndicatorImpl ) :
    ::cppu::OWeakObject()
    ,   m_xStatusIndicatorImpl( rStatusIndicatorImpl )
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
