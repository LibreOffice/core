/*************************************************************************
 *
 *  $RCSfile: test_wincb.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: tra $ $Date: 2001-03-14 14:54:58 $
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


#include "..\misc\ImplHelper.hxx"

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

//-------------------------------------------------------------
// my defines
//-------------------------------------------------------------

#define TEST_CLIPBOARD
#define RDB_SYSPATH  "d:\\projects\\src623\\dtrans\\wntmsci7\\bin\\applicat.rdb"
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

class CClipboardListener : public WeakImplHelper1 < XClipboardListener >
{
public:
    ~CClipboardListener( );

    //-------------------------------------------------
    // XClipboardListener
    //-------------------------------------------------

    virtual void SAL_CALL disposing( const EventObject& Source ) throw(RuntimeException);
    virtual void SAL_CALL changedContents( const ClipboardEvent& event ) throw( RuntimeException );
};

CClipboardListener::~CClipboardListener( )
{
}

void SAL_CALL CClipboardListener::disposing( const EventObject& Source ) throw(RuntimeException)
{

}

void SAL_CALL CClipboardListener::changedContents( const ClipboardEvent& event ) throw( RuntimeException )
{
    //MessageBox( NULL, TEXT("Clipboard content changed"), TEXT("Info"), MB_OK | MB_ICONINFORMATION );
}

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

    virtual Sequence< DataFlavor > SAL_CALL getTransferDataFlavors(  ) throw(RuntimeException);

    virtual sal_Bool SAL_CALL isDataFlavorSupported( const DataFlavor& aFlavor ) throw(RuntimeException);

    //-------------------------------------------------
    // XClipboardOwner
    //-------------------------------------------------

    virtual void SAL_CALL lostOwnership( const Reference< XClipboard >& xClipboard, const Reference< XTransferable >& xTrans )
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
        anyData = makeAny( m_Data );

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
    const Reference< XClipboard >& xClipboard, const Reference< XTransferable >& xTrans )
    throw(RuntimeException)
{
    //MessageBox( NULL, TEXT("No longer clipboard owner"), TEXT("Info"), MB_OK | MB_ICONINFORMATION );
}

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

    Reference< XClipboard >
        xClipboard( g_xFactory->createInstance( OUString( WINCLIPBOARD_SERVICE_NAME ) ), UNO_QUERY );
    if ( !xClipboard.is( ) )
    {
        OSL_ENSURE( sal_False, "Error creating Clipboard Service" );
        return(-1);
    }

    Reference< XClipboardNotifier > xClipNotifier( xClipboard, UNO_QUERY );
    Reference< XClipboardListener > rXClipListener( static_cast< XClipboardListener* >( new CClipboardListener() ) );
    xClipNotifier->addClipboardListener( rXClipListener );


    // set new clipboard content
    xClipboard->setContents( rXTransf, Reference< XClipboardOwner >( rXTransf, UNO_QUERY )  );


    MessageBox( NULL, TEXT("Stop"), TEXT("INFO"), MB_OK|MB_ICONINFORMATION);

    // flush the clipboard content
    Reference< XFlushableClipboard > rXFlushableClip( xClipboard, UNO_QUERY );
    rXFlushableClip->flushClipboard( );
    rXFlushableClip = Reference< XFlushableClipboard >( );

    xClipNotifier->removeClipboardListener( rXClipListener );
    rXClipListener = Reference< XClipboardListener >( );
    xClipNotifier  = Reference< XClipboardNotifier >( );

    //--------------------------------------------------
    // shutdown the service manager
    //--------------------------------------------------

    // Cast factory to XComponent
    Reference< XComponent > xComponent( g_xFactory, UNO_QUERY );

    if ( !xComponent.is() )
        OSL_ENSURE(sal_False, "Error shuting down");

    // Dispose and clear factory
    xComponent->dispose();
    xComponent = Reference< XComponent >( );

    g_xFactory.clear();
    g_xFactory = Reference< XMultiServiceFactory >();

    CoUninitialize( );

    return 0;
}
