/*************************************************************************
 *
 *  $RCSfile: test_wincb.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: tra $ $Date: 2001-07-26 11:20:56 $
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

//----------------------------------
//  interface includes
//----------------------------------

#include "..\misc\ImplHelper.hxx"

#ifndef _CPPUHELPER_SERVICEFACTORY_HXX_
#include <cppuhelper/servicefactory.hxx>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_XTRANSFERABLE_HPP_
#include <com/sun/star/datatransfer/XTransferable.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XCLIPBOARDOWNER_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboardOwner.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XCLIPBOARDNOTIFIER_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboardNotifier.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XCLIPBOARDEX_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboardEx.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XFLUSHABLECLIPBOARD_HPP_
#include <com/sun/star/datatransfer/clipboard/XFlushableClipboard.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif

#ifndef _RTL_USTRING_
#include <rtl/ustring>
#endif

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#include <stdio.h>
#include <windows.h>
#include <objbase.h>

#include <stl/memory>

#include <process.h>

//------------------------------------------------------------
//  namesapces
//------------------------------------------------------------

using namespace ::rtl;
using namespace ::std;
using namespace ::cppu;
using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::datatransfer::clipboard;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;

//-------------------------------------------------------------
// my defines
//-------------------------------------------------------------

#define TEST_CLIPBOARD
#define  WRITE_CB

const char RDB_SYSPATH[] = "d:\\projects\\src633\\dtrans\\wntmsci7\\bin\\applicat.rdb";
const OUString WINCLIPBOARD_SERVICE_NAME = OUString::createFromAscii( "com.sun.star.datatransfer.clipboard.SystemClipboard" );

const bool EVT_MANUAL_RESET     = true;
const bool EVT_AUTO_RESET       = false;
const bool EVT_INIT_NONSIGNALED = false;
const bool EVT_INIT_SIGNALED    = true;
const sal_Int32 MAX_LOOP        = 1000;

char EVT_NONAME[] = "";

enum APARTMENT_MODEL
{
    STA = 0,
    MTA
};

//------------------------------------------------------------
//  globales
//------------------------------------------------------------

Reference< XTransferable > g_xTransferable;
HANDLE g_HandleArray[2];
HANDLE g_hEvtThreadWakeup;
Reference< XClipboard > g_xClipboard;

//------------------------------------------------------------
//
//------------------------------------------------------------

class CClipboardListener : public WeakImplHelper1 < XClipboardListener >
{
public:
    virtual void SAL_CALL disposing( const EventObject& Source )
        throw(RuntimeException)
    {
    }

    //-------------------------------------------------
    // XClipboardListener
    //-------------------------------------------------

    virtual void SAL_CALL changedContents( const ClipboardEvent& event )
        throw( RuntimeException )
    {
        OSL_ENSURE( sal_False, "clipboard content changed" );
    }
};

//------------------------------------------------------------
//
//------------------------------------------------------------

class CTransferable : public WeakImplHelper2< XClipboardOwner, XTransferable >
{
public:
    CTransferable( );

    //-------------------------------------------------
    // XTransferable
    //-------------------------------------------------

    virtual Any SAL_CALL getTransferData( const DataFlavor& aFlavor )
        throw(UnsupportedFlavorException, IOException, RuntimeException);

    virtual Sequence< DataFlavor > SAL_CALL getTransferDataFlavors(  )
        throw(RuntimeException);

    virtual sal_Bool SAL_CALL isDataFlavorSupported( const DataFlavor& aFlavor )
        throw(RuntimeException);

    //-------------------------------------------------
    // XClipboardOwner
    //-------------------------------------------------

    virtual void SAL_CALL lostOwnership(
        const Reference< XClipboard >& xClipboard, const Reference< XTransferable >& xTransferable )
        throw(RuntimeException);

private:
    Sequence< DataFlavor > m_FlavorList;
    OUString               m_Data;
};

//----------------------------------------------------------------
//  ctor
//----------------------------------------------------------------

CTransferable::CTransferable( ) :
    m_FlavorList( 1 ),
    m_Data( OUString::createFromAscii( "Ich habe mir ein neues Fahrrad gekauft!" ) )
{
    DataFlavor df;

    df.MimeType = L"text/plain;charset=utf-16";
    df.DataType = getCppuType( ( OUString* )0 );

    m_FlavorList[0] = df;
}

//----------------------------------------------------------------
//  getTransferData
//----------------------------------------------------------------

Any SAL_CALL CTransferable::getTransferData( const DataFlavor& aFlavor )
    throw(UnsupportedFlavorException, IOException, RuntimeException)
{
    Any anyData;

    if ( aFlavor.MimeType == m_FlavorList[0].MimeType )
        anyData <<= m_Data;
    else
        throw UnsupportedFlavorException( );

    return anyData;
}

//----------------------------------------------------------------
//  getTransferDataFlavors
//----------------------------------------------------------------

Sequence< DataFlavor > SAL_CALL CTransferable::getTransferDataFlavors(  )
    throw(RuntimeException)
{
    return m_FlavorList;
}

//----------------------------------------------------------------
//  isDataFlavorSupported
//----------------------------------------------------------------

sal_Bool SAL_CALL CTransferable::isDataFlavorSupported( const DataFlavor& aFlavor )
    throw(RuntimeException)
{
    sal_Int32 nLength = m_FlavorList.getLength( );

    for ( sal_Int32 i = 0; i < nLength; ++i )
        if ( m_FlavorList[i].MimeType == aFlavor.MimeType )
            return sal_True;

    return sal_False;
}

//----------------------------------------------------------------
//  lostOwnership
//----------------------------------------------------------------

void SAL_CALL CTransferable::lostOwnership(
    const Reference< XClipboard >& xClipboard, const Reference< XTransferable >& xTransferable )
    throw(RuntimeException)
{
    OSL_ENSURE( sal_False, "lostOwnership" );
}

//------------------------------------------------------------
//
//------------------------------------------------------------

void SetupCOMApartment( APARTMENT_MODEL apm )
{
    // setup another apartment
    HRESULT hr;

    switch ( apm )
    {
        case STA:
            hr = CoInitializeEx( NULL, COINIT_APARTMENTTHREADED );
            OSL_ENSURE( SUCCEEDED( hr ), "CoInitialize Sta failed" );
            break;

        case MTA:
            hr = CoInitializeEx( NULL, COINIT_MULTITHREADED );
            OSL_ENSURE( SUCCEEDED( hr ), "CoInitialize Mta failed" );
            break;

        default:
            CoUninitialize( );
            OSL_ENSURE( false, "invalid apartment model" );
    }
}

//------------------------------------------------------------
//
//------------------------------------------------------------

unsigned int _stdcall ThreadProc(LPVOID pParam)
{
    APARTMENT_MODEL* apm = reinterpret_cast< APARTMENT_MODEL* >( pParam );

    SetupCOMApartment( *apm );

    for ( sal_Int32 i = 0; i < MAX_LOOP; i++ )
    {
        WaitForSingleObject( g_hEvtThreadWakeup, INFINITE );

        try
        {
            if ( g_xTransferable.is( ) )
            {
                Sequence< DataFlavor > aFlavorList = g_xTransferable->getTransferDataFlavors( );
                sal_Int32 nFlavors = aFlavorList.getLength( );

                OSL_ENSURE( nFlavors > 0, "clipboard empty" );

                for ( sal_Int32 i = 0; i < nFlavors; i++ )
                {
                    DataFlavor aFlavor = aFlavorList[i];
                    Any aAny = g_xTransferable->getTransferData( aFlavor );
                    OSL_ENSURE( aAny.hasValue( ), "empty clipboard" );
                }
            }
        }
        catch( UnsupportedFlavorException& )
        {
            OSL_ENSURE( sal_False, "unsupported flavor exception" );
        }
        catch( ... )
        {
            OSL_ENSURE( sal_False, "exception caught" );
        }

        SetEvent( g_HandleArray[0] );
    }

    if ( (STA == *apm) || (MTA == *apm) )
        CoUninitialize( );

    return 0;
}

//----------------------------------------------------------------
//  main
//----------------------------------------------------------------

int SAL_CALL main( int nArgc, char* Argv[] )
{
    if ( nArgc < 4 )
    {
        printf( "Use 0|1 [sta, mta] 0|1 [sta,mta] 0|1 [release XTransferable before|after CoUninitialize]\n" );
        Sleep( 3000 );
        return 0;
    }

    // read parameter
    APARTMENT_MODEL apm = (APARTMENT_MODEL)atoi( Argv[1] );
    SetupCOMApartment( apm );

    g_hEvtThreadWakeup = CreateEventA(
        0, EVT_AUTO_RESET, EVT_INIT_NONSIGNALED, EVT_NONAME );

    OSL_ENSURE( g_hEvtThreadWakeup, "can't create thread wakeup event" );

    g_HandleArray[0] = CreateEventA(
        0, EVT_AUTO_RESET, EVT_INIT_SIGNALED, EVT_NONAME );

    OSL_ENSURE( g_HandleArray[0], "can't create clipboard thread ready event" );

    apm = (APARTMENT_MODEL)atoi( Argv[2] );

    unsigned uThreadId;
    g_HandleArray[1] = (HANDLE)_beginthreadex( NULL, 0, ThreadProc, &apm, 0, &uThreadId );
    OSL_ENSURE( g_HandleArray[1], "cannot create thread" );

    //-------------------------------------------------
    // get the global service-manager
    //-------------------------------------------------
    OUString rdbName = OUString::createFromAscii( RDB_SYSPATH );
    Reference< XMultiServiceFactory > g_xFactory( createRegistryServiceFactory( rdbName ) );
    OSL_ENSURE( g_xFactory.is( ), "can't create RegistryServiceFactory");

    //-------------------------------------------------
    // try to get an Interface to a XFilePicker Service
    //-------------------------------------------------

    g_xClipboard = Reference< XClipboard >( g_xFactory->createInstance( OUString( WINCLIPBOARD_SERVICE_NAME ) ), UNO_QUERY );
    OSL_ENSURE( g_xClipboard.is( ), "error creating clipboard service" );

    Reference< XTransferable > rXTransf( static_cast< XTransferable* >( new CTransferable( ) ) );

    Reference< XClipboardNotifier > xClipboardNotifier( g_xClipboard, UNO_QUERY );
    Reference< XClipboardListener > rXClipListener( static_cast< XClipboardListener* >( new CClipboardListener( ) ), UNO_QUERY );
    OSL_ENSURE( rXClipListener.is( ), "can't create clipboard listener" );

    xClipboardNotifier->addClipboardListener( rXClipListener );

    sal_Bool bContinue = sal_True;

    while ( bContinue )
    {
        DWORD dwResult = WaitForMultipleObjects(
            2, g_HandleArray, false, INFINITE );

        switch( dwResult )
        {
        case WAIT_OBJECT_0:
            g_xTransferable = g_xClipboard->getContents( );
            SetEvent( g_hEvtThreadWakeup );
            break;

        case WAIT_OBJECT_0 + 1:
            // when Argv[3] is true we release the transferable
            // here before CoUninitialize else we can see what
            // happens when the transferable will be destroyed
            // after CoUninitialize
            if ( (bool)atoi( Argv[3] ) )
                g_xTransferable = Reference< XTransferable >( );

            // close the thread handle and end the loop
            CloseHandle( g_HandleArray[1] );
            bContinue = sal_False;
            break;

        default:
            OSL_ENSURE( sal_False, "invalid option" );
        }
    }

    xClipboardNotifier->removeClipboardListener( rXClipListener );
    rXClipListener = Reference< XClipboardListener >( );
    xClipboardNotifier  = Reference< XClipboardNotifier >( );

    //--------------------------------------------------
    // shutdown the service manager
    //--------------------------------------------------

    // Cast factory to XComponent
    Reference< XComponent > xComponent( g_xFactory, UNO_QUERY );
    OSL_ENSURE( xComponent.is( ), "Error shuting down" );

    // Dispose and clear factory
    xComponent->dispose();
    xComponent = Reference< XComponent >( );

    g_xFactory.clear();
    g_xFactory = Reference< XMultiServiceFactory >();

    if ( (STA == apm) || (MTA == apm) )
        CoUninitialize( );

    return 0;
}
