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

#include "mediawindow_impl.hxx"
#include "mediaevent_impl.hxx"
#include "mediamisc.hxx"
#include "mediawindow.hrc"
#include "helpids.hrc"

#include <algorithm>
#include <cmath>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#ifdef UNX
#include <vcl/sysdata.hxx>
#endif

#include <com/sun/star/awt/SystemPointer.hpp>
#include <com/sun/star/lang/XComponent.hpp>

using namespace ::com::sun::star;

namespace avmedia { namespace priv {

// ----------------------
// - MediaWindowControl -
// ----------------------

MediaWindowControl::MediaWindowControl( Window* pParent ) :
    MediaControl( pParent, MEDIACONTROLSTYLE_MULTILINE )
{
}

// ---------------------------------------------------------------------

MediaWindowControl::~MediaWindowControl()
{
}

// ---------------------------------------------------------------------

void MediaWindowControl::update()
{
    MediaItem aItem;

    static_cast< MediaWindowImpl* >( GetParent() )->updateMediaItem( aItem );
    setState( aItem );
}

// ---------------------------------------------------------------------

void  MediaWindowControl::execute( const MediaItem& rItem )
{
    static_cast< MediaWindowImpl* >( GetParent() )->executeMediaItem( rItem );
}

// --------------------
// - MediaChildWindow -
// --------------------

MediaChildWindow::MediaChildWindow( Window* pParent ) :
    SystemChildWindow( pParent, WB_CLIPCHILDREN )
{
}

// ---------------------------------------------------------------------

MediaChildWindow::~MediaChildWindow()
{
}

// ---------------------------------------------------------------------

void MediaChildWindow::MouseMove( const MouseEvent& rMEvt )
{
    const MouseEvent aTransformedEvent( GetParent()->ScreenToOutputPixel( OutputToScreenPixel( rMEvt.GetPosPixel() ) ),
                                          rMEvt.GetClicks(), rMEvt.GetMode(), rMEvt.GetButtons(), rMEvt.GetModifier() );

    SystemChildWindow::MouseMove( rMEvt );
    GetParent()->MouseMove( aTransformedEvent );
}

// ---------------------------------------------------------------------

void MediaChildWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    const MouseEvent aTransformedEvent( GetParent()->ScreenToOutputPixel( OutputToScreenPixel( rMEvt.GetPosPixel() ) ),
                                          rMEvt.GetClicks(), rMEvt.GetMode(), rMEvt.GetButtons(), rMEvt.GetModifier() );

    SystemChildWindow::MouseButtonDown( rMEvt );
    GetParent()->MouseButtonDown( aTransformedEvent );
}

// ---------------------------------------------------------------------

void MediaChildWindow::MouseButtonUp( const MouseEvent& rMEvt )
{
    const MouseEvent aTransformedEvent( GetParent()->ScreenToOutputPixel( OutputToScreenPixel( rMEvt.GetPosPixel() ) ),
                                          rMEvt.GetClicks(), rMEvt.GetMode(), rMEvt.GetButtons(), rMEvt.GetModifier() );

    SystemChildWindow::MouseButtonUp( rMEvt );
    GetParent()->MouseButtonUp( aTransformedEvent );
}

// ---------------------------------------------------------------------

void MediaChildWindow::KeyInput( const KeyEvent& rKEvt )
{
    SystemChildWindow::KeyInput( rKEvt );
    GetParent()->KeyInput( rKEvt );
}

// ---------------------------------------------------------------------

void MediaChildWindow::KeyUp( const KeyEvent& rKEvt )
{
    SystemChildWindow::KeyUp( rKEvt );
    GetParent()->KeyUp( rKEvt );
}

// ---------------------------------------------------------------------

void MediaChildWindow::Command( const CommandEvent& rCEvt )
{
    const CommandEvent aTransformedEvent( GetParent()->ScreenToOutputPixel( OutputToScreenPixel( rCEvt.GetMousePosPixel() ) ),
                                            rCEvt.GetCommand(), rCEvt.IsMouseEvent(), rCEvt.GetData() );

    SystemChildWindow::Command( rCEvt );
    GetParent()->Command( aTransformedEvent );
}

// ----------------------
// - MediaWindowImpl -
// ----------------------

MediaWindowImpl::MediaWindowImpl( Window* pParent, MediaWindow* pMediaWindow, bool bInternalMediaControl ) :
    Control( pParent ),
    MediaWindowBaseImpl( pMediaWindow ),
    DropTargetHelper( this ),
    DragSourceHelper( this ),
    mxEventsIf( static_cast< ::cppu::OWeakObject* >( mpEvents = new MediaEventListenersImpl( maChildWindow ) ) ),
    maChildWindow( this ),
    mpMediaWindowControl( bInternalMediaControl ? new MediaWindowControl( this ) : NULL ),
    mpEmptyBmpEx( NULL ),
    mpAudioBmpEx( NULL )
{
    maChildWindow.SetBackground( Color( COL_BLACK ) );
    maChildWindow.SetHelpId( HID_AVMEDIA_PLAYERWINDOW );
    maChildWindow.Hide();

    if( mpMediaWindowControl )
    {
        mpMediaWindowControl->SetSizePixel( mpMediaWindowControl->getMinSizePixel() );
        mpMediaWindowControl->Show();
    }
}

// ---------------------------------------------------------------------

MediaWindowImpl::~MediaWindowImpl()
{
    delete mpEmptyBmpEx;
    delete mpAudioBmpEx;
    delete mpMediaWindowControl;
}

// ---------------------------------------------------------------------

void MediaWindowImpl::cleanUp()
{
    uno::Reference< media::XPlayerWindow > xPlayerWindow( getPlayerWindow() );

    mpEvents->cleanUp();

    if( xPlayerWindow.is() )
    {
        xPlayerWindow->removeKeyListener( uno::Reference< awt::XKeyListener >( mxEventsIf, uno::UNO_QUERY ) );
        xPlayerWindow->removeMouseListener( uno::Reference< awt::XMouseListener >( mxEventsIf, uno::UNO_QUERY ) );
        xPlayerWindow->removeMouseMotionListener( uno::Reference< awt::XMouseMotionListener >( mxEventsIf, uno::UNO_QUERY ) );

        uno::Reference< lang::XComponent > xComponent( xPlayerWindow, uno::UNO_QUERY );
        if( xComponent.is() )
            xComponent->dispose();

        setPlayerWindow( NULL );
    }

    MediaWindowBaseImpl::cleanUp();
}

// ---------------------------------------------------------------------

void MediaWindowImpl::onURLChanged()
{
    if( getPlayer().is() )
    {
        uno::Sequence< uno::Any >              aArgs( 3 );
        uno::Reference< media::XPlayerWindow > xPlayerWindow;
        const Point                            aPoint;
        const Size                             aSize( maChildWindow.GetSizePixel() );
        const sal_Int32                        nWndHandle = 0;

        aArgs[ 0 ] = uno::makeAny( nWndHandle );
        aArgs[ 1 ] = uno::makeAny( awt::Rectangle( aPoint.X(), aPoint.Y(), aSize.Width(), aSize.Height() ) );
        aArgs[ 2 ] = uno::makeAny( reinterpret_cast< sal_IntPtr >( &maChildWindow ) );

        try
        {
            xPlayerWindow = getPlayer()->createPlayerWindow( aArgs );
        }
        catch( uno::RuntimeException )
        {
            // happens eg, on MacOSX where Java frames cannot be created from X11 window handles
        }

        setPlayerWindow( xPlayerWindow );

        if( xPlayerWindow.is() )
        {
            xPlayerWindow->addKeyListener( uno::Reference< awt::XKeyListener >( mxEventsIf, uno::UNO_QUERY ) );
            xPlayerWindow->addMouseListener( uno::Reference< awt::XMouseListener >( mxEventsIf, uno::UNO_QUERY ) );
            xPlayerWindow->addMouseMotionListener( uno::Reference< awt::XMouseMotionListener >( mxEventsIf, uno::UNO_QUERY ) );
            xPlayerWindow->addFocusListener( uno::Reference< awt::XFocusListener >( mxEventsIf, uno::UNO_QUERY ) );
        }
    }
    else
        setPlayerWindow( NULL );

    if( getPlayerWindow().is() )
        maChildWindow.Show();
    else
        maChildWindow.Hide();

    if( mpMediaWindowControl )
    {
        MediaItem aItem;

        updateMediaItem( aItem );
        mpMediaWindowControl->setState( aItem );
    }
}

// ---------------------------------------------------------------------

void MediaWindowImpl::setPosSize( const Rectangle& rRect )
{
    SetPosSizePixel( rRect.TopLeft(), rRect.GetSize() );
}

// ---------------------------------------------------------------------

void MediaWindowImpl::setPointer( const Pointer& rPointer )
{
    uno::Reference< media::XPlayerWindow >  xPlayerWindow( getPlayerWindow() );

    SetPointer( rPointer );
    maChildWindow.SetPointer( rPointer );

    if( xPlayerWindow.is() )
    {
        long nPointer;

        switch( rPointer.GetStyle() )
        {
            case( POINTER_CROSS ): nPointer = awt::SystemPointer::CROSS; break;
            case( POINTER_HAND ): nPointer = awt::SystemPointer::HAND; break;
            case( POINTER_MOVE ): nPointer = awt::SystemPointer::MOVE; break;
            case( POINTER_WAIT ): nPointer = awt::SystemPointer::WAIT; break;

            default: nPointer = awt::SystemPointer::ARROW; break;
        }

        xPlayerWindow->setPointerType( nPointer );
    }
}

// ---------------------------------------------------------------------

void MediaWindowImpl::Resize()
{
    uno::Reference< media::XPlayerWindow >  xPlayerWindow( getPlayerWindow() );
    const Size                              aCurSize( GetOutputSizePixel() );
    const sal_Int32                         nOffset( mpMediaWindowControl ? AVMEDIA_CONTROLOFFSET : 0 );
    Size                                    aPlayerWindowSize( aCurSize.Width() - ( nOffset << 1 ),
                                                               aCurSize.Height() - ( nOffset << 1 ) );

    if( mpMediaWindowControl )
    {
        const sal_Int32 nControlHeight = mpMediaWindowControl->GetSizePixel().Height();
        const sal_Int32 nControlY = ::std::max( aCurSize.Height() - nControlHeight - nOffset, 0L );

        aPlayerWindowSize.Height() = ( nControlY - ( nOffset << 1 ) );
        mpMediaWindowControl->SetPosSizePixel( Point( nOffset, nControlY ), Size( aCurSize.Width() - ( nOffset << 1 ), nControlHeight ) );
    }

    if( xPlayerWindow.is() )
        xPlayerWindow->setPosSize( 0, 0, aPlayerWindowSize.Width(), aPlayerWindowSize.Height(), 0 );

    maChildWindow.SetPosSizePixel( Point( nOffset, nOffset ), aPlayerWindowSize );
}

// ---------------------------------------------------------------------

void MediaWindowImpl::StateChanged( StateChangedType eType )
{
    uno::Reference< media::XPlayerWindow > xPlayerWindow( getPlayerWindow() );

    if( xPlayerWindow.is() )
    {
        // stop playing when going disabled or hidden
        switch( eType )
        {
            case STATE_CHANGE_VISIBLE:
            {
                stopPlayingInternal( !IsVisible() );
                xPlayerWindow->setVisible( IsVisible() );
            }
            break;

            case STATE_CHANGE_ENABLE:
            {
                stopPlayingInternal( !IsEnabled() );
                xPlayerWindow->setEnable( IsEnabled() );
            }
            break;

            default:
            break;
        }
    }
}

// ---------------------------------------------------------------------

void MediaWindowImpl::Paint( const Rectangle& )
{
    BitmapEx* pLogo = NULL;

    if( !getPlayer().is() )
    {
        if( !mpEmptyBmpEx )
            mpEmptyBmpEx = new BitmapEx( AVMEDIA_RESID( AVMEDIA_BMP_EMPTYLOGO ) );

        pLogo = mpEmptyBmpEx;
    }
    else if( !getPlayerWindow().is() )
    {
        if( !mpAudioBmpEx )
            mpAudioBmpEx = new BitmapEx( AVMEDIA_RESID( AVMEDIA_BMP_AUDIOLOGO ) );

        pLogo = mpAudioBmpEx;
    }

    const Point     aBasePos( maChildWindow.GetPosPixel() );
    const Rectangle aVideoRect( aBasePos, maChildWindow.GetSizePixel() );

    if( pLogo && !pLogo->IsEmpty() && ( aVideoRect.GetWidth() > 0 ) && ( aVideoRect.GetHeight() > 0 ) )
    {
        Size        aLogoSize( pLogo->GetSizePixel() );
        const Color aBackgroundColor( 0, 0, 0 );

        SetLineColor( aBackgroundColor );
        SetFillColor( aBackgroundColor );
        DrawRect( aVideoRect );

        if( ( aLogoSize.Width() > aVideoRect.GetWidth() || aLogoSize.Height() > aVideoRect.GetHeight() ) &&
            ( aLogoSize.Height() > 0 ) )
        {
            const double fLogoWH = (double) aLogoSize.Width() / aLogoSize.Height();

            if( fLogoWH < ( (double) aVideoRect.GetWidth() / aVideoRect.GetHeight() ) )
            {
                aLogoSize.Width() = (long) ( aVideoRect.GetHeight() * fLogoWH );
                aLogoSize.Height()= aVideoRect.GetHeight();
            }
            else
            {
                aLogoSize.Width() = aVideoRect.GetWidth();
                aLogoSize.Height()= (long) ( aVideoRect.GetWidth() / fLogoWH );
            }
        }

        DrawBitmapEx( Point( aBasePos.X() + ( ( aVideoRect.GetWidth() - aLogoSize.Width() ) >> 1 ),
                             aBasePos.Y() + ( ( aVideoRect.GetHeight() - aLogoSize.Height() ) >> 1 ) ),
                      aLogoSize, *pLogo );
    }
}

// ---------------------------------------------------------------------

void MediaWindowImpl::GetFocus()
{
}

// ---------------------------------------------------------------------

void MediaWindowImpl::MouseMove( const MouseEvent& rMEvt )
{
    MediaWindow* pMediaWindow = getMediaWindow();

    if( pMediaWindow )
        pMediaWindow->MouseMove( rMEvt );
}

// ---------------------------------------------------------------------

void MediaWindowImpl::MouseButtonDown( const MouseEvent& rMEvt )
{
    MediaWindow* pMediaWindow = getMediaWindow();

    if( pMediaWindow )
        pMediaWindow->MouseButtonDown( rMEvt );
}

// ---------------------------------------------------------------------

void MediaWindowImpl::MouseButtonUp( const MouseEvent& rMEvt )
{
    MediaWindow* pMediaWindow = getMediaWindow();

    if( pMediaWindow )
        pMediaWindow->MouseButtonUp( rMEvt );
}

// ---------------------------------------------------------------------

void MediaWindowImpl::KeyInput( const KeyEvent& rKEvt )
{
    MediaWindow* pMediaWindow = getMediaWindow();

    if( pMediaWindow )
        pMediaWindow->KeyInput( rKEvt );
}

// ---------------------------------------------------------------------

void MediaWindowImpl::KeyUp( const KeyEvent& rKEvt )
{
    MediaWindow* pMediaWindow = getMediaWindow();

    if( pMediaWindow )
        pMediaWindow->KeyUp( rKEvt );
}

// ---------------------------------------------------------------------

void MediaWindowImpl::Command( const CommandEvent& rCEvt )
{
    MediaWindow* pMediaWindow = getMediaWindow();

    if( pMediaWindow )
        pMediaWindow->Command( rCEvt );
}

// ---------------------------------------------------------------------

sal_Int8 MediaWindowImpl::AcceptDrop( const AcceptDropEvent& rEvt )
{
    MediaWindow* pMediaWindow = getMediaWindow();
    return( pMediaWindow ? pMediaWindow->AcceptDrop( rEvt ) : 0 );
}

// ---------------------------------------------------------------------

sal_Int8 MediaWindowImpl::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    MediaWindow* pMediaWindow = getMediaWindow();
    return( pMediaWindow ? pMediaWindow->ExecuteDrop( rEvt ) : 0 );
}

// ---------------------------------------------------------------------

void MediaWindowImpl::StartDrag( sal_Int8 nAction, const Point& rPosPixel )
{
    MediaWindow* pMediaWindow = getMediaWindow();

    if( pMediaWindow )
        pMediaWindow->StartDrag( nAction, rPosPixel );
}

} // namespace priv
} // namespace avmedia

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
