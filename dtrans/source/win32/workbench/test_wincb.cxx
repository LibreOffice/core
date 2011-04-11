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
#include "precompiled_dtrans.hxx"


//_________________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________________


#include "..\misc\ImplHelper.hxx"

//_________________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________________
#include <cppuhelper/servicefactory.hxx>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardOwner.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardNotifier.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardEx.hpp>
#include <com/sun/star/datatransfer/clipboard/XFlushableClipboard.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>
#include <osl/diagnose.h>

#include <stdio.h>
#if defined _MSC_VER
#pragma warning(push,1)
#endif
#include <windows.h>
#include <objbase.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

#include <memory>

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
    m_Data( OUString(RTL_CONSTASCII_USTRINGPARAM("Ich habe mir ein neues Fahrrad gekauft!")) )
{
    DataFlavor df;

    //df.MimeType = L"text/plain;charset=utf-16";
    //df.DataType = getCppuType( ( OUString* )0 );

    df.MimeType = L"text/plain;charset=Windows1252";
    df.DataType = getCppuType( (Sequence< sal_Int8 >*)0 );

    m_FlavorList[0] = df;
}

//----------------------------------------------------------------
//  getTransferData
//----------------------------------------------------------------

Any SAL_CALL CTransferable::getTransferData( const DataFlavor& aFlavor )
    throw(UnsupportedFlavorException, IOException, RuntimeException)
{
    Any anyData;

    /*
    if ( aFlavor.MimeType == m_FlavorList[0].MimeType )
        anyData = makeAny( m_Data );
    */
    if ( aFlavor.MimeType.equalsIgnoreCase( m_FlavorList[0].MimeType ) )
    {
        OString text(
            m_Data.getStr( ),
            m_Data.getLength( ),
            RTL_TEXTENCODING_ASCII_US );

        Sequence< sal_Int8 > textStream( text.getLength( ) + 1 );

        rtl_copyMemory( textStream.getArray( ), text.getStr( ), textStream.getLength( ) );

        anyData = makeAny( textStream );
    }
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

    char buff[6];

    LCID lcid = MAKELCID( MAKELANGID( LANG_GERMAN, SUBLANG_GERMAN ), SORT_DEFAULT );

    BOOL bValid = IsValidLocale( lcid, LCID_SUPPORTED );
    GetLocaleInfoA( lcid, LOCALE_IDEFAULTANSICODEPAGE, buff, sizeof( buff ) );

    //-------------------------------------------------
    // get the global service-manager
    //-------------------------------------------------

    OUString rdbName = OUString( RTL_CONSTASCII_USTRINGPARAM( RDB_SYSPATH ) );
    Reference< XMultiServiceFactory > g_xFactory( createRegistryServiceFactory( rdbName ) );

    // Print a message if an error occurred.
    if ( !g_xFactory.is( ) )
    {
        OSL_FAIL("Can't create RegistryServiceFactory");
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
        OSL_FAIL( "Error creating Clipboard Service" );
        return(-1);
    }

    Reference< XClipboardNotifier > xClipNotifier( xClipboard, UNO_QUERY );
    Reference< XClipboardListener > rXClipListener( static_cast< XClipboardListener* >( new CClipboardListener() ) );
    xClipNotifier->addClipboardListener( rXClipListener );

    MessageBox( NULL, TEXT("Go"), TEXT("INFO"), MB_OK|MB_ICONINFORMATION);

    // set new clipboard content
    xClipboard->setContents( rXTransf, Reference< XClipboardOwner >( rXTransf, UNO_QUERY )  );

    /*
    MessageBox( NULL, TEXT("Clear content"), TEXT("INFO"), MB_OK|MB_ICONINFORMATION);

    Reference< XClipboardOwner > rXClipOwner;
    Reference< XTransferable >   rXEmptyTransf;
    xClipboard->setContents( rXEmptyTransf, rXClipOwner );
    */

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
        OSL_FAIL("Error shuting down");

    // Dispose and clear factory
    xComponent->dispose();
    xComponent = Reference< XComponent >( );

    g_xFactory.clear();
    g_xFactory = Reference< XMultiServiceFactory >();

    CoUninitialize( );

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
