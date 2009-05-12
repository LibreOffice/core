/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: svpframe.cxx,v $
 * $Revision: 1.4 $
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

#include "svpframe.hxx"
#include "svpinst.hxx"
#include "svpgdi.hxx"

#include <basebmp/scanlineformats.hxx>
#include <basegfx/vector/b2ivector.hxx>

using namespace basebmp;
using namespace basegfx;

SvpSalFrame* SvpSalFrame::s_pFocusFrame = NULL;

SvpSalFrame::SvpSalFrame( SvpSalInstance* pInstance,
                          SalFrame* pParent,
                          ULONG nSalFrameStyle,
                          SystemParentData* ) :
    m_pInstance( pInstance ),
    m_pParent( static_cast<SvpSalFrame*>(pParent) ),
    m_nStyle( nSalFrameStyle ),
    m_bVisible( false ),
    m_nMinWidth( 0 ),
    m_nMinHeight( 0 ),
    m_nMaxWidth( 0 ),
    m_nMaxHeight( 0 )
{
    m_aSystemChildData.nSize        = sizeof( SystemChildData );
    m_aSystemChildData.pDisplay     = NULL;
    m_aSystemChildData.aWindow      = 0;
    m_aSystemChildData.pSalFrame    = this;
    m_aSystemChildData.pWidget      = NULL;
    m_aSystemChildData.pVisual      = NULL;
    m_aSystemChildData.nDepth       = 24;
    m_aSystemChildData.aColormap    = 0;
    m_aSystemChildData.pAppContext  = NULL;
    m_aSystemChildData.aShellWindow = 0;
    m_aSystemChildData.pShellWidget = NULL;

    if( m_pParent )
        m_pParent->m_aChildren.push_back( this );

    if( m_pInstance )
        m_pInstance->registerFrame( this );

    SetPosSize( 0, 0, 800, 600, SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT );
}

SvpSalFrame::~SvpSalFrame()
{
    if( m_pInstance )
        m_pInstance->deregisterFrame( this );

    std::list<SvpSalFrame*> Children = m_aChildren;
    for( std::list<SvpSalFrame*>::iterator it = Children.begin();
         it != Children.end(); ++it )
         (*it)->SetParent( m_pParent );
    if( m_pParent )
        m_pParent->m_aChildren.remove( this );
}

void SvpSalFrame::GetFocus()
{
    if( (m_nStyle & (SAL_FRAME_STYLE_OWNERDRAWDECORATION | SAL_FRAME_STYLE_FLOAT)) == 0 )
    {
        if( s_pFocusFrame )
            s_pFocusFrame->LoseFocus();
        s_pFocusFrame = this;
        m_pInstance->PostEvent( this, NULL, SALEVENT_GETFOCUS );
    }
}

void SvpSalFrame::LoseFocus()
{
    if( s_pFocusFrame == this )
    {
        m_pInstance->PostEvent( this, NULL, SALEVENT_LOSEFOCUS );
        s_pFocusFrame = NULL;
    }
}

SalGraphics* SvpSalFrame::GetGraphics()
{
    SvpSalGraphics* pGraphics = new SvpSalGraphics();
    pGraphics->setDevice( m_aFrame );
    m_aGraphics.push_back( pGraphics );
    return pGraphics;
}

void SvpSalFrame::ReleaseGraphics( SalGraphics* pGraphics )
{
    SvpSalGraphics* pSvpGraphics = dynamic_cast<SvpSalGraphics*>(pGraphics);
    m_aGraphics.remove( pSvpGraphics );
    delete pSvpGraphics;
}

BOOL SvpSalFrame::PostEvent( void* pData )
{
    m_pInstance->PostEvent( this, pData, SALEVENT_USEREVENT );
    return TRUE;
}

void SvpSalFrame::PostPaint() const
{
    if( m_bVisible )
    {
        SalPaintEvent aPEvt(0, 0, maGeometry.nWidth, maGeometry.nHeight);
        CallCallback( SALEVENT_PAINT, &aPEvt );
    }
}

void SvpSalFrame::SetTitle( const XubString& )
{
}

void SvpSalFrame::SetIcon( USHORT )
{
}

void SvpSalFrame::SetMenu( SalMenu* )
{
}

void SvpSalFrame::DrawMenuBar()
{
}

void SvpSalFrame::SetExtendedFrameStyle( SalExtStyle )
{
}

void SvpSalFrame::Show( BOOL bVisible, BOOL bNoActivate )
{
    if( bVisible && ! m_bVisible )
    {
        m_bVisible = true;
        m_pInstance->PostEvent( this, NULL, SALEVENT_RESIZE );
        if( ! bNoActivate )
            GetFocus();
    }
    else if( ! bVisible && m_bVisible )
    {
        m_bVisible = false;
        m_pInstance->PostEvent( this, NULL, SALEVENT_RESIZE );
        LoseFocus();
    }
}

void SvpSalFrame::Enable( BOOL )
{
}

void SvpSalFrame::SetMinClientSize( long nWidth, long nHeight )
{
    m_nMinWidth = nWidth;
    m_nMinHeight = nHeight;
}

void SvpSalFrame::SetMaxClientSize( long nWidth, long nHeight )
{
    m_nMaxWidth = nWidth;
    m_nMaxHeight = nHeight;
}

void SvpSalFrame::SetPosSize( long nX, long nY, long nWidth, long nHeight, USHORT nFlags )
{
    if( (nFlags & SAL_FRAME_POSSIZE_X) != 0 )
        maGeometry.nX = nX;
    if( (nFlags & SAL_FRAME_POSSIZE_Y) != 0 )
        maGeometry.nY = nY;
    if( (nFlags & SAL_FRAME_POSSIZE_WIDTH) != 0 )
    {
        maGeometry.nWidth = nWidth;
        if( m_nMaxWidth > 0 && maGeometry.nWidth > (unsigned int)m_nMaxWidth )
            maGeometry.nWidth = m_nMaxWidth;
        if( m_nMinWidth > 0 && maGeometry.nWidth < (unsigned int)m_nMinWidth )
            maGeometry.nWidth = m_nMinWidth;
    }
    if( (nFlags & SAL_FRAME_POSSIZE_HEIGHT) != 0 )
    {
        maGeometry.nHeight = nHeight;
        if( m_nMaxHeight > 0 && maGeometry.nHeight > (unsigned int)m_nMaxHeight )
            maGeometry.nHeight = m_nMaxHeight;
        if( m_nMinHeight > 0 && maGeometry.nHeight < (unsigned int)m_nMinHeight )
            maGeometry.nHeight = m_nMinHeight;
    }
    B2IVector aFrameSize( maGeometry.nWidth, maGeometry.nHeight );
    if( ! m_aFrame.get() || m_aFrame->getSize() != aFrameSize )
    {
        if( aFrameSize.getX() == 0 )
            aFrameSize.setX( 1 );
        if( aFrameSize.getY() == 0 )
            aFrameSize.setY( 1 );
        m_aFrame = createBitmapDevice( aFrameSize, false, SVP_DEFAULT_BITMAP_FORMAT );
        // update device in existing graphics
        for( std::list< SvpSalGraphics* >::iterator it = m_aGraphics.begin();
             it != m_aGraphics.end(); ++it )
             (*it)->setDevice( m_aFrame );
    }
    if( m_bVisible )
        m_pInstance->PostEvent( this, NULL, SALEVENT_RESIZE );
}

void SvpSalFrame::GetClientSize( long& rWidth, long& rHeight )
{
    if( m_bVisible )
    {
        rWidth = maGeometry.nWidth;
        rHeight = maGeometry.nHeight;
    }
    else
        rWidth = rHeight = 0;
}

void SvpSalFrame::GetWorkArea( Rectangle& rRect )
{
    rRect = Rectangle( Point( 0, 0 ),
                       Size( VIRTUAL_DESKTOP_WIDTH, VIRTUAL_DESKTOP_HEIGHT ) );
}

SalFrame* SvpSalFrame::GetParent() const
{
    return m_pParent;
}

#define _FRAMESTATE_MASK_GEOMETRY \
     (SAL_FRAMESTATE_MASK_X     | SAL_FRAMESTATE_MASK_Y |   \
      SAL_FRAMESTATE_MASK_WIDTH | SAL_FRAMESTATE_MASK_HEIGHT)
#define _FRAMESTATE_MASK_MAXIMIZED_GEOMETRY \
     (SAL_FRAMESTATE_MASK_MAXIMIZED_X     | SAL_FRAMESTATE_MASK_MAXIMIZED_Y |   \
      SAL_FRAMESTATE_MASK_MAXIMIZED_WIDTH | SAL_FRAMESTATE_MASK_MAXIMIZED_HEIGHT)

void SvpSalFrame::SetWindowState( const SalFrameState *pState )
{
    if (pState == NULL)
        return;

    // Request for position or size change
    if (pState->mnMask & _FRAMESTATE_MASK_GEOMETRY)
    {
        long nX = maGeometry.nX;
        long nY = maGeometry.nY;
        long nWidth = maGeometry.nWidth;
        long nHeight = maGeometry.nHeight;

        // change requested properties
        if (pState->mnMask & SAL_FRAMESTATE_MASK_X)
            nX = pState->mnX;
        if (pState->mnMask & SAL_FRAMESTATE_MASK_Y)
            nY = pState->mnY;
        if (pState->mnMask & SAL_FRAMESTATE_MASK_WIDTH)
            nWidth = pState->mnWidth;
        if (pState->mnMask & SAL_FRAMESTATE_MASK_HEIGHT)
            nHeight = pState->mnHeight;

        SetPosSize( nX, nY, nWidth, nHeight,
                    SAL_FRAME_POSSIZE_X | SAL_FRAME_POSSIZE_Y |
                    SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT );
    }
}

BOOL SvpSalFrame::GetWindowState( SalFrameState* pState )
{
    pState->mnState = SAL_FRAMESTATE_NORMAL;
    pState->mnX      = maGeometry.nX;
    pState->mnY      = maGeometry.nY;
    pState->mnWidth  = maGeometry.nWidth;
    pState->mnHeight = maGeometry.nHeight;
    pState->mnMask   = _FRAMESTATE_MASK_GEOMETRY | SAL_FRAMESTATE_MASK_STATE;

    return TRUE;
}

void SvpSalFrame::ShowFullScreen( BOOL, sal_Int32 )
{
    SetPosSize( 0, 0, VIRTUAL_DESKTOP_WIDTH, VIRTUAL_DESKTOP_HEIGHT,
                SAL_FRAME_POSSIZE_WIDTH | SAL_FRAME_POSSIZE_HEIGHT );
}

void SvpSalFrame::StartPresentation( BOOL )
{
}

void SvpSalFrame::SetAlwaysOnTop( BOOL )
{
}

void SvpSalFrame::ToTop( USHORT )
{
    GetFocus();
}

void SvpSalFrame::SetPointer( PointerStyle )
{
}

void SvpSalFrame::CaptureMouse( BOOL )
{
}

void SvpSalFrame::SetPointerPos( long, long )
{
}

void SvpSalFrame::Flush()
{
}

void SvpSalFrame::Sync()
{
}

void SvpSalFrame::SetInputContext( SalInputContext* )
{
}

void SvpSalFrame::EndExtTextInput( USHORT )
{
}

String SvpSalFrame::GetKeyName( USHORT )
{
    return String();
}

String SvpSalFrame::GetSymbolKeyName( const XubString&, USHORT )
{
    return String();
}

BOOL SvpSalFrame::MapUnicodeToKeyCode( sal_Unicode, LanguageType, KeyCode& )
{
    return FALSE;
}

LanguageType SvpSalFrame::GetInputLanguage()
{
    return LANGUAGE_DONTKNOW;
}

SalBitmap* SvpSalFrame::SnapShot()
{
    return NULL;
}

void SvpSalFrame::UpdateSettings( AllSettings& )
{
}

void SvpSalFrame::Beep( SoundType )
{
}

const SystemEnvData* SvpSalFrame::GetSystemData() const
{
    return &m_aSystemChildData;
}

SalFrame::SalPointerState SvpSalFrame::GetPointerState()
{
    SalPointerState aState;
    aState.mnState = 0;
    return aState;
}

void SvpSalFrame::SetParent( SalFrame* pNewParent )
{
    if( m_pParent )
        m_pParent->m_aChildren.remove( this );
    m_pParent = static_cast<SvpSalFrame*>(pNewParent);
}

bool SvpSalFrame::SetPluginParent( SystemParentData* )
{
    return true;
}

void SvpSalFrame::SetBackgroundBitmap( SalBitmap* )
{
}

void SvpSalFrame::ResetClipRegion()
{
}

void SvpSalFrame::BeginSetClipRegion( ULONG )
{
}

void SvpSalFrame::UnionClipRegion( long, long, long, long )
{
}

void SvpSalFrame::EndSetClipRegion()
{
}

