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

#include <algorithm>

#include <editeng/numitem.hxx>

#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <editeng/brushitem.hxx>
#include <vcl/font.hxx>
#include <vcl/settings.hxx>
#include <editeng/editids.hrc>
#include <editeng/numdef.hxx>
#include <editeng/eeitem.hxx>
#include <vcl/graph.hxx>
#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <editeng/unolingu.hxx>
#include <com/sun/star/text/XNumberingFormatter.hpp>
#include <com/sun/star/text/DefaultNumberingProvider.hpp>
#include <com/sun/star/text/XDefaultNumberingProvider.hpp>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <comphelper/fileformat.h>
#include <comphelper/processfactory.hxx>
#include <tools/mapunit.hxx>
#include <tools/stream.hxx>
#include <tools/debug.hxx>
#include <unotools/configmgr.hxx>
#include <libxml/xmlwriter.h>
#include <editeng/unonrule.hxx>
#include <sal/log.hxx>
#include <i18nlangtag/languagetag.hxx>

#define DEF_WRITER_LSPACE   500     //Standard Indentation
#define DEF_DRAW_LSPACE     800     //Standard Indentation

#define NUMITEM_VERSION_03        0x03
#define NUMITEM_VERSION_04        0x04

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::style;

sal_Int32 SvxNumberType::nRefCount = 0;
css::uno::Reference<css::text::XNumberingFormatter> SvxNumberType::xFormatter;
static void lcl_getFormatter(css::uno::Reference<css::text::XNumberingFormatter>& _xFormatter)
{
    if(!_xFormatter.is())
       {
        try
        {
            Reference<XComponentContext>         xContext( ::comphelper::getProcessComponentContext() );
            Reference<XDefaultNumberingProvider> xRet = text::DefaultNumberingProvider::create(xContext);
            _xFormatter.set(xRet, UNO_QUERY);
        }
        catch(const Exception&)
        {
            SAL_WARN("editeng", "service missing: \"com.sun.star.text.DefaultNumberingProvider\"");
        }
    }
}

SvxNumberType::SvxNumberType(SvxNumType nType) :
    nNumType(nType),
    bShowSymbol(true)
{
    nRefCount++;
}

SvxNumberType::SvxNumberType(const SvxNumberType& rType) :
    nNumType(rType.nNumType),
    bShowSymbol(rType.bShowSymbol)
{
    nRefCount++;
}

SvxNumberType::~SvxNumberType()
{
    if(!--nRefCount)
        xFormatter = nullptr;
}

OUString SvxNumberType::GetNumStr( sal_Int32 nNo ) const
{
    LanguageTag aLang = utl::ConfigManager::IsFuzzing() ?
        LanguageTag("en-US") :
        Application::GetSettings().GetLanguageTag();
    return GetNumStr( nNo, aLang.getLocale() );
}

OUString SvxNumberType::GetNumStr( sal_Int32 nNo, const css::lang::Locale& rLocale ) const
{
    lcl_getFormatter(xFormatter);
    if(!xFormatter.is())
        return OUString();

    if(bShowSymbol)
    {
        switch(nNumType)
        {
            case NumberingType::CHAR_SPECIAL:
            case NumberingType::BITMAP:
            break;
            default:
                {
                    // '0' allowed for ARABIC numberings
                    if(NumberingType::ARABIC == nNumType && 0 == nNo )
                        return OUString('0');
                    else
                    {
                        Sequence< PropertyValue > aProperties(2);
                        PropertyValue* pValues = aProperties.getArray();
                        pValues[0].Name = "NumberingType";
                        pValues[0].Value <<= static_cast<sal_uInt16>(nNumType);
                        pValues[1].Name = "Value";
                        pValues[1].Value <<= nNo;

                        try
                        {
                            return xFormatter->makeNumberingString( aProperties, rLocale );
                        }
                        catch(const Exception&)
                        {
                        }
                    }
                }
        }
    }
    return OUString();
}

SvxNumberFormat::SvxNumberFormat( SvxNumType eType )
    : SvxNumberType(eType),
      eNumAdjust(SvxAdjust::Left),
      nInclUpperLevels(0),
      nStart(1),
      cBullet(SVX_DEF_BULLET),
      nBulletRelSize(100),
      nBulletColor(COL_BLACK),
      mePositionAndSpaceMode( LABEL_WIDTH_AND_POSITION ),
      nFirstLineOffset(0),
      nAbsLSpace(0),
      nCharTextDistance(0),
      meLabelFollowedBy( LISTTAB ),
      mnListtabPos( 0 ),
      mnFirstLineIndent( 0 ),
      mnIndentAt( 0 ),
      eVertOrient(text::VertOrientation::NONE)
{
}

SvxNumberFormat::SvxNumberFormat(const SvxNumberFormat& rFormat) :
    SvxNumberType(rFormat),
    mePositionAndSpaceMode( rFormat.mePositionAndSpaceMode )
{
    *this = rFormat;
}

SvxNumberFormat::SvxNumberFormat( SvStream &rStream )
    : nStart(0)
    , nBulletRelSize(100)
    , nFirstLineOffset(0)
    , nAbsLSpace(0)
    , nCharTextDistance(0)
{
    sal_uInt16 nTmp16(0);
    sal_Int32  nTmp32(0);
    rStream.ReadUInt16( nTmp16 ); // Version number

    rStream.ReadUInt16( nTmp16 ); SetNumberingType( static_cast<SvxNumType>(nTmp16) );
    rStream.ReadUInt16( nTmp16 ); eNumAdjust = static_cast<SvxAdjust>(nTmp16);
    rStream.ReadUInt16( nTmp16 ); nInclUpperLevels = nTmp16;
    rStream.ReadUInt16( nStart );
    rStream.ReadUInt16( nTmp16 ); cBullet = static_cast<sal_Unicode>(nTmp16);

    sal_Int16 temp = 0;
    rStream.ReadInt16( temp );
    nFirstLineOffset = temp;
    temp = 0;
    rStream.ReadInt16( temp );
    nAbsLSpace = temp;
    rStream.SeekRel(2); //skip old now unused nLSpace;

    rStream.ReadInt16( nCharTextDistance );

    sPrefix = rStream.ReadUniOrByteString( rStream.GetStreamCharSet() );
    sSuffix = rStream.ReadUniOrByteString( rStream.GetStreamCharSet() );
    sCharStyleName = rStream.ReadUniOrByteString( rStream.GetStreamCharSet() );

    sal_uInt16 hasGraphicBrush = 0;
    rStream.ReadUInt16( hasGraphicBrush );
    if ( hasGraphicBrush )
    {
        std::unique_ptr<SvxBrushItem> pTmp( new SvxBrushItem( SID_ATTR_BRUSH ) );
        pGraphicBrush.reset( static_cast<SvxBrushItem*>(pTmp->Create( rStream, BRUSH_GRAPHIC_VERSION )) );
    }
    else pGraphicBrush = nullptr;
    rStream.ReadUInt16( nTmp16 ); eVertOrient = nTmp16;

    sal_uInt16 hasBulletFont = 0;
    rStream.ReadUInt16( hasBulletFont );
    if ( hasBulletFont )
    {
        pBulletFont.reset( new vcl::Font() );
        ReadFont( rStream, *pBulletFont );
    }
    else pBulletFont = nullptr;
    ReadPair( rStream, aGraphicSize );

    ReadColor( rStream, nBulletColor );
    rStream.ReadUInt16( nBulletRelSize );
    rStream.ReadUInt16( nTmp16 ); SetShowSymbol( nTmp16 != 0 );

    rStream.ReadUInt16( nTmp16 ); mePositionAndSpaceMode = static_cast<SvxNumPositionAndSpaceMode>(nTmp16);
    rStream.ReadUInt16( nTmp16 ); meLabelFollowedBy = static_cast<LabelFollowedBy>(nTmp16);
    rStream.ReadInt32( nTmp32 ); mnListtabPos = nTmp32;
    rStream.ReadInt32( nTmp32 ); mnFirstLineIndent = nTmp32;
    rStream.ReadInt32( nTmp32 ); mnIndentAt = nTmp32;
}

SvxNumberFormat::~SvxNumberFormat()
{
}

void SvxNumberFormat::Store(SvStream &rStream, FontToSubsFontConverter pConverter)
{
    if(pConverter && pBulletFont)
    {
        cBullet = ConvertFontToSubsFontChar(pConverter, cBullet);
        OUString sFontName = GetFontToSubsFontName(pConverter);
        pBulletFont->SetFamilyName(sFontName);
    }

    rStream.WriteUInt16( NUMITEM_VERSION_04 );

    rStream.WriteUInt16( GetNumberingType() );
    rStream.WriteUInt16( static_cast<sal_uInt16>(eNumAdjust) );
    rStream.WriteUInt16( nInclUpperLevels );
    rStream.WriteUInt16( nStart );
    rStream.WriteUInt16( cBullet );

    rStream.WriteInt16(
        sal_Int16(std::clamp<sal_Int32>(nFirstLineOffset, SAL_MIN_INT16, SAL_MAX_INT16)) );
        //TODO: better way to handle out-of-bounds value?
    rStream.WriteInt16(
        sal_Int16(std::clamp<sal_Int32>(nAbsLSpace, SAL_MIN_INT16, SAL_MAX_INT16)) );
        //TODO: better way to handle out-of-bounds value?
    rStream.WriteInt16( 0 ); // write a dummy for old now unused nLSpace

    rStream.WriteInt16( nCharTextDistance );
    rtl_TextEncoding eEnc = osl_getThreadTextEncoding();
    rStream.WriteUniOrByteString(sPrefix, eEnc);
    rStream.WriteUniOrByteString(sSuffix, eEnc);
    rStream.WriteUniOrByteString(sCharStyleName, eEnc);
    if(pGraphicBrush)
    {
        rStream.WriteUInt16( 1 );

        // in SD or SI force bullet itself to be stored,
        // for that purpose throw away link when link and graphic
        // are present, so Brush save is forced
        if(!pGraphicBrush->GetGraphicLink().isEmpty() && pGraphicBrush->GetGraphic())
        {
            pGraphicBrush->SetGraphicLink("");
        }

        pGraphicBrush->Store(rStream, BRUSH_GRAPHIC_VERSION);
    }
    else
        rStream.WriteUInt16( 0 );

    rStream.WriteUInt16( eVertOrient );
    if(pBulletFont)
    {
        rStream.WriteUInt16( 1 );
        WriteFont( rStream, *pBulletFont );
    }
    else
        rStream.WriteUInt16( 0 );
    WritePair( rStream, aGraphicSize );

    Color nTempColor = nBulletColor;
    if(COL_AUTO == nBulletColor)
        nTempColor = COL_BLACK;
    WriteColor( rStream, nTempColor );
    rStream.WriteUInt16( nBulletRelSize );
    rStream.WriteUInt16( sal_uInt16(IsShowSymbol()) );

    rStream.WriteUInt16( mePositionAndSpaceMode );
    rStream.WriteUInt16( meLabelFollowedBy );
    rStream.WriteInt32( mnListtabPos );
    rStream.WriteInt32( mnFirstLineIndent );
    rStream.WriteInt32( mnIndentAt );
}

SvxNumberFormat& SvxNumberFormat::operator=( const SvxNumberFormat& rFormat )
{
    if (& rFormat == this) { return *this; }

    SvxNumberType::SetNumberingType(rFormat.GetNumberingType());
    eNumAdjust          = rFormat.eNumAdjust ;
    nInclUpperLevels    = rFormat.nInclUpperLevels ;
    nStart              = rFormat.nStart ;
    cBullet             = rFormat.cBullet ;
    mePositionAndSpaceMode = rFormat.mePositionAndSpaceMode;
    nFirstLineOffset    = rFormat.nFirstLineOffset;
    nAbsLSpace          = rFormat.nAbsLSpace ;
    nCharTextDistance   = rFormat.nCharTextDistance ;
    meLabelFollowedBy = rFormat.meLabelFollowedBy;
    mnListtabPos = rFormat.mnListtabPos;
    mnFirstLineIndent = rFormat.mnFirstLineIndent;
    mnIndentAt = rFormat.mnIndentAt;
    eVertOrient         = rFormat.eVertOrient ;
    sPrefix             = rFormat.sPrefix     ;
    sSuffix             = rFormat.sSuffix     ;
    aGraphicSize        = rFormat.aGraphicSize  ;
    nBulletColor        = rFormat.nBulletColor   ;
    nBulletRelSize      = rFormat.nBulletRelSize;
    SetShowSymbol(rFormat.IsShowSymbol());
    sCharStyleName      = rFormat.sCharStyleName;
    pGraphicBrush.reset();
    if(rFormat.pGraphicBrush)
    {
        pGraphicBrush.reset( new SvxBrushItem(*rFormat.pGraphicBrush) );
    }
    pBulletFont.reset();
    if(rFormat.pBulletFont)
        pBulletFont.reset( new vcl::Font(*rFormat.pBulletFont) );
    return *this;
}

bool  SvxNumberFormat::operator==( const SvxNumberFormat& rFormat) const
{
    if( GetNumberingType()  != rFormat.GetNumberingType() ||
        eNumAdjust          != rFormat.eNumAdjust ||
        nInclUpperLevels    != rFormat.nInclUpperLevels ||
        nStart              != rFormat.nStart ||
        cBullet             != rFormat.cBullet ||
        mePositionAndSpaceMode != rFormat.mePositionAndSpaceMode ||
        nFirstLineOffset    != rFormat.nFirstLineOffset ||
        nAbsLSpace          != rFormat.nAbsLSpace ||
        nCharTextDistance   != rFormat.nCharTextDistance ||
        meLabelFollowedBy != rFormat.meLabelFollowedBy ||
        mnListtabPos != rFormat.mnListtabPos ||
        mnFirstLineIndent != rFormat.mnFirstLineIndent ||
        mnIndentAt != rFormat.mnIndentAt ||
        eVertOrient         != rFormat.eVertOrient ||
        sPrefix             != rFormat.sPrefix     ||
        sSuffix             != rFormat.sSuffix     ||
        aGraphicSize        != rFormat.aGraphicSize  ||
        nBulletColor        != rFormat.nBulletColor   ||
        nBulletRelSize      != rFormat.nBulletRelSize ||
        IsShowSymbol()      != rFormat.IsShowSymbol() ||
        sCharStyleName      != rFormat.sCharStyleName
        )
        return false;
    if (
        (pGraphicBrush && !rFormat.pGraphicBrush) ||
        (!pGraphicBrush && rFormat.pGraphicBrush) ||
        (pGraphicBrush && *pGraphicBrush != *rFormat.pGraphicBrush)
       )
    {
        return false;
    }
    if (
        (pBulletFont && !rFormat.pBulletFont) ||
        (!pBulletFont && rFormat.pBulletFont) ||
        (pBulletFont && *pBulletFont != *rFormat.pBulletFont)
       )
    {
        return false;
    }
    return true;
}

void SvxNumberFormat::SetGraphicBrush( const SvxBrushItem* pBrushItem,
                    const Size* pSize, const sal_Int16* pOrient)
{
    if(!pBrushItem)
    {
        pGraphicBrush.reset();
    }
    else if ( !pGraphicBrush || (*pBrushItem != *pGraphicBrush) )
    {
        pGraphicBrush.reset( static_cast<SvxBrushItem*>(pBrushItem->Clone()) );
   }

    if(pOrient)
        eVertOrient = *pOrient;
    else
        eVertOrient = text::VertOrientation::NONE;
    if(pSize)
        aGraphicSize = *pSize;
    else
    {
        aGraphicSize.setWidth(0);
        aGraphicSize.setHeight(0);
    }
}

void SvxNumberFormat::SetGraphic( const OUString& rName )
{
    if( pGraphicBrush && pGraphicBrush->GetGraphicLink() == rName )
        return ;

    pGraphicBrush.reset( new SvxBrushItem( rName, "", GPOS_AREA, 0 ) );
    if( eVertOrient == text::VertOrientation::NONE )
        eVertOrient = text::VertOrientation::TOP;

    aGraphicSize.setWidth(0);
    aGraphicSize.setHeight(0);
}

sal_Int16    SvxNumberFormat::GetVertOrient() const
{
    return eVertOrient;
}

void SvxNumberFormat::SetBulletFont(const vcl::Font* pFont)
{
    pBulletFont.reset( pFont ? new vcl::Font(*pFont): nullptr );
}

void SvxNumberFormat::SetPositionAndSpaceMode( SvxNumPositionAndSpaceMode ePositionAndSpaceMode )
{
    mePositionAndSpaceMode = ePositionAndSpaceMode;
}

sal_Int32 SvxNumberFormat::GetAbsLSpace() const
{
    return mePositionAndSpaceMode == LABEL_WIDTH_AND_POSITION
           ? nAbsLSpace
           : static_cast<sal_Int32>( GetFirstLineIndent() + GetIndentAt() );
}
sal_Int32 SvxNumberFormat::GetFirstLineOffset() const
{
    return mePositionAndSpaceMode == LABEL_WIDTH_AND_POSITION
           ? nFirstLineOffset
           : static_cast<sal_Int32>( GetFirstLineIndent() );
}
short SvxNumberFormat::GetCharTextDistance() const
{
    return mePositionAndSpaceMode == LABEL_WIDTH_AND_POSITION ? nCharTextDistance : 0;
}

void SvxNumberFormat::SetLabelFollowedBy( const LabelFollowedBy eLabelFollowedBy )
{
    meLabelFollowedBy = eLabelFollowedBy;
}
void SvxNumberFormat::SetListtabPos( const long nListtabPos )
{
    mnListtabPos = nListtabPos;
}
void SvxNumberFormat::SetFirstLineIndent( const long nFirstLineIndent )
{
    mnFirstLineIndent = nFirstLineIndent;
}
void SvxNumberFormat::SetIndentAt( const long nIndentAt )
{
    mnIndentAt = nIndentAt;
}

Size SvxNumberFormat::GetGraphicSizeMM100(const Graphic* pGraphic)
{
    const MapMode aMapMM100( MapUnit::Map100thMM );
    const Size& rSize = pGraphic->GetPrefSize();
    Size aRetSize;
    if ( pGraphic->GetPrefMapMode().GetMapUnit() == MapUnit::MapPixel )
    {
        OutputDevice* pOutDev = Application::GetDefaultDevice();
        MapMode aOldMap( pOutDev->GetMapMode() );
        pOutDev->SetMapMode( aMapMM100 );
        aRetSize = pOutDev->PixelToLogic( rSize );
        pOutDev->SetMapMode( aOldMap );
    }
    else
        aRetSize = OutputDevice::LogicToLogic( rSize, pGraphic->GetPrefMapMode(), aMapMM100 );
    return aRetSize;
}

OUString SvxNumberFormat::CreateRomanString( sal_uLong nNo, bool bUpper )
{
    nNo %= 4000;            // more can not be displayed
//      i, ii, iii, iv, v, vi, vii, vii, viii, ix
//                          (Dummy),1000,500,100,50,10,5,1
    const char *cRomanArr = bUpper
                        ? "MDCLXVI--"   // +2 Dummy entries!
                        : "mdclxvi--";  // +2 Dummy entries!

    OUStringBuffer sRet;
    sal_uInt16 nMask = 1000;
    while( nMask )
    {
        sal_uInt8 nNumber = sal_uInt8(nNo / nMask);
        sal_uInt8 nDiff = 1;
        nNo %= nMask;

        if( 5 < nNumber )
        {
            if( nNumber < 9 )
                sRet.append(*(cRomanArr-1));
            ++nDiff;
            nNumber -= 5;
        }
        switch( nNumber )
        {
        case 3:     { sRet.append(*cRomanArr); [[fallthrough]]; }
        case 2:     { sRet.append(*cRomanArr); [[fallthrough]]; }
        case 1:     { sRet.append(*cRomanArr); }
                    break;

        case 4:     {
                        sRet.append(*cRomanArr);
                        sRet.append(*(cRomanArr-nDiff));
                    }
                    break;
        case 5:     { sRet.append(*(cRomanArr-nDiff)); }
                    break;
        }

        nMask /= 10;            // for the next decade
        cRomanArr += 2;
    }
    return sRet.makeStringAndClear();
}

OUString SvxNumberFormat::GetCharFormatName()const
{
    return sCharStyleName;
}

sal_Int32 SvxNumRule::nRefCount = 0;
static SvxNumberFormat* pStdNumFmt = nullptr;
static SvxNumberFormat* pStdOutlineNumFmt = nullptr;
SvxNumRule::SvxNumRule( SvxNumRuleFlags nFeatures,
                        sal_uInt16 nLevels,
                        bool bCont,
                        SvxNumRuleType eType,
                        SvxNumberFormat::SvxNumPositionAndSpaceMode
                                    eDefaultNumberFormatPositionAndSpaceMode )
    : nLevelCount(nLevels),
      nFeatureFlags(nFeatures),
      eNumberingType(eType),
      bContinuousNumbering(bCont)
{
    ++nRefCount;
    for(sal_uInt16 i = 0; i < SVX_MAX_NUM; i++)
    {
        if(i < nLevels)
        {
            aFmts[i].reset( new SvxNumberFormat(SVX_NUM_CHARS_UPPER_LETTER) );
            // It is a distinction between writer and draw
            if(nFeatures & SvxNumRuleFlags::CONTINUOUS)
            {
                if ( eDefaultNumberFormatPositionAndSpaceMode ==
                                    SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
                {
                    aFmts[i]->SetAbsLSpace( convertMm100ToTwip(DEF_WRITER_LSPACE * (i+1)) );
                    aFmts[i]->SetFirstLineOffset(convertMm100ToTwip(-DEF_WRITER_LSPACE));
                }
                else if ( eDefaultNumberFormatPositionAndSpaceMode ==
                                            SvxNumberFormat::LABEL_ALIGNMENT )
                {
                    // first line indent of general numbering in inch: -0,25 inch
                    const long cFirstLineIndent = -1440/4;
                    // indent values of general numbering in inch:
                    //  0,5         0,75        1,0         1,25        1,5
                    //  1,75        2,0         2,25        2,5         2,75
                    const long cIndentAt = 1440/4;
                    aFmts[i]->SetPositionAndSpaceMode( SvxNumberFormat::LABEL_ALIGNMENT );
                    aFmts[i]->SetLabelFollowedBy( SvxNumberFormat::LISTTAB );
                    aFmts[i]->SetListtabPos( cIndentAt * (i+2) );
                    aFmts[i]->SetFirstLineIndent( cFirstLineIndent );
                    aFmts[i]->SetIndentAt( cIndentAt * (i+2) );
                }
            }
            else
            {
                aFmts[i]->SetAbsLSpace( DEF_DRAW_LSPACE * i );
            }
        }
        else
            aFmts[i] = nullptr;
        aFmtsSet[i] = false;
    }
}

SvxNumRule::SvxNumRule(const SvxNumRule& rCopy)
{
    ++nRefCount;
    nLevelCount          = rCopy.nLevelCount         ;
    nFeatureFlags        = rCopy.nFeatureFlags       ;
    bContinuousNumbering = rCopy.bContinuousNumbering;
    eNumberingType       = rCopy.eNumberingType;
    memset( aFmts, 0, sizeof( aFmts ));
    for(sal_uInt16 i = 0; i < SVX_MAX_NUM; i++)
    {
        if(rCopy.aFmts[i])
            aFmts[i].reset( new SvxNumberFormat(*rCopy.aFmts[i]) );
        else
            aFmts[i].reset();
        aFmtsSet[i] = rCopy.aFmtsSet[i];
    }
}

SvxNumRule::SvxNumRule( SvStream &rStream )
    : nLevelCount(0)
{
    sal_uInt16 nTmp16(0);
    rStream.ReadUInt16( nTmp16 ); // NUM_ITEM_VERSION
    rStream.ReadUInt16( nLevelCount );

    // first nFeatureFlags of old Versions
    rStream.ReadUInt16( nTmp16 ); nFeatureFlags = static_cast<SvxNumRuleFlags>(nTmp16);
    rStream.ReadUInt16( nTmp16 ); bContinuousNumbering = nTmp16;
    rStream.ReadUInt16( nTmp16 ); eNumberingType = static_cast<SvxNumRuleType>(nTmp16);

    for (sal_uInt16 i = 0; i < SVX_MAX_NUM; i++)
    {
        rStream.ReadUInt16( nTmp16 );
        bool hasNumberingFormat = nTmp16 & 1;
        aFmtsSet[i] = nTmp16 & 2; // fdo#68648 reset flag
        if ( hasNumberingFormat ){
            aFmts[i].reset( new SvxNumberFormat( rStream ) );
        }
        else
        {
            aFmts[i].reset();
            aFmtsSet[i] = false; // actually only false is valid
        }
    }
    //second nFeatureFlags for new versions
    rStream.ReadUInt16( nTmp16 ); nFeatureFlags = static_cast<SvxNumRuleFlags>(nTmp16);
}

void SvxNumRule::Store( SvStream &rStream )
{
    rStream.WriteUInt16( NUMITEM_VERSION_03 );
    rStream.WriteUInt16( nLevelCount );
    //first save of nFeatureFlags for old versions
    rStream.WriteUInt16( static_cast<sal_uInt16>(nFeatureFlags) );
    rStream.WriteUInt16( sal_uInt16(bContinuousNumbering) );
    rStream.WriteUInt16( static_cast<sal_uInt16>(eNumberingType) );

    FontToSubsFontConverter pConverter = nullptr;
    bool bConvertBulletFont = ( rStream.GetVersion() <= SOFFICE_FILEFORMAT_50 ) && ( rStream.GetVersion() );
    for(sal_uInt16 i = 0; i < SVX_MAX_NUM; i++)
    {
        sal_uInt16 nSetFlag(aFmtsSet[i] ? 2 : 0); // fdo#68648 store that too
        if(aFmts[i])
        {
            rStream.WriteUInt16( 1 | nSetFlag );
            if(bConvertBulletFont && aFmts[i]->GetBulletFont())
            {
                if(!pConverter)
                    pConverter =
                        CreateFontToSubsFontConverter(aFmts[i]->GetBulletFont()->GetFamilyName(),
                                    FontToSubsFontFlags::EXPORT);
            }
            aFmts[i]->Store(rStream, pConverter);
        }
        else
            rStream.WriteUInt16( 0 | nSetFlag );
    }
    //second save of nFeatureFlags for new versions
    rStream.WriteUInt16( static_cast<sal_uInt16>(nFeatureFlags) );
}

void SvxNumRule::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("SvxNumRule"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("levelCount"), BAD_CAST(OUString::number(nLevelCount).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("continuousNumbering"), BAD_CAST(OUString::boolean(bContinuousNumbering).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("numberingType"), BAD_CAST(OUString::number(static_cast<int>(eNumberingType)).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("featureFlags"), BAD_CAST(OUString::number(static_cast<int>(nFeatureFlags)).getStr()));
    for(sal_uInt16 i = 0; i < SVX_MAX_NUM; i++)
    {
        if(aFmts[i])
        {
            xmlTextWriterStartElement(pWriter, BAD_CAST("aFmts"));
            xmlTextWriterWriteAttribute(pWriter, BAD_CAST("i"), BAD_CAST(OUString::number(i).getStr()));
            xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("ptr"), "%p", aFmts[i].get());
            xmlTextWriterEndElement(pWriter);
        }
    }
    xmlTextWriterEndElement(pWriter);
}


SvxNumRule::~SvxNumRule()
{
    if(!--nRefCount)
    {
        DELETEZ(pStdNumFmt);
        DELETEZ(pStdOutlineNumFmt);
    }
}

SvxNumRule& SvxNumRule::operator=( const SvxNumRule& rCopy )
{
    if (this != &rCopy)
    {
        nLevelCount          = rCopy.nLevelCount;
        nFeatureFlags        = rCopy.nFeatureFlags;
        bContinuousNumbering = rCopy.bContinuousNumbering;
        eNumberingType       = rCopy.eNumberingType;
        for(sal_uInt16 i = 0; i < SVX_MAX_NUM; i++)
        {
            if(rCopy.aFmts[i])
                aFmts[i].reset( new SvxNumberFormat(*rCopy.aFmts[i]) );
            else
                aFmts[i].reset();
            aFmtsSet[i] = rCopy.aFmtsSet[i];
        }
    }
    return *this;
}

bool SvxNumRule::operator==( const SvxNumRule& rCopy) const
{
    if(nLevelCount != rCopy.nLevelCount ||
        nFeatureFlags != rCopy.nFeatureFlags ||
            bContinuousNumbering != rCopy.bContinuousNumbering ||
                eNumberingType != rCopy.eNumberingType)
            return false;
    for(sal_uInt16 i = 0; i < nLevelCount; i++)
    {
        if (
            (aFmtsSet[i] != rCopy.aFmtsSet[i]) ||
            (!aFmts[i] && rCopy.aFmts[i]) ||
            (aFmts[i] && !rCopy.aFmts[i]) ||
            (aFmts[i] && *aFmts[i] !=  *rCopy.aFmts[i])
           )
        {
            return false;
        }
    }
    return true;
}

const SvxNumberFormat*  SvxNumRule::Get(sal_uInt16 nLevel)const
{
    DBG_ASSERT(nLevel < SVX_MAX_NUM, "Wrong Level" );
    if( nLevel < SVX_MAX_NUM )
        return aFmtsSet[nLevel] ? aFmts[nLevel].get() : nullptr;
    else
        return nullptr;
}

const SvxNumberFormat&  SvxNumRule::GetLevel(sal_uInt16 nLevel)const
{
    if(!pStdNumFmt)
    {
        pStdNumFmt = new SvxNumberFormat(SVX_NUM_ARABIC);
        pStdOutlineNumFmt = new SvxNumberFormat(SVX_NUM_NUMBER_NONE);
    }

    DBG_ASSERT(nLevel < SVX_MAX_NUM, "Wrong Level" );

    return ( ( nLevel < SVX_MAX_NUM ) && aFmts[nLevel] ) ?
            *aFmts[nLevel] :  eNumberingType == SvxNumRuleType::NUMBERING ?
                                                        *pStdNumFmt : *pStdOutlineNumFmt;
}

void SvxNumRule::SetLevel( sal_uInt16 i, const SvxNumberFormat& rNumFmt, bool bIsValid )
{
    DBG_ASSERT(i < SVX_MAX_NUM, "Wrong Level" );

    if( i < SVX_MAX_NUM )
    {
        bool bReplace = !aFmtsSet[i];
        if (!bReplace)
        {
            const SvxNumberFormat *pFmt = Get(i);
            bReplace = pFmt == nullptr || rNumFmt != *pFmt;
        }

        if (bReplace)
        {
            aFmts[i].reset( new SvxNumberFormat(rNumFmt) );
            aFmtsSet[i] = bIsValid;
        }
    }
}

void SvxNumRule::SetLevel(sal_uInt16 nLevel, const SvxNumberFormat* pFmt)
{
    DBG_ASSERT(nLevel < SVX_MAX_NUM, "Wrong Level" );

    if( nLevel < SVX_MAX_NUM )
    {
        aFmtsSet[nLevel] = nullptr != pFmt;
        if(pFmt)
            SetLevel(nLevel, *pFmt);
        else
        {
            aFmts[nLevel].reset();
        }
    }
}

OUString SvxNumRule::MakeNumString( const SvxNodeNum& rNum ) const
{
    OUStringBuffer aStr;
    if( SVX_NO_NUM > rNum.GetLevel() && !( SVX_NO_NUMLEVEL & rNum.GetLevel() ) )
    {
        const SvxNumberFormat& rMyNFmt = GetLevel( rNum.GetLevel() );
        aStr.append(rMyNFmt.GetPrefix());
        if( SVX_NUM_NUMBER_NONE != rMyNFmt.GetNumberingType() )
        {
            sal_uInt8 i = rNum.GetLevel();

            if( !IsContinuousNumbering() &&
                1 < rMyNFmt.GetIncludeUpperLevels() )       // only on own level?
            {
                sal_uInt8 n = rMyNFmt.GetIncludeUpperLevels();
                if( 1 < n )
                {
                    if( i+1 >= n )
                        i -= n - 1;
                    else
                        i = 0;
                }
            }

            for( ; i <= rNum.GetLevel(); ++i )
            {
                const SvxNumberFormat& rNFmt = GetLevel( i );
                if( SVX_NUM_NUMBER_NONE == rNFmt.GetNumberingType() )
                {
                    continue;
                }

                bool bDot = true;
                if( rNum.GetLevelVal()[ i ] )
                {
                    if(SVX_NUM_BITMAP != rNFmt.GetNumberingType())
                    {
                        const LanguageTag& rLang = Application::GetSettings().GetLanguageTag();
                        aStr.append(rNFmt.GetNumStr( rNum.GetLevelVal()[ i ], rLang.getLocale()  ));
                    }
                    else
                        bDot = false;
                }
                else
                    aStr.append("0");       // all 0-levels are a 0
                if( i != rNum.GetLevel() && bDot)
                    aStr.append(".");
            }
        }

        aStr.append(rMyNFmt.GetSuffix());
    }
    return aStr.makeStringAndClear();
}

// changes linked to embedded bitmaps
void SvxNumRule::UnLinkGraphics()
{
    for(sal_uInt16 i = 0; i < GetLevelCount(); i++)
    {
        SvxNumberFormat aFmt(GetLevel(i));
        const SvxBrushItem* pBrush = aFmt.GetBrush();
        if(SVX_NUM_BITMAP == aFmt.GetNumberingType())
        {
            const Graphic* pGraphic = nullptr;
            if(pBrush &&
                !pBrush->GetGraphicLink().isEmpty() &&
                    nullptr != (pGraphic = pBrush->GetGraphic()))
            {
                SvxBrushItem aTempItem(*pBrush);
                aTempItem.SetGraphicLink("");
                aTempItem.SetGraphic(*pGraphic);
                sal_Int16    eOrient = aFmt.GetVertOrient();
                aFmt.SetGraphicBrush( &aTempItem, &aFmt.GetGraphicSize(), &eOrient );
            }
        }
        else if((SVX_NUM_BITMAP|LINK_TOKEN) == static_cast<int>(aFmt.GetNumberingType()))
            aFmt.SetNumberingType(SVX_NUM_BITMAP);
        SetLevel(i, aFmt);
    }
}

SvxNumBulletItem::SvxNumBulletItem(SvxNumRule const & rRule) :
    SfxPoolItem(SID_ATTR_NUMBERING_RULE),
    pNumRule(new SvxNumRule(rRule))
{
}

SvxNumBulletItem::SvxNumBulletItem(SvxNumRule const & rRule, sal_uInt16 _nWhich ) :
    SfxPoolItem(_nWhich),
    pNumRule(new SvxNumRule(rRule))
{
}

SvxNumBulletItem::SvxNumBulletItem(const SvxNumBulletItem& rCopy) :
    SfxPoolItem(rCopy),
    pNumRule(new SvxNumRule(*rCopy.pNumRule))
{
}

SvxNumBulletItem::~SvxNumBulletItem()
{
}

bool SvxNumBulletItem::operator==( const SfxPoolItem& rCopy) const
{
    return *pNumRule == *static_cast<const SvxNumBulletItem&>(rCopy).pNumRule;
}

SfxPoolItem*  SvxNumBulletItem::Clone( SfxItemPool * ) const
{
    return new SvxNumBulletItem(*this);
}

sal_uInt16  SvxNumBulletItem::GetVersion( sal_uInt16 /*nFileVersion*/ ) const
{
    return NUMITEM_VERSION_03;
}

bool SvxNumBulletItem::QueryValue( css::uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    rVal <<= SvxCreateNumRule( pNumRule.get() );
    return true;
}

bool SvxNumBulletItem::PutValue( const css::uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    uno::Reference< container::XIndexReplace > xRule;
    if( rVal >>= xRule )
    {
        try
        {
            std::unique_ptr<SvxNumRule> pNewRule(new SvxNumRule( SvxGetNumRule( xRule ) ));
            if( pNewRule->GetLevelCount() != pNumRule->GetLevelCount() ||
                pNewRule->GetNumRuleType() != pNumRule->GetNumRuleType() )
            {
                std::unique_ptr<SvxNumRule> pConverted = SvxConvertNumRule( pNewRule.get(), pNumRule->GetLevelCount(), pNumRule->GetNumRuleType() );
                pNewRule = std::move(pConverted);
            }
            pNumRule = std::move( pNewRule );
            return true;
        }
        catch(const lang::IllegalArgumentException&)
        {
        }
    }
    return false;
}

void SvxNumBulletItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("SvxNumBulletItem"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    pNumRule->dumpAsXml(pWriter);
    xmlTextWriterEndElement(pWriter);
}

std::unique_ptr<SvxNumRule> SvxConvertNumRule( const SvxNumRule* pRule, sal_uInt16 nLevels, SvxNumRuleType eType )
{
    const sal_uInt16 nSrcLevels = pRule->GetLevelCount();
    std::unique_ptr<SvxNumRule> pNewRule(new SvxNumRule( pRule->GetFeatureFlags(), nLevels, pRule->IsContinuousNumbering(), eType ));

    for( sal_uInt16 nLevel = 0; (nLevel < nLevels) && (nLevel < nSrcLevels); nLevel++ )
        pNewRule->SetLevel( nLevel, pRule->GetLevel( nLevel ) );

    return pNewRule;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
