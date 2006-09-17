/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdrmediawindow.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 05:34:48 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include "sdrmediawindow.hxx"

#ifndef _TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////
// Added wrappers to replace the '#define protected public' hack embracing
// the svtools/transfer include. Need to ask KA if it makes more sense to
// change the used protected methods to public in the used svtoosl classes.

class WrappedDropTargetHelper : DropTargetHelper
{
public:
    sal_Int8 WrappedAcceptDrop( const AcceptDropEvent& rEvt ) { return AcceptDrop(rEvt); }
    sal_Int8 WrappedExecuteDrop( const ExecuteDropEvent& rEvt ) { return ExecuteDrop(rEvt); }
};

class WrappedDragSourceHelper : DragSourceHelper
{
public:
    void WrappedStartDrag( sal_Int8 nAction, const Point& rPosPixel ) { StartDrag(nAction, rPosPixel); }
};

//////////////////////////////////////////////////////////////////////////////

#include <svx/sdr/contact/viewobjectcontactofsdrmediaobj.hxx>
#include <vcl/window.hxx>

namespace sdr { namespace contact {

// ------------------
// - SdrMediaWindow -
// ------------------

SdrMediaWindow::SdrMediaWindow( Window* pParent, ViewObjectContactOfSdrMediaObj& rViewObjContact ) :
    ::avmedia::MediaWindow( pParent, false ),
    mrViewObjectContactOfSdrMediaObj( rViewObjContact )
{
}

// ------------------------------------------------------------------------------

SdrMediaWindow::~SdrMediaWindow()
{
}

// ------------------------------------------------------------------------------

void SdrMediaWindow::MouseMove( const MouseEvent& rMEvt )
{
    Window* pWindow = mrViewObjectContactOfSdrMediaObj.getWindow();

    if( pWindow && getWindow() )
    {
        const MouseEvent aTransformedEvent( pWindow->ScreenToOutputPixel( getWindow()->OutputToScreenPixel( rMEvt.GetPosPixel() ) ),
                                              rMEvt.GetClicks(), rMEvt.GetMode(), rMEvt.GetButtons(), rMEvt.GetModifier() );

        pWindow->MouseMove( aTransformedEvent );
        setPointer( pWindow->GetPointer() );
    }
}

// ------------------------------------------------------------------------------

void SdrMediaWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    Window* pWindow = mrViewObjectContactOfSdrMediaObj.getWindow();

    if( pWindow && getWindow() )
    {
        const MouseEvent aTransformedEvent( pWindow->ScreenToOutputPixel( getWindow()->OutputToScreenPixel( rMEvt.GetPosPixel() ) ),
                                              rMEvt.GetClicks(), rMEvt.GetMode(), rMEvt.GetButtons(), rMEvt.GetModifier() );

        pWindow->MouseButtonDown( aTransformedEvent );
    }
}

// ------------------------------------------------------------------------------

void SdrMediaWindow::MouseButtonUp( const MouseEvent& rMEvt )
{
    Window* pWindow = mrViewObjectContactOfSdrMediaObj.getWindow();

    if( pWindow && getWindow() )
    {
        const MouseEvent aTransformedEvent( pWindow->ScreenToOutputPixel( getWindow()->OutputToScreenPixel( rMEvt.GetPosPixel() ) ),
                                              rMEvt.GetClicks(), rMEvt.GetMode(), rMEvt.GetButtons(), rMEvt.GetModifier() );

        pWindow->MouseButtonUp( aTransformedEvent );
    }
}

// ------------------------------------------------------------------------------

void SdrMediaWindow::KeyInput( const KeyEvent& rKEvt )
{
    Window* pWindow = mrViewObjectContactOfSdrMediaObj.getWindow();

    if( pWindow )
        pWindow->KeyInput( rKEvt );
}

// ------------------------------------------------------------------------------

void SdrMediaWindow::KeyUp( const KeyEvent& rKEvt )
{
    Window* pWindow = mrViewObjectContactOfSdrMediaObj.getWindow();

    if( pWindow )
        pWindow->KeyUp( rKEvt );
}

// ------------------------------------------------------------------------------

void SdrMediaWindow::Command( const CommandEvent& rCEvt )
{
    Window* pWindow = mrViewObjectContactOfSdrMediaObj.getWindow();

    if( pWindow && getWindow() )
    {
        const CommandEvent aTransformedEvent( pWindow->ScreenToOutputPixel( getWindow()->OutputToScreenPixel( rCEvt.GetMousePosPixel() ) ),
                                                rCEvt.GetCommand(), rCEvt.IsMouseEvent(), rCEvt.GetData() );

        pWindow->Command( aTransformedEvent );
    }
}

// ------------------------------------------------------------------------------

sal_Int8 SdrMediaWindow::AcceptDrop( const AcceptDropEvent& rEvt )
{
    Window*     pWindow = mrViewObjectContactOfSdrMediaObj.getWindow();
    sal_Int8    nRet = DND_ACTION_NONE;

    if( pWindow )
    {
        DropTargetHelper* pDropTargetHelper = dynamic_cast< DropTargetHelper* >( pWindow );

        if( pDropTargetHelper )
        {
            nRet = ((WrappedDropTargetHelper*)pDropTargetHelper)->WrappedAcceptDrop( rEvt );
        }
    }

    return( nRet );
}

// ------------------------------------------------------------------------------

sal_Int8 SdrMediaWindow::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    Window*     pWindow = mrViewObjectContactOfSdrMediaObj.getWindow();
    sal_Int8    nRet = DND_ACTION_NONE;

    if( pWindow )
    {
        DropTargetHelper* pDropTargetHelper = dynamic_cast< DropTargetHelper* >( pWindow );

        if( pDropTargetHelper )
        {
            nRet = ((WrappedDropTargetHelper*)pDropTargetHelper)->WrappedExecuteDrop( rEvt );
        }
    }

    return( nRet );
}

// ------------------------------------------------------------------------------

void SdrMediaWindow::StartDrag( sal_Int8 nAction, const Point& rPosPixel )
{
    Window* pWindow = mrViewObjectContactOfSdrMediaObj.getWindow();

    if( pWindow )
    {
        DragSourceHelper* pDragSourceHelper = dynamic_cast< DragSourceHelper* >( pWindow );

        if( pDragSourceHelper )
        {
            ((WrappedDragSourceHelper*)pDragSourceHelper)->WrappedStartDrag( nAction, rPosPixel );
        }
    }
}

} }
