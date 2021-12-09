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

#include <sal/config.h>

#include <memory>
#include <config_features.h>
#include <sal/log.hxx>
#include <font/PhysicalFontFace.hxx>
#include <fontsubset.hxx>
#include <salgdi.hxx>
#include <salframe.hxx>
#include <sft.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <FileDefinitionWidgetDraw.hxx>
#include <rtl/math.hxx>

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
    m_aLastMirrorW(0),
    m_nLastMirrorDeviceLTRButBiDiRtlTranslate(0),
    m_bLastMirrorDeviceLTRButBiDiRtlSet(false),
    m_bFastDrawTransformedBitmapChecked(false),
    m_bFastDrawTransformedBitmap(false),
    m_bAntiAlias(false)
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

bool SalGraphics::drawTransformedBitmap(
    const basegfx::B2DPoint& /* rNull */,
    const basegfx::B2DPoint& /* rX */,
    const basegfx::B2DPoint& /* rY */,
    const SalBitmap& /* rSourceBitmap */,
    const SalBitmap* /* pAlphaBitmap */,
    double /* fAlpha */)
{
    // here direct support for transformed bitmaps can be implemented
    return false;
}

tools::Long SalGraphics::mirror2( tools::Long x, const OutputDevice& rOutDev ) const
{
    mirror(x, rOutDev);
    return x;
}

inline tools::Long SalGraphics::GetDeviceWidth(const OutputDevice& rOutDev) const
{
    return rOutDev.IsVirtual() ? rOutDev.GetOutputWidthPixel() : GetGraphicsWidth();
}

void SalGraphics::mirror( tools::Long& x, const OutputDevice& rOutDev ) const
{
    const tools::Long w = GetDeviceWidth(rOutDev);
    if( !w )
        return;

    if (rOutDev.ImplIsAntiparallel() )
    {
        // mirror this window back
        if( m_nLayout & SalLayoutFlags::BiDiRtl )
        {
            tools::Long devX = w - rOutDev.GetOutputWidthPixel() - rOutDev.GetOutOffXPixel();   // re-mirrored mnOutOffX
            x = devX + (x - rOutDev.GetOutOffXPixel());
        }
        else
        {
            tools::Long devX = rOutDev.GetOutOffXPixel();   // re-mirrored mnOutOffX
            x = rOutDev.GetOutputWidthPixel() - (x - devX) + rOutDev.GetOutOffXPixel() - 1;
        }
    }
    else if( m_nLayout & SalLayoutFlags::BiDiRtl )
        x = w-1-x;
}

void SalGraphics::mirror( tools::Long& x, tools::Long nWidth, const OutputDevice& rOutDev, bool bBack ) const
{
    const tools::Long w = GetDeviceWidth(rOutDev);
    if( !w )
        return;

    if (rOutDev.ImplIsAntiparallel() )
    {
        // mirror this window back
        if( m_nLayout & SalLayoutFlags::BiDiRtl )
        {
            tools::Long devX = w - rOutDev.GetOutputWidthPixel() - rOutDev.GetOutOffXPixel();   // re-mirrored mnOutOffX
            if( bBack )
                x = x - devX + rOutDev.GetOutOffXPixel();
            else
                x = devX + (x - rOutDev.GetOutOffXPixel());
        }
        else
        {
            tools::Long devX = rOutDev.GetOutOffXPixel();   // re-mirrored mnOutOffX
            if( bBack )
                x = devX + (rOutDev.GetOutputWidthPixel() + devX) - (x + nWidth);
            else
                x = rOutDev.GetOutputWidthPixel() - (x - devX) + rOutDev.GetOutOffXPixel() - nWidth;
        }
    }
    else if( m_nLayout & SalLayoutFlags::BiDiRtl )
        x = w-nWidth-x;
}

bool SalGraphics::mirror( sal_uInt32 nPoints, const Point *pPtAry, Point *pPtAry2, const OutputDevice& rOutDev ) const
{
    const tools::Long w = GetDeviceWidth(rOutDev);
    if( w )
    {
        sal_uInt32 i, j;

        if (rOutDev.ImplIsAntiparallel())
        {
            // mirror this window back
            if( m_nLayout & SalLayoutFlags::BiDiRtl )
            {
                tools::Long devX = w - rOutDev.GetOutputWidthPixel() - rOutDev.GetOutOffXPixel();   // re-mirrored mnOutOffX
                for( i=0, j=nPoints-1; i<nPoints; i++,j-- )
                {
                    pPtAry2[j].setX( devX + (pPtAry[i].getX() - rOutDev.GetOutOffXPixel()) );
                    pPtAry2[j].setY( pPtAry[i].getY() );
                }
            }
            else
            {
                tools::Long devX = rOutDev.GetOutOffXPixel();   // re-mirrored mnOutOffX
                for( i=0, j=nPoints-1; i<nPoints; i++,j-- )
                {
                    pPtAry2[j].setX( rOutDev.GetOutputWidthPixel() - (pPtAry[i].getX() - devX) + rOutDev.GetOutOffXPixel() - 1 );
                    pPtAry2[j].setY( pPtAry[i].getY() );
                }
            }
        }
        else if( m_nLayout & SalLayoutFlags::BiDiRtl )
        {
            for( i=0, j=nPoints-1; i<nPoints; i++,j-- )
            {
                pPtAry2[j].setX( w-1-pPtAry[i].getX() );
                pPtAry2[j].setY( pPtAry[i].getY() );
            }
        }
        return true;
    }
    else
        return false;
}

void SalGraphics::mirror( vcl::Region& rRgn, const OutputDevice& rOutDev ) const
{
    if( rRgn.HasPolyPolygonOrB2DPolyPolygon() )
    {
        const basegfx::B2DPolyPolygon aPolyPoly(mirror(rRgn.GetAsB2DPolyPolygon(), rOutDev));

        rRgn = vcl::Region(aPolyPoly);
    }
    else
    {
        RectangleVector aRectangles;
        rRgn.GetRegionRectangles(aRectangles);
        rRgn.SetEmpty();

        for (auto & rectangle : aRectangles)
        {
            mirror(rectangle, rOutDev);
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
        //    mirror( aRect, rOutDev, bBack );
        //    aMirroredRegion.Union( aRect );
        //    bRegionRect = rRgn.ImplGetNextRect( aInfo, nX, nY, nWidth, nHeight );
        //}
        //rRgn = aMirroredRegion;
    }
}

void SalGraphics::mirror( tools::Rectangle& rRect, const OutputDevice& rOutDev, bool bBack ) const
{
    tools::Long nWidth = rRect.GetWidth();
    tools::Long x      = rRect.Left();
    tools::Long x_org = x;

    mirror( x, nWidth, rOutDev, bBack );
    rRect.Move( x - x_org, 0 );
}

basegfx::B2DPolyPolygon SalGraphics::mirror( const basegfx::B2DPolyPolygon& i_rPoly, const OutputDevice& i_rOutDev ) const
{
    const basegfx::B2DHomMatrix& rMirror(getMirror(i_rOutDev));

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

const basegfx::B2DHomMatrix& SalGraphics::getMirror( const OutputDevice& i_rOutDev ) const
{
    // get mirroring transformation
    const tools::Long w = GetDeviceWidth(i_rOutDev);
    SAL_WARN_IF( !w, "vcl", "missing graphics width" );

    const bool bMirrorDeviceLTRButBiDiRtlSet = !i_rOutDev.IsRTLEnabled();
    tools::Long nMirrorDeviceLTRButBiDiRtlTranslate(0);
    if (bMirrorDeviceLTRButBiDiRtlSet)
        nMirrorDeviceLTRButBiDiRtlTranslate = w - i_rOutDev.GetOutputWidthPixel() - (2 * i_rOutDev.GetOutOffXPixel());

    // if the device width, or mirror state of the device changed, then m_aLastMirror is invalid
    bool bLastMirrorValid = w == m_aLastMirrorW && bMirrorDeviceLTRButBiDiRtlSet == m_bLastMirrorDeviceLTRButBiDiRtlSet;
    if (bLastMirrorValid && bMirrorDeviceLTRButBiDiRtlSet)
    {
        // if the device is in the unusual mode of a LTR device, but layout flags of SalLayoutFlags::BiDiRtl are
        // in use, then the m_aLastMirror is invalid if the distance it should translate has changed
        bLastMirrorValid = nMirrorDeviceLTRButBiDiRtlTranslate == m_nLastMirrorDeviceLTRButBiDiRtlTranslate;
    }

    if (!bLastMirrorValid)
    {
        const_cast<SalGraphics*>(this)->m_aLastMirrorW = w;
        const_cast<SalGraphics*>(this)->m_bLastMirrorDeviceLTRButBiDiRtlSet = bMirrorDeviceLTRButBiDiRtlSet;
        const_cast<SalGraphics*>(this)->m_nLastMirrorDeviceLTRButBiDiRtlTranslate = nMirrorDeviceLTRButBiDiRtlTranslate;

        if(w)
        {
            if (bMirrorDeviceLTRButBiDiRtlSet)
            {
                /* This path gets exercised in calc's RTL UI (e.g. SAL_RTL_ENABLED=1)
                   with its LTR horizontal scrollbar */

                // Original code was:
                //      // mirror this window back
                //      double devX = w-i_rOutDev.GetOutputWidthPixel()-i_rOutDev.GetOutOffXPixel();   // re-mirrored mnOutOffX
                //      aRet.setX( devX + (i_rPoint.getX() - i_rOutDev.GetOutOffXPixel()) );
                // I do not really understand the comment 'mirror this window back', so cannot guarantee
                // that this works as before, but I have reduced this (by re-placing and re-formatting) to
                // a simple translation:
                const_cast<SalGraphics*>(this)->m_aLastMirror = basegfx::utils::createTranslateB2DHomMatrix(
                    nMirrorDeviceLTRButBiDiRtlTranslate, 0.0);
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

bool SalGraphics::SetClipRegion( const vcl::Region& i_rClip, const OutputDevice& rOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || rOutDev.IsRTLEnabled() )
    {
        vcl::Region aMirror( i_rClip );
        mirror( aMirror, rOutDev );
        return setClipRegion( aMirror );
    }
    return setClipRegion( i_rClip );
}

void SalGraphics::DrawPixel( tools::Long nX, tools::Long nY, const OutputDevice& rOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || rOutDev.IsRTLEnabled() )
        mirror( nX, rOutDev );
    drawPixel( nX, nY );
}

void SalGraphics::DrawPixel( tools::Long nX, tools::Long nY, Color nColor, const OutputDevice& rOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || rOutDev.IsRTLEnabled() )
        mirror( nX, rOutDev );
    drawPixel( nX, nY, nColor );
}

void SalGraphics::DrawLine( tools::Long nX1, tools::Long nY1, tools::Long nX2, tools::Long nY2, const OutputDevice& rOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || rOutDev.IsRTLEnabled() )
    {
        mirror( nX1, rOutDev );
        mirror( nX2, rOutDev );
    }
    drawLine( nX1, nY1, nX2, nY2 );
}

void SalGraphics::DrawRect( tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight, const OutputDevice& rOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || rOutDev.IsRTLEnabled() )
        mirror( nX, nWidth, rOutDev );
    drawRect( nX, nY, nWidth, nHeight );
}

void SalGraphics::DrawPolyLine( sal_uInt32 nPoints, Point const * pPtAry, const OutputDevice& rOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || rOutDev.IsRTLEnabled() )
    {
        std::unique_ptr<Point[]> pPtAry2(new Point[nPoints]);
        bool bCopied = mirror( nPoints, pPtAry, pPtAry2.get(), rOutDev );
        drawPolyLine( nPoints, bCopied ? pPtAry2.get() : pPtAry );
    }
    else
        drawPolyLine( nPoints, pPtAry );
}

void SalGraphics::DrawPolygon( sal_uInt32 nPoints, const Point* pPtAry, const OutputDevice& rOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || rOutDev.IsRTLEnabled() )
    {
        std::unique_ptr<Point[]> pPtAry2(new Point[nPoints]);
        bool bCopied = mirror( nPoints, pPtAry, pPtAry2.get(), rOutDev );
        drawPolygon( nPoints, bCopied ? pPtAry2.get() : pPtAry );
    }
    else
        drawPolygon( nPoints, pPtAry );
}

void SalGraphics::DrawPolyPolygon( sal_uInt32 nPoly, const sal_uInt32* pPoints, const Point** pPtAry, const OutputDevice& rOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || rOutDev.IsRTLEnabled() )
    {
        // TODO: optimize, reduce new/delete calls
        std::unique_ptr<Point*[]> pPtAry2( new Point*[nPoly] );
        sal_uLong i;
        for(i=0; i<nPoly; i++)
        {
            sal_uLong nPoints = pPoints[i];
            pPtAry2[i] = new Point[ nPoints ];
            mirror( nPoints, pPtAry[i], pPtAry2[i], rOutDev );
        }

        drawPolyPolygon( nPoly, pPoints, const_cast<const Point**>(pPtAry2.get()) );

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
    const OutputDevice& i_rOutDev)
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || i_rOutDev.IsRTLEnabled() )
    {
        // mirroring set
        const basegfx::B2DHomMatrix& rMirror(getMirror(i_rOutDev));
        if(!rMirror.isIdentity())
        {
            return drawPolyPolygon(
                rMirror * rObjectToDevice,
                i_rPolyPolygon,
                i_fTransparency);
        }
    }

    return drawPolyPolygon(
        rObjectToDevice,
        i_rPolyPolygon,
        i_fTransparency);
}

bool SalGraphics::DrawPolyLineBezier( sal_uInt32 nPoints, const Point* pPtAry, const PolyFlags* pFlgAry, const OutputDevice& rOutDev )
{
    bool bResult = false;
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || rOutDev.IsRTLEnabled() )
    {
        std::unique_ptr<Point[]> pPtAry2(new Point[nPoints]);
        bool bCopied = mirror( nPoints, pPtAry, pPtAry2.get(), rOutDev );
        bResult = drawPolyLineBezier( nPoints, bCopied ? pPtAry2.get() : pPtAry, pFlgAry );
    }
    else
        bResult = drawPolyLineBezier( nPoints, pPtAry, pFlgAry );
    return bResult;
}

bool SalGraphics::DrawPolygonBezier( sal_uInt32 nPoints, const Point* pPtAry, const PolyFlags* pFlgAry, const OutputDevice& rOutDev )
{
    bool bResult = false;
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || rOutDev.IsRTLEnabled() )
    {
        std::unique_ptr<Point[]> pPtAry2(new Point[nPoints]);
        bool bCopied = mirror( nPoints, pPtAry, pPtAry2.get(), rOutDev );
        bResult = drawPolygonBezier( nPoints, bCopied ? pPtAry2.get() : pPtAry, pFlgAry );
    }
    else
        bResult = drawPolygonBezier( nPoints, pPtAry, pFlgAry );
    return bResult;
}

bool SalGraphics::DrawPolyPolygonBezier( sal_uInt32 i_nPoly, const sal_uInt32* i_pPoints,
                                         const Point* const* i_pPtAry, const PolyFlags* const* i_pFlgAry, const OutputDevice& i_rOutDev )
{
    bool bRet = false;
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || i_rOutDev.IsRTLEnabled() )
    {
        // TODO: optimize, reduce new/delete calls
        std::unique_ptr<Point*[]> pPtAry2( new Point*[i_nPoly] );
        sal_uLong i;
        for(i=0; i<i_nPoly; i++)
        {
            sal_uLong nPoints = i_pPoints[i];
            pPtAry2[i] = new Point[ nPoints ];
            mirror( nPoints, i_pPtAry[i], pPtAry2[i], i_rOutDev );
        }

        bRet = drawPolyPolygonBezier( i_nPoly, i_pPoints, const_cast<const Point* const *>(pPtAry2.get()), i_pFlgAry );

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
    const OutputDevice& i_rOutDev)
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || i_rOutDev.IsRTLEnabled() )
    {
        // mirroring set
        const basegfx::B2DHomMatrix& rMirror(getMirror(i_rOutDev));
        if(!rMirror.isIdentity())
        {
            return drawPolyLine(
                rMirror * rObjectToDevice,
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

bool SalGraphics::DrawGradient(const tools::PolyPolygon& rPolyPoly, const Gradient& rGradient, const OutputDevice& rOutDev)
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || rOutDev.IsRTLEnabled() )
    {
        tools::PolyPolygon aMirrored(mirror(rPolyPoly.getB2DPolyPolygon(), rOutDev));
        return drawGradient(aMirrored, rGradient);
    }

    return drawGradient( rPolyPoly, rGradient );
}

void SalGraphics::CopyArea( tools::Long nDestX, tools::Long nDestY,
                            tools::Long nSrcX, tools::Long nSrcY,
                            tools::Long nSrcWidth, tools::Long nSrcHeight,
                            const OutputDevice& rOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || rOutDev.IsRTLEnabled() )
    {
        mirror( nDestX, nSrcWidth, rOutDev );
        mirror( nSrcX, nSrcWidth, rOutDev );
    }
    copyArea( nDestX, nDestY, nSrcX, nSrcY, nSrcWidth, nSrcHeight, true/*bWindowInvalidate*/ );
}

void SalGraphics::CopyBits(const SalTwoRect& rPosAry, const OutputDevice& rOutDev)
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || rOutDev.IsRTLEnabled() )
    {
        SalTwoRect aPosAry2 = rPosAry;
        mirror( aPosAry2.mnDestX, aPosAry2.mnDestWidth, rOutDev );
        copyBits( aPosAry2, nullptr );
    }
    else
        copyBits( rPosAry, nullptr );
}

void SalGraphics::CopyBits(const SalTwoRect& rPosAry, SalGraphics& rSrcGraphics,
                           const OutputDevice& rOutDev, const OutputDevice& rSrcOutDev)
{
    if( ( (m_nLayout & SalLayoutFlags::BiDiRtl) || rOutDev.IsRTLEnabled() ) ||
        ( (rSrcGraphics.GetLayout() & SalLayoutFlags::BiDiRtl)  || rSrcOutDev.IsRTLEnabled()) )
    {
        SalTwoRect aPosAry2 = rPosAry;
        if( (rSrcGraphics.GetLayout() & SalLayoutFlags::BiDiRtl) || rSrcOutDev.IsRTLEnabled() )
            mirror( aPosAry2.mnSrcX, aPosAry2.mnSrcWidth, rSrcOutDev );
        if( (m_nLayout & SalLayoutFlags::BiDiRtl) || rOutDev.IsRTLEnabled() )
            mirror( aPosAry2.mnDestX, aPosAry2.mnDestWidth, rOutDev );
        copyBits( aPosAry2, &rSrcGraphics );
    }
    else
        copyBits( rPosAry, &rSrcGraphics );
}

void SalGraphics::DrawBitmap( const SalTwoRect& rPosAry,
                              const SalBitmap& rSalBitmap, const OutputDevice& rOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || rOutDev.IsRTLEnabled() )
    {
        SalTwoRect aPosAry2 = rPosAry;
        mirror( aPosAry2.mnDestX, aPosAry2.mnDestWidth, rOutDev );
        drawBitmap( aPosAry2, rSalBitmap );
    }
    else
        drawBitmap( rPosAry, rSalBitmap );
}

void SalGraphics::DrawBitmap( const SalTwoRect& rPosAry,
                              const SalBitmap& rSalBitmap,
                              const SalBitmap& rTransparentBitmap, const OutputDevice& rOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || rOutDev.IsRTLEnabled() )
    {
        SalTwoRect aPosAry2 = rPosAry;
        mirror( aPosAry2.mnDestX, aPosAry2.mnDestWidth, rOutDev );
        drawBitmap( aPosAry2, rSalBitmap, rTransparentBitmap );
    }
    else
        drawBitmap( rPosAry, rSalBitmap, rTransparentBitmap );
}

void SalGraphics::DrawMask( const SalTwoRect& rPosAry,
                            const SalBitmap& rSalBitmap,
                            Color nMaskColor, const OutputDevice& rOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || rOutDev.IsRTLEnabled() )
    {
        SalTwoRect aPosAry2 = rPosAry;
        mirror( aPosAry2.mnDestX, aPosAry2.mnDestWidth, rOutDev );
        drawMask( aPosAry2, rSalBitmap, nMaskColor );
    }
    else
        drawMask( rPosAry, rSalBitmap, nMaskColor );
}

std::shared_ptr<SalBitmap> SalGraphics::GetBitmap( tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight, const OutputDevice& rOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || rOutDev.IsRTLEnabled() )
        mirror( nX, nWidth, rOutDev );
    return getBitmap( nX, nY, nWidth, nHeight );
}

Color SalGraphics::GetPixel( tools::Long nX, tools::Long nY, const OutputDevice& rOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || rOutDev.IsRTLEnabled() )
        mirror( nX, rOutDev );
    return getPixel( nX, nY );
}

void SalGraphics::Invert( tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight, SalInvert nFlags, const OutputDevice& rOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || rOutDev.IsRTLEnabled() )
        mirror( nX, nWidth, rOutDev );
    invert( nX, nY, nWidth, nHeight, nFlags );
}

void SalGraphics::Invert( sal_uInt32 nPoints, const Point* pPtAry, SalInvert nFlags, const OutputDevice& rOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || rOutDev.IsRTLEnabled() )
    {
        std::unique_ptr<Point[]> pPtAry2(new Point[nPoints]);
        bool bCopied = mirror( nPoints, pPtAry, pPtAry2.get(), rOutDev );
        invert( nPoints, bCopied ? pPtAry2.get() : pPtAry, nFlags );
    }
    else
        invert( nPoints, pPtAry, nFlags );
}

bool SalGraphics::DrawEPS( tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight, void* pPtr, sal_uInt32 nSize, const OutputDevice& rOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || rOutDev.IsRTLEnabled() )
        mirror( nX, nWidth, rOutDev );
    return drawEPS( nX, nY, nWidth, nHeight,  pPtr, nSize );
}

bool SalGraphics::HitTestNativeScrollbar(ControlPart nPart, const tools::Rectangle& rControlRegion,
                                         const Point& aPos, bool& rIsInside, const OutputDevice& rOutDev)
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || rOutDev.IsRTLEnabled() )
    {
        Point pt( aPos );
        tools::Rectangle rgn( rControlRegion );
        pt.setX( mirror2( pt.X(), rOutDev ) );
        mirror( rgn, rOutDev );
        return forWidget()->hitTestNativeControl( ControlType::Scrollbar, nPart, rgn, pt, rIsInside );
    }
    else
        return forWidget()->hitTestNativeControl( ControlType::Scrollbar, nPart, rControlRegion, aPos, rIsInside );
}

void SalGraphics::mirror( ImplControlValue& rVal, const OutputDevice& rOutDev ) const
{
    switch( rVal.getType() )
    {
        case ControlType::Slider:
        {
            SliderValue* pSlVal = static_cast<SliderValue*>(&rVal);
            mirror(pSlVal->maThumbRect, rOutDev);
        }
        break;
        case ControlType::Scrollbar:
        {
            ScrollbarValue* pScVal = static_cast<ScrollbarValue*>(&rVal);
            mirror(pScVal->maThumbRect, rOutDev);
            mirror(pScVal->maButton1Rect, rOutDev);
            mirror(pScVal->maButton2Rect, rOutDev);
        }
        break;
        case ControlType::Spinbox:
        case ControlType::SpinButtons:
        {
            SpinbuttonValue* pSpVal = static_cast<SpinbuttonValue*>(&rVal);
            mirror(pSpVal->maUpperRect, rOutDev);
            mirror(pSpVal->maLowerRect, rOutDev);
        }
        break;
        case ControlType::Toolbar:
        {
            ToolbarValue* pTVal = static_cast<ToolbarValue*>(&rVal);
            mirror(pTVal->maGripRect, rOutDev);
        }
        break;
        default: break;
    }
}

bool SalGraphics::DrawNativeControl( ControlType nType, ControlPart nPart, const tools::Rectangle& rControlRegion,
                                                ControlState nState, const ImplControlValue& aValue,
                                                const OUString& aCaption, const OutputDevice& rOutDev,
                                                const Color& rBackgroundColor)
{
    bool bRet = false;
    tools::Rectangle aControlRegion(rControlRegion);
    if (aControlRegion.IsEmpty() || aControlRegion.GetWidth() <= 0 || aControlRegion.GetHeight() <= 0)
        return bRet;

    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || rOutDev.IsRTLEnabled() )
    {
        mirror(aControlRegion, rOutDev);
        std::unique_ptr< ImplControlValue > mirrorValue( aValue.clone());
        mirror( *mirrorValue, rOutDev );
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
                                                tools::Rectangle &rNativeBoundingRegion, tools::Rectangle &rNativeContentRegion, const OutputDevice& rOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || rOutDev.IsRTLEnabled() )
    {
        tools::Rectangle rgn( rControlRegion );
        mirror( rgn, rOutDev );
        std::unique_ptr< ImplControlValue > mirrorValue( aValue.clone());
        mirror( *mirrorValue, rOutDev );
        if (forWidget()->getNativeControlRegion(nType, nPart, rgn, nState, *mirrorValue, OUString(), rNativeBoundingRegion, rNativeContentRegion))
        {
            mirror( rNativeBoundingRegion, rOutDev, true );
            mirror( rNativeContentRegion, rOutDev, true );
            return true;
        }
        return false;
    }
    else
        return forWidget()->getNativeControlRegion(nType, nPart, rControlRegion, nState, aValue, OUString(), rNativeBoundingRegion, rNativeContentRegion);
}

bool SalGraphics::BlendBitmap( const SalTwoRect& rPosAry,
                               const SalBitmap& rBitmap,
                               const OutputDevice& rOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || rOutDev.IsRTLEnabled() )
    {
        SalTwoRect aPosAry2 = rPosAry;
        mirror( aPosAry2.mnDestX, aPosAry2.mnDestWidth, rOutDev );
        return blendBitmap( aPosAry2, rBitmap );
    }
    else
        return blendBitmap( rPosAry, rBitmap );
}

bool SalGraphics::BlendAlphaBitmap( const SalTwoRect& rPosAry,
                                    const SalBitmap& rSrcBitmap,
                                    const SalBitmap& rMaskBitmap,
                                    const SalBitmap& rAlphaBitmap,
                                    const OutputDevice& rOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || rOutDev.IsRTLEnabled() )
    {
        SalTwoRect aPosAry2 = rPosAry;
        mirror( aPosAry2.mnDestX, aPosAry2.mnDestWidth, rOutDev );
        return blendAlphaBitmap( aPosAry2, rSrcBitmap, rMaskBitmap, rAlphaBitmap );
    }
    else
        return blendAlphaBitmap( rPosAry, rSrcBitmap, rMaskBitmap, rAlphaBitmap );
}

bool SalGraphics::DrawAlphaBitmap( const SalTwoRect& rPosAry,
                                   const SalBitmap& rSourceBitmap,
                                   const SalBitmap& rAlphaBitmap,
                                   const OutputDevice& rOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || rOutDev.IsRTLEnabled() )
    {
        SalTwoRect aPosAry2 = rPosAry;
        mirror( aPosAry2.mnDestX, aPosAry2.mnDestWidth, rOutDev );
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
    double fAlpha,
    const OutputDevice& rOutDev)
{
    bool bRetval(false);
    bool bDone(false);

    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || rOutDev.IsRTLEnabled() )
    {
        // mirroring set
        const basegfx::B2DHomMatrix& rMirror(getMirror(rOutDev));
        if (!rMirror.isIdentity())
        {
            basegfx::B2DPolygon aPoints({rNull, rX, rY});
            basegfx::B2DRange aBoundingBox(aPoints.getB2DRange());
            auto aTranslateToMirroredBounds = createTranslateToMirroredBounds(aBoundingBox, rMirror);

            basegfx::B2DPoint aNull = aTranslateToMirroredBounds * rNull;
            basegfx::B2DPoint aX = aTranslateToMirroredBounds * rX;
            basegfx::B2DPoint aY = aTranslateToMirroredBounds * rY;

            bRetval = drawTransformedBitmap(aNull, aX, aY, rSourceBitmap, pAlphaBitmap, fAlpha);
            bDone = true;
        }
    }

    if(!bDone)
    {
        bRetval = drawTransformedBitmap(rNull, rX, rY, rSourceBitmap, pAlphaBitmap, fAlpha);
    }

    if(bRetval && !m_bFastDrawTransformedBitmap)
    {
        // we had a successful call, note it for this SalGraphics
        m_bFastDrawTransformedBitmap = true;
    }

    return bRetval;
}

bool SalGraphics::ImplementsFastDrawTransformedBitmap(bool bTestAllowed) const
{
    // do not check when we already had a successful usage of drawTransformedBitmap (see above)
    // only check if calling OutputDevice is not empty (bTestAllowed)
    // check only once using the direct check method (see below)
    if(!m_bFastDrawTransformedBitmap && bTestAllowed && !m_bFastDrawTransformedBitmapChecked)
    {
        // for check and to not change anything, get the top-left pixel as SalBitmap
        // and try to paint it again. This avoids an extra virtual method hat would need
        // to be implemented at all backends and called all the time (virtual calls).
        // Nothing simpler came to my mind, maybe there is something...
        SalGraphics* that(const_cast<SalGraphics*>(this));
        std::shared_ptr<SalBitmap> aTest(that->getBitmap(0, 0, 1, 1));
        that->m_bFastDrawTransformedBitmapChecked = true;
        that->m_bFastDrawTransformedBitmap = that->drawTransformedBitmap(
            basegfx::B2DPoint::getEmptyPoint(),
            basegfx::B2DPoint(1.0, 0.0),
            basegfx::B2DPoint(0.0, 1.0),
            *aTest,
            nullptr,
            1.0);
    }

    return m_bFastDrawTransformedBitmap;
}

bool SalGraphics::DrawAlphaRect( tools::Long nX, tools::Long nY, tools::Long nWidth, tools::Long nHeight,
                                 sal_uInt8 nTransparency, const OutputDevice& rOutDev )
{
    if( (m_nLayout & SalLayoutFlags::BiDiRtl) || rOutDev.IsRTLEnabled() )
        mirror( nX, nWidth, rOutDev );

    return drawAlphaRect( nX, nY, nWidth, nHeight, nTransparency );
}

OUString SalGraphics::getRenderBackendName() const
{
    if (GetImpl())
        return GetImpl()->getRenderBackendName();
    return OUString();
}

void SalGraphics::GetGlyphWidths(const vcl::AbstractTrueTypeFont& rTTF,
                                 const vcl::font::PhysicalFontFace& rFontFace, const bool bVertical,
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
    // - Why doesn't the PrintFontManager care about the fake glyph? It
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
    if (!pOutFile)
        return false;
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
