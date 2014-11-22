/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "opengl/win/gdiimpl.hxx"

#include <win/wincomp.hxx>
#include <win/saldata.hxx>
#include <win/salframe.h>

WinOpenGLSalGraphicsImpl::WinOpenGLSalGraphicsImpl(WinSalGraphics& rGraphics):
    mrParent(rGraphics)
{
}

void WinOpenGLSalGraphicsImpl::copyBits( const SalTwoRect& rPosAry, SalGraphics* pSrcGraphics )
{
    OpenGLSalGraphicsImpl *pImpl = pSrcGraphics ? static_cast< OpenGLSalGraphicsImpl* >(pSrcGraphics->GetImpl()) : static_cast< OpenGLSalGraphicsImpl *>(mrParent.GetImpl());
    OpenGLSalGraphicsImpl::DoCopyBits( rPosAry, *pImpl );
}

GLfloat WinOpenGLSalGraphicsImpl::GetWidth() const
{
    if( mrParent.gethWnd() && IsWindow( mrParent.gethWnd() ) )
    {
        WinSalFrame* pFrame = GetWindowPtr( mrParent.gethWnd() );
        if( pFrame )
        {
            if( pFrame->maGeometry.nWidth )
                return pFrame->maGeometry.nWidth;
            else
            {
                // TODO: perhaps not needed, maGeometry should always be up-to-date
                RECT aRect;
                GetClientRect( mrParent.gethWnd(), &aRect );
                return aRect.right;
            }
        }
    }

    return 1;
}

GLfloat WinOpenGLSalGraphicsImpl::GetHeight() const
{
    if( mrParent.gethWnd() && IsWindow( mrParent.gethWnd() ) )
    {
        WinSalFrame* pFrame = GetWindowPtr( mrParent.gethWnd() );
        if( pFrame )
        {
            if( pFrame->maGeometry.nHeight )
                return pFrame->maGeometry.nHeight;
            else
            {
                // TODO: perhaps not needed, maGeometry should always be up-to-date
                RECT aRect;
                GetClientRect( mrParent.gethWnd(), &aRect );
                return aRect.bottom;
            }
        }
    }

    return 1;
}

bool WinOpenGLSalGraphicsImpl::IsOffscreen() const
{
    WinSalFrame* pFrame = GetWindowPtr( mrParent.gethWnd() );
    return ( pFrame == NULL );
}

OpenGLContext* WinOpenGLSalGraphicsImpl::CreateWinContext()
{
    OpenGLContext* pContext = new OpenGLContext();
    pContext->requestSingleBufferedRendering();
    pContext->init( mrParent.mhLocalDC, mrParent.mhWnd );
    return pContext;
}

bool WinOpenGLSalGraphicsImpl::CompareWinContext( OpenGLContext* pContext )
{
    if( !pContext || !pContext->isInitialized() )
        return false;
    return ( pContext->getOpenGLWindow().hWnd == mrParent.mhWnd );
}

OpenGLContext* WinOpenGLSalGraphicsImpl::CreatePixmapContext()
{
    OpenGLContext* pContext = new OpenGLContext();
    pContext->requestVirtualDevice();
    pContext->requestSingleBufferedRendering();
    pContext->init( mrParent.mhLocalDC, mrParent.mhWnd );
    return pContext;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
