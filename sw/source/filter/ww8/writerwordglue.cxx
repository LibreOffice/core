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
#include <IDocumentStylePoolAccess.hxx>

#include <algorithm>

#include <rtl/tencinfo.h>
#include <sal/log.hxx>

#include <unicode/ubidi.h>
#include <tools/tenccvt.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>

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

using namespace css;

namespace myImplHelpers
{
    static SwTwips CalcHdFtDist(const SwFrameFormat& rFormat, sal_uInt16 nSpacing)
    {
        /*
        The normal case for reexporting word docs is to have dynamic spacing,
        as this is word's only setting, and the reason for the existence of the
        dynamic spacing features. If we have dynamic spacing active then we can
        add its spacing to the value height of the h/f and get the wanted total
        size for word.

        Otherwise we have to get the real layout rendered
        height, which is totally nonoptimum, but the best we can do.
        */
        long nDist=0;
        const SwFormatFrameSize& rSz = rFormat.GetFrameSize();

        const SwHeaderAndFooterEatSpacingItem &rSpacingCtrl =
            sw::util::ItemGet<SwHeaderAndFooterEatSpacingItem>
            (rFormat, RES_HEADER_FOOTER_EAT_SPACING);
        if (rSpacingCtrl.GetValue())
            nDist += rSz.GetHeight();
        else
        {
            SwRect aRect(rFormat.FindLayoutRect());
            if (aRect.Height())
                nDist += aRect.Height();
            else
            {
                const SwFormatFrameSize& rSize = rFormat.GetFrameSize();
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

    static SwTwips CalcHdDist(const SwFrameFormat& rFormat)
    {
        return CalcHdFtDist(rFormat, rFormat.GetULSpace().GetUpper());
    }

    static SwTwips CalcFtDist(const SwFrameFormat& rFormat)
    {
        return CalcHdFtDist(rFormat, rFormat.GetULSpace().GetLower());
    }

    /*
     SwTextFormatColl and SwCharFormat are quite distinct types and how they are
     gotten is also distinct, but the algorithm to match word's equivalents into
     them is the same, so we put the different stuff into two separate helper
     implementations and a core template that uses the helpers that uses the
     same algorithm to do the work. We'll make the helpers specializations of a
     non existing template so I can let the compiler figure out the right one
     to use from a simple argument to the algorithm class
    */
    template <class C> class MapperImpl;
    template<> class MapperImpl<SwTextFormatColl>
    {
    private:
        SwDoc &mrDoc;
    public:
        MapperImpl(SwDoc &rDoc) : mrDoc(rDoc) {}
        SwTextFormatColl* GetBuiltInStyle(ww::sti eSti);
        SwTextFormatColl* GetStyle(const OUString &rName);
        SwTextFormatColl* MakeStyle(const OUString &rName);
    };

    SwTextFormatColl* MapperImpl<SwTextFormatColl>::GetBuiltInStyle(ww::sti eSti)
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

        SwTextFormatColl* pRet = nullptr;
        //If this is a built-in word style that has a built-in writer
        //equivalent, then map it to one of our built in styles regardless
        //of its name
        if (sal::static_int_cast< size_t >(eSti) < SAL_N_ELEMENTS(aArr) && aArr[eSti] != RES_NONE)
            pRet = mrDoc.getIDocumentStylePoolAccess().GetTextCollFromPool( static_cast< sal_uInt16 >(aArr[eSti]), false);
        return pRet;
    }

    SwTextFormatColl* MapperImpl<SwTextFormatColl>::GetStyle(const OUString &rName)
    {
        return sw::util::GetParaStyle(mrDoc, rName);
    }

    SwTextFormatColl* MapperImpl<SwTextFormatColl>::MakeStyle(const OUString &rName)
    {
        return mrDoc.MakeTextFormatColl(rName,
            mrDoc.GetDfltTextFormatColl());
    }

    template<> class MapperImpl<SwCharFormat>
    {
    private:
        SwDoc &mrDoc;
    public:
        MapperImpl(SwDoc &rDoc) : mrDoc(rDoc) {}
        SwCharFormat* GetBuiltInStyle(ww::sti eSti);
        SwCharFormat* GetStyle(const OUString &rName);
        SwCharFormat* MakeStyle(const OUString &rName);
    };

    SwCharFormat* MapperImpl<SwCharFormat>::GetBuiltInStyle(ww::sti eSti)
    {
        RES_POOL_CHRFMT_TYPE eLookup = RES_POOLCHR_NORMAL_END;
        switch (eSti)
        {
            case ww::stiFootnoteRef:
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
        SwCharFormat *pRet = nullptr;
        if (eLookup != RES_POOLCHR_NORMAL_END)
            pRet = mrDoc.getIDocumentStylePoolAccess().GetCharFormatFromPool( static_cast< sal_uInt16 >(eLookup) );
        return pRet;
    }

    SwCharFormat* MapperImpl<SwCharFormat>::GetStyle(const OUString &rName)
    {
        return sw::util::GetCharStyle(mrDoc, rName);
    }

    SwCharFormat* MapperImpl<SwCharFormat>::MakeStyle(const OUString &rName)
    {
        return mrDoc.MakeCharFormat(rName, mrDoc.GetDfltCharFormat());
    }

    template<class C> class StyleMapperImpl
    {
    private:
        MapperImpl<C> maHelper;
        std::set<const C*> maUsedStyles;
        C* MakeNonCollidingStyle(const OUString& rName);
    public:
        typedef std::pair<C*, bool> StyleResult;
        explicit StyleMapperImpl(SwDoc &rDoc) : maHelper(rDoc) {}
        StyleResult GetStyle(const OUString& rName, ww::sti eSti);
    };

    template<class C>
    typename StyleMapperImpl<C>::StyleResult
    StyleMapperImpl<C>::GetStyle(const OUString& rName, ww::sti eSti)
    {
        C *pRet = maHelper.GetBuiltInStyle(eSti);

        //If we've used it once, don't reuse it
        if (pRet && (maUsedStyles.end() != maUsedStyles.find(pRet)))
            pRet = nullptr;

        if (!pRet)
        {
            pRet = maHelper.GetStyle(rName);
            //If we've used it once, don't reuse it
            if (pRet && (maUsedStyles.end() != maUsedStyles.find(pRet)))
                pRet = nullptr;
        }

        bool bStyExist = pRet != nullptr;

        if (!pRet)
        {
            OUString aName(rName);
            sal_Int32 nIdx = rName.indexOf(',');
            // No commas allow in SW style names
            if (-1 != nIdx)
                aName = rName.copy( 0, nIdx );
            pRet = MakeNonCollidingStyle( aName );
        }

        if (pRet)
            maUsedStyles.insert(pRet);

        return StyleResult(pRet, bStyExist);
    }

    template<class C>
    C* StyleMapperImpl<C>::MakeNonCollidingStyle(const OUString& rName)
    {
        OUString aName(rName);
        C* pColl = 0;

        if (0 != (pColl = maHelper.GetStyle(aName)))
        {
            //If the style collides first stick WW- in front of it, unless
            //it already has it and then successively add a larger and
            //larger number after it, its got to work at some stage!
            if (!aName.startsWith("WW-"))
                aName = "WW-" + aName;

            sal_Int32 nI = 1;
            OUString aBaseName = aName;
            while (
                    0 != (pColl = maHelper.GetStyle(aName)) &&
                    (nI < SAL_MAX_INT32)
                  )
            {
                aName = aBaseName + OUString::number(nI++);
            }
        }

        return pColl ? 0 : maHelper.MakeStyle(aName);
    }

    static OUString FindBestMSSubstituteFont(const OUString &rFont)
    {
        if (IsStarSymbol(rFont))
            return OUString("Arial Unicode MS");
        return GetSubsFontName(rFont, SubsFontFlags::ONLYONE | SubsFontFlags::MS);
    }

    //Utility to remove entries before a given starting position
    class IfBeforeStart
    {
    private:
        sal_Int32 const mnStart;
    public:
        explicit IfBeforeStart(sal_Int32 nStart) : mnStart(nStart) {}
        bool operator()(const sw::util::CharRunEntry &rEntry) const
        {
            return rEntry.mnEndPos < mnStart;
        }
    };
}

/// Count what Word calls left/right margin from a format's LRSpace + Box.
static SvxLRSpaceItem lcl_getWordLRSpace(const SwFrameFormat& rFormat)
{
    SvxLRSpaceItem aLR(rFormat.GetLRSpace());
    const SvxBoxItem& rBox = rFormat.GetBox();

    aLR.SetLeft(aLR.GetLeft() + rBox.GetDistance(SvxBoxItemLine::LEFT));
    if (const editeng::SvxBorderLine* pLeft = rBox.GetLeft())
        aLR.SetLeft(aLR.GetLeft() + pLeft->GetWidth());

    aLR.SetRight(aLR.GetRight() + rBox.GetDistance(SvxBoxItemLine::RIGHT));
    if (const editeng::SvxBorderLine* pRight = rBox.GetRight())
        aLR.SetRight(aLR.GetRight() + pRight->GetWidth());

    return aLR;
}

namespace sw
{
    namespace util
    {

        bool IsPlausableSingleWordSection(const SwFrameFormat &rTitleFormat, const SwFrameFormat &rFollowFormat)
        {
            bool bPlausableSingleWordSection = true;

            const SwFormatCol& rFirstCols = rTitleFormat.GetCol();
            const SwFormatCol& rFollowCols = rFollowFormat.GetCol();
            const SwColumns& rFirstColumns = rFirstCols.GetColumns();
            const SwColumns& rFollowColumns = rFollowCols.GetColumns();
            SvxLRSpaceItem aOneLR = lcl_getWordLRSpace(rTitleFormat);
            SvxLRSpaceItem aTwoLR = lcl_getWordLRSpace(rFollowFormat);
            const SwFormatFrameSize& rFirstFrameSize = rTitleFormat.GetFrameSize();
            const SwFormatFrameSize& rFollowFrameSize = rFollowFormat.GetFrameSize();

            if (rFirstColumns.size() != rFollowColumns.size())
            {
                //e.g. #i4320#
                bPlausableSingleWordSection = false;
            }
            else if (aOneLR != aTwoLR)
                bPlausableSingleWordSection = false;
            else if (rFirstFrameSize != rFollowFrameSize)
                bPlausableSingleWordSection = false;
            else
            {
                HdFtDistanceGlue aOne(rTitleFormat.GetAttrSet());
                HdFtDistanceGlue aTwo(rFollowFormat.GetAttrSet());
                //e.g. #i14509#
                if (!aOne.StrictEqualTopBottom(aTwo))
                    bPlausableSingleWordSection = false;
            }
            return bPlausableSingleWordSection;
        }

        HdFtDistanceGlue::HdFtDistanceGlue(const SfxItemSet &rPage)
        {
            if (const SvxBoxItem *pBox = rPage.GetItem<SvxBoxItem>(RES_BOX))
            {
                dyaHdrTop = pBox->CalcLineSpace( SvxBoxItemLine::TOP, /*bEvenIfNoLine*/true );
                dyaHdrBottom = pBox->CalcLineSpace( SvxBoxItemLine::BOTTOM, /*bEvenIfNoLine*/true );
            }
            else
            {
                dyaHdrTop = dyaHdrBottom = 0;
            }
            const SvxULSpaceItem &rUL =
                ItemGet<SvxULSpaceItem>(rPage, RES_UL_SPACE);
            dyaHdrTop += rUL.GetUpper();
            dyaHdrBottom += rUL.GetLower();

            dyaTop = dyaHdrTop;
            dyaBottom = dyaHdrBottom;

            const SwFormatHeader *pHd = rPage.GetItem<SwFormatHeader>(RES_HEADER);
            if (pHd && pHd->IsActive() && pHd->GetHeaderFormat())
            {
                mbHasHeader = true;
                dyaTop = dyaTop + static_cast< sal_uInt16 >( (myImplHelpers::CalcHdDist(*(pHd->GetHeaderFormat()))) );
            }
            else
                mbHasHeader = false;

            const SwFormatFooter *pFt = rPage.GetItem<SwFormatFooter>(RES_FOOTER);
            if (pFt && pFt->IsActive() && pFt->GetFooterFormat())
            {
                mbHasFooter = true;
                dyaBottom = dyaBottom + static_cast< sal_uInt16 >( (myImplHelpers::CalcFtDist(*(pFt->GetFooterFormat()))) );
            }
            else
                mbHasFooter = false;
        }

        bool HdFtDistanceGlue::StrictEqualTopBottom(const HdFtDistanceGlue &rOther)
            const
        {
            // Check top only if both object have a header or if
            // both object don't have a header
            if (HasHeader() == rOther.HasHeader())
            {
                if (dyaTop != rOther.dyaTop)
                    return false;
            }

            // Check bottom only if both object have a footer or if
            // both object don't have a footer
            if (HasFooter() == rOther.HasFooter())
            {
                if (dyaBottom != rOther.dyaBottom)
                    return false;
            }

            return true;
        }

        ParaStyleMapper::ParaStyleMapper(SwDoc &rDoc)
            : mpImpl(new myImplHelpers::StyleMapperImpl<SwTextFormatColl>(rDoc))
        {
        }

        ParaStyleMapper::~ParaStyleMapper()
        {
        }

        ParaStyleMapper::StyleResult ParaStyleMapper::GetStyle(
            const OUString& rName, ww::sti eSti)
        {
            return mpImpl->GetStyle(rName, eSti);
        }

        CharStyleMapper::CharStyleMapper(SwDoc &rDoc)
            : mpImpl(new myImplHelpers::StyleMapperImpl<SwCharFormat>(rDoc))
        {
        }

        CharStyleMapper::~CharStyleMapper()
        {
        }

        CharStyleMapper::StyleResult CharStyleMapper::GetStyle(
            const OUString& rName, ww::sti eSti)
        {
            return mpImpl->GetStyle(rName, eSti);
        }

        FontMapExport::FontMapExport(const OUString &rFamilyName)
        {
            sal_Int32 nIndex = 0;
            msPrimary = GetNextFontToken(rFamilyName, nIndex);
            msSecondary = myImplHelpers::FindBestMSSubstituteFont(msPrimary);
            if (msSecondary.isEmpty() && nIndex != -1)
                msSecondary = GetNextFontToken(rFamilyName, nIndex);
        }

        bool ItemSort::operator()(sal_uInt16 nA, sal_uInt16 nB) const
        {
            /*
             #i24291#
             All we want to do is ensure for now is that if a charfmt exist
             in the character properties that it rises to the top and is
             exported first.  In the future we might find more ordering
             dependencies for export, in which case this is the place to do
             it
            */
            if (nA == nB)
                return false;
            if (nA == RES_TXTATR_CHARFMT)
                return true;
            if (nB == RES_TXTATR_CHARFMT)
                return false;
            if (nA == RES_TXTATR_INETFMT)
                return true;
            if (nB == RES_TXTATR_INETFMT)
               return false;
            return nA < nB;
        }

        CharRuns GetPseudoCharRuns(const SwTextNode& rTextNd)
        {
            const OUString &rText = rTextNd.GetText();

            bool bParaIsRTL = false;
            OSL_ENSURE(rTextNd.GetDoc(), "No document for node?, suspicious");
            if (rTextNd.GetDoc())
            {
                if (SvxFrameDirection::Horizontal_RL_TB ==
                    rTextNd.GetDoc()->GetTextDirection(SwPosition(rTextNd)))
                {
                    bParaIsRTL = true;
                }
            }

            using namespace ::com::sun::star::i18n;

            sal_uInt16 nScript = i18n::ScriptType::LATIN;
            assert(g_pBreakIt && g_pBreakIt->GetBreakIter().is());
            if (!rText.isEmpty())
                nScript = g_pBreakIt->GetBreakIter()->getScriptType(rText, 0);

            rtl_TextEncoding eChrSet = ItemGet<SvxFontItem>(rTextNd,
                GetWhichOfScript(RES_CHRATR_FONT, nScript)).GetCharSet();
            eChrSet = GetExtendedTextEncoding(eChrSet);

            CharRuns aRunChanges;

            if (rText.isEmpty())
            {
                aRunChanges.emplace_back(0, nScript, eChrSet,
                    bParaIsRTL);
                return aRunChanges;
            }

            typedef std::pair<int32_t, bool> DirEntry;
            typedef std::vector<DirEntry> DirChanges;

            typedef std::pair<sal_Int32, sal_uInt16> ScriptEntry;
            typedef std::vector<ScriptEntry> ScriptChanges;

            DirChanges aDirChanges;
            ScriptChanges aScripts;

            UBiDiDirection eDefaultDir = bParaIsRTL ? UBIDI_RTL : UBIDI_LTR;
            UErrorCode nError = U_ZERO_ERROR;
            UBiDi* pBidi = ubidi_openSized(rText.getLength(), 0, &nError);
            ubidi_setPara(pBidi, reinterpret_cast<const UChar *>(rText.getStr()), rText.getLength(),
                    static_cast< UBiDiLevel >(eDefaultDir), nullptr, &nError);

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
                aDirChanges.emplace_back(nEnd, nCurrDir & 0x1);
                nStart = nEnd;
            }
            ubidi_close(pBidi);

            assert(g_pBreakIt && g_pBreakIt->GetBreakIter().is());

            sal_Int32 nLen = rText.getLength();
            sal_Int32 nPos = 0;
            while (nPos < nLen)
            {
                sal_Int32 nEnd2 = g_pBreakIt->GetBreakIter()->endOfScript(rText, nPos,
                    nScript);
                if (nEnd2 < 0)
                    break;
                nPos = nEnd2;
                aScripts.emplace_back(nPos, nScript);
                nScript = g_pBreakIt->GetBreakIter()->getScriptType(rText, nPos);
            }

            auto aBiDiEnd = aDirChanges.cend();
            auto aScriptEnd = aScripts.cend();

            auto aBiDiIter = aDirChanges.cbegin();
            auto aScriptIter = aScripts.cbegin();

            bool bCharIsRTL = bParaIsRTL;

            while (
                    aBiDiIter != aBiDiEnd ||
                    aScriptIter != aScriptEnd
                  )
            {
                sal_Int32 nMinPos = rText.getLength();

                if (aBiDiIter != aBiDiEnd)
                {
                    if (aBiDiIter->first < nMinPos)
                        nMinPos = aBiDiIter->first;
                    bCharIsRTL = aBiDiIter->second;
                }

                if (aScriptIter != aScriptEnd)
                {
                    if (aScriptIter->first < nMinPos)
                        nMinPos = aScriptIter->first;
                    nScript = aScriptIter->second;
                }

                aRunChanges.emplace_back(nMinPos, nScript, eChrSet, bCharIsRTL);

                if (aBiDiIter != aBiDiEnd)
                {
                    if (aBiDiIter->first == nMinPos)
                        ++aBiDiIter;
                }

                if (aScriptIter != aScriptEnd)
                {
                    if (aScriptIter->first == nMinPos)
                        ++aScriptIter;
                }
            }

            aRunChanges.erase(std::remove_if(aRunChanges.begin(),
                aRunChanges.end(), myImplHelpers::IfBeforeStart(0/*nTextStart*/)), aRunChanges.end());

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

        static bool
        CanEncode(OUString const& rString, rtl_TextEncoding const eEncoding)
        {
            OString tmp;
            return rString.convertToString(&tmp, eEncoding,
                    RTL_TEXTTOUNICODE_FLAGS_UNDEFINED_ERROR |
                    RTL_TEXTTOUNICODE_FLAGS_INVALID_ERROR);
        }

        sal_uInt8 rtl_TextEncodingToWinCharsetRTF(
                OUString const& rFontName, OUString const& rAltName,
                rtl_TextEncoding eTextEncoding)
        {
            sal_uInt8 nRet =
                rtl_getBestWindowsCharsetFromTextEncoding(eTextEncoding);
            rtl_TextEncoding enc2 = rtl_getTextEncodingFromWindowsCharset(nRet);
            if (!rtl_isOctetTextEncoding(enc2) /* check to avoid asserts */ ||
                !(CanEncode(rFontName, enc2) && CanEncode(rAltName, enc2)))
            {
                static struct { rtl_TextEncoding enc; sal_uInt8 charset; }
                const s_fallbacks [] = {
                    { RTL_TEXTENCODING_MS_932, 0x80 }, // Shift-JIS
                    { RTL_TEXTENCODING_MS_936, 0x86 }, // GB-2312
                    { RTL_TEXTENCODING_MS_950, 0x88 }, // Big5
                    { RTL_TEXTENCODING_MS_949, 0x81 }, // EUC-KR
                };
                for (const auto & i : s_fallbacks)
                {
                    // fall back to a charset that can at least encode the
                    // font's name
                    if (CanEncode(rFontName, i.enc)
                        && CanEncode(rAltName, i.enc))
                    {
                        return i.charset;
                    }
                }
                SAL_INFO("sw.rtf", "no fallback charset found for font: "
                         << rFontName << " " << rAltName);
                nRet = 0x01; // all hope lost: "default", whatever that is
            }
            return nRet;
        }

        sal_uInt32 DateTime2DTTM( const DateTime& rDT )
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

            if ( rDT.GetDate() == 0 )
                return 0;
            sal_uInt32 nDT = ( rDT.GetDayOfWeek() + 1 ) % 7;
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


        /** Find cFind in rParams if not embedded in " double quotes.
            Will NOT find '\\' or '"'.
         */
        static sal_Int32 findUnquoted( const OUString& rParams, sal_Unicode cFind, sal_Int32 nFromPos )
        {
            const sal_Int32 nLen = rParams.getLength();
            if (nFromPos < 0 || nLen <= nFromPos)
                return -1;
            for (sal_Int32 nI = nFromPos; nI < nLen; ++nI)
            {
                const sal_Unicode c = rParams[nI];
                if (c == '\\')
                    ++nI;
                else if (c == '\"')
                {
                    ++nI;
                    // While not at the end and not at an unescaped end quote
                    while (nI < nLen)
                    {
                        if (rParams[nI] == '\"' && rParams[nI-1] != '\\')
                            break;
                        ++nI;
                    }
                }
                else //normal unquoted section
                {
                    if (c == cFind)
                        return nI;
                }
            }
            return -1;
        }

        /** Find all rFind in rParams if not embedded in " double quotes and
            replace with rReplace. Will NOT find '\\' or '"'.
         */
        static bool replaceUnquoted( OUString& rParams, const OUString& rFind, const OUString& rReplace )
        {
            bool bReplaced = false;
            if (rFind.isEmpty())
                return bReplaced;
            const sal_Unicode cFirst = rFind[0];

            sal_Int32 nLen = rParams.getLength();
            for (sal_Int32 nI = 0; nI < nLen; ++nI)
            {
                const sal_Unicode c = rParams[nI];
                if (rParams[nI] == '\\')
                    ++nI;
                else if (rParams[nI] == '\"')
                {
                    ++nI;
                    // While not at the end and not at an unescaped end quote
                    while (nI < nLen)
                    {
                        if (rParams[nI] == '\"' && rParams[nI-1] != '\\')
                            break;
                        ++nI;
                    }
                }
                else //normal unquoted section
                {
                    if (c == cFirst && rParams.match( rFind, nI))
                    {
                        const sal_Int32 nFindLen = rFind.getLength();
                        const sal_Int32 nDiff = rReplace.getLength() - nFindLen;
                        rParams = rParams.replaceAt( nI, nFindLen, rReplace);
                        nI += nFindLen + nDiff - 1;
                        nLen += nDiff;
                        bReplaced = true;
                    }
                }
            }
            return bReplaced;
        }

        sal_uLong MSDateTimeFormatToSwFormat(OUString& rParams,
            SvNumberFormatter *pFormatter, LanguageType &rLang, bool bHijri,
            LanguageType nDocLang)
        {
            // tell the Formatter about the new entry
            sal_Int32 nCheckPos = 0;
            SvNumFormatType nType = SvNumFormatType::DEFINED;
            sal_uInt32  nKey = 0;

            SwapQuotesInField(rParams);

            // Force to Japanese when finding one of 'geE'.
            // XXX This actually may not be correct, all era keywords could be
            // used in other locales as well. I just don't know about Word. But
            // this is how it was for 10 years..
            bool bForceJapanese = (-1 != findUnquoted( rParams, 'g', 0));
            // XXX Why replace? The number formatter does handle them and this
            // effectively changes from Gengou to Gregorian calendar. Legacy
            // because it wasn't supported a decade ago and now moot? Or is
            // that a Word specialty?
            bForceJapanese |= replaceUnquoted( rParams, "ee", "yyyy");
            bForceJapanese |= replaceUnquoted( rParams, "EE", "YYYY");
            if (LANGUAGE_FRENCH != nDocLang)
            {
                // Handle the 'a' case here
                sal_Int32 nLastPos = 0;
                do
                {
                    sal_Int32 nPos = findUnquoted( rParams, 'a', nLastPos + 1 );
                    bForceJapanese |= ( nPos != -1 && IsNotAM( rParams, nPos ) );
                    nLastPos = nPos;
                } while ( -1 != nLastPos );
            }

            // Force to NatNum when finding one of 'oOA'
            bool bForceNatNum  = replaceUnquoted( rParams, "o", "m")
                                 || replaceUnquoted( rParams, "O", "M");
            if (LANGUAGE_FRENCH != nDocLang)
            {
                // Handle the 'A' case here
                sal_Int32 nLastPos = 0;
                do
                {
                    sal_Int32 nPos = findUnquoted( rParams, 'A', nLastPos + 1 );
                    bool bIsCharA = ( nPos != -1 && IsNotAM( rParams, nPos ) );
                    bForceNatNum |= bIsCharA;
                    if ( bIsCharA )
                        rParams = rParams.replaceAt( nPos, 1, "D" );
                    nLastPos = nPos;
                } while ( -1 != nLastPos );
            }

            sal_Int32 nLen = rParams.getLength();
            for (sal_Int32 nI = 0; nI < nLen; ++nI)
            {
                if (rParams[nI] == '\\')
                    ++nI;
                else if (rParams[nI] == '\"')
                {
                    ++nI;
                    // While not at the end and not at an unescaped end quote
                    while (nI < nLen)
                    {
                        if (rParams[nI] == '\"' && rParams[nI-1] != '\\')
                            break;
                        ++nI;
                    }
                }
                else //normal unquoted section
                {
                    sal_Unicode nChar = rParams[nI];

                    // Change the localized word string to english
                    if ( nDocLang == LANGUAGE_FRENCH )
                    {
                        if ( ( nChar == 'a' || nChar == 'A' ) && IsNotAM(rParams, nI) )
                            rParams = rParams.replaceAt(nI, 1, "Y");
                    }
                    if (nChar == '/')
                    {
                        // MM: We have to escape '/' in case it's used as a char.
                        // But not if it's a '/' inside AM/PM
                        if (!(IsPreviousAM(rParams, nI) && IsNextPM(rParams, nI)))
                        {
                            rParams = rParams.replaceAt(nI, 1, "\\/");
                            nLen++;
                        }
                        nI++;
                    }

                    // Deal with language differences in date format expression.
                    // Should be made with i18n framework.
                    // The list of the mappings and of those "special" locales is to be found at:
                    // http://l10n.openoffice.org/i18n_framework/LocaleData.html
                    if ( !bForceJapanese && !bForceNatNum )
                    {
                        // Convert to the localized equivalent for OOo
                        if ( rLang == LANGUAGE_FINNISH )
                        {
                            if (nChar == 'y' || nChar == 'Y')
                                rParams = rParams.replaceAt(nI, 1, "V");
                            else if (nChar == 'm' || nChar == 'M')
                                rParams = rParams.replaceAt(nI, 1, "K");
                            else if (nChar == 'd' || nChar == 'D')
                                rParams = rParams.replaceAt(nI, 1, "P");
                            else if (nChar == 'h' || nChar == 'H')
                                rParams = rParams.replaceAt(nI, 1, "T");
                        }
                        else if ( rLang.anyOf(
                            LANGUAGE_DANISH,
                            LANGUAGE_NORWEGIAN,
                            LANGUAGE_NORWEGIAN_BOKMAL,
                            LANGUAGE_NORWEGIAN_NYNORSK,
                            LANGUAGE_SWEDISH,
                            LANGUAGE_SWEDISH_FINLAND))
                        {
                            if (nChar == 'h' || nChar == 'H')
                                rParams = rParams.replaceAt(nI, 1, "T");
                        }
                        else if ( rLang.anyOf(
                            LANGUAGE_PORTUGUESE,
                            LANGUAGE_PORTUGUESE_BRAZILIAN,
                            LANGUAGE_SPANISH_MODERN,
                            LANGUAGE_SPANISH_DATED,
                            LANGUAGE_SPANISH_MEXICAN,
                            LANGUAGE_SPANISH_GUATEMALA,
                            LANGUAGE_SPANISH_COSTARICA,
                            LANGUAGE_SPANISH_PANAMA,
                            LANGUAGE_SPANISH_DOMINICAN_REPUBLIC,
                            LANGUAGE_SPANISH_VENEZUELA,
                            LANGUAGE_SPANISH_COLOMBIA,
                            LANGUAGE_SPANISH_PERU,
                            LANGUAGE_SPANISH_ARGENTINA,
                            LANGUAGE_SPANISH_ECUADOR,
                            LANGUAGE_SPANISH_CHILE,
                            LANGUAGE_SPANISH_URUGUAY,
                            LANGUAGE_SPANISH_PARAGUAY,
                            LANGUAGE_SPANISH_BOLIVIA,
                            LANGUAGE_SPANISH_EL_SALVADOR,
                            LANGUAGE_SPANISH_HONDURAS,
                            LANGUAGE_SPANISH_NICARAGUA,
                            LANGUAGE_SPANISH_PUERTO_RICO))
                        {
                            if (nChar == 'a' || nChar == 'A')
                                rParams = rParams.replaceAt(nI, 1, "O");
                            else if (nChar == 'y' || nChar == 'Y')
                                rParams = rParams.replaceAt(nI, 1, "A");
                        }
                        else if ( rLang.anyOf(
                            LANGUAGE_DUTCH,
                            LANGUAGE_DUTCH_BELGIAN))
                        {
                            if (nChar == 'y' || nChar == 'Y')
                                rParams = rParams.replaceAt(nI, 1, "J");
                            else if (nChar == 'u' || nChar == 'U')
                                rParams = rParams.replaceAt(nI, 1, "H");
                        }
                        else if ( rLang.anyOf(
                                LANGUAGE_ITALIAN,
                                LANGUAGE_ITALIAN_SWISS))
                        {
                            if (nChar == 'a' || nChar == 'A')
                                rParams = rParams.replaceAt(nI, 1, "O");
                            else if (nChar == 'g' || nChar == 'G')
                                rParams = rParams.replaceAt(nI, 1, "X");
                            else if (nChar == 'y' || nChar == 'Y')
                                rParams = rParams.replaceAt(nI, 1, "A");
                            else if (nChar == 'd' || nChar == 'D')
                                rParams = rParams.replaceAt(nI, 1, "G");
                        }
                        else if ( rLang.anyOf(
                            LANGUAGE_GERMAN,
                            LANGUAGE_GERMAN_SWISS,
                            LANGUAGE_GERMAN_AUSTRIAN,
                            LANGUAGE_GERMAN_LUXEMBOURG,
                            LANGUAGE_GERMAN_LIECHTENSTEIN))
                        {
                            if (nChar == 'y' || nChar == 'Y')
                                rParams = rParams.replaceAt(nI, 1, "J");
                            else if (nChar == 'd' || nChar == 'D')
                                rParams = rParams.replaceAt(nI, 1, "T");
                        }
                        else if ( rLang.anyOf(
                            LANGUAGE_FRENCH,
                            LANGUAGE_FRENCH_BELGIAN,
                            LANGUAGE_FRENCH_CANADIAN,
                            LANGUAGE_FRENCH_SWISS,
                            LANGUAGE_FRENCH_LUXEMBOURG,
                            LANGUAGE_FRENCH_MONACO))
                        {
                            if (nChar == 'y' || nChar == 'Y' || nChar == 'a')
                                rParams = rParams.replaceAt(nI, 1, "A");
                            else if (nChar == 'd' || nChar == 'D' || nChar == 'j')
                                rParams = rParams.replaceAt(nI, 1, "J");
                        }
                    }
                }
            }

            if (bForceNatNum)
                bForceJapanese = true;

            if (bForceJapanese)
                rLang = LANGUAGE_JAPANESE;

            if (bForceNatNum)
                rParams = "[NatNum1][$-411]" + rParams;

            if (bHijri)
                rParams = "[~hijri]" + rParams;

            pFormatter->PutEntry(rParams, nCheckPos, nType, nKey, rLang);

            return nKey;
        }

        bool IsPreviousAM(OUString const & rParams, sal_Int32 nPos)
        {
            return nPos>=2 && rParams.matchIgnoreAsciiCase("am", nPos-2);
        }
        bool IsNextPM(OUString const & rParams, sal_Int32 nPos)
        {
            return nPos+2<rParams.getLength() && rParams.matchIgnoreAsciiCase("pm", nPos+1);
        }
        bool IsNotAM(OUString const & rParams, sal_Int32 nPos)
        {
            ++nPos;
            return nPos>=rParams.getLength() || (rParams[nPos]!='M' && rParams[nPos]!='m');
        }

        void SwapQuotesInField(OUString &rFormat)
        {
            //Swap unescaped " and ' with ' and "
            const sal_Int32 nLen = rFormat.getLength();
            for (sal_Int32 nI = 0; nI < nLen; ++nI)
            {
                if (!nI || rFormat[nI-1]!='\\')
                {
                    if (rFormat[nI]=='\"')
                        rFormat = rFormat.replaceAt(nI, 1, "\'");
                    else if (rFormat[nI]=='\'')
                        rFormat = rFormat.replaceAt(nI, 1, "\"");
                }
            }
        }

    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
