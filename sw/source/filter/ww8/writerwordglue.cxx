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

#include <msfilter.hxx>
#include "writerwordglue.hxx"
#include <doc.hxx>
#include "writerhelper.hxx"

#include <algorithm>
#include <functional>

#include <rtl/tencinfo.h>

#include <unicode/ubidi.h>
#include <tools/tenccvt.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>

#include <unotools/fontcvt.hxx>
#include <editeng/paperinf.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/fontitem.hxx>
#include <frmfmt.hxx>
#include <fmtclds.hxx>
#include <hfspacingitem.hxx>
#include <fmtfsize.hxx>
#include <swrect.hxx>
#include <fmthdft.hxx>
#include <frmatr.hxx>
#include <ndtxt.hxx>
#include <breakit.hxx>
#include <i18nlangtag/mslangid.hxx>

#define ASSIGN_CONST_ASC(s) AssignAscii(s)

using namespace css;

namespace myImplHelpers
{
    SwTwips CalcHdFtDist(const SwFrmFmt& rFmt, sal_uInt16 nSpacing)
    {
        /*
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
                const SwFmtFrmSize& rSize = rFmt.GetFrmSize();
                if (ATT_VAR_SIZE != rSize.GetHeightSizeType())
                    nDist += rSize.GetHeight();
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
     them is the same, so we put the different stuff into two separate helper
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
            RES_NONE, RES_NONE, RES_POOLCOLL_HEADLINE_BASE, RES_NONE,
            RES_POOLCOLL_SIGNATURE, RES_NONE, RES_POOLCOLL_TEXT,
            RES_POOLCOLL_TEXT_MOVE, RES_NONE, RES_NONE, RES_NONE, RES_NONE,
            RES_NONE, RES_NONE, RES_POOLCOLL_DOC_SUBTITEL
        };

        OSL_ENSURE(SAL_N_ELEMENTS(aArr) == 75, "Style Array has false size");

        SwTxtFmtColl* pRet = 0;
        //If this is a built-in word style that has a built-in writer
        //equivalent, then map it to one of our built in styles regardless
        //of its name
        if (sal::static_int_cast< size_t >(eSti) < SAL_N_ELEMENTS(aArr) && aArr[eSti] != RES_NONE)
            pRet = mrDoc.GetTxtCollFromPool( static_cast< sal_uInt16 >(aArr[eSti]), false);
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
            pRet = mrDoc.GetCharFmtFromPool( static_cast< sal_uInt16 >(eLookup) );
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

        if (0 != (pColl = maHelper.GetStyle(aName)))
        {
            //If the style collides first stick WW- in front of it, unless
            //it already has it and then successively add a larger and
            //larger number after it, its got to work at some stage!
            if (!aName.EqualsIgnoreCaseAscii("WW-", 0, 3))
                aName.InsertAscii("WW-" , 0);

            sal_Int32 nI = 1;
            String aBaseName = aName;
            while (
                    0 != (pColl = maHelper.GetStyle(aName)) &&
                    (nI < SAL_MAX_INT32)
                  )
            {
                aName = aBaseName;
                aName += OUString::number(nI++);
            }
        }

        return pColl ? 0 : maHelper.MakeStyle(aName);
    }

    String FindBestMSSubstituteFont(const String &rFont)
    {
        String sRet;
        if ( IsStarSymbol(rFont) )
            sRet.ASSIGN_CONST_ASC("Arial Unicode MS");
        else
            sRet = GetSubsFontName(rFont, SUBSFONT_ONLYONE | SUBSFONT_MS);
        return sRet;
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
    namespace util
    {

        bool IsPlausableSingleWordSection(const SwFrmFmt &rTitleFmt, const SwFrmFmt &rFollowFmt)
        {
            bool bPlausableSingleWordSection = true;

            const SwFmtCol& rFirstCols = rTitleFmt.GetCol();
            const SwFmtCol& rFollowCols = rFollowFmt.GetCol();
            const SwColumns& rFirstColumns = rFirstCols.GetColumns();
            const SwColumns& rFollowColumns = rFollowCols.GetColumns();
            const SvxLRSpaceItem &rOneLR = rTitleFmt.GetLRSpace();
            const SvxLRSpaceItem &rTwoLR= rFollowFmt.GetLRSpace();
            const SwFmtFrmSize& rFirstFrmSize = rTitleFmt.GetFrmSize();
            const SwFmtFrmSize& rFollowFrmSize = rFollowFmt.GetFrmSize();

            if (rFirstColumns.size() != rFollowColumns.size())
            {
                //e.g. #i4320#
                bPlausableSingleWordSection = false;
            }
            else if (rOneLR != rTwoLR)
                bPlausableSingleWordSection = false;
            else if (rFirstFrmSize != rFollowFrmSize)
                bPlausableSingleWordSection = false;
            else
            {
                HdFtDistanceGlue aOne(rTitleFmt.GetAttrSet());
                HdFtDistanceGlue aTwo(rFollowFmt.GetAttrSet());
                //e.g. #i14509#
                if (!aOne.StrictEqualTopBottom(aTwo))
                    bPlausableSingleWordSection = false;
            }
            return bPlausableSingleWordSection;
        }

        HdFtDistanceGlue::HdFtDistanceGlue(const SfxItemSet &rPage)
        {
            if (const SvxBoxItem *pBox = HasItem<SvxBoxItem>(rPage, RES_BOX))
            {
                dyaHdrTop = pBox->CalcLineSpace(BOX_LINE_TOP);
                dyaHdrBottom = pBox->CalcLineSpace(BOX_LINE_BOTTOM);
            }
            else
            {
                dyaHdrTop = dyaHdrBottom = 0;
                dyaHdrBottom = 0;
            }
            const SvxULSpaceItem &rUL =
                ItemGet<SvxULSpaceItem>(rPage, RES_UL_SPACE);
            dyaHdrTop = dyaHdrTop + rUL.GetUpper();
            dyaHdrBottom = dyaHdrBottom + rUL.GetLower();

            dyaTop = dyaHdrTop;
            dyaBottom = dyaHdrBottom;

            using sw::types::msword_cast;

            const SwFmtHeader *pHd = HasItem<SwFmtHeader>(rPage, RES_HEADER);
            if (pHd && pHd->IsActive() && pHd->GetHeaderFmt())
            {
                mbHasHeader = true;
                dyaTop = dyaTop + static_cast< sal_uInt16 >( (myImplHelpers::CalcHdDist(*(pHd->GetHeaderFmt()))) );
            }
            else
                mbHasHeader = false;

            const SwFmtFooter *pFt = HasItem<SwFmtFooter>(rPage, RES_FOOTER);
            if (pFt && pFt->IsActive() && pFt->GetFooterFmt())
            {
                mbHasFooter = true;
                dyaBottom = dyaBottom + static_cast< sal_uInt16 >( (myImplHelpers::CalcFtDist(*(pFt->GetFooterFmt()))) );
            }
            else
                mbHasFooter = false;
        }

        bool HdFtDistanceGlue::StrictEqualTopBottom(const HdFtDistanceGlue &rOther)
            const
        {
            // Check top only if both object have a header or if
            // both object don't have a header
            if ( (  HasHeader() &&  rOther.HasHeader() ) ||
                 ( !HasHeader() && !rOther.HasHeader() ) )
            {
                if (dyaTop != rOther.dyaTop)
                    return false;
            }

            // Check bottom only if both object have a footer or if
            // both object don't have a footer
            if ( (  HasFooter() &&  rOther.HasFooter() ) ||
                 ( !HasFooter() && !rOther.HasFooter() ) )
            {
                if (dyaBottom != rOther.dyaBottom)
                    return false;
            }

            return true;
        }

        ParaStyleMapper::ParaStyleMapper(SwDoc &rDoc)
            : mpImpl(new myImplHelpers::StyleMapperImpl<SwTxtFmtColl>(rDoc))
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
            : mpImpl(new myImplHelpers::StyleMapperImpl<SwCharFmt>(rDoc))
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
            sal_Int32 nIndex = 0;
            msPrimary = GetNextFontToken(rFamilyName, nIndex);
            msSecondary = myImplHelpers::FindBestMSSubstituteFont(msPrimary);
            if (!msSecondary.Len() && nIndex != -1)
                msSecondary = GetNextFontToken(rFamilyName, nIndex);
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
            OSL_ENSURE(rTxtNd.GetDoc(), "No document for node?, suspicious");
            if (rTxtNd.GetDoc())
            {
                if (FRMDIR_HORI_RIGHT_TOP ==
                    rTxtNd.GetDoc()->GetTextDirection(SwPosition(rTxtNd)))
                {
                    bParaIsRTL = true;
                }
            }

            using namespace ::com::sun::star::i18n;

            sal_uInt16 nScript = i18n::ScriptType::LATIN;
            if (rTxt.Len() && g_pBreakIt && g_pBreakIt->GetBreakIter().is())
                nScript = g_pBreakIt->GetBreakIter()->getScriptType(rTxt, 0);

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
            ubidi_setPara(pBidi, reinterpret_cast<const UChar *>(rTxt.GetBuffer()), rTxt.Len(),
                    static_cast< UBiDiLevel >(eDefaultDir), 0, &nError);

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
                        getBestMSEncodingByChar(rTxt.GetChar(nPos++));
                    while (
                            (nPos != nLen) &&
                            (ScriptType == getBestMSEncodingByChar(rTxt.GetChar(nPos)))
                          )
                    {
                        ++nPos;
                    }

                    aCharSets.push_back(CharSetEntry(nPos, ScriptType));
                }
            }

            using sw::types::writer_cast;

            if (g_pBreakIt && g_pBreakIt->GetBreakIter().is())
            {
                xub_StrLen nLen = rTxt.Len();
                xub_StrLen nPos = 0;
                while (nPos < nLen)
                {
                    sal_Int32 nEnd2 = g_pBreakIt->GetBreakIter()->endOfScript(rTxt, nPos,
                        nScript);
                    if (nEnd2 < 0)
                        break;
                    nPos = static_cast< xub_StrLen >(nEnd2);
                    aScripts.push_back(ScriptEntry(nPos, nScript));
                    nScript = g_pBreakIt->GetBreakIter()->getScriptType(rTxt, nPos);
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
                        nMinPos = static_cast< xub_StrLen >(aBiDiIter->first);
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
                aRunChanges.end(), myImplHelpers::IfBeforeStart(nTxtStart)), aRunChanges.end());

            return aRunChanges;
        }
    }

    namespace ms
    {
        sal_uInt8 rtl_TextEncodingToWinCharset(rtl_TextEncoding eTextEncoding)
        {
            sal_uInt8 nRet =
                rtl_getBestWindowsCharsetFromTextEncoding(eTextEncoding);
            switch (eTextEncoding)
            {
                case RTL_TEXTENCODING_DONTKNOW:
                case RTL_TEXTENCODING_UCS2:
                case RTL_TEXTENCODING_UTF7:
                case RTL_TEXTENCODING_UTF8:
                case RTL_TEXTENCODING_JAVA_UTF8:
                    nRet = 0x01;
                    break;
                default:
                    break;
            }
            return nRet;
        }

        long DateTime2DTTM( const DateTime& rDT )
        {
        /*
        mint    short   :6  0000003F    minutes (0-59)
        hr      short   :5  000007C0    hours (0-23)
        dom     short   :5  0000F800    days of month (1-31)
        mon     short   :4  000F0000    months (1-12)
        yr      short   :9  1FF00000    years (1900-2411)-1900
        wdy     short   :3  E0000000    weekday(Sunday=0
                                                Monday=1
        ( wdy can be ignored )                  Tuesday=2
                                                Wednesday=3
                                                Thursday=4
                                                Friday=5
                                                Saturday=6)
        */

            if ( rDT.GetDate() == 0L )
                return 0L;
            long nDT = ( rDT.GetDayOfWeek() + 1 ) % 7;
            nDT <<= 9;
            nDT += ( rDT.GetYear() - 1900 ) & 0x1ff;
            nDT <<= 4;
            nDT += rDT.GetMonth() & 0xf;
            nDT <<= 5;
            nDT += rDT.GetDay() & 0x1f;
            nDT <<= 5;
            nDT += rDT.GetHour() & 0x1f;
            nDT <<= 6;
            nDT += rDT.GetMin() & 0x3f;
            return nDT;
        }

        sal_uLong MSDateTimeFormatToSwFormat(String& rParams,
            SvNumberFormatter *pFormatter, sal_uInt16 &rLang, bool bHijri,
            sal_uInt16 nDocLang)
        {
            // tell the Formatter about the new entry
            sal_Int32 nCheckPos = 0;
            short  nType = NUMBERFORMAT_DEFINED;
            sal_uInt32  nKey = 0;

            OUString sParams(rParams);
            SwapQuotesInField(sParams);
            rParams = sParams;

            // Force to Japanese when finding one of 'geaE'
            OUString sJChars( "geE" );
            bool bForceJapanese = ( STRING_NOTFOUND != rParams.SearchChar( sJChars.getStr() ) );
            if ( bForceJapanese )
            {
                rParams.SearchAndReplaceAll( OUString( "ee" ),
                                             OUString( "yyyy" ) );
                rParams.SearchAndReplaceAll( OUString( "EE" ),
                                             OUString( "YYYY" ) );
            }
            if (LANGUAGE_FRENCH != nDocLang)
            {
                // Handle the 'a' case here
                xub_StrLen nLastPos = 0;
                do
                {
                    xub_StrLen nPos = rParams.Search( 'a', nLastPos + 1 );
                    bForceJapanese |= ( nPos != STRING_NOTFOUND && IsNotAM( rParams, nPos ) );
                    nLastPos = nPos;
                } while ( STRING_NOTFOUND != nLastPos );
            }

            // Force to NatNum when finding one of 'oOA'
            String sOldParams( rParams );
            rParams.SearchAndReplaceAll( OUString( "o" ),
                                         OUString( "m" ) );
            rParams.SearchAndReplaceAll( OUString( "O" ),
                                         OUString( "M" ) );
            bool bForceNatNum = !sOldParams.Equals( rParams );
            if (LANGUAGE_FRENCH != nDocLang)
            {
                // Handle the 'A' case here
                xub_StrLen nLastPos = 0;
                do
                {
                    xub_StrLen nPos = rParams.Search( 'A', nLastPos + 1 );
                    bool bIsCharA = ( nPos != STRING_NOTFOUND && IsNotAM( rParams, nPos ) );
                    bForceNatNum |= bIsCharA;
                    if ( bIsCharA )
                        rParams.SetChar( nPos, 'D' );
                    nLastPos = nPos;
                } while ( STRING_NOTFOUND != nLastPos );
            }

            xub_StrLen nLen = rParams.Len();
            xub_StrLen nI = 0;
            while (nI < nLen)
            {
                if (rParams.GetChar(nI) == '\\')
                    nI++;
                else if (rParams.GetChar(nI) == '\"')
                {
                    ++nI;
                    //While not at the end and not at an unescaped end quote
                    while ((nI < nLen) && (!(rParams.GetChar(nI) == '\"') && (rParams.GetChar(nI-1) != '\\')))
                        ++nI;
                }
                else //normal unquoted section
                {
                    sal_Unicode nChar = rParams.GetChar(nI);

                    // Change the localized word string to english
                    switch ( nDocLang )
                    {
                        case LANGUAGE_FRENCH:
                            if ( ( nChar == 'a' || nChar == 'A' ) && IsNotAM(rParams, nI) )
                                rParams.SetChar(nI, 'Y');
                            break;
                        default:
                            ;
                    }
                    if (nChar == '/')
                    {
                        // MM: We have to escape '/' in case it's used as a char.
                        // But not if it's a '/' inside AM/PM
                        if (!(IsPreviousAM(rParams, nI) && IsNextPM(rParams, nI)))
                        {
                            rParams.Replace(nI, 1, OUString("\\/"));
                        }
                        nI++;
                        nLen++;
                    }

                    // Deal with language differences in date format expression.
                    // Should be made with i18n framework.
                    // The list of the mappings and of those "special" locales is to be found at:
                    // http://l10n.openoffice.org/i18n_framework/LocaleData.html
                    if ( !bForceJapanese && !bForceNatNum )
                    {
                        // Convert to the localized equivalent for OOo
                        switch ( rLang )
                        {
                        case LANGUAGE_FINNISH:
                            {
                                if (nChar == 'y' || nChar == 'Y')
                                    rParams.SetChar (nI, 'V');
                                else if (nChar == 'm' || nChar == 'M')
                                    rParams.SetChar (nI, 'K');
                                else if (nChar == 'd' || nChar == 'D')
                                    rParams.SetChar (nI, 'P');
                                else if (nChar == 'h' || nChar == 'H')
                                    rParams.SetChar (nI, 'T');
                            }
                            break;
                        case LANGUAGE_DANISH:
                        case LANGUAGE_NORWEGIAN:
                        case LANGUAGE_NORWEGIAN_BOKMAL:
                        case LANGUAGE_NORWEGIAN_NYNORSK:
                        case LANGUAGE_SWEDISH:
                        case LANGUAGE_SWEDISH_FINLAND:
                            {
                                if (nChar == 'h' || nChar == 'H')
                                    rParams.SetChar (nI, 'T');
                            }
                            break;
                        case LANGUAGE_PORTUGUESE:
                        case LANGUAGE_PORTUGUESE_BRAZILIAN:
                        case LANGUAGE_SPANISH_MODERN:
                        case LANGUAGE_SPANISH_DATED:
                        case LANGUAGE_SPANISH_MEXICAN:
                        case LANGUAGE_SPANISH_GUATEMALA:
                        case LANGUAGE_SPANISH_COSTARICA:
                        case LANGUAGE_SPANISH_PANAMA:
                        case LANGUAGE_SPANISH_DOMINICAN_REPUBLIC:
                        case LANGUAGE_SPANISH_VENEZUELA:
                        case LANGUAGE_SPANISH_COLOMBIA:
                        case LANGUAGE_SPANISH_PERU:
                        case LANGUAGE_SPANISH_ARGENTINA:
                        case LANGUAGE_SPANISH_ECUADOR:
                        case LANGUAGE_SPANISH_CHILE:
                        case LANGUAGE_SPANISH_URUGUAY:
                        case LANGUAGE_SPANISH_PARAGUAY:
                        case LANGUAGE_SPANISH_BOLIVIA:
                        case LANGUAGE_SPANISH_EL_SALVADOR:
                        case LANGUAGE_SPANISH_HONDURAS:
                        case LANGUAGE_SPANISH_NICARAGUA:
                        case LANGUAGE_SPANISH_PUERTO_RICO:
                            {
                                if (nChar == 'a' || nChar == 'A')
                                    rParams.SetChar (nI, 'O');
                                else if (nChar == 'y' || nChar == 'Y')
                                    rParams.SetChar (nI, 'A');
                            }
                            break;
                        case LANGUAGE_DUTCH:
                        case LANGUAGE_DUTCH_BELGIAN:
                            {
                                if (nChar == 'y' || nChar == 'Y')
                                    rParams.SetChar (nI, 'J');
                                else if (nChar == 'u' || nChar == 'U')
                                    rParams.SetChar (nI, 'H');
                            }
                            break;
                        case LANGUAGE_ITALIAN:
                        case LANGUAGE_ITALIAN_SWISS:
                            {
                                if (nChar == 'a' || nChar == 'A')
                                    rParams.SetChar (nI, 'O');
                                else if (nChar == 'g' || nChar == 'G')
                                    rParams.SetChar (nI, 'X');
                                else if (nChar == 'y' || nChar == 'Y')
                                    rParams.SetChar(nI, 'A');
                                else if (nChar == 'd' || nChar == 'D')
                                    rParams.SetChar (nI, 'G');
                            }
                            break;
                        case LANGUAGE_GERMAN:
                        case LANGUAGE_GERMAN_SWISS:
                        case LANGUAGE_GERMAN_AUSTRIAN:
                        case LANGUAGE_GERMAN_LUXEMBOURG:
                        case LANGUAGE_GERMAN_LIECHTENSTEIN:
                            {
                                if (nChar == 'y' || nChar == 'Y')
                                    rParams.SetChar (nI, 'J');
                                else if (nChar == 'd' || nChar == 'D')
                                    rParams.SetChar (nI, 'T');
                            }
                            break;
                        case LANGUAGE_FRENCH:
                        case LANGUAGE_FRENCH_BELGIAN:
                        case LANGUAGE_FRENCH_CANADIAN:
                        case LANGUAGE_FRENCH_SWISS:
                        case LANGUAGE_FRENCH_LUXEMBOURG:
                        case LANGUAGE_FRENCH_MONACO:
                            {
                                if (nChar == 'y' || nChar == 'Y' || nChar == 'a')
                                    rParams.SetChar (nI, 'A');
                                else if (nChar == 'd' || nChar == 'D' || nChar == 'j')
                                    rParams.SetChar (nI, 'J');
                            }
                            break;
                        default:
                            {
                                ; // Nothing
                            }
                        }
                    }
                }
                ++nI;
            }

            if (bForceNatNum)
                bForceJapanese = true;

            if (bForceJapanese)
                rLang = LANGUAGE_JAPANESE;

            if (bForceNatNum)
                rParams.Insert(OUString("[NatNum1][$-411]"),0);

            if (bHijri)
                rParams.Insert(OUString("[~hijri]"), 0);

            OUString sTemp(rParams);
            pFormatter->PutEntry(sTemp, nCheckPos, nType, nKey, rLang);
            rParams = sTemp;

            return nKey;
        }

        sal_Bool IsPreviousAM(String& rParams, xub_StrLen nPos){
            xub_StrLen nPos1 = nPos - 1;
            xub_StrLen nPos2 = nPos - 2;

            if(nPos1 > nPos || nPos2 > nPos){
                return sal_False;
            }else{
                return (
                    (rParams.GetChar(nPos1) == 'M'||rParams.GetChar(nPos1) == 'm')&&
                    (rParams.GetChar(nPos2) == 'A'||rParams.GetChar(nPos2) == 'a')
                    );
            }
        }
        sal_Bool IsNextPM(String& rParams, xub_StrLen nPos){
            xub_StrLen nPos1 = nPos + 1;
            xub_StrLen nPos2 = nPos + 2;


            if(nPos1 >= rParams.Len() - 1 || nPos2 > rParams.Len() - 1){
                return sal_False;
            }else{
                return (
                    (rParams.GetChar(nPos1) == 'P'||rParams.GetChar(nPos1) == 'p')&&
                    (rParams.GetChar(nPos2) == 'M'||rParams.GetChar(nPos2) == 'm')
                    );
            }

        }
        bool IsNotAM(String& rParams, xub_StrLen nPos)
        {
            return (
                    (nPos == rParams.Len() - 1) ||
                    (
                    (rParams.GetChar(nPos+1) != 'M') &&
                    (rParams.GetChar(nPos+1) != 'm')
                    )
                );
        }

        void SwapQuotesInField(OUString &rFmt)
        {
            //Swap unescaped " and ' with ' and "
            const sal_Int32 nLen = rFmt.getLength();
            for (sal_Int32 nI = 0; nI < nLen; ++nI)
            {
                if (!nI || rFmt[nI-1]!='\\')
                {
                    if (rFmt[nI]=='\"')
                        rFmt = rFmt.replaceAt(nI, 1, "\'");
                    else if (rFmt[nI]=='\'')
                        rFmt = rFmt.replaceAt(nI, 1, "\"");
                }
            }
        }

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
