/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <vcl/wrkwin.hxx>
#include <vcl/dialog.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>

#include <editeng/tstpitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/crsditem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/cntritem.hxx>
#include <editeng/escpitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/akrnitem.hxx>
#include <editeng/cscoitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/emphitem.hxx>
#include <editeng/charscaleitem.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/xmlcnitm.hxx>
#include <editeng/editids.hrc>

#include <editdoc.hxx>
#include <editdbg.hxx>
#include <editeng/eerdll.hxx>
#include <eerdll2.hxx>
#include <tools/stream.hxx>
#include <tools/debug.hxx>
#include <tools/shl.hxx>
#include <vcl/svapp.hxx>
#include <com/sun/star/i18n/ScriptType.hpp>
#include <stdlib.h> // qsort

using namespace ::com::sun::star;


// ------------------------------------------------------------

USHORT GetScriptItemId( USHORT nItemId, short nScriptType )
{
    USHORT nId = nItemId;

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

BOOL IsScriptItemValid( USHORT nItemId, short nScriptType )
{
    BOOL bValid = TRUE;

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

SfxItemInfo aItemInfos[EDITITEMCOUNT] = {
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

USHORT aV1Map[] = {
    3999, 4001, 4002, 4003, 4004, 4005, 4006,
    4007, 4008, 4009, 4010, 4011, 4012, 4013, 4017, 4018, 4019 // MI: 4019?
};

USHORT aV2Map[] = {
    3999, 4000, 4001, 4002, 4003, 4004, 4005, 4006, 4007, 4008, 4009,
    4010, 4011, 4012, 4013, 4014, 4015, 4016, 4018, 4019, 4020
};

USHORT aV3Map[] = {
    3997, 3998, 3999, 4000, 4001, 4002, 4003, 4004, 4005, 4006, 4007,
    4009, 4010, 4011, 4012, 4013, 4014, 4015, 4016, 4017, 4018, 4019,
    4020, 4021
};

USHORT aV4Map[] = {
    3994, 3995, 3996, 3997, 3998, 3999, 4000, 4001, 4002, 4003,
    4004, 4005, 4006, 4007, 4008, 4009, 4010, 4011, 4012, 4013,
    4014, 4015, 4016, 4017, 4018,
    /* CJK Items inserted here: EE_CHAR_LANGUAGE - EE_CHAR_XMLATTRIBS */
    4034, 4035, 4036, 4037
};

USHORT aV5Map[] = {
    3994, 3995, 3996, 3997, 3998, 3999, 4000, 4001, 4002, 4003,
    4004, 4005, 4006, 4007, 4008, 4009, 4010, 4011, 4012, 4013,
    4014, 4015, 4016, 4017, 4018, 4019, 4020, 4021, 4022, 4023,
    4024, 4025, 4026, 4027, 4028, 4029, 4030, 4031, 4032, 4033,
    /* EE_CHAR_OVERLINE inserted here */
    4035, 4036, 4037, 4038
};

SV_IMPL_PTRARR( DummyContentList, ContentNode* );
SV_IMPL_VARARR( ScriptTypePosInfos, ScriptTypePosInfo );
SV_IMPL_VARARR( WritingDirectionInfos, WritingDirectionInfo );
// SV_IMPL_VARARR( ExtraCharInfos, ExtraCharInfo );


int SAL_CALL CompareStart( const void* pFirst, const void* pSecond )
{
    if ( (*((EditCharAttrib**)pFirst))->GetStart() < (*((EditCharAttrib**)pSecond))->GetStart() )
        return (-1);
    else if ( (*((EditCharAttrib**)pFirst))->GetStart() > (*((EditCharAttrib**)pSecond))->GetStart() )
        return (1);
    return 0;
}

EditCharAttrib* MakeCharAttrib( SfxItemPool& rPool, const SfxPoolItem& rAttr, USHORT nS, USHORT nE )
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

EditLine::EditLine()
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
    bHangingPunctuation = FALSE;
    bInvalid = TRUE;
}

EditLine::EditLine( const EditLine& r )
{
    DBG_CTOR( EE_EditLine, 0 );

    nEnd = r.nEnd;
    nStart = r.nStart;
    nStartPortion = r.nStartPortion;
    nEndPortion = r.nEndPortion;
    bHangingPunctuation = r.bHangingPunctuation;

    nHeight = 0;
    nStartPosX = 0;
    nTxtHeight = 0;
    nTxtWidth = 0;
    nCrsrHeight = 0;
    nMaxAscent = 0;
    bInvalid = TRUE;
}

EditLine::~EditLine()
{
    DBG_DTOR( EE_EditLine, 0 );
}

EditLine* EditLine::Clone() const
{
    EditLine* pL = new EditLine;
    if ( aPositions.Count() )
    {
        pL->aPositions.Insert (aPositions.GetData(), aPositions.Count(), 0);
    }
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

BOOL operator == ( const EditLine& r1,  const EditLine& r2  )
{
    if ( r1.nStart != r2.nStart )
        return FALSE;

    if ( r1.nEnd != r2.nEnd )
        return FALSE;

    if ( r1.nStartPortion != r2.nStartPortion )
        return FALSE;

    if ( r1.nEndPortion != r2.nEndPortion )
        return FALSE;

    return TRUE;
}

EditLine& EditLine::operator = ( const EditLine& r )
{
    nEnd = r.nEnd;
    nStart = r.nStart;
    nEndPortion = r.nEndPortion;
    nStartPortion = r.nStartPortion;
    return *this;
}


BOOL operator != ( const EditLine& r1,  const EditLine& r2  )
{
    return !( r1 == r2 );
}

Size EditLine::CalcTextSize( ParaPortion& rParaPortion )
{
    Size aSz;
    Size aTmpSz;
    TextPortion* pPortion;

    USHORT nIndex = GetStart();

    DBG_ASSERT( rParaPortion.GetTextPortions().Count(), "GetTextSize before CreatePortions !" );

    for ( USHORT n = nStartPortion; n <= nEndPortion; n++ )
    {
        pPortion = rParaPortion.GetTextPortions().GetObject(n);
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

    SetHeight( (USHORT)aSz.Height() );
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
    for ( USHORT nLine = 0; nLine < Count(); nLine++ )
        delete GetObject(nLine);
    Remove( 0, Count() );
}

void EditLineList::DeleteFromLine( USHORT nDelFrom )
{
    DBG_ASSERT( nDelFrom <= (Count() - 1), "DeleteFromLine: Out of range" );
    for ( USHORT nL = nDelFrom; nL < Count(); nL++ )
        delete GetObject(nL);
    Remove( nDelFrom, Count()-nDelFrom );
}

USHORT EditLineList::FindLine( USHORT nChar, BOOL bInclEnd )
{
    for ( USHORT nLine = 0; nLine < Count(); nLine++ )
    {
        EditLine* pLine = GetObject( nLine );
        if ( ( bInclEnd && ( pLine->GetEnd() >= nChar ) ) ||
             ( pLine->GetEnd() > nChar ) )
        {
            return nLine;
        }
    }

    DBG_ASSERT( !bInclEnd, "Line not found: FindLine" );
    return ( Count() - 1 );
}

BOOL EditPaM::DbgIsBuggy( EditDoc& rDoc )
{
    if ( !pNode )
        return TRUE;
    if ( rDoc.GetPos( pNode ) >= rDoc.Count() )
        return TRUE;
    if ( nIndex > pNode->Len() )
        return TRUE;

    return FALSE;
}

BOOL EditSelection::DbgIsBuggy( EditDoc& rDoc )
{
    if ( aStartPaM.DbgIsBuggy( rDoc ) )
        return TRUE;
    if ( aEndPaM.DbgIsBuggy( rDoc ) )
        return TRUE;

    return FALSE;
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

BOOL EditSelection::IsInvalid() const
{
    EditPaM aEmptyPaM;

    if ( aStartPaM == aEmptyPaM )
        return TRUE;

    if ( aEndPaM == aEmptyPaM )
        return TRUE;

    return FALSE;
}

BOOL EditSelection::Adjust( const ContentList& rNodes )
{
    DBG_ASSERT( aStartPaM.GetIndex() <= aStartPaM.GetNode()->Len(), "Index out of range in Adjust(1)" );
    DBG_ASSERT( aEndPaM.GetIndex() <= aEndPaM.GetNode()->Len(), "Index out of range in Adjust(2)" );

    ContentNode* pStartNode = aStartPaM.GetNode();
    ContentNode* pEndNode = aEndPaM.GetNode();

    USHORT nStartNode = rNodes.GetPos( pStartNode );
    USHORT nEndNode = rNodes.GetPos( pEndNode );

    DBG_ASSERT( nStartNode != USHRT_MAX, "Node out of range in Adjust(1)" );
    DBG_ASSERT( nEndNode != USHRT_MAX, "Node out of range in Adjust(2)" );

    BOOL bSwap = FALSE;
    if ( nStartNode > nEndNode )
        bSwap = TRUE;
    else if ( ( nStartNode == nEndNode ) && ( aStartPaM.GetIndex() > aEndPaM.GetIndex() ) )
        bSwap = TRUE;

    if ( bSwap )
    {
        EditPaM aTmpPaM( aStartPaM );
        aStartPaM = aEndPaM;
        aEndPaM = aTmpPaM;
    }

    return bSwap;
}

BOOL operator == ( const EditPaM& r1,  const EditPaM& r2  )
{
    if ( r1.GetNode() != r2.GetNode() )
        return FALSE;

    if ( r1.GetIndex() != r2.GetIndex() )
        return FALSE;

    return TRUE;
}

EditPaM& EditPaM::operator = ( const EditPaM& rPaM )
{
    nIndex = rPaM.nIndex;
    pNode = rPaM.pNode;
    return *this;
}

BOOL operator != ( const EditPaM& r1,  const EditPaM& r2  )
{
    return !( r1 == r2 );
}

ContentNode::ContentNode( SfxItemPool& rPool ) : aContentAttribs( rPool )
{
    DBG_CTOR( EE_ContentNode, 0 );
    pWrongList = NULL;
}

ContentNode::ContentNode( const XubString& rStr, const ContentAttribs& rContentAttribs ) :
    XubString( rStr ), aContentAttribs( rContentAttribs )
{
    DBG_CTOR( EE_ContentNode, 0 );
    pWrongList = NULL;
}

ContentNode::~ContentNode()
{
    DBG_DTOR( EE_ContentNode, 0 );
    delete pWrongList;
}

void ContentNode::ExpandAttribs( USHORT nIndex, USHORT nNew, SfxItemPool& rItemPool )
{
    if ( !nNew )
        return;

    // Since features are treated differently than normal character attributes,
    // can also the order of the start list be change!
    // In every if ...,  in the next (n) opportunities due to bFeature or
    // an existing special case, must (n-1) opportunities be provided with
    // bResort. The most likely possibility receives no bResort, so that is
    // not sorted anew when all attributes are the same.
    BOOL bResort = FALSE;
    BOOL bExpandedEmptyAtIndexNull = FALSE;

    USHORT nAttr = 0;
    EditCharAttrib* pAttrib = GetAttrib( aCharAttribList.GetAttribs(), nAttr );
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
                    bExpandedEmptyAtIndexNull = TRUE;
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
                    bResort = TRUE;
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
                    bResort = TRUE;
                }
                else
                {
                    BOOL bExpand = FALSE;
                    if ( nIndex == 0 )
                    {
                        bExpand = TRUE;
                        if( bExpandedEmptyAtIndexNull )
                        {
                            // Check if this kind of attribut was empty and expanded here...
                            USHORT nW = pAttrib->GetItem()->Which();
                            for ( USHORT nA = 0; nA < nAttr; nA++ )
                            {
                                EditCharAttrib* pA = aCharAttribList.GetAttribs()[nA];
                                if ( ( pA->GetStart() == 0 ) && ( pA->GetItem()->Which() == nW ) )
                                {
                                    bExpand = FALSE;
                                    break;
                                }
                            }

                        }
                    }
                    if ( bExpand )
                    {
                        pAttrib->Expand( nNew );
                        bResort = TRUE;
                    }
                    else
                    {
                        pAttrib->MoveForward( nNew );
                    }
                }
            }
        }

        if ( pAttrib->IsEdge() )
            pAttrib->SetEdge( FALSE );

        DBG_ASSERT( !pAttrib->IsFeature() || ( pAttrib->GetLen() == 1 ), "Expand: FeaturesLen != 1" );

        DBG_ASSERT( pAttrib->GetStart() <= pAttrib->GetEnd(), "Expand: Attribute distorted!" );
        DBG_ASSERT( ( pAttrib->GetEnd() <= Len() ), "Expand: Attribute larger than paragraph!" );
        if ( pAttrib->IsEmpty() )
        {
            OSL_FAIL( "Empty Attribute after ExpandAttribs?" );
            bResort = TRUE;
            aCharAttribList.GetAttribs().Remove( nAttr );
            rItemPool.Remove( *pAttrib->GetItem() );
            delete pAttrib;
            nAttr--;
        }
        nAttr++;
        pAttrib = GetAttrib( aCharAttribList.GetAttribs(), nAttr );
    }

    if ( bResort )
        aCharAttribList.ResortAttribs();

    if ( pWrongList )
    {
        BOOL bSep = ( GetChar( nIndex ) == ' ' ) || IsFeature( nIndex );
        pWrongList->TextInserted( nIndex, nNew, bSep );
    }

#ifdef EDITDEBUG
    DBG_ASSERT( CheckOrderedList( aCharAttribList.GetAttribs(), TRUE ), "Expand: Start List distorted" );
#endif
}

void ContentNode::CollapsAttribs( USHORT nIndex, USHORT nDeleted, SfxItemPool& rItemPool )
{
    if ( !nDeleted )
        return;

    // Since features are treated differently than normal character attributes,
    // can also the order of the start list be change!
    BOOL bResort = FALSE;
    BOOL bDelAttr = FALSE;
    USHORT nEndChanges = nIndex+nDeleted;

    USHORT nAttr = 0;
    EditCharAttrib* pAttrib = GetAttrib( aCharAttribList.GetAttribs(), nAttr );
    while ( pAttrib )
    {
        bDelAttr = FALSE;
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
                    bDelAttr = TRUE;
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
                    bResort = TRUE;
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
            bResort = TRUE;
            aCharAttribList.GetAttribs().Remove( nAttr );
            rItemPool.Remove( *pAttrib->GetItem() );
            delete pAttrib;
            nAttr--;
        }
        else if ( pAttrib->IsEmpty() )
            aCharAttribList.HasEmptyAttribs() = TRUE;

        nAttr++;
        pAttrib = GetAttrib( aCharAttribList.GetAttribs(), nAttr );
    }

    if ( bResort )
        aCharAttribList.ResortAttribs();

    if ( pWrongList )
        pWrongList->TextDeleted( nIndex, nDeleted );

#ifdef EDITDEBUG
    DBG_ASSERT( CheckOrderedList( aCharAttribList.GetAttribs(), TRUE ), "Collaps: Start list distorted" );
#endif
}

void ContentNode::CopyAndCutAttribs( ContentNode* pPrevNode, SfxItemPool& rPool, BOOL bKeepEndingAttribs )
{
    DBG_ASSERT( pPrevNode, "Copy of attributes to a null pointer?" );

    xub_StrLen nCut = pPrevNode->Len();

    USHORT nAttr = 0;
    EditCharAttrib* pAttrib = GetAttrib( pPrevNode->GetCharAttribs().GetAttribs(), nAttr );
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
//          pPrevNode->GetCharAttribs().RemoveAttrib( pAttrib );
            pPrevNode->GetCharAttribs().GetAttribs().Remove( nAttr );
            aCharAttribList.InsertAttrib( pAttrib );
            DBG_ASSERT( pAttrib->GetStart() >= nCut, "Start < nCut!" );
            DBG_ASSERT( pAttrib->GetEnd() >= nCut, "End < nCut!" );
            pAttrib->GetStart() = pAttrib->GetStart() - nCut;
            pAttrib->GetEnd() = pAttrib->GetEnd() - nCut;
            nAttr--;
        }
        nAttr++;
        pAttrib = GetAttrib( pPrevNode->GetCharAttribs().GetAttribs(), nAttr );
    }
}

void ContentNode::AppendAttribs( ContentNode* pNextNode )
{
    DBG_ASSERT( pNextNode, "Copy of attributes to a null pointer?" );

    USHORT nNewStart = Len();

#ifdef EDITDEBUG
    DBG_ASSERT( aCharAttribList.DbgCheckAttribs(), "Attribute before AppendAttribs broken" );
#endif

    USHORT nAttr = 0;
    EditCharAttrib* pAttrib = GetAttrib( pNextNode->GetCharAttribs().GetAttribs(), nAttr );
    while ( pAttrib )
    {
        // Move all attributes in the current node (this)
        BOOL bMelted = FALSE;
        if ( ( pAttrib->GetStart() == 0 ) && ( !pAttrib->IsFeature() ) )
        {
            // Attributes can possibly be summarized as:
            USHORT nTmpAttr = 0;
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
                        pNextNode->GetCharAttribs().GetAttribs().Remove( nAttr );
                        // Unsubscribe from the pool?!
                        delete pAttrib;
                        bMelted = TRUE;
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
            aCharAttribList.InsertAttrib( pAttrib );
            ++nAttr;
        }
        pAttrib = GetAttrib( pNextNode->GetCharAttribs().GetAttribs(), nAttr );
    }
    // For the Attributes that just moved over:
    pNextNode->GetCharAttribs().Clear();

#ifdef EDITDEBUG
    DBG_ASSERT( aCharAttribList.DbgCheckAttribs(), "Attribute after AppendAttribs broken" );
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

void ContentNode::SetStyleSheet( SfxStyleSheet* pS, BOOL bRecalcFont )
{
    aContentAttribs.SetStyleSheet( pS );
    if ( bRecalcFont )
        CreateDefFont();
}

void ContentNode::DestroyWrongList()
{
    delete pWrongList;
    pWrongList = NULL;
}

void ContentNode::CreateWrongList()
{
    DBG_ASSERT( !pWrongList, "WrongList already exist!" );
    pWrongList = new WrongList;
}

void ContentNode::SetWrongList( WrongList* p )
{
    DBG_ASSERT( !pWrongList, "WrongList already exist!" );
    pWrongList = p;
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

SvxTabStop ContentAttribs::FindTabStop( long nCurPos, USHORT nDefTab )
{
    const SvxTabStopItem& rTabs = (const SvxTabStopItem&) GetItem( EE_PARA_TABS );
    for ( USHORT i = 0; i < rTabs.Count(); i++ )
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
    BOOL bStyleChanged = ( pStyle != pS );
    pStyle = pS;
    // Only when other style sheet, not when current style sheet modified
    if ( pStyle && bStyleChanged )
    {
        // Selectively remove the attributes from the paragraph formatting
        // which are specified in the style, so that the attributes of the
        // style can have an affect.
        const SfxItemSet& rStyleAttribs = pStyle->GetItemSet();
        for ( USHORT nWhich = EE_PARA_START; nWhich <= EE_CHAR_END; nWhich++ )
        {
            // Don't change bullet on/off
            if ( ( nWhich != EE_PARA_BULLETSTATE ) && ( rStyleAttribs.GetItemState( nWhich ) == SFX_ITEM_ON ) )
                aAttribSet.ClearItem( nWhich );
        }
    }
}

const SfxPoolItem& ContentAttribs::GetItem( USHORT nWhich )
{
    // Hard paragraph attributes take precedence!
    SfxItemSet* pTakeFrom = &aAttribSet;
    if ( pStyle && ( aAttribSet.GetItemState( nWhich, FALSE ) != SFX_ITEM_ON  ) )
        pTakeFrom = &pStyle->GetItemSet();

    return pTakeFrom->Get( nWhich );
}

BOOL ContentAttribs::HasItem( USHORT nWhich )
{
    BOOL bHasItem = FALSE;
    if ( aAttribSet.GetItemState( nWhich, FALSE ) == SFX_ITEM_ON  )
        bHasItem = TRUE;
    else if ( pStyle && pStyle->GetItemSet().GetItemState( nWhich ) == SFX_ITEM_ON )
        bHasItem = TRUE;

    return bHasItem;
}


ItemList::ItemList() : CurrentItem( 0 )
{
}

const SfxPoolItem* ItemList::FindAttrib( USHORT nWhich )
{
    for ( size_t i = 0, n = aItemPool.size(); i < n; ++i )
        if ( aItemPool[ i ]->Which() == nWhich )
            return aItemPool[ i ];
    return NULL;
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


EditDoc::EditDoc( SfxItemPool* pPool )
{
    if ( pPool )
    {
        pItemPool = pPool;
        bOwnerOfPool = FALSE;
    }
    else
    {
        pItemPool = new EditEngineItemPool( FALSE );
        bOwnerOfPool = TRUE;
    }

    nDefTab = DEFTAB;
    bIsVertical = FALSE;
    bIsFixedCellHeight = FALSE;

    // Don't create a empty node, Clear() will be called in EditEngine-CTOR

    SetModified( FALSE );
};

EditDoc::~EditDoc()
{
    ImplDestroyContents();
    if ( bOwnerOfPool )
        SfxItemPool::Free(pItemPool);
}

void EditDoc::ImplDestroyContents()
{
    for ( USHORT nNode = Count(); nNode; )
        RemoveItemsFromPool( GetObject( --nNode ) );
    DeleteAndDestroy( 0, Count() );
}

void EditDoc::RemoveItemsFromPool( ContentNode* pNode )
{
    for ( USHORT nAttr = 0; nAttr < pNode->GetCharAttribs().Count(); nAttr++ )
    {
        EditCharAttrib* pAttr = pNode->GetCharAttribs().GetAttribs()[nAttr];
        GetItemPool().Remove( *pAttr->GetItem() );
    }
}

void CreateFont( SvxFont& rFont, const SfxItemSet& rSet, bool bSearchInParent, short nScriptType )
{
    Font aPrevFont( rFont );
    rFont.SetAlign( ALIGN_BASELINE );
    rFont.SetTransparent( TRUE );

    USHORT nWhich_FontInfo = GetScriptItemId( EE_CHAR_FONTINFO, nScriptType );
    USHORT nWhich_Language = GetScriptItemId( EE_CHAR_LANGUAGE, nScriptType );
    USHORT nWhich_FontHeight = GetScriptItemId( EE_CHAR_FONTHEIGHT, nScriptType );
    USHORT nWhich_Weight = GetScriptItemId( EE_CHAR_WEIGHT, nScriptType );
    USHORT nWhich_Italic = GetScriptItemId( EE_CHAR_ITALIC, nScriptType );

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

        USHORT nProp = rEsc.GetProp();
        rFont.SetPropr( (BYTE)nProp );

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

void EditDoc::CreateDefFont( BOOL bUseStyles )
{
    SfxItemSet aTmpSet( GetItemPool(), EE_PARA_START, EE_CHAR_END );
    CreateFont( aDefFont, aTmpSet );
    aDefFont.SetVertical( IsVertical() );
    aDefFont.SetOrientation( IsVertical() ? 2700 : 0 );

    for ( USHORT nNode = 0; nNode < Count(); nNode++ )
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

XubString EditDoc::GetSepStr( LineEnd eEnd )
{
    XubString aSep;
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
    ULONG nLen = GetTextLen();
    USHORT nNodes = Count();

    String aSep = EditDoc::GetSepStr( eEnd );
    USHORT nSepSize = aSep.Len();

    if ( nSepSize )
        nLen += nNodes * nSepSize;
    if ( nLen > 0xFFFb / sizeof(xub_Unicode) )
    {
        OSL_FAIL( "Text to large for String" );
        return XubString();
    }
    xub_Unicode* pStr = new xub_Unicode[nLen+1];
    xub_Unicode* pCur = pStr;
    USHORT nLastNode = nNodes-1;
    for ( USHORT nNode = 0; nNode < nNodes; nNode++ )
    {
        XubString aTmp( GetParaAsString( GetObject(nNode) ) );
        memcpy( pCur, aTmp.GetBuffer(), aTmp.Len()*sizeof(sal_Unicode) );
        pCur += aTmp.Len();
        if ( nSepSize && ( nNode != nLastNode ) )
        {
            memcpy( pCur, aSep.GetBuffer(), nSepSize*sizeof(sal_Unicode ) );
            pCur += nSepSize;
        }
    }
    *pCur = '\0';
    XubString aASCIIText( pStr );
    delete[] pStr;
    return aASCIIText;
}

XubString EditDoc::GetParaAsString( USHORT nNode ) const
{
    return GetParaAsString( SaveGetObject( nNode ) );
}

XubString EditDoc::GetParaAsString( ContentNode* pNode, USHORT nStartPos, USHORT nEndPos, BOOL bResolveFields ) const
{
    if ( nEndPos > pNode->Len() )
        nEndPos = pNode->Len();

    DBG_ASSERT( nStartPos <= nEndPos, "Start and End reversed?" );

    USHORT nIndex = nStartPos;
    XubString aStr;
    EditCharAttrib* pNextFeature = pNode->GetCharAttribs().FindFeature( nIndex );
    while ( nIndex < nEndPos )
    {
        USHORT nEnd = nEndPos;
        if ( pNextFeature && ( pNextFeature->GetStart() < nEnd ) )
            nEnd = pNextFeature->GetStart();
        else
            pNextFeature = 0;   // Feature does not interest the below

        DBG_ASSERT( nEnd >= nIndex, "End in front of the index?" );
        //!! beware of sub string length  of -1 which is also defined as STRING_LEN and
        //!! thus would result in adding the whole sub string up to the end of the node !!
        if (nEnd > nIndex)
            aStr += XubString( *pNode, nIndex, nEnd - nIndex );

        if ( pNextFeature )
        {
            switch ( pNextFeature->GetItem()->Which() )
            {
                case EE_FEATURE_TAB:    aStr += '\t';
                break;
                case EE_FEATURE_LINEBR: aStr += '\x0A';
                break;
                case EE_FEATURE_FIELD:  if ( bResolveFields )
                                            aStr += ((EditCharAttribField*)pNextFeature)->GetFieldValue();
                break;
                default:    OSL_FAIL( "What feature?" );
            }
            pNextFeature = pNode->GetCharAttribs().FindFeature( ++nEnd );
        }
        nIndex = nEnd;
    }
    return aStr;
}

ULONG EditDoc::GetTextLen() const
{
    ULONG nLen = 0;
    for ( USHORT nNode = 0; nNode < Count(); nNode++ )
    {
        ContentNode* pNode = GetObject( nNode );
        nLen += pNode->Len();
        // Fields can be longer than the placeholder in the Node
        const CharAttribArray& rAttrs = pNode->GetCharAttribs().GetAttribs();
        for ( USHORT nAttr = rAttrs.Count(); nAttr; )
        {
            EditCharAttrib* pAttr = rAttrs[--nAttr];
            if ( pAttr->Which() == EE_FEATURE_FIELD )
            {
                USHORT nFieldLen = ((EditCharAttribField*)pAttr)->GetFieldValue().Len();
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
    Insert( pNode, 0 );

    CreateDefFont( FALSE );

    SetModified( FALSE );

    EditPaM aPaM( pNode, 0 );
    return aPaM;
}

void EditDoc::SetModified( BOOL b )
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
    Insert( pNode, 0 );

    pNode->SetStyleSheet( pPrevStyle, FALSE );
    pNode->GetContentAttribs().GetItems().Set( aPrevSet );
    pNode->GetCharAttribs().GetDefFont() = aPrevFont;

    SetModified( TRUE );

    EditPaM aPaM( pNode, 0 );
    return aPaM;
}

void EditDoc::InsertText( const EditPaM& rPaM, xub_Unicode c )
{
    DBG_ASSERT( c != 0x0A, "EditDoc::InsertText: Newlines prohibited in paragraph!" );
    DBG_ASSERT( c != 0x0D, "EditDoc::InsertText: Newlines prohibited in paragraph!" );
    DBG_ASSERT( c != '\t', "EditDoc::InsertText: Newlines prohibited in paragraph!" );

    rPaM.GetNode()->Insert( c, rPaM.GetIndex() );
    rPaM.GetNode()->ExpandAttribs( rPaM.GetIndex(), 1, GetItemPool() );

    SetModified( TRUE );
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

    SetModified( TRUE );

    return aPaM;
}

EditPaM EditDoc::InsertParaBreak( EditPaM aPaM, BOOL bKeepEndingAttribs )
{
    DBG_ASSERT( aPaM.GetNode(), "Blinder PaM in EditDoc::InsertParaBreak" );
    ContentNode* pCurNode = aPaM.GetNode();
    USHORT nPos = GetPos( pCurNode );
    XubString aStr = aPaM.GetNode()->Copy( aPaM.GetIndex() );
    aPaM.GetNode()->Erase( aPaM.GetIndex() );

    // the paragraph attributes...
    ContentAttribs aContentAttribs( aPaM.GetNode()->GetContentAttribs() );

    // for a new paragraph we like to have the bullet/numbering visible by default
    aContentAttribs.GetItems().Put( SfxBoolItem( EE_PARA_BULLETSTATE, TRUE), EE_PARA_BULLETSTATE );

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

    Insert( pNode, nPos+1 );

    SetModified( TRUE );

    aPaM.SetNode( pNode );
    aPaM.SetIndex( 0 );
    return aPaM;
}

EditPaM EditDoc::InsertFeature( EditPaM aPaM, const SfxPoolItem& rItem  )
{
    DBG_ASSERT( aPaM.GetNode(), "Blinder PaM in EditDoc::InsertFeature" );

    aPaM.GetNode()->Insert( CH_FEATURE, aPaM.GetIndex() );
    aPaM.GetNode()->ExpandAttribs( aPaM.GetIndex(), 1, GetItemPool() );

    // Create a feature-attribute for the feature...
    EditCharAttrib* pAttrib = MakeCharAttrib( GetItemPool(), rItem, aPaM.GetIndex(), aPaM.GetIndex()+1 );
    DBG_ASSERT( pAttrib, "Why can not the feature be created?" );
    aPaM.GetNode()->GetCharAttribs().InsertAttrib( pAttrib );

    SetModified( TRUE );

    aPaM.GetIndex()++;
    return aPaM;
}

EditPaM EditDoc::ConnectParagraphs( ContentNode* pLeft, ContentNode* pRight )
{
    const EditPaM aPaM( pLeft, pLeft->Len() );

    // First the attributes, otherwise nLen will not be correct!
    pLeft->AppendAttribs( pRight );
    // then the Text...
    *pLeft += *pRight;

    // the one to the right disappears.
    RemoveItemsFromPool( pRight );
    USHORT nRight = GetPos( pRight );
    Remove( nRight );
    delete pRight;

    SetModified( TRUE );

    return aPaM;
}

EditPaM EditDoc::RemoveChars( EditPaM aPaM, USHORT nChars )
{
    // Maybe remove Features!
    aPaM.GetNode()->Erase( aPaM.GetIndex(), nChars );
    aPaM.GetNode()->CollapsAttribs( aPaM.GetIndex(), nChars, GetItemPool() );

    SetModified( TRUE );

    return aPaM;
}

void EditDoc::InsertAttribInSelection( ContentNode* pNode, USHORT nStart, USHORT nEnd, const SfxPoolItem& rPoolItem )
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
        pNode->GetCharAttribs().GetAttribs().Remove( pNode->GetCharAttribs().GetAttribs().GetPos( pStartingAttrib ) );
        delete pStartingAttrib;
    }
    else if ( pStartingAttrib && ( *(pStartingAttrib->GetItem()) == rPoolItem ) )
        pStartingAttrib->GetStart() = nStart;
    else if ( pEndingAttrib && ( *(pEndingAttrib->GetItem()) == rPoolItem ) )
        pEndingAttrib->GetEnd() = nEnd;
    else
        InsertAttrib( rPoolItem, pNode, nStart, nEnd );

    if ( pStartingAttrib )
        pNode->GetCharAttribs().ResortAttribs();

    SetModified( TRUE );
}

BOOL EditDoc::RemoveAttribs( ContentNode* pNode, USHORT nStart, USHORT nEnd, USHORT nWhich )
{
    EditCharAttrib* pStarting;
    EditCharAttrib* pEnding;
    return RemoveAttribs( pNode, nStart, nEnd, pStarting, pEnding, nWhich );
}

BOOL EditDoc::RemoveAttribs( ContentNode* pNode, USHORT nStart, USHORT nEnd, EditCharAttrib*& rpStarting, EditCharAttrib*& rpEnding, USHORT nWhich )
{

    DBG_ASSERT( pNode, "What to do with the attribute?" );
    DBG_ASSERT( nEnd <= pNode->Len(), "InsertAttrib: Attribute to large!" );

    // This ends at the beginning of the selection => can be expanded
    rpEnding = 0;
    // This starts at the end of the selection => can be expanded
    rpStarting = 0;

    BOOL bChanged = FALSE;

    DBG_ASSERT( nStart <= nEnd, "Small miscalculations in InsertAttribInSelection" );

    // iterate over the attributes ...
    USHORT nAttr = 0;
    EditCharAttrib* pAttr = GetAttrib( pNode->GetCharAttribs().GetAttribs(), nAttr );
    while ( pAttr )
    {
        BOOL bRemoveAttrib = FALSE;
        USHORT nAttrWhich = pAttr->Which();
        if ( ( nAttrWhich < EE_FEATURE_START ) && ( !nWhich || ( nAttrWhich == nWhich ) ) )
        {
            // Attribute starts in Selection
            if ( ( pAttr->GetStart() >= nStart ) && ( pAttr->GetStart() <= nEnd ) )
            {
                bChanged = TRUE;
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
                    bRemoveAttrib = TRUE;
                }
            }

            // Attribute ends in Selection
            else if ( ( pAttr->GetEnd() >= nStart ) && ( pAttr->GetEnd() <= nEnd ) )
            {
                bChanged = TRUE;
                if ( ( pAttr->GetStart() < nStart ) && !pAttr->IsFeature() )
                {
                    pAttr->GetEnd() = nStart;   // then it ends here
                    rpEnding = pAttr;
                }
                else if ( !pAttr->IsFeature() || ( pAttr->GetStart() == nStart ) )
                {
                    // Delete feature only if on the exact spot
                    bRemoveAttrib = TRUE;
                }
            }
            // Attribute overlaps the selection
            else if ( ( pAttr->GetStart() <= nStart ) && ( pAttr->GetEnd() >= nEnd ) )
            {
                bChanged = TRUE;
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
                    USHORT nOldEnd = pAttr->GetEnd();
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
            pNode->GetCharAttribs().GetAttribs().Remove(nAttr);
            GetItemPool().Remove( *pAttr->GetItem() );
            delete pAttr;
            nAttr--;
        }
        nAttr++;
        pAttr = GetAttrib( pNode->GetCharAttribs().GetAttribs(), nAttr );
    }

    if ( bChanged )
    {
        // char attributes need to be sorted by start again
        pNode->GetCharAttribs().ResortAttribs();

        SetModified( TRUE );
    }

    return bChanged;
}

void EditDoc::InsertAttrib( const SfxPoolItem& rPoolItem, ContentNode* pNode, USHORT nStart, USHORT nEnd )
{
    // This method no longer checks whether a corresponding attribute already
    // exists at this place!
    EditCharAttrib* pAttrib = MakeCharAttrib( GetItemPool(), rPoolItem, nStart, nEnd );
    DBG_ASSERT( pAttrib, "MakeCharAttrib failed!" );
    pNode->GetCharAttribs().InsertAttrib( pAttrib );

    SetModified( TRUE );
}

void EditDoc::InsertAttrib( ContentNode* pNode, USHORT nStart, USHORT nEnd, const SfxPoolItem& rPoolItem )
{
    if ( nStart != nEnd )
    {
        InsertAttribInSelection( pNode, nStart, nEnd, rPoolItem );
    }
    else
    {
        // Check whether already a new attribute with WhichId exists at this place:
        EditCharAttrib* pAttr = pNode->GetCharAttribs().FindEmptyAttrib( rPoolItem.Which(), nStart );
        if ( pAttr )
        {
            // Remove attribute....
            pNode->GetCharAttribs().GetAttribs().Remove(
                pNode->GetCharAttribs().GetAttribs().GetPos( pAttr ) );
        }

        // check whether 'the same' attribute exist at this place.
        pAttr = pNode->GetCharAttribs().FindAttrib( rPoolItem.Which(), nStart );
        if ( pAttr )
        {
            if ( pAttr->IsInside( nStart ) )    // split
            {
                // check again if really splitting, or return !
                USHORT nOldEnd = pAttr->GetEnd();
                pAttr->GetEnd() = nStart;
                pAttr = MakeCharAttrib( GetItemPool(), *(pAttr->GetItem()), nStart, nOldEnd );
                pNode->GetCharAttribs().InsertAttrib( pAttr );
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

    SetModified( TRUE );
}

void EditDoc::FindAttribs( ContentNode* pNode, USHORT nStartPos, USHORT nEndPos, SfxItemSet& rCurSet )
{
    DBG_ASSERT( pNode, "Where to search?" );
    DBG_ASSERT( nStartPos <= nEndPos, "Invalid region!" );

    USHORT nAttr = 0;
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
                USHORT nWhich = pItem->Which();
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
                USHORT nWhich = pItem->Which();
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


CharAttribList::CharAttribList()
{
    DBG_CTOR( EE_CharAttribList, 0 );
    bHasEmptyAttribs = FALSE;
}

CharAttribList::~CharAttribList()
{
    DBG_DTOR( EE_CharAttribList, 0 );

    USHORT nAttr = 0;
    EditCharAttrib* pAttr = GetAttrib( aAttribs, nAttr );
    while ( pAttr )
    {
        delete pAttr;
        ++nAttr;
        pAttr = GetAttrib( aAttribs, nAttr );
    }
    Clear();
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

    const USHORT nCount = Count();
    const USHORT nStart = pAttrib->GetStart(); // may be better for Comp.Opt.

    if ( pAttrib->IsEmpty() )
        bHasEmptyAttribs = TRUE;

    BOOL bInserted = FALSE;
    for ( USHORT x = 0; x < nCount; x++ )
    {
        EditCharAttribPtr pCurAttrib = aAttribs[x];
        if ( pCurAttrib->GetStart() > nStart )
        {
            aAttribs.Insert( pAttrib, x );
            bInserted = TRUE;
            break;
        }
    }
    if ( !bInserted )
        aAttribs.Insert( pAttrib, nCount );
}

void CharAttribList::ResortAttribs()
{
    if ( Count() )
    {
#if defined __SUNPRO_CC
#pragma disable_warn
#endif
        qsort( (void*)aAttribs.GetData(), aAttribs.Count(), sizeof( EditCharAttrib* ), CompareStart );
#if defined __SUNPRO_CC
#pragma enable_warn
#endif
    }
}

void CharAttribList::OptimizeRanges( SfxItemPool& rItemPool )
{
    for ( USHORT n = 0; n < aAttribs.Count(); n++ )
    {
        EditCharAttrib* pAttr = aAttribs.GetObject( n );
        for ( USHORT nNext = n+1; nNext < aAttribs.Count(); nNext++ )
        {
            EditCharAttrib* p = aAttribs.GetObject( nNext );
            if ( !pAttr->IsFeature() && ( p->GetStart() == pAttr->GetEnd() ) && ( p->Which() == pAttr->Which() ) )
            {
                if ( *p->GetItem() == *pAttr->GetItem() )
                {
                    pAttr->GetEnd() = p->GetEnd();
                    aAttribs.Remove( nNext );
                    rItemPool.Remove( *p->GetItem() );
                    delete p;
                }
                break;  // only 1 attr with same which can start here.
            }
            else if ( p->GetStart() > pAttr->GetEnd() )
            {
                break;
            }
        }
    }
}

EditCharAttrib* CharAttribList::FindAttrib( USHORT nWhich, USHORT nPos )
{
    // Backwards, if one ends where the next starts.
    // => The starting one is the valid one ...
    USHORT nAttr = aAttribs.Count()-1;
    EditCharAttrib* pAttr = GetAttrib( aAttribs, nAttr );
    while ( pAttr )
    {
        if ( ( pAttr->Which() == nWhich ) && pAttr->IsIn(nPos) )
            return pAttr;
        pAttr = GetAttrib( aAttribs, --nAttr );
    }
    return 0;
}

EditCharAttrib* CharAttribList::FindNextAttrib( USHORT nWhich, USHORT nFromPos ) const
{
    DBG_ASSERT( nWhich, "FindNextAttrib: Which?" );
    const USHORT nAttribs = aAttribs.Count();
    for ( USHORT nAttr = 0; nAttr < nAttribs; nAttr++ )
    {
        EditCharAttrib* pAttr = aAttribs[ nAttr ];
        if ( ( pAttr->GetStart() >= nFromPos ) && ( pAttr->Which() == nWhich ) )
            return pAttr;
    }
    return 0;
}

BOOL CharAttribList::HasAttrib( USHORT nWhich ) const
{
    for ( USHORT nAttr = aAttribs.Count(); nAttr; )
    {
        const EditCharAttrib* pAttr = aAttribs[--nAttr];
        if ( pAttr->Which() == nWhich )
            return TRUE;
    }
    return FALSE;
}

BOOL CharAttribList::HasAttrib( USHORT nStartPos, USHORT nEndPos ) const
{
    BOOL bAttr = FALSE;
    for ( USHORT nAttr = aAttribs.Count(); nAttr && !bAttr; )
    {
        const EditCharAttrib* pAttr = aAttribs[--nAttr];
        if ( ( pAttr->GetStart() < nEndPos ) && ( pAttr->GetEnd() > nStartPos ) )
            return bAttr = TRUE;
    }
    return bAttr;
}


BOOL CharAttribList::HasBoundingAttrib( USHORT nBound )
{
    // Backwards, if one ends where the next starts.
    // => The starting one is the valid one ...
    USHORT nAttr = aAttribs.Count()-1;
    EditCharAttrib* pAttr = GetAttrib( aAttribs, nAttr );
    while ( pAttr && ( pAttr->GetEnd() >= nBound ) )
    {
        if ( ( pAttr->GetStart() == nBound ) || ( pAttr->GetEnd() == nBound ) )
            return TRUE;
        pAttr = GetAttrib( aAttribs, --nAttr );
    }
    return FALSE;
}

EditCharAttrib* CharAttribList::FindEmptyAttrib( USHORT nWhich, USHORT nPos )
{
    if ( !bHasEmptyAttribs )
        return 0;
    USHORT nAttr = 0;
    EditCharAttrib* pAttr = GetAttrib( aAttribs, nAttr );
    while ( pAttr && ( pAttr->GetStart() <= nPos ) )
    {
        if ( ( pAttr->GetStart() == nPos ) && ( pAttr->GetEnd() == nPos ) && ( pAttr->Which() == nWhich ) )
            return pAttr;
        nAttr++;
        pAttr = GetAttrib( aAttribs, nAttr );
    }
    return 0;
}

EditCharAttrib* CharAttribList::FindFeature( USHORT nPos ) const
{

    USHORT nAttr = 0;
    EditCharAttrib* pNextAttrib = GetAttrib( aAttribs, nAttr );

    // first to the desired position ...
    while ( pNextAttrib && ( pNextAttrib->GetStart() < nPos ) )
    {
        nAttr++;
        pNextAttrib = GetAttrib( aAttribs, nAttr );
    }

    // Now search for the Feature...
    while ( pNextAttrib && !pNextAttrib->IsFeature() )
    {
        nAttr++;
        pNextAttrib = GetAttrib( aAttribs, nAttr );
    }

    return pNextAttrib;
}


void CharAttribList::DeleteEmptyAttribs( SfxItemPool& rItemPool )
{
    for ( USHORT nAttr = 0; nAttr < aAttribs.Count(); nAttr++ )
    {
        EditCharAttrib* pAttr = aAttribs[nAttr];
        if ( pAttr->IsEmpty() )
        {
            rItemPool.Remove( *pAttr->GetItem() );
            aAttribs.Remove( nAttr );
            delete pAttr;
            nAttr--;
        }
    }
    bHasEmptyAttribs = FALSE;
}

BOOL CharAttribList::DbgCheckAttribs()
{
#ifdef  DBG_UTIL
    BOOL bOK = TRUE;
    for ( USHORT nAttr = 0; nAttr < aAttribs.Count(); nAttr++ )
    {
        EditCharAttrib* pAttr = aAttribs[nAttr];
        if ( pAttr->GetStart() > pAttr->GetEnd() )
        {
            bOK = FALSE;
            OSL_FAIL( "Attribute is distorted" );
        }
        else if ( pAttr->IsFeature() && ( pAttr->GetLen() != 1 ) )
        {
            bOK = FALSE;
            OSL_FAIL( "Feature, Len != 1" );
        }
    }
    return bOK;
#else
    return TRUE;
#endif
}



SvxFontTable::SvxFontTable()
{
}

SvxFontTable::~SvxFontTable()
{
    SvxFontItem* pItem = First();
    while( pItem )
    {
        delete pItem;
        pItem = Next();
    }
}

ULONG SvxFontTable::GetId( const SvxFontItem& rFontItem )
{
    SvxFontItem* pItem = First();
    while ( pItem )
    {
        if ( *pItem == rFontItem )
            return GetCurKey();
        pItem = Next();
    }
    DBG_WARNING( "Font not found: GetId()" );
    return 0;
}

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

EditEngineItemPool::EditEngineItemPool( BOOL bPersistenRefCounts )
    : SfxItemPool( String( "EditEngineItemPool", RTL_TEXTENCODING_ASCII_US ), EE_ITEMS_START, EE_ITEMS_END,
                    aItemInfos, 0, bPersistenRefCounts )
{
    SetVersionMap( 1, 3999, 4015, aV1Map );
    SetVersionMap( 2, 3999, 4019, aV2Map );
    SetVersionMap( 3, 3997, 4020, aV3Map );
    SetVersionMap( 4, 3994, 4022, aV4Map );
    SetVersionMap( 5, 3994, 4037, aV5Map );

    DBG_ASSERT( EE_DLL(), "EditDLL?!" );
    SfxPoolItem** ppDefItems = EE_DLL()->GetGlobalData()->GetDefItems();
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
    BOOL b31Format = ( nVersion && ( nVersion <= SOFFICE_FILEFORMAT_31 ) )
                        ? TRUE : FALSE;

    EditEngineItemPool* pThis = (EditEngineItemPool*)this;
    if ( b31Format )
        pThis->SetStoringRange( 3997, 4022 );
    else
        pThis->SetStoringRange( EE_ITEMS_START, EE_ITEMS_END );

    return SfxItemPool::Store( rStream );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
