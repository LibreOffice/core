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


#include "sdrmediawindow.hxx"
#include <vcl/transfer.hxx>

#include <sdr/contact/viewobjectcontactofsdrmediaobj.hxx>
#include <vcl/window.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>

namespace sdr { namespace contact {


SdrMediaWindow::SdrMediaWindow( vcl::Window* pParent, ViewObjectContactOfSdrMediaObj& rViewObjContact ) :
    ::avmedia::MediaWindow( pParent, false ),
    mrViewObjectContactOfSdrMediaObj( rViewObjContact )
{
}


SdrMediaWindow::~SdrMediaWindow()
{
}


void SdrMediaWindow::MouseMove( const MouseEvent& rMEvt )
{
    vcl::Window* pWindow = mrViewObjectContactOfSdrMediaObj.getWindow();

    if( pWindow && getWindow() )
    {
        const MouseEvent aTransformedEvent( pWindow->ScreenToOutputPixel( getWindow()->OutputToScreenPixel( rMEvt.GetPosPixel() ) ),
                                              rMEvt.GetClicks(), rMEvt.GetMode(), rMEvt.GetButtons(), rMEvt.GetModifier() );

        pWindow->MouseMove( aTransformedEvent );
        setPointer( pWindow->GetPointer() );
    }
}


void SdrMediaWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    vcl::Window* pWindow = mrViewObjectContactOfSdrMediaObj.getWindow();

    if( pWindow && getWindow() )
    {
        const MouseEvent aTransformedEvent( pWindow->ScreenToOutputPixel( getWindow()->OutputToScreenPixel( rMEvt.GetPosPixel() ) ),
                                              rMEvt.GetClicks(), rMEvt.GetMode(), rMEvt.GetButtons(), rMEvt.GetModifier() );

        pWindow->MouseButtonDown( aTransformedEvent );
    }
}


void SdrMediaWindow::MouseButtonUp( const MouseEvent& rMEvt )
{
    vcl::Window* pWindow = mrViewObjectContactOfSdrMediaObj.getWindow();

    if( pWindow && getWindow() )
    {
        const MouseEvent aTransformedEvent( pWindow->ScreenToOutputPixel( getWindow()->OutputToScreenPixel( rMEvt.GetPosPixel() ) ),
                                              rMEvt.GetClicks(), rMEvt.GetMode(), rMEvt.GetButtons(), rMEvt.GetModifier() );

        pWindow->MouseButtonUp( aTransformedEvent );
    }
}


void SdrMediaWindow::KeyInput( const KeyEvent& rKEvt )
{
    vcl::Window* pWindow = mrViewObjectContactOfSdrMediaObj.getWindow();

    if( pWindow )
        pWindow->KeyInput( rKEvt );
}


void SdrMediaWindow::KeyUp( const KeyEvent& rKEvt )
{
    vcl::Window* pWindow = mrViewObjectContactOfSdrMediaObj.getWindow();

    if( pWindow )
        pWindow->KeyUp( rKEvt );
}


void SdrMediaWindow::Command( const CommandEvent& rCEvt )
{
    vcl::Window* pWindow = mrViewObjectContactOfSdrMediaObj.getWindow();

    if( pWindow && getWindow() )
    {
        const CommandEvent aTransformedEvent( pWindow->ScreenToOutputPixel( getWindow()->OutputToScreenPixel( rCEvt.GetMousePosPixel() ) ),
                                              rCEvt.GetCommand(), rCEvt.IsMouseEvent(), rCEvt.GetEventData() );

        pWindow->Command( aTransformedEvent );
    }
}


sal_Int8 SdrMediaWindow::AcceptDrop( const AcceptDropEvent& rEvt )
{
    vcl::Window*     pWindow = mrViewObjectContactOfSdrMediaObj.getWindow();
    sal_Int8    nRet = DND_ACTION_NONE;

    if( pWindow )
    {
        DropTargetHelper* pDropTargetHelper = dynamic_cast< DropTargetHelper* >( pWindow );

        if( pDropTargetHelper )
        {
            nRet = pDropTargetHelper->AcceptDrop( rEvt );
        }
    }

    return nRet;
}


sal_Int8 SdrMediaWindow::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    vcl::Window*     pWindow = mrViewObjectContactOfSdrMediaObj.getWindow();
    sal_Int8    nRet = DND_ACTION_NONE;

    if( pWindow )
    {
        DropTargetHelper* pDropTargetHelper = dynamic_cast< DropTargetHelper* >( pWindow );

        if( pDropTargetHelper )
        {
            nRet = pDropTargetHelper->ExecuteDrop( rEvt );
        }
    }

    return nRet;
}


void SdrMediaWindow::StartDrag( sal_Int8 nAction, const Point& rPosPixel )
{
    vcl::Window* pWindow = mrViewObjectContactOfSdrMediaObj.getWindow();

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
