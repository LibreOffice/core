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
#include <editeng/xmlcnitm.hxx>
#include <editeng/editids.hrc>
#include <editeng/editdata.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/lspcitem.hxx>

#include <editdoc.hxx>
#include "editdbg.hxx"
#include <editeng/eerdll.hxx>
#include <eerdll2.hxx>
#include "impedit.hxx"

#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>
#include <osl/diagnose.h>

#include <svl/grabbagitem.hxx>
#include <tools/stream.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <libxml/xmlwriter.h>

#include <algorithm>
#include <cassert>
#include <limits>
#include <memory>
#include <set>
#include <string_view>

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

const SfxItemInfo aItemInfos[EDITITEMCOUNT] = {
        { SID_ATTR_FRAMEDIRECTION, true },         // EE_PARA_WRITINGDIR
        { 0, true },                               // EE_PARA_XMLATTRIBS
        { SID_ATTR_PARA_HANGPUNCTUATION, true },   // EE_PARA_HANGINGPUNCTUATION
        { SID_ATTR_PARA_FORBIDDEN_RULES, true },   // EE_PARA_FORBIDDENRULES
        { SID_ATTR_PARA_SCRIPTSPACE, true },       // EE_PARA_ASIANCJKSPACING
        { SID_ATTR_NUMBERING_RULE, true },         // EE_PARA_NUMBULL
        { 0, true },                               // EE_PARA_HYPHENATE
        { 0, true },                               // EE_PARA_BULLETSTATE
        { 0, true },                               // EE_PARA_OUTLLRSPACE
        { SID_ATTR_PARA_OUTLLEVEL, true },         // EE_PARA_OUTLLEVEL
        { SID_ATTR_PARA_BULLET, true },            // EE_PARA_BULLET
        { SID_ATTR_LRSPACE, true },                // EE_PARA_LRSPACE
        { SID_ATTR_ULSPACE, true },                // EE_PARA_ULSPACE
        { SID_ATTR_PARA_LINESPACE, true },         // EE_PARA_SBL
        { SID_ATTR_PARA_ADJUST, true },            // EE_PARA_JUST
        { SID_ATTR_TABSTOP, true },                // EE_PARA_TABS
        { SID_ATTR_ALIGN_HOR_JUSTIFY_METHOD, true }, // EE_PARA_JUST_METHOD
        { SID_ATTR_ALIGN_VER_JUSTIFY, true },      // EE_PARA_VER_JUST
        { SID_ATTR_CHAR_COLOR, true },         // EE_CHAR_COLOR
        { SID_ATTR_CHAR_FONT, true },          // EE_CHAR_FONTINFO
        { SID_ATTR_CHAR_FONTHEIGHT, true },    // EE_CHAR_FONTHEIGHT
        { SID_ATTR_CHAR_SCALEWIDTH, true },    // EE_CHAR_FONTWIDTH
        { SID_ATTR_CHAR_WEIGHT, true },        // EE_CHAR_WEIGHT
        { SID_ATTR_CHAR_UNDERLINE, true },     // EE_CHAR_UNDERLINE
        { SID_ATTR_CHAR_STRIKEOUT, true },     // EE_CHAR_STRIKEOUT
        { SID_ATTR_CHAR_POSTURE, true },       // EE_CHAR_ITALIC
        { SID_ATTR_CHAR_CONTOUR, true },       // EE_CHAR_OUTLINE
        { SID_ATTR_CHAR_SHADOWED, true },      // EE_CHAR_SHADOW
        { SID_ATTR_CHAR_ESCAPEMENT, true },    // EE_CHAR_ESCAPEMENT
        { SID_ATTR_CHAR_AUTOKERN, true },      // EE_CHAR_PAIRKERNING
        { SID_ATTR_CHAR_KERNING, true },       // EE_CHAR_KERNING
        { SID_ATTR_CHAR_WORDLINEMODE, true },  // EE_CHAR_WLM
        { SID_ATTR_CHAR_LANGUAGE, true },      // EE_CHAR_LANGUAGE
        { SID_ATTR_CHAR_CJK_LANGUAGE, true },  // EE_CHAR_LANGUAGE_CJK
        { SID_ATTR_CHAR_CTL_LANGUAGE, true },  // EE_CHAR_LANGUAGE_CTL
        { SID_ATTR_CHAR_CJK_FONT, true },      // EE_CHAR_FONTINFO_CJK
        { SID_ATTR_CHAR_CTL_FONT, true },      // EE_CHAR_FONTINFO_CTL
        { SID_ATTR_CHAR_CJK_FONTHEIGHT, true }, // EE_CHAR_FONTHEIGHT_CJK
        { SID_ATTR_CHAR_CTL_FONTHEIGHT, true }, // EE_CHAR_FONTHEIGHT_CTL
        { SID_ATTR_CHAR_CJK_WEIGHT, true },    // EE_CHAR_WEIGHT_CJK
        { SID_ATTR_CHAR_CTL_WEIGHT, true },    // EE_CHAR_WEIGHT_CTL
        { SID_ATTR_CHAR_CJK_POSTURE, true },   // EE_CHAR_ITALIC_CJK
        { SID_ATTR_CHAR_CTL_POSTURE, true },   // EE_CHAR_ITALIC_CTL
        { SID_ATTR_CHAR_EMPHASISMARK, true },  // EE_CHAR_EMPHASISMARK
        { SID_ATTR_CHAR_RELIEF, true },        // EE_CHAR_RELIEF
        { 0, true },                           // EE_CHAR_RUBI_DUMMY
        { 0, true },                           // EE_CHAR_XMLATTRIBS
        { SID_ATTR_CHAR_OVERLINE, true },      // EE_CHAR_OVERLINE
        { SID_ATTR_CHAR_CASEMAP, true },       // EE_CHAR_CASEMAP
        { SID_ATTR_CHAR_GRABBAG, true },       // EE_CHAR_GRABBAG
        { SID_ATTR_CHAR_BACK_COLOR, true },    // EE_CHAR_BKGCOLOR
        { 0, true },                           // EE_FEATURE_TAB
        { 0, true },                           // EE_FEATURE_LINEBR
        { SID_ATTR_CHAR_CHARSETCOLOR, true },  // EE_FEATURE_NOTCONV
        { SID_FIELD, false },                  // EE_FEATURE_FIELD
};

EditCharAttrib* MakeCharAttrib( SfxItemPool& rPool, const SfxPoolItem& rAttr, sal_Int32 nS, sal_Int32 nE )
{
    // Create a new attribute in the pool
    const SfxPoolItem& rNew = rPool.Put( rAttr );

    EditCharAttrib* pNew = nullptr;
    switch( rNew.Which() )
    {
        case EE_CHAR_LANGUAGE:
        case EE_CHAR_LANGUAGE_CJK:
        case EE_CHAR_LANGUAGE_CTL:
        {
            pNew = new EditCharAttribLanguage( static_cast<const SvxLanguageItem&>(rNew), nS, nE );
        }
        break;
        case EE_CHAR_COLOR:
        {
            pNew = new EditCharAttribColor( static_cast<const SvxColorItem&>(rNew), nS, nE );
        }
        break;
        case EE_CHAR_FONTINFO:
        case EE_CHAR_FONTINFO_CJK:
        case EE_CHAR_FONTINFO_CTL:
        {
            pNew = new EditCharAttribFont( static_cast<const SvxFontItem&>(rNew), nS, nE );
        }
        break;
        case EE_CHAR_FONTHEIGHT:
        case EE_CHAR_FONTHEIGHT_CJK:
        case EE_CHAR_FONTHEIGHT_CTL:
        {
            pNew = new EditCharAttribFontHeight( static_cast<const SvxFontHeightItem&>(rNew), nS, nE );
        }
        break;
        case EE_CHAR_FONTWIDTH:
        {
            pNew = new EditCharAttribFontWidth( static_cast<const SvxCharScaleWidthItem&>(rNew), nS, nE );
        }
        break;
        case EE_CHAR_WEIGHT:
        case EE_CHAR_WEIGHT_CJK:
        case EE_CHAR_WEIGHT_CTL:
        {
            pNew = new EditCharAttribWeight( static_cast<const SvxWeightItem&>(rNew), nS, nE );
        }
        break;
        case EE_CHAR_UNDERLINE:
        {
            pNew = new EditCharAttribUnderline( static_cast<const SvxUnderlineItem&>(rNew), nS, nE );
        }
        break;
        case EE_CHAR_OVERLINE:
        {
            pNew = new EditCharAttribOverline( static_cast<const SvxOverlineItem&>(rNew), nS, nE );
        }
        break;
        case EE_CHAR_EMPHASISMARK:
        {
            pNew = new EditCharAttribEmphasisMark( static_cast<const SvxEmphasisMarkItem&>(rNew), nS, nE );
        }
        break;
        case EE_CHAR_RELIEF:
        {
            pNew = new EditCharAttribRelief( static_cast<const SvxCharReliefItem&>(rNew), nS, nE );
        }
        break;
        case EE_CHAR_STRIKEOUT:
        {
            pNew = new EditCharAttribStrikeout( static_cast<const SvxCrossedOutItem&>(rNew), nS, nE );
        }
        break;
        case EE_CHAR_ITALIC:
        case EE_CHAR_ITALIC_CJK:
        case EE_CHAR_ITALIC_CTL:
        {
            pNew = new EditCharAttribItalic( static_cast<const SvxPostureItem&>(rNew), nS, nE );
        }
        break;
        case EE_CHAR_OUTLINE:
        {
            pNew = new EditCharAttribOutline( static_cast<const SvxContourItem&>(rNew), nS, nE );
        }
        break;
        case EE_CHAR_SHADOW:
        {
            pNew = new EditCharAttribShadow( static_cast<const SvxShadowedItem&>(rNew), nS, nE );
        }
        break;
        case EE_CHAR_ESCAPEMENT:
        {
            pNew = new EditCharAttribEscapement( static_cast<const SvxEscapementItem&>(rNew), nS, nE );
        }
        break;
        case EE_CHAR_PAIRKERNING:
        {
            pNew = new EditCharAttribPairKerning( static_cast<const SvxAutoKernItem&>(rNew), nS, nE );
        }
        break;
        case EE_CHAR_KERNING:
        {
            pNew = new EditCharAttribKerning( static_cast<const SvxKerningItem&>(rNew), nS, nE );
        }
        break;
        case EE_CHAR_WLM:
        {
            pNew = new EditCharAttribWordLineMode( static_cast<const SvxWordLineModeItem&>(rNew), nS, nE );
        }
        break;
        case EE_CHAR_XMLATTRIBS:
        {
            pNew = new EditCharAttrib( rNew, nS, nE );  // Attribute is only for holding XML information...
        }
        break;
        case EE_CHAR_CASEMAP:
        {
            pNew = new EditCharAttribCaseMap( static_cast<const SvxCaseMapItem&>(rNew), nS, nE );
        }
        break;
        case EE_CHAR_GRABBAG:
        {
            pNew = new EditCharAttribGrabBag( static_cast<const SfxGrabBagItem&>(rNew), nS, nE );
        }
        break;
        case EE_FEATURE_TAB:
        {
            pNew = new EditCharAttribTab( static_cast<const SfxVoidItem&>(rNew), nS );
        }
        break;
        case EE_FEATURE_LINEBR:
        {
            pNew = new EditCharAttribLineBreak( static_cast<const SfxVoidItem&>(rNew), nS );
        }
        break;
        case EE_FEATURE_FIELD:
        {
            pNew = new EditCharAttribField( static_cast<const SvxFieldItem&>(rNew), nS );
        }
        break;
        case EE_CHAR_BKGCOLOR:
        {
            pNew = new EditCharAttribBackgroundColor( static_cast<const SvxBackgroundColorItem&>(rNew), nS, nE );
        }
        break;
        default:
        {
            OSL_FAIL( "Invalid Attribute!" );
        }
    }
    return pNew;
}

TextPortionList::TextPortionList()
{
}

TextPortionList::~TextPortionList()
{
    Reset();
}

void TextPortionList::Reset()
{
    maPortions.clear();
}

void TextPortionList::DeleteFromPortion(sal_Int32 nDelFrom)
{
    assert((nDelFrom < static_cast<sal_Int32>(maPortions.size())) || ((nDelFrom == 0) && maPortions.empty()));
    PortionsType::iterator it = maPortions.begin();
    std::advance(it, nDelFrom);
    maPortions.erase(it, maPortions.end());
}

sal_Int32 TextPortionList::Count() const
{
    return static_cast<sal_Int32>(maPortions.size());
}

const TextPortion& TextPortionList::operator[](sal_Int32 nPos) const
{
    return *maPortions[nPos].get();
}

TextPortion& TextPortionList::operator[](sal_Int32 nPos)
{
    return *maPortions[nPos].get();
}

void TextPortionList::Append(TextPortion* p)
{
    maPortions.push_back(std::unique_ptr<TextPortion>(p));
}

void TextPortionList::Insert(sal_Int32 nPos, TextPortion* p)
{
    maPortions.insert(maPortions.begin()+nPos, std::unique_ptr<TextPortion>(p));
}

void TextPortionList::Remove(sal_Int32 nPos)
{
    maPortions.erase(maPortions.begin()+nPos);
}

namespace {

class FindTextPortionByAddress
{
    const TextPortion* mp;
public:
    explicit FindTextPortionByAddress(const TextPortion* p) : mp(p) {}
    bool operator() (const std::unique_ptr<TextPortion>& v) const
    {
        return v.get() == mp;
    }
};

}

sal_Int32 TextPortionList::GetPos(const TextPortion* p) const
{
    PortionsType::const_iterator it =
        std::find_if(maPortions.begin(), maPortions.end(), FindTextPortionByAddress(p));

    if (it == maPortions.end())
        return std::numeric_limits<sal_Int32>::max(); // not found.

    return std::distance(maPortions.begin(), it);
}

sal_Int32 TextPortionList::FindPortion(
    sal_Int32 nCharPos, sal_Int32& nPortionStart, bool bPreferStartingPortion) const
{
    // When nCharPos at portion limit, the left portion is found
    sal_Int32 nTmpPos = 0;
    sal_Int32 n = maPortions.size();
    for (sal_Int32 i = 0; i < n; ++i)
    {
        const TextPortion& rPortion = *maPortions[i].get();
        nTmpPos = nTmpPos + rPortion.GetLen();
        if ( nTmpPos >= nCharPos )
        {
            // take this one if we don't prefer the starting portion, or if it's the last one
            if ( ( nTmpPos != nCharPos ) || !bPreferStartingPortion || ( i == n-1 ) )
            {
                nPortionStart = nTmpPos - rPortion.GetLen();
                return i;
            }
        }
    }
    OSL_FAIL( "FindPortion: Not found!" );
    return n - 1;
}

sal_Int32 TextPortionList::GetStartPos(sal_Int32 nPortion)
{
    sal_Int32 nPos = 0;
    for (sal_Int32 i = 0; i < nPortion; ++i)
    {
        const TextPortion& rPortion = *maPortions[i].get();
        nPos = nPos + rPortion.GetLen();
    }
    return nPos;
}

ExtraPortionInfo::ExtraPortionInfo()
: nOrgWidth(0)
, nWidthFullCompression(0)
, nPortionOffsetX(0)
, nMaxCompression100thPercent(0)
, nAsianCompressionTypes(AsianCompressionFlags::Normal)
, bFirstCharIsRightPunktuation(false)
, bCompressed(false)
, lineBreaksList()
{
}

ExtraPortionInfo::~ExtraPortionInfo()
{
}

void ExtraPortionInfo::SaveOrgDXArray( const long* pDXArray, sal_Int32 nLen )
{
    if (pDXArray)
    {
        pOrgDXArray.reset(new long[nLen]);
        memcpy( pOrgDXArray.get(), pDXArray, nLen * sizeof(long) );
    }
    else
        pOrgDXArray.reset();
}

ParaPortion::ParaPortion( ContentNode* pN ) :
    pNode(pN),
    nHeight(0),
    nInvalidPosStart(0),
    nFirstLineOffset(0),
    nBulletX(0),
    nInvalidDiff(0),
    bInvalid(true),
    bSimple(false),
    bVisible(true),
    bForceRepaint(false)
{
}

ParaPortion::~ParaPortion()
{
}

void ParaPortion::MarkInvalid( sal_Int32 nStart, sal_Int32 nDiff )
{
    if ( !bInvalid )
    {
//      nInvalidPosEnd = nStart;    // ??? => CreateLines
        nInvalidPosStart = ( nDiff >= 0 ) ? nStart : ( nStart + nDiff );
        nInvalidDiff = nDiff;
    }
    else
    {
        // Simple tap in succession
        if ( ( nDiff > 0 ) && ( nInvalidDiff > 0 ) &&
             ( ( nInvalidPosStart+nInvalidDiff ) == nStart ) )
        {
            nInvalidDiff = nInvalidDiff + nDiff;
        }
        // Simple delete in succession
        else if ( ( nDiff < 0 ) && ( nInvalidDiff < 0 ) && ( nInvalidPosStart == nStart ) )
        {
            nInvalidPosStart = nInvalidPosStart + nDiff;
            nInvalidDiff = nInvalidDiff + nDiff;
        }
        else
        {
//          nInvalidPosEnd = pNode->Len();
            DBG_ASSERT( ( nDiff >= 0 ) || ( (nStart+nDiff) >= 0 ), "MarkInvalid: Diff out of Range" );
            nInvalidPosStart = std::min( nInvalidPosStart, ( nDiff < 0 ? nStart+nDiff : nDiff ) );
            nInvalidDiff = 0;
            bSimple = false;
        }
    }
    bInvalid = true;
    aScriptInfos.clear();
    aWritingDirectionInfos.clear();
}

void ParaPortion::MarkSelectionInvalid( sal_Int32 nStart )
{
    if ( !bInvalid )
    {
        nInvalidPosStart = nStart;
    }
    else
    {
        nInvalidPosStart = std::min( nInvalidPosStart, nStart );
    }
    nInvalidDiff = 0;
    bInvalid = true;
    bSimple = false;
    aScriptInfos.clear();
    aWritingDirectionInfos.clear();
}

sal_Int32 ParaPortion::GetLineNumber( sal_Int32 nIndex ) const
{
    SAL_WARN_IF( !aLineList.Count(), "editeng", "Empty ParaPortion in GetLine!" );
    DBG_ASSERT( bVisible, "Why GetLine() on an invisible paragraph?" );

    for ( sal_Int32 nLine = 0; nLine < aLineList.Count(); nLine++ )
    {
        if ( aLineList[nLine].IsIn( nIndex ) )
            return nLine;
    }

    // Then it should be at the end of the last line!
    DBG_ASSERT( nIndex == aLineList[ aLineList.Count() - 1 ].GetEnd(), "Index dead wrong!" );
    return (aLineList.Count()-1);
}

void ParaPortion::SetVisible( bool bMakeVisible )
{
    bVisible = bMakeVisible;
}

void ParaPortion::CorrectValuesBehindLastFormattedLine( sal_Int32 nLastFormattedLine )
{
    sal_Int32 nLines = aLineList.Count();
    DBG_ASSERT( nLines, "CorrectPortionNumbersFromLine: Empty Portion?" );
    if ( nLastFormattedLine < ( nLines - 1 ) )
    {
        const EditLine& rLastFormatted = aLineList[ nLastFormattedLine ];
        const EditLine& rUnformatted = aLineList[ nLastFormattedLine+1 ];
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
                EditLine& rLine = aLineList[ nL ];

                rLine.GetStartPortion() = rLine.GetStartPortion() + nPDiff;
                rLine.GetEndPortion() = rLine.GetEndPortion() + nPDiff;

                rLine.GetStart() = rLine.GetStart() + nTDiff;
                rLine.GetEnd() = rLine.GetEnd() + nTDiff;

                rLine.SetValid();
            }
        }
    }
    DBG_ASSERT( aLineList[ aLineList.Count()-1 ].GetEnd() == pNode->Len(), "CorrectLines: The end is not right!" );
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
            return rLastPos = nIdx;

    // XXX "not found" condition for sal_Int32 indexes
    return EE_PARA_NOT_FOUND;
}

}

ParaPortionList::ParaPortionList() : nLastCache( 0 )
{
}

ParaPortionList::~ParaPortionList()
{
}

sal_Int32 ParaPortionList::GetPos(const ParaPortion* p) const
{
    return FastGetPos(maPortions, p, nLastCache);
}

ParaPortion* ParaPortionList::operator [](sal_Int32 nPos)
{
    return 0 <= nPos && nPos < static_cast<sal_Int32>(maPortions.size()) ? maPortions[nPos].get() : nullptr;
}

const ParaPortion* ParaPortionList::operator [](sal_Int32 nPos) const
{
    return 0 <= nPos && nPos < static_cast<sal_Int32>(maPortions.size()) ? maPortions[nPos].get() : nullptr;
}

std::unique_ptr<ParaPortion> ParaPortionList::Release(sal_Int32 nPos)
{
    if (nPos < 0 || static_cast<sal_Int32>(maPortions.size()) <= nPos)
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
    if (nPos < 0 || static_cast<sal_Int32>(maPortions.size()) <= nPos)
    {
        SAL_WARN( "editeng", "ParaPortionList::Remove - out of bounds pos " << nPos);
        return;
    }
    maPortions.erase(maPortions.begin()+nPos);
}

void ParaPortionList::Insert(sal_Int32 nPos, std::unique_ptr<ParaPortion> p)
{
    if (nPos < 0 || static_cast<sal_Int32>(maPortions.size()) < nPos)
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

long ParaPortionList::GetYOffset(const ParaPortion* pPPortion) const
{
    long nHeight = 0;
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

sal_Int32 ParaPortionList::FindParagraph(long nYOffset) const
{
    long nY = 0;
    for (size_t i = 0, n = maPortions.size(); i < n; ++i)
    {
        nY += maPortions[i]->GetHeight(); // should also be correct even in bVisible!
        if ( nY > nYOffset )
            return i <= SAL_MAX_INT32 ? static_cast<sal_Int32>(i) : SAL_MAX_INT32;
    }
    return EE_PARA_NOT_FOUND;
}

const ParaPortion* ParaPortionList::SafeGetObject(sal_Int32 nPos) const
{
    return 0 <= nPos && nPos < static_cast<sal_Int32>(maPortions.size()) ? maPortions[nPos].get() : nullptr;
}

ParaPortion* ParaPortionList::SafeGetObject(sal_Int32 nPos)
{
    return 0 <= nPos && nPos < static_cast<sal_Int32>(maPortions.size()) ? maPortions[nPos].get() : nullptr;
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

ContentAttribsInfo::ContentAttribsInfo( const SfxItemSet& rParaAttribs ) :
        aPrevParaAttribs( rParaAttribs)
{
}

void ContentAttribsInfo::RemoveAllCharAttribsFromPool(SfxItemPool& rPool) const
{
    for (const std::unique_ptr<EditCharAttrib>& rAttrib : aPrevCharAttribs)
        rPool.Remove(*rAttrib->GetItem());
}

void ContentAttribsInfo::AppendCharAttrib(EditCharAttrib* pNew)
{
    aPrevCharAttribs.push_back(std::unique_ptr<EditCharAttrib>(pNew));
}

void ConvertItem( SfxPoolItem& rPoolItem, MapUnit eSourceUnit, MapUnit eDestUnit )
{
    DBG_ASSERT( eSourceUnit != eDestUnit, "ConvertItem - Why?!" );

    switch ( rPoolItem.Which() )
    {
        case EE_PARA_LRSPACE:
        {
            assert(dynamic_cast<const SvxLRSpaceItem *>(&rPoolItem) != nullptr);
            SvxLRSpaceItem& rItem = static_cast<SvxLRSpaceItem&>(rPoolItem);
            rItem.SetTextFirstLineOfst( sal::static_int_cast< short >( OutputDevice::LogicToLogic( rItem.GetTextFirstLineOfst(), eSourceUnit, eDestUnit ) ) );
            rItem.SetTextLeft( OutputDevice::LogicToLogic( rItem.GetTextLeft(), eSourceUnit, eDestUnit ) );
            rItem.SetRight( OutputDevice::LogicToLogic( rItem.GetRight(), eSourceUnit, eDestUnit ) );
        }
        break;
        case EE_PARA_ULSPACE:
        {
            assert(dynamic_cast<const SvxULSpaceItem *>(&rPoolItem) != nullptr);
            SvxULSpaceItem& rItem = static_cast<SvxULSpaceItem&>(rPoolItem);
            rItem.SetUpper( sal::static_int_cast< sal_uInt16 >( OutputDevice::LogicToLogic( rItem.GetUpper(), eSourceUnit, eDestUnit ) ) );
            rItem.SetLower( sal::static_int_cast< sal_uInt16 >( OutputDevice::LogicToLogic( rItem.GetLower(), eSourceUnit, eDestUnit ) ) );
        }
        break;
        case EE_PARA_SBL:
        {
            assert(dynamic_cast<const SvxLineSpacingItem *>(&rPoolItem) != nullptr);
            SvxLineSpacingItem& rItem = static_cast<SvxLineSpacingItem&>(rPoolItem);
            // SetLineHeight changes also eLineSpace!
            if ( rItem.GetLineSpaceRule() == SvxLineSpaceRule::Min )
                rItem.SetLineHeight( sal::static_int_cast< sal_uInt16 >( OutputDevice::LogicToLogic( rItem.GetLineHeight(), eSourceUnit, eDestUnit ) ) );
        }
        break;
        case EE_PARA_TABS:
        {
            assert(dynamic_cast<const SvxTabStopItem *>(&rPoolItem) != nullptr);
            SvxTabStopItem& rItem = static_cast<SvxTabStopItem&>(rPoolItem);
            SvxTabStopItem aNewItem( EE_PARA_TABS );
            for ( sal_uInt16 i = 0; i < rItem.Count(); i++ )
            {
                const SvxTabStop& rTab = rItem[i];
                SvxTabStop aNewStop( OutputDevice::LogicToLogic( rTab.GetTabPos(), eSourceUnit, eDestUnit ), rTab.GetAdjustment(), rTab.GetDecimal(), rTab.GetFill() );
                aNewItem.Insert( aNewStop );
            }
            rItem = aNewItem;
        }
        break;
        case EE_CHAR_FONTHEIGHT:
        case EE_CHAR_FONTHEIGHT_CJK:
        case EE_CHAR_FONTHEIGHT_CTL:
        {
            assert(dynamic_cast<const SvxFontHeightItem *>(&rPoolItem) != nullptr);
            SvxFontHeightItem& rItem = static_cast<SvxFontHeightItem&>(rPoolItem);
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
            sal_uInt16 nW = pSourcePool->GetTrueWhich( nSlot );
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
                ConvertItem( *pItem, eSourceUnit, eDestUnit );
                pItem->SetWhich(nWhich);
                rDest.Put( *pItem );
            }
            else
            {
                std::unique_ptr<SfxPoolItem> pNewItem(rSource.Get( nSourceWhich ).CloneSetWhich(nWhich));
                rDest.Put( *pNewItem );
            }
        }
    }
}

EditLine::EditLine() :
    nTxtWidth(0),
    nStartPosX(0),
    nStart(0),
    nEnd(0),
    nStartPortion(0),   // to be able to tell the difference between a line
                        // without Portions from one with the Portion number 0
    nEndPortion(0),
    nHeight(0),
    nTxtHeight(0),
    nMaxAscent(0),
    bHangingPunctuation(false),
    bInvalid(true)
{
}

EditLine::EditLine( const EditLine& r ) :
    nTxtWidth(0),
    nStartPosX(0),
    nStart(r.nStart),
    nEnd(r.nEnd),
    nStartPortion(r.nStartPortion),
    nEndPortion(r.nEndPortion),
    nHeight(0),
    nTxtHeight(0),
    nMaxAscent(0),
    bHangingPunctuation(r.bHangingPunctuation),
    bInvalid(true)
{
}

EditLine::~EditLine()
{
}


EditLine* EditLine::Clone() const
{
    EditLine* pL = new EditLine;
    pL->aPositions = aPositions;
    pL->nStartPosX      = nStartPosX;
    pL->nStart          = nStart;
    pL->nEnd            = nEnd;
    pL->nStartPortion   = nStartPortion;
    pL->nEndPortion     = nEndPortion;
    pL->nHeight         = nHeight;
    pL->nTxtWidth       = nTxtWidth;
    pL->nTxtHeight      = nTxtHeight;
    pL->nMaxAscent      = nMaxAscent;

    return pL;
}

bool operator == ( const EditLine& r1,  const EditLine& r2  )
{
    if ( r1.nStart != r2.nStart )
        return false;

    if ( r1.nEnd != r2.nEnd )
        return false;

    if ( r1.nStartPortion != r2.nStartPortion )
        return false;

    if ( r1.nEndPortion != r2.nEndPortion )
        return false;

    return true;
}

EditLine& EditLine::operator = ( const EditLine& r )
{
    nEnd = r.nEnd;
    nStart = r.nStart;
    nEndPortion = r.nEndPortion;
    nStartPortion = r.nStartPortion;
    return *this;
}


void EditLine::SetHeight( sal_uInt16 nH, sal_uInt16 nTxtH )
{
    nHeight = nH;
    nTxtHeight = ( nTxtH ? nTxtH : nH );
}

void EditLine::SetStartPosX( long start )
{
    if (start > 0)
        nStartPosX = start;
    else
        nStartPosX = 0;
}

Size EditLine::CalcTextSize( ParaPortion& rParaPortion )
{
    Size aSz;
    Size aTmpSz;

    DBG_ASSERT( rParaPortion.GetTextPortions().Count(), "GetTextSize before CreatePortions !" );

    for ( sal_Int32 n = nStartPortion; n <= nEndPortion; n++ )
    {
        TextPortion& rPortion = rParaPortion.GetTextPortions()[n];
        switch ( rPortion.GetKind() )
        {
            case PortionKind::TEXT:
            case PortionKind::FIELD:
            case PortionKind::HYPHENATOR:
            {
                aTmpSz = rPortion.GetSize();
                aSz.AdjustWidth(aTmpSz.Width() );
                if ( aSz.Height() < aTmpSz.Height() )
                    aSz.setHeight( aTmpSz.Height() );
            }
            break;
            case PortionKind::TAB:
            {
                aSz.AdjustWidth(rPortion.GetSize().Width() );
            }
            break;
            case PortionKind::LINEBREAK: break;
        }
    }

    SetHeight( static_cast<sal_uInt16>(aSz.Height()) );
    return aSz;
}

EditLineList::EditLineList()
{
}

EditLineList::~EditLineList()
{
    Reset();
}

void EditLineList::Reset()
{
    maLines.clear();
}

void EditLineList::DeleteFromLine(sal_Int32 nDelFrom)
{
    assert(nDelFrom <= (static_cast<sal_Int32>(maLines.size()) - 1));
    LinesType::iterator it = maLines.begin();
    std::advance(it, nDelFrom);
    maLines.erase(it, maLines.end());
}

sal_Int32 EditLineList::FindLine(sal_Int32 nChar, bool bInclEnd)
{
    sal_Int32 n = maLines.size();
    for (sal_Int32 i = 0; i < n; ++i)
    {
        const EditLine& rLine = *maLines[i].get();
        if ( (bInclEnd && (rLine.GetEnd() >= nChar)) ||
             (rLine.GetEnd() > nChar) )
        {
            return i;
        }
    }

    DBG_ASSERT( !bInclEnd, "Line not found: FindLine" );
    return n - 1;
}

sal_Int32 EditLineList::Count() const
{
    return maLines.size();
}

const EditLine& EditLineList::operator[](sal_Int32 nPos) const
{
    return *maLines[nPos].get();
}

EditLine& EditLineList::operator[](sal_Int32 nPos)
{
    return *maLines[nPos].get();
}

void EditLineList::Append(EditLine* p)
{
    maLines.push_back(std::unique_ptr<EditLine>(p));
}

void EditLineList::Insert(sal_Int32 nPos, EditLine* p)
{
    maLines.insert(maLines.begin()+nPos, std::unique_ptr<EditLine>(p));
}

EditPaM::EditPaM() : pNode(nullptr), nIndex(0) {}
EditPaM::EditPaM(ContentNode* p, sal_Int32 n) : pNode(p), nIndex(n) {}


void EditPaM::SetNode(ContentNode* p)
{
    pNode = p;
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

EditSelection::EditSelection()
{
}

EditSelection::EditSelection( const EditPaM& rStartAndAnd ) :
    aStartPaM(rStartAndAnd),
    aEndPaM(rStartAndAnd)
{
}

EditSelection::EditSelection( const EditPaM& rStart, const EditPaM& rEnd ) :
    aStartPaM(rStart),
    aEndPaM(rEnd)
{
}

EditSelection& EditSelection::operator = ( const EditPaM& rPaM )
{
    aStartPaM = rPaM;
    aEndPaM = rPaM;
    return *this;
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

bool operator == ( const EditPaM& r1, const EditPaM& r2 )
{
    return ( r1.GetNode() == r2.GetNode() ) &&
           ( r1.GetIndex() == r2.GetIndex() );
}

bool operator != ( const EditPaM& r1, const EditPaM& r2 )
{
    return !( r1 == r2 );
}

ContentNode::ContentNode( SfxItemPool& rPool ) : aContentAttribs( rPool )
{
}

ContentNode::ContentNode( const OUString& rStr, const ContentAttribs& rContentAttribs ) :
    maString(rStr), aContentAttribs(rContentAttribs)
{
}

ContentNode::~ContentNode()
{
}

void ContentNode::ExpandAttribs( sal_Int32 nIndex, sal_Int32 nNew, SfxItemPool& rItemPool )
{
    if ( !nNew )
        return;

#if OSL_DEBUG_LEVEL > 0 && !defined NDEBUG
    CharAttribList::DbgCheckAttribs(aCharAttribList);
#endif

    // Since features are treated differently than normal character attributes,
    // but can also affect the order of the start list.    // In every if ...,  in the next (n) opportunities due to bFeature or
    // an existing special case, must (n-1) opportunities be provided with
    // bResort. The most likely possibility receives no bResort, so that is
    // not sorted anew when all attributes are the same.
    bool bResort = false;
    bool bExpandedEmptyAtIndexNull = false;

    sal_Int32 nAttr = 0;
    CharAttribList::AttribsType& rAttribs = aCharAttribList.GetAttribs();
    EditCharAttrib* pAttrib = GetAttrib(rAttribs, nAttr);
    while ( pAttrib )
    {
        if ( pAttrib->GetEnd() >= nIndex )
        {
            // Move all attributes behind the insertion point...
            if ( pAttrib->GetStart() > nIndex )
            {
                pAttrib->MoveForward( nNew );
            }
            // 0: Expand empty attribute, if at insertion point
            else if ( pAttrib->IsEmpty() )
            {
                // Do not check Index, a empty one could only be there
                // When later checking it anyhow:
                //   Special case: Start == 0; AbsLen == 1, nNew = 1
                // => Expand, because of paragraph break!
                // Start <= nIndex, End >= nIndex => Start=End=nIndex!
//              if ( pAttrib->GetStart() == nIndex )
                pAttrib->Expand( nNew );
                bResort = true;
                if ( pAttrib->GetStart() == 0 )
                    bExpandedEmptyAtIndexNull = true;
            }
            // 1: Attribute starts before, goes to index ...
            else if ( pAttrib->GetEnd() == nIndex ) // Start must be before
            {
                // Only expand when there is no feature
                // and if not in exclude list!
                // Otherwise, a UL will go on until a new ULDB, expanding both
//              if ( !pAttrib->IsFeature() && !rExclList.FindAttrib( pAttrib->Which() ) )
                if ( !pAttrib->IsFeature() && !aCharAttribList.FindEmptyAttrib( pAttrib->Which(), nIndex ) )
                {
                    if ( !pAttrib->IsEdge() )
                        pAttrib->Expand( nNew );
                }
                else
                    bResort = true;
            }
            // 2: Attribute starts before, goes past the Index...
            else if ( ( pAttrib->GetStart() < nIndex ) && ( pAttrib->GetEnd() > nIndex ) )
            {
                DBG_ASSERT( !pAttrib->IsFeature(), "Large Feature?!" );
                pAttrib->Expand( nNew );
            }
            // 3: Attribute starts on index...
            else if ( pAttrib->GetStart() == nIndex )
            {
                if ( pAttrib->IsFeature() )
                {
                    pAttrib->MoveForward( nNew );
                    bResort = true;
                }
                else
                {
                    bool bExpand = false;
                    if ( nIndex == 0 )
                    {
                        bExpand = true;
                        if( bExpandedEmptyAtIndexNull )
                        {
                            // Check if this kind of attribute was empty and expanded here...
                            sal_uInt16 nW = pAttrib->GetItem()->Which();
                            for ( sal_Int32 nA = 0; nA < nAttr; nA++ )
                            {
                                const EditCharAttrib& r = *aCharAttribList.GetAttribs()[nA].get();
                                if ( ( r.GetStart() == 0 ) && ( r.GetItem()->Which() == nW ) )
                                {
                                    bExpand = false;
                                    break;
                                }
                            }

                        }
                    }
                    if ( bExpand )
                    {
                        pAttrib->Expand( nNew );
                        bResort = true;
                    }
                    else
                    {
                        pAttrib->MoveForward( nNew );
                    }
                }
            }
        }

        if ( pAttrib->IsEdge() )
            pAttrib->SetEdge(false);

        DBG_ASSERT( !pAttrib->IsFeature() || ( pAttrib->GetLen() == 1 ), "Expand: FeaturesLen != 1" );

        DBG_ASSERT( pAttrib->GetStart() <= pAttrib->GetEnd(), "Expand: Attribute distorted!" );
        DBG_ASSERT( ( pAttrib->GetEnd() <= Len() ), "Expand: Attribute larger than paragraph!" );
        if ( pAttrib->IsEmpty() )
        {
            OSL_FAIL( "Empty Attribute after ExpandAttribs?" );
            bResort = true;
            rItemPool.Remove( *pAttrib->GetItem() );
            rAttribs.erase(rAttribs.begin()+nAttr);
            --nAttr;
        }
        ++nAttr;
        pAttrib = GetAttrib(rAttribs, nAttr);
    }

    if ( bResort )
        aCharAttribList.ResortAttribs();

    if (mpWrongList)
    {
        bool bSep = ( maString[ nIndex ] == ' ' ) || IsFeature( nIndex );
        mpWrongList->TextInserted( nIndex, nNew, bSep );
    }

#if OSL_DEBUG_LEVEL > 0 && !defined NDEBUG
    CharAttribList::DbgCheckAttribs(aCharAttribList);
#endif
}

void ContentNode::CollapseAttribs( sal_Int32 nIndex, sal_Int32 nDeleted, SfxItemPool& rItemPool )
{
    if ( !nDeleted )
        return;

#if OSL_DEBUG_LEVEL > 0 && !defined NDEBUG
    CharAttribList::DbgCheckAttribs(aCharAttribList);
#endif

    // Since features are treated differently than normal character attributes,
    // but can also affect the order of the start list
    bool bResort = false;
    sal_Int32 nEndChanges = nIndex+nDeleted;

    sal_Int32 nAttr = 0;
    CharAttribList::AttribsType& rAttribs = aCharAttribList.GetAttribs();
    EditCharAttrib* pAttrib = GetAttrib(rAttribs, nAttr);
    while ( pAttrib )
    {
        bool bDelAttr = false;
        if ( pAttrib->GetEnd() >= nIndex )
        {
            // Move all Attribute behind the insert point...
            if ( pAttrib->GetStart() >= nEndChanges )
            {
                pAttrib->MoveBackward( nDeleted );
            }
            // 1. Delete Internal attributes...
            else if ( ( pAttrib->GetStart() >= nIndex ) && ( pAttrib->GetEnd() <= nEndChanges ) )
            {
                // Special case: Attribute covers the area exactly
                // => keep as empty Attribute.
                if ( !pAttrib->IsFeature() && ( pAttrib->GetStart() == nIndex ) && ( pAttrib->GetEnd() == nEndChanges ) )
                {
                    pAttrib->GetEnd() = nIndex; // empty
                    bResort = true;
                }
                else
                    bDelAttr = true;
            }
            // 2. Attribute starts earlier, ends inside or behind it ...
            else if ( ( pAttrib->GetStart() <= nIndex ) && ( pAttrib->GetEnd() > nIndex ) )
            {
                DBG_ASSERT( !pAttrib->IsFeature(), "Collapsing Feature!" );
                if ( pAttrib->GetEnd() <= nEndChanges ) // ends inside
                    pAttrib->GetEnd() = nIndex;
                else
                    pAttrib->Collaps( nDeleted );       // ends behind
            }
            // 3. Attribute starts inside, ending behind ...
            else if ( ( pAttrib->GetStart() >= nIndex ) && ( pAttrib->GetEnd() > nEndChanges ) )
            {
                // Features not allowed to expand!
                if ( pAttrib->IsFeature() )
                {
                    pAttrib->MoveBackward( nDeleted );
                    bResort = true;
                }
                else
                {
                    pAttrib->GetStart() = nEndChanges;
                    pAttrib->MoveBackward( nDeleted );
                }
            }
        }
        DBG_ASSERT( !pAttrib->IsFeature() || ( pAttrib->GetLen() == 1 ), "Expand: FeaturesLen != 1" );

        DBG_ASSERT( pAttrib->GetStart() <= pAttrib->GetEnd(), "Collapse: Attribute distorted!" );
        DBG_ASSERT( ( pAttrib->GetEnd() <= Len()) || bDelAttr, "Collapse: Attribute larger than paragraph!" );
        if ( bDelAttr )
        {
            bResort = true;
            rItemPool.Remove( *pAttrib->GetItem() );
            rAttribs.erase(rAttribs.begin()+nAttr);
            nAttr--;
        }
        else if ( pAttrib->IsEmpty() )
            aCharAttribList.SetHasEmptyAttribs(true);

        nAttr++;
        pAttrib = GetAttrib(rAttribs, nAttr);
    }

    if ( bResort )
        aCharAttribList.ResortAttribs();

    if (mpWrongList)
        mpWrongList->TextDeleted(nIndex, nDeleted);

#if OSL_DEBUG_LEVEL > 0 && !defined NDEBUG
    CharAttribList::DbgCheckAttribs(aCharAttribList);
#endif
}

void ContentNode::CopyAndCutAttribs( ContentNode* pPrevNode, SfxItemPool& rPool, bool bKeepEndingAttribs )
{
    assert(pPrevNode);

#if OSL_DEBUG_LEVEL > 0 && !defined NDEBUG
    CharAttribList::DbgCheckAttribs(aCharAttribList);
    CharAttribList::DbgCheckAttribs(pPrevNode->aCharAttribList);
#endif

    sal_Int32 nCut = pPrevNode->Len();

    sal_Int32 nAttr = 0;
    CharAttribList::AttribsType& rPrevAttribs = pPrevNode->GetCharAttribs().GetAttribs();
    EditCharAttrib* pAttrib = GetAttrib(rPrevAttribs, nAttr);
    while ( pAttrib )
    {
        if ( pAttrib->GetEnd() < nCut )
        {
            // remain unchanged ....
            ;
        }
        else if ( pAttrib->GetEnd() == nCut )
        {
            // must be copied as an empty attributes.
            if ( bKeepEndingAttribs && !pAttrib->IsFeature() && !aCharAttribList.FindAttrib( pAttrib->GetItem()->Which(), 0 ) )
            {
                EditCharAttrib* pNewAttrib = MakeCharAttrib( rPool, *(pAttrib->GetItem()), 0, 0 );
                assert(pNewAttrib);
                aCharAttribList.InsertAttrib( pNewAttrib );
            }
        }
        else if ( pAttrib->IsInside( nCut ) || ( !nCut && !pAttrib->GetStart() && !pAttrib->IsFeature() ) )
        {
            // If cut is done right at the front then the attribute must be
            // kept! Has to be copied and changed.
            EditCharAttrib* pNewAttrib = MakeCharAttrib( rPool, *(pAttrib->GetItem()), 0, pAttrib->GetEnd()-nCut );
            assert(pNewAttrib);
            aCharAttribList.InsertAttrib( pNewAttrib );
            pAttrib->GetEnd() = nCut;
        }
        else
        {
            // Move all attributes in the current node (this)
            CharAttribList::AttribsType::iterator it = rPrevAttribs.begin() + nAttr;
            aCharAttribList.InsertAttrib(it->release());
            rPrevAttribs.erase(it);
            pAttrib->MoveBackward( nCut );
            nAttr--;
        }
        nAttr++;
        pAttrib = GetAttrib(rPrevAttribs, nAttr);
    }

#if OSL_DEBUG_LEVEL > 0 && !defined NDEBUG
    CharAttribList::DbgCheckAttribs(aCharAttribList);
    CharAttribList::DbgCheckAttribs(pPrevNode->aCharAttribList);
#endif
}

void ContentNode::AppendAttribs( ContentNode* pNextNode )
{
    assert(pNextNode);

    sal_Int32 nNewStart = maString.getLength();

#if OSL_DEBUG_LEVEL > 0 && !defined NDEBUG
    CharAttribList::DbgCheckAttribs(aCharAttribList);
    CharAttribList::DbgCheckAttribs(pNextNode->aCharAttribList);
#endif

    sal_Int32 nAttr = 0;
    CharAttribList::AttribsType& rNextAttribs = pNextNode->GetCharAttribs().GetAttribs();
    EditCharAttrib* pAttrib = GetAttrib(rNextAttribs, nAttr);
    while ( pAttrib )
    {
        // Move all attributes in the current node (this)
        bool bMelted = false;
        if ( ( pAttrib->GetStart() == 0 ) && ( !pAttrib->IsFeature() ) )
        {
            // Attributes can possibly be summarized as:
            sal_Int32 nTmpAttr = 0;
            EditCharAttrib* pTmpAttrib = GetAttrib( aCharAttribList.GetAttribs(), nTmpAttr );
            while ( !bMelted && pTmpAttrib )
            {
                if ( pTmpAttrib->GetEnd() == nNewStart )
                {
                    if (pTmpAttrib->Which() == pAttrib->Which())
                    {
                        // prevent adding 2 0-length attributes at same position
                        if ((*(pTmpAttrib->GetItem()) == *(pAttrib->GetItem()))
                                || (0 == pAttrib->GetLen()))
                        {
                            pTmpAttrib->GetEnd() =
                                pTmpAttrib->GetEnd() + pAttrib->GetLen();
                            rNextAttribs.erase(rNextAttribs.begin()+nAttr);
                            // Unsubscribe from the pool?!
                            bMelted = true;
                        }
                        else if (0 == pTmpAttrib->GetLen())
                        {
                            aCharAttribList.Remove(nTmpAttr);
                            --nTmpAttr; // to cancel later increment...
                        }
                    }
                }
                ++nTmpAttr;
                pTmpAttrib = GetAttrib( aCharAttribList.GetAttribs(), nTmpAttr );
            }
        }

        if ( !bMelted )
        {
            pAttrib->GetStart() = pAttrib->GetStart() + nNewStart;
            pAttrib->GetEnd() = pAttrib->GetEnd() + nNewStart;
            CharAttribList::AttribsType::iterator it = rNextAttribs.begin() + nAttr;
            aCharAttribList.InsertAttrib(it->release());
            rNextAttribs.erase(it);
        }
        pAttrib = GetAttrib(rNextAttribs, nAttr);
    }
    // For the Attributes that just moved over:
    rNextAttribs.clear();

#if OSL_DEBUG_LEVEL > 0 && !defined NDEBUG
    CharAttribList::DbgCheckAttribs(aCharAttribList);
    CharAttribList::DbgCheckAttribs(pNextNode->aCharAttribList);
#endif
}

void ContentNode::CreateDefFont()
{
    // First use the information from the style ...
    SfxStyleSheet* pS = aContentAttribs.GetStyleSheet();
    if ( pS )
        CreateFont( GetCharAttribs().GetDefFont(), pS->GetItemSet() );

    // ... then iron out the hard paragraph formatting...
    CreateFont( GetCharAttribs().GetDefFont(),
        GetContentAttribs().GetItems(), pS == nullptr );
}

void ContentNode::SetStyleSheet( SfxStyleSheet* pS, const SvxFont& rFontFromStyle )
{
    aContentAttribs.SetStyleSheet( pS );


    // First use the information from the style ...
    GetCharAttribs().GetDefFont() = rFontFromStyle;
    // ... then iron out the hard paragraph formatting...
    CreateFont( GetCharAttribs().GetDefFont(),
                GetContentAttribs().GetItems(), pS == nullptr );
}

void ContentNode::SetStyleSheet( SfxStyleSheet* pS, bool bRecalcFont )
{
    aContentAttribs.SetStyleSheet( pS );
    if ( bRecalcFont )
        CreateDefFont();
}

bool ContentNode::IsFeature( sal_Int32 nPos ) const
{
    return maString[nPos] == CH_FEATURE;
}

sal_Int32 ContentNode::Len() const
{
    return maString.getLength();
}

sal_uLong ContentNode::GetExpandedLen() const
{
    sal_uLong nLen = maString.getLength();

    // Fields can be longer than the placeholder in the Node
    const CharAttribList::AttribsType& rAttrs = GetCharAttribs().GetAttribs();
    for (sal_Int32 nAttr = rAttrs.size(); nAttr; )
    {
        const EditCharAttrib& rAttr = *rAttrs[--nAttr].get();
        if (rAttr.Which() == EE_FEATURE_FIELD)
        {
            nLen += static_cast<const EditCharAttribField&>(rAttr).GetFieldValue().getLength();
            --nLen; // Standalone, to avoid corner cases when previous getLength() returns 0
        }
    }

    return nLen;
}

OUString ContentNode::GetExpandedText(sal_Int32 nStartPos, sal_Int32 nEndPos) const
{
    if ( nEndPos < 0 || nEndPos > Len() )
        nEndPos = Len();

    DBG_ASSERT( nStartPos <= nEndPos, "Start and End reversed?" );

    sal_Int32 nIndex = nStartPos;
    OUStringBuffer aStr;
    const EditCharAttrib* pNextFeature = GetCharAttribs().FindFeature( nIndex );
    while ( nIndex < nEndPos )
    {
        sal_Int32 nEnd = nEndPos;
        if ( pNextFeature && ( pNextFeature->GetStart() < nEnd ) )
            nEnd = pNextFeature->GetStart();
        else
            pNextFeature = nullptr;   // Feature does not interest the below

        DBG_ASSERT( nEnd >= nIndex, "End in front of the index?" );
        //!! beware of sub string length  of -1
        if (nEnd > nIndex)
            aStr.append( std::u16string_view(GetString()).substr(nIndex, nEnd - nIndex) );

        if ( pNextFeature )
        {
            switch ( pNextFeature->GetItem()->Which() )
            {
                case EE_FEATURE_TAB:    aStr.append( "\t" );
                break;
                case EE_FEATURE_LINEBR: aStr.append( "\x0A" );
                break;
                case EE_FEATURE_FIELD:
                    aStr.append( static_cast<const EditCharAttribField*>(pNextFeature)->GetFieldValue() );
                break;
                default:    OSL_FAIL( "What feature?" );
            }
            pNextFeature = GetCharAttribs().FindFeature( ++nEnd );
        }
        nIndex = nEnd;
    }
    return aStr.makeStringAndClear();
}

void ContentNode::UnExpandPosition( sal_Int32 &rPos, bool bBiasStart )
{
    sal_Int32 nOffset = 0;

    const CharAttribList::AttribsType& rAttrs = GetCharAttribs().GetAttribs();
    for (size_t nAttr = 0; nAttr < rAttrs.size(); ++nAttr )
    {
        const EditCharAttrib& rAttr = *rAttrs[nAttr].get();
        assert (!(nAttr < rAttrs.size() - 1) ||
                rAttrs[nAttr]->GetStart() <= rAttrs[nAttr + 1]->GetStart());

        nOffset = rAttr.GetStart();

        if (nOffset >= rPos) // happens after the position
            return;

        sal_Int32 nChunk = 0;
        if (rAttr.Which() == EE_FEATURE_FIELD)
        {
            nChunk += static_cast<const EditCharAttribField&>(rAttr).GetFieldValue().getLength();
            nChunk--; // Character representing the field in the string

            if (nOffset + nChunk >= rPos) // we're inside the field
            {
                if (bBiasStart)
                    rPos = rAttr.GetStart();
                else
                    rPos = rAttr.GetEnd();
                return;
            }
            // Adjust for the position
            rPos -= nChunk;
        }
    }
    assert (rPos <= Len());
}

/*
 * Fields are represented by a single character in the underlying string
 * and/or selection, however, they can be expanded to the full value of
 * the field. When we're dealing with selection / offsets however we need
 * to deal in character positions inside the real (unexpanded) string.
 * This method maps us back to character offsets.
 */
void ContentNode::UnExpandPositions( sal_Int32 &rStartPos, sal_Int32 &rEndPos )
{
    UnExpandPosition( rStartPos, true );
    UnExpandPosition( rEndPos, false );
}

void ContentNode::SetChar(sal_Int32 nPos, sal_Unicode c)
{
    maString = maString.replaceAt(nPos, 1, OUString(c));
}

void ContentNode::Insert(const OUString& rStr, sal_Int32 nPos)
{
    maString = maString.replaceAt(nPos, 0, rStr);
}

void ContentNode::Append(const OUString& rStr)
{
    maString += rStr;
}

void ContentNode::Erase(sal_Int32 nPos)
{
    maString = maString.copy(0, nPos);
}

void ContentNode::Erase(sal_Int32 nPos, sal_Int32 nCount)
{
    maString = maString.replaceAt(nPos, nCount, "");
}

OUString ContentNode::Copy(sal_Int32 nPos) const
{
    return maString.copy(nPos);
}

OUString ContentNode::Copy(sal_Int32 nPos, sal_Int32 nCount) const
{
    return maString.copy(nPos, nCount);
}

sal_Unicode ContentNode::GetChar(sal_Int32 nPos) const
{
    return maString[nPos];
}

void ContentNode::EnsureWrongList()
{
    if (!mpWrongList)
        CreateWrongList();
}

WrongList* ContentNode::GetWrongList()
{
    return mpWrongList.get();
}

const WrongList* ContentNode::GetWrongList() const
{
    return mpWrongList.get();
}

void ContentNode::SetWrongList( WrongList* p )
{
    mpWrongList.reset(p);
}

void ContentNode::CreateWrongList()
{
    SAL_WARN_IF( mpWrongList && !mpWrongList->empty(), "editeng", "WrongList already exist!");
    if (!mpWrongList || !mpWrongList->empty())
        mpWrongList.reset(new WrongList);
}

void ContentNode::DestroyWrongList()
{
    mpWrongList.reset();
}

void ContentNode::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("ContentNode"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("maString"), BAD_CAST(maString.toUtf8().getStr()));
    aContentAttribs.dumpAsXml(pWriter);
    aCharAttribList.dumpAsXml(pWriter);
    xmlTextWriterEndElement(pWriter);
}


ContentAttribs::ContentAttribs( SfxItemPool& rPool )
: pStyle(nullptr)
, aAttribSet( rPool, svl::Items<EE_PARA_START, EE_CHAR_END>{} )
{
}


SvxTabStop ContentAttribs::FindTabStop( sal_Int32 nCurPos, sal_uInt16 nDefTab )
{
    const SvxTabStopItem& rTabs = GetItem( EE_PARA_TABS );
    for ( sal_uInt16 i = 0; i < rTabs.Count(); i++ )
    {
        const SvxTabStop& rTab = rTabs[i];
        if ( rTab.GetTabPos() > nCurPos  )
            return rTab;
    }

    // Determine DefTab ...
    SvxTabStop aTabStop;
    const sal_Int32 x = nCurPos / nDefTab + 1;
    aTabStop.GetTabPos() = nDefTab * x;
    return aTabStop;
}

void ContentAttribs::SetStyleSheet( SfxStyleSheet* pS )
{
    bool bStyleChanged = ( pStyle != pS );
    pStyle = pS;
    // Only when other style sheet, not when current style sheet modified
    if ( pStyle && bStyleChanged )
    {
        // Selectively remove the attributes from the paragraph formatting
        // which are specified in the style, so that the attributes of the
        // style can have an affect.
        const SfxItemSet& rStyleAttribs = pStyle->GetItemSet();
        for ( sal_uInt16 nWhich = EE_PARA_START; nWhich <= EE_CHAR_END; nWhich++ )
        {
            // Don't change bullet on/off
            if ( ( nWhich != EE_PARA_BULLETSTATE ) && ( rStyleAttribs.GetItemState( nWhich ) == SfxItemState::SET ) )
                aAttribSet.ClearItem( nWhich );
        }
    }
}

const SfxPoolItem& ContentAttribs::GetItem( sal_uInt16 nWhich ) const
{
    // Hard paragraph attributes take precedence!
    const SfxItemSet* pTakeFrom = &aAttribSet;
    if ( pStyle && ( aAttribSet.GetItemState( nWhich, false ) != SfxItemState::SET  ) )
        pTakeFrom = &pStyle->GetItemSet();

    return pTakeFrom->Get( nWhich );
}

bool ContentAttribs::HasItem( sal_uInt16 nWhich ) const
{
    bool bHasItem = false;
    if ( aAttribSet.GetItemState( nWhich, false ) == SfxItemState::SET  )
        bHasItem = true;
    else if ( pStyle && pStyle->GetItemSet().GetItemState( nWhich ) == SfxItemState::SET )
        bHasItem = true;

    return bHasItem;
}

void ContentAttribs::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("ContentAttribs"));
    xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST("style"), "%s", pStyle->GetName().toUtf8().getStr());
    aAttribSet.dumpAsXml(pWriter);
    xmlTextWriterEndElement(pWriter);
}


ItemList::ItemList() : CurrentItem( 0 )
{
}

const SfxPoolItem* ItemList::First()
{
    CurrentItem = 0;
    return aItemPool.empty() ? nullptr : aItemPool[ 0 ];
}

const SfxPoolItem* ItemList::Next()
{
    if ( CurrentItem + 1 < static_cast<sal_Int32>(aItemPool.size()) )
    {
        ++CurrentItem;
        return aItemPool[ CurrentItem ];
    }
    return nullptr;
}

void ItemList::Insert( const SfxPoolItem* pItem )
{
    aItemPool.push_back( pItem );
    CurrentItem = aItemPool.size() - 1;
}


EditDoc::EditDoc( SfxItemPool* pPool ) :
    nLastCache(0),
    pItemPool(pPool ? pPool : new EditEngineItemPool()),
    nDefTab(DEFTAB),
    bIsVertical(false),
    bIsTopToBottomVert(false),
    bIsFixedCellHeight(false),
    bOwnerOfPool(pPool == nullptr),
    bModified(false)
{
    // Don't create a empty node, Clear() will be called in EditEngine-CTOR
};

EditDoc::~EditDoc()
{
    ImplDestroyContents();
    if ( bOwnerOfPool )
        SfxItemPool::Free(pItemPool);
}

namespace {

class RemoveEachItemFromPool
{
    EditDoc& mrDoc;
public:
    explicit RemoveEachItemFromPool(EditDoc& rDoc) : mrDoc(rDoc) {}
    void operator() (const std::unique_ptr<ContentNode>& rNode)
    {
        mrDoc.RemoveItemsFromPool(*rNode);
    }
};

struct ClearSpellErrorsHandler
{
    void operator() (std::unique_ptr<ContentNode> const & rNode)
    {
        rNode->DestroyWrongList();
    }
};

}

void EditDoc::ImplDestroyContents()
{
    std::for_each(maContents.begin(), maContents.end(), RemoveEachItemFromPool(*this));
    maContents.clear();
}

void EditDoc::RemoveItemsFromPool(const ContentNode& rNode)
{
    for (sal_Int32 nAttr = 0; nAttr < rNode.GetCharAttribs().Count(); ++nAttr)
    {
        const EditCharAttrib& rAttr = *rNode.GetCharAttribs().GetAttribs()[nAttr].get();
        GetItemPool().Remove(*rAttr.GetItem());
    }
}

void CreateFont( SvxFont& rFont, const SfxItemSet& rSet, bool bSearchInParent, SvtScriptType nScriptType )
{
    vcl::Font aPrevFont( rFont );
    rFont.SetAlignment( ALIGN_BASELINE );
    rFont.SetTransparent( true );

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
        rFont.SetFillColor( rSet.Get( EE_CHAR_BKGCOLOR ).GetValue() );
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
        if ( nEsc == DFLT_ESC_AUTO_SUPER )
            nEsc = 100 - nProp;
        else if ( nEsc == DFLT_ESC_AUTO_SUB )
            nEsc = sal::static_int_cast< short >( -( 100 - nProp ) );
        rFont.SetEscapement( nEsc );
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

    // If comparing the entire font, or if checking before each alteration
    // whether the value changes, remains relatively the same thing.
    // So possible one MakeUniqFont more in the font, but as a result a quicker
    // abortion of the query, or one must each time check bChanged.
    if ( rFont == aPrevFont  )
        rFont = aPrevFont;  // => The same ImpPointer for IsSameInstance
}

void EditDoc::CreateDefFont( bool bUseStyles )
{
    SfxItemSet aTmpSet( GetItemPool(), svl::Items<EE_PARA_START, EE_CHAR_END>{} );
    CreateFont( aDefFont, aTmpSet );
    aDefFont.SetVertical( IsVertical() );
    aDefFont.SetOrientation( IsVertical() ? (IsTopToBottom() ? 2700 : 900) : 0 );

    for ( sal_Int32 nNode = 0; nNode < Count(); nNode++ )
    {
        ContentNode* pNode = GetObject( nNode );
        pNode->GetCharAttribs().GetDefFont() = aDefFont;
        if ( bUseStyles )
            pNode->CreateDefFont();
    }
}

sal_Int32 EditDoc::GetPos(const ContentNode* p) const
{
    return FastGetPos(maContents, p, nLastCache);
}

const ContentNode* EditDoc::GetObject(sal_Int32 nPos) const
{
    return 0 <= nPos && nPos < static_cast<sal_Int32>(maContents.size()) ? maContents[nPos].get() : nullptr;
}

ContentNode* EditDoc::GetObject(sal_Int32 nPos)
{
    return 0 <= nPos && nPos < static_cast<sal_Int32>(maContents.size()) ? maContents[nPos].get() : nullptr;
}

const ContentNode* EditDoc::operator[](sal_Int32 nPos) const
{
    return GetObject(nPos);
}

ContentNode* EditDoc::operator[](sal_Int32 nPos)
{
    return GetObject(nPos);
}

void EditDoc::Insert(sal_Int32 nPos, ContentNode* p)
{
    if (nPos < 0 || nPos == SAL_MAX_INT32)
    {
        SAL_WARN( "editeng", "EditDoc::Insert - overflow pos " << nPos);
        return;
    }
    maContents.insert(maContents.begin()+nPos, std::unique_ptr<ContentNode>(p));
}

void EditDoc::Remove(sal_Int32 nPos)
{
    if (nPos < 0 || nPos >= static_cast<sal_Int32>(maContents.size()))
    {
        SAL_WARN( "editeng", "EditDoc::Remove - out of bounds pos " << nPos);
        return;
    }
    maContents.erase(maContents.begin() + nPos);
}

void EditDoc::Release(sal_Int32 nPos)
{
    if (nPos < 0 || nPos >= static_cast<sal_Int32>(maContents.size()))
    {
        SAL_WARN( "editeng", "EditDoc::Release - out of bounds pos " << nPos);
        return;
    }
    (void)maContents[nPos].release();
    maContents.erase(maContents.begin() + nPos);
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
        return OUString("\015"); // 0x0d
    if ( eEnd == LINEEND_LF )
        return OUString("\012"); // 0x0a
    return OUString("\015\012"); // 0x0d, 0x0a
}

OUString EditDoc::GetText( LineEnd eEnd ) const
{
    const sal_Int32 nNodes = Count();
    if (nNodes == 0)
        return OUString();

    const OUString aSep = EditDoc::GetSepStr( eEnd );
    const sal_Int32 nSepSize = aSep.getLength();
    const sal_uInt32 nLen = GetTextLen() + (nNodes - 1)*nSepSize;

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

sal_uLong EditDoc::GetTextLen() const
{
    sal_uLong nLen = 0;
    for ( sal_Int32 nNode = 0; nNode < Count(); nNode++ )
    {
        const ContentNode* pNode = GetObject( nNode );
        nLen += pNode->GetExpandedLen();
    }
    return nLen;
}

EditPaM EditDoc::Clear()
{
    ImplDestroyContents();

    ContentNode* pNode = new ContentNode( GetItemPool() );
    Insert(0, pNode);

    CreateDefFont(false);

    SetModified(false);

    return EditPaM( pNode, 0 );
}

void EditDoc::ClearSpellErrors()
{
    std::for_each(maContents.begin(), maContents.end(), ClearSpellErrorsHandler());
}

void EditDoc::SetModified( bool b )
{
    bModified = b;
    if ( bModified )
    {
        aModifyHdl.Call( nullptr );
    }
}

EditPaM EditDoc::RemoveText()
{
    // Keep the old ItemSet, to keep the chart Font.
    ContentNode* pPrevFirstNode = GetObject(0);
    SfxStyleSheet* pPrevStyle = pPrevFirstNode->GetStyleSheet();
    SfxItemSet aPrevSet( pPrevFirstNode->GetContentAttribs().GetItems() );
    vcl::Font aPrevFont( pPrevFirstNode->GetCharAttribs().GetDefFont() );

    ImplDestroyContents();

    ContentNode* pNode = new ContentNode( GetItemPool() );
    Insert(0, pNode);

    pNode->SetStyleSheet(pPrevStyle, false);
    pNode->GetContentAttribs().GetItems().Set( aPrevSet );
    pNode->GetCharAttribs().GetDefFont() = aPrevFont;

    SetModified(true);

    return EditPaM( pNode, 0 );
}

EditPaM EditDoc::InsertText( EditPaM aPaM, const OUString& rStr )
{
    DBG_ASSERT( rStr.indexOf( 0x0A ) == -1, "EditDoc::InsertText: Newlines prohibited in paragraph!" );
    DBG_ASSERT( rStr.indexOf( 0x0D ) == -1, "EditDoc::InsertText: Newlines prohibited in paragraph!" );
    DBG_ASSERT( rStr.indexOf( '\t' ) == -1, "EditDoc::InsertText: Newlines prohibited in paragraph!" );
    assert(aPaM.GetNode());

    aPaM.GetNode()->Insert( rStr, aPaM.GetIndex() );
    aPaM.GetNode()->ExpandAttribs( aPaM.GetIndex(), rStr.getLength(), GetItemPool() );
    aPaM.SetIndex( aPaM.GetIndex() + rStr.getLength() );

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
    ContentNode* pNode = new ContentNode( aStr, aContentAttribs );

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

    Insert(nPos+1, pNode);

    SetModified(true);

    aPaM.SetNode( pNode );
    aPaM.SetIndex( 0 );
    return aPaM;
}

EditPaM EditDoc::InsertFeature( EditPaM aPaM, const SfxPoolItem& rItem  )
{
    assert(aPaM.GetNode());

    aPaM.GetNode()->Insert( OUString(CH_FEATURE), aPaM.GetIndex() );
    aPaM.GetNode()->ExpandAttribs( aPaM.GetIndex(), 1, GetItemPool() );

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
    RemoveItemsFromPool(*pRight);
    sal_Int32 nRight = GetPos( pRight );
    Remove( nRight );

    SetModified(true);

    return aPaM;
}

void EditDoc::RemoveChars( EditPaM aPaM, sal_Int32 nChars )
{
    // Maybe remove Features!
    aPaM.GetNode()->Erase( aPaM.GetIndex(), nChars );
    aPaM.GetNode()->CollapseAttribs( aPaM.GetIndex(), nChars, GetItemPool() );

    SetModified( true );
}

void EditDoc::InsertAttribInSelection( ContentNode* pNode, sal_Int32 nStart, sal_Int32 nEnd, const SfxPoolItem& rPoolItem )
{
    assert(pNode);
    DBG_ASSERT( nEnd <= pNode->Len(), "InsertAttrib: Attribute to large!" );

    // for Optimization:
    // This ends at the beginning of the selection => can be expanded
    EditCharAttrib* pEndingAttrib = nullptr;
    // This starts at the end of the selection => can be expanded
    EditCharAttrib* pStartingAttrib = nullptr;

    DBG_ASSERT( nStart <= nEnd, "Small miscalculations in InsertAttribInSelection" );

    RemoveAttribs( pNode, nStart, nEnd, pStartingAttrib, pEndingAttrib, rPoolItem.Which() );

    if ( pStartingAttrib && pEndingAttrib &&
         ( *(pStartingAttrib->GetItem()) == rPoolItem ) &&
         ( *(pEndingAttrib->GetItem()) == rPoolItem ) )
    {
        // Will become a large Attribute.
        pEndingAttrib->GetEnd() = pStartingAttrib->GetEnd();
        GetItemPool().Remove( *(pStartingAttrib->GetItem()) );
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
    DBG_ASSERT( nEnd <= pNode->Len(), "InsertAttrib: Attribute to large!" );

    // This ends at the beginning of the selection => can be expanded
    rpEnding = nullptr;
    // This starts at the end of the selection => can be expanded
    rpStarting = nullptr;

    bool bChanged = false;

    DBG_ASSERT( nStart <= nEnd, "Small miscalculations in InsertAttribInSelection" );

#if OSL_DEBUG_LEVEL > 0 && !defined NDEBUG
    CharAttribList::DbgCheckAttribs(pNode->GetCharAttribs());
#endif

    // iterate over the attributes ...
    sal_Int32 nAttr = 0;
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
            GetItemPool().Remove( *pAttr->GetItem() );
            rAttribs.erase(rAttribs.begin()+nAttr);
            nAttr--;
        }
        nAttr++;
        pAttr = GetAttrib(rAttribs, nAttr);
    }

    if ( bChanged )
    {
        // char attributes need to be sorted by start again
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
            // Remove attribute....
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

    sal_uInt16 nAttr = 0;
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
        xmlTextWriterSetIndentString(pWriter, BAD_CAST("  "));
        xmlTextWriterStartDocument(pWriter, nullptr, nullptr, nullptr);
        bOwns = true;
    }

    xmlTextWriterStartElement(pWriter, BAD_CAST("EditDoc"));
    for (auto const & i : maContents)
    {
        i->dumpAsXml(pWriter);
    }
    xmlTextWriterEndElement(pWriter);

    if (bOwns)
    {
       xmlTextWriterEndDocument(pWriter);
       xmlFreeTextWriter(pWriter);
    }
}


namespace {

struct LessByStart
{
    bool operator() (const std::unique_ptr<EditCharAttrib>& left, const std::unique_ptr<EditCharAttrib>& right) const
    {
        return left->GetStart() < right->GetStart();
    }
};

}

CharAttribList::CharAttribList()
: aAttribs()
, aDefFont()
, bHasEmptyAttribs(false)
{
}

CharAttribList::~CharAttribList()
{
}

void CharAttribList::InsertAttrib( EditCharAttrib* pAttrib )
{
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// optimize: binary search?    !
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    // Maybe just simply iterate backwards:
    // The most common and critical case: Attributes are already sorted
    // (InsertBinTextObject!) binary search would not be optimal here.
    // => Would bring something!

    const sal_Int32 nStart = pAttrib->GetStart(); // may be better for Comp.Opt.

#if OSL_DEBUG_LEVEL > 0 && !defined NDEBUG
    CharAttribList::DbgCheckAttribs(*this);
#endif

    if ( pAttrib->IsEmpty() )
        bHasEmptyAttribs = true;

    bool bInsert(true);
    for (sal_Int32 i = 0, n = aAttribs.size(); i < n; ++i)
    {
        const EditCharAttrib& rCurAttrib = *aAttribs[i].get();
        if (rCurAttrib.GetStart() > nStart)
        {
            aAttribs.insert(aAttribs.begin()+i, std::unique_ptr<EditCharAttrib>(pAttrib));
            bInsert = false;
            break;
        }
    }

    if (bInsert) aAttribs.push_back(std::unique_ptr<EditCharAttrib>(pAttrib));

#if OSL_DEBUG_LEVEL > 0 && !defined NDEBUG
    CharAttribList::DbgCheckAttribs(*this);
#endif
}

void CharAttribList::ResortAttribs()
{
    std::sort(aAttribs.begin(), aAttribs.end(), LessByStart());

#if OSL_DEBUG_LEVEL > 0 && !defined NDEBUG
    CharAttribList::DbgCheckAttribs(*this);
#endif
}

void CharAttribList::OptimizeRanges( SfxItemPool& rItemPool )
{
#if OSL_DEBUG_LEVEL > 0 && !defined NDEBUG
    CharAttribList::DbgCheckAttribs(*this);
#endif
    for (sal_Int32 i = 0; i < static_cast<sal_Int32>(aAttribs.size()); ++i)
    {
        EditCharAttrib& rAttr = *aAttribs[i].get();
        for (sal_Int32 nNext = i+1; nNext < static_cast<sal_Int32>(aAttribs.size()); ++nNext)
        {
            EditCharAttrib& rNext = *aAttribs[nNext].get();
            if (!rAttr.IsFeature() && rNext.GetStart() == rAttr.GetEnd() && rNext.Which() == rAttr.Which())
            {
                if (*rNext.GetItem() == *rAttr.GetItem())
                {
                    rAttr.GetEnd() = rNext.GetEnd();
                    rItemPool.Remove(*rNext.GetItem());
                    aAttribs.erase(aAttribs.begin()+nNext);
                }
                break;  // only 1 attr with same which can start here.
            }
            else if (rNext.GetStart() > rAttr.GetEnd())
            {
                break;
            }
        }
    }
#if OSL_DEBUG_LEVEL > 0 && !defined NDEBUG
    CharAttribList::DbgCheckAttribs(*this);
#endif
}

sal_Int32 CharAttribList::Count() const
{
    return aAttribs.size();
}

const EditCharAttrib* CharAttribList::FindAttrib( sal_uInt16 nWhich, sal_Int32 nPos ) const
{
    // Backwards, if one ends where the next starts.
    // => The starting one is the valid one ...
    AttribsType::const_reverse_iterator it = std::find_if(aAttribs.rbegin(), aAttribs.rend(),
        [&nWhich, &nPos](const AttribsType::value_type& rxAttr) {
            return rxAttr->Which() == nWhich && rxAttr->IsIn(nPos); });
    if (it != aAttribs.rend())
    {
        const EditCharAttrib& rAttr = **it;
        return &rAttr;
    }
    return nullptr;
}

EditCharAttrib* CharAttribList::FindAttrib( sal_uInt16 nWhich, sal_Int32 nPos )
{
    // Backwards, if one ends where the next starts.
    // => The starting one is the valid one ...
    AttribsType::reverse_iterator it = std::find_if(aAttribs.rbegin(), aAttribs.rend(),
        [&nWhich, &nPos](AttribsType::value_type& rxAttr) {
            return rxAttr->Which() == nWhich && rxAttr->IsIn(nPos); });
    if (it != aAttribs.rend())
    {
        EditCharAttrib& rAttr = **it;
        return &rAttr;
    }
    return nullptr;
}

const EditCharAttrib* CharAttribList::FindNextAttrib( sal_uInt16 nWhich, sal_Int32 nFromPos ) const
{
    assert(nWhich);
    for (auto const& attrib : aAttribs)
    {
        const EditCharAttrib& rAttr = *attrib;
        if (rAttr.GetStart() >= nFromPos && rAttr.Which() == nWhich)
            return &rAttr;
    }
    return nullptr;
}

bool CharAttribList::HasAttrib( sal_Int32 nStartPos, sal_Int32 nEndPos ) const
{
    return std::any_of(aAttribs.rbegin(), aAttribs.rend(),
        [&nStartPos, &nEndPos](const AttribsType::value_type& rxAttr) {
            return rxAttr->GetStart() < nEndPos && rxAttr->GetEnd() > nStartPos; });
}


namespace {

class FindByAddress
{
    const EditCharAttrib* mpAttr;
public:
    explicit FindByAddress(const EditCharAttrib* p) : mpAttr(p) {}
    bool operator() (const std::unique_ptr<EditCharAttrib>& r) const
    {
        return r.get() == mpAttr;
    }
};

}

void CharAttribList::Remove(const EditCharAttrib* p)
{
    AttribsType::iterator it = std::find_if(aAttribs.begin(), aAttribs.end(), FindByAddress(p));
    if (it != aAttribs.end())
        aAttribs.erase(it);
}

void CharAttribList::Remove(sal_Int32 nPos)
{
    if (nPos >= static_cast<sal_Int32>(aAttribs.size()))
        return;

    aAttribs.erase(aAttribs.begin()+nPos);
}

void CharAttribList::SetHasEmptyAttribs(bool b)
{
    bHasEmptyAttribs = b;
}

bool CharAttribList::HasBoundingAttrib( sal_Int32 nBound ) const
{
    // Backwards, if one ends where the next starts.
    // => The starting one is the valid one ...
    AttribsType::const_reverse_iterator it = aAttribs.rbegin(), itEnd = aAttribs.rend();
    for (; it != itEnd; ++it)
    {
        const EditCharAttrib& rAttr = **it;
        if (rAttr.GetEnd() < nBound)
            return false;

        if (rAttr.GetStart() == nBound || rAttr.GetEnd() == nBound)
            return true;
    }
    return false;
}

EditCharAttrib* CharAttribList::FindEmptyAttrib( sal_uInt16 nWhich, sal_Int32 nPos )
{
    if ( !bHasEmptyAttribs )
        return nullptr;

    for (const std::unique_ptr<EditCharAttrib>& rAttr : aAttribs)
    {
        if (rAttr->GetStart() == nPos && rAttr->GetEnd() == nPos && rAttr->Which() == nWhich)
            return rAttr.get();
    }
    return nullptr;
}

namespace {

class FindByStartPos
{
    sal_Int32 mnPos;
public:
    explicit FindByStartPos(sal_Int32 nPos) : mnPos(nPos) {}
    bool operator() (const std::unique_ptr<EditCharAttrib>& r) const
    {
        return r->GetStart() >= mnPos;
    }
};

}

const EditCharAttrib* CharAttribList::FindFeature( sal_Int32 nPos ) const
{
    // First, find the first attribute that starts at or after specified position.
    AttribsType::const_iterator it =
        std::find_if(aAttribs.begin(), aAttribs.end(), FindByStartPos(nPos));

    if (it == aAttribs.end())
        // All attributes are before the specified position.
        return nullptr;

    // And find the first attribute with feature.
    it = std::find_if(it, aAttribs.end(), [](const std::unique_ptr<EditCharAttrib>& aAttrib) { return aAttrib->IsFeature(); } );
    return it == aAttribs.end() ? nullptr : it->get();
}

namespace {

class RemoveEmptyAttrItem
{
    SfxItemPool& mrItemPool;
public:
    explicit RemoveEmptyAttrItem(SfxItemPool& rPool) : mrItemPool(rPool) {}
    void operator() (const std::unique_ptr<EditCharAttrib>& r)
    {
        if (r->IsEmpty())
            mrItemPool.Remove(*r->GetItem());
    }
};

}

void CharAttribList::DeleteEmptyAttribs( SfxItemPool& rItemPool )
{
    std::for_each(aAttribs.begin(), aAttribs.end(), RemoveEmptyAttrItem(rItemPool));
    aAttribs.erase( std::remove_if(aAttribs.begin(), aAttribs.end(), [](const std::unique_ptr<EditCharAttrib>& aAttrib) { return aAttrib->IsEmpty(); } ), aAttribs.end() );
    bHasEmptyAttribs = false;
}

#if OSL_DEBUG_LEVEL > 0 && !defined NDEBUG
void CharAttribList::DbgCheckAttribs(CharAttribList const& rAttribs)
{
    std::set<std::pair<sal_Int32, sal_uInt16>> zero_set;
    for (const std::unique_ptr<EditCharAttrib>& rAttr : rAttribs.aAttribs)
    {
        assert(rAttr->GetStart() <= rAttr->GetEnd());
        assert(!rAttr->IsFeature() || rAttr->GetLen() == 1);
        if (0 == rAttr->GetLen())
        {
            // not sure if 0-length attributes allowed at all in non-empty para?
            assert(zero_set.insert(std::make_pair(rAttr->GetStart(), rAttr->Which())).second && "duplicate 0-length attribute detected");
        }
    }
    CheckOrderedList(rAttribs.GetAttribs());
}
#endif

void CharAttribList::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("CharAttribList"));
    for (auto const & i : aAttribs) {
        i->dumpAsXml(pWriter);
    }
    xmlTextWriterEndElement(pWriter);
}

EditEngineItemPool::EditEngineItemPool()
    : SfxItemPool( "EditEngineItemPool", EE_ITEMS_START, EE_ITEMS_END,
                    aItemInfos, nullptr )
{
    m_xDefItems = EditDLL::Get().GetGlobalData()->GetDefItems();
    SetDefaults(m_xDefItems->getDefaults());
}

EditEngineItemPool::~EditEngineItemPool()
{
    ClearDefaults();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
