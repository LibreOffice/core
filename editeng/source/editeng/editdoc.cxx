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

#include <editeng/tstpitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/autokernitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/emphasismarkitem.hxx>
#include <editeng/charscaleitem.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/editids.hrc>
#include <editeng/editdata.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lspcitem.hxx>
#if defined(YRS)
#include <editeng/frmdiritem.hxx>
#include <editeng/hngpnctitem.hxx>
#include <editeng/forbiddenruleitem.hxx>
#include <editeng/scriptspaceitem.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/justifyitem.hxx>
#include <editeng/numdef.hxx>
#include <svl/itemiter.hxx>
#endif

#include <editdoc.hxx>
#include <editeng/eerdll.hxx>
#include <eerdll2.hxx>
#include "impedit.hxx"

#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <o3tl/safeint.hxx>
#include <osl/diagnose.h>

#include <svl/grabbagitem.hxx>
#include <svl/voiditem.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <libxml/xmlwriter.h>

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <memory>
#include <set>
#include <string_view>
#include <utility>

using namespace ::com::sun::star;


sal_uInt16 GetScriptItemId( sal_uInt16 nItemId, SvtScriptType nScriptType )
{
    sal_uInt16 nId = nItemId;

    if ( ( nScriptType == SvtScriptType::ASIAN ) ||
         ( nScriptType == SvtScriptType::COMPLEX ) )
    {
        switch ( nItemId )
        {
            case EE_CHAR_LANGUAGE:
                nId = ( nScriptType == SvtScriptType::ASIAN ) ? EE_CHAR_LANGUAGE_CJK : EE_CHAR_LANGUAGE_CTL;
            break;
            case EE_CHAR_FONTINFO:
                nId = ( nScriptType == SvtScriptType::ASIAN ) ? EE_CHAR_FONTINFO_CJK : EE_CHAR_FONTINFO_CTL;
            break;
            case EE_CHAR_FONTHEIGHT:
                nId = ( nScriptType == SvtScriptType::ASIAN ) ? EE_CHAR_FONTHEIGHT_CJK : EE_CHAR_FONTHEIGHT_CTL;
            break;
            case EE_CHAR_WEIGHT:
                nId = ( nScriptType == SvtScriptType::ASIAN ) ? EE_CHAR_WEIGHT_CJK : EE_CHAR_WEIGHT_CTL;
            break;
            case EE_CHAR_ITALIC:
                nId = ( nScriptType == SvtScriptType::ASIAN ) ? EE_CHAR_ITALIC_CJK : EE_CHAR_ITALIC_CTL;
            break;
        }
    }

    return nId;
}

bool IsScriptItemValid( sal_uInt16 nItemId, short nScriptType )
{
    bool bValid = true;

    switch ( nItemId )
    {
        case EE_CHAR_LANGUAGE:
            bValid = nScriptType == i18n::ScriptType::LATIN;
        break;
        case EE_CHAR_LANGUAGE_CJK:
            bValid = nScriptType == i18n::ScriptType::ASIAN;
        break;
        case EE_CHAR_LANGUAGE_CTL:
            bValid = nScriptType == i18n::ScriptType::COMPLEX;
        break;
        case EE_CHAR_FONTINFO:
            bValid = nScriptType == i18n::ScriptType::LATIN;
        break;
        case EE_CHAR_FONTINFO_CJK:
            bValid = nScriptType == i18n::ScriptType::ASIAN;
        break;
        case EE_CHAR_FONTINFO_CTL:
            bValid = nScriptType == i18n::ScriptType::COMPLEX;
        break;
        case EE_CHAR_FONTHEIGHT:
            bValid = nScriptType == i18n::ScriptType::LATIN;
        break;
        case EE_CHAR_FONTHEIGHT_CJK:
            bValid = nScriptType == i18n::ScriptType::ASIAN;
        break;
        case EE_CHAR_FONTHEIGHT_CTL:
            bValid = nScriptType == i18n::ScriptType::COMPLEX;
        break;
        case EE_CHAR_WEIGHT:
            bValid = nScriptType == i18n::ScriptType::LATIN;
        break;
        case EE_CHAR_WEIGHT_CJK:
            bValid = nScriptType == i18n::ScriptType::ASIAN;
        break;
        case EE_CHAR_WEIGHT_CTL:
            bValid = nScriptType == i18n::ScriptType::COMPLEX;
        break;
        case EE_CHAR_ITALIC:
            bValid = nScriptType == i18n::ScriptType::LATIN;
        break;
        case EE_CHAR_ITALIC_CJK:
            bValid = nScriptType == i18n::ScriptType::ASIAN;
        break;
        case EE_CHAR_ITALIC_CTL:
            bValid = nScriptType == i18n::ScriptType::COMPLEX;
        break;
    }

    return bValid;
}

EditCharAttrib* MakeCharAttrib( SfxItemPool& rPool, const SfxPoolItem& rAttr, sal_Int32 nS, sal_Int32 nE )
{
    // Create a new attribute in the pool
    switch( rAttr.Which() )
    {
        case EE_CHAR_LANGUAGE:
        case EE_CHAR_LANGUAGE_CJK:
        case EE_CHAR_LANGUAGE_CTL:
        {
            return new EditCharAttribLanguage(rPool, rAttr, nS, nE);
        }
        break;
        case EE_CHAR_COLOR:
        {
            return new EditCharAttribColor(rPool, rAttr, nS, nE );
        }
        break;
        case EE_CHAR_FONTINFO:
        case EE_CHAR_FONTINFO_CJK:
        case EE_CHAR_FONTINFO_CTL:
        {
            return new EditCharAttribFont(rPool, rAttr, nS, nE );
        }
        break;
        case EE_CHAR_FONTHEIGHT:
        case EE_CHAR_FONTHEIGHT_CJK:
        case EE_CHAR_FONTHEIGHT_CTL:
        {
            return new EditCharAttribFontHeight(rPool, rAttr, nS, nE );
        }
        break;
        case EE_CHAR_FONTWIDTH:
        {
            return new EditCharAttribFontWidth(rPool, rAttr, nS, nE );
        }
        break;
        case EE_CHAR_WEIGHT:
        case EE_CHAR_WEIGHT_CJK:
        case EE_CHAR_WEIGHT_CTL:
        {
            return new EditCharAttribWeight(rPool, rAttr, nS, nE );
        }
        break;
        case EE_CHAR_UNDERLINE:
        {
            return new EditCharAttribUnderline(rPool, rAttr, nS, nE );
        }
        break;
        case EE_CHAR_OVERLINE:
        {
            return new EditCharAttribOverline(rPool, rAttr, nS, nE );
        }
        break;
        case EE_CHAR_EMPHASISMARK:
        {
            return new EditCharAttribEmphasisMark(rPool, rAttr, nS, nE );
        }
        break;
        case EE_CHAR_RELIEF:
        {
            return new EditCharAttribRelief(rPool, rAttr, nS, nE );
        }
        break;
        case EE_CHAR_STRIKEOUT:
        {
            return new EditCharAttribStrikeout(rPool, rAttr, nS, nE );
        }
        break;
        case EE_CHAR_ITALIC:
        case EE_CHAR_ITALIC_CJK:
        case EE_CHAR_ITALIC_CTL:
        {
            return new EditCharAttribItalic(rPool, rAttr, nS, nE );
        }
        break;
        case EE_CHAR_OUTLINE:
        {
            return new EditCharAttribOutline(rPool, rAttr, nS, nE );
        }
        break;
        case EE_CHAR_SHADOW:
        {
            return new EditCharAttribShadow(rPool, rAttr, nS, nE );
        }
        break;
        case EE_CHAR_ESCAPEMENT:
        {
            return new EditCharAttribEscapement(rPool, rAttr, nS, nE );
        }
        break;
        case EE_CHAR_PAIRKERNING:
        {
            return new EditCharAttribPairKerning(rPool, rAttr, nS, nE );
        }
        break;
        case EE_CHAR_KERNING:
        {
            return new EditCharAttribKerning(rPool, rAttr, nS, nE );
        }
        break;
        case EE_CHAR_WLM:
        {
            return new EditCharAttribWordLineMode(rPool, rAttr, nS, nE );
        }
        break;
        case EE_CHAR_XMLATTRIBS:
        {
            return new EditCharAttrib(rPool, rAttr, nS, nE);  // Attribute is only for holding XML information...
        }
        break;
        case EE_CHAR_CASEMAP:
        {
            return new EditCharAttribCaseMap(rPool, rAttr, nS, nE );
        }
        break;
        case EE_CHAR_GRABBAG:
        {
            return new EditCharAttribGrabBag(rPool, rAttr, nS, nE );
        }
        break;
        case EE_FEATURE_TAB:
        {
            return new EditCharAttribTab(rPool, rAttr, nS );
        }
        break;
        case EE_FEATURE_LINEBR:
        {
            return new EditCharAttribLineBreak(rPool, rAttr, nS );
        }
        break;
        case EE_FEATURE_FIELD:
        {
            return new EditCharAttribField(rPool, rAttr, nS );
        }
        break;
        case EE_CHAR_BKGCOLOR:
        {
            return new EditCharAttribBackgroundColor(rPool, rAttr, nS, nE );
        }
        break;
        default:
        break;
    }

    OSL_FAIL( "Invalid Attribute!" );
    return nullptr;
}

void ParaPortion::MarkInvalid(sal_Int32 nStart, sal_Int32 nDiff)
{
    if (!mbInvalid)
    {
//      mnInvalidPosEnd = nStart;    // ??? => CreateLines
        mnInvalidPosStart = nDiff >= 0 ? nStart : nStart + nDiff;
        mnInvalidDiff = nDiff;
    }
    else
    {
        // Simple tap in succession
        if (nDiff > 0 && mnInvalidDiff > 0 && (mnInvalidPosStart + mnInvalidDiff) == nStart)
        {
            mnInvalidDiff = mnInvalidDiff + nDiff;
        }
        // Simple delete in succession
        else if (nDiff < 0 && mnInvalidDiff < 0 && mnInvalidPosStart == nStart)
        {
            mnInvalidPosStart = mnInvalidPosStart + nDiff;
            mnInvalidDiff = mnInvalidDiff + nDiff;
        }
        else
        {
//          mnInvalidPosEnd = pNode->Len();
            DBG_ASSERT(nDiff >= 0 || (nStart + nDiff) >= 0, "MarkInvalid: Diff out of Range");
            mnInvalidPosStart = std::min(mnInvalidPosStart, nDiff < 0 ? nStart + nDiff : nDiff);
            mnInvalidDiff = 0;
            mbSimple = false;
        }
    }
    mbInvalid = true;
    maScriptInfos.clear();
    maWritingDirectionInfos.clear();
}

void ParaPortion::MarkSelectionInvalid(sal_Int32 nStart)
{
    if ( !mbInvalid )
    {
        mnInvalidPosStart = nStart;
    }
    else
    {
        mnInvalidPosStart = std::min(mnInvalidPosStart, nStart);
    }
    mnInvalidDiff = 0;
    mbInvalid = true;
    mbSimple = false;
    maScriptInfos.clear();
    maWritingDirectionInfos.clear();
}

sal_Int32 ParaPortion::GetLineNumber( sal_Int32 nIndex ) const
{
    const sal_Int32 nCount = maLineList.Count();
    assert(nCount > 0 && "Empty ParaPortion in GetLine!");
    if (nCount == 0)
        return 0;
    DBG_ASSERT(mbVisible, "Why GetLine() on an invisible paragraph?");

    for ( sal_Int32 nLine = 0; nLine < nCount; nLine++ )
    {
        if (maLineList[nLine].IsIn(nIndex))
            return nLine;
    }

    // Then it should be at the end of the last line!
    DBG_ASSERT(nIndex == maLineList[nCount - 1].GetEnd(), "Index dead wrong!");
    return nCount - 1;
}

void ParaPortion::CorrectValuesBehindLastFormattedLine( sal_Int32 nLastFormattedLine )
{
    sal_Int32 nLines = maLineList.Count();
    DBG_ASSERT( nLines, "CorrectPortionNumbersFromLine: Empty Portion?" );
    if ( nLastFormattedLine < ( nLines - 1 ) )
    {
        const EditLine& rLastFormatted = maLineList[ nLastFormattedLine ];
        const EditLine& rUnformatted = maLineList[ nLastFormattedLine+1 ];
        sal_Int32 nPortionDiff = rUnformatted.GetStartPortion() - rLastFormatted.GetEndPortion();
        sal_Int32 nTextDiff = rUnformatted.GetStart() - rLastFormatted.GetEnd();
        nTextDiff++;    // LastFormatted->GetEnd() was included => 1 deducted too much!

        // The first unformatted must begin exactly one Portion behind the last
        // of the formatted:
        // If the modified line was split into one portion, can
        // nLastEnd > nNextStart!
        int nPDiff = -( nPortionDiff-1 );
        int nTDiff = -( nTextDiff-1 );
        if ( nPDiff || nTDiff )
        {
            for ( sal_Int32 nL = nLastFormattedLine+1; nL < nLines; nL++ )
            {
                EditLine& rLine = maLineList[ nL ];

                rLine.GetStartPortion() = rLine.GetStartPortion() + nPDiff;
                rLine.GetEndPortion() = rLine.GetEndPortion() + nPDiff;

                rLine.GetStart() = rLine.GetStart() + nTDiff;
                rLine.GetEnd() = rLine.GetEnd() + nTDiff;

                rLine.SetValid();
            }
        }
    }
    DBG_ASSERT(maLineList[maLineList.Count() - 1].GetEnd() == mpNode->Len(), "CorrectLines: The end is not right!");
}

// Shared reverse lookup acceleration pieces ...

namespace {

template<typename Array, typename Val>
sal_Int32 FastGetPos(const Array& rArray, const Val* p, sal_Int32& rLastPos)
{
    sal_Int32 nArrayLen = rArray.size();

    // Through certain filter code-paths we do a lot of appends, which in
    // turn call GetPos - creating some N^2 nightmares. If we have a
    // non-trivially large list, do a few checks from the end first.
    if (rLastPos > 16 && nArrayLen > 16)
    {
        sal_Int32 nEnd;
        if (rLastPos > nArrayLen - 2)
            nEnd = nArrayLen;
        else
            nEnd = rLastPos + 2;

        for (sal_Int32 nIdx = rLastPos - 2; nIdx < nEnd; ++nIdx)
        {
            if (rArray.at(nIdx).get() == p)
            {
                rLastPos = nIdx;
                return nIdx;
            }
        }
    }
    // The world's lamest linear search from svarray...
    for (sal_Int32 nIdx = 0; nIdx < nArrayLen; ++nIdx)
        if (rArray.at(nIdx).get() == p)
        {
            rLastPos = nIdx;
            return rLastPos;
        }

    // XXX "not found" condition for sal_Int32 indexes
    return EE_PARA_MAX;
}

}

sal_Int32 ParaPortionList::GetPos(const ParaPortion* p) const
{
    return FastGetPos(maPortions, p, nLastCache);
}

std::unique_ptr<ParaPortion> ParaPortionList::Release(sal_Int32 nPos)
{
    if (nPos < 0 || maPortions.size() <= o3tl::make_unsigned(nPos))
    {
        SAL_WARN( "editeng", "ParaPortionList::Release - out of bounds pos " << nPos);
        return nullptr;
    }
    std::unique_ptr<ParaPortion> p = std::move(maPortions[nPos]);
    maPortions.erase(maPortions.begin()+nPos);
    return p;
}

void ParaPortionList::Remove(sal_Int32 nPos)
{
    if (nPos < 0 || maPortions.size() <= o3tl::make_unsigned(nPos))
    {
        SAL_WARN( "editeng", "ParaPortionList::Remove - out of bounds pos " << nPos);
        return;
    }
    maPortions.erase(maPortions.begin()+nPos);
}

void ParaPortionList::Insert(sal_Int32 nPos, std::unique_ptr<ParaPortion> p)
{
    if (nPos < 0 || maPortions.size() < o3tl::make_unsigned(nPos))
    {
        SAL_WARN( "editeng", "ParaPortionList::Insert - out of bounds pos " << nPos);
        return;
    }
    maPortions.insert(maPortions.begin()+nPos, std::move(p));
}

void ParaPortionList::Append(std::unique_ptr<ParaPortion> p)
{
    maPortions.push_back(std::move(p));
}

sal_Int32 ParaPortionList::Count() const
{
    size_t nSize = maPortions.size();
    if (nSize > SAL_MAX_INT32)
    {
        SAL_WARN( "editeng", "ParaPortionList::Count - overflow " << nSize);
        return SAL_MAX_INT32;
    }
    return nSize;
}

void ParaPortionList::Reset()
{
    maPortions.clear();
}

tools::Long ParaPortionList::GetYOffset(const ParaPortion* pPPortion) const
{
    tools::Long nHeight = 0;
    for (const auto & rPortion : maPortions)
    {
        const ParaPortion* pTmpPortion = rPortion.get();
        if ( pTmpPortion == pPPortion )
            return nHeight;
        nHeight += pTmpPortion->GetHeight();
    }
    OSL_FAIL( "GetYOffset: Portion not found" );
    return nHeight;
}

sal_Int32 ParaPortionList::FindParagraph(tools::Long nYOffset) const
{
    tools::Long nY = 0;
    for (size_t i = 0, n = maPortions.size(); i < n; ++i)
    {
        nY += maPortions[i]->GetHeight(); // should also be correct even in bVisible!
        if ( nY > nYOffset )
            return i <= SAL_MAX_INT32 ? static_cast<sal_Int32>(i) : SAL_MAX_INT32;
    }
    return EE_PARA_MAX;
}

#if OSL_DEBUG_LEVEL > 0 && !defined NDEBUG
void
ParaPortionList::DbgCheck(ParaPortionList const& rParas, EditDoc const& rDoc)
{
    assert(rParas.Count() == rDoc.Count());
    for (sal_Int32 i = 0; i < rParas.Count(); ++i)
    {
        assert(rParas.SafeGetObject(i) != nullptr);
        assert(rParas.SafeGetObject(i)->GetNode() != nullptr);
        assert(rParas.SafeGetObject(i)->GetNode() == rDoc.GetObject(i));
    }
}
#endif

ContentAttribsInfo::ContentAttribsInfo( SfxItemSet aParaAttribs ) :
        aPrevParaAttribs(std::move( aParaAttribs))
{
}

void ContentAttribsInfo::AppendCharAttrib(EditCharAttrib* pNew)
{
    aPrevCharAttribs.push_back(std::unique_ptr<EditCharAttrib>(pNew));
}

void ConvertItem( std::unique_ptr<SfxPoolItem>& rPoolItem, MapUnit eSourceUnit, MapUnit eDestUnit )
{
    DBG_ASSERT( eSourceUnit != eDestUnit, "ConvertItem - Why?!" );

    switch ( rPoolItem->Which() )
    {
        case EE_PARA_LRSPACE:
        {
            assert(dynamic_cast<const SvxLRSpaceItem *>(rPoolItem.get()) != nullptr);
            SvxLRSpaceItem& rItem = static_cast<SvxLRSpaceItem&>(*rPoolItem);
            if (rItem.GetTextFirstLineOffset().m_nUnit == css::util::MeasureUnit::TWIP)
            {
                rItem.SetTextFirstLineOffset(
                    SvxIndentValue::twips(sal::static_int_cast<short>(OutputDevice::LogicToLogic(
                        rItem.ResolveTextFirstLineOffset({}), eSourceUnit, eDestUnit))));
            }
            rItem.SetTextLeft(SvxIndentValue::twips(
                OutputDevice::LogicToLogic(rItem.ResolveTextLeft({}), eSourceUnit, eDestUnit)));
            if (rItem.GetRight().m_nUnit == css::util::MeasureUnit::TWIP)
            {
                rItem.SetRight(SvxIndentValue::twips(
                    OutputDevice::LogicToLogic(rItem.ResolveRight({}), eSourceUnit, eDestUnit)));
            }
        }
        break;
        case EE_PARA_ULSPACE:
        {
            assert(dynamic_cast<const SvxULSpaceItem *>(rPoolItem.get()) != nullptr);
            SvxULSpaceItem& rItem = static_cast<SvxULSpaceItem&>(*rPoolItem);
            rItem.SetUpper( sal::static_int_cast< sal_uInt16 >( OutputDevice::LogicToLogic( rItem.GetUpper(), eSourceUnit, eDestUnit ) ) );
            rItem.SetLower( sal::static_int_cast< sal_uInt16 >( OutputDevice::LogicToLogic( rItem.GetLower(), eSourceUnit, eDestUnit ) ) );
        }
        break;
        case EE_PARA_SBL:
        {
            assert(dynamic_cast<const SvxLineSpacingItem *>(rPoolItem.get()) != nullptr);
            SvxLineSpacingItem& rItem = static_cast<SvxLineSpacingItem&>(*rPoolItem);
            // SetLineHeight changes also eLineSpace!
            if ( rItem.GetLineSpaceRule() == SvxLineSpaceRule::Min )
                rItem.SetLineHeight( sal::static_int_cast< sal_uInt16 >( OutputDevice::LogicToLogic( rItem.GetLineHeight(), eSourceUnit, eDestUnit ) ) );
        }
        break;
        case EE_PARA_TABS:
        {
            assert(dynamic_cast<const SvxTabStopItem *>(rPoolItem.get()) != nullptr);
            SvxTabStopItem& rItem = static_cast<SvxTabStopItem&>(*rPoolItem);
            SvxTabStopItem* pNewItem(new SvxTabStopItem(EE_PARA_TABS));

            if (sal_Int32 nDefTabDistance = rItem.GetDefaultDistance())
            {
                pNewItem->SetDefaultDistance(
                    OutputDevice::LogicToLogic(nDefTabDistance, eSourceUnit, eDestUnit));
            }

            for ( sal_uInt16 i = 0; i < rItem.Count(); i++ )
            {
                const SvxTabStop& rTab = rItem[i];
                SvxTabStop aNewStop( OutputDevice::LogicToLogic( rTab.GetTabPos(), eSourceUnit, eDestUnit ), rTab.GetAdjustment(), rTab.GetDecimal(), rTab.GetFill() );
                pNewItem->Insert( aNewStop );
            }
            rPoolItem.reset(pNewItem);
        }
        break;
        case EE_CHAR_FONTHEIGHT:
        case EE_CHAR_FONTHEIGHT_CJK:
        case EE_CHAR_FONTHEIGHT_CTL:
        {
            assert(dynamic_cast<const SvxFontHeightItem *>(rPoolItem.get()) != nullptr);
            SvxFontHeightItem& rItem = static_cast<SvxFontHeightItem&>(*rPoolItem);
            rItem.SetHeight( OutputDevice::LogicToLogic( rItem.GetHeight(), eSourceUnit, eDestUnit ) );
        }
        break;
    }
}

void ConvertAndPutItems( SfxItemSet& rDest, const SfxItemSet& rSource, const MapUnit* pSourceUnit, const MapUnit* pDestUnit )
{
    const SfxItemPool* pSourcePool = rSource.GetPool();
    const SfxItemPool* pDestPool = rDest.GetPool();

    for ( sal_uInt16 nWhich = EE_PARA_START; nWhich <= EE_CHAR_END; nWhich++ )
    {
        // If possible go through SlotID ...

        sal_uInt16 nSourceWhich = nWhich;
        sal_uInt16 nSlot = pDestPool->GetTrueSlotId( nWhich );
        if ( nSlot )
        {
            sal_uInt16 nW = pSourcePool->GetTrueWhichIDFromSlotID( nSlot );
            if ( nW )
                nSourceWhich = nW;
        }

        if ( rSource.GetItemState( nSourceWhich, false ) == SfxItemState::SET )
        {
            MapUnit eSourceUnit = pSourceUnit ? *pSourceUnit : pSourcePool->GetMetric( nSourceWhich );
            MapUnit eDestUnit = pDestUnit ? *pDestUnit : pDestPool->GetMetric( nWhich );
            if ( eSourceUnit != eDestUnit )
            {
                std::unique_ptr<SfxPoolItem> pItem(rSource.Get( nSourceWhich ).Clone());
                ConvertItem( pItem, eSourceUnit, eDestUnit );
                pItem->SetWhich(nWhich);
                rDest.Put( std::move(pItem) );
            }
            else
            {
                rDest.Put( rSource.Get( nSourceWhich ).CloneSetWhich(nWhich) );
            }
        }
    }
}

bool EditPaM::DbgIsBuggy( EditDoc const & rDoc ) const
{
    return !pNode ||
           rDoc.GetPos( pNode ) >= rDoc.Count() ||
           nIndex > pNode->Len();
}

bool EditSelection::DbgIsBuggy( EditDoc const & rDoc ) const
{
    return aStartPaM.DbgIsBuggy( rDoc ) || aEndPaM.DbgIsBuggy( rDoc );
}

void EditSelection::Adjust( const EditDoc& rNodes )
{
    DBG_ASSERT( aStartPaM.GetIndex() <= aStartPaM.GetNode()->Len(), "Index out of range in Adjust(1)" );
    DBG_ASSERT( aEndPaM.GetIndex() <= aEndPaM.GetNode()->Len(), "Index out of range in Adjust(2)" );

    const ContentNode* pStartNode = aStartPaM.GetNode();
    const ContentNode* pEndNode = aEndPaM.GetNode();

    sal_Int32 nStartNode = rNodes.GetPos( pStartNode );
    sal_Int32 nEndNode = rNodes.GetPos( pEndNode );

    DBG_ASSERT( nStartNode != SAL_MAX_INT32, "Node out of range in Adjust(1)" );
    DBG_ASSERT( nEndNode != SAL_MAX_INT32, "Node out of range in Adjust(2)" );

    const bool bSwap = ( nStartNode > nEndNode ) ||
                       ( ( nStartNode == nEndNode ) &&
                         ( aStartPaM.GetIndex() > aEndPaM.GetIndex() ) );

    if ( bSwap )
    {
        EditPaM aTmpPaM( aStartPaM );
        aStartPaM = aEndPaM;
        aEndPaM = aTmpPaM;
    }
}

#if defined(YRS)
#include <editeng/yrs.hxx>

namespace {

struct ReplayGuard
{
    IYrsTransactionSupplier *const m_pYrsSupplier;
    IYrsTransactionSupplier::Mode m_Mode;

    explicit ReplayGuard(IYrsTransactionSupplier *const pYrsSupplier)
        : m_pYrsSupplier(pYrsSupplier)
    {
        if (m_pYrsSupplier)
        {
            m_Mode = m_pYrsSupplier->SetMode(IYrsTransactionSupplier::Mode::Replay);
        }
    }
    ~ReplayGuard()
    {
        if (m_pYrsSupplier)
        {
            m_pYrsSupplier->SetMode(m_Mode);
        }
    }
};

struct YWrite
{
    YTransaction *const pTxn;
    Branch *const pProps;
    Branch *const pText;
};

constexpr char CH_PARA = 0x0d;

//struct YOutputDeleter { void operator()(YOutput *const p) const { youtput_destroy(p); } };
//struct YStringDeleter { void operator()(char *const p) const { ystring_destroy(p); } };

YWrite GetYWrite(IYrsTransactionSupplier *const pYrsSupplier, OString const& rId, YTransaction *const pInTxn = nullptr)
{
    if (!pYrsSupplier)
    {
        return { nullptr, nullptr, nullptr };
    }
    YDoc *const pDoc{pYrsSupplier->GetYDoc()};
    YTransaction *const pTxn{pInTxn ? pInTxn : pYrsSupplier->GetWriteTransaction()};
    // write is disabled when receiving edits from peers
    if (!pTxn)
    {
        return { nullptr, nullptr, nullptr };
    }
    assert(pDoc);
    Branch *const pComments{pYrsSupplier->GetCommentMap()};
    ::std::unique_ptr<YOutput, YOutputDeleter> const pComment{ymap_get(pComments, pTxn, rId.getStr())};
    yvalidate(pComment->tag == Y_ARRAY);
    yvalidate(pComment->len == 1);
    Branch *const pCommentArray{pComment->value.y_type};
    ::std::unique_ptr<YOutput, YOutputDeleter> const pProps{yarray_get(pCommentArray, pTxn, 1)};
    yvalidate(pProps->tag == Y_MAP);
    yvalidate(pProps->len == 1);
    ::std::unique_ptr<YOutput, YOutputDeleter> const pText{yarray_get(pCommentArray, pTxn, 2)};
    yvalidate(pText->tag == Y_TEXT);
    yvalidate(pText->len == 1);
    return { pTxn, pProps->value.y_type, pText->value.y_type };
}

void YSetVertical(IYrsTransactionSupplier *const pYrsSupplier, OString const& rCommentId, bool const isVertical)
{
    YWrite const yw{GetYWrite(pYrsSupplier, rCommentId)};
    if (yw.pTxn == nullptr)
    {
        return;
    }
    YInput const input{yinput_bool(isVertical ? Y_TRUE : Y_FALSE)};
    ymap_insert(yw.pProps, yw.pTxn, "is-vertical", &input);
}

void YSetRotation(IYrsTransactionSupplier *const pYrsSupplier, OString const& rCommentId, TextRotation const nRotation)
{
    YWrite const yw{GetYWrite(pYrsSupplier, rCommentId)};
    if (yw.pTxn == nullptr)
    {
        return;
    }
    YInput const input{yinput_long(static_cast<int64_t>(nRotation))};
    ymap_insert(yw.pProps, yw.pTxn, "rotation", &input);
}

void YSetDefTab(IYrsTransactionSupplier *const pYrsSupplier, OString const& rCommentId, sal_uInt16 const nDefTab)
{
    YWrite const yw{GetYWrite(pYrsSupplier, rCommentId)};
    if (yw.pTxn == nullptr)
    {
        return;
    }
    YInput const input{yinput_long(static_cast<int64_t>(nDefTab))};
    ymap_insert(yw.pProps, yw.pTxn, "def-tab", &input);
}

void YInsertAttribImpl(YWrite const& yw, uint32_t const offset, EditCharAttrib const*const pAttr);
void YInsertAttribImplImpl(YWrite const& yw, SfxPoolItem const& rItem, uint32_t const nStart, uint32_t const nLen);

void YAddPara(IYrsTransactionSupplier *const pYrsSupplier, OString const& rCommentId, EditDoc const& rDoc, uint32_t const index)
{
    YWrite const yw{GetYWrite(pYrsSupplier, rCommentId)};
    if (yw.pTxn == nullptr)
    {
        return;
    }
    SAL_DEBUG("YRS YAddPara");
    // need to encode into 1 YText
    char const para[]{ CH_PARA, '\0' };
    uint32_t i{0};
#if 0
    ::std::unique_ptr<char, YStringDeleter> const pText{ytext_string(yw.pText, yw.pTxn)};
    // UTF-8 index
    for (uint32_t paras = index; paras != 0 && pText[i] != 0; ++i)
    {
        if (pText[i] == CH_PARA)
        {
            --paras;
        }
    }
#else
    // UTF-16 index should be equal to EditDoc one
    for (auto paras{index}; paras != 0; --paras)
    {
        i += rDoc.GetObject(paras-1)->Len() + 1;
    }
#endif
    ContentAttribs const& rParaAttribs{rDoc.GetObject(index)->GetContentAttribs()};
    OString const styleName{OUStringToOString(rParaAttribs.GetStyleSheet()->GetName(), RTL_TEXTENCODING_UTF8)};
    YInput const style{yinput_string(styleName.getStr())};
    YInput attrArray[]{ style };
    char const*const attrNames[]{ "para-style" };
    YInput const attrs{yinput_json_map(const_cast<char**>(attrNames), attrArray, 1)};
    ytext_insert(yw.pText, yw.pTxn, i, para, &attrs);
    for (SfxItemIter it{rParaAttribs.GetItems()}; !it.IsAtEnd(); it.NextItem())
    {
        YInsertAttribImplImpl(yw, *it.GetCurItem(), i, 1);
    }
}

void YRemovePara(IYrsTransactionSupplier *const pYrsSupplier, OString const& rCommentId, EditDoc const& rDoc, uint32_t const index)
{
    YWrite const yw{GetYWrite(pYrsSupplier, rCommentId)};
    if (yw.pTxn == nullptr)
    {
        return;
    }
    uint32_t i{0};
#if 0
    ::std::unique_ptr<char, YStringDeleter> const pText{ytext_string(yw.pText, yw.pTxn)};
    for (uint32_t paras = index; paras != 0 && pText[i] != 0; ++i)
    {
        if (pText[i] == CH_PARA)
        {
            --paras;
        }
    }
#else
    if (index != 0)
    {
        for (auto paras{index}; paras != 0; --paras)
        {
            i += rDoc.GetObject(paras-1)->Len() + 1;
        }
    }
    uint32_t const len(rDoc.GetObject(index)->Len() + 1);
#endif
    ytext_remove_range(yw.pText, yw.pTxn, i, len);
}

void YClear(IYrsTransactionSupplier *const pYrsSupplier, OString const& rCommentId)
{
    YWrite const yw{GetYWrite(pYrsSupplier, rCommentId)};
    if (yw.pTxn == nullptr)
    {
        return;
    }
    auto const len{ytext_len(yw.pText, yw.pTxn)};
    ytext_remove_range(yw.pText, yw.pTxn, 0, len);
}

void YInsertParaBreak(IYrsTransactionSupplier *const pYrsSupplier, OString const& rCommentId, EditDoc const& rDoc, uint32_t const index, uint32_t const content)
{
    YWrite const yw{GetYWrite(pYrsSupplier, rCommentId)};
    if (yw.pTxn == nullptr)
    {
        return;
    }
    // need to encode into 1 YText
    char const para[]{ CH_PARA, '\0' };
    uint32_t i{0};
#if 0
    ::std::unique_ptr<char, YStringDeleter> const pText(ytext_string(yw.pText, yw.pTxn));
    if (index != 0)
    {
        for (uint32_t paras = index - 1; paras != 0 && pText[i] != 0; ++i)
        {
            if (pText[i] == CH_PARA)
            {
                --paras;
            }
        }
    }
    ...
#else
    for (auto paras{index}; paras != 0; --paras)
    {
        i += rDoc.GetObject(paras-1)->Len() + 1;
    }
    i += content;
#endif
    ContentAttribs const& rParaAttribs{rDoc.GetObject(index)->GetContentAttribs()};
    OString const styleName{OUStringToOString(rParaAttribs.GetStyleSheet()->GetName(), RTL_TEXTENCODING_UTF8)};
    YInput const style{yinput_string(styleName.getStr())};
    YInput attrArray[]{ style };
    char const*const attrNames[]{ "para-style" };
    YInput const attrs{yinput_json_map(const_cast<char**>(attrNames), attrArray, 1)};
    ytext_insert(yw.pText, yw.pTxn, i, para, &attrs);
    for (SfxItemIter it{rParaAttribs.GetItems()}; !it.IsAtEnd(); it.NextItem())
    {
        YInsertAttribImplImpl(yw, *it.GetCurItem(), i, 1);
    }
}

void YInsertText(IYrsTransactionSupplier *const pYrsSupplier, OString const& rCommentId, EditDoc const& rDoc, uint32_t const index, uint32_t const content, ::std::u16string_view const rText)
{
    YWrite const yw{GetYWrite(pYrsSupplier, rCommentId)};
    if (yw.pTxn == nullptr)
    {
        return;
    }
    uint32_t i{0};
    for (auto paras{index}; paras != 0; --paras)
    {
        i += rDoc.GetObject(paras-1)->Len() + 1;
    }
    i += content;
    OString const text{::rtl::OUStringToOString(rText, RTL_TEXTENCODING_UTF8)};
    ytext_insert(yw.pText, yw.pTxn, i, text.getStr(), nullptr);
}

void YConnectPara(IYrsTransactionSupplier *const pYrsSupplier, OString const& rCommentId, EditDoc const& rDoc, uint32_t const index, uint32_t const pos)
{
    YWrite const yw{GetYWrite(pYrsSupplier, rCommentId)};
    if (yw.pTxn == nullptr)
    {
        return;
    }
    uint32_t i{0};
#if 0
    ::std::unique_ptr<char, YStringDeleter> const pText{ytext_string(yw.pText, yw.pTxn)};
    for (uint32_t paras = index; paras != 0 && pText[i] != 0; ++i)
    {
        if (pText[i] == CH_PARA)
        {
            --paras;
        }
    }
#else
//    assert(index != 0);
    for (auto paras{index}; paras != 0; --paras)
    {
        i += rDoc.GetObject(paras-1)->Len() + 1;
    }
#endif
    i += pos;
    ytext_remove_range(yw.pText, yw.pTxn, i, 1);
}

void YRemoveChars(IYrsTransactionSupplier *const pYrsSupplier, OString const& rCommentId, EditDoc const& rDoc, uint32_t const index, uint32_t const content, uint32_t const length)
{
    YWrite const yw{GetYWrite(pYrsSupplier, rCommentId)};
    if (yw.pTxn == nullptr)
    {
        return;
    }
    uint32_t i{0};
    for (auto paras{index}; paras != 0; --paras)
    {
        i += rDoc.GetObject(paras-1)->Len() + 1;
    }
    i += content;
    ytext_remove_range(yw.pText, yw.pTxn, i, length);
}

void YSetStyle(IYrsTransactionSupplier *const pYrsSupplier, OString const& rCommentId, EditDoc const& rDoc, uint32_t const index, ::std::u16string_view const rStyle)
{
    YWrite const yw{GetYWrite(pYrsSupplier, rCommentId)};
    if (yw.pTxn == nullptr)
    {
        return;
    }
    uint32_t i{0};
    for (auto paras{index}; paras != 0; --paras)
    {
        i += rDoc.GetObject(paras-1)->Len() + 1;
    }
    i += rDoc.GetObject(index)->Len();
    OString const styleName{OUStringToOString(rStyle, RTL_TEXTENCODING_UTF8)};
    YInput const style{yinput_string(styleName.getStr())};
    YInput attrArray[]{ style };
    char const*const attrNames[]{ "para-style" };
    YInput const attrs{yinput_json_map(const_cast<char**>(attrNames), attrArray, 1)};
    ytext_format(yw.pText, yw.pTxn, i, 1, &attrs);
}

void YSetParaAttr(IYrsTransactionSupplier *const pYrsSupplier, OString const& rCommentId, EditDoc const& rDoc, uint32_t const index, SfxPoolItem const& rItem)
{
    YWrite const yw{GetYWrite(pYrsSupplier, rCommentId)};
    if (yw.pTxn == nullptr)
    {
        return;
    }
    uint32_t i{0};
    for (auto paras{index}; paras != 0; --paras)
    {
        i += rDoc.GetObject(paras-1)->Len() + 1;
    }
    i += rDoc.GetObject(index)->Len();
    YInsertAttribImplImpl(yw, rItem, i, 1);
}

char const* YWhichToAttrName(sal_Int16 const nWhich)
{
    switch (nWhich)
    {
        case EE_CHAR_COLOR:
            return "EE_CHAR_COLOR";
        case EE_CHAR_BKGCOLOR:
            return "EE_CHAR_BKGCOLOR";
        case EE_CHAR_FONTINFO:
            return "EE_CHAR_FONTINFO";
        case EE_CHAR_FONTINFO_CJK:
            return "EE_CHAR_FONTINFO_CJK";
        case EE_CHAR_FONTINFO_CTL:
            return "EE_CHAR_FONTINFO_CTL";
        case EE_CHAR_FONTHEIGHT:
            return "EE_CHAR_FONTHEIGHT";
        case EE_CHAR_FONTHEIGHT_CJK:
            return "EE_CHAR_FONTHEIGHT_CJK";
        case EE_CHAR_FONTHEIGHT_CTL:
            return "EE_CHAR_FONTHEIGHT_CTL";
        case EE_CHAR_FONTWIDTH:
            return "EE_CHAR_FONTWIDTH";
        case EE_CHAR_WEIGHT:
            return "EE_CHAR_WEIGHT";
        case EE_CHAR_WEIGHT_CJK:
            return "EE_CHAR_WEIGHT_CJK";
        case EE_CHAR_WEIGHT_CTL:
            return "EE_CHAR_WEIGHT_CTL";
        case EE_CHAR_UNDERLINE:
            return "EE_CHAR_UNDERLINE";
        case EE_CHAR_OVERLINE:
            return "EE_CHAR_OVERLINE";
        case EE_CHAR_STRIKEOUT:
            return "EE_CHAR_STRIKEOUT";
        case EE_CHAR_ITALIC:
            return "EE_CHAR_ITALIC";
        case EE_CHAR_ITALIC_CJK:
            return "EE_CHAR_ITALIC_CJK";
        case EE_CHAR_ITALIC_CTL:
            return "EE_CHAR_ITALIC_CTL";
        case EE_CHAR_OUTLINE:
            return "EE_CHAR_OUTLINE";
        case EE_CHAR_SHADOW:
            return "EE_CHAR_SHADOW";
        case EE_CHAR_ESCAPEMENT:
            return "EE_CHAR_ESCAPEMENT";
        case EE_CHAR_PAIRKERNING:
            return "EE_CHAR_PAIRKERNING";
        case EE_CHAR_KERNING:
            return "EE_CHAR_KERNING";
        case EE_CHAR_WLM:
            return "EE_CHAR_WLM";
        case EE_CHAR_LANGUAGE:
            return "EE_CHAR_LANGUAGE";
        case EE_CHAR_LANGUAGE_CJK:
            return "EE_CHAR_LANGUAGE_CJK";
        case EE_CHAR_LANGUAGE_CTL:
            return "EE_CHAR_LANGUAGE_CTL";
        case EE_CHAR_EMPHASISMARK:
            return "EE_CHAR_EMPHASISMARK";
        case EE_CHAR_RELIEF:
            return "EE_CHAR_RELIEF";
        case EE_CHAR_CASEMAP:
            return "EE_CHAR_CASEMAP";
        case EE_PARA_WRITINGDIR:
            return "EE_PARA_WRITINGDIR";
        case EE_PARA_HANGINGPUNCTUATION:
            return "EE_PARA_HANGINGPUNCTUATION";
        case EE_PARA_FORBIDDENRULES:
            return "EE_PARA_FORBIDDENRULES";
        case EE_PARA_ASIANCJKSPACING:
            return "EE_PARA_ASIANCJKSPACING";
//TODO complex, but apparently no way to set this in comment? inline constexpr TypedWhichId<SvxNumBulletItem>          EE_PARA_NUMBULLET          (EE_PARA_START+5);
        case EE_PARA_HYPHENATE:
            return "EE_PARA_HYPHENATE";
        case EE_PARA_HYPHENATE_NO_CAPS:
            return "EE_PARA_HYPHENATE_NO_CAPS";
        case EE_PARA_HYPHENATE_NO_LAST_WORD:
            return "EE_PARA_HYPHENATE_NO_LAST_WORD";
        case EE_PARA_BULLETSTATE:
            return "EE_PARA_BULLETSTATE";
//TODO no way to set this in comment? inline constexpr TypedWhichId<SvxLRSpaceItem>            EE_PARA_OUTLLRSPACE        (EE_PARA_START+10);
        case EE_PARA_OUTLLEVEL:
            return "EE_PARA_OUTLLEVEL";
//TODO complex, but apparently no way to set this in comment? inline constexpr TypedWhichId<SvxBulletItem>             EE_PARA_BULLET             (EE_PARA_START+12);
        case EE_PARA_LRSPACE:
            return "EE_PARA_LRSPACE";
        case EE_PARA_ULSPACE:
            return "EE_PARA_ULSPACE";
        case EE_PARA_SBL:
            return "EE_PARA_SBL";
        case EE_PARA_JUST:
            return "EE_PARA_JUST";
        case EE_PARA_TABS:
            return "EE_PARA_TABS";
        case EE_PARA_JUST_METHOD:
            return "EE_PARA_JUST_METHOD";
        case EE_PARA_VER_JUST:
            return "EE_PARA_VER_JUST";
        default:
            assert(false);
    }
}

void YRemoveAttrib(IYrsTransactionSupplier *const pYrsSupplier, OString const& rCommentId, EditDoc const& rDoc, uint32_t const index, sal_uInt16 const nWhich, sal_Int32 const nStart, sal_Int32 const nEnd)
{
    YWrite const yw{GetYWrite(pYrsSupplier, rCommentId)};
    if (yw.pTxn == nullptr)
    {
        return;
    }
    uint32_t i{0};
    for (auto paras{index}; paras != 0; --paras)
    {
        i += rDoc.GetObject(paras-1)->Len() + 1;
    }
    YInput const attr{yinput_null()};
    char const*const attrName{YWhichToAttrName(nWhich)};
    YInput const attrs{yinput_json_map(const_cast<char**>(&attrName), const_cast<YInput*>(&attr), 1)};
    ytext_format(yw.pText, yw.pTxn, i + nStart, nEnd - nStart, &attrs);
}

void YInsertAttrib(IYrsTransactionSupplier *const pYrsSupplier, OString const& rCommentId, EditDoc const& rDoc, uint32_t const index, EditCharAttrib const*const pAttr)
{
    YWrite const yw{GetYWrite(pYrsSupplier, rCommentId)};
    if (yw.pTxn == nullptr)
    {
        return;
    }
    uint32_t i{0};
    for (auto paras{index}; paras != 0; --paras)
    {
        i += rDoc.GetObject(paras-1)->Len() + 1;
    }
    YInsertAttribImpl(yw, i, pAttr);
}

void YInsertAttribImpl(YWrite const& yw, uint32_t const offset, EditCharAttrib const*const pAttr)
{
    auto const start{offset + pAttr->GetStart()};
    auto const len{pAttr->GetEnd() - pAttr->GetStart()};
    YInsertAttribImplImpl(yw, *pAttr->GetItem(), start, len);
}

void YInsertAttribImplImpl(YWrite const& yw, SfxPoolItem const& rItm, uint32_t const nStart, uint32_t const nLen)
{
    ::std::vector<YInput> tabStops;
    ::std::vector<::std::vector<YInput>> tabStopValues;
    ::std::vector<OString> tempStrings;
    ::std::vector<YInput> itemArray;
    ::std::vector<char const*> itemNames;
    YInput attr;
    char const* attrName;
    switch (rItm.Which())
    {
        case EE_CHAR_COLOR:
        case EE_CHAR_BKGCOLOR:
        {
            sal_uInt32 const nColor{static_cast<SvxColorItem const&>(rItm).getColor()};
            attr = yinput_long(nColor);
            attrName = rItm.Which() == EE_CHAR_COLOR ? "EE_CHAR_COLOR" : "EE_CHAR_BKGCOLOR";
            break;
        }
        case EE_CHAR_FONTINFO:
        case EE_CHAR_FONTINFO_CJK:
        case EE_CHAR_FONTINFO_CTL:
        {
            SvxFontItem const& rItem{static_cast<SvxFontItem const&>(rItm)};
            tempStrings.reserve(2); // prevent realloc
            tempStrings.emplace_back(OUStringToOString(rItem.GetFamilyName(), RTL_TEXTENCODING_UTF8));
            itemArray.emplace_back(yinput_string(tempStrings.back().getStr()));
            itemNames.emplace_back("familyname");
            tempStrings.emplace_back(OUStringToOString(rItem.GetStyleName(), RTL_TEXTENCODING_UTF8));
            itemArray.emplace_back(yinput_string(tempStrings.back().getStr()));
            itemNames.emplace_back("style");
            itemArray.emplace_back(yinput_long(uint64_t(rItem.GetFamily())));
            itemNames.emplace_back("family");
            itemArray.emplace_back(yinput_long(uint64_t(rItem.GetPitch())));
            itemNames.emplace_back("pitch");
            itemArray.emplace_back(yinput_long(uint64_t(rItem.GetCharSet())));
            itemNames.emplace_back("charset");
            attr = yinput_json_map(const_cast<char**>(itemNames.data()), itemArray.data(), itemArray.size());
            attrName = rItm.Which() == EE_CHAR_FONTINFO
                ? "EE_CHAR_FONTINFO"
                : rItm.Which() == EE_CHAR_FONTINFO_CJK ? "EE_CHAR_FONTINFO_CJK" : "EE_CHAR_FONTINFO_CTL";
            break;
        }
        case EE_CHAR_FONTHEIGHT:
        case EE_CHAR_FONTHEIGHT_CJK:
        case EE_CHAR_FONTHEIGHT_CTL:
        {
            SvxFontHeightItem const& rItem{static_cast<SvxFontHeightItem const&>(rItm)};
            itemNames.emplace_back("height");
            itemArray.emplace_back(yinput_long(uint64_t(rItem.GetHeight())));
            itemNames.emplace_back("prop");
            itemArray.emplace_back(yinput_long(uint64_t(rItem.GetProp())));
            itemNames.emplace_back("propunit");
            itemArray.emplace_back(yinput_long(uint64_t(rItem.GetPropUnit())));
            attr = yinput_json_map(const_cast<char**>(itemNames.data()), itemArray.data(), itemArray.size());
            attrName = rItm.Which() == EE_CHAR_FONTHEIGHT
                ? "EE_CHAR_FONTHEIGHT"
                : rItm.Which() == EE_CHAR_FONTHEIGHT_CJK ? "EE_CHAR_FONTHEIGHT_CJK" : "EE_CHAR_FONTHEIGHT_CTL";
            break;
        }
        case EE_CHAR_FONTWIDTH:
        {
            SvxCharScaleWidthItem const& rItem{static_cast<SvxCharScaleWidthItem const&>(rItm)};
            attr = yinput_long(rItem.GetValue());
            attrName = "EE_CHAR_FONTWIDTH";
            break;
        }
        case EE_CHAR_WEIGHT:
        case EE_CHAR_WEIGHT_CJK:
        case EE_CHAR_WEIGHT_CTL:
        {
            SvxWeightItem const& rItem{static_cast<SvxWeightItem const&>(rItm)};
            attr = yinput_long(uint64_t(rItem.GetWeight()));
            attrName = rItm.Which() == EE_CHAR_WEIGHT
                ? "EE_CHAR_WEIGHT"
                : rItm.Which() == EE_CHAR_WEIGHT_CJK ? "EE_CHAR_WEIGHT_CJK" : "EE_CHAR_WEIGHT_CTL";
            break;
        }
        case EE_CHAR_UNDERLINE:
        case EE_CHAR_OVERLINE:
        {
            SvxTextLineItem const& rItem{static_cast<SvxTextLineItem const&>(rItm)};
            itemArray.emplace_back(yinput_long(uint64_t(rItem.GetLineStyle())));
            itemNames.emplace_back("style");
            itemArray.emplace_back(yinput_long(uint64_t(sal_uInt32(rItem.GetColor()))));
            itemNames.emplace_back("color");
            attr = yinput_json_map(const_cast<char**>(itemNames.data()), itemArray.data(), itemArray.size());
            attrName = rItm.Which() == EE_CHAR_UNDERLINE ? "EE_CHAR_UNDERLINE" : "EE_CHAR_OVERLINE";
            break;
        }
        case EE_CHAR_STRIKEOUT:
        {
            SvxCrossedOutItem const& rItem{static_cast<SvxCrossedOutItem const&>(rItm)};
            attr = yinput_long(uint64_t(rItem.GetStrikeout()));
            attrName = "EE_CHAR_STRIKEOUT";
            break;
        }
        case EE_CHAR_ITALIC:
        case EE_CHAR_ITALIC_CJK:
        case EE_CHAR_ITALIC_CTL:
        {
            SvxPostureItem const& rItem{static_cast<SvxPostureItem const&>(rItm)};
            attr = yinput_long(uint64_t(rItem.GetPosture()));
            attrName = rItm.Which() == EE_CHAR_ITALIC
                ? "EE_CHAR_ITALIC"
                : rItm.Which() == EE_CHAR_ITALIC_CJK ? "EE_CHAR_ITALIC_CJK" : "EE_CHAR_ITALIC_CTL";
            break;
        }
        case EE_CHAR_OUTLINE:
        {
            SvxContourItem const& rItem{static_cast<SvxContourItem const&>(rItm)};
            attr = yinput_bool(rItem.GetValue() ? Y_TRUE : Y_FALSE);
            attrName = "EE_CHAR_OUTLINE";
            break;
        }
        case EE_CHAR_SHADOW:
        {
            SvxShadowedItem const& rItem{static_cast<SvxShadowedItem const&>(rItm)};
            attr = yinput_bool(rItem.GetValue() ? Y_TRUE : Y_FALSE);
            attrName = "EE_CHAR_SHADOW";
            break;
        }
        case EE_CHAR_ESCAPEMENT:
        {
            SvxEscapementItem const& rItem{static_cast<SvxEscapementItem const&>(rItm)};
            itemArray.emplace_back(yinput_long(uint64_t(rItem.GetEsc())));
            itemNames.emplace_back("esc");
            itemArray.emplace_back(yinput_long(uint64_t(rItem.GetProportionalHeight())));
            itemNames.emplace_back("prop");
            attr = yinput_json_map(const_cast<char**>(itemNames.data()), itemArray.data(), itemArray.size());
            attrName = "EE_CHAR_ESCAPEMENT";
            break;
        }
        case EE_CHAR_PAIRKERNING:
        {
            SvxAutoKernItem const& rItem{static_cast<SvxAutoKernItem const&>(rItm)};
            attr = yinput_bool(rItem.GetValue() ? Y_TRUE : Y_FALSE);
            attrName = "EE_CHAR_PAIRKERNING";
            break;
        }
        case EE_CHAR_KERNING:
        {
            SvxKerningItem const& rItem{static_cast<SvxKerningItem const&>(rItm)};
            attr = yinput_long(uint64_t(rItem.GetValue()));
            attrName = "EE_CHAR_KERNING";
            break;
        }
        case EE_CHAR_WLM:
        {
            SvxWordLineModeItem const& rItem{static_cast<SvxWordLineModeItem const&>(rItm)};
            attr = yinput_bool(rItem.GetValue() ? Y_TRUE : Y_FALSE);
            attrName = "EE_CHAR_WLM";
            break;
        }
        case EE_CHAR_LANGUAGE:
        case EE_CHAR_LANGUAGE_CJK:
        case EE_CHAR_LANGUAGE_CTL:
        {
            SvxLanguageItem const& rItem{static_cast<SvxLanguageItem const&>(rItm)};
            attr = yinput_long(uint64_t(rItem.GetValue().get()));
            attrName = rItm.Which() == EE_CHAR_LANGUAGE
                ? "EE_CHAR_LANGUAGE"
                : rItm.Which() == EE_CHAR_LANGUAGE_CJK ? "EE_CHAR_LANGUAGE_CJK" : "EE_CHAR_LANGUAGE_CTL";
            break;
        }
        case EE_CHAR_EMPHASISMARK:
        {
            SvxEmphasisMarkItem const& rItem{static_cast<SvxEmphasisMarkItem const&>(rItm)};
            attr = yinput_long(uint64_t(rItem.GetValue()));
            attrName = "EE_CHAR_EMPHASISMARK";
            break;
        }
        case EE_CHAR_RELIEF:
        {
            SvxCharReliefItem const& rItem{static_cast<SvxCharReliefItem const&>(rItm)};
            attr = yinput_long(uint64_t(rItem.GetValue()));
            attrName = "EE_CHAR_RELIEF";
            break;
        }
        case EE_CHAR_CASEMAP:
        {
            SvxCaseMapItem const& rItem{static_cast<SvxCaseMapItem const&>(rItm)};
            attr = yinput_long(uint64_t(rItem.GetValue()));
            attrName = "EE_CHAR_CASEMAP";
            break;
        }
        case EE_PARA_WRITINGDIR:
        {
            SvxFrameDirectionItem const& rItem{static_cast<SvxFrameDirectionItem const&>(rItm)};
            attr = yinput_long(uint64_t(rItem.GetValue()));
            attrName = "EE_PARA_WRITINGDIR";
            break;
        }
        case EE_PARA_HANGINGPUNCTUATION:
        {
            SvxHangingPunctuationItem const& rItem{static_cast<SvxHangingPunctuationItem const&>(rItm)};
            attr = yinput_bool(rItem.GetValue() ? Y_TRUE : Y_FALSE);
            attrName = "EE_PARA_HANGINGPUNCTUATION";
            break;
        }
        case EE_PARA_FORBIDDENRULES:
        {
            SvxForbiddenRuleItem const& rItem{static_cast<SvxForbiddenRuleItem const&>(rItm)};
            attr = yinput_bool(rItem.GetValue() ? Y_TRUE : Y_FALSE);
            attrName = "EE_PARA_FORBIDDENRULES";
            break;
        }
        case EE_PARA_ASIANCJKSPACING:
        {
            SvxScriptSpaceItem const& rItem{static_cast<SvxScriptSpaceItem const&>(rItm)};
            attr = yinput_bool(rItem.GetValue() ? Y_TRUE : Y_FALSE);
            attrName = "EE_PARA_ASIANCJKSPACING";
            break;
        }
//TODO complex, but apparently no way to set this in comment? inline constexpr TypedWhichId<SvxNumBulletItem>          EE_PARA_NUMBULLET          (EE_PARA_START+5);
        case EE_PARA_HYPHENATE:
        case EE_PARA_HYPHENATE_NO_CAPS:
        case EE_PARA_HYPHENATE_NO_LAST_WORD:
        case EE_PARA_BULLETSTATE:
        {
            SfxBoolItem const& rItem{static_cast<SfxBoolItem const&>(rItm)};
            attr = yinput_bool(rItem.GetValue() ? Y_TRUE : Y_FALSE);
            attrName = rItm.Which() == EE_PARA_HYPHENATE
                ? "EE_PARA_HYPHENATE"
                : rItm.Which() == EE_PARA_HYPHENATE_NO_CAPS
                    ? "EE_PARA_HYPHENATE_NO_CAPS"
                    : rItm.Which() == EE_PARA_HYPHENATE_NO_LAST_WORD
                        ? "EE_PARA_HYPHENATE_NO_LAST_WORD"
                        : "EE_PARA_BULLETSTATE";
            break;
        }
//TODO no way to set this in comment? inline constexpr TypedWhichId<SvxLRSpaceItem>            EE_PARA_OUTLLRSPACE        (EE_PARA_START+10);
        case EE_PARA_OUTLLEVEL:
        {
            SfxInt16Item const& rItem{static_cast<SfxInt16Item const&>(rItm)};
            attr = yinput_long(uint64_t(rItem.GetValue()));
            attrName = "EE_PARA_OUTLLEVEL";
            break;
        }
//TODO complex, but apparently no way to set this in comment? inline constexpr TypedWhichId<SvxBulletItem>             EE_PARA_BULLET             (EE_PARA_START+12);
        case EE_PARA_LRSPACE:
        {
            SvxLRSpaceItem const& rItem{static_cast<SvxLRSpaceItem const&>(rItm)};
            itemArray.emplace_back(yinput_float(rItem.GetTextFirstLineOffset().m_dValue));
            itemNames.emplace_back("first-line-offset");
            itemArray.emplace_back(yinput_long(uint64_t(rItem.GetTextFirstLineOffset().m_nUnit)));
            itemNames.emplace_back("first-line-offset-unit");
            itemArray.emplace_back(yinput_float(rItem.GetLeft().m_dValue));
            itemNames.emplace_back("left-margin");
            itemArray.emplace_back(yinput_long(uint64_t(rItem.GetLeft().m_nUnit)));
            itemNames.emplace_back("left-margin-unit");
            itemArray.emplace_back(yinput_float(rItem.GetRight().m_dValue));
            itemNames.emplace_back("right-margin");
            itemArray.emplace_back(yinput_long(uint64_t(rItem.GetRight().m_nUnit)));
            itemNames.emplace_back("right-margin-unit");
            itemArray.emplace_back(yinput_bool(rItem.IsAutoFirst() ? Y_TRUE : Y_FALSE));
            itemNames.emplace_back("auto-first");
            attr = yinput_json_map(const_cast<char**>(itemNames.data()), itemArray.data(), itemArray.size());
            attrName = "EE_PARA_LRSPACE";
            break;
        }
        case EE_PARA_ULSPACE:
        {
            SvxULSpaceItem const& rItem{static_cast<SvxULSpaceItem const&>(rItm)};
            itemArray.emplace_back(yinput_long(uint64_t(rItem.GetUpper())));
            itemNames.emplace_back("upper-margin");
            itemArray.emplace_back(yinput_long(uint64_t(rItem.GetLower())));
            itemNames.emplace_back("lower-margin");
            // TODO what does EE support here?
            attr = yinput_json_map(const_cast<char**>(itemNames.data()), itemArray.data(), itemArray.size());
            attrName = "EE_PARA_ULSPACE";
            break;
        }
        case EE_PARA_SBL:
        {
            SvxLineSpacingItem const& rItem{static_cast<SvxLineSpacingItem const&>(rItm)};
            switch (rItem.GetLineSpaceRule())
            {
                case SvxLineSpaceRule::Auto:
                    break;
                case SvxLineSpaceRule::Fix:
                    itemArray.emplace_back(yinput_long(uint64_t(rItem.GetLineHeight())));
                    itemNames.emplace_back("line-space-fix");
                    break;
                case SvxLineSpaceRule::Min:
                    itemArray.emplace_back(yinput_long(uint64_t(rItem.GetLineHeight())));
                    itemNames.emplace_back("line-space-min");
                    break;
            }
            switch (rItem.GetInterLineSpaceRule())
            {
                case SvxInterLineSpaceRule::Off:
                    break;
                case SvxInterLineSpaceRule::Prop:
                    itemArray.emplace_back(yinput_long(uint64_t(rItem.GetPropLineSpace())));
                    itemNames.emplace_back("inter-line-space-prop");
                    break;
                case SvxInterLineSpaceRule::Fix:
                    itemArray.emplace_back(yinput_long(uint64_t(rItem.GetInterLineSpace())));
                    itemNames.emplace_back("inter-line-space-fix");
                    break;
            }
            attr = yinput_json_map(const_cast<char**>(itemNames.data()), itemArray.data(), itemArray.size());
            attrName = "EE_PARA_SBL";
            break;
        }
        case EE_PARA_JUST:
        {
            SvxAdjustItem const& rItem{static_cast<SvxAdjustItem const&>(rItm)};
            switch (rItem.GetAdjust())
            {
                case SvxAdjust::Left:
                case SvxAdjust::Right:
                case SvxAdjust::Center:
                    attr = yinput_long(uint64_t(rItem.GetAdjust()));
                    break;
                case SvxAdjust::Block:
                    switch (rItem.GetLastBlock())
                    {
                        case SvxAdjust::Left:
                            attr = yinput_long(uint64_t(SvxAdjust::Block));
                            break;
                        case SvxAdjust::Center:
                            attr = yinput_long(uint64_t(5));
                            break;
                        case SvxAdjust::Block:
                            attr = yinput_long(uint64_t(rItem.GetOneWord() == SvxAdjust::Block ? 7 : 6));
                            break;
                        default:
                            assert(false);
                    }
                    break;
                default:
                    assert(false);
            }
            attrName = "EE_PARA_JUST";
            break;
        }
        case EE_PARA_TABS:
        {
            SvxTabStopItem const& rItem{static_cast<SvxTabStopItem const&>(rItm)};
            itemNames.emplace_back("default-distance");
            itemArray.emplace_back(yinput_long(uint64_t(rItem.GetDefaultDistance())));
            tabStopValues.reserve(rItem.Count()); // prevent realloc
            for (decltype(rItem.Count()) i{0}; i < rItem.Count(); ++i)
            {
                SvxTabStop const& rTab{rItem.At(i)};
                char const*const names[]{"pos", "adjustment", "decimal", "fill"};
                tabStopValues.emplace_back();
                tabStopValues.back().emplace_back(yinput_long(uint64_t(rTab.GetTabPos())));
                tabStopValues.back().emplace_back(yinput_long(uint64_t(rTab.GetAdjustment())));
                tabStopValues.back().emplace_back(yinput_long(uint64_t(rTab.GetDecimal())));
                tabStopValues.back().emplace_back(yinput_long(uint64_t(rTab.GetFill())));
                tabStops.emplace_back(yinput_json_map(const_cast<char**>(names), tabStopValues.back().data(), 4));
            }
            itemNames.emplace_back("tab-stops");
            itemArray.emplace_back(yinput_json_array(tabStops.data(), tabStops.size()));
            attr = yinput_json_map(const_cast<char**>(itemNames.data()), itemArray.data(), itemArray.size());
            attrName = "EE_PARA_TABS";
            break;
        }
        case EE_PARA_JUST_METHOD:
        {
            SvxJustifyMethodItem const& rItem{static_cast<SvxJustifyMethodItem const&>(rItm)};
            attr = yinput_long(uint64_t(rItem.GetValue()));
            attrName = "EE_PARA_JUST_METHOD";
            break;
        }
        case EE_PARA_VER_JUST:
        {
            SvxVerJustifyItem const& rItem{static_cast<SvxVerJustifyItem const&>(rItm)};
            attr = yinput_long(uint64_t(rItem.GetValue()));
            attrName = "EE_PARA_VER_JUST";
            break;
        }
        // these aren't editable?
//constexpr TypedWhichId<SvXMLAttrContainerItem> EE_CHAR_XMLATTRIBS     (EE_CHAR_START+27);
//constexpr TypedWhichId<SfxGrabBagItem>         EE_CHAR_GRABBAG        (EE_CHAR_START+30);

        default:
            assert(false);
    }
    assert(itemNames.size() == itemArray.size());
    YInput const attrs{yinput_json_map(const_cast<char**>(&attrName), &attr, 1)};
    ytext_format(yw.pText, yw.pTxn, nStart, nLen, &attrs);
}

void YInsertFeature(IYrsTransactionSupplier *const pYrsSupplier, OString const& rCommentId, EditDoc const& rDoc, uint32_t const index, EditCharAttrib const*const pAttr)
{
    YWrite const yw{GetYWrite(pYrsSupplier, rCommentId)};
    if (yw.pTxn == nullptr)
    {
        return;
    }
    uint32_t i{0};
    for (auto paras{index}; paras != 0; --paras)
    {
        i += rDoc.GetObject(paras-1)->Len() + 1;
    }
    i += pAttr->GetStart();
    char const feature[]{ CH_FEATURE, '\0' };
    switch (pAttr->Which())
    {
        case EE_FEATURE_TAB:
        case EE_FEATURE_LINEBR:
        {
            YInput const type{yinput_string(pAttr->Which() == EE_FEATURE_TAB ? "tab" : "line")};
            YInput attrArray[]{ type };
            char const*const attrNames[]{ "feature" };
            YInput const attrs{yinput_json_map(const_cast<char**>(attrNames), attrArray, 1)};
            ytext_insert(yw.pText, yw.pTxn, i, feature, &attrs);
            break;
        }
        case EE_FEATURE_FIELD:
        {
            SvxURLField const*const pURLField{dynamic_cast<SvxURLField const*>(dynamic_cast<SvxFieldItem const*>(pAttr->GetItem())->GetField())};
            assert(pURLField);
            YInput const type{yinput_string("url")};
            // ??? somehow this comes out as Y_JSON_NUM at the other end?
            YInput const format{yinput_long(static_cast<int64_t>(pURLField->GetFormat()))};
            OString const urlStr{OUStringToOString(pURLField->GetURL(), RTL_TEXTENCODING_UTF8)};
            YInput const url{yinput_string(urlStr.getStr())};
            OString const reprStr{OUStringToOString(pURLField->GetRepresentation(), RTL_TEXTENCODING_UTF8)};
            YInput const representation{yinput_string(reprStr.getStr())};
            OString const targetStr{OUStringToOString(pURLField->GetTargetFrame(), RTL_TEXTENCODING_UTF8)};
            YInput const targetframe{yinput_string(targetStr.getStr())};
            YInput attrArray[]{ type, format, url, representation, targetframe };
            char const*const attrNames[]{ "feature", "url-format", "url-url", "url-representation", "url-targetframe" };
            // yinput_ymap doesn't work!
            YInput const attrs{yinput_json_map(const_cast<char**>(attrNames), attrArray, 5)};
            ytext_insert(yw.pText, yw.pTxn, i, feature, &attrs);

            break;
        }
        default: // EE_FEATURE_NOTCONV appears unused?
            assert(false);
    }
}

uint64_t YReadInt(YOutput const& rValue)
{
    // with the v1 encoding, JSON is being sent apparently (like "family":2) , which has issues with integers being sometimes read as floats so workaround here
    if (rValue.tag == Y_JSON_INT)
    {
        return rValue.value.integer;
    }
    else
    {
        yvalidate(rValue.tag == Y_JSON_NUM);
        return ::std::lround(rValue.value.num);
    }
}

void
YImplInsertAttr(SfxItemSet & rSet, ::std::vector<sal_uInt16> *const pRemoved,
    char const*const pKey, YOutput const& rValue)
{
    sal_uInt16 nWhich{0};
    if (strcmp(pKey, "EE_CHAR_COLOR") == 0)
    {
        nWhich = EE_CHAR_COLOR;
    }
    else if (strcmp(pKey, "EE_CHAR_BKGCOLOR") == 0)
    {
        nWhich = EE_CHAR_BKGCOLOR;
    }
    else if (strcmp(pKey, "EE_CHAR_FONTINFO") == 0)
    {
        nWhich = EE_CHAR_FONTINFO;
    }
    else if (strcmp(pKey, "EE_CHAR_FONTINFO_CJK") == 0)
    {
        nWhich = EE_CHAR_FONTINFO_CJK;
    }
    else if (strcmp(pKey, "EE_CHAR_FONTINFO_CTL") == 0)
    {
        nWhich = EE_CHAR_FONTINFO_CTL;
    }
    else if (strcmp(pKey, "EE_CHAR_FONTHEIGHT") == 0)
    {
        nWhich = EE_CHAR_FONTHEIGHT;
    }
    else if (strcmp(pKey, "EE_CHAR_FONTHEIGHT_CJK") == 0)
    {
        nWhich = EE_CHAR_FONTHEIGHT_CJK;
    }
    else if (strcmp(pKey, "EE_CHAR_FONTHEIGHT_CTL") == 0)
    {
        nWhich = EE_CHAR_FONTHEIGHT_CTL;
    }
    else if (strcmp(pKey, "EE_CHAR_FONTWIDTH") == 0)
    {
        nWhich = EE_CHAR_FONTWIDTH;
    }
    else if (strcmp(pKey, "EE_CHAR_WEIGHT") == 0)
    {
        nWhich = EE_CHAR_WEIGHT;
    }
    else if (strcmp(pKey, "EE_CHAR_WEIGHT_CJK") == 0)
    {
        nWhich = EE_CHAR_WEIGHT_CJK;
    }
    else if (strcmp(pKey, "EE_CHAR_WEIGHT_CTL") == 0)
    {
        nWhich = EE_CHAR_WEIGHT_CTL;
    }
    else if (strcmp(pKey, "EE_CHAR_UNDERLINE") == 0)
    {
        nWhich = EE_CHAR_UNDERLINE;
    }
    else if (strcmp(pKey, "EE_CHAR_OVERLINE") == 0)
    {
        nWhich = EE_CHAR_OVERLINE;
    }
    else if (strcmp(pKey, "EE_CHAR_STRIKEOUT") == 0)
    {
        nWhich = EE_CHAR_STRIKEOUT;
    }
    else if (strcmp(pKey, "EE_CHAR_ITALIC") == 0)
    {
        nWhich = EE_CHAR_ITALIC;
    }
    else if (strcmp(pKey, "EE_CHAR_ITALIC_CJK") == 0)
    {
        nWhich = EE_CHAR_ITALIC_CJK;
    }
    else if (strcmp(pKey, "EE_CHAR_ITALIC_CTL") == 0)
    {
        nWhich = EE_CHAR_ITALIC_CTL;
    }
    else if (strcmp(pKey, "EE_CHAR_OUTLINE") == 0)
    {
        nWhich = EE_CHAR_OUTLINE;
    }
    else if (strcmp(pKey, "EE_CHAR_SHADOW") == 0)
    {
        nWhich = EE_CHAR_SHADOW;
    }
    else if (strcmp(pKey, "EE_CHAR_ESCAPEMENT") == 0)
    {
        nWhich = EE_CHAR_ESCAPEMENT;
    }
    else if (strcmp(pKey, "EE_CHAR_PAIRKERNING") == 0)
    {
        nWhich = EE_CHAR_PAIRKERNING;
    }
    else if (strcmp(pKey, "EE_CHAR_KERNING") == 0)
    {
        nWhich = EE_CHAR_KERNING;
    }
    else if (strcmp(pKey, "EE_CHAR_WLM") == 0)
    {
        nWhich = EE_CHAR_WLM;
    }
    else if (strcmp(pKey, "EE_CHAR_LANGUAGE") == 0)
    {
        nWhich = EE_CHAR_LANGUAGE;
    }
    else if (strcmp(pKey, "EE_CHAR_LANGUAGE_CJK") == 0)
    {
        nWhich = EE_CHAR_LANGUAGE_CJK;
    }
    else if (strcmp(pKey, "EE_CHAR_LANGUAGE_CTL") == 0)
    {
        nWhich = EE_CHAR_LANGUAGE_CTL;
    }
    else if (strcmp(pKey, "EE_CHAR_EMPHASISMARK") == 0)
    {
        nWhich = EE_CHAR_EMPHASISMARK;
    }
    else if (strcmp(pKey, "EE_CHAR_RELIEF") == 0)
    {
        nWhich = EE_CHAR_RELIEF;
    }
    else if (strcmp(pKey, "EE_CHAR_CASEMAP") == 0)
    {
        nWhich = EE_CHAR_CASEMAP;
    }
    else if (strcmp(pKey, "EE_PARA_WRITINGDIR") == 0)
    {
        nWhich = EE_PARA_WRITINGDIR;
    }
    else if (strcmp(pKey, "EE_PARA_HANGINGPUNCTUATION") == 0)
    {
        nWhich = EE_PARA_HANGINGPUNCTUATION;
    }
    else if (strcmp(pKey, "EE_PARA_FORBIDDENRULES") == 0)
    {
        nWhich = EE_PARA_FORBIDDENRULES;
    }
    else if (strcmp(pKey, "EE_PARA_ASIANCJKSPACING") == 0)
    {
        nWhich = EE_PARA_ASIANCJKSPACING;
    }
    else if (strcmp(pKey, "EE_PARA_HYPHENATE") == 0)
    {
        nWhich = EE_PARA_HYPHENATE;
    }
    else if (strcmp(pKey, "EE_PARA_HYPHENATE_NO_CAPS") == 0)
    {
        nWhich = EE_PARA_HYPHENATE_NO_CAPS;
    }
    else if (strcmp(pKey, "EE_PARA_HYPHENATE_NO_LAST_WORD") == 0)
    {
        nWhich = EE_PARA_HYPHENATE_NO_LAST_WORD;
    }
    else if (strcmp(pKey, "EE_PARA_BULLETSTATE") == 0)
    {
        nWhich = EE_PARA_BULLETSTATE;
    }
    else if (strcmp(pKey, "EE_PARA_OUTLLEVEL") == 0)
    {
        nWhich = EE_PARA_OUTLLEVEL;
    }
    else if (strcmp(pKey, "EE_PARA_LRSPACE") == 0)
    {
        nWhich = EE_PARA_LRSPACE;
    }
    else if (strcmp(pKey, "EE_PARA_ULSPACE") == 0)
    {
        nWhich = EE_PARA_ULSPACE;
    }
    else if (strcmp(pKey, "EE_PARA_SBL") == 0)
    {
        nWhich = EE_PARA_SBL;
    }
    else if (strcmp(pKey, "EE_PARA_JUST") == 0)
    {
        nWhich = EE_PARA_JUST;
    }
    else if (strcmp(pKey, "EE_PARA_TABS") == 0)
    {
        nWhich = EE_PARA_TABS;
    }
    else if (strcmp(pKey, "EE_PARA_JUST_METHOD") == 0)
    {
        nWhich = EE_PARA_JUST_METHOD;
    }
    else if (strcmp(pKey, "EE_PARA_VER_JUST") == 0)
    {
        nWhich = EE_PARA_VER_JUST;
    }
    else if (pKey[0] == 'E' && pKey[1] == 'E' && pKey[2] == '_')
    {
        abort();
    }
    else
    {
        return;
    }

    if (rValue.tag == Y_JSON_NULL)
    {
        assert(pRemoved);
        if (pRemoved)
        {
            pRemoved->emplace_back(nWhich);
        }
        return;
    }
    else switch (nWhich)
    {
        case EE_CHAR_COLOR:
        case EE_CHAR_BKGCOLOR:
        {
            Color const c(ColorTransparency, YReadInt(rValue));
            SvxColorItem const item(c, nWhich);
            rSet.Put(item);
            break;
        }
        case EE_CHAR_FONTINFO:
        case EE_CHAR_FONTINFO_CJK:
        case EE_CHAR_FONTINFO_CTL:
        {
            yvalidate(rValue.tag == Y_JSON_MAP);
            ::std::optional<OUString> oFamilyName;
            ::std::optional<OUString> oStyle;
            ::std::optional<FontFamily> oFamily;
            ::std::optional<FontPitch> oPitch;
            ::std::optional<rtl_TextEncoding> oCharset;
            for (decltype(rValue.len) i = 0; i < rValue.len; ++i)
            {
                const char*const pEntry{rValue.value.map[i].key};
                if (strcmp(pEntry, "familyname") == 0)
                {
                    yvalidate(rValue.value.map[i].value->tag == Y_JSON_STR);
                    oFamilyName.emplace(OStringToOUString(rValue.value.map[i].value->value.str, RTL_TEXTENCODING_UTF8));
                }
                else if (strcmp(pEntry, "style") == 0)
                {
                    yvalidate(rValue.value.map[i].value->tag == Y_JSON_STR);
                    oStyle.emplace(OStringToOUString(rValue.value.map[i].value->value.str, RTL_TEXTENCODING_UTF8));
                }
                else if (strcmp(pEntry, "family") == 0)
                {
                    auto const value{YReadInt(*rValue.value.map[i].value)};
                    yvalidate(FAMILY_DONTKNOW <= value && value <= FAMILY_SYSTEM);
                    oFamily.emplace(FontFamily(value));
                }
                else if (strcmp(pEntry, "pitch") == 0)
                {
                    auto const value{YReadInt(*rValue.value.map[i].value)};
                    yvalidate(PITCH_DONTKNOW <= value && value <= PITCH_VARIABLE);
                    oPitch.emplace(FontPitch(value));
                }
                else if (strcmp(pEntry, "charset") == 0)
                {
                    auto const value{YReadInt(*rValue.value.map[i].value)};
                    yvalidate(/*RTL_TEXTENCODING_DONTKNOW <= value &&*/ value <= RTL_TEXTENCODING_UNICODE);
                    oCharset.emplace(rtl_TextEncoding(value));
                }
                else yvalidate(false);
            }
            if (oFamilyName && oStyle && oFamily && oPitch && oCharset)
            {
                SvxFontItem const item{
                    *oFamily, *oFamilyName, *oStyle, *oPitch, *oCharset, nWhich};
                rSet.Put(item);
            }
            else yvalidate(false);
            break;
        }
        case EE_CHAR_FONTHEIGHT:
        case EE_CHAR_FONTHEIGHT_CJK:
        case EE_CHAR_FONTHEIGHT_CTL:
        {
            yvalidate(rValue.tag == Y_JSON_MAP);
            ::std::optional<sal_uInt32> oHeight;
            ::std::optional<sal_uInt16> oProp;
            ::std::optional<MapUnit> oMapUnit;
            for (decltype(rValue.len) i = 0; i < rValue.len; ++i)
            {
                const char*const pEntry{rValue.value.map[i].key};
                if (strcmp(pEntry, "height") == 0)
                {
                    auto const value{YReadInt(*rValue.value.map[i].value)};
                    yvalidate(/*0 <= value && */value <= SAL_MAX_INT32);
                    oHeight.emplace(value);
                }
                else if (strcmp(pEntry, "prop") == 0)
                {
                    auto const value{YReadInt(*rValue.value.map[i].value)};
                    yvalidate(/*0 <= value && */value <= SAL_MAX_INT16);
                    oProp.emplace(value);
                }
                else if (strcmp(pEntry, "propunit") == 0)
                {
                    auto const value{YReadInt(*rValue.value.map[i].value)};
                    yvalidate(/*0 <= value && */value <= ::std::underlying_type_t<MapUnit>(MapUnit::LAST));
                    oMapUnit.emplace(MapUnit(value));
                }
                else yvalidate(false);
            }
            if (oHeight && oProp && oMapUnit)
            {
                SvxFontHeightItem item{*oHeight, 100, nWhich};
                item.SetProp(*oProp, *oMapUnit);
                rSet.Put(item);
            }
            else yvalidate(false);
            break;
        }
        case EE_CHAR_FONTWIDTH:
        {
            auto const value{YReadInt(rValue)};
            yvalidate(/*0 <= value && */value <= SAL_MAX_INT16);
            SvxCharScaleWidthItem const item{sal_uInt16(value), TypedWhichId<SvxCharScaleWidthItem>(nWhich)};
            rSet.Put(item);
            break;
        }
        case EE_CHAR_WEIGHT:
        case EE_CHAR_WEIGHT_CJK:
        case EE_CHAR_WEIGHT_CTL:
        {
            auto const value{YReadInt(rValue)};
            yvalidate(WEIGHT_DONTKNOW <= value && value <= WEIGHT_BLACK);
            SvxWeightItem const item{FontWeight(value), nWhich};
            rSet.Put(item);
            break;
        }
        case EE_CHAR_UNDERLINE:
        case EE_CHAR_OVERLINE:
        {
            yvalidate(rValue.tag == Y_JSON_MAP);
            ::std::optional<FontLineStyle> oStyle;
            ::std::optional<Color> oColor;
            for (decltype(rValue.len) i = 0; i < rValue.len; ++i)
            {
                const char*const pEntry{rValue.value.map[i].key};
                if (strcmp(pEntry, "style") == 0)
                {
                    auto const value{YReadInt(*rValue.value.map[i].value)};
                    yvalidate(LINESTYLE_NONE <= value && value <= LINESTYLE_BOLDWAVE);
                    oStyle.emplace(FontLineStyle(value));
                }
                else if (strcmp(pEntry, "color") == 0)
                {
                    oColor.emplace(ColorTransparency, YReadInt(*rValue.value.map[i].value));
                }
                else yvalidate(false);
            }
            if (oStyle && oColor)
            {
                if (nWhich == EE_CHAR_UNDERLINE)
                {
                    SvxUnderlineItem item{*oStyle, EE_CHAR_UNDERLINE};
                    item.SetColor(*oColor);
                    rSet.Put(item);
                }
                else
                {
                    SvxOverlineItem item{*oStyle, EE_CHAR_OVERLINE};
                    item.SetColor(*oColor);
                    rSet.Put(item);
                }
            }
            else yvalidate(false);
            break;
        }
        case EE_CHAR_STRIKEOUT:
        {
            auto const value{YReadInt(rValue)};
            yvalidate(STRIKEOUT_NONE <= value && value <= STRIKEOUT_X);
            SvxCrossedOutItem const item{FontStrikeout(value), nWhich};
            rSet.Put(item);
            break;
        }
        case EE_CHAR_ITALIC:
        case EE_CHAR_ITALIC_CJK:
        case EE_CHAR_ITALIC_CTL:
        {
            auto const value{YReadInt(rValue)};
            yvalidate(ITALIC_NONE <= value && value <= ITALIC_DONTKNOW);
            SvxPostureItem const item{FontItalic(value), nWhich};
            rSet.Put(item);
            break;
        }
        case EE_CHAR_OUTLINE:
        {
            yvalidate(rValue.tag == Y_JSON_BOOL);
            SvxContourItem const item{rValue.value.flag == Y_TRUE, nWhich};
            rSet.Put(item);
            break;
        }
        case EE_CHAR_SHADOW:
        {
            yvalidate(rValue.tag == Y_JSON_BOOL);
            SvxShadowedItem const item{rValue.value.flag == Y_TRUE, nWhich};
            rSet.Put(item);
            break;
        }
        case EE_CHAR_ESCAPEMENT:
        {
            yvalidate(rValue.tag == Y_JSON_MAP);
            ::std::optional<short> oEsc;
            ::std::optional<sal_uInt8> oProp;
            for (decltype(rValue.len) i = 0; i < rValue.len; ++i)
            {
                const char*const pEntry{rValue.value.map[i].key};
                if (strcmp(pEntry, "esc") == 0)
                {
                    int64_t const value{static_cast<int64_t>(YReadInt(*rValue.value.map[i].value))};
                    yvalidate(SAL_MIN_INT16 <= value && value <= SAL_MAX_INT16);
                    oEsc.emplace(short(value));
                }
                else if (strcmp(pEntry, "prop") == 0)
                {
                    auto const value{YReadInt(*rValue.value.map[i].value)};
                    yvalidate(/*0 <= value && */value <= SAL_MAX_UINT8);
                    oProp.emplace(sal_uInt8(value));
                }
                else yvalidate(false);
            }
            if (oEsc && oProp)
            {
                SvxEscapementItem const item{*oEsc, *oProp, EE_CHAR_ESCAPEMENT};
                rSet.Put(item);
            }
            else yvalidate(false);
            break;
        }
        case EE_CHAR_PAIRKERNING:
        {
            yvalidate(rValue.tag == Y_JSON_BOOL);
            SvxAutoKernItem const item{rValue.value.flag == Y_TRUE, nWhich};
            rSet.Put(item);
            break;
        }
        case EE_CHAR_KERNING:
        {
            int64_t const value{static_cast<int64_t>(YReadInt(rValue))};
            yvalidate(SAL_MIN_INT16 <= value && value <= SAL_MAX_INT16);
            SvxKerningItem const item{short(value), nWhich};
            rSet.Put(item);
            break;
        }
        case EE_CHAR_WLM:
        {
            yvalidate(rValue.tag == Y_JSON_BOOL);
            SvxWordLineModeItem const item{rValue.value.flag == Y_TRUE, nWhich};
            rSet.Put(item);
            break;
        }
        case EE_CHAR_LANGUAGE:
        case EE_CHAR_LANGUAGE_CJK:
        case EE_CHAR_LANGUAGE_CTL:
        {
            auto const value{YReadInt(rValue)};
            yvalidate(/*0 <= value && */value <= SAL_MAX_UINT16);
            SvxLanguageItem const item{LanguageType(value), nWhich};
            rSet.Put(item);
            break;
        }
        case EE_CHAR_EMPHASISMARK:
        {
            auto const value{YReadInt(rValue)};
            yvalidate((value & 0x300f) == value);
            SvxEmphasisMarkItem const item{FontEmphasisMark(value), TypedWhichId<SvxEmphasisMarkItem>(nWhich)};
            rSet.Put(item);
            break;
        }
        case EE_CHAR_RELIEF:
        {
            auto const value{YReadInt(rValue)};
            yvalidate(::std::underlying_type_t<FontRelief>(FontRelief::NONE) <= value && value <= ::std::underlying_type_t<FontRelief>(FontRelief::Engraved));
            SvxCharReliefItem const item{FontRelief(value), nWhich};
            rSet.Put(item);
            break;
        }
        case EE_CHAR_CASEMAP:
        {
            auto const value{YReadInt(rValue)};
            yvalidate(::std::underlying_type_t<SvxCaseMap>(SvxCaseMap::NotMapped) <= value && value < ::std::underlying_type_t<SvxCaseMap>(SvxCaseMap::End));
            SvxCaseMapItem const item{SvxCaseMap(value), nWhich};
            rSet.Put(item);
            break;
        }
        case EE_PARA_WRITINGDIR:
        {
            auto const value{YReadInt(rValue)};
            yvalidate(::std::underlying_type_t<SvxFrameDirection>(SvxFrameDirection::Horizontal_LR_TB) <= value && value < ::std::underlying_type_t<SvxFrameDirection>(SvxFrameDirection::Stacked));
            SvxFrameDirectionItem const item{SvxFrameDirection(value), nWhich};
            rSet.Put(item);
            break;
        }
        case EE_PARA_HANGINGPUNCTUATION:
        {
            yvalidate(rValue.tag == Y_JSON_BOOL);
            SvxHangingPunctuationItem const item{rValue.value.flag == Y_TRUE, nWhich};
            rSet.Put(item);
            break;
        }
        case EE_PARA_FORBIDDENRULES:
        {
            yvalidate(rValue.tag == Y_JSON_BOOL);
            SvxForbiddenRuleItem const item{rValue.value.flag == Y_TRUE, nWhich};
            rSet.Put(item);
            break;
        }
        case EE_PARA_ASIANCJKSPACING:
        {
            yvalidate(rValue.tag == Y_JSON_BOOL);
            SvxScriptSpaceItem const item{rValue.value.flag == Y_TRUE, nWhich};
            rSet.Put(item);
            break;
        }
        case EE_PARA_HYPHENATE:
        case EE_PARA_HYPHENATE_NO_CAPS:
        case EE_PARA_HYPHENATE_NO_LAST_WORD:
        case EE_PARA_BULLETSTATE:
        {
            yvalidate(rValue.tag == Y_JSON_BOOL);
            SfxBoolItem const item{nWhich, rValue.value.flag == Y_TRUE};
            rSet.Put(item);
            break;
        }
        case EE_PARA_OUTLLEVEL:
        {
            int64_t const value{static_cast<int64_t>(YReadInt(rValue))};
            yvalidate(-1 <= value && value < SVX_MAX_NUM);
            SfxInt16Item const item{nWhich, sal_Int16(value)};
            rSet.Put(item);
            break;
        }
        case EE_PARA_LRSPACE:
        {
            yvalidate(rValue.tag == Y_JSON_MAP);
            ::std::optional<double> oFirstLineIndent;
            ::std::optional<sal_Int16> oFirstLineIndentUnit;
            ::std::optional<double> oLeft;
            ::std::optional<sal_Int16> oLeftUnit;
            ::std::optional<double> oRight;
            ::std::optional<sal_Int16> oRightUnit;
            ::std::optional<bool> oAutoFirst;
            for (decltype(rValue.len) i = 0; i < rValue.len; ++i)
            {
                const char*const pEntry{rValue.value.map[i].key};
                if (strcmp(pEntry, "first-line-offset") == 0)
                {
                    yvalidate(rValue.value.map[i].value->tag == Y_JSON_NUM);
                    oFirstLineIndent.emplace(rValue.value.map[i].value->value.num);
                }
                else if (strcmp(pEntry, "first-line-offset-unit") == 0)
                {
                    auto const value{YReadInt(*rValue.value.map[i].value)};
                    yvalidate(css::util::MeasureUnit::MM_100TH <= value && value <= css::util::MeasureUnit::FONT_CJK_ADVANCE);
                    oFirstLineIndentUnit.emplace(sal_Int16(value));
                }
                else if (strcmp(pEntry, "left-margin") == 0)
                {
                    yvalidate(rValue.value.map[i].value->tag == Y_JSON_NUM);
                    oLeft.emplace(rValue.value.map[i].value->value.num);
                }
                else if (strcmp(pEntry, "left-margin-unit") == 0)
                {
                    auto const value{YReadInt(*rValue.value.map[i].value)};
                    yvalidate(css::util::MeasureUnit::MM_100TH <= value && value <= css::util::MeasureUnit::FONT_CJK_ADVANCE);
                    oLeftUnit.emplace(sal_Int16(value));
                }
                else if (strcmp(pEntry, "right-margin") == 0)
                {
                    yvalidate(rValue.value.map[i].value->tag == Y_JSON_NUM);
                    oRight.emplace(rValue.value.map[i].value->value.num);
                }
                else if (strcmp(pEntry, "right-margin-unit") == 0)
                {
                    auto const value{YReadInt(*rValue.value.map[i].value)};
                    yvalidate(css::util::MeasureUnit::MM_100TH <= value && value <= css::util::MeasureUnit::FONT_CJK_ADVANCE);
                    oRightUnit.emplace(sal_Int16(value));
                }
                else if (strcmp(pEntry, "auto-first") == 0)
                {
                    yvalidate(rValue.value.map[i].value->tag == Y_JSON_BOOL);
                    oAutoFirst.emplace(rValue.value.map[i].value->value.flag == Y_TRUE);
                }
                else yvalidate(false);
            }
            if (oFirstLineIndent && oFirstLineIndentUnit && oAutoFirst.has_value()
                && oLeft && oLeftUnit && oRight && oRightUnit)
            {
                SvxLRSpaceItem item{{*oLeft, *oLeftUnit}, {*oRight, *oRightUnit}, {*oFirstLineIndent, *oFirstLineIndentUnit}, EE_PARA_LRSPACE};
                item.SetAutoFirst(*oAutoFirst);
                rSet.Put(item);
            }
            else yvalidate(false);
            break;
        }
        case EE_PARA_ULSPACE:
        {
            yvalidate(rValue.tag == Y_JSON_MAP);
            ::std::optional<sal_uInt16> oUpper;
            ::std::optional<sal_uInt16> oLower;
            for (decltype(rValue.len) i = 0; i < rValue.len; ++i)
            {
                const char*const pEntry{rValue.value.map[i].key};
                if (strcmp(pEntry, "upper-margin") == 0)
                {
                    auto const value{YReadInt(*rValue.value.map[i].value)};
                    yvalidate(/*0 <= value && */value <= SAL_MAX_UINT16);
                    oUpper.emplace(sal_uInt16(value));
                }
                else if (strcmp(pEntry, "lower-margin") == 0)
                {
                    auto const value{YReadInt(*rValue.value.map[i].value)};
                    yvalidate(/*0 <= value && */value <= SAL_MAX_UINT16);
                    oLower.emplace(sal_uInt16(value));
                }
                else yvalidate(false);
            }
            if (oUpper && oLower)
            {
                SvxULSpaceItem const item{*oUpper, *oLower, EE_PARA_ULSPACE};
                rSet.Put(item);
            }
            else yvalidate(false);
            break;
        }
        case EE_PARA_SBL:
        {
            yvalidate(rValue.tag == Y_JSON_MAP);
            ::std::optional<sal_uInt16> oLineSpaceFix;
            ::std::optional<sal_uInt16> oLineSpaceMin;
            ::std::optional<sal_uInt16> oInterLineSpaceProp;
            ::std::optional<sal_Int16> oInterLineSpaceFix;
            for (decltype(rValue.len) i = 0; i < rValue.len; ++i)
            {
                const char*const pEntry{rValue.value.map[i].key};
                if (strcmp(pEntry, "line-space-fix") == 0)
                {
                    auto const value{YReadInt(*rValue.value.map[i].value)};
                    yvalidate(/*0 <= value && */value <= SAL_MAX_UINT16);
                    oLineSpaceFix.emplace(sal_uInt16(value));
                }
                else if (strcmp(pEntry, "line-space-min") == 0)
                {
                    auto const value{YReadInt(*rValue.value.map[i].value)};
                    yvalidate(/*0 <= value && */value <= SAL_MAX_UINT16);
                    oLineSpaceMin.emplace(sal_uInt16(value));
                }
                else if (strcmp(pEntry, "inter-line-space-prop") == 0)
                {
                    auto const value{YReadInt(*rValue.value.map[i].value)};
                    yvalidate(/*0 <= value && */value <= SAL_MAX_UINT16);
                    oInterLineSpaceProp.emplace(sal_uInt16(value));
                }
                else if (strcmp(pEntry, "inter-line-space-fix") == 0)
                {
                    auto const value{YReadInt(*rValue.value.map[i].value)};
                    yvalidate(/*0 <= value && */value <= SAL_MAX_UINT16);
                    oInterLineSpaceFix.emplace(sal_Int16(value));
                }
                else yvalidate(false);
            }
            SvxLineSpacingItem item{0, EE_PARA_SBL};
            if (oLineSpaceFix)
            {
                item.SetLineHeight(*oLineSpaceFix);
                item.SetLineSpaceRule(SvxLineSpaceRule::Fix);
                rSet.Put(item);
            }
            else if (oLineSpaceMin)
            {
                item.SetLineHeight(*oLineSpaceMin);
                rSet.Put(item);
            }
            else if (oInterLineSpaceProp)
            {
                item.SetPropLineSpace(*oInterLineSpaceProp);
                rSet.Put(item);
            }
            else if (oInterLineSpaceFix)
            {
                item.SetInterLineSpace(*oInterLineSpaceFix);
                rSet.Put(item);
            }
            else yvalidate(false);
            break;
        }
        case EE_PARA_JUST:
        {
            auto const value{YReadInt(rValue)};
            SvxAdjustItem item{SvxAdjust(), nWhich};
            switch (value)
            {
                case uint64_t(SvxAdjust::Left):
                case uint64_t(SvxAdjust::Right):
                case uint64_t(SvxAdjust::Center):
                    item.SetAdjust(SvxAdjust(value));
                    break;
                case uint64_t(SvxAdjust::Block):
                    item.SetAdjust(SvxAdjust(value));
                    item.SetLastBlock(SvxAdjust::Left);
                    break;
                case 5:
                    item.SetAdjust(SvxAdjust::Block);
                    item.SetLastBlock(SvxAdjust::Center);
                    break;
                case 6:
                    item.SetAdjust(SvxAdjust::Block);
                    item.SetLastBlock(SvxAdjust::Block);
                    item.SetOneWord(SvxAdjust::Left);
                    break;
                case 7:
                    item.SetAdjust(SvxAdjust::Block);
                    item.SetLastBlock(SvxAdjust::Block);
                    item.SetOneWord(SvxAdjust::Block);
                    break;
                default:
                    abort();
            }
            rSet.Put(item);
            break;
        }
        case EE_PARA_TABS:
        {
            yvalidate(rValue.tag == Y_JSON_MAP);
            ::std::optional<sal_Int32> oDefault;
            ::std::optional<::std::vector<SvxTabStop>> oTabs;
            for (decltype(rValue.len) i = 0; i < rValue.len; ++i)
            {
                const char*const pEntry{rValue.value.map[i].key};
                if (strcmp(pEntry, "default-distance") == 0)
                {
                    auto const value{YReadInt(*rValue.value.map[i].value)};
                    yvalidate(/*0 <= value && */value <= SAL_MAX_INT32);
                    oDefault.emplace(sal_Int32(value));
                }
                else if (strcmp(pEntry, "tab-stops") == 0)
                {
                    oTabs.emplace();
                    yvalidate(rValue.value.map[i].value->tag == Y_JSON_ARR);
                    YOutput const& rArray{*rValue.value.map[i].value};
                    for (decltype(rArray.len) j = 0; j < rArray.len; ++j)
                    {
                        YOutput const& rMap{rArray.value.array[j]};
                        yvalidate(rMap.tag == Y_JSON_MAP);
                        ::std::optional<sal_Int32> oPos;
                        ::std::optional<SvxTabAdjust> oAdjust;
                        ::std::optional<sal_Unicode> oDecimal;
                        ::std::optional<sal_Unicode> oFill;
                        for (decltype(rMap.len) k = 0; k < rMap.len; ++k)
                        {
                            const char*const pE{rMap.value.map[k].key};
                            if (strcmp(pE, "pos") == 0)
                            {
                                auto const value{YReadInt(*rMap.value.map[k].value)};
                                yvalidate(/*0 <= value && */value <= SAL_MAX_INT32);
                                oPos.emplace(sal_Int32(value));
                            }
                            else if (strcmp(pE, "adjustment") == 0)
                            {
                                auto const value{YReadInt(*rMap.value.map[k].value)};
                                yvalidate(::std::underlying_type_t<SvxTabAdjust>(SvxTabAdjust::Left) <= value && value < ::std::underlying_type_t<SvxTabAdjust>(SvxTabAdjust::End));
                                oAdjust.emplace(SvxTabAdjust(value));
                            }
                            else if (strcmp(pE, "decimal") == 0)
                            {
                                auto const value{YReadInt(*rMap.value.map[k].value)};
                                yvalidate(/*0 <= value && */value < SAL_MAX_UINT16);
                                oDecimal.emplace(sal_Unicode(value));
                            }
                            else if (strcmp(pE, "fill") == 0)
                            {
                                auto const value{YReadInt(*rMap.value.map[k].value)};
                                yvalidate(/*0 <= value && */value < SAL_MAX_UINT16);
                                oFill.emplace(sal_Unicode(value));
                            }
                            else yvalidate(false);
                        }
                        if (oPos && oAdjust && oDecimal && oFill)
                        {
                            oTabs->emplace_back(*oPos, *oAdjust, *oDecimal, *oFill);
                        }
                        else yvalidate(false);
                    }
                }
            }
            if (oDefault && oTabs)
            {
                SvxTabStopItem item{nWhich};
                item.SetDefaultDistance(*oDefault);
                for (SvxTabStop const& it : *oTabs)
                {
                    item.Insert(it);
                }
                rSet.Put(item);
            }
            else yvalidate(false);
            break;
        }
        case EE_PARA_JUST_METHOD:
        {
            auto const value{YReadInt(rValue)};
            yvalidate(::std::underlying_type_t<SvxCellJustifyMethod>(SvxCellJustifyMethod::Auto) <= value && value <= ::std::underlying_type_t<SvxCellJustifyMethod>(SvxCellJustifyMethod::Distribute));
            SvxJustifyMethodItem const item{SvxCellJustifyMethod(value), nWhich};
            rSet.Put(item);
            break;
        }
        case EE_PARA_VER_JUST:
        {
            auto const value{YReadInt(rValue)};
            yvalidate(::std::underlying_type_t<SvxCellVerJustify>(SvxCellVerJustify::Standard) <= value && value <= ::std::underlying_type_t<SvxCellVerJustify>(SvxCellVerJustify::Block));
            SvxVerJustifyItem const item{SvxCellVerJustify(value), nWhich};
            rSet.Put(item);
            break;
        }

        default:
            assert(false);
    }
}

// TODO: this could be a lot simpler if feature were a nested json-map like attr
template<typename T> void
YImplInsertFeature(ImpEditEngine & rIEE,
    EditPaM const& rPam,
    T const*const elements, uint32_t const len,
    ::std::function<::std::pair<char const*, YOutput const&>(T const&)> pGetAttr)
{
    ::std::optional<OUString> oFeature;
    ::std::optional<SvxURLFormat> oFormat;
    ::std::optional<OUString> oURL;
    ::std::optional<OUString> oRepresentation;
    ::std::optional<OUString> oTargetFrame;
    //for (decltype(pChunks[i].fmt_len) k = 0; k < pChunks[i].fmt_len; ++k)
    for (::std::remove_const_t<decltype(len)> k = 0; k < len; ++k)
    {
//                    YMapEntry const& rAttr{pChunks[i].fmt[k]};
        T const& rAttr{elements[k]};
        auto [pKey, rValue]{pGetAttr(rAttr)};
        if (strcmp(pKey, "feature") == 0)
        {
            yvalidate(rValue.tag == Y_JSON_STR);
            oFeature.emplace(OStringToOUString(rValue.value.str, RTL_TEXTENCODING_UTF8));
        }
        else if (strcmp(pKey, "url-format") == 0)
        {
            // ??? why is this Y_JSON_NUM here?
//                        assert(rValue.tag == Y_JSON_INT || rValue.tag == Y_JSON_NUM);
            uint64_t const value{YReadInt(rValue)};
            switch (value)
            {
                case ::std::underlying_type_t<SvxURLFormat>(SvxURLFormat::AppDefault):
                case ::std::underlying_type_t<SvxURLFormat>(SvxURLFormat::Url):
                case ::std::underlying_type_t<SvxURLFormat>(SvxURLFormat::Repr):
                    oFormat.emplace(SvxURLFormat(value));
                    break;
                default:
                    yvalidate(false);
            }
        }
        else if (strcmp(pKey, "url-url") == 0)
        {
            yvalidate(rValue.tag == Y_JSON_STR);
            oURL.emplace(OStringToOUString(rValue.value.str, RTL_TEXTENCODING_UTF8));
        }
        else if (strcmp(pKey, "url-representation") == 0)
        {
            yvalidate(rValue.tag == Y_JSON_STR);
            oRepresentation.emplace(OStringToOUString(rValue.value.str, RTL_TEXTENCODING_UTF8));
        }
        else if (strcmp(pKey, "url-targetframe") == 0)
        {
            yvalidate(rValue.tag == Y_JSON_STR);
            oTargetFrame.emplace(OStringToOUString(rValue.value.str, RTL_TEXTENCODING_UTF8));
        }
    }
    if (oFeature && *oFeature == "tab")
    {
        EditSelection const sel{rPam};
        rIEE.InsertTab(sel);
    }
    else if (oFeature && *oFeature == "line")
    {
        EditSelection const sel{rPam};
        rIEE.InsertLineBreak(sel);
    }
    else if (oFeature && *oFeature == "url")
    {
        if (oFormat && oURL && oRepresentation)
        {
            SvxURLField field{*oURL, *oRepresentation, *oFormat};
            if (oTargetFrame)
            {
                field.SetTargetFrame(*oTargetFrame);
            }
            SvxFieldItem const item{field, EE_FEATURE_FIELD};
            EditSelection const sel{rPam};
            rIEE.InsertField(sel, item);
        }
        else yvalidate(false);
    }
    else
    {
        yvalidate(false);
    }
}

} // namespace

void EditDoc::YrsSetStyle(sal_Int32 const index, ::std::u16string_view const rStyle)
{
    YSetStyle(m_pYrsSupplier, m_CommentId, *this, index, rStyle);
}

void EditDoc::YrsSetParaAttr(sal_Int32 const index, SfxPoolItem const& rItem)
{
    YSetParaAttr(m_pYrsSupplier, m_CommentId, *this, index, rItem);
}

void EditDoc::YrsWriteEEState()
{
    assert(m_pYrsSupplier);

    YWrite const yw{GetYWrite(m_pYrsSupplier, m_CommentId)};
    assert(yw.pTxn && yw.pProps && yw.pText);

    YInput const vertical{yinput_bool(mbIsVertical ? Y_TRUE : Y_FALSE)};
    ymap_insert(yw.pProps, yw.pTxn, "is-vertical", &vertical);
    YInput const rotation{yinput_long(static_cast<int64_t>(mnRotation))};
    ymap_insert(yw.pProps, yw.pTxn, "rotation", &rotation);
    YInput const deftab{yinput_long(static_cast<int64_t>(mnDefTab))};
    ymap_insert(yw.pProps, yw.pTxn, "def-tab", &deftab);

    for (::std::unique_ptr<ContentNode> const& rpNode : maContents)
    {
        auto const start{ytext_len(yw.pText, yw.pTxn)};
        sal_Int32 pos{0};
        for (sal_Int32 i = rpNode->GetString().indexOf(CH_FEATURE, pos);
              i != -1; i = rpNode->GetString().indexOf(CH_FEATURE, pos))
        {
            if (i != pos)
            {
                OString const content{OUStringToOString(rpNode->GetString().subView(pos, i - pos), RTL_TEXTENCODING_UTF8)};
                ytext_insert(yw.pText, yw.pTxn, start + pos, content.getStr(), nullptr);
            }
            EditCharAttrib const*const pAttrib{rpNode->GetCharAttribs().FindFeature(i)};
            assert(pAttrib);
            // this will insert the CH_FEATURE too
            YInsertFeature(m_pYrsSupplier, m_CommentId, *this, GetPos(rpNode.get()), pAttrib);
            pos = i+1;
        }
        if (pos != rpNode->GetString().getLength())
        {
            OString const content{OUStringToOString(rpNode->GetString().subView(pos, rpNode->GetString().getLength() - pos), RTL_TEXTENCODING_UTF8)};
            ytext_insert(yw.pText, yw.pTxn, start + pos, content.getStr(), nullptr);
        }
        for (::std::unique_ptr<EditCharAttrib> const& rpAttr : rpNode->GetCharAttribs().GetAttribs())
        {
            if (!rpAttr->IsFeature())
            {
                YInsertAttribImpl(yw, start, rpAttr.get());
            }
        }
        char const para[]{ CH_PARA, '\0' };
        ContentAttribs const& rParaAttribs{rpNode->GetContentAttribs()};
        OString const styleName{OUStringToOString(rpNode->GetStyleSheet()->GetName(), RTL_TEXTENCODING_UTF8)};
        YInput const style{yinput_string(styleName.getStr())};
        YInput attrArray[]{ style };
        char const*const attrNames[]{ "para-style" };
        YInput const attrs{yinput_json_map(const_cast<char**>(attrNames), attrArray, 1)};
        auto const end{ytext_len(yw.pText, yw.pTxn)};
        ytext_insert(yw.pText, yw.pTxn, end, para, &attrs);
        for (SfxItemIter it{rParaAttribs.GetItems()}; !it.IsAtEnd(); it.NextItem())
        {
            YInsertAttribImplImpl(yw, *it.GetCurItem(), end, 1);
        }
    }
}

void EditDoc::YrsReadEEState(YTransaction *const pTxn, ImpEditEngine & rIEE)
{
    assert(m_pYrsSupplier);
    YWrite const yw{GetYWrite(m_pYrsSupplier, m_CommentId, pTxn)};
    assert(yw.pTxn && yw.pProps && yw.pText);

    ::std::unique_ptr<YOutput, YOutputDeleter> const pVertical{ymap_get(yw.pProps, yw.pTxn, "is-vertical")};
    yvalidate(!pVertical || pVertical->tag == Y_JSON_BOOL);
    if (pVertical && pVertical->tag == Y_JSON_BOOL)
    {
        //mbIsVertical = pVertical->value.flag == Y_TRUE;
        SetVertical(pVertical->value.flag == Y_TRUE);
    }
    ::std::unique_ptr<YOutput, YOutputDeleter> const pRotation{ymap_get(yw.pProps, yw.pTxn, "rotation")};
    yvalidate(pRotation);
    {
        auto const value{YReadInt(*pRotation)};
        switch (value)
        {
            case int64_t(TextRotation::NONE):
            case int64_t(TextRotation::TOPTOBOTTOM):
            case int64_t(TextRotation::BOTTOMTOTOP):
                SetRotation(TextRotation(value));
                break;
            default:
                yvalidate(false);
        }
    }
    ::std::unique_ptr<YOutput, YOutputDeleter> const pDefTab{ymap_get(yw.pProps, yw.pTxn, "def-tab")};
    yvalidate(pDefTab);
    {
        auto const value{YReadInt(*pDefTab)};
        yvalidate(/*0 <= value && */value <= SAL_MAX_INT16);
        SetDefTab(value);
    }

    assert(maContents.size() == 1);
#if 0
    // there is a getImpl().GetEditSelection() too, who knows what else... try to reuse existing node?
    rIEE.GetParaPortions().Reset();
    maContents.clear();
    // the last paragraph cannot be removed, as cursors and a11y need it
    rIEE.RemoveParagraph(0); // remove pre-existing one from InitDoc()
#endif
    uint32_t chunks{0};
    //struct YChunksDeleter { uint32_t len; void operator()(YChunk *const p) const { ychunks_destroy(p, len); } };
    YChunk *const pChunks{ytext_chunks(yw.pText, yw.pTxn, &chunks)};
    sal_Int32 nodes{0};
    ContentNode * pNode{nullptr};
    for (decltype(chunks) i = 0; i < chunks; ++i)
    {
        decltype(nodes) const nodeStart{nodes};
        sal_Int32 const posStart{pNode ? pNode->Len() : 0};

        yvalidate(pChunks[i].data.tag == Y_JSON_STR);
        OString const str(pChunks[i].data.value.str);
        sal_Int32 strStart{0};

        for (sal_Int32 j = 0; j < str.getLength(); ++j)
        {
            if (!pNode)
            {
                //pNode = new ContentNode(GetItemPool());
                //rIEE.InsertContent(pNode, nodes); // does not set DefFont?
                pNode = rIEE.ImpFastInsertParagraph(nodes).GetNode();
//                rIEE.GetParaPortions().Insert(nodes, ::std::make_unique<ParaPortion>(pNode));
//does not call ParagraphInserted so no a11y event                Insert(nodes, ::std::unique_ptr<ContentNode>(pNode));
//                rIEE.UpdateSelections(); // prevent later rIEEE.RemoveParagraph from crashing? doesnothelp, only does deletes
            }
            if (str[j] == CH_PARA)
            {
                if (j != 0)
                {
                    OString const portion{str.copy(strStart, j - strStart)};
                    pNode->Append(OStringToOUString(portion, RTL_TEXTENCODING_UTF8));
                }
                for (decltype(pChunks[i].fmt_len) k = 0; k < pChunks[i].fmt_len; ++k)
                {
                    if (strcmp(pChunks[i].fmt[k].key, "para-style") == 0)
                    {
                        yvalidate(pChunks[i].fmt[k].value->tag == Y_JSON_STR);
                        OUString const style{OStringToOUString(pChunks[i].fmt[k].value->value.str, RTL_TEXTENCODING_UTF8)};
                        SfxStyleSheet *const pStyle{dynamic_cast<SfxStyleSheet *>(
                            rIEE.GetStyleSheetPool()->Find(style, SfxStyleFamily::Para))};
                        if (!pStyle) { abort(); }
                        pNode->SetStyleSheet(pStyle);
                    }
                }
                pNode = nullptr;
                ++nodes;
                strStart = j+1;
            }
            else if (str[j] == CH_FEATURE)
            {
                if (j != strStart)
                {
                    OString const portion{str.copy(strStart, j - strStart)};
                    pNode->Append(OStringToOUString(portion, RTL_TEXTENCODING_UTF8));
                }
                auto GetAttr = [](YMapEntry const& rEntry) -> ::std::pair<char const*, YOutput const&>
                    { return {rEntry.key, *rEntry.value}; };
                EditPaM const pam{pNode, pNode->Len()};
                YImplInsertFeature<YMapEntry>(rIEE, pam, pChunks[i].fmt, pChunks[i].fmt_len, GetAttr);
                strStart = j+1;
            }
        }
        if (strStart != str.getLength())
        {
            OString const portion{str.copy(strStart, str.getLength() - strStart)};
            pNode->Append(OStringToOUString(portion, RTL_TEXTENCODING_UTF8));
        }
        assert((pNode == nullptr) == (str[str.getLength()-1] == CH_PARA));
        ContentNode *const pEndNode{pNode ? pNode : maContents[nodes-1].get()};
        EditSelection const sel{EditPaM(maContents[nodeStart].get(), posStart), EditPaM(pEndNode, pEndNode->Len())};
        SfxItemSet set{rIEE.GetEmptyItemSet()};
        for (decltype(pChunks[i].fmt_len) j = 0; j < pChunks[i].fmt_len; ++j)
        {
            YImplInsertAttr(set, nullptr, pChunks[i].fmt[j].key, *pChunks[i].fmt[j].value);
        }
        if (set.Count())
        {
            rIEE.SetAttribs(sel, set);
        }
    }
    ychunks_destroy(pChunks, chunks);
#if 1
    rIEE.RemoveParagraph(nodes); // remove pre-existing one from InitDoc()
    //rIEE.ImpRemoveParagraph(nodes); // remove pre-existing one from InitDoc()
    //rIEE.FormatAndLayout();
#endif
}

static void YAdjustCursors(ImpEditEngine & rIEE, EditDoc & rDoc, /*ContentNode *const pNode,*/sal_Int32 const node, sal_Int32 const pos, ContentNode *const pNewNode, sal_Int32 const delta)
{
    for (EditView *const pView : rIEE.GetEditViews())
    {
        bool bSet{false};
        EditSelection sel{pView->getImpl().GetEditSelection()};
        ContentNode const*const pNode{rDoc.GetObject(node)};
        if (sel.Min().GetNode() == pNode
            && pos <= sel.Min().GetIndex())
        {
            sel.Min().SetNode(pNewNode);
            sel.Min().SetIndex(sel.Min().GetIndex() + delta);
            bSet = true;
        }
        if (sel.Max().GetNode() == pNode
            && pos <= sel.Max().GetIndex())
        {
            sel.Max().SetNode(pNewNode);
            sel.Max().SetIndex(sel.Max().GetIndex() + delta);
            bSet = true;
        }
        if (bSet)
        {
            pView->getImpl().SetEditSelection(sel);
        }
    }
}

// TODO test this
static void YAdjustCursorsDel(ImpEditEngine & rIEE, EditDoc & rDoc, sal_Int32 const startNode, sal_Int32 const startPos, sal_Int32 const endNode, sal_Int32 const endPos)
{
    for (EditView *const pView : rIEE.GetEditViews())
    {
        bool bSet{false};
        EditSelection sel{pView->getImpl().GetEditSelection()};
        ContentNode *const pStartNode{rDoc.GetObject(startNode)};
        ContentNode const*const pEndNode{rDoc.GetObject(endNode)};
        if ((sel.Min().GetNode() == pStartNode && startPos < sel.Min().GetIndex())
            || (startNode < rDoc.GetPos(sel.Min().GetNode()) && rDoc.GetPos(sel.Min().GetNode()) < endNode)
            || (sel.Min().GetNode() == pEndNode && sel.Min().GetIndex() < endPos))
        {
            sel.Min().SetNode(pStartNode);
            sel.Min().SetIndex(startPos);
            bSet = true;
        }
        else if (sel.Min().GetNode() == pEndNode)
        {
            sel.Min().SetNode(pStartNode);
            sel.Min().SetIndex(startPos + sel.Min().GetIndex() - endPos);
            bSet = true;
        }
        if ((sel.Max().GetNode() == pStartNode && startPos < sel.Max().GetIndex())
            || (startNode < rDoc.GetPos(sel.Max().GetNode()) && rDoc.GetPos(sel.Max().GetNode()) < endNode)
            || (sel.Max().GetNode() == pEndNode && sel.Max().GetIndex() < endPos))
        {
            sel.Max().SetNode(pStartNode);
            sel.Max().SetIndex(startPos);
            bSet = true;
        }
        else if (sel.Max().GetNode() == pEndNode)
        {
            sel.Max().SetNode(pStartNode);
            sel.Max().SetIndex(startPos + sel.Max().GetIndex() - endPos);
            bSet = true;
        }
        if (bSet)
        {
            pView->getImpl().SetEditSelection(sel);
        }
    }
}

void EditDoc::YrsApplyEEDelta(YTransaction *const /*pTxn*/, YTextEvent const*const pEvent, ImpEditEngine & rIEE)
{
    uint32_t lenC{0};
    YDeltaOut *const pChange{ytext_event_delta(pEvent, &lenC)};

    sal_Int32 node{0};
    sal_Int32 pos{0};

    for (decltype(lenC) i = 0; i < lenC; ++i)
    {
        switch (pChange[i].tag)
        {
            case Y_EVENT_CHANGE_ADD:
            {
                decltype(node) const nodeStart{node};
                decltype(pos) const posStart{pos};

                SfxStyleSheet * pStyle{nullptr};
                SfxItemSet set{rIEE.GetEmptyItemSet()};
                for (decltype(pChange[i].attributes_len) j = 0; j < pChange[i].attributes_len; ++j)
                {
                    YImplInsertAttr(set, nullptr, pChange[i].attributes[j].key, pChange[i].attributes[j].value);
                    if (strcmp(pChange[i].attributes[j].key, "para-style") == 0)
                    {
                        yvalidate(pChange[i].attributes[j].value.tag == Y_JSON_STR);
                        OUString const style{OStringToOUString(pChange[i].attributes[j].value.value.str, RTL_TEXTENCODING_UTF8)};
                        pStyle = dynamic_cast<SfxStyleSheet *>(
                            rIEE.GetStyleSheetPool()->Find(style, SfxStyleFamily::Para));
                        if (!pStyle) { abort(); }
                    }
                }

                for (decltype(pChange[i].len) j = 0; j < pChange[i].len; ++j)
                {
                    switch (pChange[i].insert[j].tag)
                    {
                        case Y_JSON_STR:
                        {
                            OUString const str{OStringToOUString(::std::string_view(pChange[i].insert[j].value.str), RTL_TEXTENCODING_UTF8)};
                            if (str.getLength() == 1 && str[0] == CH_FEATURE)
                            {
                                auto GetAttr = [](YDeltaAttr const& rEntry) -> ::std::pair<char const*, YOutput const&>
                                    { return {rEntry.key, rEntry.value}; };
                                EditPaM const pam{maContents[node].get(), pos};
                                YImplInsertFeature<YDeltaAttr>(rIEE, pam, pChange[i].attributes, pChange[i].attributes_len, GetAttr);
                                YAdjustCursors(rIEE, *this, node, pos, maContents[node].get(), 1);
                                ++pos;
                                break;
                            }
                            sal_Int32 index{0};
                            sal_Int32 iPara{str.indexOf(CH_PARA)};
                            if (iPara != -1)
                            {
                                if (index != iPara)
                                {
                                    EditSelection const sel{EditPaM{maContents[node].get(), pos}};
                                    rIEE.InsertText(sel, str.copy(index, iPara));
                                }
                                EditPaM const newPos{rIEE.SplitContent(node, pos + iPara)};
                                rIEE.SetStyleSheet(node, pStyle);
                                YAdjustCursors(rIEE, *this, node, pos, const_cast<ContentNode*>(newPos.GetNode()), -pos);
                                index = iPara + 1;
                                pos = 0;
                                ++node;
                                iPara = str.indexOf(CH_PARA, index);
                            }
                            while (iPara != -1)
                            {
//                                EditSelection const sel{EditPaM{maContents[node].get(), pos}};
//                                rIEE.InsertText(sel, str);
                                rIEE.InsertParagraph(node, str.copy(index, iPara - index));
                                rIEE.SetStyleSheet(node, pStyle);
                                index = iPara + 1;
                                ++node;
                                iPara = str.indexOf(CH_PARA, index);
                            }
                            assert(iPara == -1);
                            if (index != str.getLength())
                            {
                                EditSelection const sel{EditPaM{maContents[node].get(), pos}};
                                rIEE.InsertText(sel, str.copy(index, str.getLength() - index));
                                YAdjustCursors(rIEE, *this, node, pos, maContents[node].get(), str.getLength() - index);
                                pos += str.getLength() - index;
                            }
                            break;
                        }
                        default:
                            assert(false);
                    }
                }

                EditPaM const start{maContents[nodeStart].get(), posStart};
                EditPaM const end{maContents[node].get(), pos};
                EditSelection const sel{start, end};
                if (set.Count())
                {
                    rIEE.SetAttribs(sel, set);
                }
            }
            break;
            case Y_EVENT_CHANGE_DELETE:
            case Y_EVENT_CHANGE_RETAIN:
            {
                decltype(node) const nodeStart{node};
                decltype(pos) const posStart{pos};

                // len should be UTF16 via Y_OFFSET_UTF16
                sal_Int32 len{static_cast<sal_Int32>(pChange[i].len)};
                while (0 < len)
                {
                    yvalidate(o3tl::make_unsigned(node) < maContents.size());
                    ContentNode & rNode{*maContents[node]};
                    if (pos + len <= rNode.Len())
                    {
                        pos += len;
                        len = 0;
                    }
                    else
                    {
                        len -= rNode.Len() - pos + 1;
                        pos = 0;
                        ++node;
                    }
                }
                ::std::optional<EditSelection> oSel;
                // setting attribute on node special case - there may be
                // EE_PARA_* obviously and possibly also EE_CHAR_* but
                // unclear how those would be inserted - and a "para-style"
                if (pos == 0 && pChange[i].len == 1 && pChange[i].tag == Y_EVENT_CHANGE_RETAIN)
                {
                    assert(o3tl::make_unsigned(node) <= maContents.size());
                    EditPaM const start{maContents[node - 1].get(), 0};
                    oSel.emplace(start);
                }
                else
                {
                    if (pos == 0 && static_cast<size_t>(node) == maContents.size())
                    {   // adjust past-the-end positon (formatting change)
                        --node;
                        pos = maContents[node]->Len();
                    }
                    assert(o3tl::make_unsigned(node) < maContents.size() && pos <= maContents[node]->Len());
                    EditPaM const start{maContents[nodeStart].get(), posStart};
                    EditPaM const end{maContents[node].get(), pos};
                    oSel.emplace(start, end);
                }
                if (pChange[i].tag == Y_EVENT_CHANGE_DELETE)
                {
                    YAdjustCursorsDel(rIEE, *this, nodeStart, posStart, node, pos);
                    rIEE.DeleteSelected(*oSel);
                    node = nodeStart;
                    pos = posStart;
                }
                else if (0 < pChange[i].attributes_len)
                {
                    assert(pChange[i].tag == Y_EVENT_CHANGE_RETAIN);
                    SfxItemSet set{rIEE.GetEmptyItemSet()};
                    ::std::vector<sal_uInt16> removed;
                    for (decltype(pChange[i].attributes_len) j = 0; j < pChange[i].attributes_len; ++j)
                    {
                        if (pos == 0 && pChange[i].len == 1
                            && strcmp(pChange[i].attributes[j].key, "para-style") == 0)
                        {
                            yvalidate(pChange[i].attributes[j].value.tag == Y_JSON_STR);
                            OUString const style{OStringToOUString(pChange[i].attributes[j].value.value.str, RTL_TEXTENCODING_UTF8)};
                            SfxStyleSheet *const pStyle{dynamic_cast<SfxStyleSheet *>(
                                rIEE.GetStyleSheetPool()->Find(style, SfxStyleFamily::Para))};
                            if (!pStyle) { abort(); }
                            rIEE.SetStyleSheet(node - 1, pStyle);
                        }
                        else
                        {
                            YImplInsertAttr(set, &removed, pChange[i].attributes[j].key, pChange[i].attributes[j].value);
                        }
                    }
                    if (set.Count())
                    {
                        rIEE.SetAttribs(*oSel, set);
                    }
                    for (auto const nWhich : removed)
                    {
                        rIEE.RemoveCharAttribs(*oSel, EERemoveParaAttribsMode::RemoveAll, nWhich);
                    }
                }
            }
            break;
            default:
                assert(false);
        }
    }

    ytext_delta_destroy(pChange, lenC);
}

#if 0
void EditDoc::SetYrsTransactionSupplier(IYrsTransactionSupplier *const pYrsSupplier)
{
    assert(!m_pYrsSupplier);
    m_pYrsSupplier = pYrsSupplier;
    assert(!m_CommentId.isEmpty());

}
#endif

void EditDoc::SetYrsCommentId(IYrsTransactionSupplier *const pYrsSupplier, OString const& rId)
{
    assert(!m_pYrsSupplier);
    m_pYrsSupplier = pYrsSupplier;
    m_CommentId = rId;
}

OString EditDoc::GetCommentId() const
{
    return m_CommentId;
}

void EditView::SetYrsCommentId(IYrsTransactionSupplier *const pYrsSupplier, OString const& rId)
{
    getEditEngine().GetEditDoc().SetYrsCommentId(pYrsSupplier, rId);
}

OString EditView::GetCommentId() const
{
    return getEditEngine().GetEditDoc().GetCommentId();
}

void EditView::YrsWriteEEState()
{
    return getEditEngine().GetEditDoc().YrsWriteEEState();
}

void EditView::YrsReadEEState(YTransaction *const pTxn)
{
    return getEditEngine().GetEditDoc().YrsReadEEState(pTxn, getImpEditEngine());
}

void EditView::YrsApplyEEDelta(YTransaction *const pTxn, YTextEvent const*const pEvent)
{
    return getEditEngine().GetEditDoc().YrsApplyEEDelta(pTxn, pEvent, getImpEditEngine());
}
#endif

EditDoc::EditDoc( SfxItemPool* pPool ) :
    mnLastCache(0),
    mpItemPool(pPool ? pPool : new EditEngineItemPool()),
    mnDefTab(DEFTAB),
    mbIsVertical(false),
    mnRotation(TextRotation::NONE),
    mbIsFixedCellHeight(false),
    mbModified(false),
    mbDisableAttributeExpanding(false)
{
    SAL_DEBUG("YRS +EditDoc");
    // Don't create an empty node, Clear() will be called in EditEngine-CTOR
};

EditDoc::~EditDoc()
{
    SAL_DEBUG("YRS -EditDoc");
    maContents.clear();
}

// not sure which of the members make sense to sync - if its only a cache for
// some value elsewhere in the model then probably not?
void EditDoc::SetVertical(bool const bVertical)
{
    mbIsVertical = bVertical;
#if defined(YRS)
    YSetVertical(m_pYrsSupplier, m_CommentId, mbIsVertical);
#endif
}

void EditDoc::SetRotation(TextRotation const nRotation)
{
    mnRotation = nRotation;
#if defined(YRS)
    YSetRotation(m_pYrsSupplier, m_CommentId, mnRotation);
#endif
}

void EditDoc::SetDefTab(sal_uInt16 const nTab)
{
    mnDefTab = nTab ? nTab : DEFTAB;
#if defined(YRS)
    YSetDefTab(m_pYrsSupplier, m_CommentId, mnDefTab);
#endif
}

void CreateFont( SvxFont& rFont, const SfxItemSet& rSet, bool bSearchInParent, SvtScriptType nScriptType )
{
    vcl::Font aPrevFont( rFont );
    rFont.SetAlignment( ALIGN_BASELINE );

    sal_uInt16 nWhich_FontInfo = GetScriptItemId( EE_CHAR_FONTINFO, nScriptType );
    sal_uInt16 nWhich_Language = GetScriptItemId( EE_CHAR_LANGUAGE, nScriptType );
    sal_uInt16 nWhich_FontHeight = GetScriptItemId( EE_CHAR_FONTHEIGHT, nScriptType );
    sal_uInt16 nWhich_Weight = GetScriptItemId( EE_CHAR_WEIGHT, nScriptType );
    sal_uInt16 nWhich_Italic = GetScriptItemId( EE_CHAR_ITALIC, nScriptType );

    if ( bSearchInParent || ( rSet.GetItemState( nWhich_FontInfo ) == SfxItemState::SET ) )
    {
        const SvxFontItem& rFontItem = static_cast<const SvxFontItem&>(rSet.Get( nWhich_FontInfo ));
        rFont.SetFamilyName( rFontItem.GetFamilyName() );
        rFont.SetFamily( rFontItem.GetFamily() );
        rFont.SetPitch( rFontItem.GetPitch() );
        rFont.SetCharSet( rFontItem.GetCharSet() );
    }
    if ( bSearchInParent || ( rSet.GetItemState( nWhich_Language ) == SfxItemState::SET ) )
        rFont.SetLanguage( static_cast<const SvxLanguageItem&>(rSet.Get( nWhich_Language )).GetLanguage() );
    if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_COLOR ) == SfxItemState::SET ) )
        rFont.SetColor( rSet.Get( EE_CHAR_COLOR ).GetValue() );
    if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_BKGCOLOR ) == SfxItemState::SET ) )
    {
        auto& aColor = rSet.Get( EE_CHAR_BKGCOLOR ).GetValue();
        rFont.SetTransparent(aColor.IsTransparent());
        rFont.SetFillColor(aColor);
    }
    if ( bSearchInParent || ( rSet.GetItemState( nWhich_FontHeight ) == SfxItemState::SET ) )
        rFont.SetFontSize( Size( rFont.GetFontSize().Width(), static_cast<const SvxFontHeightItem&>(rSet.Get( nWhich_FontHeight ) ).GetHeight() ) );
    if ( bSearchInParent || ( rSet.GetItemState( nWhich_Weight ) == SfxItemState::SET ) )
        rFont.SetWeight( static_cast<const SvxWeightItem&>(rSet.Get( nWhich_Weight )).GetWeight() );
    if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_UNDERLINE ) == SfxItemState::SET ) )
        rFont.SetUnderline( rSet.Get( EE_CHAR_UNDERLINE ).GetLineStyle() );
    if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_OVERLINE ) == SfxItemState::SET ) )
        rFont.SetOverline( rSet.Get( EE_CHAR_OVERLINE ).GetLineStyle() );
    if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_STRIKEOUT ) == SfxItemState::SET ) )
        rFont.SetStrikeout( rSet.Get( EE_CHAR_STRIKEOUT ).GetStrikeout() );
    if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_CASEMAP ) == SfxItemState::SET ) )
        rFont.SetCaseMap( rSet.Get( EE_CHAR_CASEMAP ).GetCaseMap() );
    if ( bSearchInParent || ( rSet.GetItemState( nWhich_Italic ) == SfxItemState::SET ) )
        rFont.SetItalic( static_cast<const SvxPostureItem&>(rSet.Get( nWhich_Italic )).GetPosture() );
    if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_OUTLINE ) == SfxItemState::SET ) )
        rFont.SetOutline( rSet.Get( EE_CHAR_OUTLINE ).GetValue() );
    if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_SHADOW ) == SfxItemState::SET ) )
        rFont.SetShadow( rSet.Get( EE_CHAR_SHADOW ).GetValue() );
    if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_ESCAPEMENT ) == SfxItemState::SET ) )
    {
        const SvxEscapementItem& rEsc = rSet.Get( EE_CHAR_ESCAPEMENT );

        sal_uInt16 const nProp = rEsc.GetProportionalHeight();
        rFont.SetPropr( static_cast<sal_uInt8>(nProp) );

        short nEsc = rEsc.GetEsc();
        rFont.SetNonAutoEscapement( nEsc );
    }
    if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_PAIRKERNING ) == SfxItemState::SET ) )
        rFont.SetKerning( rSet.Get( EE_CHAR_PAIRKERNING ).GetValue() ? FontKerning::FontSpecific : FontKerning::NONE );
    if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_KERNING ) == SfxItemState::SET ) )
        rFont.SetFixKerning( rSet.Get( EE_CHAR_KERNING ).GetValue() );
    if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_WLM ) == SfxItemState::SET ) )
        rFont.SetWordLineMode( rSet.Get( EE_CHAR_WLM ).GetValue() );
    if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_EMPHASISMARK ) == SfxItemState::SET ) )
        rFont.SetEmphasisMark( rSet.Get( EE_CHAR_EMPHASISMARK ).GetEmphasisMark() );
    if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_RELIEF ) == SfxItemState::SET ) )
        rFont.SetRelief( rSet.Get( EE_CHAR_RELIEF ).GetValue() );

    // Operator == compares the individual members of the font if the impl pointer is
    // not equal. If all members are the same, this assignment makes
    // sure that both also point to the same internal instance of the font.
    // To avoid this assignment, you would need to check in
    // every if statement above whether or not the new value differs from the
    // old value before making an assignment.
    if ( rFont == aPrevFont  )
        rFont = aPrevFont;  // => The same ImpPointer for IsSameInstance
}

void EditDoc::CreateDefFont( bool bUseStyles )
{
    SfxItemSetFixed<EE_PARA_START, EE_CHAR_END> aTmpSet( GetItemPool() );
    // maDefFont depends only on items and flags, no need to sync
    CreateFont(maDefFont, aTmpSet);
    maDefFont.SetVertical( IsEffectivelyVertical() );
    maDefFont.SetOrientation( Degree10(IsEffectivelyVertical() ? (IsTopToBottom() ? 2700 : 900) : 0) );

    for (std::unique_ptr<ContentNode>& pNode : maContents)
    {
        pNode->GetCharAttribs().GetDefFont() = maDefFont;
        if (bUseStyles)
            pNode->CreateDefFont();
    }
}

bool EditDoc::IsEffectivelyVertical() const
{
    return (mbIsVertical && mnRotation == TextRotation::NONE) ||
        (!mbIsVertical && mnRotation != TextRotation::NONE);
}

bool EditDoc::IsTopToBottom() const
{
    return (mbIsVertical && mnRotation == TextRotation::NONE) ||
        (!mbIsVertical && mnRotation == TextRotation::TOPTOBOTTOM);
}

bool EditDoc::GetVertical() const
{
    return mbIsVertical;
}

sal_Int32 EditDoc::GetPos(const ContentNode* pContentNode) const
{
    return FastGetPos(maContents, pContentNode, mnLastCache);
}

const ContentNode* EditDoc::GetObject(sal_Int32 nPos) const
{
    return 0 <= nPos && o3tl::make_unsigned(nPos) < maContents.size() ? maContents[nPos].get() : nullptr;
}

ContentNode* EditDoc::GetObject(sal_Int32 nPos)
{
    return 0 <= nPos && o3tl::make_unsigned(nPos) < maContents.size() ? maContents[nPos].get() : nullptr;
}

void EditDoc::Insert(sal_Int32 nPos, std::unique_ptr<ContentNode> pNode)
{
    if (nPos < 0 || nPos == SAL_MAX_INT32)
    {
        SAL_WARN( "editeng", "EditDoc::Insert - overflow pos " << nPos);
        return;
    }
    maContents.insert(maContents.begin()+nPos, std::move(pNode));
#if defined(YRS)
    YAddPara(m_pYrsSupplier, m_CommentId, *this, nPos);
#endif
}

void EditDoc::Remove(sal_Int32 nPos)
{
    if (nPos < 0 || o3tl::make_unsigned(nPos) >= maContents.size())
    {
        SAL_WARN( "editeng", "EditDoc::Remove - out of bounds pos " << nPos);
        return;
    }
#if defined(YRS)
    YRemovePara(m_pYrsSupplier, m_CommentId, *this, nPos);
#endif
    maContents.erase(maContents.begin() + nPos);
}

std::unique_ptr<ContentNode> EditDoc::Release(sal_Int32 nPos)
{
    if (nPos < 0 || o3tl::make_unsigned(nPos) >= maContents.size())
    {
        SAL_WARN( "editeng", "EditDoc::Release - out of bounds pos " << nPos);
        return nullptr;
    }

#if defined(YRS)
    YRemovePara(m_pYrsSupplier, m_CommentId, *this, nPos);
#endif
    std::unique_ptr<ContentNode> pNode = std::move(maContents[nPos]);
    maContents.erase(maContents.begin() + nPos);
    return pNode;
}

sal_Int32 EditDoc::Count() const
{
    size_t nSize = maContents.size();
    if (nSize > SAL_MAX_INT32)
    {
        SAL_WARN( "editeng", "EditDoc::Count - overflow " << nSize);
        return SAL_MAX_INT32;
    }
    return nSize;
}

OUString EditDoc::GetSepStr( LineEnd eEnd )
{
    if ( eEnd == LINEEND_CR )
        return u"\015"_ustr; // 0x0d
    if ( eEnd == LINEEND_LF )
        return u"\012"_ustr; // 0x0a
    return u"\015\012"_ustr; // 0x0d, 0x0a
}

OUString EditDoc::GetText( LineEnd eEnd ) const
{
    const sal_Int32 nNodes = Count();
    if (nNodes == 0)
        return OUString();

    const OUString aSep = EditDoc::GetSepStr( eEnd );
    const sal_Int32 nSepSize = aSep.getLength();
    const sal_Int32 nLen = GetTextLen() + (nNodes - 1)*nSepSize;

    OUStringBuffer aBuffer(nLen + 16); // leave some slack

    for ( sal_Int32 nNode = 0; nNode < nNodes; nNode++ )
    {
        if ( nSepSize && nNode>0 )
        {
            aBuffer.append(aSep);
        }
        aBuffer.append(GetParaAsString( GetObject(nNode) ));
    }

    return aBuffer.makeStringAndClear();
}

OUString EditDoc::GetParaAsString( sal_Int32 nNode ) const
{
    return GetParaAsString( GetObject( nNode ) );
}

OUString EditDoc::GetParaAsString(
    const ContentNode* pNode, sal_Int32 nStartPos, sal_Int32 nEndPos)
{
    return pNode->GetExpandedText(nStartPos, nEndPos);
}

EditPaM EditDoc::GetStartPaM() const
{
    ContentNode* p = const_cast<ContentNode*>(GetObject(0));
    return EditPaM(p, 0);
}

EditPaM EditDoc::GetEndPaM() const
{
    ContentNode* pLastNode = const_cast<ContentNode*>(GetObject(Count()-1));
    return EditPaM( pLastNode, pLastNode->Len() );
}

sal_Int32 EditDoc::GetTextLen() const
{
    sal_Int32 nLength = 0;
    for (auto const& pContent : maContents)
    {
        nLength += pContent->GetExpandedLen();
    }
    return nLength;
}

EditPaM EditDoc::Clear()
{
    maContents.clear();

#if defined(YRS)
    // Insert will call YAddPara()
    YClear(m_pYrsSupplier, m_CommentId);
#endif

    ContentNode* pNode = new ContentNode(GetItemPool());
    Insert(0, std::unique_ptr<ContentNode>(pNode));

    CreateDefFont(false);

    SetModified(false);

    return EditPaM( pNode, 0 );
}

namespace
{
struct ClearSpellErrorsHandler
{
    void operator() (std::unique_ptr<ContentNode> const & rNode)
    {
        rNode->DestroyWrongList();
    }
};
}

void EditDoc::ClearSpellErrors()
{
    std::for_each(maContents.begin(), maContents.end(), ClearSpellErrorsHandler());
}

void EditDoc::SetModified( bool b )
{
    mbModified = b;
    if (mbModified)
        // TODO use this to send?
        maModifyHdl.Call(nullptr);
}

EditPaM EditDoc::RemoveText()
{
    // Keep the old ItemSet, to keep the chart Font.
    ContentNode* pPrevFirstNode = GetObject(0);
    SfxStyleSheet* pPrevStyle = pPrevFirstNode->GetStyleSheet();
    SfxItemSet aPrevSet( pPrevFirstNode->GetContentAttribs().GetItems() );
    vcl::Font aPrevFont( pPrevFirstNode->GetCharAttribs().GetDefFont() );

    maContents.clear();

#if defined(YRS)
    // Insert will call YAddPara()
    YClear(m_pYrsSupplier, m_CommentId);
#endif

    ContentNode* pNode = new ContentNode(GetItemPool());
    Insert(0, std::unique_ptr<ContentNode>(pNode));

    pNode->SetStyleSheet(pPrevStyle, false);
    pNode->GetContentAttribs().GetItems().Set( aPrevSet );
    pNode->GetCharAttribs().GetDefFont() = aPrevFont;

    SetModified(true);

    return EditPaM(pNode, 0);
}

EditPaM EditDoc::InsertText( EditPaM aPaM, const OUString& rStr )
{
    DBG_ASSERT( rStr.indexOf( 0x0A ) == -1, "EditDoc::InsertText: Newlines prohibited in paragraph!" );
    DBG_ASSERT( rStr.indexOf( 0x0D ) == -1, "EditDoc::InsertText: Newlines prohibited in paragraph!" );
    DBG_ASSERT( rStr.indexOf( '\t' ) == -1, "EditDoc::InsertText: Newlines prohibited in paragraph!" );

    ContentNode* pNode = aPaM.GetNode();
    assert(pNode);
    pNode->Insert( rStr, aPaM.GetIndex() );
    pNode->ExpandAttribs( aPaM.GetIndex(), rStr.getLength() );
    aPaM.SetIndex( aPaM.GetIndex() + rStr.getLength() );

    SetModified( true );

#if defined(YRS)
    // ??? ExpandAttribs - does ycrdt do the same or what?
    YInsertText(m_pYrsSupplier, m_CommentId, *this, GetPos(aPaM.GetNode()), aPaM.GetIndex() - rStr.getLength(), rStr);
#endif

    return aPaM;
}

EditPaM EditDoc::InsertParaBreak( EditPaM aPaM, bool bKeepEndingAttribs )
{
    assert(aPaM.GetNode());
    ContentNode* pCurNode = aPaM.GetNode();
    sal_Int32 nPos = GetPos( pCurNode );
    assert(nPos != EE_PARA_MAX);
    OUString aStr = aPaM.GetNode()->Copy( aPaM.GetIndex() );
    aPaM.GetNode()->Erase( aPaM.GetIndex() );

    // the paragraph attributes...
    ContentAttribs aContentAttribs( aPaM.GetNode()->GetContentAttribs() );

    // for a new paragraph we like to have the bullet/numbering visible by default
    aContentAttribs.GetItems().Put( SfxBoolItem( EE_PARA_BULLETSTATE, true) );

    // ContentNode constructor copies also the paragraph attributes
    ContentNode* pNode = new ContentNode(aStr, std::move(aContentAttribs));

    // Copy the Default Font
    pNode->GetCharAttribs().GetDefFont() = aPaM.GetNode()->GetCharAttribs().GetDefFont();
    SfxStyleSheet* pStyle = aPaM.GetNode()->GetStyleSheet();
    if ( pStyle )
    {
        OUString aFollow( pStyle->GetFollow() );
        if ( !aFollow.isEmpty() && ( aFollow != pStyle->GetName() ) )
        {
            SfxStyleSheetBase* pNext = pStyle->GetPool()->Find( aFollow, pStyle->GetFamily() );
            pNode->SetStyleSheet( static_cast<SfxStyleSheet*>(pNext) );
        }
    }

    // Character attributes may need to be copied or trimmed:
    pNode->CopyAndCutAttribs( aPaM.GetNode(), GetItemPool(), bKeepEndingAttribs );

#if defined(YRS)
    {
    // skip the YAddPara in Insert
        ReplayGuard const g{m_pYrsSupplier};
#endif
    Insert(nPos+1, std::unique_ptr<ContentNode>(pNode));
#if defined(YRS)
    }
#endif

    SetModified(true);

#if defined(YRS)
    YInsertParaBreak(m_pYrsSupplier, m_CommentId, *this, nPos, aPaM.GetIndex());
#endif

    aPaM.SetNode( pNode );
    aPaM.SetIndex( 0 );
    return aPaM;
}

EditPaM EditDoc::InsertFeature( EditPaM aPaM, const SfxPoolItem& rItem  )
{
    assert(aPaM.GetNode());

    aPaM.GetNode()->Insert( rtl::OUStringChar(CH_FEATURE), aPaM.GetIndex() );
    aPaM.GetNode()->ExpandAttribs( aPaM.GetIndex(), 1 );

    // Create a feature-attribute for the feature...
    EditCharAttrib* pAttrib = MakeCharAttrib( GetItemPool(), rItem, aPaM.GetIndex(), aPaM.GetIndex()+1 );
    assert(pAttrib);
    aPaM.GetNode()->GetCharAttribs().InsertAttrib( pAttrib );

    SetModified( true );

#if defined(YRS)
    YInsertFeature(m_pYrsSupplier, m_CommentId, *this, GetPos(aPaM.GetNode()), pAttrib);
#endif

    aPaM.SetIndex( aPaM.GetIndex() + 1 );
    return aPaM;
}

EditPaM EditDoc::ConnectParagraphs( ContentNode* pLeft, ContentNode* pRight )
{
    const EditPaM aPaM( pLeft, pLeft->Len() );

    // First the attributes, otherwise nLen will not be correct!
    pLeft->AppendAttribs( pRight );
    // then the Text...
    pLeft->Append(pRight->GetString());

    // the one to the right disappears.
    sal_Int32 nRight = GetPos( pRight );
#if defined(YRS)
    {
    // skip the YRemovePara in Remove, the node even still has the text...
        ReplayGuard const g{m_pYrsSupplier};
#endif
    Remove( nRight );
#if defined(YRS)
    }
#endif

    SetModified(true);

#if defined(YRS)
    assert(nRight != 0);
    YConnectPara(m_pYrsSupplier, m_CommentId, *this, nRight - 1, aPaM.GetIndex());
#endif

    return aPaM;
}

void EditDoc::RemoveChars( EditPaM aPaM, sal_Int32 nChars )
{
    // Maybe remove Features!
    aPaM.GetNode()->Erase( aPaM.GetIndex(), nChars );
    aPaM.GetNode()->CollapseAttribs( aPaM.GetIndex(), nChars );

    SetModified( true );

#if defined(YRS)
    YRemoveChars(m_pYrsSupplier, m_CommentId, *this, GetPos(aPaM.GetNode()), aPaM.GetIndex(), nChars);
#endif
}

void EditDoc::InsertAttribInSelection( ContentNode* pNode, sal_Int32 nStart, sal_Int32 nEnd, const SfxPoolItem& rPoolItem )
{
    assert(pNode);
    DBG_ASSERT( nEnd <= pNode->Len(), "InsertAttrib: Attribute too large!" );

    // for Optimization:
    // This ends at the beginning of the selection => can be expanded
    EditCharAttrib* pEndingAttrib = nullptr;
    // This starts at the end of the selection => can be expanded
    EditCharAttrib* pStartingAttrib = nullptr;

    DBG_ASSERT( nStart <= nEnd, "Small miscalculations in InsertAttribInSelection" );

    RemoveAttribs( pNode, nStart, nEnd, pStartingAttrib, pEndingAttrib, rPoolItem.Which() );

    // tdf#132288  By default inserting an attribute beside another that is of
    // the same type expands the original instead of inserting another. But the
    // spell check dialog doesn't want that behaviour
    if (mbDisableAttributeExpanding)
    {
        pStartingAttrib = nullptr;
        pEndingAttrib = nullptr;
    }

    if ( pStartingAttrib && pEndingAttrib &&
         ( *(pStartingAttrib->GetItem()) == rPoolItem ) &&
         ( *(pEndingAttrib->GetItem()) == rPoolItem ) )
    {
        // Will become a large Attribute.
        pEndingAttrib->GetEnd() = pStartingAttrib->GetEnd();
        pNode->GetCharAttribs().Remove(pStartingAttrib);
#if defined(YRS)
        YInsertAttrib(m_pYrsSupplier, m_CommentId, *this, GetPos(pNode), pEndingAttrib);
#endif
    }
    else if ( pStartingAttrib && ( *(pStartingAttrib->GetItem()) == rPoolItem ) )
    {
        pStartingAttrib->GetStart() = nStart;
#if defined(YRS)
        YInsertAttrib(m_pYrsSupplier, m_CommentId, *this, GetPos(pNode), pStartingAttrib);
#endif
    }
    else if ( pEndingAttrib && ( *(pEndingAttrib->GetItem()) == rPoolItem ) )
    {
        pEndingAttrib->GetEnd() = nEnd;
#if defined(YRS)
        YInsertAttrib(m_pYrsSupplier, m_CommentId, *this, GetPos(pNode), pEndingAttrib);
#endif
    }
    else
        InsertAttrib( rPoolItem, pNode, nStart, nEnd );

    if ( pStartingAttrib )
        pNode->GetCharAttribs().ResortAttribs();

    SetModified(true);
}

bool EditDoc::RemoveAttribs( ContentNode* pNode, sal_Int32 nStart, sal_Int32 nEnd, sal_uInt16 nWhich )
{
    EditCharAttrib* pStarting;
    EditCharAttrib* pEnding;
    return RemoveAttribs( pNode, nStart, nEnd, pStarting, pEnding, nWhich );
}

bool EditDoc::RemoveAttribs( ContentNode* pNode, sal_Int32 nStart, sal_Int32 nEnd, EditCharAttrib*& rpStarting, EditCharAttrib*& rpEnding, sal_uInt16 nWhich )
{

    assert(pNode);
    DBG_ASSERT( nEnd <= pNode->Len(), "InsertAttrib: Attribute too large!" );

    // This ends at the beginning of the selection => can be expanded
    rpEnding = nullptr;
    // This starts at the end of the selection => can be expanded
    rpStarting = nullptr;

    bool bChanged = false;
    bool bNeedsSorting = false;

    DBG_ASSERT( nStart <= nEnd, "Small miscalculations in InsertAttribInSelection" );

#if OSL_DEBUG_LEVEL > 0 && !defined NDEBUG
    CharAttribList::DbgCheckAttribs(pNode->GetCharAttribs());
#endif

    // iterate over the attributes ...
    std::size_t nAttr = 0;
    CharAttribList::AttribsType& rAttribs = pNode->GetCharAttribs().GetAttribs();
    EditCharAttrib* pAttr = GetAttrib(rAttribs, nAttr);
    while ( pAttr )
    {
        bool bRemoveAttrib = false;
        sal_uInt16 nAttrWhich = pAttr->Which();
        if ( ( nAttrWhich < EE_FEATURE_START ) && ( !nWhich || ( nAttrWhich == nWhich ) ) )
        {
            // Attribute starts in Selection
            if ( ( pAttr->GetStart() >= nStart ) && ( pAttr->GetStart() <= nEnd ) )
            {
                bChanged = true;
                if ( pAttr->GetEnd() > nEnd )
                {
                    bNeedsSorting = true;
#if defined(YRS)
                    YRemoveAttrib(m_pYrsSupplier, m_CommentId, *this, GetPos(pNode), pAttr->Which(), pAttr->GetStart(), nEnd);
#endif
                    pAttr->GetStart() = nEnd;   // then it starts after this
                    rpStarting = pAttr;
                    if ( nWhich )
                        break;  // There can be no further attributes here
                }
                else if ( !pAttr->IsFeature() || ( pAttr->GetStart() == nStart ) )
                {
                    // Delete feature only if on the exact spot
                    bRemoveAttrib = true;
                }
            }

            // Attribute ends in Selection
            else if ( ( pAttr->GetEnd() >= nStart ) && ( pAttr->GetEnd() <= nEnd ) )
            {
                bChanged = true;
                if ( ( pAttr->GetStart() < nStart ) && !pAttr->IsFeature() )
                {
#if defined(YRS)
                    YRemoveAttrib(m_pYrsSupplier, m_CommentId, *this, GetPos(pNode), pAttr->Which(), nStart, pAttr->GetEnd());
#endif
                    pAttr->GetEnd() = nStart;   // then it ends here
                    rpEnding = pAttr;
                }
                else if ( !pAttr->IsFeature() || ( pAttr->GetStart() == nStart ) )
                {
                    // Delete feature only if on the exact spot
                    bRemoveAttrib = true;
                }
            }
            // Attribute overlaps the selection
            else if ( ( pAttr->GetStart() <= nStart ) && ( pAttr->GetEnd() >= nEnd ) )
            {
                bChanged = true;
                if ( pAttr->GetStart() == nStart )
                {
                    bNeedsSorting = true;
                    pAttr->GetStart() = nEnd;
#if defined(YRS)
                    YRemoveAttrib(m_pYrsSupplier, m_CommentId, *this, GetPos(pNode), pAttr->Which(), nStart, nEnd);
#endif
                    rpStarting = pAttr;
                    if ( nWhich )
                        break;  // There can be further attributes!
                }
                else if ( pAttr->GetEnd() == nEnd )
                {
                    pAttr->GetEnd() = nStart;
#if defined(YRS)
                    YRemoveAttrib(m_pYrsSupplier, m_CommentId, *this, GetPos(pNode), pAttr->Which(), nStart, nEnd);
#endif
                    rpEnding = pAttr;
                    if ( nWhich )
                        break;  // There can be further attributes!
                }
                else // Attribute must be split ...
                {
                    bNeedsSorting = true;
                    sal_Int32 nOldEnd = pAttr->GetEnd();
                    pAttr->GetEnd() = nStart;
                    rpEnding = pAttr;
#if defined(YRS)
                    YRemoveAttrib(m_pYrsSupplier, m_CommentId, *this, GetPos(pNode), pAttr->Which(), nStart, nEnd);
                    {
                    ReplayGuard const g{m_pYrsSupplier};
#endif
                    InsertAttrib( *pAttr->GetItem(), pNode, nEnd, nOldEnd );
#if defined(YRS)
                    }
#endif
                    if ( nWhich )
                        break;  // There can be further attributes!
                }
            }
        }
        if ( bRemoveAttrib )
        {
#if defined(YRS)
            YRemoveAttrib(m_pYrsSupplier, m_CommentId, *this, GetPos(pNode), pAttr->Which(), pAttr->GetStart(), pAttr->GetEnd());
#endif
            DBG_ASSERT( ( pAttr != rpStarting ) && ( pAttr != rpEnding ), "Delete and retain the same attribute?" );
            DBG_ASSERT( !pAttr->IsFeature(), "RemoveAttribs: Remove a feature?!" );
            rAttribs.erase(rAttribs.begin()+nAttr);
        }
        else
        {
            nAttr++;
        }
        pAttr = GetAttrib(rAttribs, nAttr);
    }

    if ( bChanged )
    {
        // char attributes need to be sorted by start again
        if (bNeedsSorting)
            pNode->GetCharAttribs().ResortAttribs();
        SetModified(true);
    }

#if OSL_DEBUG_LEVEL > 0 && !defined NDEBUG
    CharAttribList::DbgCheckAttribs(pNode->GetCharAttribs());
#endif

    return bChanged;
}

void EditDoc::InsertAttrib( const SfxPoolItem& rPoolItem, ContentNode* pNode, sal_Int32 nStart, sal_Int32 nEnd )
{
    // This method no longer checks whether a corresponding attribute already
    // exists at this place!
    EditCharAttrib* pAttrib = MakeCharAttrib( GetItemPool(), rPoolItem, nStart, nEnd );
    assert(pAttrib);
    pNode->GetCharAttribs().InsertAttrib( pAttrib );

    SetModified( true );

#if defined(YRS)
    YInsertAttrib(m_pYrsSupplier, m_CommentId, *this, GetPos(pNode), pAttrib);
#endif
}

void EditDoc::InsertAttrib( ContentNode* pNode, sal_Int32 nStart, sal_Int32 nEnd, const SfxPoolItem& rPoolItem )
{
    if ( nStart != nEnd )
    {
        InsertAttribInSelection( pNode, nStart, nEnd, rPoolItem );
    }
    else
    {
        // Check whether already a new attribute with WhichId exists at this place:
        CharAttribList& rAttrList = pNode->GetCharAttribs();
        EditCharAttrib* pAttr = rAttrList.FindEmptyAttrib( rPoolItem.Which(), nStart );
        if ( pAttr )
        {
            // Remove attribute...
            rAttrList.Remove(pAttr);
        }

        // check whether 'the same' attribute exist at this place.
        pAttr = rAttrList.FindAttrib( rPoolItem.Which(), nStart );
        if ( pAttr )
        {
            if ( pAttr->IsInside( nStart ) )    // split
            {
                // check again if really splitting, or return !
                sal_Int32 nOldEnd = pAttr->GetEnd();
                pAttr->GetEnd() = nStart;
                EditCharAttrib* pNew = MakeCharAttrib( GetItemPool(), *(pAttr->GetItem()), nStart, nOldEnd );
                rAttrList.InsertAttrib(pNew);
            }
            else if ( pAttr->GetEnd() == nStart )
            {
                DBG_ASSERT( !pAttr->IsEmpty(), "Still an empty attribute?" );
                // Check if exactly the same attribute
                if ( *(pAttr->GetItem()) == rPoolItem )
                    return;
            }
        }
        InsertAttrib( rPoolItem, pNode, nStart, nStart );
    }

    SetModified( true );
}

void EditDoc::FindAttribs( ContentNode* pNode, sal_Int32 nStartPos, sal_Int32 nEndPos, SfxItemSet& rCurSet )
{
    assert(pNode);
    DBG_ASSERT( nStartPos <= nEndPos, "Invalid region!" );

    std::size_t nAttr = 0;
    EditCharAttrib* pAttr = GetAttrib( pNode->GetCharAttribs().GetAttribs(), nAttr );
    // No Selection...
    if ( nStartPos == nEndPos )
    {
        while ( pAttr && ( pAttr->GetStart() <= nEndPos) )
        {
            const SfxPoolItem* pItem = nullptr;
            // Attribute is about...
            if ( ( pAttr->GetStart() < nStartPos ) && ( pAttr->GetEnd() > nStartPos ) )
                pItem = pAttr->GetItem();
            // Attribute ending here is not empty
            else if ( ( pAttr->GetStart() < nStartPos ) && ( pAttr->GetEnd() == nStartPos ) )
            {
                if ( !pNode->GetCharAttribs().FindEmptyAttrib( pAttr->GetItem()->Which(), nStartPos ) )
                    pItem = pAttr->GetItem();
            }
            // Attribute ending here is empty
            else if ( ( pAttr->GetStart() == nStartPos ) && ( pAttr->GetEnd() == nStartPos ) )
            {
                pItem = pAttr->GetItem();
            }
            // Attribute starts here
            else if ( ( pAttr->GetStart() == nStartPos ) && ( pAttr->GetEnd() > nStartPos ) )
            {
                if ( nStartPos == 0 )   // special case
                    pItem = pAttr->GetItem();
            }

            if ( pItem )
            {
                sal_uInt16 nWhich = pItem->Which();
                if ( rCurSet.GetItemState( nWhich ) == SfxItemState::DEFAULT )
                {
                    rCurSet.Put( *pItem );
                }
                else if ( rCurSet.GetItemState( nWhich ) == SfxItemState::SET )
                {
                    const SfxPoolItem& rItem = rCurSet.Get( nWhich );
                    if ( rItem != *pItem )
                    {
                        rCurSet.InvalidateItem( nWhich );
                    }
                }
            }
            nAttr++;
            pAttr = GetAttrib( pNode->GetCharAttribs().GetAttribs(), nAttr );
        }
    }
    else    // Selection
    {
        while ( pAttr && ( pAttr->GetStart() < nEndPos) )
        {
            const SfxPoolItem* pItem = nullptr;
            // Attribute is about...
            if ( ( pAttr->GetStart() <= nStartPos ) && ( pAttr->GetEnd() >= nEndPos ) )
                pItem = pAttr->GetItem();
            // Attribute starts right in the middle ...
            else if ( pAttr->GetStart() >= nStartPos )
            {
                // !!! pItem = pAttr->GetItem();
                // PItem is simply not enough, since one for example in case
                // of Shadow, would never find an unequal item, since such a
                // item represents its presence by absence!
                // If (...)
                // It needs to be examined on exactly the same attribute at the
                // break point, which is quite expensive.
                // Since optimization is done when inserting the attributes
                // this case does not appear so fast...
                // So based on the need for speed:
                rCurSet.InvalidateItem( pAttr->GetItem()->Which() );

            }
            // Attribute ends in the middle of it ...
            else if ( pAttr->GetEnd() > nStartPos )
            {
                rCurSet.InvalidateItem( pAttr->GetItem()->Which() );
            }

            if ( pItem )
            {
                sal_uInt16 nWhich = pItem->Which();
                if ( rCurSet.GetItemState( nWhich ) == SfxItemState::DEFAULT )
                {
                    rCurSet.Put( *pItem );
                }
                else if ( rCurSet.GetItemState( nWhich ) == SfxItemState::SET )
                {
                    const SfxPoolItem& rItem = rCurSet.Get( nWhich );
                    if ( rItem != *pItem )
                    {
                        rCurSet.InvalidateItem( nWhich );
                    }
                }
            }
            nAttr++;
            pAttr = GetAttrib( pNode->GetCharAttribs().GetAttribs(), nAttr );
        }
    }
}

void EditDoc::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    bool bOwns = false;
    if (!pWriter)
    {
        pWriter = xmlNewTextWriterFilename("editdoc.xml", 0);
        xmlTextWriterSetIndent(pWriter,1);
        (void)xmlTextWriterSetIndentString(pWriter, BAD_CAST("  "));
        (void)xmlTextWriterStartDocument(pWriter, nullptr, nullptr, nullptr);
        bOwns = true;
    }

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("EditDoc"));
    for (auto const & i : maContents)
    {
        i->dumpAsXml(pWriter);
    }
    (void)xmlTextWriterEndElement(pWriter);

    if (bOwns)
    {
       (void)xmlTextWriterEndDocument(pWriter);
       xmlFreeTextWriter(pWriter);
    }
}

EditEngineItemPool::EditEngineItemPool()
: SfxItemPool(u"EditEngineItemPool"_ustr)
{
    registerItemInfoPackage(getItemInfoPackageEditEngine());
}

EditEngineItemPool::~EditEngineItemPool()
{
    SetSecondaryPool(nullptr);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
