/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_editeng.hxx"

#include <editeng/numitem.hxx>

#include <com/sun/star/text/HoriOrientation.hpp>
#include <com/sun/star/text/VertOrientation.hpp>
#include <com/sun/star/text/RelOrientation.hpp>
#include <editeng/brshitem.hxx>
#include <vcl/font.hxx>
#include <editeng/editids.hrc>
#include <editeng/editrids.hrc>
#include <editeng/numdef.hxx>
#include <vcl/graph.hxx>
#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <editeng/unolingu.hxx>
#include <com/sun/star/text/XNumberingFormatter.hpp>
#include <com/sun/star/text/XDefaultNumberingProvider.hpp>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <comphelper/processfactory.hxx>

#include <editeng/unonrule.hxx>

#define MM100_TO_TWIP(MM100)    ((MM100*72L+63L)/127L)

#define DEF_WRITER_LSPACE   500     //Standardeinrueckung
#define DEF_DRAW_LSPACE     800     //Standardeinrueckung

#define NUMITEM_VERSION_01        0x01
#define NUMITEM_VERSION_02        0x02
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
void lcl_getFormatter(com::sun::star::uno::Reference<com::sun::star::text::XNumberingFormatter>& _xFormatter)
{
    if(!_xFormatter.is())
       {
        try
        {
            Reference< XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
            Reference < XInterface > xI = xMSF->createInstance(
                ::rtl::OUString::createFromAscii( "com.sun.star.text.DefaultNumberingProvider" ) );
            Reference<XDefaultNumberingProvider> xRet(xI, UNO_QUERY);
            DBG_ASSERT(xRet.is(), "service missing: \"com.sun.star.text.DefaultNumberingProvider\"");
            _xFormatter = Reference<XNumberingFormatter> (xRet, UNO_QUERY);
        }
        catch(Exception& )
        {
        }
    }
}
/* -----------------------------22.02.01 14:24--------------------------------

 ---------------------------------------------------------------------------*/
SvxNumberType::SvxNumberType(sal_Int16 nType) :
    nNumType(nType),
    bShowSymbol(sal_True)
{
    nRefCount++;
}
/* -----------------------------22.02.01 14:31--------------------------------

 ---------------------------------------------------------------------------*/
SvxNumberType::SvxNumberType(const SvxNumberType& rType) :
    nNumType(rType.nNumType),
    bShowSymbol(rType.bShowSymbol)
{
    nRefCount++;
}
/* -----------------------------22.02.01 14:24--------------------------------

 ---------------------------------------------------------------------------*/
SvxNumberType::~SvxNumberType()
{
    if(!--nRefCount)
        xFormatter = 0;
}
/* -----------------------------22.02.01 11:09--------------------------------

 ---------------------------------------------------------------------------*/
String SvxNumberType::GetNumStr( sal_uLong nNo ) const
{
    LanguageType eLang = Application::GetSettings().GetLanguage();
    Locale aLocale = SvxCreateLocale(eLang);
    return GetNumStr( nNo, aLocale );
}
/* -----------------28.10.98 15:56-------------------
 *
 * --------------------------------------------------*/
String  SvxNumberType::GetNumStr( sal_uLong nNo, const Locale& rLocale ) const
{
    lcl_getFormatter(xFormatter);
    String aTmpStr;
    if(!xFormatter.is())
        return aTmpStr;

    if(bShowSymbol)
    {
        switch(nNumType)
        {
            case NumberingType::CHAR_SPECIAL:
            case NumberingType::BITMAP:
            break;
            default:
                {
                    //#95525# '0' allowed for ARABIC numberings
                    if(NumberingType::ARABIC == nNumType && 0 == nNo )
                        aTmpStr = '0';
                    else
                    {
                        Sequence< PropertyValue > aProperties(2);
                        PropertyValue* pValues = aProperties.getArray();
                        pValues[0].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NumberingType"));
                        pValues[0].Value <<= nNumType;
                        pValues[1].Name = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Value"));
                        pValues[1].Value <<= (sal_Int32)nNo;

                        try
                        {
                            aTmpStr = xFormatter->makeNumberingString( aProperties, rLocale );
                        }
                        catch(Exception&)
                        {
                        }
                    }
                }
        }
    }
    return aTmpStr;
}
/* -----------------27.10.98 10:33-------------------
 *
 * --------------------------------------------------*/
// --> OD 2008-01-09 #newlistlevelattrs#
SvxNumberFormat::SvxNumberFormat( sal_Int16 eType,
                                  SvxNumPositionAndSpaceMode ePositionAndSpaceMode )
// <--
    : SvxNumberType(eType),
      eNumAdjust(SVX_ADJUST_LEFT),
      nInclUpperLevels(0),
      nStart(1),
      cBullet(SVX_DEF_BULLET),
      nBulletRelSize(100),
      nBulletColor(COL_BLACK),
      // --> OD 2008-01-09 #newlistlevelattrs#
      mePositionAndSpaceMode( ePositionAndSpaceMode ),
      // <--
      nFirstLineOffset(0),
      nAbsLSpace(0),
      nLSpace(0),
      nCharTextDistance(0),
      // --> OD 2008-01-09 #newlistlevelattrs#
      meLabelFollowedBy( LISTTAB ),
      mnListtabPos( 0 ),
      mnFirstLineIndent( 0 ),
      mnIndentAt( 0 ),
      // <--
      pGraphicBrush(0),
      eVertOrient(text::VertOrientation::NONE),
      pBulletFont(0)
{
}
/* -----------------27.10.98 10:56-------------------
 *
 * --------------------------------------------------*/
SvxNumberFormat::SvxNumberFormat(const SvxNumberFormat& rFormat) :
    SvxNumberType(rFormat),
    // --> OD 2008-01-09 #newlistlevelattrs#
    mePositionAndSpaceMode( rFormat.mePositionAndSpaceMode ),
    // <--
    pGraphicBrush(0),
    pBulletFont(0)
{
    *this = rFormat;
}
/* -----------------27.10.98 10:56-------------------
 *
 * --------------------------------------------------*/
SvxNumberFormat::~SvxNumberFormat()
{
    delete pGraphicBrush;
    delete pBulletFont;
}
/* -----------------08.12.98 11:14-------------------
 *
 * --------------------------------------------------*/
SvxNumberFormat::SvxNumberFormat(SvStream &rStream)
: mePositionAndSpaceMode( LABEL_WIDTH_AND_POSITION ),
  meLabelFollowedBy( LISTTAB ),
  mnListtabPos( 0 ),
  mnFirstLineIndent( 0 ),
  mnIndentAt( 0 )
{

    sal_uInt16 nVersion;
  rStream >> nVersion;

  sal_uInt16 nUSHORT;
  rStream >> nUSHORT;
  SetNumberingType((sal_Int16)nUSHORT);
  rStream >> nUSHORT;
  eNumAdjust      = (SvxAdjust)nUSHORT;
  rStream >> nUSHORT;
  nInclUpperLevels = (sal_uInt8)nUSHORT;
  rStream >> nUSHORT;
  nStart          = nUSHORT;
  rStream >> nUSHORT;
  cBullet = nUSHORT;

  short nShort;
  rStream >> nShort;
  nFirstLineOffset        = nShort;
  rStream >> nShort;
  nAbsLSpace              = nShort;
  rStream >> nShort;
  nLSpace                 = nShort;

  rStream >> nShort;
  nCharTextDistance       = nShort;
  rtl_TextEncoding eEnc = gsl_getSystemTextEncoding();
  rStream.ReadByteString(sPrefix, eEnc);
  rStream.ReadByteString(sSuffix, eEnc);
  rStream.ReadByteString(sCharStyleName, eEnc);
  rStream >> nUSHORT;
  if(nUSHORT)
  {
        SvxBrushItem aHelper(0);
      pGraphicBrush = (SvxBrushItem*) aHelper.Create( rStream, BRUSH_GRAPHIC_VERSION );
  }
  else
      pGraphicBrush = 0;

  rStream >> nUSHORT;
    eVertOrient          = (sal_Int16)nUSHORT;

  rStream >> nUSHORT;
  if(nUSHORT)
  {
      pBulletFont = new Font;
      rStream >> *pBulletFont;
        if(!pBulletFont->GetCharSet())
            pBulletFont->SetCharSet(rStream.GetStreamCharSet());
    }
  else
      pBulletFont = 0;
  rStream >> aGraphicSize;

  rStream >> nBulletColor;
  rStream >> nUSHORT;
  nBulletRelSize = nUSHORT;
  rStream >> nUSHORT;
  SetShowSymbol((sal_Bool)nUSHORT);

  if( nVersion < NUMITEM_VERSION_03 )
      cBullet = ByteString::ConvertToUnicode( (sal_Char)cBullet,
                          (pBulletFont&&pBulletFont->GetCharSet()) ?  pBulletFont->GetCharSet()
                                      : RTL_TEXTENCODING_SYMBOL );
    if(pBulletFont)
    {
        sal_Bool bConvertBulletFont = rStream.GetVersion() <= SOFFICE_FILEFORMAT_50;
        if(bConvertBulletFont)
        {

            FontToSubsFontConverter pConverter =
                        CreateFontToSubsFontConverter(pBulletFont->GetName(),
                            FONTTOSUBSFONT_IMPORT|FONTTOSUBSFONT_ONLYOLDSOSYMBOLFONTS);
            if(pConverter)
            {
                cBullet = ConvertFontToSubsFontChar(pConverter, cBullet);
                String sFontName = GetFontToSubsFontName(pConverter);
                pBulletFont->SetName(sFontName);
                DestroyFontToSubsFontConverter(pConverter);
            }
        }
    }

    if( NUMITEM_VERSION_04 <= nVersion )
    {
        rStream >> nUSHORT;
        mePositionAndSpaceMode = (SvxNumPositionAndSpaceMode) nUSHORT;
        rStream >> nUSHORT;
        meLabelFollowedBy = ( SvxNumLabelFollowedBy ) nUSHORT;
        long nLong;
        rStream >> nLong;
        mnListtabPos = nLong;
        rStream >> nLong;
        mnFirstLineIndent = nLong;
        rStream >> nLong;
        mnIndentAt = nLong;
    }
}
/* -----------------08.12.98 11:14-------------------
 *
 * --------------------------------------------------*/
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
    rtl_TextEncoding eEnc = gsl_getSystemTextEncoding();
    rStream.WriteByteString(sPrefix, eEnc);
    rStream.WriteByteString(sSuffix, eEnc);
    rStream.WriteByteString(sCharStyleName, eEnc);
    if(pGraphicBrush)
    {
        rStream << (sal_uInt16)1;

        // #75113# in SD or SI force bullet itself to be stored,
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
    rStream << ( long ) mnListtabPos;
    rStream << ( long ) mnFirstLineIndent;
    rStream << ( long ) mnIndentAt;

    return rStream;
}

/* -----------------------------23.02.01 11:10--------------------------------

 ---------------------------------------------------------------------------*/
SvxNumberFormat& SvxNumberFormat::operator=( const SvxNumberFormat& rFormat )
{
    SetNumberingType(rFormat.GetNumberingType());
        eNumAdjust          = rFormat.eNumAdjust ;
        nInclUpperLevels    = rFormat.nInclUpperLevels ;
        nStart              = rFormat.nStart ;
        cBullet             = rFormat.cBullet ;
        // --> OD 2008-01-09 #newlistlevelattrs#
        mePositionAndSpaceMode = rFormat.mePositionAndSpaceMode;
        // <--
        nFirstLineOffset    = rFormat.nFirstLineOffset;
        nAbsLSpace          = rFormat.nAbsLSpace ;
        nLSpace             = rFormat.nLSpace ;
        nCharTextDistance   = rFormat.nCharTextDistance ;
        // --> OD 2008-01-09 #newlistlevelattrs#
        meLabelFollowedBy = rFormat.meLabelFollowedBy;
        mnListtabPos = rFormat.mnListtabPos;
        mnFirstLineIndent = rFormat.mnFirstLineIndent;
        mnIndentAt = rFormat.mnIndentAt;
        // <--
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
/* -----------------27.10.98 10:56-------------------
 *
 * --------------------------------------------------*/
sal_Bool  SvxNumberFormat::operator==( const SvxNumberFormat& rFormat) const
{
    if( GetNumberingType()  != rFormat.GetNumberingType() ||
        eNumAdjust          != rFormat.eNumAdjust ||
        nInclUpperLevels    != rFormat.nInclUpperLevels ||
        nStart              != rFormat.nStart ||
        cBullet             != rFormat.cBullet ||
        // --> OD 2008-01-09 #newlistlevelattrs#
        mePositionAndSpaceMode != rFormat.mePositionAndSpaceMode ||
        // <--
        nFirstLineOffset    != rFormat.nFirstLineOffset ||
        nAbsLSpace          != rFormat.nAbsLSpace ||
        nLSpace             != rFormat.nLSpace ||
        nCharTextDistance   != rFormat.nCharTextDistance ||
        // --> OD 2008-01-09 #newlistlevelattrs#
        meLabelFollowedBy != rFormat.meLabelFollowedBy ||
        mnListtabPos != rFormat.mnListtabPos ||
        mnFirstLineIndent != rFormat.mnFirstLineIndent ||
        mnIndentAt != rFormat.mnIndentAt ||
        // <--
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
/* -----------------28.10.98 09:53-------------------
 *
 * --------------------------------------------------*/
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
/* -----------------28.10.98 09:59-------------------
 *
 * --------------------------------------------------*/
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
/* -----------------------------22.02.01 15:55--------------------------------

 ---------------------------------------------------------------------------*/
void SvxNumberFormat::SetVertOrient(sal_Int16 eSet)
{
    eVertOrient = eSet;
}
/* -----------------------------22.02.01 15:55--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int16    SvxNumberFormat::GetVertOrient() const
{
    return eVertOrient;
}
/* -----------------28.10.98 09:59-------------------
 *
 * --------------------------------------------------*/
void SvxNumberFormat::SetBulletFont(const Font* pFont)
{
    delete pBulletFont;
    pBulletFont = pFont ? new Font(*pFont): 0;
}

// --> OD 2008-01-09 #newlistlevelattrs#
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
//#if OSL_DEBUG_LEVEL > 1
//    DBG_ASSERT( mePositionAndSpaceMode == LABEL_WIDTH_AND_POSITION,
//                "<SvxNumberFormat::GetLSpace()> - misusage: position-and-space-mode does not equal LABEL_WIDTH_AND_POSITION");
//#endif
    return mePositionAndSpaceMode == LABEL_WIDTH_AND_POSITION ? nLSpace : 0;
}
short SvxNumberFormat::GetAbsLSpace() const
{
//#if OSL_DEBUG_LEVEL > 1
//    DBG_ASSERT( mePositionAndSpaceMode == LABEL_WIDTH_AND_POSITION,
//                "<SvxNumberFormat::GetAbsLSpace()> - misusage: position-and-space-mode does not equal LABEL_WIDTH_AND_POSITION");
//#endif
    return mePositionAndSpaceMode == LABEL_WIDTH_AND_POSITION
           ? nAbsLSpace
           : static_cast<short>( GetFirstLineIndent() + GetIndentAt() );
}
short SvxNumberFormat::GetFirstLineOffset() const
{
//#if OSL_DEBUG_LEVEL > 1
//    DBG_ASSERT( mePositionAndSpaceMode == LABEL_WIDTH_AND_POSITION,
//                "<SvxNumberFormat::GetFirstLineOffset()> - misusage: position-and-space-mode does not equal LABEL_WIDTH_AND_POSITION");
//#endif
    return mePositionAndSpaceMode == LABEL_WIDTH_AND_POSITION
           ? nFirstLineOffset
           : static_cast<short>( GetFirstLineIndent() );
}
short SvxNumberFormat::GetCharTextDistance() const
{
//#if OSL_DEBUG_LEVEL > 1
//    DBG_ASSERT( mePositionAndSpaceMode == LABEL_WIDTH_AND_POSITION,
//                "<SvxNumberFormat::GetCharTextDistance()> - misusage: position-and-space-mode does not equal LABEL_WIDTH_AND_POSITION");
//#endif
    return mePositionAndSpaceMode == LABEL_WIDTH_AND_POSITION ? nCharTextDistance : 0;
}

void SvxNumberFormat::SetLabelFollowedBy( const SvxNumLabelFollowedBy eLabelFollowedBy )
{
    meLabelFollowedBy = eLabelFollowedBy;
}
SvxNumberFormat::SvxNumLabelFollowedBy SvxNumberFormat::GetLabelFollowedBy() const
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
// <--

/* -----------------28.10.98 10:03-------------------
 *
 * --------------------------------------------------*/
IMPL_STATIC_LINK( SvxNumberFormat, GraphicArrived, void *, EMPTYARG )
{
    // ggfs. die GrfSize setzen:
    if( !pThis->aGraphicSize.Width() || !pThis->aGraphicSize.Height() )
    {
        const Graphic* pGrf = pThis->pGraphicBrush->GetGraphic();
        if( pGrf )
            pThis->aGraphicSize = SvxNumberFormat::GetGraphicSizeMM100( pGrf );
    }
    pThis->NotifyGraphicArrived();
    return 0;
}
/* -----------------------------02.07.01 15:36--------------------------------

 ---------------------------------------------------------------------------*/
void SvxNumberFormat::NotifyGraphicArrived()
{
}

/* -----------------28.10.98 10:38-------------------
 *
 * --------------------------------------------------*/
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
/* -----------------28.10.98 15:57-------------------
 *
 * --------------------------------------------------*/
String SvxNumberFormat::CreateRomanString( sal_uLong nNo, sal_Bool bUpper )
{
    nNo %= 4000;            // mehr kann nicht dargestellt werden
//      i, ii, iii, iv, v, vi, vii, vii, viii, ix
//                          (Dummy),1000,500,100,50,10,5,1
    const char *cRomanArr = bUpper
                        ? "MDCLXVI--"   // +2 Dummy-Eintraege !!
                        : "mdclxvi--";  // +2 Dummy-Eintraege !!

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

        nMask /= 10;            // zur naechsten Dekade
        cRomanArr += 2;
    }
    return sRet;
}
#ifdef OLD_NUMBER_FORMATTING
void SvxNumberFormat::GetCharStr( sal_uLong nNo, String& rStr ) const
{
    DBG_ASSERT( nNo, "0 ist eine ungueltige Nummer !!" );

    const sal_uLong coDiff = 'Z' - 'A' +1;
    char cAdd = (SVX_NUM_CHARS_UPPER_LETTER == eNumType ? 'A' : 'a') - 1;
    sal_uLong nCalc;

    do {
        nCalc = nNo % coDiff;
        if( !nCalc )
            nCalc = coDiff;
        rStr.Insert( sal_Unicode(cAdd + nCalc ), 0 );
        nNo -= nCalc;
        if( nNo )
            nNo /= coDiff;
    } while( nNo );
}

void SvxNumberFormat::GetCharStrN( sal_uLong nNo, String& rStr ) const
{
    DBG_ASSERT( nNo, "0 ist eine ungueltige Nummer !!" );

    const sal_uLong coDiff = 'Z' - 'A' +1;
    char cChar = (char)(--nNo % coDiff);
    if( SVX_NUM_CHARS_UPPER_LETTER_N == eNumType )
        cChar += 'A';
    else
        cChar += 'a';

    rStr.Fill( (sal_uInt16)(nNo / coDiff) + 1, sal_Unicode(cChar) );
}
#endif //OLD_NUMBER_FORMATTING
/* -----------------------------22.02.01 13:31--------------------------------

 ---------------------------------------------------------------------------*/
const String&   SvxNumberFormat::GetCharFmtName()const
{
    return sCharStyleName;
}
/* -----------------27.10.98 10:38-------------------
 *
 * --------------------------------------------------*/
sal_Int32 SvxNumRule::nRefCount = 0;
static SvxNumberFormat* pStdNumFmt = 0;
static SvxNumberFormat* pStdOutlineNumFmt = 0;
// --> OD 2008-02-11 #newlistlevelattrs#
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
    LanguageType eLang = Application::GetSettings().GetLanguage();
    aLocale = SvxCreateLocale(eLang);
    for(sal_uInt16 i = 0; i < SVX_MAX_NUM; i++)
    {
        if(i < nLevels)
        {
            aFmts[i] = new SvxNumberFormat(SVX_NUM_CHARS_UPPER_LETTER);
            //daran wird zwischen writer und draw unterschieden
            if(nFeatures & NUM_CONTINUOUS)
            {
                // --> OD 2008-02-11 #newlistlevelattrs#
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
                // <--
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
/* -----------------27.10.98 10:41-------------------
 *
 * --------------------------------------------------*/
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
/* -----------------08.12.98 11:07-------------------
 *
 * --------------------------------------------------*/
SvxNumRule::SvxNumRule(SvStream &rStream)
{
    ++nRefCount;
    LanguageType eLang = Application::GetSettings().GetLanguage();
    aLocale = SvxCreateLocale(eLang);
    sal_uInt16 nVersion;
    sal_uInt16 nTemp;
    rStream >> nVersion;
    rStream >> nLevelCount;
    rStream >> nTemp;
    nFeatureFlags = nTemp;
    rStream >> nTemp;
    bContinuousNumbering = (sal_Bool)nTemp;
    rStream >> nTemp;
    eNumberingType       = (SvxNumRuleType)nTemp;
    memset( aFmts, 0, sizeof( aFmts ));

    for(sal_uInt16 i = 0; i < SVX_MAX_NUM; i++)
    {
        sal_uInt16 nSet;
        rStream >> nSet;
        if(nSet)
            aFmts[i] = new SvxNumberFormat(rStream);
        else
            aFmts[i] = 0;
        aFmtsSet[i] = aFmts[i] ? sal_True : sal_False;
    }
    if(NUMITEM_VERSION_02 <= nVersion)
    {
        sal_uInt16 nShort;
        rStream >> nShort;
        nFeatureFlags = nShort;
    }
}

/* -----------------08.12.98 11:07-------------------
 *
 * --------------------------------------------------*/
SvStream&   SvxNumRule::Store(SvStream &rStream)
{
    rStream<<(sal_uInt16)NUMITEM_VERSION_03;
    rStream<<nLevelCount;
    //first save of nFeatureFlags for old versions
    rStream<<(sal_uInt16)nFeatureFlags;
    rStream<<(sal_uInt16)bContinuousNumbering;
    rStream<<(sal_uInt16)eNumberingType;

    FontToSubsFontConverter pConverter = 0;
    sal_Bool bConvertBulletFont = rStream.GetVersion() <= SOFFICE_FILEFORMAT_50;
    for(sal_uInt16 i = 0; i < SVX_MAX_NUM; i++)
    {
        if(aFmts[i])
        {
            rStream << sal_uInt16(1);
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
            rStream << sal_uInt16(0);
    }
    //second save of nFeatureFlags for new versions
    rStream<<(sal_uInt16)nFeatureFlags;
    if(pConverter)
        DestroyFontToSubsFontConverter(pConverter);

    return rStream;
}

/* -----------------27.10.98 10:41-------------------
 *
 * --------------------------------------------------*/
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
/* -----------------29.10.98 16:07-------------------
 *
 * --------------------------------------------------*/
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
/* -----------------27.10.98 10:41-------------------
 *
 * --------------------------------------------------*/
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
/* -----------------27.10.98 10:41-------------------
 *
 * --------------------------------------------------*/
const SvxNumberFormat*  SvxNumRule::Get(sal_uInt16 nLevel)const
{
    DBG_ASSERT(nLevel < SVX_MAX_NUM, "falsches Level" );
    if( nLevel < SVX_MAX_NUM )
        return aFmtsSet[nLevel] ? aFmts[nLevel] : 0;
    else
        return 0;
}
/* -----------------02.11.98 09:10-------------------
 *
 * --------------------------------------------------*/
const SvxNumberFormat&  SvxNumRule::GetLevel(sal_uInt16 nLevel)const
{
    if(!pStdNumFmt)
    {
        pStdNumFmt = new SvxNumberFormat(SVX_NUM_ARABIC);
         pStdOutlineNumFmt = new SvxNumberFormat(SVX_NUM_NUMBER_NONE);
    }

    DBG_ASSERT(nLevel < SVX_MAX_NUM, "falsches Level" );

    return ( ( nLevel < SVX_MAX_NUM ) && aFmts[nLevel] ) ?
            *aFmts[nLevel] :  eNumberingType == SVX_RULETYPE_NUMBERING ?
                                                        *pStdNumFmt : *pStdOutlineNumFmt;
}

/* -----------------29.10.98 09:08-------------------
 *
 * --------------------------------------------------*/
void SvxNumRule::SetLevel( sal_uInt16 i, const SvxNumberFormat& rNumFmt, sal_Bool bIsValid )
{
    DBG_ASSERT(i < SVX_MAX_NUM, "falsches Level" );

    if( (i < SVX_MAX_NUM) && (!aFmtsSet[i] || !(rNumFmt == *Get( i ))) )
    {
        delete aFmts[ i ];
        aFmts[ i ] = new SvxNumberFormat( rNumFmt );
        aFmtsSet[i] = bIsValid;
//      bInvalidRuleFlag = sal_True;
    }
}
/* -----------------30.10.98 12:44-------------------
 *
 * --------------------------------------------------*/
void SvxNumRule::SetLevel(sal_uInt16 nLevel, const SvxNumberFormat* pFmt)
{
    DBG_ASSERT(nLevel < SVX_MAX_NUM, "falsches Level" );

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
/* -----------------28.10.98 15:38-------------------
 *
 * --------------------------------------------------*/
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
                1 < rMyNFmt.GetIncludeUpperLevels() )       // nur der eigene Level ?
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
    // Soll aus 1.1.1 --> 2. NoNum --> 1..1 oder 1.1 ??
    //                 if( i != rNum.nMyLevel )
    //                    aStr += aDotStr;
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
                    aStr += sal_Unicode('0');       // alle 0-Level sind eine 0
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
/* -----------------18.08.99 10:18-------------------
    Description: changes linked to embedded bitmaps
 --------------------------------------------------*/
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

/* -----------------27.10.98 10:41-------------------
 *
 * --------------------------------------------------*/
SvxNumBulletItem::SvxNumBulletItem(SvxNumRule& rRule) :
    SfxPoolItem(SID_ATTR_NUMBERING_RULE),
    pNumRule(new SvxNumRule(rRule))
{
}

/*-----------------23.11.98 10:36-------------------
 MT: Das sind ja sehr sinnige Kommentare...
--------------------------------------------------*/
SvxNumBulletItem::SvxNumBulletItem(SvxNumRule& rRule, sal_uInt16 _nWhich ) :
    SfxPoolItem(_nWhich),
    pNumRule(new SvxNumRule(rRule))
{
}

SfxPoolItem* SvxNumBulletItem::Create(SvStream &s, sal_uInt16 n) const
{
    return SfxPoolItem::Create(s, n );
}

/* -----------------27.10.98 10:41-------------------
 *
 * --------------------------------------------------*/
SvxNumBulletItem::SvxNumBulletItem(const SvxNumBulletItem& rCopy) :
    SfxPoolItem(rCopy.Which())
{
    pNumRule = new SvxNumRule(*rCopy.pNumRule);
}
/* -----------------27.10.98 10:41-------------------
 *
 * --------------------------------------------------*/
SvxNumBulletItem::~SvxNumBulletItem()
{
    delete pNumRule;
}

/* -----------------27.10.98 10:41-------------------
 *
 * --------------------------------------------------*/
int  SvxNumBulletItem::operator==( const SfxPoolItem& rCopy) const
{
    return *pNumRule == *((SvxNumBulletItem&)rCopy).pNumRule;
}
/* -----------------27.10.98 10:41-------------------
 *
 * --------------------------------------------------*/
SfxPoolItem*  SvxNumBulletItem::Clone( SfxItemPool * ) const
{
    return new SvxNumBulletItem(*this);
}
/* -----------------08.12.98 10:43-------------------
 *
 * --------------------------------------------------*/
sal_uInt16  SvxNumBulletItem::GetVersion( sal_uInt16 /*nFileVersion*/ ) const
{
    return NUMITEM_VERSION_03;
}
/* -----------------08.12.98 10:43-------------------
 *
 * --------------------------------------------------*/
SvStream&   SvxNumBulletItem::Store(SvStream &rStream, sal_uInt16 /*nItemVersion*/ )const
{
    pNumRule->Store(rStream);
    return rStream;
}

/* -----------------08.12.98 10:43-------------------
 *
 * --------------------------------------------------*/

sal_Bool SvxNumBulletItem::QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/ ) const
{
    rVal <<= SvxCreateNumRule( pNumRule );
    return sal_True;
}

sal_Bool SvxNumBulletItem::PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 /*nMemberId*/ )
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
            return sal_True;
        }
        catch(lang::IllegalArgumentException&)
        {
        }
    }
    return sal_False;
}

/* -----------------08.12.98 10:43-------------------
 *
 * --------------------------------------------------*/
SvxNumRule* SvxConvertNumRule( const SvxNumRule* pRule, sal_uInt16 nLevels, SvxNumRuleType eType )
{
    const sal_uInt16 nSrcLevels = pRule->GetLevelCount();
    SvxNumRule* pNewRule = new SvxNumRule( pRule->GetFeatureFlags(), nLevels, pRule->IsContinuousNumbering(), eType );

    for( sal_uInt16 nLevel = 0; (nLevel < nLevels) && (nLevel < nSrcLevels); nLevel++ )
        pNewRule->SetLevel( nLevel, pRule->GetLevel( nLevel ) );

    return pNewRule;
}
