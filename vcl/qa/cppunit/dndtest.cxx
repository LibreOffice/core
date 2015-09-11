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

#include <unotest/filters-test.hxx>
#include <test/bootstrapfixture.hxx>

#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/lstbox.hxx>

#include <cppuhelper/implbase.hxx>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>
#include <com/sun/star/datatransfer/dnd/XDragSourceListener.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTargetListener.hpp>
#include <com/sun/star/datatransfer/dnd/XDragGestureRecognizer.hpp>
#include <com/sun/star/datatransfer/dnd/XDragGestureListener.hpp>

using namespace ::com::sun::star::io;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::datatransfer;
using namespace ::com::sun::star::datatransfer::clipboard;
using namespace ::com::sun::star::datatransfer::dnd;

class MyWin : public WorkWindow
{
public:
                MyWin( vcl::Window* pParent, WinBits nWinStyle );

    void        MouseMove( const MouseEvent& rMEvt );
    void        MouseButtonDown( const MouseEvent& rMEvt );
    void        MouseButtonUp( const MouseEvent& rMEvt );
    void        KeyInput( const KeyEvent& rKEvt );
    void        KeyUp( const KeyEvent& rKEvt );
    void        Paint( const Rectangle& rRect );
    void        Resize();
};

class MyDragAndDropListener: public ::cppu::WeakImplHelper < XDropTargetListener, XDragGestureListener, XDragSourceListener >
{
    vcl::Window * m_pWindow;

public:

    explicit MyDragAndDropListener( vcl::Window * pWindow ) : m_pWindow( pWindow ) {};

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

class MyInfoBox : public InfoBox
{

public:

    explicit MyInfoBox( vcl::Window* pParent );
};

class MyListBox : public ListBox
{

public:

    explicit MyListBox( vcl::Window* pParent );
};

class StringTransferable : public ::cppu::WeakImplHelper< XTransferable >
{
    const OUString         m_aData;
    Sequence< DataFlavor > m_aFlavorList;

public:
    explicit StringTransferable( const OUString& rString ) : m_aData( rString ), m_aFlavorList( 1 )
    {
        DataFlavor df;

        df.MimeType = "text/plain;charset=utf-16";
        df.DataType = cppu::UnoType<OUString>::get();

         m_aFlavorList[0] = df;
    };

    virtual Any SAL_CALL getTransferData( const DataFlavor& aFlavor ) throw(UnsupportedFlavorException, IOException, RuntimeException);
    virtual Sequence< DataFlavor > SAL_CALL getTransferDataFlavors(  ) throw(RuntimeException);
    virtual bool SAL_CALL isDataFlavorSupported( const DataFlavor& aFlavor ) throw(RuntimeException);
};

class VclDnDTest : public test::BootstrapFixture
{
public:
    VclDnDTest() : BootstrapFixture(true, false) {}

    /// Play with drag and drop
    void testDnD();

    CPPUNIT_TEST_SUITE(VclDnDTest);
    CPPUNIT_TEST(testDnD);
    CPPUNIT_TEST_SUITE_END();
};

void VclDnDTest::testDnD()
{
    MyWin aMainWin( NULL, WB_APP | WB_STDWORK );
    aMainWin.SetText( OUString( "Drag And Drop - Workbench"  ) );
    aMainWin.Show();

    // test the clipboard code
    Reference< XClipboard > xClipboard = aMainWin.GetClipboard();
    CPPUNIT_ASSERT_MESSAGE("System clipboard not available",
                           xClipboard.is());

    MyInfoBox aInfoBox( &aMainWin );
    aInfoBox.Execute();

    MyListBox aListBox( &aMainWin );
    aListBox.setPosSizePixel( 10, 10, 100, 100 );
    aListBox.InsertEntry( OUString("TestItem"));
    aListBox.Show();
}

MyWin::MyWin( vcl::Window* pParent, WinBits nWinStyle ) :
    WorkWindow( pParent, nWinStyle )
{
    Reference< XDropTargetListener > xListener = new MyDragAndDropListener( this );

    Reference< XDropTarget > xDropTarget = GetDropTarget();
    if( xDropTarget.is() )
    {
        xDropTarget->addDropTargetListener( xListener );
        xDropTarget->setActive( true );
    }

    Reference< XDragGestureRecognizer > xRecognizer = GetDragGestureRecognizer();
    if( xRecognizer.is() )
        xRecognizer->addDragGestureListener( Reference< XDragGestureListener > ( xListener, UNO_QUERY ) );
}

void MyWin::MouseMove( const MouseEvent& rMEvt )
{
    WorkWindow::MouseMove( rMEvt );
}

void MyWin::MouseButtonDown( const MouseEvent& rMEvt )
{
    WorkWindow::MouseButtonDown( rMEvt );
}

void MyWin::MouseButtonUp( const MouseEvent& rMEvt )
{
    WorkWindow::MouseButtonUp( rMEvt );
}

void MyWin::KeyInput( const KeyEvent& rKEvt )
{
    WorkWindow::KeyInput( rKEvt );
}

void MyWin::KeyUp( const KeyEvent& rKEvt )
{
    WorkWindow::KeyUp( rKEvt );
}

void MyWin::Paint( const Rectangle& rRect )
{
    WorkWindow::Paint( rRect );
}

void MyWin::Resize()
{
    WorkWindow::Resize();
}

void SAL_CALL MyDragAndDropListener::dragGestureRecognized( const DragGestureEvent& dge ) throw(RuntimeException)
{
    Reference< XDragSource > xDragSource( dge.DragSource, UNO_QUERY );
    xDragSource->startDrag( dge, -1, 0, 0, new StringTransferable( OUString("TestString") ), this );
}

void SAL_CALL MyDragAndDropListener::drop( const DropTargetDropEvent& dtde ) throw(RuntimeException)
{
    dtde.Context->dropComplete( true );
}

void SAL_CALL MyDragAndDropListener::dragEnter( const DropTargetDragEnterEvent& dtdee ) throw(RuntimeException)
{
    dtdee.Context->acceptDrag( dtdee.DropAction );
}

void SAL_CALL MyDragAndDropListener::dragExit( const DropTargetEvent& ) throw(RuntimeException)
{
}

void SAL_CALL MyDragAndDropListener::dragOver( const DropTargetDragEvent& dtde ) throw(RuntimeException)
{
    dtde.Context->acceptDrag( dtde.DropAction );
}

void SAL_CALL MyDragAndDropListener::dropActionChanged( const DropTargetDragEvent& dtde ) throw(RuntimeException)
{
    dtde.Context->acceptDrag( dtde.DropAction );
}

void SAL_CALL MyDragAndDropListener::dragDropEnd( const DragSourceDropEvent& ) throw(RuntimeException)
{
}

void SAL_CALL MyDragAndDropListener::dragEnter( const DragSourceDragEvent& ) throw(RuntimeException)
{
}

void SAL_CALL MyDragAndDropListener::dragExit( const DragSourceEvent& ) throw(RuntimeException)
{
}

void SAL_CALL MyDragAndDropListener::dragOver( const DragSourceDragEvent& ) throw(RuntimeException)
{
}

void SAL_CALL MyDragAndDropListener::dropActionChanged( const DragSourceDragEvent& ) throw(RuntimeException)
{
}

void SAL_CALL MyDragAndDropListener::disposing( const EventObject& ) throw(RuntimeException)
{
}

MyInfoBox::MyInfoBox( vcl::Window* pParent ) : InfoBox( pParent,
    OUString("dragging over this box should result in another window id in the drag log.") )
{
    Reference< XDropTargetListener > xListener = new MyDragAndDropListener( this );

    Reference< XDropTarget > xDropTarget = GetDropTarget();
    if( xDropTarget.is() )
    {
        xDropTarget->addDropTargetListener( xListener );
        xDropTarget->setActive( true );
    }

    Reference< XDragGestureRecognizer > xRecognizer = GetDragGestureRecognizer();
    if( xRecognizer.is() )
        xRecognizer->addDragGestureListener( Reference< XDragGestureListener > ( xListener, UNO_QUERY ) );
};

MyListBox::MyListBox( vcl::Window* pParent ) : ListBox( pParent )
{
    Reference< XDropTargetListener > xListener = new MyDragAndDropListener( this );

    Reference< XDropTarget > xDropTarget = GetDropTarget();
    if( xDropTarget.is() )
    {
        xDropTarget->addDropTargetListener( xListener );
        xDropTarget->setActive( true );
    }

    Reference< XDragGestureRecognizer > xRecognizer = GetDragGestureRecognizer();
    if( xRecognizer.is() )
        xRecognizer->addDragGestureListener( Reference< XDragGestureListener > ( xListener, UNO_QUERY ) );
};

Any SAL_CALL StringTransferable::getTransferData( const DataFlavor& )
    throw(UnsupportedFlavorException, IOException, RuntimeException)
{
    return makeAny( m_aData );
}

Sequence< DataFlavor > SAL_CALL StringTransferable::getTransferDataFlavors(  )
    throw(RuntimeException)
{
    return m_aFlavorList;
}

bool SAL_CALL StringTransferable::isDataFlavorSupported( const DataFlavor& )
    throw(RuntimeException)
{
    return true;
}

CPPUNIT_TEST_SUITE_REGISTRATION(VclDnDTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
