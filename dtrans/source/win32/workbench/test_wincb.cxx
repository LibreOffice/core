/*************************************************************************
 *
 *  $RCSfile: test_wincb.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: armin $ $Date: 2001-03-08 12:11:56 $
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


//_________________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________________


#include "..\ImplHelper.hxx"

//_________________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________________

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

#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
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

#include <memory>

#include <process.h>

//-------------------------------------------------------------
// my defines
//-------------------------------------------------------------

#define TEST_CLIPBOARD
#define RDB_SYSPATH  "d:\\projects\\src616\\dtrans\\wntmsci7\\bin\\applicat.rdb"
#define WINCLIPBOARD_SERVICE_NAME L"com.sun.star.datatransfer.clipboard.SystemClipboard"
#define  WRITE_CB
#define EVT_MANUAL_RESET     TRUE
#define EVT_INIT_NONSIGNALED FALSE
#define EVT_NONAME           ""

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

//------------------------------------------------------------
//  globales
//------------------------------------------------------------

Reference< XTransferable > rXTransfRead;
HANDLE  g_hEvtThreadWakeup;

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

    virtual Any SAL_CALL getTransferData( const DataFlavor& aFlavor ) throw(UnsupportedFlavorException, IOException, RuntimeException);
    virtual Sequence< DataFlavor > SAL_CALL getTransferDataFlavors(  ) throw(RuntimeException);
    virtual sal_Bool SAL_CALL isDataFlavorSupported( const DataFlavor& aFlavor ) throw(RuntimeException);

    //-------------------------------------------------
    // XClipboardOwner
    //-------------------------------------------------

    virtual void SAL_CALL lostOwnership( const Reference< XClipboard >& xClipboard, const Reference< XTransferable >& xTrans ) throw(RuntimeException);

private:
    Sequence< DataFlavor > m_seqDFlv;
    OUString               m_Data;
};

//----------------------------------------------------------------
//  ctor
//----------------------------------------------------------------

CTransferable::CTransferable( ) :
    m_seqDFlv( 1 ),
    m_Data( L"Ich habe mir ein neues Fahrrad gekauft!" )
{
    DataFlavor df;

    /*
    df.MimeType = L"text/plain; charset=unicode";
    df.DataType = getCppuType( ( OUString* )0 );

    m_seqDFlv[0] = df;
    */

    //df.MimeType = L"text/plain; charset=windows1252";
    df.MimeType = L"text/html";
    df.DataType = getCppuType( ( Sequence< sal_Int8 >* )0 );

    m_seqDFlv[0] = df;
}

//----------------------------------------------------------------
//  getTransferData
//----------------------------------------------------------------

Any SAL_CALL CTransferable::getTransferData( const DataFlavor& aFlavor )
    throw(UnsupportedFlavorException, IOException, RuntimeException)
{
    Any anyData;

    /*if ( aFlavor == m_seqDFlv[0] )
    {
        anyData = makeAny( m_Data );
    }
    else*/ if ( aFlavor == m_seqDFlv[0] )
    {
        OString aStr( m_Data.getStr( ), m_Data.getLength( ), 1252 );
        Sequence< sal_Int8 > sOfChars( aStr.getLength( ) );
        sal_Int32 lenStr = aStr.getLength( );

        for ( sal_Int32 i = 0; i < lenStr; ++i )
            sOfChars[i] = aStr[i];

        anyData = makeAny( sOfChars );
    }

    return anyData;
}

//----------------------------------------------------------------
//  getTransferDataFlavors
//----------------------------------------------------------------

Sequence< DataFlavor > SAL_CALL CTransferable::getTransferDataFlavors(  )
    throw(RuntimeException)
{
    return m_seqDFlv;
}

//----------------------------------------------------------------
//  isDataFlavorSupported
//----------------------------------------------------------------

sal_Bool SAL_CALL CTransferable::isDataFlavorSupported( const DataFlavor& aFlavor )
    throw(RuntimeException)
{
    sal_Int32 nLength = m_seqDFlv.getLength( );
    sal_Bool bRet     = sal_False;

    for ( sal_Int32 i = 0; i < nLength; ++i )
    {
        if ( m_seqDFlv[i] == aFlavor )
        {
            bRet = sal_True;
            break;
        }
    }

    return bRet;
}

//----------------------------------------------------------------
//  lostOwnership
//----------------------------------------------------------------

void SAL_CALL CTransferable::lostOwnership( const Reference< XClipboard >& xClipboard, const Reference< XTransferable >& xTrans )
    throw(RuntimeException)
{
}

//################################################################
// a thread in another apartment to test apartment transparency

unsigned int _stdcall ThreadProc(LPVOID pParam)
{
    // setup another apartment
    HRESULT hr = CoInitialize( NULL );
    //HRESULT hr = CoInitializeEx( NULL, COINIT_MULTITHREADED );

    WaitForSingleObject( g_hEvtThreadWakeup, INFINITE );

    Sequence< DataFlavor > flavorList = rXTransfRead->getTransferDataFlavors( );
    sal_Int32 nFlavors = flavorList.getLength( );
    OUString  mimeType;
    OUString  hpName;
    for ( sal_Int32 i = 0; i < nFlavors; ++i )
    {
        mimeType = flavorList[i].MimeType;
        hpName   = flavorList[i].HumanPresentableName;
    }

    /*
    try
    {
        DataFlavor flv;
        flv.MimeType = L"text/html";
        flv.DataType = getCppuType( ( Sequence< sal_Int8 >* )0 );

        rXTransfRead->getTransferData( flv );
    }
    catch( UnsupportedFlavorException& ex )
    {
        OSL_ENSURE( false, "### DataFlavor not supported" );
    }
    */

    CoUninitialize( );

    return 0;
}

//################################################################

//----------------------------------------------------------------
//  main
//----------------------------------------------------------------

int SAL_CALL main( int nArgc, char* Argv[] )
{
    // create a multi-threaded apartment; we can test only
    // with a multithreaded apartment because for a single
    // threaded apartment we need a message loop to deliver
    // messages to our XTDataObject
    //HRESULT hr = CoInitializeEx( NULL, COINIT_MULTITHREADED );
    HRESULT hr = CoInitialize( NULL );


#ifndef WRITE_CB

    g_hEvtThreadWakeup = CreateEvent( 0,
                                      EVT_MANUAL_RESET,
                                      EVT_INIT_NONSIGNALED,
                                      EVT_NONAME );

    unsigned uThreadId;
    HANDLE   hThread;

    // create a thread in another apartment
    hThread = (void*)_beginthreadex( NULL, 0, ThreadProc, NULL, 0, &uThreadId );

#endif

    //-------------------------------------------------
    // get the global service-manager
    //-------------------------------------------------
    OUString rdbName = OUString( RTL_CONSTASCII_USTRINGPARAM( RDB_SYSPATH ) );
    Reference< XMultiServiceFactory > g_xFactory( createRegistryServiceFactory( rdbName ) );

    // Print a message if an error occured.
    if ( !g_xFactory.is( ) )
    {
        OSL_ENSURE(sal_False, "Can't create RegistryServiceFactory");
        return(-1);
    }

    //-------------------------------------------------
    // try to get an Interface to a XFilePicker Service
    //-------------------------------------------------

    Reference< XTransferable > rXTransf( static_cast< XTransferable* >( new CTransferable ) );

    Reference< XClipboard > xClipboard( g_xFactory->createInstance( OUString( WINCLIPBOARD_SERVICE_NAME ) ), UNO_QUERY );
    if ( !xClipboard.is( ) )
    {
        OSL_ENSURE( sal_False, "Error creating FolderPicker Service" );
        return(-1);
    }

    Reference< XTypeProvider > rXTypProv( xClipboard, UNO_QUERY );

    if ( rXTypProv.is( ) )
    {
        Sequence< Type >     seqType = rXTypProv->getTypes( );
        sal_Int32 nLen = seqType.getLength( );
        for ( sal_Int32 i = 0; i < nLen; ++nLen )
        {
            Type nxtType = seqType[i];
        }

        Sequence< sal_Int8 > seqInt8 = rXTypProv->getImplementationId( );
    }

#ifdef WRITE_CB

    xClipboard->setContents( rXTransf, Reference< XClipboardOwner >( rXTransf, UNO_QUERY )  );

#else

    rXTransfRead = xClipboard->getContents( );

    // wakeup the thread and waiting util it ends
    SetEvent( g_hEvtThreadWakeup );

    // wait for thread end
    WaitForSingleObject( hThread, INFINITE );

    // destroy the transferable explicitly
    rXTransfRead = Reference< XTransferable>( );

#endif

    // destroy the clipboard
    xClipboard = Reference< XClipboard >( );

    CoUninitialize( );

    //--------------------------------------------------
    // shutdown the service manager
    //--------------------------------------------------

    // Cast factory to XComponent
    Reference< XComponent > xComponent( g_xFactory, UNO_QUERY );

    if ( !xComponent.is() )
        OSL_ENSURE(sal_False, "Error shuting down");

    // Dispose and clear factory
    xComponent->dispose();
    g_xFactory.clear();
    g_xFactory = Reference< XMultiServiceFactory >();

    return 0;
}
