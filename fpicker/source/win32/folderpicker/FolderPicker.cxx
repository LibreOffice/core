/*************************************************************************
 *
 *  $RCSfile: FolderPicker.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: tra $ $Date: 2001-06-28 11:15:52 $
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

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _FOLDERPICKER_HXX_
#include "folderpicker.hxx"
#endif

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif

#ifndef _WINFOPIMPL_HXX_
#include "WinFOPImpl.hxx"
#endif

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using com::sun::star::uno::Reference;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::XInterface;
using com::sun::star::lang::XMultiServiceFactory;
using com::sun::star::lang::XServiceInfo;
using com::sun::star::lang::DisposedException;
using com::sun::star::lang::IllegalArgumentException;
using rtl::OUString;
using osl::MutexGuard;

using namespace cppu;
using namespace com::sun::star::ui::dialogs;

//------------------------------------------------------------------------
// defines
//------------------------------------------------------------------------

#define FOLDERPICKER_IMPL_NAME  "com.sun.star.ui.dialogs.Win32FolderPicker"

//------------------------------------------------------------------------
// helper functions
//------------------------------------------------------------------------

namespace
{
    Sequence< OUString > SAL_CALL FolderPicker_getSupportedServiceNames()
    {
        Sequence< OUString > aRet(2);
        aRet[0] = OUString::createFromAscii("com.sun.star.ui.dialogs.FolderPicker");
        aRet[1] = OUString::createFromAscii("com.sun.star.ui.dialogs.SystemFolderPicker");
        return aRet;
    }
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

CFolderPicker::CFolderPicker( const Reference< XMultiServiceFactory >& xServiceMgr ) :
    m_xServiceMgr( xServiceMgr )
{
    m_pFolderPickerImpl = std::auto_ptr< CWinFolderPickerImpl > ( new CWinFolderPickerImpl( this ) );
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CFolderPicker::setTitle( const OUString& aTitle ) throw( RuntimeException )
{
    OSL_ASSERT( m_pFolderPickerImpl.get( ) );
    MutexGuard aGuard( m_aMutex );
    m_pFolderPickerImpl->setTitle( aTitle );
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CFolderPicker::setDisplayDirectory( const OUString& aDirectory )
    throw( IllegalArgumentException, RuntimeException )
{
    OSL_ASSERT( m_pFolderPickerImpl.get( ) );
    MutexGuard aGuard( m_aMutex );
    m_pFolderPickerImpl->setDisplayDirectory( aDirectory );
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

OUString SAL_CALL CFolderPicker::getDisplayDirectory( )
    throw( RuntimeException )
{
    OSL_ASSERT( m_pFolderPickerImpl.get( ) );
    MutexGuard aGuard( m_aMutex );
    return m_pFolderPickerImpl->getDisplayDirectory( );
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

OUString SAL_CALL CFolderPicker::getDirectory( ) throw( RuntimeException )
{
    OSL_ASSERT( m_pFolderPickerImpl.get( ) );
    MutexGuard aGuard( m_aMutex );
    return m_pFolderPickerImpl->getDirectory( );
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

void SAL_CALL CFolderPicker::setDescription( const OUString& aDescription ) throw( RuntimeException )
{
    OSL_ASSERT( m_pFolderPickerImpl.get( ) );
    MutexGuard aGuard( m_aMutex );
    m_pFolderPickerImpl->setDescription( aDescription );
}

//-----------------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------------------

sal_Int16 SAL_CALL CFolderPicker::execute( )
    throw( RuntimeException )
{
    OSL_ASSERT( m_pFolderPickerImpl.get( ) );

    // we should not block in this call else
    // in the case of an event the client can'tgetPImplFromHandle( hWnd )
    // call another function an we run into a
    // deadlock !!!!!
    return m_pFolderPickerImpl->execute( );
}

// -------------------------------------------------
// XServiceInfo
// -------------------------------------------------

OUString SAL_CALL CFolderPicker::getImplementationName(  )
    throw( RuntimeException )
{
    return OUString::createFromAscii( FOLDERPICKER_IMPL_NAME );
}

// -------------------------------------------------
//  XServiceInfo
// -------------------------------------------------

sal_Bool SAL_CALL CFolderPicker::supportsService( const OUString& ServiceName )
    throw( RuntimeException )
{
    Sequence < OUString > SupportedServicesNames = FolderPicker_getSupportedServiceNames();

    for ( sal_Int32 n = SupportedServicesNames.getLength(); n--; )
        if (SupportedServicesNames[n].compareTo(ServiceName) == 0)
            return sal_True;

    return sal_False;
}

// -------------------------------------------------
//  XServiceInfo
// -------------------------------------------------

Sequence< OUString > SAL_CALL CFolderPicker::getSupportedServiceNames(   )
    throw( RuntimeException )
{
    return FolderPicker_getSupportedServiceNames();
}

//------------------------------------------------
// overwrite base class method, which is called
// by base class dispose function
//------------------------------------------------

void SAL_CALL CFolderPicker::disposing()
{
}

