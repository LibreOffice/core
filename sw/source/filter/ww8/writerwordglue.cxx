/*************************************************************************
 *
 *  $RCSfile: writerwordglue.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: obo $ $Date: 2004-04-27 14:10:57 $
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
 *  Contributor(s): cmc@openoffice.org
 *
 *
 ************************************************************************/

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */
/// @HTML

#ifndef SW_MS_MSFILTER_HXX
#include <msfilter.hxx>
#endif
#ifndef SW_WRITERWORDGLUE
#   include "writerwordglue.hxx"
#endif
#ifndef SW_MS_MSFILTER_HXX
#   include "../inc/msfilter.hxx"
#endif
#ifndef SW_WRITERHELPER
#   include "writerhelper.hxx"
#endif

#include <algorithm>                //std::find_if
#include <functional>               //std::unary_function

#include <unicode/ubidi.h>          //ubidi_getLogicalRun

#ifndef _TOOLS_TENCCVT_HXX
#   include <tools/tenccvt.hxx>     //GetExtendedTextEncoding
#endif
#ifndef INCLUDED_I18NUTIL_UNICODE_HXX
#   include <i18nutil/unicode.hxx>  //unicode::getUnicodeScriptType
#endif
#ifndef _COM_SUN_STAR_I18N_SCRIPTTYPE_HDL_
#   include <com/sun/star/i18n/ScriptType.hdl> //ScriptType
#endif
#ifndef _RTL_TENCINFO_H
#   include <rtl/tencinfo.h>        //rtl_getBestWindowsCharsetFromTextEncoding
#endif
#ifndef SV_FONTCVT_HXX
#   include <vcl/fontcvt.hxx>   //GetSubsFontName
#endif
#ifndef _HINTIDS_HXX
#   include <hintids.hxx>           //ITEMID_LRSPACE...
#endif
#ifndef _SVX_PAPERINF_HXX
#   include <svx/paperinf.hxx>      //lA0Width...
#endif
#ifndef _SVX_LRSPITEM_HXX
#   include <svx/lrspitem.hxx>      //SvxLRSpaceItem
#endif
#ifndef _SVX_ULSPITEM_HXX
#   include <svx/ulspitem.hxx>      //SvxULSpaceItem
#endif
#ifndef _SVX_BOXITEM_HXX
#   include <svx/boxitem.hxx>       //SvxBoxItem
#endif
#ifndef _SVX_FONTITEM_HXX
#   include <svx/fontitem.hxx>      //SvxFontItem
#endif
#ifndef _FRMFMT_HXX
#   include <frmfmt.hxx>            //SwFrmFmt
#endif
#ifndef _FMTCLDS_HXX
#   include <fmtclds.hxx>           //SwFmtCol
#endif
#ifndef _SW_HF_EAT_SPACINGITEM_HXX
#   include <hfspacingitem.hxx>     //SwHeaderAndFooterEatSpacingItem
#endif
#ifndef _FMTFSIZE_HXX
#   include <fmtfsize.hxx>          //SwFmtFrmSize
#endif
#ifndef _SWRECT_HXX
#   include <swrect.hxx>            //SwRect
#endif
#ifndef _FMTHDFT_HXX
#   include <fmthdft.hxx>           //SwFmtHeader/SwFmtFooter
#endif
#ifndef _POOLFMT_HXX
#   include <poolfmt.hxx>           //RES_POOL_COLLFMT_TYPE
#endif
#ifndef _FRMATR_HXX
#   include <frmatr.hxx>            //GetLRSpace...
#endif
#ifndef _NDTXT_HXX
#   include <ndtxt.hxx>             //SwTxtNode
#endif
#ifndef _BREAKIT_HXX
#   include <breakit.hxx>           //pBreakIt
#endif

#define ASSIGN_CONST_ASC(s) AssignAscii(RTL_CONSTASCII_STRINGPARAM(s))

namespace
{
    class closeenough : public std::unary_function<long, bool>
    {
    private:
        long mnValue;
        long mnWriggleRoom;
    public:
        closeenough(long nValue, long nWriggleRoom)
            : mnValue(nValue), mnWriggleRoom(nWriggleRoom) {}
        bool operator()(long nTest) const
        {
            return (
                    (mnValue - nTest < mnWriggleRoom) &&
                    (mnValue - nTest > -mnWriggleRoom)
                   );
        }
    };

    SwTwips CalcHdFtDist(const SwFrmFmt& rFmt, sal_uInt16 nSpacing)
    {
        /*
        #98506#
        The normal case for reexporting word docs is to have dynamic spacing,
        as this is word's only setting, and the reason for the existance of the
        dynamic spacing features. If we have dynamic spacing active then we can
        add its spacing to the value height of the h/f and get the wanted total
        size for word.

        Otherwise we have to get the real layout rendered
        height, which is totally nonoptimum, but the best we can do.
        */
        long nDist=0;
        const SwFmtFrmSize& rSz = rFmt.GetFrmSize();

        const SwHeaderAndFooterEatSpacingItem &rSpacingCtrl =
            sw::util::ItemGet<SwHeaderAndFooterEatSpacingItem>
            (rFmt, RES_HEADER_FOOTER_EAT_SPACING);
        if (rSpacingCtrl.GetValue())
            nDist += rSz.GetHeight();
        else
        {
            SwRect aRect(rFmt.FindLayoutRect(false));
            if (aRect.Height())
                nDist += aRect.Height();
            else
            {
                const SwFmtFrmSize& rSz = rFmt.GetFrmSize();
                if (ATT_VAR_SIZE != rSz.GetSizeType())
                    nDist += rSz.GetHeight();
                else
                {
                    nDist += 274;       // default for 12pt text
                    nDist += nSpacing;
                }
            }
        }
        return nDist;
    }

    SwTwips CalcHdDist(const SwFrmFmt& rFmt)
    {
        return CalcHdFtDist(rFmt, rFmt.GetULSpace().GetUpper());
    }

    SwTwips CalcFtDist(const SwFrmFmt& rFmt)
    {
        return CalcHdFtDist(rFmt, rFmt.GetULSpace().GetLower());
    }


    /*
     SwTxtFmtColl and SwCharFmt are quite distinct types and how they are
     gotten is also distinct, but the algorithm to match word's eqivalents into
     them is the same, so we put the different stuff into two seperate helper
     implementations and a core template that uses the helpers that uses the
     same algorithm to do the work. We'll make the helpers specializations of a
     non existing template so I can let the compiler figure out the right one
     to use from a simple argument to the algorithm class
    */
    template <class C> class MapperImpl;
    template<> class MapperImpl<SwTxtFmtColl>
    {
    private:
        SwDoc &mrDoc;
    public:
        MapperImpl(SwDoc &rDoc) : mrDoc(rDoc) {}
        SwTxtFmtColl* GetBuiltInStyle(ww::sti eSti);
        SwTxtFmtColl* GetStyle(const String &rName);
        SwTxtFmtColl* MakeStyle(const String &rName);
    };

    SwTxtFmtColl* MapperImpl<SwTxtFmtColl>::GetBuiltInStyle(ww::sti eSti)
    {
        const RES_POOL_COLLFMT_TYPE RES_NONE  = RES_POOLCOLL_DOC_END;
        static const RES_POOL_COLLFMT_TYPE aArr[]=
        {
            RES_POOLCOLL_STANDARD, RES_POOLCOLL_HEADLINE1,
            RES_POOLCOLL_HEADLINE2, RES_POOLCOLL_HEADLINE3,
            RES_POOLCOLL_HEADLINE4, RES_POOLCOLL_HEADLINE5,
            RES_POOLCOLL_HEADLINE6, RES_POOLCOLL_HEADLINE7,
            RES_POOLCOLL_HEADLINE8, RES_POOLCOLL_HEADLINE9,
            RES_POOLCOLL_TOX_IDX1, RES_POOLCOLL_TOX_IDX2,
            RES_POOLCOLL_TOX_IDX3, RES_NONE, RES_NONE, RES_NONE, RES_NONE,
            RES_NONE, RES_NONE, RES_POOLCOLL_TOX_CNTNT1,
            RES_POOLCOLL_TOX_CNTNT2, RES_POOLCOLL_TOX_CNTNT3,
            RES_POOLCOLL_TOX_CNTNT4, RES_POOLCOLL_TOX_CNTNT5,
            RES_POOLCOLL_TOX_CNTNT6, RES_POOLCOLL_TOX_CNTNT7,
            RES_POOLCOLL_TOX_CNTNT8, RES_POOLCOLL_TOX_CNTNT9, RES_NONE,
            RES_POOLCOLL_FOOTNOTE, RES_NONE, RES_POOLCOLL_HEADER,
            RES_POOLCOLL_FOOTER, RES_POOLCOLL_TOX_IDXH, RES_NONE, RES_NONE,
            RES_POOLCOLL_JAKETADRESS, RES_POOLCOLL_SENDADRESS, RES_NONE,
            RES_NONE, RES_NONE, RES_NONE, RES_NONE, RES_POOLCOLL_ENDNOTE,
            RES_NONE, RES_NONE, RES_NONE, RES_POOLCOLL_LISTS_BEGIN,
            RES_NONE, RES_NONE, RES_NONE, RES_NONE, RES_NONE, RES_NONE,
            RES_NONE, RES_NONE, RES_NONE, RES_NONE, RES_NONE, RES_NONE,
            RES_NONE, RES_NONE, RES_POOLCOLL_DOC_TITEL, RES_NONE,
            RES_POOLCOLL_SIGNATURE, RES_NONE, RES_POOLCOLL_TEXT,
            RES_POOLCOLL_TEXT_MOVE, RES_NONE, RES_NONE, RES_NONE, RES_NONE,
            RES_NONE, RES_NONE, RES_POOLCOLL_DOC_SUBTITEL
        };

        const size_t nArrSize = (sizeof(aArr) / sizeof(aArr[0]));
        ASSERT(nArrSize == 75, "Style Array has false size");

        SwTxtFmtColl* pRet = 0;
        //If this is a built-in word style that has a built-in writer
        //equivalent, then map it to one of our built in styles regardless
        //of its name
        if (eSti < nArrSize && aArr[eSti] != RES_NONE)
            pRet = mrDoc.GetTxtCollFromPoolSimple(aArr[eSti], false);
        return pRet;
    }

    SwTxtFmtColl* MapperImpl<SwTxtFmtColl>::GetStyle(const String &rName)
    {
        return sw::util::GetParaStyle(mrDoc, rName);
    }

    SwTxtFmtColl* MapperImpl<SwTxtFmtColl>::MakeStyle(const String &rName)
    {
        return mrDoc.MakeTxtFmtColl(rName,
            const_cast<SwTxtFmtColl *>(mrDoc.GetDfltTxtFmtColl()));
    }

    template<> class MapperImpl<SwCharFmt>
    {
    private:
        SwDoc &mrDoc;
    public:
        MapperImpl(SwDoc &rDoc) : mrDoc(rDoc) {}
        SwCharFmt* GetBuiltInStyle(ww::sti eSti);
        SwCharFmt* GetStyle(const String &rName);
        SwCharFmt* MakeStyle(const String &rName);
    };

    SwCharFmt* MapperImpl<SwCharFmt>::GetBuiltInStyle(ww::sti eSti)
    {
        RES_POOL_CHRFMT_TYPE eLookup = RES_POOLCHR_NORMAL_END;
        switch (eSti)
        {
            case ww::stiFtnRef:
                eLookup = RES_POOLCHR_FOOTNOTE;
                break;
            case ww::stiLnn:
                eLookup = RES_POOLCHR_LINENUM;
                break;
            case ww::stiPgn:
                eLookup = RES_POOLCHR_PAGENO;
                break;
            case ww::stiEdnRef:
                eLookup = RES_POOLCHR_ENDNOTE;
                break;
            case ww::stiHyperlink:
                eLookup = RES_POOLCHR_INET_NORMAL;
                break;
            case ww::stiHyperlinkFollowed:
                eLookup = RES_POOLCHR_INET_VISIT;
                break;
            case ww::stiStrong:
                eLookup = RES_POOLCHR_HTML_STRONG;
                break;
            case ww::stiEmphasis:
                eLookup = RES_POOLCHR_HTML_EMPHASIS;
                break;
            default:
                eLookup = RES_POOLCHR_NORMAL_END;
                break;
        }
        SwCharFmt *pRet = 0;
        if (eLookup != RES_POOLCHR_NORMAL_END)
            pRet = mrDoc.GetCharFmtFromPool(eLookup);
        return pRet;
    }

    SwCharFmt* MapperImpl<SwCharFmt>::GetStyle(const String &rName)
    {
        return sw::util::GetCharStyle(mrDoc, rName);
    }

    SwCharFmt* MapperImpl<SwCharFmt>::MakeStyle(const String &rName)
    {
        return mrDoc.MakeCharFmt(rName, mrDoc.GetDfltCharFmt());
    }

    template<class C> class StyleMapperImpl
    {
    private:
        MapperImpl<C> maHelper;
        std::set<const C*> maUsedStyles;
        C* MakeNonCollidingStyle(const String& rName);
    public:
        typedef std::pair<C*, bool> StyleResult;
        StyleMapperImpl(SwDoc &rDoc) : maHelper(rDoc) {}
        StyleResult GetStyle(const String& rName, ww::sti eSti);
    };

    template<class C>
    typename StyleMapperImpl<C>::StyleResult
    StyleMapperImpl<C>::GetStyle(const String& rName, ww::sti eSti)
    {
        C *pRet = maHelper.GetBuiltInStyle(eSti);

        //If we've used it once, don't reuse it
        if (pRet && (maUsedStyles.end() != maUsedStyles.find(pRet)))
            pRet = 0;

        if (!pRet)
        {
            pRet = maHelper.GetStyle(rName);
            //If we've used it once, don't reuse it
            if (pRet && (maUsedStyles.end() != maUsedStyles.find(pRet)))
                pRet = 0;
        }

        bool bStyExist = pRet ? true : false;

        if (!pRet)
        {
            String aName(rName);
            xub_StrLen nPos = aName.Search(',');
            // No commas allow in SW style names
            if (STRING_NOTFOUND != nPos)
                aName.Erase(nPos);
            pRet = MakeNonCollidingStyle(aName);
        }

        if (pRet)
            maUsedStyles.insert(pRet);

        return StyleResult(pRet, bStyExist);
    }

    template<class C>
    C* StyleMapperImpl<C>::MakeNonCollidingStyle(const String& rName)
    {
        String aName(rName);
        C* pColl = 0;

        if (pColl = maHelper.GetStyle(aName))
        {
            //If the style collides first stick WW- in front of it, unless
            //it already has it and then successively add a larger and
            //larger number after it, its got to work at some stage!
            if (!aName.EqualsIgnoreCaseAscii("WW-", 0, 3))
                aName.InsertAscii("WW-" , 0);

            sal_Int32 nI = 1;
            while (
                    (pColl = maHelper.GetStyle(aName)) &&
                    (nI < SAL_MAX_INT32)
                  )
            {
                aName += String::CreateFromInt32(nI++);
            }
        }

        return pColl ? 0 : maHelper.MakeStyle(aName);
    }

    String FindBestMSSubstituteFont(const String &rFont)
    {
        String sRet;
        if (sw::util::IsStarSymbol(rFont))
            sRet.ASSIGN_CONST_ASC("Arial Unicode MS");
        else
            sRet = GetSubsFontName(rFont, SUBSFONT_ONLYONE | SUBSFONT_MS);
        return sRet;
    }

    /*
     Utility to categorize unicode characters into the best fit windows charset
     range for exporting to ww6, or as a hint to non \u unicode token aware rtf
     readers
    */
    rtl_TextEncoding getScriptClass(sal_Unicode cChar)
    {
        using namespace com::sun::star::i18n;

        static ScriptTypeList aScripts[] =
        {
            { UnicodeScript_kBasicLatin, RTL_TEXTENCODING_MS_1252},
            { UnicodeScript_kLatin1Supplement, RTL_TEXTENCODING_MS_1252},
            { UnicodeScript_kLatinExtendedA, RTL_TEXTENCODING_MS_1250},
            { UnicodeScript_kLatinExtendedB, RTL_TEXTENCODING_MS_1257},
            { UnicodeScript_kGreek, RTL_TEXTENCODING_MS_1253},
            { UnicodeScript_kCyrillic, RTL_TEXTENCODING_MS_1251},
            { UnicodeScript_kHebrew, RTL_TEXTENCODING_MS_1255},
            { UnicodeScript_kArabic, RTL_TEXTENCODING_MS_1256},
            { UnicodeScript_kThai, RTL_TEXTENCODING_MS_1258},
            { UnicodeScript_kScriptCount, RTL_TEXTENCODING_MS_1252}
        };

        return unicode::getUnicodeScriptType(cChar, aScripts,
            RTL_TEXTENCODING_MS_1252);
    }

    //Utility to remove entries before a given starting position
    class IfBeforeStart
        : public std::unary_function<const sw::util::CharRunEntry&, bool>
    {
    private:
        xub_StrLen mnStart;
    public:
        IfBeforeStart(xub_StrLen nStart) : mnStart(nStart) {}
        bool operator()(const sw::util::CharRunEntry &rEntry) const
        {
            return rEntry.mnEndPos < mnStart;
        }
    };
}

namespace sw
{
    namespace types
    {
        long SnapPageDimension(long nSize) throw()
        {
            static const long aSizes[] =
            {
                lA0Width, lA0Height, lA1Width, lA2Width, lA3Width, lA4Width,
                lA5Width, lB4Width, lB4Height, lB5Width, lB6Width, lC4Width,
                lC4Height, lC5Width, lC6Width, lC65Width, lC65Height, lDLWidth,
                lDLHeight, lJISB4Width, lJISB4Height, lJISB5Width, lJISB6Width,
                lLetterWidth, lLetterHeight, lLegalHeight, lTabloidWidth,
                lTabloidHeight, lDiaWidth, lDiaHeight, lScreenWidth,
                lScreenHeight, lAWidth, lAHeight, lBHeight, lCHeight, lDHeight,
                lEHeight, lExeWidth, lExeHeight, lLegal2Width, lLegal2Height,
                lCom675Width, lCom675Height, lCom9Width, lCom9Height,
                lCom10Width, lCom10Height, lCom11Width, lCom11Height,
                lCom12Width, lMonarchHeight, lKai16Width, lKai16Height,
                lKai32Width, lKai32BigWidth, lKai32BigHeight
            };

            const long nWriggleRoom = 5;
            const long *pEnd = aSizes + sizeof(aSizes) / sizeof(aSizes[0]);
            const long *pEntry =
                std::find_if(aSizes, pEnd, closeenough(nSize, nWriggleRoom));

            if (pEntry != pEnd)
                nSize = *pEntry;

            return nSize;
        }
    }

    namespace util
    {

        bool IsPlausableSingleWordSection(const SwFrmFmt &rTitleFmt,
            const SwFrmFmt &rFollowFmt)
        {
            bool bPlausableTitlePage = true;

            const SwFmtCol& rFirstCols = rTitleFmt.GetCol();
            const SwFmtCol& rFollowCols = rFollowFmt.GetCol();
            const SwColumns& rFirstColumns = rFirstCols.GetColumns();
            const SwColumns& rFollowColumns = rFollowCols.GetColumns();
            const SvxLRSpaceItem &rOneLR = rTitleFmt.GetLRSpace();
            const SvxLRSpaceItem &rTwoLR= rFollowFmt.GetLRSpace();

            if (rFirstColumns.Count() != rFollowColumns.Count())
            {
                //e.g. #i4320#
                bPlausableTitlePage = false;
            }
            else if (rOneLR != rTwoLR)
                bPlausableTitlePage = false;
            else
            {
                HdFtDistanceGlue aOne(rTitleFmt.GetAttrSet());
                HdFtDistanceGlue aTwo(rFollowFmt.GetAttrSet());
                //e.g. #i14509#
                if (!aOne.EqualTopBottom(aTwo))
                    bPlausableTitlePage = false;
            }
            return bPlausableTitlePage;
        }

        HdFtDistanceGlue::HdFtDistanceGlue(const SfxItemSet &rPage)
        {
            if (const SvxBoxItem *pBox = HasItem<SvxBoxItem>(rPage, RES_BOX))
            {
                dyaHdrTop = pBox->CalcLineSpace(BOX_LINE_TOP);
                dyaHdrBottom = pBox->CalcLineSpace(BOX_LINE_BOTTOM);
            }
            else
                dyaHdrTop = dyaHdrBottom = 0;
            const SvxULSpaceItem &rUL =
                ItemGet<SvxULSpaceItem>(rPage, RES_UL_SPACE);
            dyaHdrTop += rUL.GetUpper();
            dyaHdrBottom += rUL.GetLower();

            dyaTop = dyaHdrTop;
            dyaBottom = dyaHdrBottom;

            using sw::types::msword_cast;

            const SwFmtHeader *pHd = HasItem<SwFmtHeader>(rPage, RES_HEADER);
            if (pHd && pHd->IsActive() && pHd->GetHeaderFmt())
            {
                mbHasHeader = true;
                dyaTop += (CalcHdDist(*(pHd->GetHeaderFmt())));
            }

            const SwFmtFooter *pFt = HasItem<SwFmtFooter>(rPage, RES_FOOTER);
            if (pFt && pFt->IsActive() && pFt->GetFooterFmt())
            {
                mbHasFooter = true;
                dyaBottom += (CalcFtDist(*(pFt->GetFooterFmt())));
            }
        }

        bool HdFtDistanceGlue::EqualTopBottom(const HdFtDistanceGlue &rOther)
            const
        {
            return (dyaTop == rOther.dyaTop && dyaBottom == rOther.dyaBottom);
        }

        ParaStyleMapper::ParaStyleMapper(SwDoc &rDoc)
            : mpImpl(new StyleMapperImpl<SwTxtFmtColl>(rDoc))
        {
        }

        ParaStyleMapper::~ParaStyleMapper()
        {
            delete mpImpl;
        }

        ParaStyleMapper::StyleResult ParaStyleMapper::GetStyle(
            const String& rName, ww::sti eSti)
        {
            return mpImpl->GetStyle(rName, eSti);
        }

        CharStyleMapper::CharStyleMapper(SwDoc &rDoc)
            : mpImpl(new StyleMapperImpl<SwCharFmt>(rDoc))
        {
        }

        CharStyleMapper::~CharStyleMapper()
        {
            delete mpImpl;
        }

        CharStyleMapper::StyleResult CharStyleMapper::GetStyle(
            const String& rName, ww::sti eSti)
        {
            return mpImpl->GetStyle(rName, eSti);
        }

        FontMapExport::FontMapExport(const String &rFamilyName)
        {
            msPrimary = GetFontToken(rFamilyName, 0);
            msSecondary = FindBestMSSubstituteFont(msPrimary);
            if (!msSecondary.Len())
                msSecondary = GetFontToken(rFamilyName, 1);
        }

        bool FontMapExport::HasDistinctSecondary() const
        {
            if (msSecondary.Len() && msSecondary != msPrimary)
                return true;
            return false;
        }

        bool ItemSort::operator()(sal_uInt16 nA, sal_uInt16 nB) const
        {
            /*
             #i24291#
             All we want to do is ensure for now is that if a charfmt exist
             in the character properties that it rises to the top and is
             exported first.  In the future we might find more ordering
             depandancies for export, in which case this is the place to do
             it
            */
            if (nA == nB)
                return false;
            if (nA == RES_TXTATR_CHARFMT)
                return true;
            if (nB == RES_TXTATR_CHARFMT)
                return false;
            return nA < nB;
        }

        CharRuns GetPseudoCharRuns(const SwTxtNode& rTxtNd,
            xub_StrLen nTxtStart, bool bSplitOnCharSet)
        {
            const String &rTxt = rTxtNd.GetTxt();

            bool bParaIsRTL = false;
            ASSERT(rTxtNd.GetDoc(), "No document for node?, suspicious");
            if (rTxtNd.GetDoc())
            {
                if (FRMDIR_HORI_RIGHT_TOP ==
                    rTxtNd.GetDoc()->GetTextDirection(SwPosition(rTxtNd)))
                {
                    bParaIsRTL = true;
                }
            }

            using namespace com::sun::star::i18n;

            sal_uInt16 nScript = ScriptType::LATIN;
            if (rTxt.Len() && pBreakIt && pBreakIt->xBreak.is())
                nScript = pBreakIt->xBreak->getScriptType(rTxt, 0);

            rtl_TextEncoding eChrSet = ItemGet<SvxFontItem>(rTxtNd,
                GetWhichOfScript(RES_CHRATR_FONT, nScript)).GetCharSet();
            eChrSet = GetExtendedTextEncoding(eChrSet);

            CharRuns aRunChanges;

            if (!rTxt.Len())
            {
                aRunChanges.push_back(CharRunEntry(0, nScript, eChrSet,
                    bParaIsRTL));
                return aRunChanges;
            }

            typedef std::pair<int32_t, bool> DirEntry;
            typedef std::vector<DirEntry> DirChanges;
            typedef DirChanges::const_iterator cDirIter;

            typedef std::pair<xub_StrLen, sal_Int16> CharSetEntry;
            typedef std::vector<CharSetEntry> CharSetChanges;
            typedef CharSetChanges::const_iterator cCharSetIter;

            typedef std::pair<xub_StrLen, sal_uInt16> ScriptEntry;
            typedef std::vector<ScriptEntry> ScriptChanges;
            typedef ScriptChanges::const_iterator cScriptIter;

            DirChanges aDirChanges;
            CharSetChanges aCharSets;
            ScriptChanges aScripts;

            UBiDiDirection eDefaultDir = bParaIsRTL ? UBIDI_RTL : UBIDI_LTR;
            UErrorCode nError = U_ZERO_ERROR;
            UBiDi* pBidi = ubidi_openSized(rTxt.Len(), 0, &nError);
            ubidi_setPara(pBidi, rTxt.GetBuffer(), rTxt.Len(), eDefaultDir,
                0, &nError);

            sal_Int32 nCount = ubidi_countRuns(pBidi, &nError);
            aDirChanges.reserve(nCount);

            int32_t nStart = 0;
            int32_t nEnd;
            UBiDiLevel nCurrDir;

            for (sal_Int32 nIdx = 0; nIdx < nCount; ++nIdx)
            {
                ubidi_getLogicalRun(pBidi, nStart, &nEnd, &nCurrDir);
                /*
                UBiDiLevel is the type of the level values in this BiDi
                implementation.

                It holds an embedding level and indicates the visual direction
                by its bit 0 (even/odd value).

                The value for UBIDI_DEFAULT_LTR is even and the one for
                UBIDI_DEFAULT_RTL is odd
                */
                aDirChanges.push_back(DirEntry(nEnd, nCurrDir & 0x1));
                nStart = nEnd;
            }
            ubidi_close(pBidi);

            if (bSplitOnCharSet)
            {
                //Split unicode text into plausable 8bit ranges for export to
                //older non unicode aware format
                xub_StrLen nLen = rTxt.Len();
                xub_StrLen nPos = 0;
                while (nPos != nLen)
                {
                    rtl_TextEncoding ScriptType =
                        getScriptClass(rTxt.GetChar(nPos++));
                    while (
                            (nPos != nLen) &&
                            (ScriptType == getScriptClass(rTxt.GetChar(nPos)))
                          )
                    {
                        ++nPos;
                    }

                    aCharSets.push_back(CharSetEntry(nPos, ScriptType));
                }
            }

            using sw::types::writer_cast;

            if (pBreakIt && pBreakIt->xBreak.is())
            {
                xub_StrLen nLen = rTxt.Len();
                xub_StrLen nPos = 0;
                while (nPos < nLen)
                {
                    sal_Int32 nEnd = pBreakIt->xBreak->endOfScript(rTxt, nPos,
                        nScript);
                    if (nEnd < 0)
                        break;
//                    nPos = writer_cast<xub_StrLen>(nEnd);
                    nPos = nEnd;
                    aScripts.push_back(ScriptEntry(nPos, nScript));
                    nScript = pBreakIt->xBreak->getScriptType(rTxt, nPos);
                }
            }

            cDirIter aBiDiEnd = aDirChanges.end();
            cCharSetIter aCharSetEnd = aCharSets.end();
            cScriptIter aScriptEnd = aScripts.end();

            cDirIter aBiDiIter = aDirChanges.begin();
            cCharSetIter aCharSetIter = aCharSets.begin();
            cScriptIter aScriptIter = aScripts.begin();

            bool bCharIsRTL = bParaIsRTL;

            while (
                    aBiDiIter != aBiDiEnd ||
                    aCharSetIter != aCharSetEnd ||
                    aScriptIter != aScriptEnd
                  )
            {
                xub_StrLen nMinPos = rTxt.Len();

                if (aBiDiIter != aBiDiEnd)
                {
                    if (aBiDiIter->first < nMinPos)
//                        nMinPos = writer_cast<xub_StrLen>(aBiDiIter->first);
                        nMinPos = aBiDiIter->first;
                    bCharIsRTL = aBiDiIter->second;
                }

                if (aCharSetIter != aCharSetEnd)
                {
                    if (aCharSetIter->first < nMinPos)
                        nMinPos = aCharSetIter->first;
                    eChrSet = aCharSetIter->second;
                }

                if (aScriptIter != aScriptEnd)
                {
                    if (aScriptIter->first < nMinPos)
                        nMinPos = aScriptIter->first;
                    nScript = aScriptIter->second;
                }

                aRunChanges.push_back(
                    CharRunEntry(nMinPos, nScript, eChrSet, bCharIsRTL));

                if (aBiDiIter != aBiDiEnd)
                {
                    if (aBiDiIter->first == nMinPos)
                        ++aBiDiIter;
                }

                if (aCharSetIter != aCharSetEnd)
                {
                    if (aCharSetIter->first == nMinPos)
                        ++aCharSetIter;
                }

                if (aScriptIter != aScriptEnd)
                {
                    if (aScriptIter->first == nMinPos)
                        ++aScriptIter;
                }
            }

            aRunChanges.erase(std::remove_if(aRunChanges.begin(),
                aRunChanges.end(), IfBeforeStart(nTxtStart)), aRunChanges.end());

            return aRunChanges;
        }
    }

    namespace ms
    {
        sal_uInt8 rtl_TextEncodingToWinCharset(rtl_TextEncoding eTextEncoding)
        {
            sal_uInt8 nRet =
                rtl_getBestWindowsCharsetFromTextEncoding(eTextEncoding);
            if (eTextEncoding == RTL_TEXTENCODING_UCS2)
            {
                ASSERT(nRet != 0x80, "This method may be redundant");
                nRet = 0x80;
            }
            else if (eTextEncoding == RTL_TEXTENCODING_DONTKNOW)
            {
                ASSERT(nRet != 0x80, "This method may be redundant");
                nRet = 0x80;
            }
            return nRet;
        }

        rtl_TextEncoding rtl_TextEncodingToWinCharsetAndBack(
            rtl_TextEncoding eTextEncoding)
        {
            return rtl_getTextEncodingFromWindowsCharset(
                    rtl_TextEncodingToWinCharset(eTextEncoding));
        }
    }

}

/* vi:set tabstop=4 shiftwidth=4 expandtab: */
