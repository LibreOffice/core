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
#include "precompiled_svtools.hxx"

#include <svtools/htmlcfg.hxx>
#include <svtools/parhtml.hxx>
#include <unotools/syslocale.hxx>
#include <tools/debug.hxx>
#include <tools/list.hxx>
#include <tools/link.hxx>

// -----------------------------------------------------------------------
#define HTMLCFG_UNKNOWN_TAGS            0x01
//#define HTMLCFG_STYLE_SHEETS          0x02
//#define HTMLCFG_NETSCAPE3             0x04
#define HTMLCFG_STAR_BASIC              0x08
#define HTMLCFG_LOCAL_GRF               0x10
#define HTMLCFG_PRINT_LAYOUT_EXTENSION  0x20
#define HTMLCFG_IGNORE_FONT_FAMILY      0x40
#define HTMLCFG_IS_BASIC_WARNING        0x80
#define HTMLCFG_NUMBERS_ENGLISH_US      0x100

using namespace utl;
using namespace rtl;
using namespace com::sun::star::uno;

static SvxHtmlOptions* pOptions = 0;

DECLARE_LIST( LinkList, Link * )

#define C2U(cChar) OUString::createFromAscii(cChar)
/* -----------------------------23.11.00 11:39--------------------------------

 ---------------------------------------------------------------------------*/
struct HtmlOptions_Impl
{
    LinkList    aList;
    sal_Int32   nFlags;
    sal_Int32   nExportMode;
    sal_Int32   aFontSizeArr[HTML_FONT_COUNT];
    sal_Int32   eEncoding;
    sal_Bool    bIsEncodingDefault;

    HtmlOptions_Impl() :
        nFlags(HTMLCFG_LOCAL_GRF|HTMLCFG_IS_BASIC_WARNING),
        nExportMode(HTML_CFG_NS40),
        eEncoding( gsl_getSystemTextEncoding() ),
        bIsEncodingDefault(sal_True)
    {
        aFontSizeArr[0] = HTMLFONTSZ1_DFLT;
        aFontSizeArr[1] = HTMLFONTSZ2_DFLT;
        aFontSizeArr[2] = HTMLFONTSZ3_DFLT;
        aFontSizeArr[3] = HTMLFONTSZ4_DFLT;
        aFontSizeArr[4] = HTMLFONTSZ5_DFLT;
        aFontSizeArr[5] = HTMLFONTSZ6_DFLT;
        aFontSizeArr[6] = HTMLFONTSZ7_DFLT;
    }
};

/* -----------------------------23.11.00 11:39--------------------------------

 ---------------------------------------------------------------------------*/
const Sequence<OUString>& SvxHtmlOptions::GetPropertyNames()
{
    static Sequence<OUString> aNames;
    if(!aNames.getLength())
    {
        static const char* aPropNames[] =
        {
            "Import/UnknownTag",                    //  0
            "Import/FontSetting",                   //  1
            "Import/FontSize/Size_1",               //  2
            "Import/FontSize/Size_2",               //  3
            "Import/FontSize/Size_3",               //  4
            "Import/FontSize/Size_4",               //  5
            "Import/FontSize/Size_5",               //  6
            "Import/FontSize/Size_6",               //  7
            "Import/FontSize/Size_7",               //  8
            "Export/Browser",                       //  9
            "Export/Basic",                         //  0
            "Export/PrintLayout",                   // 11
            "Export/LocalGraphic",                  // 12
            "Export/Warning",                       // 13
            "Export/Encoding",                      // 14
            "Import/NumbersEnglishUS"               // 15
        };
        const int nCount = sizeof(aPropNames) / sizeof(aPropNames[0]);
        aNames.realloc(nCount);
        OUString* pNames = aNames.getArray();
        for(int i = 0; i < nCount; i++)
            pNames[i] = C2U(aPropNames[i]);
    }
    return aNames;
}
// -----------------------------------------------------------------------
SvxHtmlOptions::SvxHtmlOptions() :
    ConfigItem(C2U("Office.Common/Filter/HTML"))
{
    pImp = new HtmlOptions_Impl;
    Load( GetPropertyNames() );
}

// -----------------------------------------------------------------------
SvxHtmlOptions::~SvxHtmlOptions()
{
    delete pImp;
}

void SvxHtmlOptions::Load( const Sequence< OUString >& aNames )
{
    Sequence<Any> aValues = GetProperties(aNames);
    const Any* pValues = aValues.getConstArray();
    DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() == aNames.getLength())
    {
        pImp->nFlags = 0;
        for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        {
            if(pValues[nProp].hasValue())
            {
                switch(nProp)
                {
                    case  0:
                        if(*(sal_Bool*)pValues[nProp].getValue())
                            pImp->nFlags |= HTMLCFG_UNKNOWN_TAGS;
                    break;//"Import/UnknownTag",
                    case  1:
                        if(*(sal_Bool*)pValues[nProp].getValue())
                            pImp->nFlags |= HTMLCFG_IGNORE_FONT_FAMILY;
                    break;//"Import/FontSetting",
                    case  2: pValues[nProp] >>= pImp->aFontSizeArr[0]; break;//"Import/FontSize/Size_1",
                    case  3: pValues[nProp] >>= pImp->aFontSizeArr[1]; break;//"Import/FontSize/Size_2",
                    case  4: pValues[nProp] >>= pImp->aFontSizeArr[2]; break;//"Import/FontSize/Size_3",
                    case  5: pValues[nProp] >>= pImp->aFontSizeArr[3]; break;//"Import/FontSize/Size_4",
                    case  6: pValues[nProp] >>= pImp->aFontSizeArr[4]; break;//"Import/FontSize/Size_5",
                    case  7: pValues[nProp] >>= pImp->aFontSizeArr[5]; break;//"Import/FontSize/Size_6",
                    case  8: pValues[nProp] >>= pImp->aFontSizeArr[6]; break;//"Import/FontSize/Size_7",
                    case  9://"Export/Browser",
                        {
                            sal_Int32 nExpMode = 0;
//                          pValues[nProp] >>= pImp->nExportMode;
                            pValues[nProp] >>= nExpMode;
                            switch( nExpMode )
                            {
                                case 0:     nExpMode = HTML_CFG_HTML32;     break;
                                case 1:     nExpMode = HTML_CFG_MSIE_40;    break;
//                              case 2:     nExpMode = HTML_CFG_NS30;       break;  depricated
                                case 3:     nExpMode = HTML_CFG_WRITER;     break;
                                case 4:     nExpMode = HTML_CFG_NS40;       break;
                                case 5:     nExpMode = HTML_CFG_MSIE_40_OLD;break;
                                default:    nExpMode = HTML_CFG_NS40;       break;
                            }

                            pImp->nExportMode = nExpMode;
                        }
                        break;
                    case 10:
                        if(*(sal_Bool*)pValues[nProp].getValue())
                            pImp->nFlags |= HTMLCFG_STAR_BASIC;
                    break;//"Export/Basic",
                    case 11:
                        if(*(sal_Bool*)pValues[nProp].getValue())
                            pImp->nFlags |= HTMLCFG_PRINT_LAYOUT_EXTENSION;
                    break;//"Export/PrintLayout",
                    case 12:
                        if(*(sal_Bool*)pValues[nProp].getValue())
                            pImp->nFlags |= HTMLCFG_LOCAL_GRF;
                    break;//"Export/LocalGraphic",
                    case 13:
                        if(*(sal_Bool*)pValues[nProp].getValue())
                            pImp->nFlags |= HTMLCFG_IS_BASIC_WARNING;
                    break;//"Export/Warning"

                    case 14: pValues[nProp] >>= pImp->eEncoding;
                             pImp->bIsEncodingDefault = sal_False;
                    break;//"Export/Encoding"

                    case 15:
                        if(*(sal_Bool*)pValues[nProp].getValue())
                            pImp->nFlags |= HTMLCFG_NUMBERS_ENGLISH_US;
                    break;//"Import/NumbersEnglishUS"
                }
            }
        }
    }
}

// -----------------------------------------------------------------------
void    SvxHtmlOptions::Commit()
{
    const Sequence<OUString>& aNames = GetPropertyNames();

//  const OUString* pNames = aNames.getConstArray();
    Sequence<Any> aValues(aNames.getLength());
    Any* pValues = aValues.getArray();

//  const Type& rType = ::getBooleanCppuType();
    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        sal_Bool bSet = sal_False;
        switch(nProp)
        {
            case  0: bSet = 0 != (pImp->nFlags & HTMLCFG_UNKNOWN_TAGS);break;//"Import/UnknownTag",
            case  1: bSet = 0 != (pImp->nFlags & HTMLCFG_IGNORE_FONT_FAMILY);break;//"Import/FontSetting",
            case  2: pValues[nProp] <<= pImp->aFontSizeArr[0];break;//"Import/FontSize/Size_1",
            case  3: pValues[nProp] <<= pImp->aFontSizeArr[1];break;//"Import/FontSize/Size_2",
            case  4: pValues[nProp] <<= pImp->aFontSizeArr[2];break;//"Import/FontSize/Size_3",
            case  5: pValues[nProp] <<= pImp->aFontSizeArr[3];break;//"Import/FontSize/Size_4",
            case  6: pValues[nProp] <<= pImp->aFontSizeArr[4];break;//"Import/FontSize/Size_5",
            case  7: pValues[nProp] <<= pImp->aFontSizeArr[5];break;//"Import/FontSize/Size_6",
            case  8: pValues[nProp] <<= pImp->aFontSizeArr[6];break;//"Import/FontSize/Size_7",
            case  9:                //"Export/Browser",
                {
                    sal_Int32 nExpMode = pImp->nExportMode;

                    switch( nExpMode )
                    {
                        case HTML_CFG_HTML32:       nExpMode = 0;   break;
                        case HTML_CFG_MSIE_40:      nExpMode = 1;   break;
//                      case HTML_CFG_NS30:         nExpMode = 2;   break;  depricated
                        case HTML_CFG_WRITER:       nExpMode = 3;   break;
                        case HTML_CFG_NS40:         nExpMode = 4;   break;
                        case HTML_CFG_MSIE_40_OLD:  nExpMode = 5;   break;
                        default:                    nExpMode = 4;   break;  // NS40
                    }

                    pValues[nProp] <<= nExpMode;
                    break;
                }
            case 10: bSet = 0 != (pImp->nFlags & HTMLCFG_STAR_BASIC);break;//"Export/Basic",
            case 11: bSet = 0 != (pImp->nFlags & HTMLCFG_PRINT_LAYOUT_EXTENSION);break;//"Export/PrintLayout",
            case 12: bSet = 0 != (pImp->nFlags & HTMLCFG_LOCAL_GRF);break;//"Export/LocalGraphic",
            case 13: bSet = 0 != (pImp->nFlags & HTMLCFG_IS_BASIC_WARNING);break;//"Export/Warning"
            case 14:
                if(!pImp->bIsEncodingDefault)
                    pValues[nProp] <<= pImp->eEncoding;
                break;//"Export/Encoding",
            case 15: bSet = 0 != (pImp->nFlags & HTMLCFG_NUMBERS_ENGLISH_US);break;//"Import/NumbersEnglishUS"
        }
        if(nProp < 2 || ( nProp > 9 && nProp < 14 ) || nProp == 15)
            pValues[nProp].setValue(&bSet, ::getCppuBooleanType());
    }
    PutProperties(aNames, aValues);
}

void SvxHtmlOptions::AddListenerLink( const Link& rLink )
{
    pImp->aList.Insert( new Link( rLink ) );
}

void SvxHtmlOptions::RemoveListenerLink( const Link& rLink )
{
    for ( sal_uInt16 n=0; n<pImp->aList.Count(); n++ )
    {
        if ( (*pImp->aList.GetObject(n) ) == rLink )
        {
            delete pImp->aList.Remove(n);
            break;
        }
    }
}

void SvxHtmlOptions::CallListeners()
{
    for ( sal_uInt16 n = 0; n < pImp->aList.Count(); ++n )
        pImp->aList.GetObject(n)->Call( this );
}


void SvxHtmlOptions::Notify( const com::sun::star::uno::Sequence< rtl::OUString >& )
{
    Load( GetPropertyNames() );
    CallListeners();
}

// -----------------------------------------------------------------------
sal_uInt16  SvxHtmlOptions::GetFontSize(sal_uInt16 nPos) const
{
    if(nPos < HTML_FONT_COUNT)
        return (sal_uInt16)pImp->aFontSizeArr[nPos];
    return 0;
}
// -----------------------------------------------------------------------
void SvxHtmlOptions::SetFontSize(sal_uInt16 nPos, sal_uInt16 nSize)
{
    if(nPos < HTML_FONT_COUNT)
    {
        pImp->aFontSizeArr[nPos] = nSize;
        SetModified();
    }
}

// -----------------------------------------------------------------------

// -----------------------------------------------------------------------


sal_Bool SvxHtmlOptions::IsImportUnknown() const
{
    return 0 != (pImp->nFlags & HTMLCFG_UNKNOWN_TAGS) ;
}

// -----------------------------------------------------------------------


void SvxHtmlOptions::SetImportUnknown(sal_Bool bSet)
{
    if(bSet)
        pImp->nFlags |= HTMLCFG_UNKNOWN_TAGS;
    else
        pImp->nFlags &= ~HTMLCFG_UNKNOWN_TAGS;
    SetModified();
}

// -----------------------------------------------------------------------


sal_uInt16  SvxHtmlOptions::GetExportMode() const
{
    return (sal_uInt16)pImp->nExportMode;
}

// -----------------------------------------------------------------------


void SvxHtmlOptions::SetExportMode(sal_uInt16 nSet)
{
    if(nSet <= HTML_CFG_MAX )
    {
        pImp->nExportMode = nSet;
        SetModified();
        CallListeners();
    }
}

// -----------------------------------------------------------------------


sal_Bool SvxHtmlOptions::IsStarBasic() const
{
    return 0 != (pImp->nFlags & HTMLCFG_STAR_BASIC) ;
}

// -----------------------------------------------------------------------


void SvxHtmlOptions::SetStarBasic(sal_Bool bSet)
{
    if(bSet)
        pImp->nFlags |=  HTMLCFG_STAR_BASIC;
    else
        pImp->nFlags &= ~HTMLCFG_STAR_BASIC;
    SetModified();
}

/*-----------------14.02.97 08.34-------------------

--------------------------------------------------*/

sal_Bool SvxHtmlOptions::IsSaveGraphicsLocal() const
{
    return 0 != (pImp->nFlags & HTMLCFG_LOCAL_GRF) ;
}
/*-----------------14.02.97 08.34-------------------

--------------------------------------------------*/
void SvxHtmlOptions::SetSaveGraphicsLocal(sal_Bool bSet)
{
    if(bSet)
        pImp->nFlags |=  HTMLCFG_LOCAL_GRF;
    else
        pImp->nFlags &= ~HTMLCFG_LOCAL_GRF;
    SetModified();
}

/*-----------------10/21/97 08:34am-----------------

--------------------------------------------------*/

sal_Bool    SvxHtmlOptions::IsPrintLayoutExtension() const
{
    sal_Bool bRet = 0 != (pImp->nFlags & HTMLCFG_PRINT_LAYOUT_EXTENSION);
    switch( pImp->nExportMode )
    {
        case HTML_CFG_MSIE_40:
        case HTML_CFG_NS40  :
        case HTML_CFG_WRITER :
        break;
        default:
            bRet = sal_False;
    }
    return bRet;
}
/*-----------------10/21/97 08:34am-----------------

--------------------------------------------------*/
void    SvxHtmlOptions::SetPrintLayoutExtension(sal_Bool bSet)
{
    if(bSet)
        pImp->nFlags |=  HTMLCFG_PRINT_LAYOUT_EXTENSION;
    else
        pImp->nFlags &= ~HTMLCFG_PRINT_LAYOUT_EXTENSION;
    SetModified();
}

/*-----------------10.07.98 10.02-------------------

--------------------------------------------------*/

sal_Bool SvxHtmlOptions::IsIgnoreFontFamily() const
{
    return 0 != (pImp->nFlags & HTMLCFG_IGNORE_FONT_FAMILY) ;
}
/*-----------------10.07.98 10.02-------------------

--------------------------------------------------*/
void SvxHtmlOptions::SetIgnoreFontFamily(sal_Bool bSet)
{
    if(bSet)
        pImp->nFlags |=  HTMLCFG_IGNORE_FONT_FAMILY;
    else
        pImp->nFlags &= ~HTMLCFG_IGNORE_FONT_FAMILY;
    SetModified();
}
/* -----------------05.02.99 09:03-------------------
 *
 * --------------------------------------------------*/
sal_Bool SvxHtmlOptions::IsStarBasicWarning() const
{
    return 0 != (pImp->nFlags & HTMLCFG_IS_BASIC_WARNING) ;
}
/* -----------------05.02.99 09:03-------------------
 *
 * --------------------------------------------------*/
void SvxHtmlOptions::SetStarBasicWarning(sal_Bool bSet)
{
    if(bSet)
        pImp->nFlags |=  HTMLCFG_IS_BASIC_WARNING;
    else
        pImp->nFlags &= ~HTMLCFG_IS_BASIC_WARNING;
    SetModified();
}

/*-----------------19.02.2001 18:40-----------------
 *
 * --------------------------------------------------*/
rtl_TextEncoding SvxHtmlOptions::GetTextEncoding() const
{
    rtl_TextEncoding eRet;
    if(pImp->bIsEncodingDefault)
        eRet = SvtSysLocale::GetBestMimeEncoding();
    else
        eRet = (rtl_TextEncoding)pImp->eEncoding;
    return eRet;
}

/*-----------------19.02.2001 18:40-----------------
 *
 * --------------------------------------------------*/
void SvxHtmlOptions::SetTextEncoding( rtl_TextEncoding eEnc )
{
    pImp->eEncoding = eEnc;
    pImp->bIsEncodingDefault = sal_False;
    SetModified();
}
/* -----------------------------15.08.2001 12:01------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool SvxHtmlOptions::IsDefaultTextEncoding() const
{
    return pImp->bIsEncodingDefault;
}

SvxHtmlOptions* SvxHtmlOptions::Get()
{
    if ( !pOptions )
        pOptions = new SvxHtmlOptions;
    return pOptions;
}


/* ---------------------- 2006-06-07T21:02+0200 ---------------------- */
sal_Bool SvxHtmlOptions::IsNumbersEnglishUS() const
{
    return 0 != (pImp->nFlags & HTMLCFG_NUMBERS_ENGLISH_US) ;
}


/* ---------------------- 2006-06-07T21:02+0200 ---------------------- */
void SvxHtmlOptions::SetNumbersEnglishUS(sal_Bool bSet)
{
    if(bSet)
        pImp->nFlags |=  HTMLCFG_NUMBERS_ENGLISH_US;
    else
        pImp->nFlags &= ~HTMLCFG_NUMBERS_ENGLISH_US;
    SetModified();
}
