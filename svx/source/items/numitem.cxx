/*************************************************************************
 *
 *  $RCSfile: numitem.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: mt $ $Date: 2001-03-22 14:59:59 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#pragma hdrstop
#include <numitem.hxx>

#define ITEMID_BRUSH SID_ATTR_BRUSH
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <brshitem.hxx>
#endif
#ifndef _SV_FONT_HXX //autogen
#include <vcl/font.hxx>
#endif
#ifndef _SVX_SVXIDS_HRC
#include <svxids.hrc>
#endif
#ifndef _SVX_NUMDEF_HXX
#include <numdef.hxx>
#endif
#ifndef _SV_GRAPH_HXX //autogen
#include <vcl/graph.hxx>
#endif
#ifndef _SV_WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif
#include <vcl/svapp.hxx>
#ifndef _SV_SYSTEM_HXX //autogen
#include <vcl/system.hxx>
#endif
#ifndef _UNO_LINGU_HXX
#include <unolingu.hxx>
#endif
#ifndef _COM_SUN_STAR_TEXT_XNUMBERINGFORMATTER_HPP_
#include <com/sun/star/text/XNumberingFormatter.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XDEFAULTNUMBERINGPROVIDER_HPP_
#include <com/sun/star/text/XDefaultNumberingProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#include "unonrule.hxx"

#define MM100_TO_TWIP(MM100)    ((MM100*72L+63L)/127L)

#define DEF_WRITER_LSPACE   500     //Standardeinrueckung
#define DEF_DRAW_LSPACE     800     //Standardeinrueckung

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::beans;

sal_Int32 SvxNumberType::nRefCount = 0;
com::sun::star::uno::Reference<com::sun::star::text::XNumberingFormatter> SvxNumberType::xFormatter = 0;
/* -----------------------------22.02.01 14:24--------------------------------

 ---------------------------------------------------------------------------*/
SvxNumberType::SvxNumberType(sal_Int16 nType) :
    nNumType(nType),
    bShowSymbol(sal_True)
{
    if(!xFormatter.is())
       {
        try
        {
            Reference< XMultiServiceFactory > xMSF = ::comphelper::getProcessServiceFactory();
            Reference < XInterface > xI = xMSF->createInstance(
                ::rtl::OUString::createFromAscii( "com.sun.star.text.DefaultNumberingProvider" ) );
            Reference<XDefaultNumberingProvider> xRet(xI, UNO_QUERY);
            DBG_ASSERT(xRet.is(), "service missing: \"com.sun.star.text.DefaultNumberingProvider\"")
            xFormatter = Reference<XNumberingFormatter> (xRet, UNO_QUERY);
        }
        catch(Exception& )
        {
        }
    }
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
String SvxNumberType::GetNumStr( ULONG nNo ) const
{
    LanguageType eLang = System::GetLanguage();
    if(LANGUAGE_SYSTEM == eLang)
        eLang = ::GetSystemLanguage();
    Locale aLocale = SvxCreateLocale(eLang);
    return GetNumStr( nNo, aLocale );
}
/* -----------------28.10.98 15:56-------------------
 *
 * --------------------------------------------------*/
String  SvxNumberType::GetNumStr( ULONG nNo, const Locale& rLocale ) const
{
    String aTmpStr;
    if(!xFormatter.is())
        return aTmpStr;

    if(bShowSymbol)
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
    return aTmpStr;
}
/* -----------------27.10.98 10:33-------------------
 *
 * --------------------------------------------------*/
SvxNumberFormat::SvxNumberFormat(sal_Int16 eType) :
    SvxNumberType(eType),
    eNumAdjust(SVX_ADJUST_LEFT),
    nInclUpperLevels(0),
    nStart(1),
    cBullet(SVX_DEF_BULLET),
    nFirstLineOffset(0),
    nAbsLSpace(0),
    nLSpace(0),
    nCharTextDistance(0),
    pGraphicBrush(0),
    eVertOrient(SVX_VERT_NONE),
    nBulletRelSize(100),
    nBulletColor(COL_BLACK),
    pBulletFont(0)
{
}
/* -----------------27.10.98 10:56-------------------
 *
 * --------------------------------------------------*/
SvxNumberFormat::SvxNumberFormat(const SvxNumberFormat& rFormat) :
    SvxNumberType(rFormat)
{
    eNumAdjust          = rFormat.eNumAdjust;
    nInclUpperLevels    = rFormat.nInclUpperLevels;
    nStart              = rFormat.nStart;
    cBullet             = rFormat.cBullet;
    nFirstLineOffset    = rFormat.nFirstLineOffset;
    nAbsLSpace          = rFormat.nAbsLSpace;
    nLSpace             = rFormat.nLSpace;
    nCharTextDistance   = rFormat.nCharTextDistance;
    sPrefix             = rFormat.sPrefix;
    sSuffix             = rFormat.sSuffix;
    sCharStyleName      = rFormat.sCharStyleName;
    pGraphicBrush       = rFormat.pGraphicBrush ? new SvxBrushItem(*rFormat.pGraphicBrush) : 0;
    eVertOrient         = rFormat.eVertOrient;
    pBulletFont         = rFormat.pBulletFont  ? new Font(*rFormat.pBulletFont) : 0;
    aGraphicSize        = rFormat.aGraphicSize;
    nBulletColor        = rFormat.nBulletColor;
    nBulletRelSize      = rFormat.nBulletRelSize;
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
{
    USHORT nVersion;
    rStream >> nVersion;

    USHORT nUSHORT;
    rStream >> nUSHORT;
    SetNumberingType((sal_Int16)nUSHORT);
    rStream >> nUSHORT;
    eNumAdjust      = (SvxAdjust)nUSHORT;
    rStream >> nUSHORT;
    nInclUpperLevels = (BYTE)nUSHORT;
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
        SvxBrushItem aHelper;
        pGraphicBrush = (SvxBrushItem*) aHelper.Create( rStream, BRUSH_GRAPHIC_VERSION );
    }
    else
        pGraphicBrush = 0;

    rStream >> nUSHORT;
    eVertOrient          = (SvxFrameVertOrient)nUSHORT;

    rStream >> nUSHORT;
    if(nUSHORT)
    {
        pBulletFont = new Font;
        rStream >> *pBulletFont;
    }
    else
        pBulletFont = 0;
    rStream >> aGraphicSize;

    rStream >> nBulletColor;
    rStream >> nUSHORT;
    nBulletRelSize = nUSHORT;
    rStream >> nUSHORT;
    SetShowSymbol((BOOL)nUSHORT);

    if( nVersion < NUMITEM_VERSION_03 )
        cBullet = ByteString::ConvertToUnicode( cBullet,
                            (pBulletFont&&pBulletFont->GetCharSet()) ?  pBulletFont->GetCharSet()
                                        : RTL_TEXTENCODING_SYMBOL );
}
/* -----------------08.12.98 11:14-------------------
 *
 * --------------------------------------------------*/
SvStream&   SvxNumberFormat::Store(SvStream &rStream)
{
    rStream << (USHORT)NUMITEM_VERSION_03;

    rStream << (USHORT)GetNumberingType();
    rStream << (USHORT)eNumAdjust;
    rStream << (USHORT)nInclUpperLevels;
    rStream << nStart;
    rStream << (USHORT)cBullet;

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
        rStream << (USHORT)1;

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
        rStream << (USHORT)0;

    rStream << (USHORT)eVertOrient;
    if(pBulletFont)
    {
        rStream << (USHORT)1;
        rStream << *pBulletFont;
    }
    else
        rStream << (USHORT)0;
    rStream << aGraphicSize;

    rStream << nBulletColor;
    rStream << nBulletRelSize;
    rStream << (USHORT)IsShowSymbol();
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
        nFirstLineOffset    = rFormat.nFirstLineOffset;
        nAbsLSpace          = rFormat.nAbsLSpace ;
        nLSpace             = rFormat.nLSpace ;
        nCharTextDistance   = rFormat.nCharTextDistance ;
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
        pGraphicBrush = new SvxBrushItem(*rFormat.pGraphicBrush);
    DELETEZ(pBulletFont);
    if(rFormat.pBulletFont)
            pBulletFont = new Font(*rFormat.pBulletFont);
    return *this;
}
/* -----------------27.10.98 10:56-------------------
 *
 * --------------------------------------------------*/
BOOL  SvxNumberFormat::operator==( const SvxNumberFormat& rFormat) const
{
    if( GetNumberingType()  != rFormat.GetNumberingType() ||
        eNumAdjust          != rFormat.eNumAdjust ||
        nInclUpperLevels    != rFormat.nInclUpperLevels ||
        nStart              != rFormat.nStart ||
        cBullet             != rFormat.cBullet ||
        nFirstLineOffset    != rFormat.nFirstLineOffset ||
        nAbsLSpace          != rFormat.nAbsLSpace ||
        nLSpace             != rFormat.nLSpace ||
        nCharTextDistance   != rFormat.nCharTextDistance ||
        eVertOrient         != rFormat.eVertOrient ||
        sPrefix             != rFormat.sPrefix     ||
        sSuffix             != rFormat.sSuffix     ||
        aGraphicSize        != rFormat.aGraphicSize  ||
        nBulletColor        != rFormat.nBulletColor   ||
        nBulletRelSize      != rFormat.nBulletRelSize ||
        IsShowSymbol()      != rFormat.IsShowSymbol() ||
        sCharStyleName      != rFormat.sCharStyleName
        )
        return FALSE;
    if(pGraphicBrush && !rFormat.pGraphicBrush ||
            !pGraphicBrush && rFormat.pGraphicBrush ||
                pGraphicBrush && *pGraphicBrush != *rFormat.pGraphicBrush )
        return FALSE;
    if(pBulletFont && !rFormat.pBulletFont ||
            !pBulletFont && rFormat.pBulletFont ||
                pBulletFont && *pBulletFont != *rFormat.pBulletFont)
        return FALSE;
    return TRUE;
}
/* -----------------28.10.98 09:53-------------------
 *
 * --------------------------------------------------*/
void SvxNumberFormat::SetGraphicBrush( const SvxBrushItem* pBrushItem,
                    const Size* pSize, const SvxFrameVertOrient* pOrient)
{
    if(!pBrushItem)
    {
        delete pGraphicBrush;
        pGraphicBrush = 0;
    }
    else if(!pGraphicBrush || pGraphicBrush && !(*pBrushItem == *pGraphicBrush))
    {
        delete pGraphicBrush;
        pGraphicBrush =  (SvxBrushItem*)pBrushItem->Clone();
    }

    if(pOrient)
        eVertOrient = *pOrient;
    else
        eVertOrient = SVX_VERT_NONE;
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
    pGraphicBrush = new SvxBrushItem( rName, sTmp, GPOS_AREA );
    pGraphicBrush->SetDoneLink( STATIC_LINK( this, SvxNumberFormat, GraphicArrived) );
    if( eVertOrient == SVX_VERT_NONE )
        eVertOrient = SVX_VERT_TOP;

    aGraphicSize.Width() = aGraphicSize.Height() = 0;
}
/* -----------------------------22.02.01 15:55--------------------------------

 ---------------------------------------------------------------------------*/
void SvxNumberFormat::SetVertOrient(SvxFrameVertOrient eSet)
{
    eVertOrient = eSet;
}
/* -----------------------------22.02.01 15:55--------------------------------

 ---------------------------------------------------------------------------*/
SvxFrameVertOrient  SvxNumberFormat::GetVertOrient() const
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
    return 0;
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
String SvxNumberFormat::CreateRomanString( ULONG nNo, BOOL bUpper )
{
    nNo %= 4000;            // mehr kann nicht dargestellt werden
//      i, ii, iii, iv, v, vi, vii, vii, viii, ix
//                          (Dummy),1000,500,100,50,10,5,1
    char *cRomanArr = bUpper
                        ? "MDCLXVI--"   // +2 Dummy-Eintraege !!
                        : "mdclxvi--";  // +2 Dummy-Eintraege !!

    String sRet;
    USHORT nMask = 1000;
    while( nMask )
    {
        BYTE nZahl = BYTE(nNo / nMask);
        BYTE nDiff = 1;
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
void SvxNumberFormat::GetCharStr( ULONG nNo, String& rStr ) const
{
    DBG_ASSERT( nNo, "0 ist eine ungueltige Nummer !!" );

    const ULONG coDiff = 'Z' - 'A' +1;
    char cAdd = (SVX_NUM_CHARS_UPPER_LETTER == eNumType ? 'A' : 'a') - 1;
    ULONG nCalc;

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

void SvxNumberFormat::GetCharStrN( ULONG nNo, String& rStr ) const
{
    DBG_ASSERT( nNo, "0 ist eine ungueltige Nummer !!" );

    const ULONG coDiff = 'Z' - 'A' +1;
    char cChar = (char)(--nNo % coDiff);
    if( SVX_NUM_CHARS_UPPER_LETTER_N == eNumType )
        cChar += 'A';
    else
        cChar += 'a';

    rStr.Fill( (USHORT)(nNo / coDiff) + 1, sal_Unicode(cChar) );
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
SvxNumRule::SvxNumRule(ULONG nFeatures, USHORT nLevels, BOOL bCont, SvxNumRuleType eType) :
    nLevelCount(nLevels),
    nFeatureFlags(nFeatures),
    bContinuousNumbering(bCont),
    eNumberingType(eType)
{
    ++nRefCount;
    LanguageType eLang = System::GetLanguage();
    if(LANGUAGE_SYSTEM == eLang)
        eLang = ::GetSystemLanguage();
    aLocale = SvxCreateLocale(eLang);
    for(USHORT i = 0; i < SVX_MAX_NUM; i++)
    {
        if(i < nLevels)
        {
            aFmts[i] = new SvxNumberFormat(SVX_NUM_CHARS_UPPER_LETTER);
            //daran wird zwischen writer und draw unterschieden
            if(bContinuousNumbering)
            {
                aFmts[i]->SetLSpace( MM100_TO_TWIP(DEF_WRITER_LSPACE) );
                aFmts[i]->SetAbsLSpace( MM100_TO_TWIP(DEF_WRITER_LSPACE * (i+1)) );
            }
            else
            {
                aFmts[i]->SetLSpace( DEF_DRAW_LSPACE );
                aFmts[i]->SetAbsLSpace( DEF_DRAW_LSPACE * (i) );
            }
        }
        else
            aFmts[i] = 0;
        aFmtsSet[i] = FALSE;
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
    for(USHORT i = 0; i < SVX_MAX_NUM; i++)
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
    LanguageType eLang = System::GetLanguage();
    if(LANGUAGE_SYSTEM == eLang)
        eLang = ::GetSystemLanguage();
    aLocale = SvxCreateLocale(eLang);
    USHORT nVersion;
    USHORT nTemp;
    rStream >> nVersion;
    rStream >> nLevelCount;
    rStream >> nTemp;
    nFeatureFlags = nTemp;
    rStream >> nTemp;
    bContinuousNumbering = (BOOL)nTemp;
    rStream >> nTemp;
    eNumberingType       = (SvxNumRuleType)nTemp;
    memset( aFmts, 0, sizeof( aFmts ));

    for(USHORT i = 0; i < SVX_MAX_NUM; i++)
    {
        USHORT nSet;
        rStream >> nSet;
        if(nSet)
            aFmts[i] = new SvxNumberFormat(rStream);
        else
            aFmts[i] = 0;
        aFmtsSet[i] = aFmts[i] ? TRUE : FALSE;
    }
    if(NUMITEM_VERSION_02 <= nVersion)
    {
        USHORT nShort;
        rStream >> nShort;
        nFeatureFlags = nShort;
    }
}

/* -----------------08.12.98 11:07-------------------
 *
 * --------------------------------------------------*/
SvStream&   SvxNumRule::Store(SvStream &rStream)
{
    rStream<<(USHORT)NUMITEM_VERSION_03;
    rStream<<nLevelCount;
    //first save of nFeatureFlags for old versions
    rStream<<(USHORT)nFeatureFlags;
    rStream<<(USHORT)bContinuousNumbering;
    rStream<<(USHORT)eNumberingType;

    for(USHORT i = 0; i < SVX_MAX_NUM; i++)
    {
        if(aFmts[i])
        {
            rStream << USHORT(1);
            aFmts[i]->Store(rStream);
        }
        else
            rStream << USHORT(0);
    }
    //second save of nFeatureFlags for new versions
    rStream<<(USHORT)nFeatureFlags;

    return rStream;
}

/* -----------------27.10.98 10:41-------------------
 *
 * --------------------------------------------------*/
SvxNumRule::~SvxNumRule()
{
    for(USHORT i = 0; i < SVX_MAX_NUM; i++)
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
    for(USHORT i = 0; i < SVX_MAX_NUM; i++)
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
            return FALSE;
    for(USHORT i = 0; i < nLevelCount; i++)
    {
        if( aFmtsSet[i] != rCopy.aFmtsSet[i] ||
            !aFmts[i] &&  rCopy.aFmts[i] ||
            aFmts[i] &&  !rCopy.aFmts[i] ||
            aFmts[i] && *aFmts[i] !=  *rCopy.aFmts[i] )
            return FALSE;
    }
    return TRUE;
}
/* -----------------27.10.98 10:41-------------------
 *
 * --------------------------------------------------*/
const SvxNumberFormat*  SvxNumRule::Get(USHORT nLevel)const
{
    DBG_ASSERT(nLevel < SVX_MAX_NUM, "falsches Level" )
    return aFmtsSet[nLevel] ? aFmts[nLevel] : 0;
}
/* -----------------02.11.98 09:10-------------------
 *
 * --------------------------------------------------*/
const SvxNumberFormat&  SvxNumRule::GetLevel(USHORT nLevel)const
{
    if(!pStdNumFmt)
    {
        pStdNumFmt = new SvxNumberFormat(SVX_NUM_ARABIC);
         pStdOutlineNumFmt = new SvxNumberFormat(SVX_NUM_NUMBER_NONE);
    }

    DBG_ASSERT(nLevel < SVX_MAX_NUM, "falsches Level" )
    return aFmts[nLevel] ?
        *aFmts[nLevel] :  eNumberingType == SVX_RULETYPE_NUMBERING ?
                                                    *pStdNumFmt : *pStdOutlineNumFmt;
}

/* -----------------29.10.98 09:08-------------------
 *
 * --------------------------------------------------*/
void SvxNumRule::SetLevel( USHORT i, const SvxNumberFormat& rNumFmt, BOOL bIsValid )
{
    if( !aFmtsSet[i] || !(rNumFmt == *Get( i )) )
    {
        delete aFmts[ i ];
        aFmts[ i ] = new SvxNumberFormat( rNumFmt );
        aFmtsSet[i] = bIsValid;
//      bInvalidRuleFlag = TRUE;
    }
}
/* -----------------30.10.98 12:44-------------------
 *
 * --------------------------------------------------*/
void SvxNumRule::SetLevel(USHORT nLevel, const SvxNumberFormat* pFmt)
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
/* -----------------28.10.98 15:38-------------------
 *
 * --------------------------------------------------*/
String  SvxNumRule::MakeNumString( const SvxNodeNum& rNum, BOOL bInclStrings ) const
{
    String aStr;
    if( SVX_NO_NUM > rNum.GetLevel() && !( SVX_NO_NUMLEVEL & rNum.GetLevel() ) )
    {
        const SvxNumberFormat& rMyNFmt = GetLevel( rNum.GetLevel() );
        if( SVX_NUM_NUMBER_NONE != rMyNFmt.GetNumberingType() )
        {
            BYTE i = rNum.GetLevel();

            if( !IsContinuousNumbering() &&
                1 < rMyNFmt.GetIncludeUpperLevels() )       // nur der eigene Level ?
            {
                BYTE n = rMyNFmt.GetIncludeUpperLevels();
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

                if( rNum.GetLevelVal()[ i ] )
                    aStr += rNFmt.GetNumStr( rNum.GetLevelVal()[ i ], aLocale );
                else
                    aStr += sal_Unicode('0');       // alle 0-Level sind eine 0
                if( i != rNum.GetLevel() )
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
BOOL SvxNumRule::UnLinkGraphics()
{
    BOOL bRet = FALSE;
    for(USHORT i = 0; i < GetLevelCount(); i++)
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
                SvxFrameVertOrient  eOrient = aFmt.GetVertOrient();
                aFmt.SetGraphicBrush( &aTempItem, &aFmt.GetGraphicSize(), &eOrient );
                bRet = TRUE;
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
SvxNumBulletItem::SvxNumBulletItem(SvxNumRule& rRule, USHORT nWhich ) :
    SfxPoolItem(nWhich),
    pNumRule(new SvxNumRule(rRule))
{
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
SfxPoolItem*  SvxNumBulletItem::Clone( SfxItemPool *pPool ) const
{
    return new SvxNumBulletItem(*this);
}
/* -----------------08.12.98 10:43-------------------
 *
 * --------------------------------------------------*/
SfxPoolItem*     SvxNumBulletItem::Create(SvStream &rStream, USHORT) const
{
    SvxNumRule aRule(rStream);
    return new SvxNumBulletItem(aRule, Which() );
}
/* -----------------08.12.98 10:43-------------------
 *
 * --------------------------------------------------*/
SvStream&   SvxNumBulletItem::Store(SvStream &rStream, USHORT nItemVersion )const
{
    pNumRule->Store(rStream);
    return rStream;
}
/* -----------------08.12.98 10:43-------------------
 *
 * --------------------------------------------------*/
USHORT  SvxNumBulletItem::GetVersion( USHORT nFileVersion ) const
{
    return NUMITEM_VERSION_03;
}

/* -----------------08.12.98 10:43-------------------
 *
 * --------------------------------------------------*/

sal_Bool SvxNumBulletItem::QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
{
    rVal <<= SvxCreateNumRule( pNumRule );
    return sal_True;
}

sal_Bool SvxNumBulletItem::PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId )
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
        catch(...)
        {
        }
    }
    return sal_False;
}

/* -----------------08.12.98 10:43-------------------
 *
 * --------------------------------------------------*/
SvxNumRule* SvxConvertNumRule( const SvxNumRule* pRule, USHORT nLevels, SvxNumRuleType eType )
{
    const USHORT nSrcLevels = pRule->GetLevelCount();
    SvxNumRule* pNewRule = new SvxNumRule( pRule->GetFeatureFlags(), nLevels, pRule->IsContinuousNumbering(), eType );

    // move all levels one level up if the destination is a presentation numbering and the source is not
    const sal_Bool bConvertUp = pRule->GetNumRuleType() != SVX_RULETYPE_PRESENTATION_NUMBERING &&
                                  eType == SVX_RULETYPE_PRESENTATION_NUMBERING;

    // move all levels one level down if the source is a presentation numbering and the destination is not
    const sal_Bool bConvertDown = pRule->GetNumRuleType() == SVX_RULETYPE_PRESENTATION_NUMBERING &&
                                  eType != SVX_RULETYPE_PRESENTATION_NUMBERING;

    USHORT nSrcLevel = bConvertDown ? 1 : 0;
    USHORT nDstLevel = bConvertUp ? 1 : 0;
    for( ; (nDstLevel < nLevels) && (nSrcLevel < nSrcLevels); nSrcLevel++, nDstLevel++ )
        pNewRule->SetLevel( nDstLevel, pRule->GetLevel( nSrcLevel ) );

    return pNewRule;
}
