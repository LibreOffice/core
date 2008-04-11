/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dndtest.cxx,v $
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
#include "precompiled_vcl.hxx"
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
#include <vos/process.hxx>

#include <stdio.h>

using namespace ::rtl;
using namespace ::vos;
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
    StringTransferable( const OUString& rString ) : m_aFlavorList( 1 ), m_aData( rString )
    {
        DataFlavor df;

        df.MimeType = OUString::createFromAscii( "text/plain;charset=utf-16" );
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
    OStartupInfo aInfo;

    for( sal_Int32 n = 0, nmax = aInfo.getCommandArgCount(); n < nmax; n++ )
    {
        OUString aArg;

        aInfo.getCommandArg( n, aArg );

        if( aArg.compareTo( OUString::createFromAscii( "-r" ), 2 ) == 0 )
        {
            if ( n + 1 < nmax )
                aInfo.getCommandArg( ++n, aRegistry );
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
    aMainWin.SetText( XubString( RTL_CONSTASCII_USTRINGPARAM( "Drag And Drop - Workbench" ) ) );
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
    aListBox.InsertEntry( OUString::createFromAscii( "TestItem" ));
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
    printf( "XDragGestureListener::dragGestureRecognized called ( Window: %X, %d, %d ).\n", m_pWindow, dge.DragOriginX, dge.DragOriginY );

    Reference< XDragSource > xDragSource( dge.DragSource, UNO_QUERY );
    xDragSource->startDrag( dge, -1, 0, 0, new StringTransferable( OUString::createFromAscii( "TestString" ) ), this );
    printf( "XDragSource::startDrag returned.\n" );
}

// -----------------------------------------------------------------------

void SAL_CALL MyDragAndDropListener::drop( const DropTargetDropEvent& dtde ) throw(RuntimeException)
{
    printf( "XDropTargetListener::drop called ( Window: %X, %d, %d ).\n", m_pWindow, dtde.LocationX, dtde.LocationY );

    dtde.Context->dropComplete( sal_True );
}

// -----------------------------------------------------------------------

void SAL_CALL MyDragAndDropListener::dragEnter( const DropTargetDragEnterEvent& dtdee ) throw(RuntimeException)
{
    printf( "XDropTargetListener::dragEnter called ( Window: %X, %d, %d ).\n", m_pWindow, dtdee.LocationX, dtdee.LocationY );
    dtdee.Context->acceptDrag( dtdee.DropAction );
}

// -----------------------------------------------------------------------

void SAL_CALL MyDragAndDropListener::dragExit( const DropTargetEvent& dte ) throw(RuntimeException)
{
    printf( "XDropTargetListener::dragExit called ( Window: %X ).\n", m_pWindow );
}

// -----------------------------------------------------------------------

void SAL_CALL MyDragAndDropListener::dragOver( const DropTargetDragEvent& dtde ) throw(RuntimeException)
{
    printf( "XDropTargetListener::dragOver called ( Window: %X, %d, %d ).\n", m_pWindow, dtde.LocationX, dtde.LocationY );
    dtde.Context->acceptDrag( dtde.DropAction );
}

// -----------------------------------------------------------------------

void SAL_CALL MyDragAndDropListener::dropActionChanged( const DropTargetDragEvent& dtde ) throw(RuntimeException)
{
    printf( "XDropTargetListener::dropActionChanged called ( Window: %X, %d, %d ).\n", m_pWindow, dtde.LocationX, dtde.LocationY );
    dtde.Context->acceptDrag( dtde.DropAction );
}

// -----------------------------------------------------------------------

void SAL_CALL MyDragAndDropListener::dragDropEnd( const DragSourceDropEvent& dsde ) throw(RuntimeException)
{
    printf( "XDragSourceListener::dropDropEnd called ( Window: %X, %s ).\n", m_pWindow, dsde.DropSuccess ? "sucess" : "failed" );
}

// -----------------------------------------------------------------------

void SAL_CALL MyDragAndDropListener::dragEnter( const DragSourceDragEvent& dsde ) throw(RuntimeException)
{
    printf( "XDragSourceListener::dragEnter called ( Window: %X ).\n", m_pWindow );
}

// -----------------------------------------------------------------------

void SAL_CALL MyDragAndDropListener::dragExit( const DragSourceEvent& dse ) throw(RuntimeException)
{
    printf( "XDragSourceListener::dragExit called ( Window: %X ).\n", m_pWindow );
}

// -----------------------------------------------------------------------

void SAL_CALL MyDragAndDropListener::dragOver( const DragSourceDragEvent& dsde ) throw(RuntimeException)
{
    printf( "XDragSourceListener::dragOver called ( Window: %X ).\n", m_pWindow );
}

// -----------------------------------------------------------------------

void SAL_CALL MyDragAndDropListener::dropActionChanged( const DragSourceDragEvent& dsde ) throw(RuntimeException)
{
    printf( "XDragSourceListener::dropActionChanged called ( Window: %X ).\n", m_pWindow );
}

// -----------------------------------------------------------------------

void SAL_CALL MyDragAndDropListener::disposing( const EventObject& eo ) throw(RuntimeException)
{
    printf( "XEventListener::disposing called ( Window: %X ).\n", m_pWindow );
}

// -----------------------------------------------------------------------

MyInfoBox::MyInfoBox( Window* pParent ) : InfoBox( pParent,
    OUString::createFromAscii( "dragging over this box should result in another window id in the drag log." ) )
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

Any SAL_CALL StringTransferable::getTransferData( const DataFlavor& aFlavor )
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

sal_Bool SAL_CALL StringTransferable::isDataFlavorSupported( const DataFlavor& aFlavor )
    throw(RuntimeException)
{
    return sal_True;
}


