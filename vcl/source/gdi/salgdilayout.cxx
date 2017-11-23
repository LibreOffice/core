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

#include <memory>
#include <config_features.h>
#if HAVE_FEATURE_OPENGL
#include <openglgdiimpl.hxx>
#include <opengl/zone.hxx>
#include <desktop/exithelper.h>
#ifdef _WIN32
#include <svsys.h>
#endif
#endif
#include <salgdi.hxx>
#include <salframe.hxx>
#include <basegfx/numeric/ftools.hxx> //for F_PI180

// The only common SalFrame method

SalFrameGeometry SalFrame::GetGeometry()
{
    // mirror frame coordinates at parent
    SalFrame *pParent = GetParent();
    if( pParent && AllSettings::GetLayoutRTL() )
    {
        SalFrameGeometry aGeom = maGeometry;
        int parent_x = aGeom.nX - pParent->maGeometry.nX;
        aGeom.nX = pParent->maGeometry.nX + pParent->maGeometry.nWidth - maGeometry.nWidth - parent_x;
        return aGeom;
    }
    else
        return maGeometry;
}

SalGraphics::SalGraphics()
:   m_nLayout( SalLayoutFlags::NONE ),
    m_bAntiAliasB2DDraw(false)
{
    // read global RTL settings
    if( AllSettings::GetLayoutRTL() )
        m_nLayout = SalLayoutFlags::BiDiRtl;
}

SalGraphics::~SalGraphics()
{
}

#if HAVE_FEATURE_OPENGL

namespace
{
    void disableOpenGLAndTerminateForRestart()
    {
        OpenGLZone::hardDisable();
#ifdef _WIN32
        TerminateProcess(GetCurrentProcess(), EXITHELPER_NORMAL_RESTART);
#endif
    }
}

rtl::Reference<OpenGLContext> SalGraphics::GetOpenGLContext() const
{
    OpenGLSalGraphicsImpl *pImpl = dynamic_cast<OpenGLSalGraphicsImpl*>(GetImpl());
    if (pImpl)
    {
        // If we notice that OpenGL is broken the first time being called, it is not too late to call
        // disableOpenGLAndTerminateForRestart(). The first time this will be called is from displaying
        // the splash screen, so if OpenGL is broken, it is "early enough" for us to be able to disable
        // OpenGL and terminate bluntly with EXITHELPER_NORMAL_RESTART, thus causing the wrapper process
        // to restart us, then without using OpenGL.
        static bool bFirstCall = true;
        rtl::Reference<OpenGLContext> xRet(pImpl->GetOpenGLContext());
        if (!xRet.is() && bFirstCall)
            disableOpenGLAndTerminateForRestart();
        bFirstCall = false;
        return xRet;
    }
    return nullptr;
}
#endif

bool SalGraphics::drawTransformedBitmap(
    const basegfx::B2DPoint& /* rNull */,
    const basegfx::B2DPoint& /* rX */,
    const basegfx::B2DPoint& /* rY */,
    const SalBitmap& /* rSourceBitmap */,
    const SalBitmap* /* pAlphaBitmap */)
{
    // here direct support for transformed bitmaps can be implemented
    return false;
}

void SalGraphics::mirror( long& x, const OutputDevice *pOutDev ) const
{
    long w;
    if( pOutDev && pOutDev->GetOutDevType() == OUTDEV_VIRDEV )
        w = pOutDev->GetOutputWidthPixel();
    else
        w = GetGraphicsWidth();

    if( w )
    {
        if( pOutDev && pOutDev->ImplIsAntiparallel() )
        {
            OutputDevice *pOutDevRef = const_cast<OutputDevice*>(pOutDev);
            // mirror this window back
            if( m_nLayout & SalLayoutFlags::BiDiRtl )
            {
                long devX = w-pOutDevRef->GetOutputWidthPixel()-pOutDevRef->GetOutOffXPixel();   // re-mirrored mnOutOffX
                x = devX + (x - pOutDevRef->GetOutOffXPixel());
            }
            else
            {
                long devX = pOutDevRef->GetOutOffXPixel();   // re-mirrored mnOutOffX
                x = pOutDevRef->GetOutputWidthPixel() - (x - devX) + pOutDevRef->GetOutOffXPixel() - 1;
            }
        }
        else if( m_nLayout & SalLayoutFlags::BiDiRtl )
            x = w-1-x;
    }
}

void SalGraphics::mirror( long& x, long nWidth, const OutputDevice *pOutDev, bool bBack ) const
{
    long w;
    if( pOutDev && pOutDev->GetOutDevType() == OUTDEV_VIRDEV )
        w = pOutDev->GetOutputWidthPixel();
    else
        w = GetGraphicsWidth();

    if( w )
    {
        if( pOutDev && pOutDev->ImplIsAntiparallel() )
        {
            OutputDevice *pOutDevRef = const_cast<OutputDevice*>(pOutDev);
            // mirror this window back
            if( m_nLayout & SalLayoutFlags::BiDiRtl )
            {
                long devX = w-pOutDevRef->GetOutputWidthPixel()-pOutDevRef->GetOutOffXPixel();   // re-mirrored mnOutOffX
                if( bBack )
                    x = x - devX + pOutDevRef->GetOutOffXPixel();
                else
                    x = devX + (x - pOutDevRef->GetOutOffXPixel());
            }
            else
            {
                long devX = pOutDevRef->GetOutOffXPixel();   // re-mirrored mnOutOffX
                if( bBack )
                    x = devX + (pOutDevRef->GetOutputWidthPixel() + devX) - (x + nWidth);
                else
                    x = pOutDevRef->GetOutputWidthPixel() - (x - devX) + pOutDevRef->GetOutOffXPixel() - nWidth;
            }
        }
        else if( m_nLayout & SalLayoutFlags::BiDiRtl )
            x = w-nWidth-x;

    }
}

bool SalGraphics::mirror( sal_uInt32 nPoints, const SalPoint *pPtAry, SalPoint *pPtAry2, const OutputDevice *pOutDev ) const
{
    long w;
    if( pOutDev && pOutDev->GetOutDevType() == OUTDEV_VIRDEV )
        w = pOutDev->GetOutputWidthPixel();
    else
        w = GetGraphicsWidth();

    if( w )
    {
        sal_uInt32 i, j;

        if( pOutDev && pOutDev->ImplIsAntiparallel() )
        {
            OutputDevice *pOutDevRef = const_cast<OutputDevice*>(pOutDev);
            // mirror this window back
            if( m_nLayout & SalLayoutFlags::BiDiRtl )
            {
                long devX = w-pOutDevRef->GetOutputWidthPixel()-pOutDevRef->GetOutOffXPixel();   // re-mirrored mnOutOffX
                for( i=0, j=nPoints-1; i<nPoints; i++,j-- )
                {
                    pPtAry2[j].mnX = devX + (pPtAry[i].mnX - pOutDevRef->GetOutOffXPixel());
                    pPtAry2[j].mnY = pPtAry[i].mnY;
                }
            }
            else
            {
                long devX = pOutDevRef->GetOutOffXPixel();   // re-mirrored mnOutOffX
                for( i=0, j=nPoints-1; i<nPoints; i++,j-- )
                {
                    pPtAry2[j].mnX = pOutDevRef->GetOutputWidthPixel() - (pPtAry[i].mnX - devX) + pOutDevRef->GetOutOffXPixel() - 1;
                    pPtAry2[j].mnY = pPtAry[i].mnY;
                }
            }
        }
        else if( m_nLayout & SalLayoutFlags::BiDiRtl )
        {
            for( i=0, j=nPoints-1; i<nPoints; i++,j-- )
            {
                pPtAry2[j].mnX = w-1-pPtAry[i].mnX;
                pPtAry2[j].mnY = pPtAry[i].mnY;
            }
        }
        return true;
    }
    else
        return false;
}

void SalGraphics::mirror( vcl::Region& rRgn, const OutputDevice *pOutDev ) const
{
    if( rRgn.HasPolyPolygonOrB2DPolyPolygon() )
    {
        const basegfx::B2DPolyPolygon aPolyPoly(mirror(rRgn.GetAsB2DPolyPolygon(), pOutDev));

        rRgn = vcl::Region(aPolyPoly);
    }
    else
    {
        RectangleVector aRectangles;
        rRgn.GetRegionRectangles(aRectangles);
        rRgn.SetEmpty();

        for(RectangleVector::iterator aRectIter(aRectangles.begin()); aRectIter != aRectangles.end(); ++aRectIter)
        {
            mirror(*aRectIter, pOutDev);
            rRgn.Union(*aRectIter);
        }

        //ImplRegionInfo        aInfo;
        //bool              bRegionRect;
        //Region              aMirroredRegion;
        //long nX, nY, nWidth, nHeight;

        //bRegionRect = rRgn.ImplGetFirstRect( aInfo, nX, nY, nWidth, nHeight );
        //while ( bRegionRect )
        //{
        //    Rectangle aRect( Point(nX, nY), Size(nWidth, nHeight) );
        //    mirror( aRect, pOutDev, bBack );
        //    aMirroredRegion.Union( aRect );
        //    bRegionRect = rRgn.ImplGetNextRect( aInfo, nX, nY, nWidth, nHeight );
        //}
        //rRgn = aMirroredRegion;
    }
}

void SalGraphics::mirror( tools::Rectangle& rRect, const OutputDevice *pOutDev, bool bBack ) const
{
    long nWidth = rRect.GetWidth();
    long x      = rRect.Left();
    long x_org = x;

    mirror( x, nWidth, pOutDev, bBack );
    rRect.Move( x - x_org, 0 );
}

basegfx::B2DPoint SalGraphics::mirror( const basegfx::B2DPoint& i_rPoint, const OutputDevice *i_pOutDev ) const
{
    long w;
    if( i_pOutDev && i_pOutDev->GetOutDevType() == OUTDEV_VIRDEV )
        w = i_pOutDev->GetOutputWidthPixel();
    else
        w = GetGraphicsWidth();

    SAL_WARN_IF( !w, "vcl", "missing graphics width" );

    basegfx::B2DPoint aRet( i_rPoint );
    if( w )
    {
        if( i_pOutDev && !i_pOutDev->IsRTLEnabled() )
        {
            OutputDevice *pOutDevRef = const_cast<OutputDevice*>(i_pOutDev);
            // mirror this window back
            double devX = w-pOutDevRef->GetOutputWidthPixel()-pOutDevRef->GetOutOffXPixel();   // re-mirrored mnOutOffX
            aRet.setX( devX + (i_rPoint.getX() - pOutDevRef->GetOutOffXPixel()) );
        }
        else
            aRet.setX( w-1-i_rPoint.getX() );
    }
    return aRet;
}

basegfx::B2DPolygon SalGraphics::mirror( const basegfx::B2DPolygon& i_rPoly, const OutputDevice *i_pOutDev ) const
{
    long w;
    if( i_pOutDev && i_pOutDev->GetOutDevType() == OUTDEV_VIRDEV )
        w = i_pOutDev->GetOutputWidthPixel();
    else
        w = GetGraphicsWidth();

    SAL_WARN_IF( !w, "vcl", "missing graphics width" );

    basegfx::B2DPolygon aRet;
    if( w )
    {
        sal_Int32 nPoints = i_rPoly.count();
        for( sal_Int32 i = 0; i < nPoints; i++ )
        {
            aRet.append( mirror( i_rPoly.getB2DPoint( i ), i_pOutDev ) );
            if( i_rPoly.isPrevControlPointUsed( i ) )
                aRet.setPrevControlPoint( i, mirror( i_rPoly.getPrevControlPoint( i ), i_pOutDev ) );
            if( i_rPoly.isNextControlPointUsed( i ) )
                aRet.setNextControlPoint( i, mirror( i_rPoly.getNextControlPoint( i ), i_pOutDev ) );
        }
        aRet.setClosed( i_rPoly.isClosed() );
        aRet.flip();
    }
    else
        aRet = i_rPoly;
    return aRet;
}

basegfx::B2DPolyPolygon SalGraphics::mirror( const basegfx::B2DPolyPolygon& i_rPoly, const OutputDevice *i_pOutDev ) const
{
    long w;
    if( i_pOutDev && i_pOutDev->GetOutDevType() == OUTDEV_VIRDEV )
        w = i_pOutDev->GetOutputWidthPixel();
    else
        w = GetGraphicsWidth();

    SAL_WARN_IF( !w, "vcl", "missing graphics width" );

    basegfx::B2DPolyPolygon aRet;
    if( w )
    {
        sal_Int32 nPoly = i_rPoly.count();
        for( sal_Int32 i = 0; i < nPoly; i++ )
            aRet.append( mirror( i_rPoly.getB2DPolygon( i ), i_pOutDev ) );
        aRet.setClosed( i_rPoly.isClosed() );
        aRet.flip();
    }
    else
        aRet = i_rPoly;
    return aRet;
}

bool SalGraphics::SetClipRegion( const vcl::Region& i_rClip, const OutputDevice *pOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || (pOutDev && pOutDev->IsRTLEnabled()) )
    {
        vcl::Region aMirror( i_rClip );
        mirror( aMirror, pOutDev );
        return setClipRegion( aMirror );
    }
    return setClipRegion( i_rClip );
}

void SalGraphics::DrawPixel( long nX, long nY, const OutputDevice *pOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || (pOutDev && pOutDev->IsRTLEnabled()) )
        mirror( nX, pOutDev );
    drawPixel( nX, nY );
}

void SalGraphics::DrawPixel( long nX, long nY, SalColor nSalColor, const OutputDevice *pOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || (pOutDev && pOutDev->IsRTLEnabled()) )
        mirror( nX, pOutDev );
    drawPixel( nX, nY, nSalColor );
}

void SalGraphics::DrawLine( long nX1, long nY1, long nX2, long nY2, const OutputDevice *pOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || (pOutDev && pOutDev->IsRTLEnabled()) )
    {
        mirror( nX1, pOutDev );
        mirror( nX2, pOutDev );
    }
    drawLine( nX1, nY1, nX2, nY2 );
}

void SalGraphics::DrawRect( long nX, long nY, long nWidth, long nHeight, const OutputDevice *pOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || (pOutDev && pOutDev->IsRTLEnabled()) )
        mirror( nX, nWidth, pOutDev );
    drawRect( nX, nY, nWidth, nHeight );
}

void SalGraphics::DrawPolyLine( sal_uInt32 nPoints, SalPoint const * pPtAry, const OutputDevice *pOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || (pOutDev && pOutDev->IsRTLEnabled()) )
    {
        std::unique_ptr<SalPoint[]> pPtAry2(new SalPoint[nPoints]);
        bool bCopied = mirror( nPoints, pPtAry, pPtAry2.get(), pOutDev );
        drawPolyLine( nPoints, bCopied ? pPtAry2.get() : pPtAry );
    }
    else
        drawPolyLine( nPoints, pPtAry );
}

void SalGraphics::DrawPolygon( sal_uInt32 nPoints, const SalPoint* pPtAry, const OutputDevice *pOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || (pOutDev && pOutDev->IsRTLEnabled()) )
    {
        std::unique_ptr<SalPoint[]> pPtAry2(new SalPoint[nPoints]);
        bool bCopied = mirror( nPoints, pPtAry, pPtAry2.get(), pOutDev );
        drawPolygon( nPoints, bCopied ? pPtAry2.get() : pPtAry );
    }
    else
        drawPolygon( nPoints, pPtAry );
}

void SalGraphics::DrawPolyPolygon( sal_uInt32 nPoly, const sal_uInt32* pPoints, PCONSTSALPOINT* pPtAry, const OutputDevice *pOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || (pOutDev && pOutDev->IsRTLEnabled()) )
    {
        // TODO: optimize, reduce new/delete calls
        std::unique_ptr<SalPoint*[]> pPtAry2( new SalPoint*[nPoly] );
        sal_uLong i;
        for(i=0; i<nPoly; i++)
        {
            sal_uLong nPoints = pPoints[i];
            pPtAry2[i] = new SalPoint[ nPoints ];
            mirror( nPoints, pPtAry[i], pPtAry2[i], pOutDev );
        }

        drawPolyPolygon( nPoly, pPoints, const_cast<PCONSTSALPOINT*>(pPtAry2.get()) );

        for(i=0; i<nPoly; i++)
            delete [] pPtAry2[i];
    }
    else
        drawPolyPolygon( nPoly, pPoints, pPtAry );
}

bool SalGraphics::DrawPolyPolygon( const basegfx::B2DPolyPolygon& i_rPolyPolygon, double i_fTransparency, const OutputDevice* i_pOutDev )
{
    bool bRet = false;
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || (i_pOutDev && i_pOutDev->IsRTLEnabled()) )
    {
        basegfx::B2DPolyPolygon aMirror( mirror( i_rPolyPolygon, i_pOutDev ) );
        bRet = drawPolyPolygon( aMirror, i_fTransparency );
    }
    else
        bRet = drawPolyPolygon( i_rPolyPolygon, i_fTransparency );
    return bRet;
}

bool SalGraphics::DrawPolyLineBezier( sal_uInt32 nPoints, const SalPoint* pPtAry, const PolyFlags* pFlgAry, const OutputDevice* pOutDev )
{
    bool bResult = false;
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || (pOutDev && pOutDev->IsRTLEnabled()) )
    {
        std::unique_ptr<SalPoint[]> pPtAry2(new SalPoint[nPoints]);
        bool bCopied = mirror( nPoints, pPtAry, pPtAry2.get(), pOutDev );
        bResult = drawPolyLineBezier( nPoints, bCopied ? pPtAry2.get() : pPtAry, pFlgAry );
    }
    else
        bResult = drawPolyLineBezier( nPoints, pPtAry, pFlgAry );
    return bResult;
}

bool SalGraphics::DrawPolygonBezier( sal_uInt32 nPoints, const SalPoint* pPtAry, const PolyFlags* pFlgAry, const OutputDevice* pOutDev )
{
    bool bResult = false;
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || (pOutDev && pOutDev->IsRTLEnabled()) )
    {
        std::unique_ptr<SalPoint[]> pPtAry2(new SalPoint[nPoints]);
        bool bCopied = mirror( nPoints, pPtAry, pPtAry2.get(), pOutDev );
        bResult = drawPolygonBezier( nPoints, bCopied ? pPtAry2.get() : pPtAry, pFlgAry );
    }
    else
        bResult = drawPolygonBezier( nPoints, pPtAry, pFlgAry );
    return bResult;
}

bool SalGraphics::DrawPolyPolygonBezier( sal_uInt32 i_nPoly, const sal_uInt32* i_pPoints,
                                         const SalPoint* const* i_pPtAry, const PolyFlags* const* i_pFlgAry, const OutputDevice* i_pOutDev )
{
    bool bRet = false;
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || (i_pOutDev && i_pOutDev->IsRTLEnabled()) )
    {
        // TODO: optimize, reduce new/delete calls
        std::unique_ptr<SalPoint*[]> pPtAry2( new SalPoint*[i_nPoly] );
        sal_uLong i;
        for(i=0; i<i_nPoly; i++)
        {
            sal_uLong nPoints = i_pPoints[i];
            pPtAry2[i] = new SalPoint[ nPoints ];
            mirror( nPoints, i_pPtAry[i], pPtAry2[i], i_pOutDev );
        }

        bRet = drawPolyPolygonBezier( i_nPoly, i_pPoints, const_cast<PCONSTSALPOINT const *>(pPtAry2.get()), i_pFlgAry );

        for(i=0; i<i_nPoly; i++)
            delete [] pPtAry2[i];
    }
    else
        bRet = drawPolyPolygonBezier( i_nPoly, i_pPoints, i_pPtAry, i_pFlgAry );
    return bRet;
}

bool SalGraphics::DrawPolyLine( const basegfx::B2DPolygon& i_rPolygon,
                                double i_fTransparency,
                                const basegfx::B2DVector& i_rLineWidth,
                                basegfx::B2DLineJoin i_eLineJoin,
                                css::drawing::LineCap i_eLineCap,
                                double i_fMiterMinimumAngle,
                                const OutputDevice* i_pOutDev )
{
    bool bRet = false;
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || (i_pOutDev && i_pOutDev->IsRTLEnabled()) )
    {
        basegfx::B2DPolygon aMirror( mirror( i_rPolygon, i_pOutDev ) );
        bRet = drawPolyLine( aMirror, i_fTransparency, i_rLineWidth, i_eLineJoin, i_eLineCap, i_fMiterMinimumAngle );
    }
    else
        bRet = drawPolyLine( i_rPolygon, i_fTransparency, i_rLineWidth, i_eLineJoin, i_eLineCap, i_fMiterMinimumAngle );
    return bRet;
}

bool SalGraphics::DrawGradient( const tools::PolyPolygon& rPolyPoly, const Gradient& rGradient )
{
    return drawGradient( rPolyPoly, rGradient );
}

void SalGraphics::CopyArea( long nDestX, long nDestY,
                            long nSrcX, long nSrcY,
                            long nSrcWidth, long nSrcHeight,
                            const OutputDevice *pOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || (pOutDev && pOutDev->IsRTLEnabled()) )
    {
        mirror( nDestX, nSrcWidth, pOutDev );
        mirror( nSrcX, nSrcWidth, pOutDev );
    }
    copyArea( nDestX, nDestY, nSrcX, nSrcY, nSrcWidth, nSrcHeight, true/*bWindowInvalidate*/ );
}

void SalGraphics::CopyBits( const SalTwoRect& rPosAry,
                            SalGraphics* pSrcGraphics, const OutputDevice *pOutDev, const OutputDevice *pSrcOutDev )
{
    if( ( (m_nLayout & SalLayoutFlags::BiDiRtl) || (pOutDev && pOutDev->IsRTLEnabled()) ) ||
        (pSrcGraphics && ( (pSrcGraphics->GetLayout() & SalLayoutFlags::BiDiRtl)  || (pSrcOutDev && pSrcOutDev->IsRTLEnabled()) ) ) )
    {
        SalTwoRect aPosAry2 = rPosAry;
        if( (pSrcGraphics && (pSrcGraphics->GetLayout() & SalLayoutFlags::BiDiRtl)) || (pSrcOutDev && pSrcOutDev->IsRTLEnabled()) )
            mirror( aPosAry2.mnSrcX, aPosAry2.mnSrcWidth, pSrcOutDev );
        if( (m_nLayout & SalLayoutFlags::BiDiRtl) || (pOutDev && pOutDev->IsRTLEnabled()) )
            mirror( aPosAry2.mnDestX, aPosAry2.mnDestWidth, pOutDev );
        copyBits( aPosAry2, pSrcGraphics );
    }
    else
        copyBits( rPosAry, pSrcGraphics );
}

void SalGraphics::DrawBitmap( const SalTwoRect& rPosAry,
                              const SalBitmap& rSalBitmap, const OutputDevice *pOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || (pOutDev && pOutDev->IsRTLEnabled()) )
    {
        SalTwoRect aPosAry2 = rPosAry;
        mirror( aPosAry2.mnDestX, aPosAry2.mnDestWidth, pOutDev );
        drawBitmap( aPosAry2, rSalBitmap );
    }
    else
        drawBitmap( rPosAry, rSalBitmap );
}

void SalGraphics::DrawBitmap( const SalTwoRect& rPosAry,
                              const SalBitmap& rSalBitmap,
                              const SalBitmap& rTransparentBitmap, const OutputDevice *pOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || (pOutDev && pOutDev->IsRTLEnabled()) )
    {
        SalTwoRect aPosAry2 = rPosAry;
        mirror( aPosAry2.mnDestX, aPosAry2.mnDestWidth, pOutDev );
        drawBitmap( aPosAry2, rSalBitmap, rTransparentBitmap );
    }
    else
        drawBitmap( rPosAry, rSalBitmap, rTransparentBitmap );
}

void SalGraphics::DrawMask( const SalTwoRect& rPosAry,
                            const SalBitmap& rSalBitmap,
                            SalColor nMaskColor, const OutputDevice *pOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || (pOutDev && pOutDev->IsRTLEnabled()) )
    {
        SalTwoRect aPosAry2 = rPosAry;
        mirror( aPosAry2.mnDestX, aPosAry2.mnDestWidth, pOutDev );
        drawMask( aPosAry2, rSalBitmap, nMaskColor );
    }
    else
        drawMask( rPosAry, rSalBitmap, nMaskColor );
}

SalBitmap* SalGraphics::GetBitmap( long nX, long nY, long nWidth, long nHeight, const OutputDevice *pOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || (pOutDev && pOutDev->IsRTLEnabled()) )
        mirror( nX, nWidth, pOutDev );
    return getBitmap( nX, nY, nWidth, nHeight );
}

SalColor SalGraphics::GetPixel( long nX, long nY, const OutputDevice *pOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || (pOutDev && pOutDev->IsRTLEnabled()) )
        mirror( nX, pOutDev );
    return getPixel( nX, nY );
}

void SalGraphics::Invert( long nX, long nY, long nWidth, long nHeight, SalInvert nFlags, const OutputDevice *pOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || (pOutDev && pOutDev->IsRTLEnabled()) )
        mirror( nX, nWidth, pOutDev );
    invert( nX, nY, nWidth, nHeight, nFlags );
}

void SalGraphics::Invert( sal_uInt32 nPoints, const SalPoint* pPtAry, SalInvert nFlags, const OutputDevice *pOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || (pOutDev && pOutDev->IsRTLEnabled()) )
    {
        std::unique_ptr<SalPoint[]> pPtAry2(new SalPoint[nPoints]);
        bool bCopied = mirror( nPoints, pPtAry, pPtAry2.get(), pOutDev );
        invert( nPoints, bCopied ? pPtAry2.get() : pPtAry, nFlags );
    }
    else
        invert( nPoints, pPtAry, nFlags );
}

bool SalGraphics::DrawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, sal_uLong nSize, const OutputDevice *pOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || (pOutDev && pOutDev->IsRTLEnabled()) )
        mirror( nX, nWidth, pOutDev );
    return drawEPS( nX, nY, nWidth, nHeight,  pPtr, nSize );
}

bool SalGraphics::HitTestNativeScrollbar( ControlPart nPart, const tools::Rectangle& rControlRegion,
                                                const Point& aPos, bool& rIsInside, const OutputDevice *pOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || (pOutDev && pOutDev->IsRTLEnabled()) )
    {
        Point pt( aPos );
        tools::Rectangle rgn( rControlRegion );
        mirror( pt.X(), pOutDev );
        mirror( rgn, pOutDev );
        return hitTestNativeControl( ControlType::Scrollbar, nPart, rgn, pt, rIsInside );
    }
    else
        return hitTestNativeControl( ControlType::Scrollbar, nPart, rControlRegion, aPos, rIsInside );
}

void SalGraphics::mirror( ImplControlValue& rVal, const OutputDevice* pOutDev ) const
{
    switch( rVal.getType() )
    {
        case ControlType::Slider:
        {
            SliderValue* pSlVal = static_cast<SliderValue*>(&rVal);
            mirror(pSlVal->maThumbRect,pOutDev);
        }
        break;
        case ControlType::Scrollbar:
        {
            ScrollbarValue* pScVal = static_cast<ScrollbarValue*>(&rVal);
            mirror(pScVal->maThumbRect,pOutDev);
            mirror(pScVal->maButton1Rect,pOutDev);
            mirror(pScVal->maButton2Rect,pOutDev);
        }
        break;
        case ControlType::Spinbox:
        case ControlType::SpinButtons:
        {
            SpinbuttonValue* pSpVal = static_cast<SpinbuttonValue*>(&rVal);
            mirror(pSpVal->maUpperRect,pOutDev);
            mirror(pSpVal->maLowerRect,pOutDev);
        }
        break;
        case ControlType::Toolbar:
        {
            ToolbarValue* pTVal = static_cast<ToolbarValue*>(&rVal);
            mirror(pTVal->maGripRect,pOutDev);
        }
        break;
        default: break;
    }
}

bool SalGraphics::DrawNativeControl( ControlType nType, ControlPart nPart, const tools::Rectangle& rControlRegion,
                                                ControlState nState, const ImplControlValue& aValue,
                                                const OUString& aCaption, const OutputDevice *pOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || (pOutDev && pOutDev->IsRTLEnabled()) )
    {
        tools::Rectangle rgn( rControlRegion );
        tools::Rectangle aNull;
        if (rgn != aNull)
            mirror(rgn, pOutDev);
        std::unique_ptr< ImplControlValue > mirrorValue( aValue.clone());
        mirror( *mirrorValue, pOutDev );
        bool bRet = drawNativeControl( nType, nPart, rgn, nState, *mirrorValue, aCaption );
        return bRet;
    }
    else
        return drawNativeControl( nType, nPart, rControlRegion, nState, aValue, aCaption );
}

bool SalGraphics::GetNativeControlRegion( ControlType nType, ControlPart nPart, const tools::Rectangle& rControlRegion, ControlState nState,
                                                const ImplControlValue& aValue,
                                                tools::Rectangle &rNativeBoundingRegion, tools::Rectangle &rNativeContentRegion, const OutputDevice *pOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || (pOutDev && pOutDev->IsRTLEnabled()) )
    {
        tools::Rectangle rgn( rControlRegion );
        mirror( rgn, pOutDev );
        std::unique_ptr< ImplControlValue > mirrorValue( aValue.clone());
        mirror( *mirrorValue, pOutDev );
        if( getNativeControlRegion( nType, nPart, rgn, nState, *mirrorValue, OUString(),
                                                rNativeBoundingRegion, rNativeContentRegion ) )
        {
            mirror( rNativeBoundingRegion, pOutDev, true );
            mirror( rNativeContentRegion, pOutDev, true );
            return true;
        }
        return false;
    }
    else
        return getNativeControlRegion( nType, nPart, rControlRegion, nState, aValue, OUString(),
                                                rNativeBoundingRegion, rNativeContentRegion );
}

bool SalGraphics::BlendBitmap( const SalTwoRect& rPosAry,
                               const SalBitmap& rBitmap,
                               const OutputDevice *pOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || (pOutDev && pOutDev->IsRTLEnabled()) )
    {
        SalTwoRect aPosAry2 = rPosAry;
        mirror( aPosAry2.mnDestX, aPosAry2.mnDestWidth, pOutDev );
        return blendBitmap( aPosAry2, rBitmap );
    }
    else
        return blendBitmap( rPosAry, rBitmap );
}

bool SalGraphics::BlendAlphaBitmap( const SalTwoRect& rPosAry,
                                    const SalBitmap& rSrcBitmap,
                                    const SalBitmap& rMaskBitmap,
                                    const SalBitmap& rAlphaBitmap,
                                    const OutputDevice *pOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || (pOutDev && pOutDev->IsRTLEnabled()) )
    {
        SalTwoRect aPosAry2 = rPosAry;
        mirror( aPosAry2.mnDestX, aPosAry2.mnDestWidth, pOutDev );
        return blendAlphaBitmap( aPosAry2, rSrcBitmap, rMaskBitmap, rAlphaBitmap );
    }
    else
        return blendAlphaBitmap( rPosAry, rSrcBitmap, rMaskBitmap, rAlphaBitmap );
}

bool SalGraphics::DrawAlphaBitmap( const SalTwoRect& rPosAry,
                                   const SalBitmap& rSourceBitmap,
                                   const SalBitmap& rAlphaBitmap,
                                   const OutputDevice *pOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || (pOutDev && pOutDev->IsRTLEnabled()) )
    {
        SalTwoRect aPosAry2 = rPosAry;
        mirror( aPosAry2.mnDestX, aPosAry2.mnDestWidth, pOutDev );
        return drawAlphaBitmap( aPosAry2, rSourceBitmap, rAlphaBitmap );
    }
    else
        return drawAlphaBitmap( rPosAry, rSourceBitmap, rAlphaBitmap );
}

bool SalGraphics::DrawTransformedBitmap(
    const basegfx::B2DPoint& rNull,
    const basegfx::B2DPoint& rX,
    const basegfx::B2DPoint& rY,
    const SalBitmap& rSourceBitmap,
    const SalBitmap* pAlphaBitmap,
    const OutputDevice* pOutDev)
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || (pOutDev && pOutDev->IsRTLEnabled()) )
    {
        mirror(rNull, pOutDev);
        mirror(rX, pOutDev);
        mirror(rY, pOutDev);

        return drawTransformedBitmap(rNull, rX, rY, rSourceBitmap, pAlphaBitmap);
    }
    else
    {
        return drawTransformedBitmap(rNull, rX, rY, rSourceBitmap, pAlphaBitmap);
    }
}

bool SalGraphics::DrawAlphaRect( long nX, long nY, long nWidth, long nHeight,
                                 sal_uInt8 nTransparency, const OutputDevice *pOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || (pOutDev && pOutDev->IsRTLEnabled()) )
        mirror( nX, nWidth, pOutDev );

    return drawAlphaRect( nX, nY, nWidth, nHeight, nTransparency );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
