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

#include <editeng/legacyitem.hxx>
#include <unotools/fontdefs.hxx>
#include <tools/tenccvt.hxx>
#include <tools/stream.hxx>
#include <comphelper/fileformat.h>
#include <vcl/graph.hxx>
#include <vcl/GraphicObject.hxx>
#include <osl/diagnose.h>
#include <tools/urlobj.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/borderline.hxx>
#include <editeng/lineitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/editerr.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/justifyitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/shaditem.hxx>
#include <tools/GenericTypeSerializer.hxx>


void Create_legacy_direct_set(SvxFontHeightItem& rItem, sal_uInt32 nH, sal_uInt16 nP, MapUnit eP)
{
    rItem.legacy_direct_set(nH, nP, eP);
}

namespace legacy
{
    namespace SvxFont
    {
        sal_uInt16 GetVersion(sal_uInt16)
        {
            return 0;
        }

        void Create(SvxFontItem& rItem, SvStream& rStrm, sal_uInt16)
        {
            sal_uInt8 _eFamily, eFontPitch, eFontTextEncoding;
            OUString aName, aStyle;
            rStrm.ReadUChar( _eFamily );
            rStrm.ReadUChar( eFontPitch );
            rStrm.ReadUChar( eFontTextEncoding );

            // UNICODE: rStrm >> aName;
            aName = rStrm.ReadUniOrByteString(rStrm.GetStreamCharSet());

            // UNICODE: rStrm >> aStyle;
            aStyle = rStrm.ReadUniOrByteString(rStrm.GetStreamCharSet());

            // Set the "correct" textencoding
            eFontTextEncoding = static_cast<sal_uInt8>(GetSOLoadTextEncoding( eFontTextEncoding ));

            // at some point, the StarBats changes from  ANSI font to SYMBOL font
            if ( RTL_TEXTENCODING_SYMBOL != eFontTextEncoding && aName == "StarBats" )
                eFontTextEncoding = RTL_TEXTENCODING_SYMBOL;

            // Check if we have stored unicode
            sal_uInt64 const nStreamPos = rStrm.Tell();
            // #define STORE_UNICODE_MAGIC_MARKER  0xFE331188
            sal_uInt32 nMagic = 0xFE331188;
            rStrm.ReadUInt32( nMagic );
            if ( nMagic == 0xFE331188 )
            {
                aName = rStrm.ReadUniOrByteString( RTL_TEXTENCODING_UNICODE );
                aStyle = rStrm.ReadUniOrByteString( RTL_TEXTENCODING_UNICODE );
            }
            else
            {
                rStrm.Seek( nStreamPos );
            }

            rItem.SetFamilyName(aName);
            rItem.SetStyleName(aStyle);
            rItem.SetFamily(static_cast<FontFamily>(_eFamily));
            rItem.SetPitch(static_cast<FontPitch>(eFontPitch));
            rItem.SetCharSet(static_cast<rtl_TextEncoding>(eFontTextEncoding));
        }

        SvStream& Store(const SvxFontItem& rItem, SvStream& rStrm, sal_uInt16)
        {
            const bool bToBats(IsStarSymbol(rItem.GetFamilyName()));

            rStrm.WriteUChar(rItem.GetFamily()).WriteUChar(rItem.GetPitch()).WriteUChar(bToBats ?
                    RTL_TEXTENCODING_SYMBOL :
                    GetSOStoreTextEncoding(rItem.GetCharSet()));

            const OUString aStoreFamilyName(bToBats ? "StarBats" : rItem.GetFamilyName());

            rStrm.WriteUniOrByteString(aStoreFamilyName, rStrm.GetStreamCharSet());
            rStrm.WriteUniOrByteString(rItem.GetStyleName(), rStrm.GetStreamCharSet());

            return rStrm;
        }
    }

    namespace SvxFontHeight
    {
        sal_uInt16 GetVersion(sal_uInt16 nFileFormatVersion)
        {
            return (nFileFormatVersion <= SOFFICE_FILEFORMAT_40)
                    ? FONTHEIGHT_16_VERSION
                    : FONTHEIGHT_UNIT_VERSION;
        }

        void Create(SvxFontHeightItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion)
        {
            sal_uInt16 nsize, nprop = 0;
            MapUnit nPropUnit = MapUnit::MapRelative;

            rStrm.ReadUInt16( nsize );

            if( FONTHEIGHT_16_VERSION <= nItemVersion )
                rStrm.ReadUInt16( nprop );
            else
            {
                sal_uInt8 nP;
                rStrm .ReadUChar( nP );
                nprop = static_cast<sal_uInt16>(nP);
            }

            if( FONTHEIGHT_UNIT_VERSION <= nItemVersion )
            {
                sal_uInt16 nTmp;
                rStrm.ReadUInt16( nTmp );
                nPropUnit = static_cast<MapUnit>(nTmp);
            }

            Create_legacy_direct_set(rItem, nsize, nprop, nPropUnit);
        }

        SvStream& Store(const SvxFontHeightItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion)
        {
            rStrm.WriteUInt16( rItem.GetHeight() );

            if( FONTHEIGHT_UNIT_VERSION <= nItemVersion )
                rStrm.WriteUInt16( rItem.GetProp() ).WriteUInt16( static_cast<sal_uInt16>(rItem.GetPropUnit()) );
            else
            {
                // When exporting to the old versions the relative information is lost
                // when there is no percentage
                sal_uInt16 _nProp = rItem.GetProp();
                if( MapUnit::MapRelative != rItem.GetPropUnit() )
                    _nProp = 100;
                rStrm.WriteUInt16( _nProp );
            }
            return rStrm;
        }
    }

    namespace SvxWeight
    {
        sal_uInt16 GetVersion(sal_uInt16)
        {
            return 0;
        }

        void Create(SvxWeightItem& rItem, SvStream& rStrm, sal_uInt16)
        {
            sal_uInt8 nWeight(0);
            rStrm.ReadUChar(nWeight);
            rItem.SetValue(static_cast<FontWeight>(nWeight));
        }

        SvStream& Store(const SvxWeightItem& rItem, SvStream& rStrm, sal_uInt16)
        {
            rStrm.WriteUChar(rItem.GetValue());
            return rStrm;
        }
    }

    namespace SvxPosture
    {
        sal_uInt16 GetVersion(sal_uInt16)
        {
            return 0;
        }

        void Create(SvxPostureItem& rItem, SvStream& rStrm, sal_uInt16)
        {
            sal_uInt8 nPosture(0);
            rStrm.ReadUChar(nPosture);
            rItem.SetValue(static_cast<FontItalic>(nPosture));
        }

        SvStream& Store(const SvxPostureItem& rItem, SvStream& rStrm, sal_uInt16)
        {
            rStrm.WriteUChar( rItem.GetValue() );
            return rStrm;
        }
    }

    namespace SvxTextLine // SvxUnderlineItem, SvxOverlineItem -> SvxTextLineItem
    {
        sal_uInt16 GetVersion(sal_uInt16)
        {
            return 0;
        }

        void Create(SvxTextLineItem& rItem, SvStream& rStrm, sal_uInt16)
        {
            sal_uInt8 nState(0);
            rStrm.ReadUChar(nState);
            rItem.SetValue(static_cast<FontLineStyle>(nState));
            // GetColor() is *not* saved/loaded ?!?
        }

        SvStream& Store(const SvxTextLineItem& rItem, SvStream& rStrm, sal_uInt16)
        {
            rStrm.WriteUChar(rItem.GetValue());
            // GetColor() is *not* saved/loaded ?!?
            return rStrm;
        }
    }

    namespace SvxCrossedOut
    {
        sal_uInt16 GetVersion(sal_uInt16)
        {
            return 0;
        }

        void Create(SvxCrossedOutItem& rItem, SvStream& rStrm, sal_uInt16)
        {
            sal_uInt8 eCross(0);
            rStrm.ReadUChar(eCross);
            rItem.SetValue(static_cast<FontStrikeout>(eCross));
        }

        SvStream& Store(const SvxCrossedOutItem& rItem, SvStream& rStrm, sal_uInt16)
        {
            rStrm.WriteUChar(rItem.GetValue());
            return rStrm;
        }
    }

    namespace SvxColor
    {
        sal_uInt16 GetVersion(sal_uInt16 nFileFormatVersion)
        {
            DBG_ASSERT( SOFFICE_FILEFORMAT_31==nFileFormatVersion ||
                    SOFFICE_FILEFORMAT_40==nFileFormatVersion ||
                    SOFFICE_FILEFORMAT_50==nFileFormatVersion,
                    "SvxColorItem: Is there a new file format? ");
            return  SOFFICE_FILEFORMAT_50 >= nFileFormatVersion ? VERSION_USEAUTOCOLOR : 0;
        }

        void Create(SvxColorItem& rItem, SvStream& rStrm, sal_uInt16)
        {
            Color aColor(COL_AUTO);
            tools::GenericTypeSerializer aSerializer(rStrm);
            aSerializer.readColor(aColor);
            rItem.SetValue(aColor);
        }

        SvStream& Store(const SvxColorItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion)
        {
            tools::GenericTypeSerializer aSerializer(rStrm);
            if( VERSION_USEAUTOCOLOR == nItemVersion && COL_AUTO == rItem.GetValue() )
                aSerializer.writeColor(COL_BLACK);
            else
                aSerializer.writeColor(rItem.GetValue());
            return rStrm;
        }
    }

    namespace SvxBox
    {
        sal_uInt16 GetVersion(sal_uInt16 nFileFormatVersion)
        {
            DBG_ASSERT( SOFFICE_FILEFORMAT_31==nFileFormatVersion ||
                    SOFFICE_FILEFORMAT_40==nFileFormatVersion ||
                    SOFFICE_FILEFORMAT_50==nFileFormatVersion,
                    "SvxBoxItem: Is there a new file format?" );
            return SOFFICE_FILEFORMAT_31==nFileFormatVersion ||
                SOFFICE_FILEFORMAT_40==nFileFormatVersion ? 0 : BOX_BORDER_STYLE_VERSION;
        }

        /// Item version for saved border lines. The old version saves the line without style information.
        const int BORDER_LINE_OLD_VERSION = 0;
        /// Item version for saved border lies. The new version includes line style.
        const int BORDER_LINE_WITH_STYLE_VERSION = 1;

        /// Creates a border line from a stream.
        static ::editeng::SvxBorderLine CreateBorderLine(SvStream &stream, sal_uInt16 version)
        {
            sal_uInt16 nOutline, nInline, nDistance;
            sal_uInt16 nStyle = css::table::BorderLineStyle::NONE;
            Color aColor;
            tools::GenericTypeSerializer aSerializer(stream);
            aSerializer.readColor(aColor);
            stream.ReadUInt16( nOutline ).ReadUInt16( nInline ).ReadUInt16( nDistance );

            if (version >= BORDER_LINE_WITH_STYLE_VERSION)
                stream.ReadUInt16( nStyle );

            ::editeng::SvxBorderLine border(&aColor);
            border.GuessLinesWidths(static_cast<SvxBorderLineStyle>(nStyle), nOutline, nInline, nDistance);
            return border;
        }

        /// Retrieves a BORDER_LINE_* version from a BOX_BORDER_* version.
        static sal_uInt16 BorderLineVersionFromBoxVersion(sal_uInt16 boxVersion)
        {
            return (boxVersion >= BOX_BORDER_STYLE_VERSION)? BORDER_LINE_WITH_STYLE_VERSION : BORDER_LINE_OLD_VERSION;
        }

        void Create(SvxBoxItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion)
        {
            sal_uInt16 nDistance(0);
            rStrm.ReadUInt16( nDistance );
            SvxBoxItemLine aLineMap[4] = { SvxBoxItemLine::TOP, SvxBoxItemLine::LEFT,
                                SvxBoxItemLine::RIGHT, SvxBoxItemLine::BOTTOM };
            sal_Int8 cLine(0);

            while (rStrm.good())
            {
                rStrm.ReadSChar( cLine );

                if( cLine > 3 )
                    break;

                ::editeng::SvxBorderLine aBorder = CreateBorderLine(rStrm, BorderLineVersionFromBoxVersion(nItemVersion));
                rItem.SetLine( &aBorder, aLineMap[cLine] );
            }

            if( nItemVersion >= BOX_4DISTS_VERSION && (cLine&0x10) != 0 )
            {
                for(const SvxBoxItemLine & i : aLineMap)
                {
                    sal_uInt16 nDist;
                    rStrm.ReadUInt16( nDist );
                    rItem.SetDistance( nDist, i );
                }
            }
            else
            {
                rItem.SetAllDistances(nDistance);
            }
        }

        /// Store a border line to a stream.
        static SvStream& StoreBorderLine(SvStream &stream, const ::editeng::SvxBorderLine &l, sal_uInt16 version)
        {
            tools::GenericTypeSerializer aSerializer(stream);
            aSerializer.writeColor(l.GetColor());

            stream.WriteUInt16( l.GetOutWidth() )
                .WriteUInt16( l.GetInWidth() )
                .WriteUInt16( l.GetDistance() );

            if (version >= BORDER_LINE_WITH_STYLE_VERSION)
                stream.WriteUInt16( static_cast<sal_uInt16>(l.GetBorderLineStyle()) );

            return stream;
        }

        SvStream& Store(const SvxBoxItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion)
        {
            rStrm.WriteUInt16( rItem.GetSmallestDistance() );
            const ::editeng::SvxBorderLine* pLine[ 4 ];    // top, left, right, bottom
            pLine[ 0 ] = rItem.GetTop();
            pLine[ 1 ] = rItem.GetLeft();
            pLine[ 2 ] = rItem.GetRight();
            pLine[ 3 ] = rItem.GetBottom();

            for( int i = 0; i < 4; i++ )
            {
                const ::editeng::SvxBorderLine* l = pLine[ i ];
                if( l )
                {
                    rStrm.WriteSChar(i);
                    StoreBorderLine(rStrm, *l, BorderLineVersionFromBoxVersion(nItemVersion));
                }
            }
            sal_Int8 cLine = 4;
            const sal_uInt16 nTopDist(rItem.GetDistance(SvxBoxItemLine::TOP));
            const sal_uInt16 nLeftDist(rItem.GetDistance(SvxBoxItemLine::LEFT));
            const sal_uInt16 nRightDist(rItem.GetDistance(SvxBoxItemLine::RIGHT));
            const sal_uInt16 nBottomDist(rItem.GetDistance(SvxBoxItemLine::BOTTOM));

            if( nItemVersion >= BOX_4DISTS_VERSION &&
                !(nTopDist == nLeftDist &&
                nTopDist == nRightDist &&
                nTopDist == nBottomDist) )
            {
                cLine |= 0x10;
            }

            rStrm.WriteSChar( cLine );

            if( nItemVersion >= BOX_4DISTS_VERSION && (cLine & 0x10) != 0 )
            {
                rStrm.WriteUInt16( nTopDist )
                    .WriteUInt16( nLeftDist )
                    .WriteUInt16( nRightDist )
                    .WriteUInt16( nBottomDist );
            }

            return rStrm;
        }
    }

    namespace SvxLine
    {
        sal_uInt16 GetVersion(sal_uInt16)
        {
            return 0;
        }

        void Create(SvxLineItem& rItem, SvStream& rStrm, sal_uInt16)
        {
            short        nOutline, nInline, nDistance;
            Color        aColor;

            tools::GenericTypeSerializer aSerializer(rStrm);
            aSerializer.readColor(aColor);
            rStrm.ReadInt16( nOutline ).ReadInt16( nInline ).ReadInt16( nDistance );
            if( nOutline )
            {
                ::editeng::SvxBorderLine aLine( &aColor );
                aLine.GuessLinesWidths(SvxBorderLineStyle::NONE, nOutline, nInline, nDistance);
                rItem.SetLine( &aLine );
            }
        }

        SvStream& Store(const SvxLineItem& rItem, SvStream& rStrm, sal_uInt16)
        {
            const ::editeng::SvxBorderLine* pLine(rItem.GetLine());

            if(nullptr != pLine)
            {
                tools::GenericTypeSerializer aSerializer(rStrm);
                aSerializer.writeColor(pLine->GetColor());
                rStrm.WriteInt16( pLine->GetOutWidth() )
                    .WriteInt16( pLine->GetInWidth() )
                    .WriteInt16( pLine->GetDistance() );
            }
            else
            {
                tools::GenericTypeSerializer aSerializer(rStrm);
                aSerializer.writeColor(Color());
                rStrm.WriteInt16( 0 ).WriteInt16( 0 ).WriteInt16( 0 );
            }

            return rStrm;
        }
    }

    namespace SvxBrush
    {
        sal_uInt16 GetVersion(sal_uInt16)
        {
            return BRUSH_GRAPHIC_VERSION;
        }

        const sal_uInt16 LOAD_GRAPHIC = (sal_uInt16(0x0001));
        const sal_uInt16 LOAD_LINK = (sal_uInt16(0x0002));
        const sal_uInt16 LOAD_FILTER = (sal_uInt16(0x0004));

        void Create(SvxBrushItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion)
        {
            bool bTrans;
            Color aTempColor;
            Color aTempFillColor;
            sal_Int8 nStyle;

            rStrm.ReadCharAsBool( bTrans );
            tools::GenericTypeSerializer aSerializer(rStrm);
            aSerializer.readColor(aTempColor);
            aSerializer.readColor(aTempFillColor);
            rStrm.ReadSChar( nStyle );

            switch ( nStyle )
            {
                case 8: // BRUSH_25:
                {
                    sal_uInt32  nRed    = aTempColor.GetRed();
                    sal_uInt32  nGreen  = aTempColor.GetGreen();
                    sal_uInt32  nBlue   = aTempColor.GetBlue();
                    nRed   += static_cast<sal_uInt32>(aTempFillColor.GetRed())*2;
                    nGreen += static_cast<sal_uInt32>(aTempFillColor.GetGreen())*2;
                    nBlue  += static_cast<sal_uInt32>(aTempFillColor.GetBlue())*2;
                    rItem.SetColor(Color( static_cast<sal_Int8>(nRed/3), static_cast<sal_Int8>(nGreen/3), static_cast<sal_Int8>(nBlue/3) ));
                }
                break;

                case 9: // BRUSH_50:
                {
                    sal_uInt32  nRed    = aTempColor.GetRed();
                    sal_uInt32  nGreen  = aTempColor.GetGreen();
                    sal_uInt32  nBlue   = aTempColor.GetBlue();
                    nRed   += static_cast<sal_uInt32>(aTempFillColor.GetRed());
                    nGreen += static_cast<sal_uInt32>(aTempFillColor.GetGreen());
                    nBlue  += static_cast<sal_uInt32>(aTempFillColor.GetBlue());
                    rItem.SetColor(Color( static_cast<sal_Int8>(nRed/2), static_cast<sal_Int8>(nGreen/2), static_cast<sal_Int8>(nBlue/2) ));
                }
                break;

                case 10: // BRUSH_75:
                {
                    sal_uInt32  nRed    = aTempColor.GetRed()*2;
                    sal_uInt32  nGreen  = aTempColor.GetGreen()*2;
                    sal_uInt32  nBlue   = aTempColor.GetBlue()*2;
                    nRed   += static_cast<sal_uInt32>(aTempFillColor.GetRed());
                    nGreen += static_cast<sal_uInt32>(aTempFillColor.GetGreen());
                    nBlue  += static_cast<sal_uInt32>(aTempFillColor.GetBlue());
                    rItem.SetColor(Color( static_cast<sal_Int8>(nRed/3), static_cast<sal_Int8>(nGreen/3), static_cast<sal_Int8>(nBlue/3) ));
                }
                break;

                case 0: // BRUSH_NULL:
                    rItem.SetColor(COL_TRANSPARENT);
                break;

                default:
                    rItem.SetColor(aTempColor);
            }

            if ( nItemVersion < BRUSH_GRAPHIC_VERSION )
                return;

            sal_uInt16 nDoLoad = 0;
            sal_Int8 nPos;

            rStrm.ReadUInt16( nDoLoad );

            if ( nDoLoad & LOAD_GRAPHIC )
            {
                Graphic aGraphic;

                ReadGraphic( rStrm, aGraphic );
                rItem.SetGraphicObject(GraphicObject(aGraphic));

                if( SVSTREAM_FILEFORMAT_ERROR == rStrm.GetError() )
                {
                    rStrm.ResetError();
                    rStrm.SetError( ERRCODE_SVX_GRAPHIC_WRONG_FILEFORMAT.MakeWarning() );
                }
            }

            if ( nDoLoad & LOAD_LINK )
            {
                // UNICODE: rStrm >> aRel;
                OUString aRel = rStrm.ReadUniOrByteString(rStrm.GetStreamCharSet());

                // TODO/MBA: how can we get a BaseURL here?!
                OSL_FAIL("No BaseURL!");
                OUString aAbs = INetURLObject::GetAbsURL( "", aRel );
                DBG_ASSERT( !aAbs.isEmpty(), "Invalid URL!" );
                rItem.SetGraphicLink(aAbs);
            }

            if ( nDoLoad & LOAD_FILTER )
            {
                // UNICODE: rStrm >> maStrFilter;
                rItem.SetGraphicFilter(rStrm.ReadUniOrByteString(rStrm.GetStreamCharSet()));
            }

            rStrm.ReadSChar( nPos );

            rItem.SetGraphicPos(static_cast<SvxGraphicPosition>(nPos));
        }

        SvStream& Store(const SvxBrushItem& rItem, SvStream& rStrm, sal_uInt16)
        {
            rStrm.WriteBool( false );
            tools::GenericTypeSerializer aSerializer(rStrm);
            aSerializer.writeColor(rItem.GetColor());
            aSerializer.writeColor(rItem.GetColor());
            rStrm.WriteSChar( rItem.GetColor().IsTransparent() ? 0 : 1 ); //BRUSH_NULL : BRUSH_SOLID

            sal_uInt16 nDoLoad = 0;
            const GraphicObject* pGraphicObject(rItem.GetGraphicObject());

            if (nullptr != pGraphicObject && rItem.GetGraphicLink().isEmpty())
                nDoLoad |= LOAD_GRAPHIC;
            if ( !rItem.GetGraphicLink().isEmpty() )
                nDoLoad |= LOAD_LINK;
            if ( !rItem.GetGraphicFilter().isEmpty() )
                nDoLoad |= LOAD_FILTER;
            rStrm.WriteUInt16( nDoLoad );

            if (nullptr != pGraphicObject && rItem.GetGraphicLink().isEmpty())
                WriteGraphic(rStrm, pGraphicObject->GetGraphic());
            if ( !rItem.GetGraphicLink().isEmpty() )
            {
                OSL_FAIL("No BaseURL!");
                // TODO/MBA: how to get a BaseURL?!
                OUString aRel = INetURLObject::GetRelURL( "", rItem.GetGraphicLink() );
                // UNICODE: rStrm << aRel;
                rStrm.WriteUniOrByteString(aRel, rStrm.GetStreamCharSet());
            }
            if ( !rItem.GetGraphicFilter().isEmpty() )
            {
                // UNICODE: rStrm << rItem.GetGraphicFilter();
                rStrm.WriteUniOrByteString(rItem.GetGraphicFilter(), rStrm.GetStreamCharSet());
            }
            rStrm.WriteSChar( rItem.GetGraphicPos() );
            return rStrm;
        }
    }

    namespace SvxAdjust
    {
        sal_uInt16 GetVersion(sal_uInt16 nFileFormatVersion)
        {
            return (nFileFormatVersion == SOFFICE_FILEFORMAT_31)
                    ? 0 : ADJUST_LASTBLOCK_VERSION;
        }

        void Create(SvxAdjustItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion)
        {
            char eAdjustment;
            rStrm.ReadChar(eAdjustment);
            rItem.SetAdjust(static_cast<::SvxAdjust>(eAdjustment));

            if( nItemVersion >= ADJUST_LASTBLOCK_VERSION )
            {
                sal_Int8 nFlags;
                rStrm.ReadSChar( nFlags );
                rItem.SetAsFlags(nFlags);
            }
        }

        SvStream& Store(const SvxAdjustItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion)
        {
            rStrm.WriteChar( static_cast<char>(rItem.GetAdjust()) );
            if ( nItemVersion >= ADJUST_LASTBLOCK_VERSION )
            {
                const sal_Int8 nFlags(rItem.GetAsFlags());
                rStrm.WriteSChar( nFlags );
            }
            return rStrm;
        }
    }

    namespace SvxHorJustify
    {
        sal_uInt16 GetVersion(sal_uInt16)
        {
            return 0;
        }

        void Create(SvxHorJustifyItem& rItem, SvStream& rStrm, sal_uInt16)
        {
            sal_uInt16 nVal(0);
            rStrm.ReadUInt16( nVal );
            rItem.SetValue(static_cast<::SvxCellHorJustify>(nVal));
        }

        SvStream& Store(const SvxHorJustifyItem& rItem, SvStream& rStrm, sal_uInt16)
        {
            rStrm.WriteUInt16( static_cast<sal_uInt16>(rItem.GetValue()) );
            return rStrm;
        }
    }

    namespace SvxVerJustify
    {
        sal_uInt16 GetVersion(sal_uInt16)
        {
            return 0;
        }

        void Create(SvxVerJustifyItem& rItem, SvStream& rStrm, sal_uInt16)
        {
            sal_uInt16 nVal(0);
            rStrm.ReadUInt16( nVal );
            rItem.SetValue(static_cast<::SvxCellVerJustify>(nVal));
        }

        SvStream& Store(const SvxVerJustifyItem& rItem, SvStream& rStrm, sal_uInt16)
        {
            rStrm.WriteUInt16( static_cast<sal_uInt16>(rItem.GetValue()) );
            return rStrm;
        }
    }

    namespace SvxFrameDirection
    {
        sal_uInt16 GetVersion(sal_uInt16 nFileFormatVersion)
        {
            return SOFFICE_FILEFORMAT_50 > nFileFormatVersion ? USHRT_MAX : 0;
        }

        void Create(SvxFrameDirectionItem& rItem, SvStream& rStrm, sal_uInt16)
        {
            sal_uInt16 nVal(0);
            rStrm.ReadUInt16( nVal );
            rItem.SetValue(static_cast<::SvxFrameDirection>(nVal));
        }

        SvStream& Store(const SvxFrameDirectionItem& rItem, SvStream& rStrm, sal_uInt16)
        {
            rStrm.WriteUInt16( static_cast<sal_uInt16>(rItem.GetValue()) );
            return rStrm;
        }
    }

    namespace SvxFormatBreak
    {
        sal_uInt16 GetVersion(sal_uInt16 nFileFormatVersion)
        {
            DBG_ASSERT( SOFFICE_FILEFORMAT_31==nFileFormatVersion ||
                    SOFFICE_FILEFORMAT_40==nFileFormatVersion ||
                    SOFFICE_FILEFORMAT_50==nFileFormatVersion,
                    "SvxFormatBreakItem: Is there a new file format? ");
            return SOFFICE_FILEFORMAT_31==nFileFormatVersion ||
                SOFFICE_FILEFORMAT_40==nFileFormatVersion ? 0 : FMTBREAK_NOAUTO;
        }

        void Create(SvxFormatBreakItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion)
        {
            sal_Int8 eBreak, bDummy;
            rStrm.ReadSChar( eBreak );
            if( FMTBREAK_NOAUTO > nItemVersion )
                rStrm.ReadSChar( bDummy );
            rItem.SetValue(static_cast<::SvxBreak>(eBreak));
        }

        SvStream& Store(const SvxFormatBreakItem& rItem, SvStream& rStrm, sal_uInt16 nItemVersion)
        {
            rStrm.WriteSChar( rItem.GetEnumValue() );
            if( FMTBREAK_NOAUTO > nItemVersion )
                rStrm.WriteSChar( 0x01 );
            return rStrm;
        }
    }

    namespace SvxFormatKeep
    {
        sal_uInt16 GetVersion(sal_uInt16)
        {
            return 0;
        }

        void Create(SvxFormatKeepItem& rItem, SvStream& rStrm, sal_uInt16)
        {
            // derived from SfxBoolItem, but that uses
            //    rStream.ReadCharAsBool( tmp );
            sal_Int8 bIsKeep;
            rStrm.ReadSChar( bIsKeep );
            rItem.SetValue(static_cast<bool>(bIsKeep));
        }

        SvStream& Store(const SvxFormatKeepItem& rItem, SvStream& rStrm, sal_uInt16)
        {
            // derived from SfxBoolItem, but that uses
            //    rStream.WriteBool( m_bValue ); // not bool for serialization!
            rStrm.WriteSChar( static_cast<sal_Int8>(rItem.GetValue()) );
            return rStrm;
        }
    }

    namespace SvxShadow
    {
        sal_uInt16 GetVersion(sal_uInt16)
        {
            return 0;
        }

        void Create(SvxShadowItem& rItem, SvStream& rStrm, sal_uInt16)
        {
            sal_Int8 cLoc;
            sal_uInt16 _nWidth;
            bool bTrans;
            Color aColor;
            Color aFillColor;
            sal_Int8 nStyle;
            rStrm.ReadSChar( cLoc ).ReadUInt16( _nWidth ).ReadCharAsBool( bTrans );
            tools::GenericTypeSerializer aSerializer(rStrm);
            aSerializer.readColor(aColor);
            aSerializer.readColor(aFillColor);
            rStrm.ReadSChar(nStyle);
            aColor.SetTransparency(bTrans ? 0xff : 0);

            rItem.SetLocation(static_cast<SvxShadowLocation>(cLoc));
            rItem.SetWidth(_nWidth);
            rItem.SetColor(aColor);
        }

        SvStream& Store(const SvxShadowItem& rItem, SvStream& rStrm, sal_uInt16)
        {
            rStrm.WriteSChar( static_cast<sal_uInt8>(rItem.GetLocation()) )
                .WriteUInt16( rItem.GetWidth() )
                .WriteBool( rItem.GetColor().IsTransparent() );
            tools::GenericTypeSerializer aSerializer(rStrm);
            aSerializer.writeColor(rItem.GetColor());
            aSerializer.writeColor(rItem.GetColor());
            rStrm.WriteSChar( rItem.GetColor().IsTransparent() ? 0 : 1 ); //BRUSH_NULL : BRUSH_SOLID
            return rStrm;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
