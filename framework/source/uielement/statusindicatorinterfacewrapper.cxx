/*************************************************************************
 *
 *  $RCSfile: statusindicatorinterfacewrapper.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-09-09 17:12:01 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_UIELEMENT_STATUSINDICATORINTERFACEWRAPPER_HXX_
#include <uielement/statusindicatorinterfacewrapper.hxx>
#endif

#ifndef _FRAMEWORK_UIELEMENT_PROGRESSBARWRAPPER_HXX_
#include <uielement/progressbarwrapper.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_RESETABLEGUARD_HXX_
#include <threadhelp/resetableguard.hxx>
#endif

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
