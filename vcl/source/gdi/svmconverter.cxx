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

#include <algorithm>
#include <string.h>
#include <osl/thread.h>
#include <tools/debug.hxx>
#include <tools/fract.hxx>
#include <tools/stream.hxx>
#include <tools/helpers.hxx>
#include <vcl/dibtools.hxx>
#include <vcl/virdev.hxx>
#include <vcl/graph.hxx>
#include <vcl/lineinfo.hxx>
#include <rtl/strbuf.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>

#include <svmconverter.hxx>

#include <memory>

// Inlines
static void ImplReadRect( SvStream& rIStm, tools::Rectangle& rRect )
{
    Point aTL;
    Point aBR;

    ReadPair( rIStm, aTL );
    ReadPair( rIStm, aBR );

    rRect = tools::Rectangle( aTL, aBR );
}

static bool ImplReadPoly(SvStream& rIStm, tools::Polygon& rPoly)
{
    sal_Int32 nSize32(0);
    rIStm.ReadInt32(nSize32);
    sal_uInt16 nSize = nSize32;

    const size_t nMaxPossiblePoints = rIStm.remainingSize() / 2 * sizeof(sal_Int32);
    if (nSize > nMaxPossiblePoints)
    {
        SAL_WARN("vcl.gdi", "svm record claims to have: " << nSize << " points, but only " << nMaxPossiblePoints << " possible");
        return false;
    }

    rPoly = tools::Polygon(nSize);

    for (sal_uInt16 i = 0; i < nSize && rIStm.good(); ++i)
        ReadPair(rIStm, rPoly[i]);

    return rIStm.good();
}

static bool ImplReadPolyPoly(SvStream& rIStm, tools::PolyPolygon& rPolyPoly)
{
    bool bSuccess = true;

    tools::Polygon aPoly;
    sal_Int32 nPolyCount32(0);
    rIStm.ReadInt32(nPolyCount32);
    sal_uInt16 nPolyCount = static_cast<sal_uInt16>(nPolyCount32);

    for (sal_uInt16 i = 0; i < nPolyCount && rIStm.good(); ++i)
    {
        if (!ImplReadPoly(rIStm, aPoly))
        {
            bSuccess = false;
            break;
        }
        rPolyPoly.Insert(aPoly);
    }

    return bSuccess && rIStm.good();
}

static void ImplReadColor( SvStream& rIStm, Color& rColor )
{
    sal_Int16 nVal(0);

    rIStm.ReadInt16( nVal ); rColor.SetRed( sal::static_int_cast<sal_uInt8>(static_cast<sal_uInt16>(nVal) >> 8) );
    rIStm.ReadInt16( nVal ); rColor.SetGreen( sal::static_int_cast<sal_uInt8>(static_cast<sal_uInt16>(nVal) >> 8) );
    rIStm.ReadInt16( nVal ); rColor.SetBlue( sal::static_int_cast<sal_uInt8>(static_cast<sal_uInt16>(nVal) >> 8) );
}

static bool ImplReadMapMode(SvStream& rIStm, MapMode& rMapMode)
{
    sal_Int16 nUnit(0);
    rIStm.ReadInt16(nUnit);

    Point aOrg;
    ReadPair(rIStm, aOrg);

    sal_Int32 nXNum(0), nXDenom(0), nYNum(0), nYDenom(0);
    rIStm.ReadInt32(nXNum).ReadInt32(nXDenom).ReadInt32(nYNum).ReadInt32(nYDenom);

    if (!rIStm.good() || nXDenom <= 0 || nYDenom <= 0 || nXNum <= 0 || nYNum <= 0)
    {
        SAL_WARN("vcl.gdi", "Parsing error: invalid mapmode fraction");
        return false;
    }

    if (nUnit < sal_Int16(MapUnit::Map100thMM) || nUnit > sal_Int16(MapUnit::LAST))
    {
        SAL_WARN("vcl.gdi", "Parsing error: invalid mapmode");
        return false;
    }

    rMapMode = MapMode(static_cast<MapUnit>(nUnit), aOrg, Fraction(nXNum, nXDenom), Fraction(nYNum, nYDenom));

    return true;
}

static void ImplReadUnicodeComment( sal_uInt32 nStrmPos, SvStream& rIStm, OUString& rString )
{
    sal_uInt32 nOld = rIStm.Tell();
    if ( nStrmPos )
    {
        sal_uInt16  nType;
        sal_uInt32  nActionSize;
        std::size_t nStringLen;

        rIStm.Seek( nStrmPos );
        rIStm  .ReadUInt16( nType )
               .ReadUInt32( nActionSize );

        nStringLen = (nActionSize - 4) >> 1;

        if ( nStringLen && ( nType == GDI_UNICODE_COMMENT ) )
            rString = read_uInt16s_ToOUString(rIStm, nStringLen);
    }
    rIStm.Seek( nOld );
}

static void ImplSkipActions(SvStream& rIStm, sal_uLong nSkipCount)
{
    sal_Int32 nActionSize;
    sal_Int16 nType;
    for (sal_uLong i = 0; i < nSkipCount; ++i)
    {
        rIStm.ReadInt16(nType).ReadInt32(nActionSize);
        if (!rIStm.good() || nActionSize < 4)
            break;
        rIStm.SeekRel(nActionSize - 4);
    }
}

static void ImplReadExtendedPolyPolygonAction(SvStream& rIStm, tools::PolyPolygon& rPolyPoly)
{
    rPolyPoly.Clear();
    sal_uInt16 nPolygonCount(0);
    rIStm.ReadUInt16( nPolygonCount );

    if (!nPolygonCount)
        return;

    const size_t nMinRecordSize = sizeof(sal_uInt16);
    const size_t nMaxRecords = rIStm.remainingSize() / nMinRecordSize;
    if (nPolygonCount > nMaxRecords)
    {
        SAL_WARN("vcl.gdi", "Parsing error: " << nMaxRecords <<
                 " max possible entries, but " << nPolygonCount << " claimed, truncating");
        nPolygonCount = nMaxRecords;
    }

    for(sal_uInt16 a(0); a < nPolygonCount; a++)
    {
        sal_uInt16 nPointCount(0);
        rIStm.ReadUInt16(nPointCount);

        const size_t nMinPolygonSize = sizeof(sal_Int32) * 2;
        const size_t nMaxPolygons = rIStm.remainingSize() / nMinPolygonSize;
        if (nPointCount > nMaxPolygons)
        {
            SAL_WARN("vcl.gdi", "Parsing error: " << nMaxPolygons <<
                     " max possible entries, but " << nPointCount << " claimed, truncating");
            nPointCount = nMaxPolygons;
        }

        tools::Polygon aCandidate(nPointCount);

        if (nPointCount)
        {
            for(sal_uInt16 b(0); b < nPointCount; b++)
            {
                ReadPair( rIStm , aCandidate[b] );
            }

            sal_uInt8 bHasFlags(int(false));
            rIStm.ReadUChar( bHasFlags );

            if(bHasFlags)
            {
                sal_uInt8 aPolyFlags(0);

                for(sal_uInt16 c(0); c < nPointCount; c++)
                {
                    rIStm.ReadUChar( aPolyFlags );
                    aCandidate.SetFlags(c, static_cast<PolyFlags>(aPolyFlags));
                }
            }
        }

        rPolyPoly.Insert(aCandidate);
    }
}

SVMConverter::SVMConverter( SvStream& rStm, GDIMetaFile& rMtf )
{
    if( !rStm.GetError() )
    {
        ImplConvertFromSVM1( rStm, rMtf );
    }
}

namespace
{
    sal_Int32 SkipActions(sal_Int32 i, sal_Int32 nFollowingActionCount, sal_Int32 nActions)
    {
        sal_Int32 remainingActions = nActions - i;
        if (nFollowingActionCount < 0)
            nFollowingActionCount = remainingActions;
        return std::min(remainingActions, nFollowingActionCount);
    }
}

#define LF_FACESIZE 32

void static lcl_error( SvStream& rIStm, const SvStreamEndian& nOldFormat, const sal_uLong& nPos)
{
    rIStm.SetError(SVSTREAM_FILEFORMAT_ERROR);
    rIStm.SetEndian(nOldFormat);
    rIStm.Seek(nPos);
    return;
}
void SVMConverter::ImplConvertFromSVM1( SvStream& rIStm, GDIMetaFile& rMtf )
{
    const sal_uLong         nPos = rIStm.Tell();
    const SvStreamEndian    nOldFormat = rIStm.GetEndian();

    rIStm.SetEndian( SvStreamEndian::LITTLE );

    char    aCode[ 5 ];
    Size    aPrefSz;

    // read header
    rIStm.ReadBytes(aCode, sizeof(aCode));  // Identifier
    sal_Int16 nSize(0);
    rIStm.ReadInt16( nSize );                                 // Size
    sal_Int16 nVersion(0);
    rIStm.ReadInt16( nVersion );                              // Version
    sal_Int32 nTmp32(0);
    rIStm.ReadInt32( nTmp32 );
    if (nTmp32 < 0)
    {
        SAL_WARN("vcl.gdi", "svm: value for width should be positive");
        lcl_error(rIStm, nOldFormat, nPos);
        return;
    }
    aPrefSz.setWidth( nTmp32 );                       // PrefSize.Width()
    rIStm.ReadInt32( nTmp32 );
    if (nTmp32 < 0)
    {
        SAL_WARN("vcl.gdi", "svm: value for height should be positive");
        lcl_error(rIStm, nOldFormat, nPos);
        return;
    }
    aPrefSz.setHeight( nTmp32 );                      // PrefSize.Height()

    // check header-magic and version
    if( rIStm.GetError()
        || ( memcmp( aCode, "SVGDI", sizeof( aCode ) ) != 0 )
        || ( nVersion != 200 ) )
    {
        SAL_WARN("vcl.gdi", "svm: wrong check for header-magic and version");
        lcl_error(rIStm, nOldFormat, nPos);
        return;
    }

    LineInfo            aLineInfo( LineStyle::NONE, 0 );
    std::stack<std::unique_ptr<LineInfo>> aLIStack;
    ScopedVclPtrInstance< VirtualDevice > aFontVDev;
    rtl_TextEncoding    eActualCharSet = osl_getThreadTextEncoding();
    bool                bFatLine = false;

    tools::Polygon     aActionPoly;
    tools::Rectangle   aRect;
    Point       aPt, aPt1;
    Size        aSz;
    Color       aActionColor;

    sal_uInt32  nUnicodeCommentStreamPos = 0;
    sal_Int32       nUnicodeCommentActionNumber = 0;

    rMtf.SetPrefSize(aPrefSz);

    MapMode aMapMode;
    if (ImplReadMapMode(rIStm, aMapMode))           // MapMode
        rMtf.SetPrefMapMode(aMapMode);

    sal_Int32 nActions(0);
    rIStm.ReadInt32(nActions);                      // Action count
    if (nActions < 0)
    {
        SAL_WARN("vcl.gdi", "svm claims negative action count (" << nActions << ")");
        nActions = 0;
    }

    const size_t nMinActionSize = sizeof(sal_uInt16) + sizeof(sal_Int32);
    const size_t nMaxPossibleActions = rIStm.remainingSize() / nMinActionSize;
    if (static_cast<sal_uInt32>(nActions) > nMaxPossibleActions)
    {
        SAL_WARN("vcl.gdi", "svm claims more actions (" << nActions << ") than stream could provide, truncating");
        nActions = nMaxPossibleActions;
    }

    size_t nLastPolygonAction(0);

    for (sal_Int32 i = 0; i < nActions && rIStm.good(); ++i)
    {
        sal_Int16 nType(0);
        rIStm.ReadInt16(nType);
        sal_Int32 nActBegin = rIStm.Tell();
        sal_Int32 nActionSize(0);
        rIStm.ReadInt32(nActionSize);

        SAL_WARN_IF( ( nType > 33 ) && ( nType < 1024 ), "vcl.gdi", "Unknown GDIMetaAction while converting!" );

        switch( nType )
        {
            case GDI_PIXEL_ACTION:
            {
                ReadPair( rIStm, aPt );
                ImplReadColor( rIStm, aActionColor );
                rMtf.AddAction( new MetaPixelAction( aPt, aActionColor ) );
            }
            break;

            case GDI_POINT_ACTION:
            {
                ReadPair( rIStm, aPt );
                rMtf.AddAction( new MetaPointAction( aPt ) );
            }
            break;

            case GDI_LINE_ACTION:
            {
                ReadPair( rIStm, aPt );
                ReadPair( rIStm, aPt1 );
                rMtf.AddAction( new MetaLineAction( aPt, aPt1, aLineInfo ) );
            }
            break;

            case GDI_LINEJOIN_ACTION :
            {
                sal_Int16 nLineJoin(0);
                rIStm.ReadInt16( nLineJoin );
                aLineInfo.SetLineJoin(static_cast<basegfx::B2DLineJoin>(nLineJoin));
            }
            break;

            case GDI_LINECAP_ACTION :
            {
                sal_Int16 nLineCap(0);
                rIStm.ReadInt16( nLineCap );
                aLineInfo.SetLineCap(static_cast<css::drawing::LineCap>(nLineCap));
            }
            break;

            case GDI_LINEDASHDOT_ACTION :
            {
                sal_Int16 a(0);
                sal_Int32 b(0);

                rIStm.ReadInt16( a ); aLineInfo.SetDashCount(a);
                rIStm.ReadInt32( b ); aLineInfo.SetDashLen(b);
                rIStm.ReadInt16( a ); aLineInfo.SetDotCount(a);
                rIStm.ReadInt32( b ); aLineInfo.SetDotLen(b);
                rIStm.ReadInt32( b ); aLineInfo.SetDistance(b);

                if(((aLineInfo.GetDashCount() && aLineInfo.GetDashLen())
                    || (aLineInfo.GetDotCount() && aLineInfo.GetDotLen()))
                    && aLineInfo.GetDistance())
                {
                    aLineInfo.SetStyle(LineStyle::Dash);
                }
            }
            break;

            case GDI_EXTENDEDPOLYGON_ACTION :
            {
                // read the tools::PolyPolygon in every case
                tools::PolyPolygon aInputPolyPolygon;
                ImplReadExtendedPolyPolygonAction(rIStm, aInputPolyPolygon);

                // now check if it can be set somewhere
                if(nLastPolygonAction < rMtf.GetActionSize())
                {
                    MetaPolyLineAction* pPolyLineAction = dynamic_cast< MetaPolyLineAction* >(rMtf.GetAction(nLastPolygonAction));

                    if(pPolyLineAction)
                    {
                        // replace MetaPolyLineAction when we have a single polygon. Do not rely on the
                        // same point count; the originally written GDI_POLYLINE_ACTION may have been
                        // Subdivided for better quality for older usages
                        if(1 == aInputPolyPolygon.Count())
                        {
                            rMtf.ReplaceAction(
                                new MetaPolyLineAction(
                                    aInputPolyPolygon.GetObject(0),
                                    pPolyLineAction->GetLineInfo()),
                                nLastPolygonAction);
                        }
                    }
                    else
                    {
                        MetaPolyPolygonAction* pPolyPolygonAction = dynamic_cast< MetaPolyPolygonAction* >(rMtf.GetAction(nLastPolygonAction));

                        if(pPolyPolygonAction)
                        {
                            // replace MetaPolyPolygonAction when we have a curved polygon. Do rely on the
                            // same sub-polygon count
                            if(pPolyPolygonAction->GetPolyPolygon().Count() == aInputPolyPolygon.Count())
                            {
                                rMtf.ReplaceAction(
                                    new MetaPolyPolygonAction(
                                        aInputPolyPolygon),
                                    nLastPolygonAction);
                            }
                        }
                        else
                        {
                            MetaPolygonAction* pPolygonAction = dynamic_cast< MetaPolygonAction* >(rMtf.GetAction(nLastPolygonAction));

                            if(pPolygonAction)
                            {
                                // replace MetaPolygonAction
                                if(1 == aInputPolyPolygon.Count())
                                {
                                    rMtf.ReplaceAction(
                                        new MetaPolygonAction(
                                            aInputPolyPolygon.GetObject(0)),
                                        nLastPolygonAction);
                                }
                            }
                        }
                    }
                }
            }
            break;

            case GDI_RECT_ACTION:
            {
                ImplReadRect( rIStm, aRect );
                sal_Int32 nTmp(0), nTmp1(0);
                rIStm.ReadInt32( nTmp ).ReadInt32( nTmp1 );

                if( nTmp || nTmp1 )
                    rMtf.AddAction( new MetaRoundRectAction( aRect, nTmp, nTmp1 ) );
                else
                {
                    rMtf.AddAction( new MetaRectAction( aRect ) );

                    if( bFatLine )
                        rMtf.AddAction( new MetaPolyLineAction( aRect, aLineInfo ) );
                }
            }
            break;

            case GDI_ELLIPSE_ACTION:
            {
                ImplReadRect( rIStm, aRect );

                if( bFatLine )
                {
                    const tools::Polygon aPoly( aRect.Center(), aRect.GetWidth() >> 1, aRect.GetHeight() >> 1 );

                    rMtf.AddAction( new MetaPushAction( PushFlags::LINECOLOR ) );
                    rMtf.AddAction( new MetaLineColorAction( COL_TRANSPARENT, false ) );
                    rMtf.AddAction( new MetaPolygonAction( aPoly ) );
                    rMtf.AddAction( new MetaPopAction() );
                    rMtf.AddAction( new MetaPolyLineAction( aPoly, aLineInfo ) );
                }
                else
                    rMtf.AddAction( new MetaEllipseAction( aRect ) );
            }
            break;

            case GDI_ARC_ACTION:
            {
                ImplReadRect( rIStm, aRect );
                ReadPair( rIStm, aPt );
                ReadPair( rIStm, aPt1 );

                if( bFatLine )
                {
                    const tools::Polygon aPoly( aRect, aPt, aPt1, PolyStyle::Arc );

                    rMtf.AddAction( new MetaPushAction( PushFlags::LINECOLOR ) );
                    rMtf.AddAction( new MetaLineColorAction( COL_TRANSPARENT, false ) );
                    rMtf.AddAction( new MetaPolygonAction( aPoly ) );
                    rMtf.AddAction( new MetaPopAction() );
                    rMtf.AddAction( new MetaPolyLineAction( aPoly, aLineInfo ) );
                }
                else
                    rMtf.AddAction( new MetaArcAction( aRect, aPt, aPt1 ) );
            }
            break;

            case GDI_PIE_ACTION:
            {
                ImplReadRect( rIStm, aRect );
                ReadPair( rIStm, aPt );
                ReadPair( rIStm, aPt1 );

                if( bFatLine )
                {
                    const tools::Polygon aPoly( aRect, aPt, aPt1, PolyStyle::Pie );

                    rMtf.AddAction( new MetaPushAction( PushFlags::LINECOLOR ) );
                    rMtf.AddAction( new MetaLineColorAction( COL_TRANSPARENT, false ) );
                    rMtf.AddAction( new MetaPolygonAction( aPoly ) );
                    rMtf.AddAction( new MetaPopAction() );
                    rMtf.AddAction( new MetaPolyLineAction( aPoly, aLineInfo ) );
                }
                else
                    rMtf.AddAction( new MetaPieAction( aRect, aPt, aPt1 ) );
            }
            break;

            case GDI_INVERTRECT_ACTION:
            case GDI_HIGHLIGHTRECT_ACTION:
            {
                ImplReadRect( rIStm, aRect );
                rMtf.AddAction( new MetaPushAction( PushFlags::RASTEROP ) );
                rMtf.AddAction( new MetaRasterOpAction( RasterOp::Invert ) );
                rMtf.AddAction( new MetaRectAction( aRect ) );
                rMtf.AddAction( new MetaPopAction() );
            }
            break;

            case GDI_POLYLINE_ACTION:
            {
                if (ImplReadPoly(rIStm, aActionPoly))
                {
                    nLastPolygonAction = rMtf.GetActionSize();

                    if( bFatLine )
                        rMtf.AddAction( new MetaPolyLineAction( aActionPoly, aLineInfo ) );
                    else
                        rMtf.AddAction( new MetaPolyLineAction( aActionPoly ) );
                }
            }
            break;

            case GDI_POLYGON_ACTION:
            {
                if (ImplReadPoly(rIStm, aActionPoly))
                {
                    if( bFatLine )
                    {
                        rMtf.AddAction( new MetaPushAction( PushFlags::LINECOLOR ) );
                        rMtf.AddAction( new MetaLineColorAction( COL_TRANSPARENT, false ) );
                        rMtf.AddAction( new MetaPolygonAction( aActionPoly ) );
                        rMtf.AddAction( new MetaPopAction() );
                        rMtf.AddAction( new MetaPolyLineAction( aActionPoly, aLineInfo ) );
                    }
                    else
                    {
                        nLastPolygonAction = rMtf.GetActionSize();
                        rMtf.AddAction( new MetaPolygonAction( aActionPoly ) );
                    }
                }
            }
            break;

            case GDI_POLYPOLYGON_ACTION:
            {
                tools::PolyPolygon aPolyPoly;

                if (ImplReadPolyPoly(rIStm, aPolyPoly))
                {
                    if( bFatLine )
                    {
                        rMtf.AddAction( new MetaPushAction( PushFlags::LINECOLOR ) );
                        rMtf.AddAction( new MetaLineColorAction( COL_TRANSPARENT, false ) );
                        rMtf.AddAction( new MetaPolyPolygonAction( aPolyPoly ) );
                        rMtf.AddAction( new MetaPopAction() );

                        for( sal_uInt16 nPoly = 0, nCount = aPolyPoly.Count(); nPoly < nCount; nPoly++ )
                            rMtf.AddAction( new MetaPolyLineAction( aPolyPoly[ nPoly ], aLineInfo ) );
                    }
                    else
                    {
                        nLastPolygonAction = rMtf.GetActionSize();
                        rMtf.AddAction( new MetaPolyPolygonAction( aPolyPoly ) );
                    }
                }
            }
            break;

            case GDI_FONT_ACTION:
            {
                vcl::Font   aFont;
                char        aName[LF_FACESIZE+1];

                ImplReadColor( rIStm, aActionColor ); aFont.SetColor( aActionColor );
                ImplReadColor( rIStm, aActionColor ); aFont.SetFillColor( aActionColor );
                size_t nRet = rIStm.ReadBytes(aName, LF_FACESIZE);
                aName[nRet] = 0;
                aFont.SetFamilyName( OUString( aName, strlen(aName), rIStm.GetStreamCharSet() ) );

                sal_Int32 nWidth(0), nHeight(0);
                rIStm.ReadInt32(nWidth).ReadInt32(nHeight);
                sal_Int16 nCharOrient(0), nLineOrient(0);
                rIStm.ReadInt16(nCharOrient).ReadInt16(nLineOrient);
                sal_Int16 nCharSet(0), nFamily(0), nPitch(0), nAlign(0), nWeight(0), nUnderline(0), nStrikeout(0);
                rIStm.ReadInt16(nCharSet).ReadInt16(nFamily).ReadInt16(nPitch).ReadInt16(nAlign).ReadInt16(nWeight).ReadInt16(nUnderline).ReadInt16(nStrikeout);
                bool bItalic(false), bOutline(false), bShadow(false), bTransparent(false);
                rIStm.ReadCharAsBool(bItalic).ReadCharAsBool(bOutline).ReadCharAsBool(bShadow).ReadCharAsBool(bTransparent);

                aFont.SetFontSize( Size( nWidth, nHeight ) );
                aFont.SetCharSet( static_cast<rtl_TextEncoding>(nCharSet) );
                aFont.SetFamily( static_cast<FontFamily>(nFamily) );
                aFont.SetPitch( static_cast<FontPitch>(nPitch) );
                aFont.SetAlignment( static_cast<FontAlign>(nAlign) );
                aFont.SetWeight( ( nWeight == 1 ) ? WEIGHT_LIGHT : ( nWeight == 2 ) ? WEIGHT_NORMAL :
                                 ( nWeight == 3 ) ? WEIGHT_BOLD : WEIGHT_DONTKNOW );
                aFont.SetUnderline( static_cast<FontLineStyle>(nUnderline) );
                aFont.SetStrikeout( static_cast<FontStrikeout>(nStrikeout) );
                aFont.SetItalic( bItalic ? ITALIC_NORMAL : ITALIC_NONE );
                aFont.SetOutline( bOutline );
                aFont.SetShadow( bShadow );
                aFont.SetOrientation( nLineOrient );
                aFont.SetTransparent( bTransparent );

                eActualCharSet = aFont.GetCharSet();
                if ( eActualCharSet == RTL_TEXTENCODING_DONTKNOW )
                    eActualCharSet = osl_getThreadTextEncoding();

                rMtf.AddAction( new MetaFontAction( aFont ) );
                rMtf.AddAction( new MetaTextAlignAction( aFont.GetAlignment() ) );
                rMtf.AddAction( new MetaTextColorAction( aFont.GetColor() ) );
                rMtf.AddAction( new MetaTextFillColorAction( aFont.GetFillColor(), !aFont.IsTransparent() ) );

                // #106172# Track font relevant data in shadow VDev
                aFontVDev->SetFont( aFont );
            }
            break;

            case GDI_TEXT_ACTION:
            {
                sal_Int32 nIndex(0), nLen(0), nTmp(0);

                ReadPair( rIStm, aPt ).ReadInt32( nIndex ).ReadInt32( nLen ).ReadInt32( nTmp );
                if (nTmp > 0)
                {
                    OString aByteStr = read_uInt8s_ToOString(rIStm, nTmp);
                    sal_uInt8 nTerminator = 0;
                    rIStm.ReadUChar( nTerminator );
                    SAL_WARN_IF( nTerminator != 0, "vcl.gdi", "expected string to be NULL terminated" );

                    OUString aStr(OStringToOUString(aByteStr, eActualCharSet));
                    if ( nUnicodeCommentActionNumber == i )
                        ImplReadUnicodeComment( nUnicodeCommentStreamPos, rIStm, aStr );
                    rMtf.AddAction( new MetaTextAction( aPt, aStr, nIndex, nLen ) );
                }

                if (nActionSize < 24)
                    rIStm.SetError(SVSTREAM_FILEFORMAT_ERROR);
                else
                    rIStm.Seek(nActBegin + nActionSize);
            }
            break;

            case GDI_TEXTARRAY_ACTION:
            {
                sal_Int32 nIndex(0), nLen(0), nAryLen(0), nTmp(0);

                ReadPair( rIStm, aPt ).ReadInt32( nIndex ).ReadInt32( nLen ).ReadInt32( nTmp ).ReadInt32( nAryLen );
                if (nTmp > 0)
                {
                    OString aByteStr = read_uInt8s_ToOString(rIStm, nTmp);
                    sal_uInt8 nTerminator = 0;
                    rIStm.ReadUChar( nTerminator );
                    SAL_WARN_IF( nTerminator != 0, "vcl.gdi", "expected string to be NULL terminated" );

                    OUString aStr(OStringToOUString(aByteStr, eActualCharSet));

                    std::unique_ptr<long[]> pDXAry;
                    sal_Int32 nDXAryLen = 0;
                    if (nAryLen > 0)
                    {
                        const size_t nMinRecordSize = sizeof(sal_Int32);
                        const size_t nMaxRecords = rIStm.remainingSize() / nMinRecordSize;
                        if (static_cast<sal_uInt32>(nAryLen) > nMaxRecords)
                        {
                            SAL_WARN("vcl.gdi", "Parsing error: " << nMaxRecords <<
                                     " max possible entries, but " << nAryLen << " claimed, truncating");
                            nAryLen = nMaxRecords;
                        }

                        sal_Int32 nStrLen( aStr.getLength() );

                        nDXAryLen = std::max(nAryLen, nStrLen);

                        if (nDXAryLen < nLen)
                        {
                            //MetaTextArrayAction ctor expects pDXAry to be >= nLen if set, so if this can't
                            //be achieved, don't read it, it's utterly broken.
                            SAL_WARN("vcl.gdi", "dxary too short, discarding completely");
                            rIStm.SeekRel(sizeof(sal_Int32) * nDXAryLen);
                            nLen = 0;
                            nIndex = 0;
                        }
                        else
                        {
                            pDXAry.reset(new long[nDXAryLen]);

                            for (sal_Int32 j = 0; j < nAryLen; ++j)
                            {
                                rIStm.ReadInt32( nTmp );
                                pDXAry[ j ] = nTmp;
                            }

                            // #106172# Add last DX array elem, if missing
                            if( nAryLen != nStrLen )
                            {
                                if (nAryLen+1 == nStrLen && nIndex >= 0)
                                {
                                    std::unique_ptr<long[]> pTmpAry(new long[nStrLen]);

                                    aFontVDev->GetTextArray( aStr, pTmpAry.get(), nIndex, nLen );

                                    // now, the difference between the
                                    // last and the second last DX array
                                    // is the advancement for the last
                                    // glyph. Thus, to complete our meta
                                    // action's DX array, just add that
                                    // difference to last elem and store
                                    // in very last.
                                    if( nStrLen > 1 )
                                        pDXAry[ nStrLen-1 ] = pDXAry[ nStrLen-2 ] + pTmpAry[ nStrLen-1 ] - pTmpAry[ nStrLen-2 ];
                                    else
                                        pDXAry[ nStrLen-1 ] = pTmpAry[ nStrLen-1 ]; // len=1: 0th position taken to be 0
                                }
#ifdef DBG_UTIL
                                else
                                    OSL_FAIL("More than one DX array element missing on SVM import");
#endif
                            }
                        }
                    }
                    if ( nUnicodeCommentActionNumber == i )
                        ImplReadUnicodeComment( nUnicodeCommentStreamPos, rIStm, aStr );
                    rMtf.AddAction( new MetaTextArrayAction( aPt, aStr, pDXAry.get(), nIndex, nLen ) );
                }

                if (nActionSize < 24)
                    rIStm.SetError(SVSTREAM_FILEFORMAT_ERROR);
                else
                    rIStm.Seek(nActBegin + nActionSize);
            }
            break;

            case GDI_STRETCHTEXT_ACTION:
            {
                sal_Int32 nIndex(0), nLen(0), nWidth(0), nTmp(0);

                ReadPair( rIStm, aPt ).ReadInt32( nIndex ).ReadInt32( nLen ).ReadInt32( nTmp ).ReadInt32( nWidth );
                if (nTmp > 0)
                {
                    OString aByteStr = read_uInt8s_ToOString(rIStm, nTmp);
                    sal_uInt8 nTerminator = 0;
                    rIStm.ReadUChar( nTerminator );
                    SAL_WARN_IF( nTerminator != 0, "vcl.gdi", "expected string to be NULL terminated" );

                    OUString aStr(OStringToOUString(aByteStr, eActualCharSet));
                    if ( nUnicodeCommentActionNumber == i )
                        ImplReadUnicodeComment( nUnicodeCommentStreamPos, rIStm, aStr );
                    rMtf.AddAction( new MetaStretchTextAction( aPt, nWidth, aStr, nIndex, nLen ) );
                }

                if (nActionSize < 28)
                    rIStm.SetError(SVSTREAM_FILEFORMAT_ERROR);
                else
                    rIStm.Seek(nActBegin + nActionSize);
            }
            break;

            case GDI_BITMAP_ACTION:
            {
                Bitmap aBmp;

                ReadPair( rIStm, aPt );
                ReadDIB(aBmp, rIStm, true);
                rMtf.AddAction( new MetaBmpAction( aPt, aBmp ) );
            }
            break;

            case GDI_BITMAPSCALE_ACTION:
            {
                Bitmap aBmp;

                ReadPair( rIStm, aPt );
                ReadPair( rIStm, aSz );
                ReadDIB(aBmp, rIStm, true);
                rMtf.AddAction( new MetaBmpScaleAction( aPt, aSz, aBmp ) );
            }
            break;

            case GDI_BITMAPSCALEPART_ACTION:
            {
                Bitmap  aBmp;
                Size    aSz2;

                ReadPair( rIStm, aPt );
                ReadPair( rIStm, aSz );
                ReadPair( rIStm, aPt1 );
                ReadPair( rIStm, aSz2 );
                ReadDIB(aBmp, rIStm, true);
                rMtf.AddAction( new MetaBmpScalePartAction( aPt, aSz, aPt1, aSz2, aBmp ) );
            }
            break;

            case GDI_PEN_ACTION:
            {
                sal_Int32 nPenWidth;
                sal_Int16 nPenStyle;

                ImplReadColor( rIStm, aActionColor );
                rIStm.ReadInt32( nPenWidth ).ReadInt16( nPenStyle );

                aLineInfo.SetStyle( nPenStyle ? LineStyle::Solid : LineStyle::NONE );
                aLineInfo.SetWidth( nPenWidth );
                bFatLine = nPenStyle && !aLineInfo.IsDefault();

                rMtf.AddAction( new MetaLineColorAction( aActionColor, nPenStyle != 0 ) );
            }
            break;

            case GDI_FILLBRUSH_ACTION:
            {
                sal_Int16 nBrushStyle;

                ImplReadColor( rIStm, aActionColor );
                rIStm.SeekRel( 6 );
                rIStm.ReadInt16( nBrushStyle );
                rMtf.AddAction( new MetaFillColorAction( aActionColor, nBrushStyle != 0 ) );
                rIStm.SeekRel( 2 );
            }
            break;

            case GDI_MAPMODE_ACTION:
            {
                if (ImplReadMapMode(rIStm, aMapMode))
                {
                    rMtf.AddAction(new MetaMapModeAction(aMapMode));

                    // #106172# Track font relevant data in shadow VDev
                    aFontVDev->SetMapMode(aMapMode);
                };
            }
            break;

            case GDI_CLIPREGION_ACTION:
            {
                vcl::Region  aRegion;
                sal_Int16   nRegType;
                sal_Int16   bIntersect;
                bool    bClip = false;

                rIStm.ReadInt16( nRegType ).ReadInt16( bIntersect );
                ImplReadRect( rIStm, aRect );

                switch( nRegType )
                {
                    case 0:
                    break;

                    case 1:
                    {
                        tools::Rectangle aRegRect;

                        ImplReadRect( rIStm, aRegRect );
                        aRegion = vcl::Region( aRegRect );
                        bClip = true;
                    }
                    break;

                    case 2:
                    {
                        if (ImplReadPoly(rIStm, aActionPoly))
                        {
                            aRegion = vcl::Region( aActionPoly );
                            bClip = true;
                        }
                    }
                    break;

                    case 3:
                    {
                        bool bSuccess = true;
                        tools::PolyPolygon aPolyPoly;
                        sal_Int32 nPolyCount32(0);
                        rIStm.ReadInt32(nPolyCount32);
                        sal_uInt16 nPolyCount(nPolyCount32);

                        for (sal_uInt16 j = 0; j < nPolyCount && rIStm.good(); ++j)
                        {
                            if (!ImplReadPoly(rIStm, aActionPoly))
                            {
                                bSuccess = false;
                                break;
                            }
                            aPolyPoly.Insert(aActionPoly);
                        }

                        if (bSuccess)
                        {
                            aRegion = vcl::Region( aPolyPoly );
                            bClip = true;
                        }
                    }
                    break;
                }

                if( bIntersect )
                    aRegion.Intersect( aRect );

                rMtf.AddAction( new MetaClipRegionAction( aRegion, bClip ) );
            }
            break;

            case GDI_MOVECLIPREGION_ACTION:
            {
                sal_Int32 nTmp(0), nTmp1(0);
                rIStm.ReadInt32( nTmp ).ReadInt32( nTmp1 );
                rMtf.AddAction( new MetaMoveClipRegionAction( nTmp, nTmp1 ) );
            }
            break;

            case GDI_ISECTCLIPREGION_ACTION:
            {
                ImplReadRect( rIStm, aRect );
                rMtf.AddAction( new MetaISectRectClipRegionAction( aRect ) );
            }
            break;

            case GDI_RASTEROP_ACTION:
            {
                RasterOp    eRasterOp;
                sal_Int16       nRasterOp;

                rIStm.ReadInt16( nRasterOp );

                switch( nRasterOp )
                {
                    case 1:
                        eRasterOp = RasterOp::Invert;
                    break;

                    case 4:
                    case 5:
                        eRasterOp = RasterOp::Xor;
                    break;

                    default:
                        eRasterOp = RasterOp::OverPaint;
                    break;
                }

                rMtf.AddAction( new MetaRasterOpAction( eRasterOp ) );
            }
            break;

            case GDI_PUSH_ACTION:
            {
                aLIStack.push(std::make_unique<LineInfo>(aLineInfo));
                rMtf.AddAction( new MetaPushAction( PushFlags::ALL ) );

                // #106172# Track font relevant data in shadow VDev
                aFontVDev->Push();
            }
            break;

            case GDI_POP_ACTION:
            {

                std::unique_ptr<LineInfo> xLineInfo;
                if (!aLIStack.empty())
                {
                    xLineInfo = std::move(aLIStack.top());
                    aLIStack.pop();
                }

                // restore line info
                if (xLineInfo)
                {
                    aLineInfo = *xLineInfo;
                    xLineInfo.reset();
                    bFatLine = ( LineStyle::NONE != aLineInfo.GetStyle() ) && !aLineInfo.IsDefault();
                }

                rMtf.AddAction( new MetaPopAction() );

                // #106172# Track font relevant data in shadow VDev
                aFontVDev->Pop();
            }
            break;

            case GDI_GRADIENT_ACTION:
            {
                Color   aStartCol;
                Color   aEndCol;
                sal_Int16   nStyle;
                sal_Int16   nAngle;
                sal_Int16   nBorder;
                sal_Int16   nOfsX;
                sal_Int16   nOfsY;
                sal_Int16   nIntensityStart;
                sal_Int16   nIntensityEnd;

                ImplReadRect( rIStm, aRect );
                rIStm.ReadInt16( nStyle );
                ImplReadColor( rIStm, aStartCol );
                ImplReadColor( rIStm, aEndCol );
                rIStm.ReadInt16( nAngle ).ReadInt16( nBorder ).ReadInt16( nOfsX ).ReadInt16( nOfsY ).ReadInt16( nIntensityStart ).ReadInt16( nIntensityEnd );

                Gradient aGrad( static_cast<GradientStyle>(nStyle), aStartCol, aEndCol );

                aGrad.SetAngle( nAngle );
                aGrad.SetBorder( nBorder );
                aGrad.SetOfsX( nOfsX );
                aGrad.SetOfsY( nOfsY );
                aGrad.SetStartIntensity( nIntensityStart );
                aGrad.SetEndIntensity( nIntensityEnd );
                rMtf.AddAction( new MetaGradientAction( aRect, aGrad ) );
            }
            break;

            case GDI_TRANSPARENT_COMMENT:
            {
                tools::PolyPolygon aPolyPoly;
                sal_Int32 nFollowingActionCount(0);
                sal_Int16 nTrans(0);

                ReadPolyPolygon( rIStm, aPolyPoly );
                rIStm.ReadInt16( nTrans ).ReadInt32( nFollowingActionCount );
                ImplSkipActions( rIStm, nFollowingActionCount );
                rMtf.AddAction( new MetaTransparentAction( aPolyPoly, nTrans ) );

                i = SkipActions(i, nFollowingActionCount, nActions);
            }
            break;

            case GDI_FLOATTRANSPARENT_COMMENT:
            {
                GDIMetaFile aMtf;
                Point       aPos;
                Size        aSize;
                Gradient    aGradient;
                sal_Int32   nFollowingActionCount(0);

                ReadGDIMetaFile( rIStm, aMtf );
                ReadPair( rIStm, aPos );
                ReadPair( rIStm, aSize );
                ReadGradient( rIStm, aGradient );
                rIStm.ReadInt32( nFollowingActionCount );
                ImplSkipActions( rIStm, nFollowingActionCount );
                rMtf.AddAction( new MetaFloatTransparentAction( aMtf, aPos, aSize, aGradient ) );

                i = SkipActions(i, nFollowingActionCount, nActions);
            }
            break;

            case GDI_HATCH_COMMENT:
            {
                tools::PolyPolygon aPolyPoly;
                Hatch       aHatch;
                sal_Int32   nFollowingActionCount(0);

                ReadPolyPolygon( rIStm, aPolyPoly );
                ReadHatch( rIStm, aHatch );
                rIStm.ReadInt32( nFollowingActionCount );
                ImplSkipActions( rIStm, nFollowingActionCount );
                rMtf.AddAction( new MetaHatchAction( aPolyPoly, aHatch ) );

                i = SkipActions(i, nFollowingActionCount, nActions);
            }
            break;

            case GDI_REFPOINT_COMMENT:
            {
                Point   aRefPoint;
                bool    bSet;
                sal_Int32 nFollowingActionCount(0);

                ReadPair( rIStm, aRefPoint );
                rIStm.ReadCharAsBool( bSet ).ReadInt32( nFollowingActionCount );
                ImplSkipActions( rIStm, nFollowingActionCount );
                rMtf.AddAction( new MetaRefPointAction( aRefPoint, bSet ) );

                i = SkipActions(i, nFollowingActionCount, nActions);

                // #106172# Track font relevant data in shadow VDev
                if( bSet )
                    aFontVDev->SetRefPoint( aRefPoint );
                else
                    aFontVDev->SetRefPoint();
            }
            break;

            case GDI_TEXTLINECOLOR_COMMENT:
            {
                Color   aColor;
                bool    bSet;
                sal_Int32 nFollowingActionCount(0);

                ReadColor( rIStm, aColor );
                rIStm.ReadCharAsBool( bSet ).ReadInt32( nFollowingActionCount );
                ImplSkipActions( rIStm, nFollowingActionCount );
                rMtf.AddAction( new MetaTextLineColorAction( aColor, bSet ) );

                i = SkipActions(i, nFollowingActionCount, nActions);
            }
            break;

            case GDI_TEXTLINE_COMMENT:
            {
                Point   aStartPt;
                sal_Int32  nWidth(0);
                sal_uInt32 nStrikeout(0);
                sal_uInt32 nUnderline(0);
                sal_Int32  nFollowingActionCount(0);

                ReadPair( rIStm, aStartPt );
                rIStm.ReadInt32(nWidth ).ReadUInt32(nStrikeout).ReadUInt32(nUnderline).ReadInt32(nFollowingActionCount);
                ImplSkipActions(rIStm, nFollowingActionCount);
                rMtf.AddAction( new MetaTextLineAction( aStartPt, nWidth,
                                                        static_cast<FontStrikeout>(nStrikeout),
                                                        static_cast<FontLineStyle>(nUnderline),
                                                        LINESTYLE_NONE ) );

                i = SkipActions(i, nFollowingActionCount, nActions);
            }
            break;

            case GDI_GRADIENTEX_COMMENT:
            {
                tools::PolyPolygon aPolyPoly;
                Gradient    aGradient;
                sal_Int32 nFollowingActionCount(0);

                ReadPolyPolygon( rIStm, aPolyPoly );
                ReadGradient( rIStm, aGradient );
                rIStm.ReadInt32( nFollowingActionCount );
                ImplSkipActions( rIStm, nFollowingActionCount );
                rMtf.AddAction( new MetaGradientExAction( aPolyPoly, aGradient ) );

                i = SkipActions(i, nFollowingActionCount, nActions);
            }
            break;

            case GDI_COMMENT_COMMENT:
            {
                std::vector<sal_uInt8> aData;

                OString aComment = read_uInt16_lenPrefixed_uInt8s_ToOString(rIStm);
                sal_Int32 nValue(0);
                sal_uInt32 nDataSize(0);
                rIStm.ReadInt32(nValue).ReadUInt32(nDataSize);

                if (nDataSize)
                {
                    const size_t nMaxPossibleData = rIStm.remainingSize();
                    if (nDataSize > nMaxPossibleActions)
                    {
                        SAL_WARN("vcl.gdi", "svm record claims to have: " << nDataSize << " data, but only " << nMaxPossibleData << " possible");
                        nDataSize = nMaxPossibleActions;
                    }
                    aData.resize(nDataSize);
                    nDataSize = rIStm.ReadBytes(aData.data(), nDataSize);
                }

                sal_Int32 nFollowingActionCount(0);
                rIStm.ReadInt32(nFollowingActionCount);
                ImplSkipActions( rIStm, nFollowingActionCount );
                rMtf.AddAction(new MetaCommentAction(aComment, nValue, aData.data(), nDataSize));

                i = SkipActions(i, nFollowingActionCount, nActions);
            }
            break;

            case GDI_UNICODE_COMMENT:
            {
                nUnicodeCommentActionNumber = i + 1;
                nUnicodeCommentStreamPos = rIStm.Tell() - 6;
                if (nActionSize < 4)
                    rIStm.SetError(SVSTREAM_FILEFORMAT_ERROR);
                else
                    rIStm.SeekRel(nActionSize - 4);
            }
            break;

            default:
                if (nActionSize < 4)
                    rIStm.SetError(SVSTREAM_FILEFORMAT_ERROR);
                else
                    rIStm.SeekRel(nActionSize - 4);
            break;
        }
    }

    rIStm.SetEndian( nOldFormat );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
