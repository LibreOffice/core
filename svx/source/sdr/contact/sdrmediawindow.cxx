/*************************************************************************
 *
 *  $RCSfile: sdrmediawindow.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-08-12 09:04:14 $
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

#include "sdrmediawindow.hxx"

#undef protected
#define protected public
#include <svtools/transfer.hxx>
#undef protected
#define protected protected

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
            nRet = pDropTargetHelper->AcceptDrop( rEvt );
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
            nRet = pDropTargetHelper->ExecuteDrop( rEvt );
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
            pDragSourceHelper->StartDrag( nAction, rPosPixel );
    }
}

} }
