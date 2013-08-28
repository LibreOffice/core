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


#include <editeng/numitem.hxx>

#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <editeng/brushitem.hxx>
#include <vcl/font.hxx>
#include <editeng/editids.hrc>
#include <editeng/editrids.hrc>
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
#include <comphelper/processfactory.hxx>

#include <editeng/unonrule.hxx>

#define MM100_TO_TWIP(MM100)    ((MM100*72L+63L)/127L)

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
com::sun::star::uno::Reference<com::sun::star::text::XNumberingFormatter> SvxNumberType::xFormatter = 0;
static void lcl_getFormatter(com::sun::star::uno::Reference<com::sun::star::text::XNumberingFormatter>& _xFormatter)
{
    if(!_xFormatter.is())
       {
        try
        {
            Reference<XComponentContext>         xContext( ::comphelper::getProcessComponentContext() );
            Reference<XDefaultNumberingProvider> xRet = text::DefaultNumberingProvider::create(xContext);
            _xFormatter = Reference<XNumberingFormatter> (xRet, UNO_QUERY);
        }
        catch(const Exception&)
        {
            SAL_WARN("editeng", "service missing: \"com.sun.star.text.DefaultNumberingProvider\"");
        }
    }
}

SvxNumberType::SvxNumberType(sal_Int16 nType) :
    nNumType(nType),
    bShowSymbol(sal_True)
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
        xFormatter = 0;
}

OUString SvxNumberType::GetNumStr( sal_uLong nNo ) const
{
    const LanguageTag& rLang = Application::GetSettings().GetLanguageTag();
    return GetNumStr( nNo, rLang.getLocale() );
}

OUString SvxNumberType::GetNumStr( sal_uLong nNo, const Locale& rLocale ) const
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
                        pValues[0].Name = OUString("NumberingType");
                        pValues[0].Value <<= nNumType;
                        pValues[1].Name = OUString("Value");
                        pValues[1].Value <<= (sal_Int32)nNo;

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

SvxNumberFormat::SvxNumberFormat( sal_Int16 eType,
                                  SvxNumPositionAndSpaceMode ePositionAndSpaceMode )
    : SvxNumberType(eType),
      eNumAdjust(SVX_ADJUST_LEFT),
      nInclUpperLevels(0),
      nStart(1),
      cBullet(SVX_DEF_BULLET),
      nBulletRelSize(100),
      nBulletColor(COL_BLACK),
      mePositionAndSpaceMode( ePositionAndSpaceMode ),
      nFirstLineOffset(0),
      nAbsLSpace(0),
      nLSpace(0),
      nCharTextDistance(0),
      meLabelFollowedBy( LISTTAB ),
      mnListtabPos( 0 ),
      mnFirstLineIndent( 0 ),
      mnIndentAt( 0 ),
      pGraphicBrush(0),
      eVertOrient(text::VertOrientation::NONE),
      pBulletFont(0)
{
}

SvxNumberFormat::SvxNumberFormat(const SvxNumberFormat& rFormat) :
    SvxNumberType(rFormat),
    mePositionAndSpaceMode( rFormat.mePositionAndSpaceMode ),
    pGraphicBrush(0),
    pBulletFont(0)
{
    *this = rFormat;
}

SvxNumberFormat::SvxNumberFormat( SvStream &rStream )
{
    sal_uInt16 nTmp16;
    sal_Int32  nTmp32;
    rStream >> nTmp16; // Version number

    rStream >> nTmp16; SetNumberingType( nTmp16 );
    rStream >> nTmp16; eNumAdjust = ( SvxAdjust )nTmp16;
    rStream >> nTmp16; nInclUpperLevels = nTmp16;
    rStream >> nStart;
    rStream >> nTmp16; cBullet = (sal_Unicode)nTmp16;

    rStream >> nFirstLineOffset;
    rStream >> nAbsLSpace;
    rStream >> nLSpace;

    rStream >> nCharTextDistance;

    sPrefix = rStream.ReadUniOrByteString( rStream.GetStreamCharSet() );
    sSuffix = rStream.ReadUniOrByteString( rStream.GetStreamCharSet() );
    sCharStyleName = rStream.ReadUniOrByteString( rStream.GetStreamCharSet() );

    sal_uInt16 hasGraphicBrush = 0;
    rStream >> hasGraphicBrush;
    if ( hasGraphicBrush )
    {
        pGraphicBrush = new SvxBrushItem( SID_ATTR_BRUSH );
        pGraphicBrush = (SvxBrushItem*)(pGraphicBrush->Create( rStream, BRUSH_GRAPHIC_VERSION ));
    }
    else pGraphicBrush = 0;
    rStream >> nTmp16; eVertOrient = nTmp16;

    sal_uInt16 hasBulletFont = 0;
    rStream >> hasBulletFont;
    if ( hasBulletFont )
    {
        pBulletFont = new Font( );
        rStream >> *pBulletFont;
    }
    else pBulletFont = NULL;
    rStream >> aGraphicSize;

    rStream >> nBulletColor;
    rStream >> nBulletRelSize;
    rStream >> nTmp16; SetShowSymbol( nTmp16 );

    rStream >> nTmp16; mePositionAndSpaceMode = ( SvxNumPositionAndSpaceMode )nTmp16;
    rStream >> nTmp16; meLabelFollowedBy = ( LabelFollowedBy )nTmp16;
    rStream >> nTmp32; mnListtabPos = nTmp32;
    rStream >> nTmp32; mnFirstLineIndent = nTmp32;
    rStream >> nTmp32; mnIndentAt = nTmp32;

}
SvxNumberFormat::~SvxNumberFormat()
{
    delete pGraphicBrush;
    delete pBulletFont;
}

SvStream&   SvxNumberFormat::Store(SvStream &rStream, FontToSubsFontConverter pConverter)
{
    if(pConverter && pBulletFont)
    {
        cBullet = ConvertFontToSubsFontChar(pConverter, cBullet);
        String sFontName = GetFontToSubsFontName(pConverter);
        pBulletFont->SetName(sFontName);
    }

    rStream << (sal_uInt16)NUMITEM_VERSION_04;

    rStream << (sal_uInt16)GetNumberingType();
    rStream << (sal_uInt16)eNumAdjust;
    rStream << (sal_uInt16)nInclUpperLevels;
    rStream << nStart;
    rStream << (sal_uInt16)cBullet;

    rStream << nFirstLineOffset;
    rStream << nAbsLSpace;
    rStream << nLSpace;

    rStream << nCharTextDistance;
    rtl_TextEncoding eEnc = osl_getThreadTextEncoding();
    rStream.WriteUniOrByteString(sPrefix, eEnc);
    rStream.WriteUniOrByteString(sSuffix, eEnc);
    rStream.WriteUniOrByteString(sCharStyleName, eEnc);
    if(pGraphicBrush)
    {
        rStream << (sal_uInt16)1;

        // in SD or SI force bullet itself to be stored,
        // for that purpose throw away link when link and graphic
        // are present, so Brush save is forced
        if(pGraphicBrush->GetGraphicLink() && pGraphicBrush->GetGraphic())
        {
            String aEmpty;
            pGraphicBrush->SetGraphicLink(aEmpty);
        }

        pGraphicBrush->Store(rStream, BRUSH_GRAPHIC_VERSION);
    }
    else
        rStream << (sal_uInt16)0;

    rStream << (sal_uInt16)eVertOrient;
    if(pBulletFont)
    {
        rStream << (sal_uInt16)1;
        rStream << *pBulletFont;
    }
    else
        rStream << (sal_uInt16)0;
    rStream << aGraphicSize;

    Color nTempColor = nBulletColor;
    if(COL_AUTO == nBulletColor.GetColor())
        nTempColor = COL_BLACK;
    rStream << nTempColor;
    rStream << nBulletRelSize;
    rStream << (sal_uInt16)IsShowSymbol();

    rStream << ( sal_uInt16 ) mePositionAndSpaceMode;
    rStream << ( sal_uInt16 ) meLabelFollowedBy;
    rStream << ( sal_Int32 ) mnListtabPos;
    rStream << ( sal_Int32 ) mnFirstLineIndent;
    rStream << ( sal_Int32 ) mnIndentAt;

    return rStream;
}

SvxNumberFormat* SvxNumberFormat::Create( SvStream &rStream )
{
    return new SvxNumberFormat( rStream );
}

SvxNumberFormat& SvxNumberFormat::operator=( const SvxNumberFormat& rFormat )
{
    if (& rFormat == this) { return *this; }

    SetNumberingType(rFormat.GetNumberingType());
        eNumAdjust          = rFormat.eNumAdjust ;
        nInclUpperLevels    = rFormat.nInclUpperLevels ;
        nStart              = rFormat.nStart ;
        cBullet             = rFormat.cBullet ;
        mePositionAndSpaceMode = rFormat.mePositionAndSpaceMode;
        nFirstLineOffset    = rFormat.nFirstLineOffset;
        nAbsLSpace          = rFormat.nAbsLSpace ;
        nLSpace             = rFormat.nLSpace ;
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
    DELETEZ(pGraphicBrush);
    if(rFormat.pGraphicBrush)
    {
        pGraphicBrush = new SvxBrushItem(*rFormat.pGraphicBrush);
        pGraphicBrush->SetDoneLink( STATIC_LINK( this, SvxNumberFormat, GraphicArrived) );
    }
    DELETEZ(pBulletFont);
    if(rFormat.pBulletFont)
            pBulletFont = new Font(*rFormat.pBulletFont);
    return *this;
}

sal_Bool  SvxNumberFormat::operator==( const SvxNumberFormat& rFormat) const
{
    if( GetNumberingType()  != rFormat.GetNumberingType() ||
        eNumAdjust          != rFormat.eNumAdjust ||
        nInclUpperLevels    != rFormat.nInclUpperLevels ||
        nStart              != rFormat.nStart ||
        cBullet             != rFormat.cBullet ||
        mePositionAndSpaceMode != rFormat.mePositionAndSpaceMode ||
        nFirstLineOffset    != rFormat.nFirstLineOffset ||
        nAbsLSpace          != rFormat.nAbsLSpace ||
        nLSpace             != rFormat.nLSpace ||
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
        return sal_False;
    if (
        (pGraphicBrush && !rFormat.pGraphicBrush) ||
        (!pGraphicBrush && rFormat.pGraphicBrush) ||
        (pGraphicBrush && *pGraphicBrush != *rFormat.pGraphicBrush)
       )
    {
        return sal_False;
    }
    if (
        (pBulletFont && !rFormat.pBulletFont) ||
        (!pBulletFont && rFormat.pBulletFont) ||
        (pBulletFont && *pBulletFont != *rFormat.pBulletFont)
       )
    {
        return sal_False;
    }
    return sal_True;
}

void SvxNumberFormat::SetGraphicBrush( const SvxBrushItem* pBrushItem,
                    const Size* pSize, const sal_Int16* pOrient)
{
    if(!pBrushItem)
    {
        delete pGraphicBrush;
        pGraphicBrush = 0;
    }
    else if ( !pGraphicBrush || (pGraphicBrush && !(*pBrushItem == *pGraphicBrush)) )
    {
        delete pGraphicBrush;
        pGraphicBrush =  (SvxBrushItem*)pBrushItem->Clone();
        pGraphicBrush->SetDoneLink( STATIC_LINK( this, SvxNumberFormat, GraphicArrived) );
   }

    if(pOrient)
        eVertOrient = *pOrient;
    else
        eVertOrient = text::VertOrientation::NONE;
    if(pSize)
        aGraphicSize = *pSize;
    else
        aGraphicSize.Width() = aGraphicSize.Height() = 0;
}

void SvxNumberFormat::SetGraphic( const String& rName )
{
    const String* pName;
    if( pGraphicBrush &&
            0 != (pName = pGraphicBrush->GetGraphicLink())
                && *pName == rName )
        return ;

    delete pGraphicBrush;
    String sTmp;
    pGraphicBrush = new SvxBrushItem( rName, sTmp, GPOS_AREA, 0 );
    pGraphicBrush->SetDoneLink( STATIC_LINK( this, SvxNumberFormat, GraphicArrived) );
    if( eVertOrient == text::VertOrientation::NONE )
        eVertOrient = text::VertOrientation::TOP;

    aGraphicSize.Width() = aGraphicSize.Height() = 0;
}

void SvxNumberFormat::SetVertOrient(sal_Int16 eSet)
{
    eVertOrient = eSet;
}

sal_Int16    SvxNumberFormat::GetVertOrient() const
{
    return eVertOrient;
}

void SvxNumberFormat::SetBulletFont(const Font* pFont)
{
    delete pBulletFont;
    pBulletFont = pFont ? new Font(*pFont): 0;
}

SvxNumberFormat::SvxNumPositionAndSpaceMode SvxNumberFormat::GetPositionAndSpaceMode() const
{
    return mePositionAndSpaceMode;
}
void SvxNumberFormat::SetPositionAndSpaceMode( SvxNumPositionAndSpaceMode ePositionAndSpaceMode )
{
    mePositionAndSpaceMode = ePositionAndSpaceMode;
}

short SvxNumberFormat::GetLSpace() const
{
    return mePositionAndSpaceMode == LABEL_WIDTH_AND_POSITION ? nLSpace : 0;
}
short SvxNumberFormat::GetAbsLSpace() const
{
    return mePositionAndSpaceMode == LABEL_WIDTH_AND_POSITION
           ? nAbsLSpace
           : static_cast<short>( GetFirstLineIndent() + GetIndentAt() );
}
short SvxNumberFormat::GetFirstLineOffset() const
{
    return mePositionAndSpaceMode == LABEL_WIDTH_AND_POSITION
           ? nFirstLineOffset
           : static_cast<short>( GetFirstLineIndent() );
}
short SvxNumberFormat::GetCharTextDistance() const
{
    return mePositionAndSpaceMode == LABEL_WIDTH_AND_POSITION ? nCharTextDistance : 0;
}

void SvxNumberFormat::SetLabelFollowedBy( const LabelFollowedBy eLabelFollowedBy )
{
    meLabelFollowedBy = eLabelFollowedBy;
}
SvxNumberFormat::LabelFollowedBy SvxNumberFormat::GetLabelFollowedBy() const
{
    return meLabelFollowedBy;
}
void SvxNumberFormat::SetListtabPos( const long nListtabPos )
{
    mnListtabPos = nListtabPos;
}
long SvxNumberFormat::GetListtabPos() const
{
    return mnListtabPos;
}
void SvxNumberFormat::SetFirstLineIndent( const long nFirstLineIndent )
{
    mnFirstLineIndent = nFirstLineIndent;
}
long SvxNumberFormat::GetFirstLineIndent() const
{
    return mnFirstLineIndent;
}
void SvxNumberFormat::SetIndentAt( const long nIndentAt )
{
    mnIndentAt = nIndentAt;
}
long SvxNumberFormat::GetIndentAt() const
{
    return mnIndentAt;
}

IMPL_STATIC_LINK( SvxNumberFormat, GraphicArrived, void *, EMPTYARG )
{
    // if necessary, set the GrfSize:
    if( !pThis->aGraphicSize.Width() || !pThis->aGraphicSize.Height() )
    {
        const Graphic* pGrf = pThis->pGraphicBrush->GetGraphic();
        if( pGrf )
            pThis->aGraphicSize = SvxNumberFormat::GetGraphicSizeMM100( pGrf );
    }
    pThis->NotifyGraphicArrived();
    return 0;
}

void SvxNumberFormat::NotifyGraphicArrived()
{
}

Size SvxNumberFormat::GetGraphicSizeMM100(const Graphic* pGraphic)
{
    const MapMode aMapMM100( MAP_100TH_MM );
    const Size& rSize = pGraphic->GetPrefSize();
    Size aRetSize;
    if ( pGraphic->GetPrefMapMode().GetMapUnit() == MAP_PIXEL )
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

String SvxNumberFormat::CreateRomanString( sal_uLong nNo, sal_Bool bUpper )
{
    nNo %= 4000;            // more can not be displayed
//      i, ii, iii, iv, v, vi, vii, vii, viii, ix
//                          (Dummy),1000,500,100,50,10,5,1
    const char *cRomanArr = bUpper
                        ? "MDCLXVI--"   // +2 Dummy entries!
                        : "mdclxvi--";  // +2 Dummy entries!

    String sRet;
    sal_uInt16 nMask = 1000;
    while( nMask )
    {
        sal_uInt8 nZahl = sal_uInt8(nNo / nMask);
        sal_uInt8 nDiff = 1;
        nNo %= nMask;

        if( 5 < nZahl )
        {
            if( nZahl < 9 )
                sRet += sal_Unicode(*(cRomanArr-1));
            ++nDiff;
            nZahl -= 5;
        }
        switch( nZahl )
        {
        case 3:     { sRet += sal_Unicode(*cRomanArr); }
        case 2:     { sRet += sal_Unicode(*cRomanArr); }
        case 1:     { sRet += sal_Unicode(*cRomanArr); }
                    break;

        case 4:     {
                        sRet += sal_Unicode(*cRomanArr);
                        sRet += sal_Unicode(*(cRomanArr-nDiff));
                    }
                    break;
        case 5:     { sRet += sal_Unicode(*(cRomanArr-nDiff)); }
                    break;
        }

        nMask /= 10;            // for the next decade
        cRomanArr += 2;
    }
    return sRet;
}

OUString SvxNumberFormat::GetCharFmtName()const
{
    return sCharStyleName;
}

sal_Int32 SvxNumRule::nRefCount = 0;
static SvxNumberFormat* pStdNumFmt = 0;
static SvxNumberFormat* pStdOutlineNumFmt = 0;
SvxNumRule::SvxNumRule( sal_uLong nFeatures,
                        sal_uInt16 nLevels,
                        sal_Bool bCont,
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
            aFmts[i] = new SvxNumberFormat(SVX_NUM_CHARS_UPPER_LETTER);
            // It is a distinction between writer and draw
            if(nFeatures & NUM_CONTINUOUS)
            {
                if ( eDefaultNumberFormatPositionAndSpaceMode ==
                                    SvxNumberFormat::LABEL_WIDTH_AND_POSITION )
                {
                    aFmts[i]->SetLSpace( MM100_TO_TWIP(DEF_WRITER_LSPACE) );
                    aFmts[i]->SetAbsLSpace( MM100_TO_TWIP(DEF_WRITER_LSPACE * (i+1)) );
                    aFmts[i]->SetFirstLineOffset(MM100_TO_TWIP(-DEF_WRITER_LSPACE));
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
                aFmts[i]->SetLSpace( DEF_DRAW_LSPACE );
                aFmts[i]->SetAbsLSpace( DEF_DRAW_LSPACE * (i) );
            }
        }
        else
            aFmts[i] = 0;
        aFmtsSet[i] = sal_False;
    }
}

SvxNumRule::SvxNumRule(const SvxNumRule& rCopy)
{
    ++nRefCount;
    aLocale             = rCopy.aLocale;
    nLevelCount          = rCopy.nLevelCount         ;
    nFeatureFlags        = rCopy.nFeatureFlags       ;
    bContinuousNumbering = rCopy.bContinuousNumbering;
    eNumberingType       = rCopy.eNumberingType;
    memset( aFmts, 0, sizeof( aFmts ));
    for(sal_uInt16 i = 0; i < SVX_MAX_NUM; i++)
    {
        if(rCopy.aFmts[i])
            aFmts[i] = new SvxNumberFormat(*rCopy.aFmts[i]);
        else
            aFmts[i] = 0;
        aFmtsSet[i] = rCopy.aFmtsSet[i];
    }
}

SvxNumRule::SvxNumRule( SvStream &rStream )
{
    sal_uInt16 nTmp16;
    rStream >> nTmp16; // NUM_ITEM_VERSION
    rStream >> nLevelCount;

    // first nFeatureFlags of old Versions
    rStream >> nTmp16; nFeatureFlags = nTmp16;
    rStream >> nTmp16; bContinuousNumbering = nTmp16;
    rStream >> nTmp16; eNumberingType = ( SvxNumRuleType )nTmp16;

    for (sal_uInt16 i = 0; i < SVX_MAX_NUM; i++)
    {
        rStream >> nTmp16;
        sal_Bool hasNumberingFormat = nTmp16 & 1;
        aFmtsSet[i] = nTmp16 & 2; // fdo#68648 reset flag
        if ( hasNumberingFormat ){
            aFmts[i] = new SvxNumberFormat( rStream );
        }
        else
        {
            aFmts[i] = 0;
            aFmtsSet[i] = sal_False; // actually only false is valid
        }
    }
    //second nFeatureFlags for new versions
    rStream >> nTmp16; nFeatureFlags = nTmp16;
}

SvxNumRule* SvxNumRule::Create( SvStream & rStream )
{
    return new SvxNumRule( rStream );
}

SvStream& SvxNumRule::Store( SvStream &rStream )
{
    rStream<<(sal_uInt16)NUMITEM_VERSION_03;
    rStream<<nLevelCount;
    //first save of nFeatureFlags for old versions
    rStream<<(sal_uInt16)nFeatureFlags;
    rStream<<(sal_uInt16)bContinuousNumbering;
    rStream<<(sal_uInt16)eNumberingType;

    FontToSubsFontConverter pConverter = 0;
    sal_Bool bConvertBulletFont = ( rStream.GetVersion() <= SOFFICE_FILEFORMAT_50 ) && ( rStream.GetVersion() );
    for(sal_uInt16 i = 0; i < SVX_MAX_NUM; i++)
    {
        sal_uInt16 nSetFlag(aFmtsSet[i] ? 2 : 0); // fdo#68648 store that too
        if(aFmts[i])
        {
            rStream << sal_uInt16(1 | nSetFlag);
            if(bConvertBulletFont && aFmts[i]->GetBulletFont())
            {
                if(!pConverter)
                    pConverter =
                        CreateFontToSubsFontConverter(aFmts[i]->GetBulletFont()->GetName(),
                                    FONTTOSUBSFONT_EXPORT|FONTTOSUBSFONT_ONLYOLDSOSYMBOLFONTS);
            }
            aFmts[i]->Store(rStream, pConverter);
        }
        else
            rStream << sal_uInt16(0 | nSetFlag);
    }
    //second save of nFeatureFlags for new versions
    rStream<<(sal_uInt16)nFeatureFlags;
    if(pConverter)
        DestroyFontToSubsFontConverter(pConverter);

    return rStream;
}
SvxNumRule::~SvxNumRule()
{
    for(sal_uInt16 i = 0; i < SVX_MAX_NUM; i++)
        delete aFmts[i];
    if(!--nRefCount)
    {
        DELETEZ(pStdNumFmt);
        DELETEZ(pStdOutlineNumFmt);
    }
}

SvxNumRule& SvxNumRule::operator=( const SvxNumRule& rCopy )
{
    nLevelCount          = rCopy.nLevelCount;
    nFeatureFlags        = rCopy.nFeatureFlags;
    bContinuousNumbering = rCopy.bContinuousNumbering;
    eNumberingType       = rCopy.eNumberingType;
    for(sal_uInt16 i = 0; i < SVX_MAX_NUM; i++)
    {
        delete aFmts[i];
        if(rCopy.aFmts[i])
            aFmts[i] = new SvxNumberFormat(*rCopy.aFmts[i]);
        else
            aFmts[i] = 0;
        aFmtsSet[i] = rCopy.aFmtsSet[i];
    }
    return *this;
}

int   SvxNumRule::operator==( const SvxNumRule& rCopy) const
{
    if(nLevelCount != rCopy.nLevelCount ||
        nFeatureFlags != rCopy.nFeatureFlags ||
            bContinuousNumbering != rCopy.bContinuousNumbering ||
                eNumberingType != rCopy.eNumberingType)
            return sal_False;
    for(sal_uInt16 i = 0; i < nLevelCount; i++)
    {
        if (
            (aFmtsSet[i] != rCopy.aFmtsSet[i]) ||
            (!aFmts[i] && rCopy.aFmts[i]) ||
            (aFmts[i] && !rCopy.aFmts[i]) ||
            (aFmts[i] && *aFmts[i] !=  *rCopy.aFmts[i])
           )
        {
            return sal_False;
        }
    }
    return sal_True;
}

const SvxNumberFormat*  SvxNumRule::Get(sal_uInt16 nLevel)const
{
    DBG_ASSERT(nLevel < SVX_MAX_NUM, "Wrong Level" );
    if( nLevel < SVX_MAX_NUM )
        return aFmtsSet[nLevel] ? aFmts[nLevel] : 0;
    else
        return 0;
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
            *aFmts[nLevel] :  eNumberingType == SVX_RULETYPE_NUMBERING ?
                                                        *pStdNumFmt : *pStdOutlineNumFmt;
}

void SvxNumRule::SetLevel( sal_uInt16 i, const SvxNumberFormat& rNumFmt, sal_Bool bIsValid )
{
    DBG_ASSERT(i < SVX_MAX_NUM, "Wrong Level" );

    if( (i < SVX_MAX_NUM) && (!aFmtsSet[i] || !(rNumFmt == *Get( i ))) )
    {
        delete aFmts[ i ];
        aFmts[ i ] = new SvxNumberFormat( rNumFmt );
        aFmtsSet[i] = bIsValid;
    }
}

void SvxNumRule::SetLevel(sal_uInt16 nLevel, const SvxNumberFormat* pFmt)
{
    DBG_ASSERT(nLevel < SVX_MAX_NUM, "Wrong Level" );

    if( nLevel < SVX_MAX_NUM )
    {
        aFmtsSet[nLevel] = 0 != pFmt;
        if(pFmt)
            SetLevel(nLevel, *pFmt);
        else
        {
            delete aFmts[nLevel];
            aFmts[nLevel] = 0;
        }
    }
}

String  SvxNumRule::MakeNumString( const SvxNodeNum& rNum, sal_Bool bInclStrings ) const
{
    String aStr;
    if( SVX_NO_NUM > rNum.GetLevel() && !( SVX_NO_NUMLEVEL & rNum.GetLevel() ) )
    {
        const SvxNumberFormat& rMyNFmt = GetLevel( rNum.GetLevel() );
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

                sal_Bool bDot = sal_True;
                if( rNum.GetLevelVal()[ i ] )
                {
                    if(SVX_NUM_BITMAP != rNFmt.GetNumberingType())
                        aStr += rNFmt.GetNumStr( rNum.GetLevelVal()[ i ], aLocale );
                    else
                        bDot = sal_False;
                }
                else
                    aStr += sal_Unicode('0');       // all 0-levels are a 0
                if( i != rNum.GetLevel() && bDot)
                    aStr += sal_Unicode('.');
            }
        }

        if( bInclStrings )
        {
            aStr.Insert( rMyNFmt.GetPrefix(), 0 );
            aStr += rMyNFmt.GetSuffix();
        }
    }
    return aStr;
}

// changes linked to embedded bitmaps
sal_Bool SvxNumRule::UnLinkGraphics()
{
    sal_Bool bRet = sal_False;
    for(sal_uInt16 i = 0; i < GetLevelCount(); i++)
    {
        SvxNumberFormat aFmt(GetLevel(i));
        const SvxBrushItem* pBrush = aFmt.GetBrush();
        const String* pLinkStr;
        const Graphic* pGraphic;
        if(SVX_NUM_BITMAP == aFmt.GetNumberingType())
        {
            if(pBrush &&
                0 != (pLinkStr = pBrush->GetGraphicLink()) &&
                    pLinkStr->Len() &&
                    0 !=(pGraphic = pBrush->GetGraphic()))
            {
                SvxBrushItem aTempItem(*pBrush);
                aTempItem.SetGraphicLink( String());
                aTempItem.SetGraphic(*pGraphic);
                sal_Int16    eOrient = aFmt.GetVertOrient();
                aFmt.SetGraphicBrush( &aTempItem, &aFmt.GetGraphicSize(), &eOrient );
                bRet = sal_True;
            }
        }
        else if((SVX_NUM_BITMAP|LINK_TOKEN) == aFmt.GetNumberingType())
            aFmt.SetNumberingType(SVX_NUM_BITMAP);
        SetLevel(i, aFmt);
    }
    return bRet;
}

SvxNumBulletItem::SvxNumBulletItem(SvxNumRule& rRule) :
    SfxPoolItem(SID_ATTR_NUMBERING_RULE),
    pNumRule(new SvxNumRule(rRule))
{
}

SvxNumBulletItem::SvxNumBulletItem(SvxNumRule& rRule, sal_uInt16 _nWhich ) :
    SfxPoolItem(_nWhich),
    pNumRule(new SvxNumRule(rRule))
{
}

SfxPoolItem* SvxNumBulletItem::Create(SvStream &rStream, sal_uInt16 /*nItemVersion*/ ) const
{
    SvxNumRule aNumRule( rStream );
    return new SvxNumBulletItem( aNumRule, EE_PARA_NUMBULLET );
}

SvxNumBulletItem::SvxNumBulletItem(const SvxNumBulletItem& rCopy) :
    SfxPoolItem(rCopy.Which())
{
    pNumRule = new SvxNumRule(*rCopy.pNumRule);
}

SvxNumBulletItem::~SvxNumBulletItem()
{
    delete pNumRule;
}

int  SvxNumBulletItem::operator==( const SfxPoolItem& rCopy) const
{
    return *pNumRule == *((SvxNumBulletItem&)rCopy).pNumRule;
}

SfxPoolItem*  SvxNumBulletItem::Clone( SfxItemPool * ) const
{
    return new SvxNumBulletItem(*this);
}

sal_uInt16  SvxNumBulletItem::GetVersion( sal_uInt16 /*nFileVersion*/ ) const
{
    return NUMITEM_VERSION_03;
}

SvStream&   SvxNumBulletItem::Store(SvStream &rStream, sal_uInt16 /*nItemVersion*/ )const
{
    pNumRule->Store(rStream);
    return rStream;
}

bool SvxNumBulletItem::QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    rVal <<= SvxCreateNumRule( pNumRule );
    return true;
}

bool SvxNumBulletItem::PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
{
    uno::Reference< container::XIndexReplace > xRule;
    if( rVal >>= xRule )
    {
        try
        {
            SvxNumRule* pNewRule = new SvxNumRule( SvxGetNumRule( xRule ) );
            if( pNewRule->GetLevelCount() != pNumRule->GetLevelCount() ||
                pNewRule->GetNumRuleType() != pNumRule->GetNumRuleType() )
            {
                SvxNumRule* pConverted = SvxConvertNumRule( pNewRule, pNumRule->GetLevelCount(), pNumRule->GetNumRuleType() );
                delete pNewRule;
                pNewRule = pConverted;
            }
            delete pNumRule;
            pNumRule = pNewRule;
            return true;
        }
        catch(const lang::IllegalArgumentException&)
        {
        }
    }
    return false;
}

SvxNumRule* SvxConvertNumRule( const SvxNumRule* pRule, sal_uInt16 nLevels, SvxNumRuleType eType )
{
    const sal_uInt16 nSrcLevels = pRule->GetLevelCount();
    SvxNumRule* pNewRule = new SvxNumRule( pRule->GetFeatureFlags(), nLevels, pRule->IsContinuousNumbering(), eType );

    for( sal_uInt16 nLevel = 0; (nLevel < nLevels) && (nLevel < nSrcLevels); nLevel++ )
        pNewRule->SetLevel( nLevel, pRule->GetLevel( nLevel ) );

    return pNewRule;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
