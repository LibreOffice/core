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
#include <sal/log.hxx>
#if HAVE_FEATURE_OPENGL
#include <opengl/gdiimpl.hxx>
#include <opengl/zone.hxx>
#include <desktop/exithelper.h>
#ifdef _WIN32
#include <svsys.h>
#endif
#endif
#include <PhysicalFontFace.hxx>
#include <fontsubset.hxx>
#include <salgdi.hxx>
#include <salframe.hxx>
#include <sft.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <FileDefinitionWidgetDraw.hxx>

// The only common SalFrame method

SalFrameGeometry SalFrame::GetGeometry() const
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
    m_aLastMirror(),
    m_aLastMirrorW(0),
    m_bAntiAliasB2DDraw(false)
{
    // read global RTL settings
    if( AllSettings::GetLayoutRTL() )
        m_nLayout = SalLayoutFlags::BiDiRtl;
}

bool SalGraphics::initWidgetDrawBackends(bool bForce)
{
    bool bFileDefinitionsWidgetDraw = !!getenv("VCL_DRAW_WIDGETS_FROM_FILE");

    if (bFileDefinitionsWidgetDraw || bForce)
    {
        m_pWidgetDraw.reset(new vcl::FileDefinitionWidgetDraw(*this));
        auto pFileDefinitionWidgetDraw = static_cast<vcl::FileDefinitionWidgetDraw*>(m_pWidgetDraw.get());
        if (!pFileDefinitionWidgetDraw->isActive())
        {
            m_pWidgetDraw.reset();
            return false;
        }
        return true;
    }
    return false;
}

SalGraphics::~SalGraphics() COVERITY_NOEXCEPT_FALSE
{
    // can't call ReleaseFonts here, as the destructor just calls this classes SetFont (pure virtual)!
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

long SalGraphics::mirror2( long x, const OutputDevice *pOutDev ) const
{
    mirror(x, pOutDev);
    return x;
}

inline long SalGraphics::GetDeviceWidth(const OutputDevice* pOutDev) const
{
    return (pOutDev && pOutDev->IsVirtual())
        ? pOutDev->GetOutputWidthPixel() : GetGraphicsWidth();
}

void SalGraphics::mirror( long& x, const OutputDevice *pOutDev ) const
{
    const long w = GetDeviceWidth(pOutDev);
    if( !w )
        return;

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

void SalGraphics::mirror( long& x, long nWidth, const OutputDevice *pOutDev, bool bBack ) const
{
    const long w = GetDeviceWidth(pOutDev);
    if( !w )
        return;

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

bool SalGraphics::mirror( sal_uInt32 nPoints, const SalPoint *pPtAry, SalPoint *pPtAry2, const OutputDevice *pOutDev ) const
{
    const long w = GetDeviceWidth(pOutDev);
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

        for (auto & rectangle : aRectangles)
        {
            mirror(rectangle, pOutDev);
            rRgn.Union(rectangle);
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

basegfx::B2DPolyPolygon SalGraphics::mirror( const basegfx::B2DPolyPolygon& i_rPoly, const OutputDevice* i_pOutDev ) const
{
    const basegfx::B2DHomMatrix& rMirror(getMirror(i_pOutDev));

    if(rMirror.isIdentity())
    {
        return i_rPoly;
    }
    else
    {
        basegfx::B2DPolyPolygon aRet(i_rPoly);
        aRet.transform(rMirror);
        aRet.flip();
        return aRet;
    }
}

const basegfx::B2DHomMatrix& SalGraphics::getMirror( const OutputDevice* i_pOutDev ) const
{
    // get mirroring transformation
    const long w = GetDeviceWidth(i_pOutDev);
    SAL_WARN_IF( !w, "vcl", "missing graphics width" );

    if(w != m_aLastMirrorW)
    {
        const_cast<SalGraphics*>(this)->m_aLastMirrorW = w;

        if(w)
        {
            if(nullptr != i_pOutDev && !i_pOutDev->IsRTLEnabled())
            {
                // Original code was (removed here already pOutDevRef->i_pOutDev):
                //      // mirror this window back
                //      double devX = w-i_pOutDev->GetOutputWidthPixel()-i_pOutDev->GetOutOffXPixel();   // re-mirrored mnOutOffX
                //      aRet.setX( devX + (i_rPoint.getX() - i_pOutDev->GetOutOffXPixel()) );
                // I do not really understand the comment 'mirror this window back', so cannot guarantee
                // that this works as before, but I have reduced this (by re-placing and re-formatting) to
                // a simple translation:
                const_cast<SalGraphics*>(this)->m_aLastMirror = basegfx::utils::createTranslateB2DHomMatrix(
                    w - i_pOutDev->GetOutputWidthPixel() - (2 * i_pOutDev->GetOutOffXPixel()),
                    0.0);
            }
            else
            {
                // Original code was:
                //      aRet.setX( w-1-i_rPoint.getX() );
                // -mirror X -> scale(-1.0, 1.0)
                // -translate X -> translate(w-1, 0)
                // Checked this one, works as expected.
                const_cast<SalGraphics*>(this)->m_aLastMirror = basegfx::utils::createScaleTranslateB2DHomMatrix(
                    -1.0,
                    1.0,
                    w-1,
                    0.0);
            }
        }
        else
        {
            const_cast<SalGraphics*>(this)->m_aLastMirror.identity();
        }
    }

    return m_aLastMirror;
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

void SalGraphics::DrawPixel( long nX, long nY, Color nColor, const OutputDevice *pOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || (pOutDev && pOutDev->IsRTLEnabled()) )
        mirror( nX, pOutDev );
    drawPixel( nX, nY, nColor );
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

namespace
{
    basegfx::B2DHomMatrix createTranslateToMirroredBounds(const basegfx::B2DRange &rBoundingBox, const basegfx::B2DHomMatrix& rMirror)
    {
        basegfx::B2DRange aRTLBoundingBox(rBoundingBox);
        aRTLBoundingBox *= rMirror;
        return basegfx::utils::createTranslateB2DHomMatrix(aRTLBoundingBox.getMinX() - rBoundingBox.getMinX(), 0);
    }
}

bool SalGraphics::DrawPolyPolygon(
    const basegfx::B2DHomMatrix& rObjectToDevice,
    const basegfx::B2DPolyPolygon& i_rPolyPolygon,
    double i_fTransparency,
    const OutputDevice* i_pOutDev)
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || (i_pOutDev && i_pOutDev->IsRTLEnabled()) )
    {
        // mirroring set
        const basegfx::B2DHomMatrix& rMirror(getMirror(i_pOutDev));
        if(!rMirror.isIdentity())
        {
            basegfx::B2DRange aBoundingBox(i_rPolyPolygon.getB2DRange());
            aBoundingBox *= rObjectToDevice;
            auto aTranslateToMirroredBounds = createTranslateToMirroredBounds(aBoundingBox, rMirror);

            return drawPolyPolygon(
                aTranslateToMirroredBounds * rObjectToDevice,
                i_rPolyPolygon,
                i_fTransparency);
        }
    }

    return drawPolyPolygon(
        rObjectToDevice,
        i_rPolyPolygon,
        i_fTransparency);
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

bool SalGraphics::DrawPolyLine(
    const basegfx::B2DHomMatrix& rObjectToDevice,
    const basegfx::B2DPolygon& i_rPolygon,
    double i_fTransparency,
    double i_rLineWidth,
    const std::vector< double >* i_pStroke, // MM01
    basegfx::B2DLineJoin i_eLineJoin,
    css::drawing::LineCap i_eLineCap,
    double i_fMiterMinimumAngle,
    bool bPixelSnapHairline,
    const OutputDevice* i_pOutDev)
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || (i_pOutDev && i_pOutDev->IsRTLEnabled()) )
    {
        // mirroring set
        const basegfx::B2DHomMatrix& rMirror(getMirror(i_pOutDev));
        if(!rMirror.isIdentity())
        {
            basegfx::B2DRange aBoundingBox(i_rPolygon.getB2DRange());
            aBoundingBox *= rObjectToDevice;
            auto aTranslateToMirroredBounds = createTranslateToMirroredBounds(aBoundingBox, rMirror);

            return drawPolyLine(
                aTranslateToMirroredBounds * rObjectToDevice,
                i_rPolygon,
                i_fTransparency,
                i_rLineWidth,
                i_pStroke, // MM01
                i_eLineJoin,
                i_eLineCap,
                i_fMiterMinimumAngle,
                bPixelSnapHairline);
        }
    }

    // no mirroring set (or identity), use standard call
    return drawPolyLine(
        rObjectToDevice,
        i_rPolygon,
        i_fTransparency,
        i_rLineWidth,
        i_pStroke, // MM01
        i_eLineJoin,
        i_eLineCap,
        i_fMiterMinimumAngle,
        bPixelSnapHairline);
}

bool SalGraphics::DrawGradient( const tools::PolyPolygon& rPolyPoly, const Gradient& rGradient )
{
    return drawGradient( rPolyPoly, rGradient );
}

bool SalGraphics::DrawGradient(basegfx::B2DPolyPolygon const & rPolyPolygon, SalGradient const & rSalGradient)
{
    return implDrawGradient(rPolyPolygon, rSalGradient);
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
                            Color nMaskColor, const OutputDevice *pOutDev )
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

std::shared_ptr<SalBitmap> SalGraphics::GetBitmap( long nX, long nY, long nWidth, long nHeight, const OutputDevice *pOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || (pOutDev && pOutDev->IsRTLEnabled()) )
        mirror( nX, nWidth, pOutDev );
    return getBitmap( nX, nY, nWidth, nHeight );
}

Color SalGraphics::GetPixel( long nX, long nY, const OutputDevice *pOutDev )
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

bool SalGraphics::DrawEPS( long nX, long nY, long nWidth, long nHeight, void* pPtr, sal_uInt32 nSize, const OutputDevice *pOutDev )
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
        pt.setX( mirror2( pt.X(), pOutDev ) );
        mirror( rgn, pOutDev );
        return forWidget()->hitTestNativeControl( ControlType::Scrollbar, nPart, rgn, pt, rIsInside );
    }
    else
        return forWidget()->hitTestNativeControl( ControlType::Scrollbar, nPart, rControlRegion, aPos, rIsInside );
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
                                                const OUString& aCaption, const OutputDevice *pOutDev,
                                                const Color& rBackgroundColor)
{
    bool bRet = false;
    tools::Rectangle aControlRegion(rControlRegion);
    if (aControlRegion.IsEmpty() || aControlRegion.GetWidth() <= 0 || aControlRegion.GetHeight() <= 0)
        return bRet;

    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || (pOutDev && pOutDev->IsRTLEnabled()) )
    {
        mirror(aControlRegion, pOutDev);
        std::unique_ptr< ImplControlValue > mirrorValue( aValue.clone());
        mirror( *mirrorValue, pOutDev );
        bRet = forWidget()->drawNativeControl(nType, nPart, aControlRegion, nState, *mirrorValue, aCaption, rBackgroundColor);
    }
    else
        bRet = forWidget()->drawNativeControl(nType, nPart, aControlRegion, nState, aValue, aCaption, rBackgroundColor);

    if (bRet && m_pWidgetDraw)
        handleDamage(aControlRegion);
    return bRet;
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
        if (forWidget()->getNativeControlRegion(nType, nPart, rgn, nState, *mirrorValue, OUString(), rNativeBoundingRegion, rNativeContentRegion))
        {
            mirror( rNativeBoundingRegion, pOutDev, true );
            mirror( rNativeContentRegion, pOutDev, true );
            return true;
        }
        return false;
    }
    else
        return forWidget()->getNativeControlRegion(nType, nPart, rControlRegion, nState, aValue, OUString(), rNativeBoundingRegion, rNativeContentRegion);
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
        // mirroring set
        const basegfx::B2DHomMatrix& rMirror(getMirror(pOutDev));
        if (!rMirror.isIdentity())
        {
            basegfx::B2DPolygon aPoints({rNull, rX, rY});
            basegfx::B2DRange aBoundingBox(aPoints.getB2DRange());
            auto aTranslateToMirroredBounds = createTranslateToMirroredBounds(aBoundingBox, rMirror);

            basegfx::B2DPoint aNull = aTranslateToMirroredBounds * rNull;
            basegfx::B2DPoint aX = aTranslateToMirroredBounds * rX;
            basegfx::B2DPoint aY = aTranslateToMirroredBounds * rY;

            return drawTransformedBitmap(aNull, aX, aY, rSourceBitmap, pAlphaBitmap);
        }
    }

    return drawTransformedBitmap(rNull, rX, rY, rSourceBitmap, pAlphaBitmap);
}

bool SalGraphics::DrawAlphaRect( long nX, long nY, long nWidth, long nHeight,
                                 sal_uInt8 nTransparency, const OutputDevice *pOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || (pOutDev && pOutDev->IsRTLEnabled()) )
        mirror( nX, nWidth, pOutDev );

    return drawAlphaRect( nX, nY, nWidth, nHeight, nTransparency );
}

OUString SalGraphics::getRenderBackendName() const
{
    if (GetImpl())
        return GetImpl()->getRenderBackendName();
    return OUString();
}

void SalGraphics::GetGlyphWidths(const vcl::AbstractTrueTypeFont& rTTF,
                                 const PhysicalFontFace& rFontFace, const bool bVertical,
                                 std::vector<sal_Int32>& rWidths, Ucs2UIntMap& rUnicodeEnc)
{
    rWidths.clear();
    rUnicodeEnc.clear();

    const int nGlyphCount = rTTF.glyphCount();
    if (nGlyphCount <= 0)
        return;

    FontCharMapRef xFCMap = rFontFace.GetFontCharMap();
    if (!xFCMap.is() || !xFCMap->GetCharCount())
    {
        SAL_WARN("vcl.fonts", "no charmap");
        return;
    }

    rWidths.resize(nGlyphCount);
    std::vector<sal_uInt16> aGlyphIds(nGlyphCount);
    for (int i = 0; i < nGlyphCount; i++)
        aGlyphIds[i] = static_cast<sal_uInt16>(i);

    std::unique_ptr<sal_uInt16[]> pGlyphMetrics
        = GetTTSimpleGlyphMetrics(&rTTF, aGlyphIds.data(), nGlyphCount, bVertical);
    if (pGlyphMetrics)
    {
        for (int i = 0; i < nGlyphCount; ++i)
            rWidths[i] = pGlyphMetrics[i];
        pGlyphMetrics.reset();
    }

    int nCharCount = xFCMap->GetCharCount();
    sal_uInt32 nChar = xFCMap->GetFirstChar();
    for (; --nCharCount >= 0; nChar = xFCMap->GetNextChar(nChar))
    {
        if (nChar > 0xFFFF)
            continue;

        sal_Ucs nUcsChar = static_cast<sal_Ucs>(nChar);
        sal_uInt32 nGlyph = xFCMap->GetGlyphIndex(nUcsChar);
        if (nGlyph > 0)
            rUnicodeEnc[nUcsChar] = nGlyph;
    }
}

bool SalGraphics::CreateTTFfontSubset(vcl::AbstractTrueTypeFont& rTTF, const OString& rSysPath,
                                   const bool bVertical, const sal_GlyphId* pGlyphIds,
                                   const sal_uInt8* pEncoding, sal_Int32* pGlyphWidths,
                                   const int nOrigGlyphCount)
{
    // Multiple questions:
    // - Why is there a glyph limit?
    //   MacOS used to handle 257 glyphs...
    //   Also the much more complex PrintFontManager variant has this limit.
    //   Also the very first implementation has the limit in
    //   commit 8789ed701e98031f2a1657ea0dfd6f7a0b050992
    // - Why doesn't the PrintFontManager care about the fake glpyh? It
    //   is used on all unx platforms to create the subset font.
    // - Should the SAL_WARN actually be asserts, like on MacOS?
    if (nOrigGlyphCount > 256)
    {
        SAL_WARN("vcl.fonts", "too many glyphs for subsetting");
        return false;
    }

    int nGlyphCount = nOrigGlyphCount;
    sal_uInt16 aShortIDs[256];
    sal_uInt8 aTempEncs[256];

    // handle the undefined / first font glyph
    int nNotDef = -1, i;
    for (i = 0; i < nGlyphCount; ++i)
    {
        aTempEncs[i] = pEncoding[i];
        aShortIDs[i] = static_cast<sal_uInt16>(pGlyphIds[i]);
        if (!aShortIDs[i])
            if (nNotDef < 0)
                nNotDef = i;
    }

    // nNotDef glyph must be in pos 0 => swap glyphids
    if (nNotDef != 0)
    {
        if (nNotDef < 0)
        {
            if (nGlyphCount == 256)
            {
                SAL_WARN("vcl.fonts", "too many glyphs for subsetting");
                return false;
            }
            nNotDef = nGlyphCount++;
        }

        aShortIDs[nNotDef] = aShortIDs[0];
        aTempEncs[nNotDef] = aTempEncs[0];
        aShortIDs[0] = 0;
        aTempEncs[0] = 0;
    }

    std::unique_ptr<sal_uInt16[]> pMetrics
        = GetTTSimpleGlyphMetrics(&rTTF, aShortIDs, nGlyphCount, bVertical);
    if (!pMetrics)
        return false;

    sal_uInt16 nNotDefAdv = pMetrics[0];
    pMetrics[0] = pMetrics[nNotDef];
    pMetrics[nNotDef] = nNotDefAdv;
    for (i = 0; i < nOrigGlyphCount; ++i)
        pGlyphWidths[i] = pMetrics[i];
    pMetrics.reset();

    // write subset into destination file
    return (CreateTTFromTTGlyphs(&rTTF, rSysPath.getStr(), aShortIDs, aTempEncs, nGlyphCount)
            == vcl::SFErrCodes::Ok);
}

bool SalGraphics::CreateCFFfontSubset(const unsigned char* pFontBytes, int nByteLength,
                                      const OString& rSysPath, const sal_GlyphId* pGlyphIds,
                                      const sal_uInt8* pEncoding, sal_Int32* pGlyphWidths,
                                      int nGlyphCount, FontSubsetInfo& rInfo)
{
    FILE* pOutFile = fopen(rSysPath.getStr(), "wb");
    rInfo.LoadFont(FontType::CFF_FONT, pFontBytes, nByteLength);
    bool bRet = rInfo.CreateFontSubset(FontType::TYPE1_PFB, pOutFile, nullptr, pGlyphIds, pEncoding,
                                       nGlyphCount, pGlyphWidths);
    fclose(pOutFile);
    return bRet;
}

void SalGraphics::FillFontSubsetInfo(const vcl::TTGlobalFontInfo& rTTInfo, const OUString& pPSName,
                                     FontSubsetInfo& rInfo)
{
    rInfo.m_aPSName = pPSName;
    rInfo.m_nFontType = FontType::SFNT_TTF;
    rInfo.m_aFontBBox
        = tools::Rectangle(Point(rTTInfo.xMin, rTTInfo.yMin), Point(rTTInfo.xMax, rTTInfo.yMax));
    rInfo.m_nCapHeight = rTTInfo.yMax; // Well ...
    rInfo.m_nAscent = rTTInfo.winAscent;
    rInfo.m_nDescent = rTTInfo.winDescent;

    // mac fonts usually do not have an OS2-table
    // => get valid ascent/descent values from other tables
    if (!rInfo.m_nAscent)
        rInfo.m_nAscent = +rTTInfo.typoAscender;
    if (!rInfo.m_nAscent)
        rInfo.m_nAscent = +rTTInfo.ascender;
    if (!rInfo.m_nDescent)
        rInfo.m_nDescent = +rTTInfo.typoDescender;
    if (!rInfo.m_nDescent)
        rInfo.m_nDescent = -rTTInfo.descender;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
