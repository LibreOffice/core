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

#include <comphelper/string.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/dialog.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>

#include <editeng/tstpitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/autokernitem.hxx>
#include <editeng/charsetcoloritem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/emphasismarkitem.hxx>
#include <editeng/charscaleitem.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/xmlcnitm.hxx>
#include <editeng/editids.hrc>
#include "editeng/editdata.hxx"
#include "editeng/lrspitem.hxx"
#include "editeng/ulspitem.hxx"
#include "editeng/lspcitem.hxx"

#include <editdoc.hxx>
#include <editdbg.hxx>
#include <editeng/eerdll.hxx>
#include <eerdll2.hxx>

#include <tools/stream.hxx>
#include <tools/debug.hxx>
#include <tools/shl.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>

#include <cassert>
#include <limits>

#include <boost/bind.hpp>

using namespace ::com::sun::star;


// ------------------------------------------------------------

sal_uInt16 GetScriptItemId( sal_uInt16 nItemId, short nScriptType )
{
    sal_uInt16 nId = nItemId;

    if ( ( nScriptType == i18n::ScriptType::ASIAN ) ||
         ( nScriptType == i18n::ScriptType::COMPLEX ) )
    {
        switch ( nItemId )
        {
            case EE_CHAR_LANGUAGE:
                nId = ( nScriptType == i18n::ScriptType::ASIAN ) ? EE_CHAR_LANGUAGE_CJK : EE_CHAR_LANGUAGE_CTL;
            break;
            case EE_CHAR_FONTINFO:
                nId = ( nScriptType == i18n::ScriptType::ASIAN ) ? EE_CHAR_FONTINFO_CJK : EE_CHAR_FONTINFO_CTL;
            break;
            case EE_CHAR_FONTHEIGHT:
                nId = ( nScriptType == i18n::ScriptType::ASIAN ) ? EE_CHAR_FONTHEIGHT_CJK : EE_CHAR_FONTHEIGHT_CTL;
            break;
            case EE_CHAR_WEIGHT:
                nId = ( nScriptType == i18n::ScriptType::ASIAN ) ? EE_CHAR_WEIGHT_CJK : EE_CHAR_WEIGHT_CTL;
            break;
            case EE_CHAR_ITALIC:
                nId = ( nScriptType == i18n::ScriptType::ASIAN ) ? EE_CHAR_ITALIC_CJK : EE_CHAR_ITALIC_CTL;
            break;
        }
    }

    return nId;
}

sal_Bool IsScriptItemValid( sal_uInt16 nItemId, short nScriptType )
{
    sal_Bool bValid = sal_True;

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


// ------------------------------------------------------------

// Should later be moved to TOOLS/STRING (Current: 303)
// for Grep: WS_TARGET

DBG_NAME( EE_TextPortion );
DBG_NAME( EE_EditLine );
DBG_NAME( EE_ContentNode );
DBG_NAME( EE_CharAttribList );
DBG_NAME( EE_ParaPortion )

const SfxItemInfo aItemInfos[EDITITEMCOUNT] = {
        { SID_ATTR_FRAMEDIRECTION, SFX_ITEM_POOLABLE },         // EE_PARA_WRITINGDIR
        { 0, SFX_ITEM_POOLABLE },                               // EE_PARA_XMLATTRIBS
        { SID_ATTR_PARA_HANGPUNCTUATION, SFX_ITEM_POOLABLE },   // EE_PARA_HANGINGPUNCTUATION
        { SID_ATTR_PARA_FORBIDDEN_RULES, SFX_ITEM_POOLABLE },
        { SID_ATTR_PARA_SCRIPTSPACE, SFX_ITEM_POOLABLE },       // EE_PARA_ASIANCJKSPACING
        { SID_ATTR_NUMBERING_RULE, SFX_ITEM_POOLABLE },         // EE_PARA_NUMBULL
        { 0, SFX_ITEM_POOLABLE },                               // EE_PARA_HYPHENATE
        { 0, SFX_ITEM_POOLABLE },                               // EE_PARA_BULLETSTATE
        { 0, SFX_ITEM_POOLABLE },                               // EE_PARA_OUTLLRSPACE
        { SID_ATTR_PARA_OUTLLEVEL, SFX_ITEM_POOLABLE },         // EE_PARA_OUTLLEVEL
        { SID_ATTR_PARA_BULLET, SFX_ITEM_POOLABLE },            // EE_PARA_BULLET
        { SID_ATTR_LRSPACE, SFX_ITEM_POOLABLE },                // EE_PARA_LRSPACE
        { SID_ATTR_ULSPACE, SFX_ITEM_POOLABLE },                // EE_PARA_ULSPACE
        { SID_ATTR_PARA_LINESPACE, SFX_ITEM_POOLABLE },         // EE_PARA_SBL
        { SID_ATTR_PARA_ADJUST, SFX_ITEM_POOLABLE },            // EE_PARA_JUST
        { SID_ATTR_TABSTOP, SFX_ITEM_POOLABLE },                // EE_PARA_TABS
        { SID_ATTR_ALIGN_HOR_JUSTIFY_METHOD, SFX_ITEM_POOLABLE }, // EE_PARA_JUST_METHOD
        { SID_ATTR_ALIGN_VER_JUSTIFY, SFX_ITEM_POOLABLE },      // EE_PARA_VER_JUST
        { SID_ATTR_CHAR_COLOR, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_FONT, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_FONTHEIGHT, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_SCALEWIDTH, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_WEIGHT, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_UNDERLINE, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_STRIKEOUT, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_POSTURE, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_CONTOUR, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_SHADOWED, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_ESCAPEMENT, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_AUTOKERN, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_KERNING, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_WORDLINEMODE, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_LANGUAGE, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_CJK_LANGUAGE, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_CTL_LANGUAGE, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_CJK_FONT, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_CTL_FONT, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_CJK_FONTHEIGHT, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_CTL_FONTHEIGHT, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_CJK_WEIGHT, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_CTL_WEIGHT, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_CJK_POSTURE, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_CTL_POSTURE, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_EMPHASISMARK, SFX_ITEM_POOLABLE },
        { SID_ATTR_CHAR_RELIEF, SFX_ITEM_POOLABLE },
        { 0, SFX_ITEM_POOLABLE },                           // EE_CHAR_RUBI_DUMMY
        { 0, SFX_ITEM_POOLABLE },                           // EE_CHAR_XMLATTRIBS
        { SID_ATTR_CHAR_OVERLINE, SFX_ITEM_POOLABLE },
        { 0, SFX_ITEM_POOLABLE },                           // EE_FEATURE_TAB
        { 0, SFX_ITEM_POOLABLE },                           // EE_FEATURE_LINEBR
        { SID_ATTR_CHAR_CHARSETCOLOR, SFX_ITEM_POOLABLE },  // EE_FEATURE_NOTCONV
        { SID_FIELD, SFX_ITEM_POOLABLE }
};

const sal_uInt16 aV1Map[] = {
    3999, 4001, 4002, 4003, 4004, 4005, 4006,
    4007, 4008, 4009, 4010, 4011, 4012, 4013, 4017, 4018, 4019 // MI: 4019?
};

const sal_uInt16 aV2Map[] = {
    3999, 4000, 4001, 4002, 4003, 4004, 4005, 4006, 4007, 4008, 4009,
    4010, 4011, 4012, 4013, 4014, 4015, 4016, 4018, 4019, 4020
};

const sal_uInt16 aV3Map[] = {
    3997, 3998, 3999, 4000, 4001, 4002, 4003, 4004, 4005, 4006, 4007,
    4009, 4010, 4011, 4012, 4013, 4014, 4015, 4016, 4017, 4018, 4019,
    4020, 4021
};

const sal_uInt16 aV4Map[] = {
    3994, 3995, 3996, 3997, 3998, 3999, 4000, 4001, 4002, 4003,
    4004, 4005, 4006, 4007, 4008, 4009, 4010, 4011, 4012, 4013,
    4014, 4015, 4016, 4017, 4018,
    /* CJK Items inserted here: EE_CHAR_LANGUAGE - EE_CHAR_XMLATTRIBS */
    4034, 4035, 4036, 4037
};

const sal_uInt16 aV5Map[] = {
    3994, 3995, 3996, 3997, 3998, 3999, 4000, 4001, 4002, 4003,
    4004, 4005, 4006, 4007, 4008, 4009, 4010, 4011, 4012, 4013,
    4014, 4015, 4016, 4017, 4018, 4019, 4020, 4021, 4022, 4023,
    4024, 4025, 4026, 4027, 4028, 4029, 4030, 4031, 4032, 4033,
    /* EE_CHAR_OVERLINE inserted here */
    4035, 4036, 4037, 4038
};

EditCharAttrib* MakeCharAttrib( SfxItemPool& rPool, const SfxPoolItem& rAttr, sal_uInt16 nS, sal_uInt16 nE )
{
    // Create a new attribute in the pool
    const SfxPoolItem& rNew = rPool.Put( rAttr );

    EditCharAttrib* pNew = 0;
    switch( rNew.Which() )
    {
        case EE_CHAR_LANGUAGE:
        case EE_CHAR_LANGUAGE_CJK:
        case EE_CHAR_LANGUAGE_CTL:
        {
            pNew = new EditCharAttribLanguage( (const SvxLanguageItem&)rNew, nS, nE );
        }
        break;
        case EE_CHAR_COLOR:
        {
            pNew = new EditCharAttribColor( (const SvxColorItem&)rNew, nS, nE );
        }
        break;
        case EE_CHAR_FONTINFO:
        case EE_CHAR_FONTINFO_CJK:
        case EE_CHAR_FONTINFO_CTL:
        {
            pNew = new EditCharAttribFont( (const SvxFontItem&)rNew, nS, nE );
        }
        break;
        case EE_CHAR_FONTHEIGHT:
        case EE_CHAR_FONTHEIGHT_CJK:
        case EE_CHAR_FONTHEIGHT_CTL:
        {
            pNew = new EditCharAttribFontHeight( (const SvxFontHeightItem&)rNew, nS, nE );
        }
        break;
        case EE_CHAR_FONTWIDTH:
        {
            pNew = new EditCharAttribFontWidth( (const SvxCharScaleWidthItem&)rNew, nS, nE );
        }
        break;
        case EE_CHAR_WEIGHT:
        case EE_CHAR_WEIGHT_CJK:
        case EE_CHAR_WEIGHT_CTL:
        {
            pNew = new EditCharAttribWeight( (const SvxWeightItem&)rNew, nS, nE );
        }
        break;
        case EE_CHAR_UNDERLINE:
        {
            pNew = new EditCharAttribUnderline( (const SvxUnderlineItem&)rNew, nS, nE );
        }
        break;
        case EE_CHAR_OVERLINE:
        {
            pNew = new EditCharAttribOverline( (const SvxOverlineItem&)rNew, nS, nE );
        }
        break;
        case EE_CHAR_EMPHASISMARK:
        {
            pNew = new EditCharAttribEmphasisMark( (const SvxEmphasisMarkItem&)rNew, nS, nE );
        }
        break;
        case EE_CHAR_RELIEF:
        {
            pNew = new EditCharAttribRelief( (const SvxCharReliefItem&)rNew, nS, nE );
        }
        break;
        case EE_CHAR_STRIKEOUT:
        {
            pNew = new EditCharAttribStrikeout( (const SvxCrossedOutItem&)rNew, nS, nE );
        }
        break;
        case EE_CHAR_ITALIC:
        case EE_CHAR_ITALIC_CJK:
        case EE_CHAR_ITALIC_CTL:
        {
            pNew = new EditCharAttribItalic( (const SvxPostureItem&)rNew, nS, nE );
        }
        break;
        case EE_CHAR_OUTLINE:
        {
            pNew = new EditCharAttribOutline( (const SvxContourItem&)rNew, nS, nE );
        }
        break;
        case EE_CHAR_SHADOW:
        {
            pNew = new EditCharAttribShadow( (const SvxShadowedItem&)rNew, nS, nE );
        }
        break;
        case EE_CHAR_ESCAPEMENT:
        {
            pNew = new EditCharAttribEscapement( (const SvxEscapementItem&)rNew, nS, nE );
        }
        break;
        case EE_CHAR_PAIRKERNING:
        {
            pNew = new EditCharAttribPairKerning( (const SvxAutoKernItem&)rNew, nS, nE );
        }
        break;
        case EE_CHAR_KERNING:
        {
            pNew = new EditCharAttribKerning( (const SvxKerningItem&)rNew, nS, nE );
        }
        break;
        case EE_CHAR_WLM:
        {
            pNew = new EditCharAttribWordLineMode( (const SvxWordLineModeItem&)rNew, nS, nE );
        }
        break;
        case EE_CHAR_XMLATTRIBS:
        {
            pNew = new EditCharAttrib( rNew, nS, nE );  // Attribute is only for holding XML information...
        }
        break;
        case EE_FEATURE_TAB:
        {
            pNew = new EditCharAttribTab( (const SfxVoidItem&)rNew, nS );
        }
        break;
        case EE_FEATURE_LINEBR:
        {
            pNew = new EditCharAttribLineBreak( (const SfxVoidItem&)rNew, nS );
        }
        break;
        case EE_FEATURE_FIELD:
        {
            pNew = new EditCharAttribField( (const SvxFieldItem&)rNew, nS );
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

void TextPortionList::DeleteFromPortion(size_t nDelFrom)
{
    DBG_ASSERT( ( nDelFrom < maPortions.size() ) || ( (nDelFrom == 0) && maPortions.empty() ), "DeleteFromPortion: Out of range" );
    PortionsType::iterator it = maPortions.begin();
    std::advance(it, nDelFrom);
    maPortions.erase(it, maPortions.end());
}

size_t TextPortionList::Count() const
{
    return maPortions.size();
}

const TextPortion* TextPortionList::operator[](size_t nPos) const
{
    return &maPortions[nPos];
}

TextPortion* TextPortionList::operator[](size_t nPos)
{
    return &maPortions[nPos];
}

void TextPortionList::Append(TextPortion* p)
{
    maPortions.push_back(p);
}

void TextPortionList::Insert(size_t nPos, TextPortion* p)
{
    maPortions.insert(maPortions.begin()+nPos, p);
}

void TextPortionList::Remove(size_t nPos)
{
    maPortions.erase(maPortions.begin()+nPos);
}

namespace {

class FindTextPortionByAddress : std::unary_function<TextPortion, bool>
{
    const TextPortion* mp;
public:
    FindTextPortionByAddress(const TextPortion* p) : mp(p) {}
    bool operator() (const TextPortion& v) const
    {
        return &v == mp;
    }
};

}

size_t TextPortionList::GetPos(const TextPortion* p) const
{
    PortionsType::const_iterator it =
        std::find_if(maPortions.begin(), maPortions.end(), FindTextPortionByAddress(p));

    if (it == maPortions.end())
        return std::numeric_limits<size_t>::max(); // not found.

    return std::distance(maPortions.begin(), it);
}

size_t TextPortionList::FindPortion(
    sal_uInt16 nCharPos, sal_uInt16& nPortionStart, bool bPreferStartingPortion) const
{
    // When nCharPos at portion limit, the left portion is found
    sal_uInt16 nTmpPos = 0;
    size_t n = maPortions.size();
    for (size_t i = 0; i < n; ++i)
    {
        const TextPortion& rPortion = maPortions[i];
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

sal_uInt16 TextPortionList::GetStartPos(size_t nPortion)
{
    sal_uInt16 nPos = 0;
    for (size_t i = 0; i < nPortion; ++i)
    {
        const TextPortion& rPortion = maPortions[i];
        nPos = nPos + rPortion.GetLen();
    }
    return nPos;
}


ExtraPortionInfo::ExtraPortionInfo()
{
    nOrgWidth = 0;
    nWidthFullCompression = 0;
    nMaxCompression100thPercent = 0;
    nAsianCompressionTypes = 0;
    nPortionOffsetX = 0;
    bFirstCharIsRightPunktuation = sal_False;
    bCompressed = sal_False;
    pOrgDXArray = NULL;
}

ExtraPortionInfo::~ExtraPortionInfo()
{
    delete[] pOrgDXArray;
}

void ExtraPortionInfo::SaveOrgDXArray( const sal_Int32* pDXArray, sal_uInt16 nLen )
{
    delete[] pOrgDXArray;
    pOrgDXArray = new sal_Int32[nLen];
    memcpy( pOrgDXArray, pDXArray, nLen*sizeof(sal_Int32) );
}


ParaPortion::ParaPortion( ContentNode* pN )
{
    DBG_CTOR( EE_ParaPortion, 0 );

    pNode               = pN;
    bInvalid            = sal_True;
    bVisible            = sal_True;
    bSimple             = sal_False;
    bForceRepaint       = sal_False;
    nInvalidPosStart    = 0;
    nInvalidDiff        = 0;
    nHeight             = 0;
    nFirstLineOffset    = 0;
    nBulletX            = 0;
}

ParaPortion::~ParaPortion()
{
    DBG_DTOR( EE_ParaPortion, 0 );
}

void ParaPortion::MarkInvalid( sal_uInt16 nStart, short nDiff )
{
    if ( bInvalid == sal_False )
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
            nInvalidPosStart = std::min( nInvalidPosStart, (sal_uInt16) ( nDiff < 0 ? nStart+nDiff : nDiff ) );
            nInvalidDiff = 0;
            bSimple = sal_False;
        }
    }
    bInvalid = sal_True;
    aScriptInfos.clear();
    aWritingDirectionInfos.clear();
}

void ParaPortion::MarkSelectionInvalid( sal_uInt16 nStart, sal_uInt16 /* nEnd */ )
{
    if ( bInvalid == sal_False )
    {
        nInvalidPosStart = nStart;
//      nInvalidPosEnd = nEnd;
    }
    else
    {
        nInvalidPosStart = std::min( nInvalidPosStart, nStart );
//      nInvalidPosEnd = pNode->Len();
    }
    nInvalidDiff = 0;
    bInvalid = sal_True;
    bSimple = sal_False;
    aScriptInfos.clear();
    aWritingDirectionInfos.clear();
}

sal_uInt16 ParaPortion::GetLineNumber( sal_uInt16 nIndex ) const
{
    DBG_ASSERTWARNING( aLineList.Count(), "Empty ParaPortion in GetLine!" );
    DBG_ASSERT( bVisible, "Why GetLine() on an invisible paragraph?" );

    for ( sal_uInt16 nLine = 0; nLine < aLineList.Count(); nLine++ )
    {
        if ( aLineList[nLine]->IsIn( nIndex ) )
            return nLine;
    }

    // Then it should be at the end of the last line!
    DBG_ASSERT( nIndex == aLineList[ aLineList.Count() - 1 ]->GetEnd(), "Index dead wrong!" );
    return (aLineList.Count()-1);
}

void ParaPortion::SetVisible( sal_Bool bMakeVisible )
{
    bVisible = bMakeVisible;
}

void ParaPortion::CorrectValuesBehindLastFormattedLine( sal_uInt16 nLastFormattedLine )
{
    sal_uInt16 nLines = aLineList.Count();
    DBG_ASSERT( nLines, "CorrectPortionNumbersFromLine: Empty Portion?" );
    if ( nLastFormattedLine < ( nLines - 1 ) )
    {
        const EditLine* pLastFormatted = aLineList[ nLastFormattedLine ];
        const EditLine* pUnformatted = aLineList[ nLastFormattedLine+1 ];
        short nPortionDiff = pUnformatted->GetStartPortion() - pLastFormatted->GetEndPortion();
        short nTextDiff = pUnformatted->GetStart() - pLastFormatted->GetEnd();
        nTextDiff++;    // LastFormatted->GetEnd() was included => 1 deducted too much!

        // The first unformatted must begin exactly one Portion behind the last
        // of the formatted:
        // If the modified line was split into one portion, can
        // nLastEnd > nNextStart!
        int nPDiff = -( nPortionDiff-1 );
        int nTDiff = -( nTextDiff-1 );
        if ( nPDiff || nTDiff )
        {
            for ( sal_uInt16 nL = nLastFormattedLine+1; nL < nLines; nL++ )
            {
                EditLine* pLine = aLineList[ nL ];

                pLine->GetStartPortion() = sal::static_int_cast< sal_uInt16 >(
                    pLine->GetStartPortion() + nPDiff);
                pLine->GetEndPortion() = sal::static_int_cast< sal_uInt16 >(
                    pLine->GetEndPortion() + nPDiff);

                pLine->GetStart() = sal::static_int_cast< sal_uInt16 >(
                    pLine->GetStart() + nTDiff);
                pLine->GetEnd() = sal::static_int_cast< sal_uInt16 >(
                    pLine->GetEnd() + nTDiff);

                pLine->SetValid();
            }
        }
    }
    DBG_ASSERT( aLineList[ aLineList.Count()-1 ]->GetEnd() == pNode->Len(), "CorrectLines: The end is not right!" );
}

// Shared reverse lookup acceleration pieces ...

namespace {

template<typename _Array, typename _Val>
size_t FastGetPos(const _Array& rArray, const _Val* p, size_t& rLastPos)
{
    size_t nArrayLen = rArray.size();

    // Through certain filter code-paths we do a lot of appends, which in
    // turn call GetPos - creating some N^2 nightmares. If we have a
    // non-trivially large list, do a few checks from the end first.
    if (rLastPos > 16 && nArrayLen > 16)
    {
        size_t nEnd;
        if (rLastPos > nArrayLen - 2)
            nEnd = nArrayLen;
        else
            nEnd = rLastPos + 2;

        for (size_t nIdx = rLastPos - 2; nIdx < nEnd; ++nIdx)
        {
            if (&rArray.at(nIdx) == p)
            {
                rLastPos = nIdx;
                return nIdx;
            }
        }
    }
    // The world's lamest linear search from svarray ...
    for (size_t nIdx = 0; nIdx < nArrayLen; ++nIdx)
        if (&rArray.at(nIdx) == p)
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
    return 0 <= nPos && static_cast<size_t>(nPos) < maPortions.size() ? &maPortions[nPos] : NULL;
}

const ParaPortion* ParaPortionList::operator [](sal_Int32 nPos) const
{
    return 0 <= nPos && static_cast<size_t>(nPos) < maPortions.size() ? &maPortions[nPos] : NULL;
}

ParaPortion* ParaPortionList::Release(sal_Int32 nPos)
{
    if (nPos < 0 || maPortions.size() <= static_cast<size_t>(nPos))
    {
        SAL_WARN( "editeng", "ParaPortionList::Release - out of bounds pos " << nPos);
        return NULL;
    }
    return maPortions.release(maPortions.begin()+nPos).release();
}

void ParaPortionList::Remove(sal_Int32 nPos)
{
    if (nPos < 0 || maPortions.size() <= static_cast<size_t>(nPos))
    {
        SAL_WARN( "editeng", "ParaPortionList::Remove - out of bounds pos " << nPos);
        return;
    }
    maPortions.erase(maPortions.begin()+nPos);
}

void ParaPortionList::Insert(sal_Int32 nPos, ParaPortion* p)
{
    if (nPos < 0 || maPortions.size() < static_cast<size_t>(nPos))
    {
        SAL_WARN( "editeng", "ParaPortionList::Insert - out of bounds pos " << nPos);
        return;
    }
    maPortions.insert(maPortions.begin()+nPos, p);
}

void ParaPortionList::Append(ParaPortion* p)
{
    maPortions.push_back(p);
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
    for (size_t i = 0, n = maPortions.size(); i < n; ++i)
    {
        const ParaPortion* pTmpPortion = &maPortions[i];
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
        nY += maPortions[i].GetHeight(); // should also be correct even in bVisible!
        if ( nY > nYOffset )
            return i <= SAL_MAX_INT32 ? static_cast<sal_Int32>(i) : SAL_MAX_INT32;
    }
    return EE_PARA_NOT_FOUND;
}

const ParaPortion* ParaPortionList::SafeGetObject(sal_Int32 nPos) const
{
    return 0 <= nPos && static_cast<size_t>(nPos) < maPortions.size() ? &maPortions[nPos] : NULL;
}

ParaPortion* ParaPortionList::SafeGetObject(sal_Int32 nPos)
{
    return 0 <= nPos && static_cast<size_t>(nPos) < maPortions.size() ? &maPortions[nPos] : NULL;
}

#if OSL_DEBUG_LEVEL > 2
void ParaPortionList::DbgCheck( EditDoc& rDoc)
{
    DBG_ASSERT( Count() == rDoc.Count(), "ParaPortionList::DbgCheck() - Count() unequal!" );
    for ( sal_uInt16 i = 0; i < Count(); i++ )
    {
        DBG_ASSERT( SafeGetObject(i), "ParaPortionList::DbgCheck() - Null-Pointer in List!" );
        DBG_ASSERT( GetObject(i)->GetNode(), "ParaPortionList::DbgCheck() - Null-Pointer in List(2)!" );
        DBG_ASSERT( GetObject(i)->GetNode() == rDoc.GetObject(i), "ParaPortionList::DbgCheck() - Entries intersect!" );
    }
}
#endif

ContentAttribsInfo::ContentAttribsInfo( const SfxItemSet& rParaAttribs ) :
        aPrevParaAttribs( rParaAttribs)
{
}

void ContentAttribsInfo::RemoveAllCharAttribsFromPool(SfxItemPool& rPool) const
{
    CharAttribsType::const_iterator it = aPrevCharAttribs.begin(), itEnd = aPrevCharAttribs.end();
    for (; it != itEnd; ++it)
        rPool.Remove(*it->GetItem());
}

void ContentAttribsInfo::AppendCharAttrib(EditCharAttrib* pNew)
{
    aPrevCharAttribs.push_back(pNew);
}

void ConvertItem( SfxPoolItem& rPoolItem, MapUnit eSourceUnit, MapUnit eDestUnit )
{
    DBG_ASSERT( eSourceUnit != eDestUnit, "ConvertItem - Why?!" );

    switch ( rPoolItem.Which() )
    {
        case EE_PARA_LRSPACE:
        {
            DBG_ASSERT( rPoolItem.IsA( TYPE( SvxLRSpaceItem ) ), "ConvertItem: invalid Item!" );
            SvxLRSpaceItem& rItem = (SvxLRSpaceItem&)rPoolItem;
            rItem.SetTxtFirstLineOfst( sal::static_int_cast< short >( OutputDevice::LogicToLogic( rItem.GetTxtFirstLineOfst(), eSourceUnit, eDestUnit ) ) );
            rItem.SetTxtLeft( OutputDevice::LogicToLogic( rItem.GetTxtLeft(), eSourceUnit, eDestUnit ) );
            rItem.SetRight( OutputDevice::LogicToLogic( rItem.GetRight(), eSourceUnit, eDestUnit ) );
        }
        break;
        case EE_PARA_ULSPACE:
        {
            DBG_ASSERT( rPoolItem.IsA( TYPE( SvxULSpaceItem ) ), "ConvertItem: Invalid Item!" );
            SvxULSpaceItem& rItem = (SvxULSpaceItem&)rPoolItem;
            rItem.SetUpper( sal::static_int_cast< sal_uInt16 >( OutputDevice::LogicToLogic( rItem.GetUpper(), eSourceUnit, eDestUnit ) ) );
            rItem.SetLower( sal::static_int_cast< sal_uInt16 >( OutputDevice::LogicToLogic( rItem.GetLower(), eSourceUnit, eDestUnit ) ) );
        }
        break;
        case EE_PARA_SBL:
        {
            DBG_ASSERT( rPoolItem.IsA( TYPE( SvxLineSpacingItem ) ), "ConvertItem: Invalid Item!" );
            SvxLineSpacingItem& rItem = (SvxLineSpacingItem&)rPoolItem;
            // SetLineHeight changes also eLineSpace!
            if ( rItem.GetLineSpaceRule() == SVX_LINE_SPACE_MIN )
                rItem.SetLineHeight( sal::static_int_cast< sal_uInt16 >( OutputDevice::LogicToLogic( rItem.GetLineHeight(), eSourceUnit, eDestUnit ) ) );
        }
        break;
        case EE_PARA_TABS:
        {
            DBG_ASSERT( rPoolItem.IsA( TYPE( SvxTabStopItem ) ), "ConvertItem: Invalid Item!" );
            SvxTabStopItem& rItem = (SvxTabStopItem&)rPoolItem;
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
            DBG_ASSERT( rPoolItem.IsA( TYPE( SvxFontHeightItem ) ), "ConvertItem: Invalid Item!" );
            SvxFontHeightItem& rItem = (SvxFontHeightItem&)rPoolItem;
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

        if ( rSource.GetItemState( nSourceWhich, sal_False ) == SFX_ITEM_ON )
        {
            MapUnit eSourceUnit = pSourceUnit ? *pSourceUnit : (MapUnit)pSourcePool->GetMetric( nSourceWhich );
            MapUnit eDestUnit = pDestUnit ? *pDestUnit : (MapUnit)pDestPool->GetMetric( nWhich );
            if ( eSourceUnit != eDestUnit )
            {
                SfxPoolItem* pItem = rSource.Get( nSourceWhich ).Clone();
//              pItem->SetWhich( nWhich );
                ConvertItem( *pItem, eSourceUnit, eDestUnit );
                rDest.Put( *pItem, nWhich );
                delete pItem;
            }
            else
            {
                rDest.Put( rSource.Get( nSourceWhich ), nWhich );
            }
        }
    }
}

EditLine::EditLine() :
    bHangingPunctuation(false),
    bInvalid(true)
{
    DBG_CTOR( EE_EditLine, 0 );

    nStart = nEnd = 0;
    nStartPortion = 0;  // to be able to tell the difference between a line
                        // without Ptorions form one with the Portion number 0
    nEndPortion = 0;
    nHeight = 0;
    nStartPosX = 0;
    nTxtHeight = 0;
    nTxtWidth = 0;
    nCrsrHeight = 0;
    nMaxAscent = 0;
}

EditLine::EditLine( const EditLine& r ) :
    bHangingPunctuation(r.bHangingPunctuation),
    bInvalid(true)
{
    DBG_CTOR( EE_EditLine, 0 );

    nEnd = r.nEnd;
    nStart = r.nStart;
    nStartPortion = r.nStartPortion;
    nEndPortion = r.nEndPortion;

    nHeight = 0;
    nStartPosX = 0;
    nTxtHeight = 0;
    nTxtWidth = 0;
    nCrsrHeight = 0;
    nMaxAscent = 0;
}

EditLine::~EditLine()
{
    DBG_DTOR( EE_EditLine, 0 );
}

EditLine::CharPosArrayType& EditLine::GetCharPosArray()
{
    return aPositions;
}

const EditLine::CharPosArrayType& EditLine::GetCharPosArray() const
{
    return aPositions;
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
    pL->nCrsrHeight     = nCrsrHeight;
    pL->nMaxAscent      = nMaxAscent;

    return pL;
}

sal_Bool operator == ( const EditLine& r1,  const EditLine& r2  )
{
    if ( r1.nStart != r2.nStart )
        return sal_False;

    if ( r1.nEnd != r2.nEnd )
        return sal_False;

    if ( r1.nStartPortion != r2.nStartPortion )
        return sal_False;

    if ( r1.nEndPortion != r2.nEndPortion )
        return sal_False;

    return sal_True;
}

EditLine& EditLine::operator = ( const EditLine& r )
{
    nEnd = r.nEnd;
    nStart = r.nStart;
    nEndPortion = r.nEndPortion;
    nStartPortion = r.nStartPortion;
    return *this;
}


sal_Bool operator != ( const EditLine& r1,  const EditLine& r2  )
{
    return !( r1 == r2 );
}

void EditLine::SetHeight( sal_uInt16 nH, sal_uInt16 nTxtH, sal_uInt16 nCrsrH )
{
    nHeight = nH;
    nTxtHeight = ( nTxtH ? nTxtH : nH );
    nCrsrHeight = ( nCrsrH ? nCrsrH : nTxtHeight );
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
    TextPortion* pPortion;

    sal_uInt16 nIndex = GetStart();

    DBG_ASSERT( rParaPortion.GetTextPortions().Count(), "GetTextSize before CreatePortions !" );

    for ( sal_uInt16 n = nStartPortion; n <= nEndPortion; n++ )
    {
        pPortion = rParaPortion.GetTextPortions()[n];
        switch ( pPortion->GetKind() )
        {
            case PORTIONKIND_TEXT:
            case PORTIONKIND_FIELD:
            case PORTIONKIND_HYPHENATOR:
            {
                aTmpSz = pPortion->GetSize();
                aSz.Width() += aTmpSz.Width();
                if ( aSz.Height() < aTmpSz.Height() )
                    aSz.Height() = aTmpSz.Height();
            }
            break;
            case PORTIONKIND_TAB:
            {
                aSz.Width() += pPortion->GetSize().Width();
            }
            break;
        }
        nIndex = nIndex + pPortion->GetLen();
    }

    SetHeight( (sal_uInt16)aSz.Height() );
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

void EditLineList::DeleteFromLine(size_t nDelFrom)
{
    DBG_ASSERT( nDelFrom <= (maLines.size() - 1), "DeleteFromLine: Out of range" );
    LinesType::iterator it = maLines.begin();
    std::advance(it, nDelFrom);
    maLines.erase(it, maLines.end());
}

size_t EditLineList::FindLine(sal_uInt16 nChar, bool bInclEnd)
{
    size_t n = maLines.size();
    for (size_t i = 0; i < n; ++i)
    {
        const EditLine& rLine = maLines[i];
        if ( (bInclEnd && (rLine.GetEnd() >= nChar)) ||
             (rLine.GetEnd() > nChar) )
        {
            return i;
        }
    }

    DBG_ASSERT( !bInclEnd, "Line not found: FindLine" );
    return n - 1;
}

size_t EditLineList::Count() const
{
    return maLines.size();
}

const EditLine* EditLineList::operator[](size_t nPos) const
{
    return &maLines[nPos];
}

EditLine* EditLineList::operator[](size_t nPos)
{
    return &maLines[nPos];
}

void EditLineList::Append(EditLine* p)
{
    maLines.push_back(p);
}

void EditLineList::Insert(size_t nPos, EditLine* p)
{
    maLines.insert(maLines.begin()+nPos, p);
}

EditPaM::EditPaM() : pNode(NULL), nIndex(0) {}
EditPaM::EditPaM(const EditPaM& r) : pNode(r.pNode), nIndex(r.nIndex) {}
EditPaM::EditPaM(ContentNode* p, sal_uInt16 n) : pNode(p), nIndex(n) {}

const ContentNode* EditPaM::GetNode() const
{
    return pNode;
}

ContentNode* EditPaM::GetNode()
{
    return pNode;
}

void EditPaM::SetNode(ContentNode* p)
{
    pNode = p;
}

sal_Bool EditPaM::DbgIsBuggy( EditDoc& rDoc )
{
    if ( !pNode )
        return sal_True;
    if ( rDoc.GetPos( pNode ) >= rDoc.Count() )
        return sal_True;
    if ( nIndex > pNode->Len() )
        return sal_True;

    return sal_False;
}

sal_Bool EditSelection::DbgIsBuggy( EditDoc& rDoc )
{
    if ( aStartPaM.DbgIsBuggy( rDoc ) )
        return sal_True;
    if ( aEndPaM.DbgIsBuggy( rDoc ) )
        return sal_True;

    return sal_False;
}

EditSelection::EditSelection()
{
}

EditSelection::EditSelection( const EditPaM& rStartAndAnd )
{
    // could still be optimized!
    // do no first call the Def-constructor from PaM!
    aStartPaM = rStartAndAnd;
    aEndPaM = rStartAndAnd;
}

EditSelection::EditSelection( const EditPaM& rStart, const EditPaM& rEnd )
{
    // could still be optimized!
    aStartPaM = rStart;
    aEndPaM = rEnd;
}

EditSelection& EditSelection::operator = ( const EditPaM& rPaM )
{
    aStartPaM = rPaM;
    aEndPaM = rPaM;
    return *this;
}

sal_Bool EditSelection::IsInvalid() const
{
    EditPaM aEmptyPaM;

    if ( aStartPaM == aEmptyPaM )
        return sal_True;

    if ( aEndPaM == aEmptyPaM )
        return sal_True;

    return sal_False;
}

sal_Bool EditSelection::Adjust( const EditDoc& rNodes )
{
    DBG_ASSERT( aStartPaM.GetIndex() <= aStartPaM.GetNode()->Len(), "Index out of range in Adjust(1)" );
    DBG_ASSERT( aEndPaM.GetIndex() <= aEndPaM.GetNode()->Len(), "Index out of range in Adjust(2)" );

    const ContentNode* pStartNode = aStartPaM.GetNode();
    const ContentNode* pEndNode = aEndPaM.GetNode();

    sal_Int32 nStartNode = rNodes.GetPos( pStartNode );
    sal_Int32 nEndNode = rNodes.GetPos( pEndNode );

    DBG_ASSERT( nStartNode != SAL_MAX_INT32, "Node out of range in Adjust(1)" );
    DBG_ASSERT( nEndNode != SAL_MAX_INT32, "Node out of range in Adjust(2)" );

    sal_Bool bSwap = sal_False;
    if ( nStartNode > nEndNode )
        bSwap = sal_True;
    else if ( ( nStartNode == nEndNode ) && ( aStartPaM.GetIndex() > aEndPaM.GetIndex() ) )
        bSwap = sal_True;

    if ( bSwap )
    {
        EditPaM aTmpPaM( aStartPaM );
        aStartPaM = aEndPaM;
        aEndPaM = aTmpPaM;
    }

    return bSwap;
}

sal_Bool operator == ( const EditPaM& r1,  const EditPaM& r2  )
{
    if ( r1.GetNode() != r2.GetNode() )
        return sal_False;

    if ( r1.GetIndex() != r2.GetIndex() )
        return sal_False;

    return sal_True;
}

EditPaM& EditPaM::operator = ( const EditPaM& rPaM )
{
    nIndex = rPaM.nIndex;
    pNode = rPaM.pNode;
    return *this;
}

sal_Bool operator != ( const EditPaM& r1,  const EditPaM& r2  )
{
    return !( r1 == r2 );
}

ContentNode::ContentNode( SfxItemPool& rPool ) : aContentAttribs( rPool )
{
    DBG_CTOR( EE_ContentNode, 0 );
}

ContentNode::ContentNode( const XubString& rStr, const ContentAttribs& rContentAttribs ) :
    maString(rStr), aContentAttribs(rContentAttribs)
{
    DBG_CTOR( EE_ContentNode, 0 );
}

ContentNode::~ContentNode()
{
    DBG_DTOR( EE_ContentNode, 0 );
}

void ContentNode::ExpandAttribs( sal_uInt16 nIndex, sal_uInt16 nNew, SfxItemPool& rItemPool )
{
    if ( !nNew )
        return;

    // Since features are treated differently than normal character attributes,
    // can also the order of the start list be change!
    // In every if ...,  in the next (n) opportunities due to bFeature or
    // an existing special case, must (n-1) opportunities be provided with
    // bResort. The most likely possibility receives no bResort, so that is
    // not sorted anew when all attributes are the same.
    bool bResort = false;
    bool bExpandedEmptyAtIndexNull = false;

    size_t nAttr = 0;
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
                // Do not check Index, a emty one could only be there
                // When later checking it anyhow:
                //   Special caase: Start == 0; AbsLen == 1, nNew = 1
                // => Expand, because of paragraph break!
                // Start <= nIndex, End >= nIndex => Start=End=nIndex!
//              if ( pAttrib->GetStart() == nIndex )
                pAttrib->Expand( nNew );
                if ( pAttrib->GetStart() == 0 )
                    bExpandedEmptyAtIndexNull = sal_True;
            }
            // 1: Attribute starts before, goes to index ...
            else if ( pAttrib->GetEnd() == nIndex ) // Start must be before
            {
                // Only expand when there is no feature
                // and if not in exclude list!
                // Otherwise, a UL will go on until a new ULDB, expaning both
//              if ( !pAttrib->IsFeature() && !rExclList.FindAttrib( pAttrib->Which() ) )
                if ( !pAttrib->IsFeature() && !aCharAttribList.FindEmptyAttrib( pAttrib->Which(), nIndex ) )
                {
                    if ( !pAttrib->IsEdge() )
                        pAttrib->Expand( nNew );
                }
                else
                    bResort = sal_True;
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
                    bResort = sal_True;
                }
                else
                {
                    bool bExpand = false;
                    if ( nIndex == 0 )
                    {
                        bExpand = sal_True;
                        if( bExpandedEmptyAtIndexNull )
                        {
                            // Check if this kind of attribut was empty and expanded here...
                            sal_uInt16 nW = pAttrib->GetItem()->Which();
                            for ( sal_uInt16 nA = 0; nA < nAttr; nA++ )
                            {
                                const EditCharAttrib& r = aCharAttribList.GetAttribs()[nA];
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
        bool bSep = ( maString.GetChar( nIndex ) == ' ' ) || IsFeature( nIndex );
        mpWrongList->TextInserted( nIndex, nNew, bSep );
    }

#if OSL_DEBUG_LEVEL > 2
    OSL_ENSURE( CheckOrderedList( aCharAttribList.GetAttribs(), sal_True ), "Expand: Start List distorted" );
#endif
}

void ContentNode::CollapsAttribs( sal_uInt16 nIndex, sal_uInt16 nDeleted, SfxItemPool& rItemPool )
{
    if ( !nDeleted )
        return;

    // Since features are treated differently than normal character attributes,
    // can also the order of the start list be change!
    bool bResort = false;
    bool bDelAttr = false;
    sal_uInt16 nEndChanges = nIndex+nDeleted;

    size_t nAttr = 0;
    CharAttribList::AttribsType& rAttribs = aCharAttribList.GetAttribs();
    EditCharAttrib* pAttrib = GetAttrib(rAttribs, nAttr);
    while ( pAttrib )
    {
        bDelAttr = false;
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
                    pAttrib->GetEnd() = nIndex; // empty
                else
                    bDelAttr = sal_True;
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
                    bResort = sal_True;
                }
                else
                {
                    pAttrib->GetStart() = nEndChanges;
                    pAttrib->MoveBackward( nDeleted );
                }
            }
        }
        DBG_ASSERT( !pAttrib->IsFeature() || ( pAttrib->GetLen() == 1 ), "Expand: FeaturesLen != 1" );

        DBG_ASSERT( pAttrib->GetStart() <= pAttrib->GetEnd(), "Collaps: Attribut distorted!" );
        DBG_ASSERT( ( pAttrib->GetEnd() <= Len()) || bDelAttr, "Collaps: Attribute larger than paragraph!" );
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

#if OSL_DEBUG_LEVEL > 2
    OSL_ENSURE( CheckOrderedList( aCharAttribList.GetAttribs(), sal_True ), "Collaps: Start list distorted" );
#endif
}

void ContentNode::CopyAndCutAttribs( ContentNode* pPrevNode, SfxItemPool& rPool, sal_Bool bKeepEndingAttribs )
{
    DBG_ASSERT( pPrevNode, "Copy of attributes to a null pointer?" );

    sal_uInt16 nCut = pPrevNode->Len();

    size_t nAttr = 0;
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
                DBG_ASSERT( pNewAttrib, "MakeCharAttrib failed!" );
                aCharAttribList.InsertAttrib( pNewAttrib );
            }
        }
        else if ( pAttrib->IsInside( nCut ) || ( !nCut && !pAttrib->GetStart() && !pAttrib->IsFeature() ) )
        {
            // If cut is done right at the front then the attribute must be
            // kept! Has to be copied and changed.
            EditCharAttrib* pNewAttrib = MakeCharAttrib( rPool, *(pAttrib->GetItem()), 0, pAttrib->GetEnd()-nCut );
            DBG_ASSERT( pNewAttrib, "MakeCharAttrib failed!" );
            aCharAttribList.InsertAttrib( pNewAttrib );
            pAttrib->GetEnd() = nCut;
        }
        else
        {
            // Move all attributes in the current node (this)
            CharAttribList::AttribsType::iterator it = rPrevAttribs.begin() + nAttr;
            aCharAttribList.InsertAttrib(rPrevAttribs.release(it).release());

            DBG_ASSERT( pAttrib->GetStart() >= nCut, "Start < nCut!" );
            DBG_ASSERT( pAttrib->GetEnd() >= nCut, "End < nCut!" );
            pAttrib->GetStart() = pAttrib->GetStart() - nCut;
            pAttrib->GetEnd() = pAttrib->GetEnd() - nCut;
            nAttr--;
        }
        nAttr++;
        pAttrib = GetAttrib(rPrevAttribs, nAttr);
    }
}

void ContentNode::AppendAttribs( ContentNode* pNextNode )
{
    DBG_ASSERT( pNextNode, "Copy of attributes to a null pointer?" );

    sal_uInt16 nNewStart = maString.Len();

#if OSL_DEBUG_LEVEL > 2
    OSL_ENSURE( aCharAttribList.DbgCheckAttribs(), "Attribute before AppendAttribs broken" );
#endif

    size_t nAttr = 0;
    CharAttribList::AttribsType& rNextAttribs = pNextNode->GetCharAttribs().GetAttribs();
    EditCharAttrib* pAttrib = GetAttrib(rNextAttribs, nAttr);
    while ( pAttrib )
    {
        // Move all attributes in the current node (this)
        bool bMelted = false;
        if ( ( pAttrib->GetStart() == 0 ) && ( !pAttrib->IsFeature() ) )
        {
            // Attributes can possibly be summarized as:
            size_t nTmpAttr = 0;
            EditCharAttrib* pTmpAttrib = GetAttrib( aCharAttribList.GetAttribs(), nTmpAttr );
            while ( !bMelted && pTmpAttrib )
            {
                if ( pTmpAttrib->GetEnd() == nNewStart )
                {
                    if ( ( pTmpAttrib->Which() == pAttrib->Which() ) &&
                         ( *(pTmpAttrib->GetItem()) == *(pAttrib->GetItem() ) ) )
                    {
                        pTmpAttrib->GetEnd() =
                            pTmpAttrib->GetEnd() + pAttrib->GetLen();
                        rNextAttribs.erase(rNextAttribs.begin()+nAttr);
                        // Unsubscribe from the pool?!
                        bMelted = true;
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
            aCharAttribList.InsertAttrib(rNextAttribs.release(it).release());
        }
        pAttrib = GetAttrib(rNextAttribs, nAttr);
    }
    // For the Attributes that just moved over:
    rNextAttribs.clear();

#if OSL_DEBUG_LEVEL > 2
    OSL_ENSURE( aCharAttribList.DbgCheckAttribs(), "Attribute after AppendAttribs broken" );
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
        GetContentAttribs().GetItems(), pS == NULL );
}

void ContentNode::SetStyleSheet( SfxStyleSheet* pS, const SvxFont& rFontFromStyle )
{
    aContentAttribs.SetStyleSheet( pS );


    // First use the information from the style ...
    GetCharAttribs().GetDefFont() = rFontFromStyle;
    // ... then iron out the hard paragraph formatting...
    CreateFont( GetCharAttribs().GetDefFont(),
        GetContentAttribs().GetItems(), pS == NULL );
}

void ContentNode::SetStyleSheet( SfxStyleSheet* pS, sal_Bool bRecalcFont )
{
    aContentAttribs.SetStyleSheet( pS );
    if ( bRecalcFont )
        CreateDefFont();
}

bool ContentNode::IsFeature( sal_uInt16 nPos ) const
{
    return maString.GetChar(nPos) == CH_FEATURE;
}

sal_uInt16 ContentNode::Len() const
{
    return maString.Len();
}

const XubString& ContentNode::GetString() const
{
    return maString;
}

void ContentNode::SetChar(sal_uInt16 nPos, sal_Unicode c)
{
    maString.SetChar(nPos, c);
}

void ContentNode::Insert(const XubString& rStr, sal_uInt16 nPos)
{
    maString.Insert(rStr, nPos);
}

void ContentNode::Append(const XubString& rStr)
{
    maString.Append(rStr);
}

void ContentNode::Erase(sal_uInt16 nPos)
{
    maString.Erase(nPos);
}

void ContentNode::Erase(sal_uInt16 nPos, sal_uInt16 nCount)
{
    maString.Erase(nPos, nCount);
}

XubString ContentNode::Copy(sal_uInt16 nPos) const
{
    return maString.Copy(nPos);
}

XubString ContentNode::Copy(sal_uInt16 nPos, sal_uInt16 nCount) const
{
    return maString.Copy(nPos, nCount);
}

sal_Unicode ContentNode::GetChar(sal_uInt16 nPos) const
{
    return maString.GetChar(nPos);
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
    DBG_ASSERT(!mpWrongList, "WrongList already exist!");
    mpWrongList.reset(p);
}

void ContentNode::CreateWrongList()
{
    DBG_ASSERT(!mpWrongList, "WrongList already exist!");
    mpWrongList.reset(new WrongList);
}

void ContentNode::DestroyWrongList()
{
    mpWrongList.reset();
}

ContentAttribs::ContentAttribs( SfxItemPool& rPool ) :
                    aAttribSet( rPool, EE_PARA_START, EE_CHAR_END )
{
    pStyle = 0;
}

ContentAttribs::ContentAttribs( const ContentAttribs& rRef ) :
                    aAttribSet( rRef.aAttribSet )
{
    pStyle = rRef.pStyle;
}

ContentAttribs::~ContentAttribs()
{
}

SvxTabStop ContentAttribs::FindTabStop( long nCurPos, sal_uInt16 nDefTab )
{
    const SvxTabStopItem& rTabs = (const SvxTabStopItem&) GetItem( EE_PARA_TABS );
    for ( sal_uInt16 i = 0; i < rTabs.Count(); i++ )
    {
        const SvxTabStop& rTab = rTabs[i];
        if ( rTab.GetTabPos() > nCurPos  )
            return rTab;
    }

    // Determine DefTab ...
    SvxTabStop aTabStop;
    long x = nCurPos / nDefTab + 1;
    aTabStop.GetTabPos() = nDefTab * x;
    return aTabStop;
}

void ContentAttribs::SetStyleSheet( SfxStyleSheet* pS )
{
    sal_Bool bStyleChanged = ( pStyle != pS );
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
            if ( ( nWhich != EE_PARA_BULLETSTATE ) && ( rStyleAttribs.GetItemState( nWhich ) == SFX_ITEM_ON ) )
                aAttribSet.ClearItem( nWhich );
        }
    }
}

const SfxPoolItem& ContentAttribs::GetItem( sal_uInt16 nWhich ) const
{
    // Hard paragraph attributes take precedence!
    const SfxItemSet* pTakeFrom = &aAttribSet;
    if ( pStyle && ( aAttribSet.GetItemState( nWhich, sal_False ) != SFX_ITEM_ON  ) )
        pTakeFrom = &pStyle->GetItemSet();

    return pTakeFrom->Get( nWhich );
}

bool ContentAttribs::HasItem( sal_uInt16 nWhich ) const
{
    bool bHasItem = false;
    if ( aAttribSet.GetItemState( nWhich, sal_False ) == SFX_ITEM_ON  )
        bHasItem = true;
    else if ( pStyle && pStyle->GetItemSet().GetItemState( nWhich ) == SFX_ITEM_ON )
        bHasItem = true;

    return bHasItem;
}


ItemList::ItemList() : CurrentItem( 0 )
{
}

const SfxPoolItem* ItemList::First()
{
    CurrentItem = 0;
    return aItemPool.empty() ? NULL : aItemPool[ 0 ];
}

const SfxPoolItem* ItemList::Next()
{
    if ( CurrentItem + 1 < aItemPool.size() )
    {
        ++CurrentItem;
        return aItemPool[ CurrentItem ];
    }
    return NULL;
}

void ItemList::Insert( const SfxPoolItem* pItem )
{
    aItemPool.push_back( pItem );
    CurrentItem = aItemPool.size() - 1;
}


EditDoc::EditDoc( SfxItemPool* pPool ) :
    nLastCache(0),
    pItemPool(pPool ? pPool : new EditEngineItemPool(false)),
    nDefTab(DEFTAB),
    bIsVertical(false),
    bIsFixedCellHeight(false),
    bOwnerOfPool(pPool ? false : true),
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

class RemoveEachItemFromPool : std::unary_function<ContentNode, void>
{
    EditDoc& mrDoc;
public:
    RemoveEachItemFromPool(EditDoc& rDoc) : mrDoc(rDoc) {}
    void operator() (const ContentNode& rNode)
    {
        mrDoc.RemoveItemsFromPool(rNode);
    }
};

struct ClearSpellErrorsHandler : std::unary_function<ContentNode, void>
{
    void operator() (ContentNode& rNode)
    {
        rNode.DestroyWrongList();
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
    for (size_t nAttr = 0; nAttr < rNode.GetCharAttribs().Count(); ++nAttr)
    {
        const EditCharAttrib& rAttr = rNode.GetCharAttribs().GetAttribs()[nAttr];
        GetItemPool().Remove(*rAttr.GetItem());
    }
}

void CreateFont( SvxFont& rFont, const SfxItemSet& rSet, bool bSearchInParent, short nScriptType )
{
    Font aPrevFont( rFont );
    rFont.SetAlign( ALIGN_BASELINE );
    rFont.SetTransparent( sal_True );

    sal_uInt16 nWhich_FontInfo = GetScriptItemId( EE_CHAR_FONTINFO, nScriptType );
    sal_uInt16 nWhich_Language = GetScriptItemId( EE_CHAR_LANGUAGE, nScriptType );
    sal_uInt16 nWhich_FontHeight = GetScriptItemId( EE_CHAR_FONTHEIGHT, nScriptType );
    sal_uInt16 nWhich_Weight = GetScriptItemId( EE_CHAR_WEIGHT, nScriptType );
    sal_uInt16 nWhich_Italic = GetScriptItemId( EE_CHAR_ITALIC, nScriptType );

    if ( bSearchInParent || ( rSet.GetItemState( nWhich_FontInfo ) == SFX_ITEM_ON ) )
    {
        const SvxFontItem& rFontItem = (const SvxFontItem&)rSet.Get( nWhich_FontInfo );
        rFont.SetName( rFontItem.GetFamilyName() );
        rFont.SetFamily( rFontItem.GetFamily() );
        rFont.SetPitch( rFontItem.GetPitch() );
        rFont.SetCharSet( rFontItem.GetCharSet() );
    }
    if ( bSearchInParent || ( rSet.GetItemState( nWhich_Language ) == SFX_ITEM_ON ) )
        rFont.SetLanguage( ((const SvxLanguageItem&)rSet.Get( nWhich_Language )).GetLanguage() );
    if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_COLOR ) == SFX_ITEM_ON ) )
        rFont.SetColor( ((const SvxColorItem&)rSet.Get( EE_CHAR_COLOR )).GetValue() );
    if ( bSearchInParent || ( rSet.GetItemState( nWhich_FontHeight ) == SFX_ITEM_ON ) )
        rFont.SetSize( Size( rFont.GetSize().Width(), ((const SvxFontHeightItem&)rSet.Get( nWhich_FontHeight ) ).GetHeight() ) );
    if ( bSearchInParent || ( rSet.GetItemState( nWhich_Weight ) == SFX_ITEM_ON ) )
        rFont.SetWeight( ((const SvxWeightItem&)rSet.Get( nWhich_Weight )).GetWeight() );
    if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_UNDERLINE ) == SFX_ITEM_ON ) )
        rFont.SetUnderline( ((const SvxUnderlineItem&)rSet.Get( EE_CHAR_UNDERLINE )).GetLineStyle() );
    if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_OVERLINE ) == SFX_ITEM_ON ) )
        rFont.SetOverline( ((const SvxOverlineItem&)rSet.Get( EE_CHAR_OVERLINE )).GetLineStyle() );
    if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_STRIKEOUT ) == SFX_ITEM_ON ) )
        rFont.SetStrikeout( ((const SvxCrossedOutItem&)rSet.Get( EE_CHAR_STRIKEOUT )).GetStrikeout() );
    if ( bSearchInParent || ( rSet.GetItemState( nWhich_Italic ) == SFX_ITEM_ON ) )
        rFont.SetItalic( ((const SvxPostureItem&)rSet.Get( nWhich_Italic )).GetPosture() );
    if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_OUTLINE ) == SFX_ITEM_ON ) )
        rFont.SetOutline( ((const SvxContourItem&)rSet.Get( EE_CHAR_OUTLINE )).GetValue() );
    if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_SHADOW ) == SFX_ITEM_ON ) )
        rFont.SetShadow( ((const SvxShadowedItem&)rSet.Get( EE_CHAR_SHADOW )).GetValue() );
    if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_ESCAPEMENT ) == SFX_ITEM_ON ) )
    {
        const SvxEscapementItem& rEsc = (const SvxEscapementItem&) rSet.Get( EE_CHAR_ESCAPEMENT );

        sal_uInt16 nProp = rEsc.GetProp();
        rFont.SetPropr( (sal_uInt8)nProp );

        short nEsc = rEsc.GetEsc();
        if ( nEsc == DFLT_ESC_AUTO_SUPER )
            nEsc = 100 - nProp;
        else if ( nEsc == DFLT_ESC_AUTO_SUB )
            nEsc = sal::static_int_cast< short >( -( 100 - nProp ) );
        rFont.SetEscapement( nEsc );
    }
    if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_PAIRKERNING ) == SFX_ITEM_ON ) )
        rFont.SetKerning( ((const SvxAutoKernItem&)rSet.Get( EE_CHAR_PAIRKERNING )).GetValue() );
    if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_KERNING ) == SFX_ITEM_ON ) )
        rFont.SetFixKerning( ((const SvxKerningItem&)rSet.Get( EE_CHAR_KERNING )).GetValue() );
    if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_WLM ) == SFX_ITEM_ON ) )
        rFont.SetWordLineMode( ((const SvxWordLineModeItem&)rSet.Get( EE_CHAR_WLM )).GetValue() );
    if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_EMPHASISMARK ) == SFX_ITEM_ON ) )
        rFont.SetEmphasisMark( ((const SvxEmphasisMarkItem&)rSet.Get( EE_CHAR_EMPHASISMARK )).GetValue() );
    if ( bSearchInParent || ( rSet.GetItemState( EE_CHAR_RELIEF ) == SFX_ITEM_ON ) )
        rFont.SetRelief( (FontRelief)((const SvxCharReliefItem&)rSet.Get( EE_CHAR_RELIEF )).GetValue() );

    // If comparing the entire font, or if checking before each alteration
    // whether the value changes, remains relatively the same thing.
    // So possible one MakeUniqFont more in the font, but as a result a quicker
    // abortion of the query, or one must each time check bChanged.
    if ( rFont == aPrevFont  )
        rFont = aPrevFont;  // => The same ImpPointer for IsSameInstance
}

void EditDoc::CreateDefFont( sal_Bool bUseStyles )
{
    SfxItemSet aTmpSet( GetItemPool(), EE_PARA_START, EE_CHAR_END );
    CreateFont( aDefFont, aTmpSet );
    aDefFont.SetVertical( IsVertical() );
    aDefFont.SetOrientation( IsVertical() ? 2700 : 0 );

    for ( sal_Int32 nNode = 0; nNode < Count(); nNode++ )
    {
        ContentNode* pNode = GetObject( nNode );
        pNode->GetCharAttribs().GetDefFont() = aDefFont;
        if ( bUseStyles )
            pNode->CreateDefFont();
    }
}

static const sal_Unicode aCR[] = { 0x0d, 0x00 };
static const sal_Unicode aLF[] = { 0x0a, 0x00 };
static const sal_Unicode aCRLF[] = { 0x0d, 0x0a, 0x00 };

sal_Int32 EditDoc::GetPos(const ContentNode* p) const
{
    return FastGetPos(maContents, p, nLastCache);
}

const ContentNode* EditDoc::GetObject(sal_Int32 nPos) const
{
    return 0 <= nPos && static_cast<size_t>(nPos) < maContents.size() ? &maContents[nPos] : NULL;
}

ContentNode* EditDoc::GetObject(sal_Int32 nPos)
{
    return 0 <= nPos && static_cast<size_t>(nPos) < maContents.size() ? &maContents[nPos] : NULL;
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
    maContents.insert(maContents.begin()+nPos, p);
}

void EditDoc::Remove(sal_Int32 nPos)
{
    if (nPos < 0 || static_cast<size_t>(nPos) >= maContents.size())
    {
        SAL_WARN( "editeng", "EditDoc::Remove - out of bounds pos " << nPos);
        return;
    }
    maContents.erase(maContents.begin() + nPos);
}

void EditDoc::Release(sal_Int32 nPos)
{
    if (nPos < 0 || static_cast<size_t>(nPos) >= maContents.size())
    {
        SAL_WARN( "editeng", "EditDoc::Release - out of bounds pos " << nPos);
        return;
    }
    maContents.release(maContents.begin() + nPos).release();
}

sal_Int32 EditDoc::Count() const
{
    size_t nSize = maContents.size();
    if (nSize > SAL_MAX_INT32)
    {
        SAL_WARN( "editeng", "EditDoc::Count - overflow " << nSize);
        return SAL_MAX_INT32;
    }
    return static_cast<sal_Int32>(nSize);
}

OUString EditDoc::GetSepStr( LineEnd eEnd )
{
    OUString aSep;
    if ( eEnd == LINEEND_CR )
        aSep = aCR;
    else if ( eEnd == LINEEND_LF )
        aSep = aLF;
    else
        aSep = aCRLF;
    return aSep;
}

XubString EditDoc::GetText( LineEnd eEnd ) const
{
    sal_uLong nLen = GetTextLen();
    sal_Int32 nNodes = Count();
    if (nNodes == 0)
        return OUString();

    OUString aSep = EditDoc::GetSepStr( eEnd );
    sal_Int32 nSepSize = aSep.getLength();

    if ( nSepSize )
        nLen += (nNodes - 1) * nSepSize;

    rtl_uString* newStr = rtl_uString_alloc(nLen);
    sal_Unicode* pCur = newStr->buffer;
    sal_Int32 nLastNode = nNodes-1;
    for ( sal_Int32 nNode = 0; nNode < nNodes; nNode++ )
    {
        String aTmp( GetParaAsString( GetObject(nNode) ) );
        memcpy( pCur, aTmp.GetBuffer(), aTmp.Len()*sizeof(sal_Unicode) );
        pCur += aTmp.Len();
        if ( nSepSize && ( nNode != nLastNode ) )
        {
            memcpy( pCur, aSep.getStr(), nSepSize*sizeof(sal_Unicode ) );
            pCur += nSepSize;
        }
    }
    assert(pCur - newStr->buffer == newStr->length);
    return OUString(newStr, SAL_NO_ACQUIRE);
}

XubString EditDoc::GetParaAsString( sal_Int32 nNode ) const
{
    return GetParaAsString( GetObject( nNode ) );
}

XubString EditDoc::GetParaAsString(
    const ContentNode* pNode, sal_uInt16 nStartPos, sal_uInt16 nEndPos, bool bResolveFields) const
{
    if ( nEndPos > pNode->Len() )
        nEndPos = pNode->Len();

    DBG_ASSERT( nStartPos <= nEndPos, "Start and End reversed?" );

    sal_uInt16 nIndex = nStartPos;
    XubString aStr;
    const EditCharAttrib* pNextFeature = pNode->GetCharAttribs().FindFeature( nIndex );
    while ( nIndex < nEndPos )
    {
        sal_uInt16 nEnd = nEndPos;
        if ( pNextFeature && ( pNextFeature->GetStart() < nEnd ) )
            nEnd = pNextFeature->GetStart();
        else
            pNextFeature = 0;   // Feature does not interest the below

        DBG_ASSERT( nEnd >= nIndex, "End in front of the index?" );
        //!! beware of sub string length  of -1 which is also defined as STRING_LEN and
        //!! thus would result in adding the whole sub string up to the end of the node !!
        if (nEnd > nIndex)
            aStr += XubString(pNode->GetString(), nIndex, nEnd - nIndex);

        if ( pNextFeature )
        {
            switch ( pNextFeature->GetItem()->Which() )
            {
                case EE_FEATURE_TAB:    aStr += '\t';
                break;
                case EE_FEATURE_LINEBR: aStr += '\x0A';
                break;
                case EE_FEATURE_FIELD:
                    if ( bResolveFields )
                        aStr += static_cast<const EditCharAttribField*>(pNextFeature)->GetFieldValue();
                break;
                default:    OSL_FAIL( "What feature?" );
            }
            pNextFeature = pNode->GetCharAttribs().FindFeature( ++nEnd );
        }
        nIndex = nEnd;
    }
    return aStr;
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
        nLen += pNode->Len();
        // Fields can be longer than the placeholder in the Node
        const CharAttribList::AttribsType& rAttrs = pNode->GetCharAttribs().GetAttribs();
        for (size_t nAttr = rAttrs.size(); nAttr; )
        {
            const EditCharAttrib& rAttr = rAttrs[--nAttr];
            if (rAttr.Which() == EE_FEATURE_FIELD)
            {
                sal_Int32 nFieldLen = static_cast<const EditCharAttribField&>(rAttr).GetFieldValue().getLength();
                if ( !nFieldLen )
                    nLen--;
                else
                    nLen += nFieldLen-1;
            }
        }
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

    EditPaM aPaM( pNode, 0 );
    return aPaM;
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
        aModifyHdl.Call( NULL );
    }
}

EditPaM EditDoc::RemoveText()
{
    // Keep the old ItemSet, to keep the chart Font.
    ContentNode* pPrevFirstNode = GetObject(0);
    SfxStyleSheet* pPrevStyle = pPrevFirstNode->GetStyleSheet();
    SfxItemSet aPrevSet( pPrevFirstNode->GetContentAttribs().GetItems() );
    Font aPrevFont( pPrevFirstNode->GetCharAttribs().GetDefFont() );

    ImplDestroyContents();

    ContentNode* pNode = new ContentNode( GetItemPool() );
    Insert(0, pNode);

    pNode->SetStyleSheet(pPrevStyle, false);
    pNode->GetContentAttribs().GetItems().Set( aPrevSet );
    pNode->GetCharAttribs().GetDefFont() = aPrevFont;

    SetModified(true);

    EditPaM aPaM( pNode, 0 );
    return aPaM;
}

EditPaM EditDoc::InsertText( EditPaM aPaM, const XubString& rStr )
{
    DBG_ASSERT( rStr.Search( 0x0A ) == STRING_NOTFOUND, "EditDoc::InsertText: Newlines prohibited in paragraph!" );
    DBG_ASSERT( rStr.Search( 0x0D ) == STRING_NOTFOUND, "EditDoc::InsertText: Newlines prohibited in paragraph!" );
    DBG_ASSERT( rStr.Search( '\t' ) == STRING_NOTFOUND, "EditDoc::InsertText: Newlines prohibited in paragraph!" );
    DBG_ASSERT( aPaM.GetNode(), "Blinder PaM in EditDoc::InsertText1" );

    aPaM.GetNode()->Insert( rStr, aPaM.GetIndex() );
    aPaM.GetNode()->ExpandAttribs( aPaM.GetIndex(), rStr.Len(), GetItemPool() );
    aPaM.GetIndex() = aPaM.GetIndex() + rStr.Len();

    SetModified( sal_True );

    return aPaM;
}

EditPaM EditDoc::InsertParaBreak( EditPaM aPaM, sal_Bool bKeepEndingAttribs )
{
    DBG_ASSERT( aPaM.GetNode(), "Blinder PaM in EditDoc::InsertParaBreak" );
    ContentNode* pCurNode = aPaM.GetNode();
    sal_Int32 nPos = GetPos( pCurNode );
    XubString aStr = aPaM.GetNode()->Copy( aPaM.GetIndex() );
    aPaM.GetNode()->Erase( aPaM.GetIndex() );

    // the paragraph attributes...
    ContentAttribs aContentAttribs( aPaM.GetNode()->GetContentAttribs() );

    // for a new paragraph we like to have the bullet/numbering visible by default
    aContentAttribs.GetItems().Put( SfxBoolItem( EE_PARA_BULLETSTATE, sal_True), EE_PARA_BULLETSTATE );

    // ContenNode constructor copies also the paragraph attributes
    ContentNode* pNode = new ContentNode( aStr, aContentAttribs );

    // Copy the Default Font
    pNode->GetCharAttribs().GetDefFont() = aPaM.GetNode()->GetCharAttribs().GetDefFont();
    SfxStyleSheet* pStyle = aPaM.GetNode()->GetStyleSheet();
    if ( pStyle )
    {
        XubString aFollow( pStyle->GetFollow() );
        if ( aFollow.Len() && ( aFollow != pStyle->GetName() ) )
        {
            SfxStyleSheetBase* pNext = pStyle->GetPool().Find( aFollow, pStyle->GetFamily() );
            pNode->SetStyleSheet( (SfxStyleSheet*)pNext );
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
    DBG_ASSERT( aPaM.GetNode(), "Blinder PaM in EditDoc::InsertFeature" );

    aPaM.GetNode()->Insert( OUString(CH_FEATURE), aPaM.GetIndex() );
    aPaM.GetNode()->ExpandAttribs( aPaM.GetIndex(), 1, GetItemPool() );

    // Create a feature-attribute for the feature...
    EditCharAttrib* pAttrib = MakeCharAttrib( GetItemPool(), rItem, aPaM.GetIndex(), aPaM.GetIndex()+1 );
    DBG_ASSERT( pAttrib, "Why can not the feature be created?" );
    aPaM.GetNode()->GetCharAttribs().InsertAttrib( pAttrib );

    SetModified( sal_True );

    aPaM.GetIndex()++;
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

EditPaM EditDoc::RemoveChars( EditPaM aPaM, sal_uInt16 nChars )
{
    // Maybe remove Features!
    aPaM.GetNode()->Erase( aPaM.GetIndex(), nChars );
    aPaM.GetNode()->CollapsAttribs( aPaM.GetIndex(), nChars, GetItemPool() );

    SetModified( sal_True );

    return aPaM;
}

void EditDoc::InsertAttribInSelection( ContentNode* pNode, sal_uInt16 nStart, sal_uInt16 nEnd, const SfxPoolItem& rPoolItem )
{
    DBG_ASSERT( pNode, "What to do with the attribute?" );
    DBG_ASSERT( nEnd <= pNode->Len(), "InsertAttrib: Attribute to large!" );

    // for Optimization:
    // This ends at the beginning of the selection => can be expanded
    EditCharAttrib* pEndingAttrib = 0;
    // This starts at the end of the selection => can be expanded
    EditCharAttrib* pStartingAttrib = 0;

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

sal_Bool EditDoc::RemoveAttribs( ContentNode* pNode, sal_uInt16 nStart, sal_uInt16 nEnd, sal_uInt16 nWhich )
{
    EditCharAttrib* pStarting;
    EditCharAttrib* pEnding;
    return RemoveAttribs( pNode, nStart, nEnd, pStarting, pEnding, nWhich );
}

sal_Bool EditDoc::RemoveAttribs( ContentNode* pNode, sal_uInt16 nStart, sal_uInt16 nEnd, EditCharAttrib*& rpStarting, EditCharAttrib*& rpEnding, sal_uInt16 nWhich )
{

    DBG_ASSERT( pNode, "What to do with the attribute?" );
    DBG_ASSERT( nEnd <= pNode->Len(), "InsertAttrib: Attribute to large!" );

    // This ends at the beginning of the selection => can be expanded
    rpEnding = 0;
    // This starts at the end of the selection => can be expanded
    rpStarting = 0;

    bool bChanged = false;

    DBG_ASSERT( nStart <= nEnd, "Small miscalculations in InsertAttribInSelection" );

    // iterate over the attributes ...
    size_t nAttr = 0;
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
                bChanged = sal_True;
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
                    bRemoveAttrib = sal_True;
                }
            }

            // Attribute ends in Selection
            else if ( ( pAttr->GetEnd() >= nStart ) && ( pAttr->GetEnd() <= nEnd ) )
            {
                bChanged = sal_True;
                if ( ( pAttr->GetStart() < nStart ) && !pAttr->IsFeature() )
                {
                    pAttr->GetEnd() = nStart;   // then it ends here
                    rpEnding = pAttr;
                }
                else if ( !pAttr->IsFeature() || ( pAttr->GetStart() == nStart ) )
                {
                    // Delete feature only if on the exact spot
                    bRemoveAttrib = sal_True;
                }
            }
            // Attribute overlaps the selection
            else if ( ( pAttr->GetStart() <= nStart ) && ( pAttr->GetEnd() >= nEnd ) )
            {
                bChanged = sal_True;
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
                    sal_uInt16 nOldEnd = pAttr->GetEnd();
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

    return bChanged;
}

void EditDoc::InsertAttrib( const SfxPoolItem& rPoolItem, ContentNode* pNode, sal_uInt16 nStart, sal_uInt16 nEnd )
{
    // This method no longer checks whether a corresponding attribute already
    // exists at this place!
    EditCharAttrib* pAttrib = MakeCharAttrib( GetItemPool(), rPoolItem, nStart, nEnd );
    DBG_ASSERT( pAttrib, "MakeCharAttrib failed!" );
    pNode->GetCharAttribs().InsertAttrib( pAttrib );

    SetModified( sal_True );
}

void EditDoc::InsertAttrib( ContentNode* pNode, sal_uInt16 nStart, sal_uInt16 nEnd, const SfxPoolItem& rPoolItem )
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
            rAttrList.Release(pAttr);
        }

        // check whether 'the same' attribute exist at this place.
        pAttr = rAttrList.FindAttrib( rPoolItem.Which(), nStart );
        if ( pAttr )
        {
            if ( pAttr->IsInside( nStart ) )    // split
            {
                // check again if really splitting, or return !
                sal_uInt16 nOldEnd = pAttr->GetEnd();
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

    SetModified( sal_True );
}

void EditDoc::FindAttribs( ContentNode* pNode, sal_uInt16 nStartPos, sal_uInt16 nEndPos, SfxItemSet& rCurSet )
{
    DBG_ASSERT( pNode, "Where to search?" );
    DBG_ASSERT( nStartPos <= nEndPos, "Invalid region!" );

    sal_uInt16 nAttr = 0;
    EditCharAttrib* pAttr = GetAttrib( pNode->GetCharAttribs().GetAttribs(), nAttr );
    // No Selection...
    if ( nStartPos == nEndPos )
    {
        while ( pAttr && ( pAttr->GetStart() <= nEndPos) )
        {
            const SfxPoolItem* pItem = 0;
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
                if ( rCurSet.GetItemState( nWhich ) == SFX_ITEM_OFF )
                {
                    rCurSet.Put( *pItem );
                }
                else if ( rCurSet.GetItemState( nWhich ) == SFX_ITEM_ON )
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
            const SfxPoolItem* pItem = 0;
            // Attribut is about...
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
                // breaki point, which is quite expensive.
                // Since optimazation is done when inserting the  attributes
                // this case does not appear so fast ...
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
                if ( rCurSet.GetItemState( nWhich ) == SFX_ITEM_OFF )
                {
                    rCurSet.Put( *pItem );
                }
                else if ( rCurSet.GetItemState( nWhich ) == SFX_ITEM_ON )
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

namespace {

struct LessByStart : std::binary_function<EditCharAttrib, EditCharAttrib, bool>
{
    bool operator() (const EditCharAttrib& left, const EditCharAttrib& right) const
    {
        return left.GetStart() < right.GetStart();
    }
};

}

CharAttribList::CharAttribList()
{
    DBG_CTOR( EE_CharAttribList, 0 );
    bHasEmptyAttribs = sal_False;
}

CharAttribList::~CharAttribList()
{
    DBG_DTOR( EE_CharAttribList, 0 );
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

    const sal_uInt16 nStart = pAttrib->GetStart(); // may be better for Comp.Opt.

    if ( pAttrib->IsEmpty() )
        bHasEmptyAttribs = true;

    for (size_t i = 0, n = aAttribs.size(); i < n; ++i)
    {
        const EditCharAttrib& rCurAttrib = aAttribs[i];
        if (rCurAttrib.GetStart() > nStart)
        {
            aAttribs.insert(aAttribs.begin()+i, pAttrib);
            return;
        }
    }

    aAttribs.push_back(pAttrib);
}

void CharAttribList::ResortAttribs()
{
    aAttribs.sort(LessByStart());
}

void CharAttribList::OptimizeRanges( SfxItemPool& rItemPool )
{
    for (size_t i = 0; i < aAttribs.size(); ++i)
    {
        EditCharAttrib& rAttr = aAttribs[i];
        for (size_t nNext = i+1; nNext < aAttribs.size(); ++nNext)
        {
            EditCharAttrib& rNext = aAttribs[nNext];
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
}

size_t CharAttribList::Count() const
{
    return aAttribs.size();
}

const EditCharAttrib* CharAttribList::FindAttrib( sal_uInt16 nWhich, sal_uInt16 nPos ) const
{
    // Backwards, if one ends where the next starts.
    // => The starting one is the valid one ...
    AttribsType::const_reverse_iterator it = aAttribs.rbegin(), itEnd = aAttribs.rend();
    for (; it != itEnd; ++it)
    {
        const EditCharAttrib& rAttr = *it;
        if (rAttr.Which() == nWhich && rAttr.IsIn(nPos))
            return &rAttr;
    }
    return NULL;
}

EditCharAttrib* CharAttribList::FindAttrib( sal_uInt16 nWhich, sal_uInt16 nPos )
{
    // Backwards, if one ends where the next starts.
    // => The starting one is the valid one ...
    AttribsType::reverse_iterator it = aAttribs.rbegin(), itEnd = aAttribs.rend();
    for (; it != itEnd; ++it)
    {
        EditCharAttrib& rAttr = *it;
        if (rAttr.Which() == nWhich && rAttr.IsIn(nPos))
            return &rAttr;
    }
    return NULL;
}

const EditCharAttrib* CharAttribList::FindNextAttrib( sal_uInt16 nWhich, sal_uInt16 nFromPos ) const
{
    DBG_ASSERT( nWhich, "FindNextAttrib: Which?" );
    AttribsType::const_iterator it = aAttribs.begin(), itEnd = aAttribs.end();
    for (; it != itEnd; ++it)
    {
        const EditCharAttrib& rAttr = *it;
        if (rAttr.GetStart() >= nFromPos && rAttr.Which() == nWhich)
            return &rAttr;
    }
    return NULL;
}

bool CharAttribList::HasAttrib( sal_uInt16 nStartPos, sal_uInt16 nEndPos ) const
{
    AttribsType::const_reverse_iterator it = aAttribs.rbegin(), itEnd = aAttribs.rend();
    for (; it != itEnd; ++it)
    {
        const EditCharAttrib& rAttr = *it;
        if (rAttr.GetStart() < nEndPos && rAttr.GetEnd() > nStartPos)
            return true;
    }
    return false;
}

CharAttribList::AttribsType& CharAttribList::GetAttribs()
{
    return aAttribs;
}

const CharAttribList::AttribsType& CharAttribList::GetAttribs() const
{
    return aAttribs;
}

namespace {

class FindByAddress : std::unary_function<EditCharAttrib, bool>
{
    const EditCharAttrib* mpAttr;
public:
    FindByAddress(const EditCharAttrib* p) : mpAttr(p) {}
    bool operator() (const EditCharAttrib& r) const
    {
        return &r == mpAttr;
    }
};

}

void CharAttribList::Remove(const EditCharAttrib* p)
{
    AttribsType::iterator it = std::find_if(aAttribs.begin(), aAttribs.end(), FindByAddress(p));
    if (it != aAttribs.end())
        aAttribs.erase(it);
}

void CharAttribList::Remove(size_t nPos)
{
    if (nPos >= aAttribs.size())
        return;

    aAttribs.erase(aAttribs.begin()+nPos);
}

void CharAttribList::Release(const EditCharAttrib* p)
{
    AttribsType::iterator it = std::find_if(aAttribs.begin(), aAttribs.end(), FindByAddress(p));
    if (it != aAttribs.end())
        aAttribs.release(it).release();
}

void CharAttribList::SetHasEmptyAttribs(bool b)
{
    bHasEmptyAttribs = b;
}

bool CharAttribList::HasBoundingAttrib( sal_uInt16 nBound ) const
{
    // Backwards, if one ends where the next starts.
    // => The starting one is the valid one ...
    AttribsType::const_reverse_iterator it = aAttribs.rbegin(), itEnd = aAttribs.rend();
    for (; it != itEnd; ++it)
    {
        const EditCharAttrib& rAttr = *it;
        if (rAttr.GetEnd() < nBound)
            return false;

        if (rAttr.GetStart() == nBound || rAttr.GetEnd() == nBound)
            return true;
    }
    return false;
}

const EditCharAttrib* CharAttribList::FindEmptyAttrib( sal_uInt16 nWhich, sal_uInt16 nPos ) const
{
    if ( !bHasEmptyAttribs )
        return NULL;

    AttribsType::const_iterator it = aAttribs.begin(), itEnd = aAttribs.end();
    for (; it != itEnd; ++it)
    {
        const EditCharAttrib& rAttr = *it;
        if (rAttr.GetStart() == nPos && rAttr.GetEnd() == nPos && rAttr.Which() == nWhich)
            return &rAttr;
    }
    return NULL;
}

EditCharAttrib* CharAttribList::FindEmptyAttrib( sal_uInt16 nWhich, sal_uInt16 nPos )
{
    if ( !bHasEmptyAttribs )
        return NULL;

    AttribsType::iterator it = aAttribs.begin(), itEnd = aAttribs.end();
    for (; it != itEnd; ++it)
    {
        EditCharAttrib& rAttr = *it;
        if (rAttr.GetStart() == nPos && rAttr.GetEnd() == nPos && rAttr.Which() == nWhich)
            return &rAttr;
    }
    return NULL;
}

namespace {

class FindByStartPos : std::unary_function<EditCharAttrib, bool>
{
    sal_uInt16 mnPos;
public:
    FindByStartPos(sal_uInt16 nPos) : mnPos(nPos) {}
    bool operator() (const EditCharAttrib& r) const
    {
        return r.GetStart() >= mnPos;
    }
};

}

const EditCharAttrib* CharAttribList::FindFeature( sal_uInt16 nPos ) const
{
    // First, find the first attribute that starts at or after specified position.
    AttribsType::const_iterator it =
        std::find_if(aAttribs.begin(), aAttribs.end(), FindByStartPos(nPos));

    if (it == aAttribs.end())
        // All attributes are before the specified position.
        return NULL;

    // And find the first attribute with feature.
    it = std::find_if(it, aAttribs.end(), boost::bind(&EditCharAttrib::IsFeature, _1) == true);
    return it == aAttribs.end() ? NULL : &(*it);
}

namespace {

class RemoveEmptyAttrItem : std::unary_function<EditCharAttrib, void>
{
    SfxItemPool& mrItemPool;
public:
    RemoveEmptyAttrItem(SfxItemPool& rPool) : mrItemPool(rPool) {}
    void operator() (const EditCharAttrib& r)
    {
        if (r.IsEmpty())
            mrItemPool.Remove(*r.GetItem());
    }
};

}

void CharAttribList::DeleteEmptyAttribs( SfxItemPool& rItemPool )
{
    std::for_each(aAttribs.begin(), aAttribs.end(), RemoveEmptyAttrItem(rItemPool));
    aAttribs.erase_if(boost::bind(&EditCharAttrib::IsEmpty, _1) == true);
    bHasEmptyAttribs = false;
}

#if OSL_DEBUG_LEVEL > 2
bool CharAttribList::DbgCheckAttribs() const
{
    bool bOK = true;
    AttribsType::const_iterator it = aAttribs.begin(), itEnd = aAttribs.end();
    for (; it != itEnd; ++it)
    {
        const EditCharAttrib& rAttr = *it;
        if (rAttr.GetStart() > rAttr.GetEnd())
        {
            bOK = false;
            OSL_FAIL( "Attribute is distorted" );
        }
        else if (rAttr.IsFeature() && rAttr.GetLen() != 1)
        {
            bOK = false;
            OSL_FAIL( "Feature, Len != 1" );
        }
    }
    return bOK;
}
#endif


SvxColorList::SvxColorList()
{
}

SvxColorList::~SvxColorList()
{
    for ( size_t i = 0, n = aColorList.size(); i < n; ++i )
        delete aColorList[ i ];
    aColorList.clear();
}

size_t SvxColorList::GetId( const SvxColorItem& rColorItem )
{
    for ( size_t i = 0, n = aColorList.size(); i < n; ++i )
        if ( *aColorList[ i ] == rColorItem )
            return i;
    DBG_WARNING( "Color not found: GetId()" );
    return 0;
}

void SvxColorList::Insert( SvxColorItem* pItem, size_t nIndex )
{
    if ( nIndex >= aColorList.size() )
    {
        aColorList.push_back( pItem );
    }
    else
    {
        DummyColorList::iterator it = aColorList.begin();
        ::std::advance( it, nIndex );
        aColorList.insert( it, pItem );
    }
}

SvxColorItem* SvxColorList::GetObject( size_t nIndex )
{
    return ( nIndex >= aColorList.size() ) ? NULL : aColorList[ nIndex ];
}

EditEngineItemPool::EditEngineItemPool( sal_Bool bPersistenRefCounts )
    : SfxItemPool( String( "EditEngineItemPool", RTL_TEXTENCODING_ASCII_US ), EE_ITEMS_START, EE_ITEMS_END,
                    aItemInfos, 0, bPersistenRefCounts )
{
    SetVersionMap( 1, 3999, 4015, aV1Map );
    SetVersionMap( 2, 3999, 4019, aV2Map );
    SetVersionMap( 3, 3997, 4020, aV3Map );
    SetVersionMap( 4, 3994, 4022, aV4Map );
    SetVersionMap( 5, 3994, 4037, aV5Map );

    SfxPoolItem** ppDefItems = EE_DLL().GetGlobalData()->GetDefItems();
    SetDefaults( ppDefItems );
}

EditEngineItemPool::~EditEngineItemPool()
{
}

SvStream& EditEngineItemPool::Store( SvStream& rStream ) const
{
    // for a 3.1 export a hack has to be installed, as in there is a BUG in
    // SfxItemSet::Load, but not subsequently after 3.1.

    // The selected range must be kept after Store, because itemsets are not
    // stored until then...

    long nVersion = rStream.GetVersion();
    sal_Bool b31Format = ( nVersion && ( nVersion <= SOFFICE_FILEFORMAT_31 ) )
                        ? sal_True : sal_False;

    EditEngineItemPool* pThis = (EditEngineItemPool*)this;
    if ( b31Format )
        pThis->SetStoringRange( 3997, 4022 );
    else
        pThis->SetStoringRange( EE_ITEMS_START, EE_ITEMS_END );

    return SfxItemPool::Store( rStream );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
