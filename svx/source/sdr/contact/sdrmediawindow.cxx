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
#include "precompiled_svx.hxx"

#include "sdrmediawindow.hxx"
#include <svtools/transfer.hxx>

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
            nRet = pDropTargetHelper->AcceptDrop( rEvt );
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
            nRet = pDropTargetHelper->ExecuteDrop( rEvt );
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
            pDragSourceHelper->StartDrag( nAction, rPosPixel );
        }
    }
}

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
