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

#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/lstbox.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase3.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>
#include <com/sun/star/datatransfer/dnd/XDragSourceListener.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTargetListener.hpp>
#include <com/sun/star/datatransfer/dnd/XDragGestureRecognizer.hpp>
#include <com/sun/star/datatransfer/dnd/XDragGestureListener.hpp>
#include <osl/process.h>

#include <stdio.h>

using namespace ::rtl;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::datatransfer::clipboard;
using namespace ::com::sun::star::datatransfer::dnd;

// -----------------------------------------------------------------------

class MyApp : public Application
{
public:
    void        Main();
};

MyApp aMyApp;

// -----------------------------------------------------------------------

class MyWin : public WorkWindow
{
public:
                MyWin( Window* pParent, WinBits nWinStyle );

    void        MouseMove( const MouseEvent& rMEvt );
    void        MouseButtonDown( const MouseEvent& rMEvt );
    void        MouseButtonUp( const MouseEvent& rMEvt );
    void        KeyInput( const KeyEvent& rKEvt );
    void        KeyUp( const KeyEvent& rKEvt );
    void        Paint( const Rectangle& rRect );
    void        Resize();
};

// -----------------------------------------------------------------------

class MyDragAndDropListener: public ::cppu::WeakImplHelper3 < XDropTargetListener, XDragGestureListener, XDragSourceListener >
{
    Window * m_pWindow;

public:

    MyDragAndDropListener( Window * pWindow ) : m_pWindow( pWindow ) {};

    virtual void SAL_CALL dragGestureRecognized( const DragGestureEvent& dge ) throw(RuntimeException);
    virtual void SAL_CALL drop( const DropTargetDropEvent& dtde ) throw(RuntimeException);
    virtual void SAL_CALL dragEnter( const DropTargetDragEnterEvent& dtde ) throw(RuntimeException);
    virtual void SAL_CALL dragExit( const DropTargetEvent& dte ) throw(RuntimeException);
    virtual void SAL_CALL dragOver( const DropTargetDragEvent& dtde ) throw(RuntimeException);
    virtual void SAL_CALL dropActionChanged( const DropTargetDragEvent& dtde ) throw(RuntimeException);
    virtual void SAL_CALL dragDropEnd( const DragSourceDropEvent& dsde ) throw(RuntimeException);
    virtual void SAL_CALL dragEnter( const DragSourceDragEvent& dsdee ) throw(RuntimeException);
    virtual void SAL_CALL dragExit( const DragSourceEvent& dse ) throw(RuntimeException);
    virtual void SAL_CALL dragOver( const DragSourceDragEvent& dsde ) throw(RuntimeException);
    virtual void SAL_CALL dropActionChanged( const DragSourceDragEvent& dsde ) throw(RuntimeException);
    virtual void SAL_CALL disposing( const EventObject& eo ) throw(RuntimeException);
};

// -----------------------------------------------------------------------

class MyInfoBox : public InfoBox
{

public:

    MyInfoBox( Window* pParent );
};

// -----------------------------------------------------------------------

class MyListBox : public ListBox
{

public:

    MyListBox( Window* pParent );
};

// -----------------------------------------------------------------------

class StringTransferable : public ::cppu::WeakImplHelper1< XTransferable >
{
    const OUString         m_aData;
    Sequence< DataFlavor > m_aFlavorList;

public:
    StringTransferable( const OUString& rString ) : m_aData( rString ), m_aFlavorList( 1 )
    {
        DataFlavor df;

        df.MimeType = OUString("text/plain;charset=utf-16");
        df.DataType = getCppuType( static_cast < OUString * > ( 0 ) );

         m_aFlavorList[0] = df;
    };

    virtual Any SAL_CALL getTransferData( const DataFlavor& aFlavor ) throw(UnsupportedFlavorException, IOException, RuntimeException);
    virtual Sequence< DataFlavor > SAL_CALL getTransferDataFlavors(  ) throw(RuntimeException);
    virtual sal_Bool SAL_CALL isDataFlavorSupported( const DataFlavor& aFlavor ) throw(RuntimeException);
};


// -----------------------------------------------------------------------

void MyApp::Main()
{
    OUString aRegistry;

    for( sal_Int32 n = 0, nmax = osl_getCommandArgCount(); n < nmax; n++ )
    {
        OUString aArg;

        osl_getCommandArg( n, &aArg.pData );

        if( aArg.startsWith( "-r" ) )
        {
            if ( n + 1 < nmax )
                osl_getCommandArg( ++n, &aRegistry.pData );
        }
    }

    Reference< XMultiServiceFactory > xServiceManager;

    if( aRegistry.getLength() )
    {
        xServiceManager = ::cppu::createRegistryServiceFactory( aRegistry, sal_True );

        if( xServiceManager.is() )
        {
            ::comphelper::setProcessServiceFactory( xServiceManager );
        }

        if( ! xServiceManager.is() )
            printf( "No servicemanager available.\n" );
        else
            printf( "Ok\n" );

    }
    else
        fprintf( stderr, "Usage: %s -r full-path-to-applicat.rdb\n", "dnddemo" );


    MyWin aMainWin( NULL, WB_APP | WB_STDWORK );
    aMainWin.SetText( rtl::OUString( "Drag And Drop - Workbench"  ) );
    aMainWin.Show();

    // test the clipboard code
    Reference< XClipboard > xClipboard = aMainWin.GetClipboard();
    if( xClipboard.is() )
    {
        printf( "System clipboard available.\n" );
        xClipboard->getContents();
    }
    else
        fprintf( stderr, "System clipboard not available.\n" );

    MyInfoBox aInfoBox( &aMainWin );
    aInfoBox.Execute();

    MyListBox aListBox( &aMainWin );
    aListBox.SetPosSizePixel( 10, 10, 100, 100 );
    aListBox.InsertEntry( OUString("TestItem"));
    aListBox.Show();

    Execute();

    Reference< XComponent > xComponent( xServiceManager, UNO_QUERY );
    if( xComponent.is() )
        xComponent->dispose();

}

// -----------------------------------------------------------------------

MyWin::MyWin( Window* pParent, WinBits nWinStyle ) :
    WorkWindow( pParent, nWinStyle )
{
    Reference< XDropTargetListener > xListener = new MyDragAndDropListener( this );

    Reference< XDropTarget > xDropTarget = GetDropTarget();
    if( xDropTarget.is() )
    {
        xDropTarget->addDropTargetListener( xListener );
        xDropTarget->setActive( sal_True );
    }

    Reference< XDragGestureRecognizer > xRecognizer = GetDragGestureRecognizer();
    if( xRecognizer.is() )
        xRecognizer->addDragGestureListener( Reference< XDragGestureListener > ( xListener, UNO_QUERY ) );
}

// -----------------------------------------------------------------------

void MyWin::MouseMove( const MouseEvent& rMEvt )
{
    WorkWindow::MouseMove( rMEvt );
}

// -----------------------------------------------------------------------

void MyWin::MouseButtonDown( const MouseEvent& rMEvt )
{
    WorkWindow::MouseButtonDown( rMEvt );
}

// -----------------------------------------------------------------------

void MyWin::MouseButtonUp( const MouseEvent& rMEvt )
{
    WorkWindow::MouseButtonUp( rMEvt );
}

// -----------------------------------------------------------------------

void MyWin::KeyInput( const KeyEvent& rKEvt )
{
    WorkWindow::KeyInput( rKEvt );
}

// -----------------------------------------------------------------------

void MyWin::KeyUp( const KeyEvent& rKEvt )
{
    WorkWindow::KeyUp( rKEvt );
}

// -----------------------------------------------------------------------

void MyWin::Paint( const Rectangle& rRect )
{
    WorkWindow::Paint( rRect );
}

// -----------------------------------------------------------------------

void MyWin::Resize()
{
    WorkWindow::Resize();
}

// -----------------------------------------------------------------------

void SAL_CALL MyDragAndDropListener::dragGestureRecognized( const DragGestureEvent& dge ) throw(RuntimeException)
{
    printf( "XDragGestureListener::dragGestureRecognized called ( Window: %p, %" SAL_PRIdINT32 ", %" SAL_PRIdINT32 " ).\n", m_pWindow, dge.DragOriginX, dge.DragOriginY );

    Reference< XDragSource > xDragSource( dge.DragSource, UNO_QUERY );
    xDragSource->startDrag( dge, -1, 0, 0, new StringTransferable( OUString("TestString") ), this );
    printf( "XDragSource::startDrag returned.\n" );
}

// -----------------------------------------------------------------------

void SAL_CALL MyDragAndDropListener::drop( const DropTargetDropEvent& dtde ) throw(RuntimeException)
{
    printf( "XDropTargetListener::drop called ( Window: %p, %" SAL_PRIdINT32 ", %" SAL_PRIdINT32 " ).\n", m_pWindow, dtde.LocationX, dtde.LocationY );

    dtde.Context->dropComplete( sal_True );
}

// -----------------------------------------------------------------------

void SAL_CALL MyDragAndDropListener::dragEnter( const DropTargetDragEnterEvent& dtdee ) throw(RuntimeException)
{
    printf( "XDropTargetListener::dragEnter called ( Window: %p, %" SAL_PRIdINT32 ", %" SAL_PRIdINT32 " ).\n", m_pWindow, dtdee.LocationX, dtdee.LocationY );
    dtdee.Context->acceptDrag( dtdee.DropAction );
}

// -----------------------------------------------------------------------

void SAL_CALL MyDragAndDropListener::dragExit( const DropTargetEvent& ) throw(RuntimeException)
{
    printf( "XDropTargetListener::dragExit called ( Window: %p ).\n", m_pWindow );
}

// -----------------------------------------------------------------------

void SAL_CALL MyDragAndDropListener::dragOver( const DropTargetDragEvent& dtde ) throw(RuntimeException)
{
    printf( "XDropTargetListener::dragOver called ( Window: %p, %" SAL_PRIdINT32 ", %" SAL_PRIdINT32 " ).\n", m_pWindow, dtde.LocationX, dtde.LocationY );
    dtde.Context->acceptDrag( dtde.DropAction );
}

// -----------------------------------------------------------------------

void SAL_CALL MyDragAndDropListener::dropActionChanged( const DropTargetDragEvent& dtde ) throw(RuntimeException)
{
    printf( "XDropTargetListener::dropActionChanged called ( Window: %p, %" SAL_PRIdINT32 ", %" SAL_PRIdINT32 " ).\n", m_pWindow, dtde.LocationX, dtde.LocationY );
    dtde.Context->acceptDrag( dtde.DropAction );
}

// -----------------------------------------------------------------------

void SAL_CALL MyDragAndDropListener::dragDropEnd( const DragSourceDropEvent& dsde ) throw(RuntimeException)
{
    printf( "XDragSourceListener::dropDropEnd called ( Window: %p, %s ).\n", m_pWindow, dsde.DropSuccess ? "sucess" : "failed" );
}

// -----------------------------------------------------------------------

void SAL_CALL MyDragAndDropListener::dragEnter( const DragSourceDragEvent& ) throw(RuntimeException)
{
    printf( "XDragSourceListener::dragEnter called ( Window: %p ).\n", m_pWindow );
}

// -----------------------------------------------------------------------

void SAL_CALL MyDragAndDropListener::dragExit( const DragSourceEvent& ) throw(RuntimeException)
{
    printf( "XDragSourceListener::dragExit called ( Window: %p ).\n", m_pWindow );
}

// -----------------------------------------------------------------------

void SAL_CALL MyDragAndDropListener::dragOver( const DragSourceDragEvent& ) throw(RuntimeException)
{
    printf( "XDragSourceListener::dragOver called ( Window: %p ).\n", m_pWindow );
}

// -----------------------------------------------------------------------

void SAL_CALL MyDragAndDropListener::dropActionChanged( const DragSourceDragEvent& ) throw(RuntimeException)
{
    printf( "XDragSourceListener::dropActionChanged called ( Window: %p ).\n", m_pWindow );
}

// -----------------------------------------------------------------------

void SAL_CALL MyDragAndDropListener::disposing( const EventObject& ) throw(RuntimeException)
{
    printf( "XEventListener::disposing called ( Window: %p ).\n", m_pWindow );
}

// -----------------------------------------------------------------------

MyInfoBox::MyInfoBox( Window* pParent ) : InfoBox( pParent,
    OUString("dragging over this box should result in another window id in the drag log.") )
{
    Reference< XDropTargetListener > xListener = new MyDragAndDropListener( this );

    Reference< XDropTarget > xDropTarget = GetDropTarget();
    if( xDropTarget.is() )
    {
        xDropTarget->addDropTargetListener( xListener );
        xDropTarget->setActive( sal_True );
    }

    Reference< XDragGestureRecognizer > xRecognizer = GetDragGestureRecognizer();
    if( xRecognizer.is() )
        xRecognizer->addDragGestureListener( Reference< XDragGestureListener > ( xListener, UNO_QUERY ) );
};

// -----------------------------------------------------------------------

MyListBox::MyListBox( Window* pParent ) : ListBox( pParent )
{
    Reference< XDropTargetListener > xListener = new MyDragAndDropListener( this );

    Reference< XDropTarget > xDropTarget = GetDropTarget();
    if( xDropTarget.is() )
    {
        xDropTarget->addDropTargetListener( xListener );
        xDropTarget->setActive( sal_True );
    }

    Reference< XDragGestureRecognizer > xRecognizer = GetDragGestureRecognizer();
    if( xRecognizer.is() )
        xRecognizer->addDragGestureListener( Reference< XDragGestureListener > ( xListener, UNO_QUERY ) );
};

// -----------------------------------------------------------------------

Any SAL_CALL StringTransferable::getTransferData( const DataFlavor& )
    throw(UnsupportedFlavorException, IOException, RuntimeException)
{
    return makeAny( m_aData );
}

// -----------------------------------------------------------------------

Sequence< DataFlavor > SAL_CALL StringTransferable::getTransferDataFlavors(  )
    throw(RuntimeException)
{
    return m_aFlavorList;
}

// -----------------------------------------------------------------------

sal_Bool SAL_CALL StringTransferable::isDataFlavorSupported( const DataFlavor& )
    throw(RuntimeException)
{
    return sal_True;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
