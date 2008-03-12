/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: impgrfll.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 09:47:48 $
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

#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif

#ifndef _SV_SALBTYPE_HXX
#include <vcl/salbtype.hxx>     // FRound
#endif

#ifndef _SV_VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif

#ifndef _SV_GDIMTF_HXX
#include <vcl/gdimtf.hxx>
#endif

#ifndef _SV_METAACT_HXX
#include <vcl/metaact.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

#ifndef _VCL_GRAPHICTOOLS_HXX_
#include <vcl/graphictools.hxx>
#endif

#ifndef _BGFX_POLYGON_B2DPOLYGON_HXX
#include <basegfx/polygon/b2dpolygon.hxx>
#endif

#ifndef _BGFX_MATRIX_B2DHOMMATRIX_HXX
#include <basegfx/matrix/b2dhommatrix.hxx>
#endif

#ifndef _BGFX_RANGE_B2DRANGE_HXX
#include <basegfx/range/b2drange.hxx>
#endif

#ifndef _BGFX_POLYPOLYGON_B2DPOLYGONTOOLS_HXX
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#endif

#include <svx/svdobj.hxx>
#include <svx/xpoly.hxx>
#include "xattr.hxx"
#include "xoutbmp.hxx"
#include <svx/xoutx.hxx>

#include <svx/xenum.hxx>
#include <svx/xgrad.hxx>
#include <svx/xhatch.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xbtmpit.hxx>
#include "svdoimp.hxx"
#include <svx/svdattr.hxx>

///////////////////////////////////////////////////////////////////////////////

#define ITEMVALUE(ItemSet,Id,Cast)  ((const Cast&)(ItemSet).Get(Id)).GetValue()

////////////////////////////////////////////////////////////////////////////////////////////////////

// #100127# Bracket filled output with a comment, if recording a Mtf
ImpGraphicFill::ImpGraphicFill( const SdrObject&        rObj,
                                const XOutputDevice&    rXOut,
                                const SfxItemSet&       rFillItemSet,
                                bool                    bIsShadow       ) :
    mrXOut( rXOut ),
    mbCommentWritten( false )
{
    basegfx::B2DPolyPolygon aGeometry(rObj.TakeXorPoly(sal_True));
    prepare( rXOut, rObj.GetMergedItemSet(), aGeometry, rFillItemSet, bIsShadow );
}

ImpGraphicFill::ImpGraphicFill( const XOutputDevice& rXOut,
                                const SfxItemSet& rSet,
                                basegfx::B2DPolyPolygon& aGeometry,
                                const SfxItemSet& rFillItemSet,
                                bool bIsShadow ) :
    mrXOut( rXOut ),
    mbCommentWritten( false )
{
    prepare( rXOut, rSet, aGeometry, rFillItemSet, bIsShadow );
}

void ImpGraphicFill::prepare( const XOutputDevice&  rXOut,
                         const SfxItemSet& rSet,
                         basegfx::B2DPolyPolygon& aGeometry,
                         const SfxItemSet& rFillItemSet,
                         bool bIsShadow     )
{
    XFillStyle eFillStyle( ITEMVALUE( rFillItemSet, XATTR_FILLSTYLE, XFillStyleItem ) );
    XGradient aGradient(((const XFillGradientItem&)rFillItemSet.Get(XATTR_FILLGRADIENT)).GetGradientValue());
    XHatch aHatch(((const XFillHatchItem&)rFillItemSet.Get(XATTR_FILLHATCH)).GetHatchValue());

    sal_Int32 nDX( ((SdrShadowXDistItem&)(rSet.Get(SDRATTR_SHADOWXDIST))).GetValue() );
    sal_Int32 nDY( ((SdrShadowYDistItem&)(rSet.Get(SDRATTR_SHADOWYDIST))).GetValue() );

    UINT16 nTransp( ((XFillTransparenceItem&)
                        (rSet.Get(XATTR_FILLTRANSPARENCE))).GetValue() );
    XFillFloatTransparenceItem aFillFloatTransparence((XFillFloatTransparenceItem&)rSet.Get(XATTR_FILLFLOATTRANSPARENCE));

    GDIMetaFile* pMtf=NULL;
    if( ( eFillStyle != XFILL_NONE ) && ( !aFillFloatTransparence.IsEnabled() ) && ( !nTransp ) )
        pMtf=mrXOut.GetOutDev()->GetConnectMetaFile();
    if( pMtf != NULL )
    {
        // #104686# Prune non-closed polygons from geometry
        basegfx::B2DPolyPolygon aPolyPoly;
        sal_uInt32 i;
        for( i=0L; i<aGeometry.count(); ++i )
        {
            const basegfx::B2DPolygon aCandidate(aGeometry.getB2DPolygon(i));

            if(aCandidate.isClosed())
            {
                aPolyPoly.append(aCandidate);
            }
        }

        // #103692# Offset original geometry for shadows
        if( bIsShadow && (nDX || nDY) )
        {
            // transformation necessary
            basegfx::B2DHomMatrix aMatrix;

            aMatrix.translate(nDX, nDY);
            aPolyPoly.transform(aMatrix);
        }

        SvtGraphicFill::FillType eType(SvtGraphicFill::fillSolid);
        switch( eFillStyle )
        {
            case XFILL_NONE:
            case XFILL_SOLID:
                eType = SvtGraphicFill::fillSolid;
                break;

            case XFILL_GRADIENT:
                eType = SvtGraphicFill::fillGradient;
                break;

            case XFILL_HATCH:
                eType = SvtGraphicFill::fillHatch;
                break;

            case XFILL_BITMAP:
                eType = SvtGraphicFill::fillTexture;
                break;

            default:
                DBG_ERROR( "ImpGraphicFill::ImpGraphicFill invalid fill type");
                break;
        }

        SvtGraphicFill::Transform aTransform;
        SvtGraphicFill::HatchType eHatch(SvtGraphicFill::hatchSingle);
        // TODO: Set hatch background color. Do that via multi-texturing
        switch( aHatch.GetHatchStyle() )
        {
            case XHATCH_SINGLE:
                eHatch = SvtGraphicFill::hatchSingle;
                break;

            case XHATCH_DOUBLE:
                eHatch = SvtGraphicFill::hatchDouble;
                break;

            case XHATCH_TRIPLE:
                eHatch = SvtGraphicFill::hatchTriple;
                break;

            default:
                DBG_ERROR( "ImpGraphicFill::ImpGraphicFill invalid hatch type");
                break;
        }

        if( SvtGraphicFill::fillHatch == eType )
        {
            // scale
            aTransform.matrix[0] *= aHatch.GetDistance();
            aTransform.matrix[4] *= aHatch.GetDistance();

            // rotate
            aTransform.matrix[0] *= cos( (double) aHatch.GetAngle() );
            aTransform.matrix[1] *= -sin( (double) aHatch.GetAngle() );
            aTransform.matrix[3] *= sin( (double) aHatch.GetAngle() );
            aTransform.matrix[4] *= cos( (double) aHatch.GetAngle() );
        }

        SvtGraphicFill::GradientType eGrad(SvtGraphicFill::gradientLinear);
        switch( aGradient.GetGradientStyle() )
        {
            case XGRAD_LINEAR:
            case XGRAD_AXIAL:
                // TODO: setup transformation
                eGrad = SvtGraphicFill::gradientLinear;
                break;

            case XGRAD_RADIAL:
            case XGRAD_ELLIPTICAL:
                // TODO: setup transformation
                eGrad = SvtGraphicFill::gradientRadial;
                break;

            case XGRAD_SQUARE:
            case XGRAD_RECT:
                // TODO: setup transformation
                eGrad = SvtGraphicFill::gradientRectangular;
                break;

            default:
                DBG_ERROR( "ImpGraphicFill::ImpGraphicFill invalid gradient type");
                break;
        }

        Graphic aFillGraphic;
        bool bTile( ITEMVALUE( rFillItemSet, XATTR_FILLBMP_TILE, SfxBoolItem ) );
        if( SvtGraphicFill::fillTexture == eType )
        {
            OutputDevice* pOut = rXOut.GetOutDev();

            if( pOut )
            {
                Bitmap          aBitmap(((const XFillBitmapItem&)(rSet.Get(XATTR_FILLBITMAP))).GetBitmapValue().GetBitmap());
                const basegfx::B2DRange aTempRange(basegfx::tools::getRange(basegfx::tools::adaptiveSubdivideByAngle(aPolyPoly)));
                const Rectangle aPolyRect(FRound(aTempRange.getMinX()), FRound(aTempRange.getMinY()), FRound(aTempRange.getMaxX()), FRound(aTempRange.getMaxY()));
                MapMode         aMap( pOut->GetMapMode().GetMapUnit() );
                Size            aStartOffset;
                Size            aBmpOutputSize;

                // #104609# Call extracted method from XOutputDevice to
                // determine bitmap size and offset
                RECT_POINT      eRectPoint = (RECT_POINT) ITEMVALUE( rSet, XATTR_FILLBMP_POS, SfxEnumItem );
                USHORT          nOffX = ITEMVALUE( rSet, XATTR_FILLBMP_TILEOFFSETX, SfxUInt16Item );
                USHORT          nOffY = ITEMVALUE( rSet, XATTR_FILLBMP_TILEOFFSETY, SfxUInt16Item );
                Size            aPosOffset( ITEMVALUE( rSet, XATTR_FILLBMP_POSOFFSETX, SfxUInt16Item ),
                                            ITEMVALUE( rSet, XATTR_FILLBMP_POSOFFSETY, SfxUInt16Item ) );
                BOOL            bStretch = ITEMVALUE( rSet, XATTR_FILLBMP_STRETCH, SfxBoolItem );
                BOOL            bLogSize = ITEMVALUE( rSet, XATTR_FILLBMP_SIZELOG, SfxBoolItem );
                Size            aSize( labs( ITEMVALUE( rSet, XATTR_FILLBMP_SIZEX, SfxMetricItem ) ),
                                       labs( ITEMVALUE( rSet, XATTR_FILLBMP_SIZEY, SfxMetricItem ) ) );

                Size            aBmpRenderSize;
                Size            aBmpPerCentSize;
                Size            aBmpSizePixel( aBitmap.GetSizePixel() );

                if( bLogSize )
                    aBmpRenderSize = aSize;
                else
                    aBmpPerCentSize = aSize;

                ImpCalcBmpFillSizes( aStartOffset, aBmpOutputSize, aPolyRect,
                                     aMap, aBitmap, aBmpRenderSize, aBmpPerCentSize,
                                     aPosOffset, bLogSize, bTile, bStretch, eRectPoint );

                // avoid reimplementation of tiling and offset calculation
                // -- simply render our texture into a VDev
                if( bTile && (nOffX || nOffY) )
                {
                    // paint texture to VDev, then extract first 2x2
                    // tiles (to cope with the offset variations)

                    // setup VDev
                    VirtualDevice   aVDev;

                    aVDev.SetOutputSizePixel( pOut->LogicToPixel( aPolyRect, aMap ).GetSize() );
                    aVDev.SetMapMode( aMap );

                    // setup XOutDev
                    XOutputDevice aXOut( &aVDev );
                    aXOut.SetFillAttr( rFillItemSet );

                    // prepare ItemSet to avoid line drawing
                    SfxItemSet aEmptySet( *rFillItemSet.GetPool() );
                    aEmptySet.Put(XLineStyleItem(XLINE_NONE));
                    aXOut.SetLineAttr( aEmptySet );

                    // render into VDev (note: cannot paint smaller
                    // area here, because bitmap size might be
                    // relative to draw rect)
                    Rectangle aOutRect( aPolyRect );
                    aOutRect.SetPos( Point(0,0) );
                    aXOut.DrawRect( aOutRect );

                    // extract first 2x2 tiles
                    Size aTileBmpSize( aBmpOutputSize );
                    aTileBmpSize.Width() *= 2;
                    aTileBmpSize.Height() *= 2;
                    Size aTileBmpSizePixel( pOut->LogicToPixel( aTileBmpSize ) );
                    aFillGraphic = Graphic( aVDev.GetBitmap( Point(), aTileBmpSize ) );
                    aFillGraphic.SetPrefMapMode( MapMode(MAP_PIXEL) );
                    aFillGraphic.SetPrefSize( aTileBmpSizePixel );

                    if( aTileBmpSizePixel.Width() == 0 )
                        aTileBmpSizePixel.Width() = 1;
                    if( aTileBmpSizePixel.Height() == 0 )
                        aTileBmpSizePixel.Height() = 1;

                    // setup transformation (scale to logical coordinate system. no need for translation)
                    aTransform.matrix[0] *= (double)2*aBmpOutputSize.Width() / aTileBmpSizePixel.Width();
                    aTransform.matrix[4] *= (double)2*aBmpOutputSize.Height() / aTileBmpSizePixel.Height();
                }
                else
                {
                    // setup fill graphic
                    aFillGraphic = Graphic( aBitmap );
                    aFillGraphic.SetPrefMapMode( MapMode(MAP_PIXEL) );
                    aFillGraphic.SetPrefSize( aBmpSizePixel );

                    if( aBmpSizePixel.Width() == 0 )
                        aBmpSizePixel.Width() = 1;
                    if( aBmpSizePixel.Height() == 0 )
                        aBmpSizePixel.Height() = 1;

                    // setup transformation from size and offset values (scale to logical coordinate system)
                    aTransform.matrix[0] *= (double)aBmpOutputSize.Width() / aBmpSizePixel.Width();
                    aTransform.matrix[4] *= (double)aBmpOutputSize.Height() / aBmpSizePixel.Height();

                    // translate
                    aTransform.matrix[2] += aStartOffset.Width();
                    aTransform.matrix[5] += aStartOffset.Height();
                }
            }
        }

        const Color aColorSolid = ((const XFillColorItem&) (rFillItemSet.Get(XATTR_FILLCOLOR))).GetColorValue();
        SvtGraphicFill aFill = SvtGraphicFill( PolyPolygon(aPolyPoly),
                              aColorSolid,
                              ITEMVALUE( rFillItemSet, XATTR_FILLTRANSPARENCE, XFillTransparenceItem ) / 100.0,
                              SvtGraphicFill::fillEvenOdd,
                              eType,
                              aTransform,
                              SvtGraphicFill::fillTexture == eType ? bTile : false,
                              eHatch,
                              aHatch.GetColor(),
                              eGrad,
                              aGradient.GetStartColor(),
                              aGradient.GetEndColor(),
                              0 == aGradient.GetSteps() ? ((int)SvtGraphicFill::gradientStepsInfinite) : aGradient.GetSteps(), // 0 means adaptive/infinite step count
                              aFillGraphic );

#ifdef DBG_UTIL
        ::rtl::OString aStr( aFill.toString() );
#endif

        SvMemoryStream  aMemStm;

        aMemStm << aFill;

        pMtf->AddAction( new MetaCommentAction( "XPATHFILL_SEQ_BEGIN", 0,
                                                static_cast<const BYTE*>(aMemStm.GetData()),

                                                aMemStm.Seek( STREAM_SEEK_TO_END ) ) );
        mbCommentWritten = true;
    }
}

// #100127# Bracket filled output with a comment, if recording a Mtf
ImpGraphicFill::~ImpGraphicFill()
{
    GDIMetaFile* pMtf=NULL;
    if( mbCommentWritten )
        pMtf=mrXOut.GetOutDev()->GetConnectMetaFile();
    if( pMtf != NULL )
    {
        pMtf->AddAction( new MetaCommentAction( "XPATHFILL_SEQ_END" ) );
    }
}

///////////////////////////////////////////////////////////////////////////////

// #104609# Extracted from XOutputDevice::ImpCalcBmpFillStartValues
void ImpCalcBmpFillSizes( Size&            rStartOffset,
                          Size&            rBmpOutputSize,
                          const Rectangle& rOutputRect,
                          const MapMode&   rOutputMapMode,
                          const Bitmap&    rFillBitmap,
                          const Size&      rBmpSize,
                          const Size&      rBmpPerCent,
                          const Size&      rBmpOffPerCent,
                          BOOL             bBmpLogSize,
                          BOOL             bBmpTile,
                          BOOL             bBmpStretch,
                          RECT_POINT       eBmpRectPoint )
{
    BOOL    bOriginalSize = FALSE, bScaleSize = FALSE;

    // Falls keine Groessen gegeben sind ( z.B. alte Dokumente )
    // berechnen wir uns die Groesse selber aus der Bitmap
    // ==> altes Verhalten;
    // wenn nur eine Groesse gegeben ist, wird die andere
    // Groesse angepasst berechnet
    if( bBmpLogSize )
    {
        if( !rBmpSize.Width() && !rBmpSize.Height() )
            bOriginalSize = TRUE;
        else if( !rBmpSize.Width() || !rBmpSize.Height() )
            bScaleSize = TRUE;
    }
    else
    {
        if( !rBmpPerCent.Width() && !rBmpPerCent.Height() )
            bOriginalSize = TRUE;
        else if( !rBmpPerCent.Width() || !rBmpPerCent.Height() )
            bScaleSize = TRUE;
    }

    // entweder Originalgroesse oder angepasste Groesse
    if( bOriginalSize || bScaleSize )
    {
        MapMode aBmpPrefMapMode( rFillBitmap.GetPrefMapMode() );
        Size    aBmpPrefSize( rFillBitmap.GetPrefSize() );

        // Falls keine gesetzt ist, nehmen wir Pixel
        if( !aBmpPrefSize.Width() || !aBmpPrefSize.Height() )
        {
            aBmpPrefSize = rFillBitmap.GetSizePixel();
            aBmpPrefMapMode = MAP_PIXEL;
        }

        if( bOriginalSize )
        {
            if( MAP_PIXEL == aBmpPrefMapMode.GetMapUnit() )
                rBmpOutputSize = Application::GetDefaultDevice()->PixelToLogic( aBmpPrefSize, rOutputMapMode );
            else
                rBmpOutputSize = OutputDevice::LogicToLogic( aBmpPrefSize, aBmpPrefMapMode, rOutputMapMode );
        }
        else
        {
            if( bBmpLogSize )
            {
                rBmpOutputSize = rBmpSize;

                if( !rBmpSize.Width() )
                    rBmpOutputSize.Width() = FRound( (double) rBmpSize.Height() * aBmpPrefSize.Width() / aBmpPrefSize.Height() );
                else
                    rBmpOutputSize.Height() = FRound( (double) rBmpSize.Width() * aBmpPrefSize.Height() / aBmpPrefSize.Width() );
            }
            else
            {
                if( !rBmpPerCent.Width() )
                {
                    rBmpOutputSize.Height() = FRound( (double) rOutputRect.GetHeight() * rBmpPerCent.Height() / 100. );
                    rBmpOutputSize.Width() = FRound( (double) rBmpOutputSize.Height() * aBmpPrefSize.Width() / aBmpPrefSize.Height() );
                }
                else
                {
                    rBmpOutputSize.Width() = FRound( (double) rOutputRect.GetWidth() * rBmpPerCent.Width() / 100. );
                    rBmpOutputSize.Height() = FRound( (double) rBmpOutputSize.Width() * aBmpPrefSize.Height() / aBmpPrefSize.Width() );
                }
            }
        }
    }
    // ansonsten koennen wir die Groesse leicht selber berechnen
    else
    {
        if( bBmpLogSize )
            rBmpOutputSize = rBmpSize;
        else
        {
            rBmpOutputSize.Width() = FRound( (double) rOutputRect.GetWidth() * rBmpPerCent.Width() / 100. );
            rBmpOutputSize.Height() = FRound( (double) rOutputRect.GetHeight() * rBmpPerCent.Height() / 100. );
        }
    }

    // nur bei Kachelung die anderen Positionen berechnen
    if( bBmpTile )
    {
        Point aStartPoint;

        // Grundposition der ersten Kachel berechen;
        // Diese Position wird spaeter zur Berechnung der absoluten
        // Startposition links oberhalb des Objektes benutzt
        switch( eBmpRectPoint )
        {
            case( RP_MT ):
            {
                aStartPoint.X() = rOutputRect.Left() + ( ( rOutputRect.GetWidth() - rBmpOutputSize.Width() ) >> 1 );
                aStartPoint.Y() = rOutputRect.Top();
            }
            break;

            case( RP_RT ):
            {
                aStartPoint.X() = rOutputRect.Right() - rBmpOutputSize.Width();
                aStartPoint.Y() = rOutputRect.Top();
            }
            break;

            case( RP_LM ):
            {
                aStartPoint.X() = rOutputRect.Left();
                aStartPoint.Y() = rOutputRect.Top() + ( ( rOutputRect.GetHeight() - rBmpOutputSize.Height() ) >> 1  );
            }
            break;

            case( RP_MM ):
            {
                aStartPoint.X() = rOutputRect.Left() + ( ( rOutputRect.GetWidth() - rBmpOutputSize.Width() ) >> 1 );
                aStartPoint.Y() = rOutputRect.Top() + ( ( rOutputRect.GetHeight() - rBmpOutputSize.Height() ) >> 1 );
            }
            break;

            case( RP_RM ):
            {
                aStartPoint.X() = rOutputRect.Right() - rBmpOutputSize.Width();
                aStartPoint.Y() = rOutputRect.Top() + ( ( rOutputRect.GetHeight() - rBmpOutputSize.Height() ) >> 1 );
            }
            break;

            case( RP_LB ):
            {
                aStartPoint.X() = rOutputRect.Left();
                aStartPoint.Y() = rOutputRect.Bottom() - rBmpOutputSize.Height();
            }
            break;

            case( RP_MB ):
            {
                aStartPoint.X() = rOutputRect.Left() + ( ( rOutputRect.GetWidth() - rBmpOutputSize.Width() ) >> 1 );
                aStartPoint.Y() = rOutputRect.Bottom() - rBmpOutputSize.Height();
            }
            break;

            case( RP_RB ):
            {
                aStartPoint.X() = rOutputRect.Right() - rBmpOutputSize.Width();
                aStartPoint.Y() = rOutputRect.Bottom() - rBmpOutputSize.Height();
            }
            break;

            // default linke obere Ecke
            default:
                aStartPoint = rOutputRect.TopLeft();
            break;
        }

        // X- oder Y-Positionsoffset beruecksichtigen
        if( rBmpOffPerCent.Width() )
            aStartPoint.X() += ( rBmpOutputSize.Width() * rBmpOffPerCent.Width() / 100 );

        if( rBmpOffPerCent.Height() )
            aStartPoint.Y() += ( rBmpOutputSize.Height() * rBmpOffPerCent.Height() / 100 );

        // echten Startpunkt berechnen ( links oben )
        if( rBmpOutputSize.Width() && rBmpOutputSize.Height() )
        {
            const long nDiffX = aStartPoint.X() - rOutputRect.Left();
            const long nDiffY = aStartPoint.Y() - rOutputRect.Top();

            if ( nDiffX )
            {
                long nCount = nDiffX / rBmpOutputSize.Width() + 1;

                if ( rBmpOffPerCent.Height() && ( nCount & 1L ) )
                    nCount++;

                aStartPoint.X() -= ( nCount * rBmpOutputSize.Width() );
            }

            if ( nDiffY )
            {
                long nCount = nDiffY / rBmpOutputSize.Height() + 1;

                if ( rBmpOffPerCent.Width() && ( nCount & 1L ) )
                    nCount++;

                aStartPoint.Y() -= ( nCount * rBmpOutputSize.Height() );
            }
        }

        rStartOffset = Size( aStartPoint.X() - rOutputRect.Left(),
                             aStartPoint.Y() - rOutputRect.Top() );
    }
    else
    {
        if( bBmpStretch )
        {
            rStartOffset = Size(0, 0);
            rBmpOutputSize = rOutputRect.GetSize();
        }
        else
        {
            rStartOffset = Size( ( rOutputRect.GetWidth() - rBmpOutputSize.Width() ) >> 1,
                                 ( rOutputRect.GetHeight() - rBmpOutputSize.Height() ) >> 1 );
        }
    }
}
