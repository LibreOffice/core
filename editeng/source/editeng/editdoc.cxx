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
    SAL_WARN_IF(!maLineList.Count(), "editeng", "Empty ParaPortion in GetLine!");
    DBG_ASSERT(mbVisible, "Why GetLine() on an invisible paragraph?");

    for ( sal_Int32 nLine = 0; nLine < maLineList.Count(); nLine++ )
    {
        if (maLineList[nLine].IsIn(nIndex))
            return nLine;
    }

    // Then it should be at the end of the last line!
    DBG_ASSERT(nIndex == maLineList[maLineList.Count() - 1].GetEnd(), "Index dead wrong!");
    return (maLineList.Count() - 1);
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
    return EE_PARA_NOT_FOUND;
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
    return EE_PARA_NOT_FOUND;
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
            rItem.SetTextFirstLineOffset( sal::static_int_cast< short >( OutputDevice::LogicToLogic( rItem.GetTextFirstLineOffset(), eSourceUnit, eDestUnit ) ) );
            rItem.SetTextLeft( OutputDevice::LogicToLogic( rItem.GetTextLeft(), eSourceUnit, eDestUnit ) );
            rItem.SetRight( OutputDevice::LogicToLogic( rItem.GetRight(), eSourceUnit, eDestUnit ) );
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
    // Don't create an empty node, Clear() will be called in EditEngine-CTOR
};

EditDoc::~EditDoc()
{
    maContents.clear();
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
}

void EditDoc::Remove(sal_Int32 nPos)
{
    if (nPos < 0 || o3tl::make_unsigned(nPos) >= maContents.size())
    {
        SAL_WARN( "editeng", "EditDoc::Remove - out of bounds pos " << nPos);
        return;
    }
    maContents.erase(maContents.begin() + nPos);
}

std::unique_ptr<ContentNode> EditDoc::Release(sal_Int32 nPos)
{
    if (nPos < 0 || o3tl::make_unsigned(nPos) >= maContents.size())
    {
        SAL_WARN( "editeng", "EditDoc::Release - out of bounds pos " << nPos);
        return nullptr;
    }

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

    ContentNode* pNode = new ContentNode(GetItemPool());
    Insert(0, std::unique_ptr<ContentNode>(pNode));

    pNode->SetStyleSheet(pPrevStyle, false);
    pNode->GetContentAttribs().GetItems().Set( aPrevSet );
    pNode->GetCharAttribs().GetDefFont() = aPrevFont;

    SetModified(true);

    return EditPaM(pNode, 0);
}

EditPaM EditDoc::InsertText( EditPaM aPaM, std::u16string_view rStr )
{
    DBG_ASSERT( rStr.find( 0x0A ) == std::u16string_view::npos, "EditDoc::InsertText: Newlines prohibited in paragraph!" );
    DBG_ASSERT( rStr.find( 0x0D ) == std::u16string_view::npos, "EditDoc::InsertText: Newlines prohibited in paragraph!" );
    DBG_ASSERT( rStr.find( '\t' ) == std::u16string_view::npos, "EditDoc::InsertText: Newlines prohibited in paragraph!" );
    assert(aPaM.GetNode());

    aPaM.GetNode()->Insert( rStr, aPaM.GetIndex() );
    aPaM.GetNode()->ExpandAttribs( aPaM.GetIndex(), rStr.size() );
    aPaM.SetIndex( aPaM.GetIndex() + rStr.size() );

    SetModified( true );

    return aPaM;
}

EditPaM EditDoc::InsertParaBreak( EditPaM aPaM, bool bKeepEndingAttribs )
{
    assert(aPaM.GetNode());
    ContentNode* pCurNode = aPaM.GetNode();
    sal_Int32 nPos = GetPos( pCurNode );
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

    Insert(nPos+1, std::unique_ptr<ContentNode>(pNode));

    SetModified(true);

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
    Remove( nRight );

    SetModified(true);

    return aPaM;
}

void EditDoc::RemoveChars( EditPaM aPaM, sal_Int32 nChars )
{
    // Maybe remove Features!
    aPaM.GetNode()->Erase( aPaM.GetIndex(), nChars );
    aPaM.GetNode()->CollapseAttribs( aPaM.GetIndex(), nChars );

    SetModified( true );
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
    }
    else if ( pStartingAttrib && ( *(pStartingAttrib->GetItem()) == rPoolItem ) )
        pStartingAttrib->GetStart() = nStart;
    else if ( pEndingAttrib && ( *(pEndingAttrib->GetItem()) == rPoolItem ) )
        pEndingAttrib->GetEnd() = nEnd;
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
                    rpStarting = pAttr;
                    if ( nWhich )
                        break;  // There can be further attributes!
                }
                else if ( pAttr->GetEnd() == nEnd )
                {
                    pAttr->GetEnd() = nStart;
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
                    InsertAttrib( *pAttr->GetItem(), pNode, nEnd, nOldEnd );
                    if ( nWhich )
                        break;  // There can be further attributes!
                }
            }
        }
        if ( bRemoveAttrib )
        {
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
