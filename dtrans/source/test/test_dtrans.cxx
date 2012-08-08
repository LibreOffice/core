/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardManager.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardOwner.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardNotifier.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardEx.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>
#include <rtl/ustring.hxx>
#include <osl/diagnose.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//------------------------------------------------------------------------
// my defines
//------------------------------------------------------------------------

#ifdef UNX
#define PATH_SEPERATOR '/'
#else
#define PATH_SEPERATOR '\\'
#endif

#define ENSURE( a, b ) if( !a ) { fprintf( stderr, b "\n" ); exit( -1 ); }
#define TEST( a, b ) fprintf( stderr, "Testing " a ); fprintf( stderr, b ? "passed\n" : "FAILED\n" )
#define PERFORM( a, b ) fprintf( stderr, "Performing " a); b; fprintf( stderr, "done\n" )
#define TRACE( a ) fprintf( stderr, a )

//------------------------------------------------------------------------
//  namespaces
//------------------------------------------------------------------------

using namespace ::rtl;
using namespace ::std;
using namespace ::cppu;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::datatransfer::clipboard;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;

//------------------------------------------------------------------------
//  globals
//------------------------------------------------------------------------

const char * app = NULL;

//------------------------------------------------------------------------
//  ClipboardOwner
//------------------------------------------------------------------------

class ClipboardOwner : public WeakImplHelper1< XClipboardOwner >
{
    Reference< XClipboard >    m_xClipboard;
    Reference< XTransferable > m_xTransferable;

    sal_uInt32 m_nReceivedLostOwnerships;

public:
    ClipboardOwner();

    //--------------------------------------------------------------------
    // XClipboardOwner
    //--------------------------------------------------------------------

    virtual void SAL_CALL lostOwnership( const Reference< XClipboard >& xClipboard, const Reference< XTransferable >& xTrans ) throw(RuntimeException);

    sal_uInt32 receivedLostOwnerships() { return m_nReceivedLostOwnerships; };
    Reference< XClipboard >    lostOwnershipClipboardValue() { return m_xClipboard; }
    Reference< XTransferable > lostOwnershipTransferableValue() { return m_xTransferable; };
};

//------------------------------------------------------------------------
//  ctor
//------------------------------------------------------------------------

ClipboardOwner::ClipboardOwner():
    m_nReceivedLostOwnerships( 0 )
{
}

//------------------------------------------------------------------------
//  lostOwnership
//------------------------------------------------------------------------

void SAL_CALL ClipboardOwner::lostOwnership( const Reference< XClipboard >& xClipboard, const Reference< XTransferable >& xTrans )
    throw(RuntimeException)
{
    m_nReceivedLostOwnerships++;
    m_xClipboard = xClipboard;
    m_xTransferable = xTrans;
}

//------------------------------------------------------------------------
//  ClipboardListener
//------------------------------------------------------------------------

class ClipboardListener : public WeakImplHelper1< XClipboardListener >
{
    Reference< XClipboard >    m_xClipboard;
    Reference< XTransferable > m_xTransferable;

    sal_uInt32 m_nReceivedChangedContentsEvents;

public:
    ClipboardListener();

    //--------------------------------------------------------------------
    // XClipboardOwner
    //--------------------------------------------------------------------

    virtual void SAL_CALL changedContents( const ClipboardEvent& event ) throw(RuntimeException);

    //--------------------------------------------------------------------
    // XEventListener
    //--------------------------------------------------------------------

    virtual void SAL_CALL disposing( const EventObject& event ) throw(RuntimeException);

    sal_uInt32 receivedChangedContentsEvents() { return m_nReceivedChangedContentsEvents; };
    Reference< XClipboard >    changedContentsEventClipboardValue() { return m_xClipboard; }
    Reference< XTransferable > changedContentsEventTransferableValue() { return m_xTransferable; };
};

//------------------------------------------------------------------------
//  ctor
//------------------------------------------------------------------------

ClipboardListener::ClipboardListener():
    m_nReceivedChangedContentsEvents( 0 )
{
}

//------------------------------------------------------------------------
//  changedContents
//------------------------------------------------------------------------

void SAL_CALL ClipboardListener::changedContents( const ClipboardEvent& event )
    throw(RuntimeException)
{
    m_nReceivedChangedContentsEvents++;
    m_xClipboard = Reference< XClipboard > (event.Source, UNO_QUERY);
    m_xTransferable = event.Contents;
}

//------------------------------------------------------------------------
//  disposing
//------------------------------------------------------------------------

void SAL_CALL ClipboardListener::disposing( const EventObject& event )
    throw(RuntimeException)
{
}

//------------------------------------------------------------------------
//  StringTransferable
//------------------------------------------------------------------------

class StringTransferable : public WeakImplHelper2< XClipboardOwner, XTransferable >
{
public:
    StringTransferable( );

    //--------------------------------------------------------------------
    // XTransferable
    //--------------------------------------------------------------------

    virtual Any SAL_CALL getTransferData( const DataFlavor& aFlavor ) throw(UnsupportedFlavorException, IOException, RuntimeException);
    virtual Sequence< DataFlavor > SAL_CALL getTransferDataFlavors(  ) throw(RuntimeException);
    virtual sal_Bool SAL_CALL isDataFlavorSupported( const DataFlavor& aFlavor ) throw(RuntimeException);

    //--------------------------------------------------------------------
    // XClipboardOwner
    //--------------------------------------------------------------------

    virtual void SAL_CALL lostOwnership( const Reference< XClipboard >& xClipboard, const Reference< XTransferable >& xTrans ) throw(RuntimeException);

    sal_Bool receivedLostOwnership() { return m_receivedLostOwnership; };
    void clearReceivedLostOwnership() { m_receivedLostOwnership = sal_False; };

private:
    Sequence< DataFlavor > m_seqDFlv;
    OUString               m_Data;
    sal_Bool               m_receivedLostOwnership;
};

//------------------------------------------------------------------------
//  ctor
//------------------------------------------------------------------------

StringTransferable::StringTransferable( ) :
    m_seqDFlv( 1 ),
    m_receivedLostOwnership( sal_False ),
    m_Data( OUString("clipboard test content") )
{
    DataFlavor df;

    /*
    df.MimeType = L"text/plain; charset=unicode";
    df.DataType = getCppuType( ( OUString* )0 );

    m_seqDFlv[0] = df;
    */

    //df.MimeType = L"text/plain; charset=windows1252";
    df.MimeType = OUString("text/html");
    df.DataType = getCppuType( ( Sequence< sal_Int8 >* )0 );

    m_seqDFlv[0] = df;
}

//------------------------------------------------------------------------
//  getTransferData
//------------------------------------------------------------------------

Any SAL_CALL StringTransferable::getTransferData( const DataFlavor& aFlavor )
    throw(UnsupportedFlavorException, IOException, RuntimeException)
{
    Any anyData;

    /*if ( aFlavor == m_seqDFlv[0] )
    {
        anyData = makeAny( m_Data );
    } */

    return anyData;
}

//------------------------------------------------------------------------
//  getTransferDataFlavors
//------------------------------------------------------------------------

Sequence< DataFlavor > SAL_CALL StringTransferable::getTransferDataFlavors(  )
    throw(RuntimeException)
{
    return m_seqDFlv;
}

//------------------------------------------------------------------------
//  isDataFlavorSupported
//------------------------------------------------------------------------

sal_Bool SAL_CALL StringTransferable::isDataFlavorSupported( const DataFlavor& aFlavor )
    throw(RuntimeException)
{
    sal_Int32 nLength = m_seqDFlv.getLength( );
    sal_Bool bRet     = sal_False;

//  for ( sal_Int32 i = 0; i < nLength; ++i )
//  {
//      if ( m_seqDFlv[i] == aFlavor )
//      {
//          bRet = sal_True;
//          break;
//      }
//  }

    return bRet;
}

//------------------------------------------------------------------------
//  lostOwnership
//------------------------------------------------------------------------

void SAL_CALL StringTransferable::lostOwnership( const Reference< XClipboard >& xClipboard, const Reference< XTransferable >& xTrans )
    throw(RuntimeException)
{
    m_receivedLostOwnership = sal_True;
}

//------------------------------------------------------------------------
//  main
//------------------------------------------------------------------------

int SAL_CALL main( int argc, const char* argv[] )
{
    OUString aRegistry;

    //------------------------------------------------------------------
    // check command line parameters
    //------------------------------------------------------------------

    if ( NULL == ( app = strrchr( argv[0], PATH_SEPERATOR ) ) )
        app = argv[0];
    else
        app++;

    for( int n = 1; n < argc; n++ )
    {
        if( strncmp( argv[n], "-r", 2 ) == 0 )
        {
            if( strlen( argv[n] ) > 2 )
                aRegistry = OUString::createFromAscii( argv[n] + 2 );
            else if ( n + 1 < argc )
                aRegistry = OUString::createFromAscii( argv[++n] );
        }
    }

    if( aRegistry.isEmpty( ) )
        fprintf( stderr, "Usage: %s -r full-path-to-applicat.rdb\n", app );

    //------------------------------------------------------------------
    // create service manager
    //------------------------------------------------------------------
    Reference< XMultiServiceFactory > xServiceManager;

    try
    {
        xServiceManager = createRegistryServiceFactory( aRegistry, sal_True );
        ENSURE( xServiceManager.is(), "*** ERROR *** service manager could not be created." );

        //--------------------------------------------------------------
        // create an instance of GenericClipboard service
        //--------------------------------------------------------------

        Sequence< Any > arguments(1);
        arguments[0] = makeAny( OUString("generic") );

        Reference< XClipboard > xClipboard( xServiceManager->createInstanceWithArguments(
            OUString("com.sun.star.datatransfer.clipboard.GenericClipboard"),
            arguments ), UNO_QUERY );

        ENSURE( xClipboard.is(), "*** ERROR *** generic clipboard service could not be created." );

        Reference< XClipboardNotifier > xClipboardNotifier( xClipboard, UNO_QUERY );
        Reference< XClipboardListener > xClipboardListener = new ClipboardListener();
        ClipboardListener * pListener = (ClipboardListener *) xClipboardListener.get();

        if( xClipboardNotifier.is() )
            xClipboardNotifier->addClipboardListener( xClipboardListener );

        //--------------------------------------------------------------
        // run various tests on clipboard implementation
        //--------------------------------------------------------------

        TRACE( "\n*** testing generic clipboard service ***\n" );

        Reference< XTransferable > xContents = new StringTransferable();
        Reference< XClipboardOwner > xOwner  = new ClipboardOwner();
        ClipboardOwner *pOwner = (ClipboardOwner *) xOwner.get();

        TEST( "initial contents (none): ", xClipboard->getContents().is() == sal_False );

        PERFORM( "update on contents with clipboard owner: ", xClipboard->setContents( xContents, xOwner ) );
        TEST( "current clipboard contents: ", xContents == xClipboard->getContents() );

        if( xClipboardNotifier.is() )
        {
            TEST( "if received changedContents notifications: ", pListener->receivedChangedContentsEvents() > 0 );
            TEST( "if received exactly 1 changedContents notification: ", pListener->receivedChangedContentsEvents() == 1 );
            TEST( "if received changedContents notification for correct clipboard: ", pListener->changedContentsEventClipboardValue() == xClipboard );
            TEST( "if received changedContents notification for correct clipboard: ", pListener->changedContentsEventTransferableValue() == xContents );
        }

        PERFORM( "update on contents without data (clear): ", xClipboard->setContents( Reference< XTransferable >(), Reference< XClipboardOwner >() ) );
        TEST( "if received lostOwnership message(s): ", pOwner->receivedLostOwnerships() > 0 );
        TEST( "if received exactly 1 lostOwnership message: ", pOwner->receivedLostOwnerships() == 1 );
        TEST( "if received lostOwnership message for the correct clipboard: ", pOwner->lostOwnershipClipboardValue() == xClipboard );
        TEST( "if received lostOwnership message for the correct transferable: ", pOwner->lostOwnershipTransferableValue() == xContents );
        TEST( "current clipboard contents (none): ", xClipboard->getContents().is() == sal_False );

        if( xClipboardNotifier.is() )
        {
            TEST( "if received changedContents notifications: ", pListener->receivedChangedContentsEvents() > 1 );
            TEST( "if received exactly 1 changedContents notification: ", pListener->receivedChangedContentsEvents() == 2 );
            TEST( "if received changedContents notification for correct clipboard: ", pListener->changedContentsEventClipboardValue() == xClipboard );
            TEST( "if received changedContents notification for correct transferable: ", ! pListener->changedContentsEventTransferableValue().is() );
        }

        PERFORM( "update on contents without clipboard owner: ", xClipboard->setContents( xContents, Reference< XClipboardOwner >() ) );
        TEST( "that no further lostOwnership messages were received: ", pOwner->receivedLostOwnerships() == 1 );
        TEST( "current clipboard contents: ", xContents == xClipboard->getContents() );

        if( xClipboardNotifier.is() )
        {
            TEST( "if received changedContents notifications: ", pListener->receivedChangedContentsEvents() > 2 );
            TEST( "if received exactly 1 changedContents notification: ", pListener->receivedChangedContentsEvents() == 3 );
            TEST( "if received changedContents notification for correct clipboard: ", pListener->changedContentsEventClipboardValue() == xClipboard );
            TEST( "if received changedContents notification for correct transferable: ", pListener->changedContentsEventTransferableValue() == xContents );
        }


        PERFORM( "update on contents without data (clear): ", xClipboard->setContents( Reference< XTransferable >(), Reference< XClipboardOwner >() ) );
        TEST( "that no further lostOwnership messages were received: ", pOwner->receivedLostOwnerships() == 1 );
        TEST( "current clipboard contents (none): ", xClipboard->getContents().is() == sal_False );

        if( xClipboardNotifier.is() )
        {
            TEST( "if received changedContents notifications: ", pListener->receivedChangedContentsEvents() > 3 );
            TEST( "if received exactly 1 changedContents notification: ", pListener->receivedChangedContentsEvents() == 4 );
            TEST( "if received changedContents notification for correct clipboard: ", pListener->changedContentsEventClipboardValue() == xClipboard );
            TEST( "if received changedContents notification for correct transferable: ", ! pListener->changedContentsEventTransferableValue().is() );
        }

        //--------------------------------------------------------------
        // create an instance of ClipboardManager service
        //--------------------------------------------------------------

        Reference< XClipboardManager > xClipboardManager( xServiceManager->createInstance(
            OUString("com.sun.star.datatransfer.clipboard.ClipboardManager") ), UNO_QUERY );

        ENSURE( xClipboardManager.is(), "*** ERROR *** clipboard manager service could not be created." );

        //--------------------------------------------------------------
        // run various tests on clipboard manager implementation
        //--------------------------------------------------------------

        TRACE( "\n*** testing clipboard manager service ***\n" );

        TEST( "initial number of clipboards (0): ", xClipboardManager->listClipboardNames().getLength() == 0 );
        PERFORM( "insertion of generic clipboard: ", xClipboardManager->addClipboard( xClipboard ) );
        TEST( "number of inserted clipboards (1): ", xClipboardManager->listClipboardNames().getLength() == 1 );
        TEST( "name of inserted clipboard (generic): ", xClipboardManager->listClipboardNames()[0] == OUString("generic") );
        TEST( "inserted clipboard instance: ", xClipboardManager->getClipboard( OUString("generic") ) == xClipboard );
        PERFORM( "removal of generic clipboard: ", xClipboardManager->removeClipboard( OUString("generic") ) );
        TEST( "number of inserted clipboards (0): ", xClipboardManager->listClipboardNames().getLength() == 0 );
        TRACE( "Testing inserted clipboard instance (none): " );
        try
        {
            xClipboardManager->getClipboard( OUString("generic") );
            TRACE( "FAILED\n" );
        }
        catch (const NoSuchElementException&)
        {
            TRACE( "passed\n" );
        }
    }

    catch (const Exception&)
    {
        ENSURE( sal_False, "*** ERROR *** exception caught." );
    }

    //--------------------------------------------------------------------
    // shutdown the service manager
    //--------------------------------------------------------------------

    // query XComponent interface
    Reference< XComponent > xComponent( xServiceManager, UNO_QUERY );

    ENSURE( xComponent.is(), "*** ERROR *** service manager does not support XComponent." );

    // Dispose and clear factory
    xComponent->dispose();
    xServiceManager.clear();

    fprintf( stderr, "Done.\n" );
    return 0;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
