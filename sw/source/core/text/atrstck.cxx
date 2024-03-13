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

#include "atrhndl.hxx"
#include <svl/itemiter.hxx>
#include <vcl/outdev.hxx>
#include <editeng/cmapitem.hxx>
#include <editeng/colritem.hxx>
#include <editeng/contouritem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/charreliefitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/wrlmitem.hxx>
#include <editeng/autokernitem.hxx>
#include <editeng/charrotateitem.hxx>
#include <editeng/emphasismarkitem.hxx>
#include <editeng/charscaleitem.hxx>
#include <editeng/twolinesitem.hxx>
#include <editeng/charhiddenitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/shaditem.hxx>
#include <viewopt.hxx>
#include <charfmt.hxx>
#include <fchrfmt.hxx>
#include <fmtautofmt.hxx>
#include <editeng/brushitem.hxx>
#include <fmtinfmt.hxx>
#include <txtinet.hxx>
#include <IDocumentSettingAccess.hxx>
#include <viewsh.hxx>

/**
 * Attribute to Stack Mapping
 *
 * Attributes applied to a text are pushed on different stacks. For each
 * stack, the top most attribute on the stack is valid. Because some
 * kinds of attributes have to be pushed to the same stacks we map their
 * ids to stack ids
 * Attention: The first NUM_DEFAULT_VALUES ( defined in swfntcch.hxx )
 * are stored in the defaultitem-cache, if you add one, you have to increase
 * NUM_DEFAULT_VALUES.
 * Also adjust NUM_ATTRIBUTE_STACKS in atrhndl.hxx.
 */
const sal_uInt8 StackPos[ RES_TXTATR_WITHEND_END - RES_CHRATR_BEGIN + 1 ] =
{
     0, //                                       //  0
     1, // RES_CHRATR_CASEMAP = RES_CHRATR_BEGIN //  1
     0, // RES_CHRATR_CHARSETCOLOR,              //  2
     2, // RES_CHRATR_COLOR,                     //  3
     3, // RES_CHRATR_CONTOUR,                   //  4
     4, // RES_CHRATR_CROSSEDOUT,                //  5
     5, // RES_CHRATR_ESCAPEMENT,                //  6
     6, // RES_CHRATR_FONT,                      //  7
     7, // RES_CHRATR_FONTSIZE,                  //  8
     8, // RES_CHRATR_KERNING,                   //  9
     9, // RES_CHRATR_LANGUAGE,                  // 10
    10, // RES_CHRATR_POSTURE,                   // 11
     0, // RES_CHRATR_UNUSED1,                   // 12
    11, // RES_CHRATR_SHADOWED,                  // 13
    12, // RES_CHRATR_UNDERLINE,                 // 14
    13, // RES_CHRATR_WEIGHT,                    // 15
    14, // RES_CHRATR_WORDLINEMODE,              // 16
    15, // RES_CHRATR_AUTOKERN,                  // 17
    16, // RES_CHRATR_BLINK,                     // 18
    17, // RES_CHRATR_NOHYPHEN,                  // 19
     0, // RES_CHRATR_UNUSED2,                   // 20
    18, // RES_CHRATR_BACKGROUND,                // 21
    19, // RES_CHRATR_CJK_FONT,                  // 22
    20, // RES_CHRATR_CJK_FONTSIZE,              // 23
    21, // RES_CHRATR_CJK_LANGUAGE,              // 24
    22, // RES_CHRATR_CJK_POSTURE,               // 25
    23, // RES_CHRATR_CJK_WEIGHT,                // 26
    24, // RES_CHRATR_CTL_FONT,                  // 27
    25, // RES_CHRATR_CTL_FONTSIZE,              // 28
    26, // RES_CHRATR_CTL_LANGUAGE,              // 29
    27, // RES_CHRATR_CTL_POSTURE,               // 30
    28, // RES_CHRATR_CTL_WEIGHT,                // 31
    29, // RES_CHRATR_ROTATE,                    // 32
    30, // RES_CHRATR_EMPHASIS_MARK,             // 33
    31, // RES_CHRATR_TWO_LINES,                 // 34
    32, // RES_CHRATR_SCALEW,                    // 35
    33, // RES_CHRATR_RELIEF,                    // 36
    34, // RES_CHRATR_HIDDEN,                    // 37
    35, // RES_CHRATR_OVERLINE,                  // 38
     0, // RES_CHRATR_RSID,                      // 39
    36, // RES_CHRATR_BOX,                       // 40
    37, // RES_CHRATR_SHADOW,                    // 41
    38, // RES_CHRATR_HIGHLIGHT,                 // 42
     0, // RES_CHRATR_GRABBAG,                   // 43
     0, // RES_CHRATR_BIDIRTL,                   // 44
     0, // RES_CHRATR_IDCTHINT,                  // 45
    39, // RES_TXTATR_REFMARK,                   // 46
    40, // RES_TXTATR_TOXMARK,                   // 47
    41, // RES_TXTATR_META,                      // 48
    41, // RES_TXTATR_METAFIELD,                 // 49
     0, // RES_TXTATR_AUTOFMT,                   // 50
     0, // RES_TXTATR_INETFMT                    // 51
     0, // RES_TXTATR_CHARFMT,                   // 52
    42, // RES_TXTATR_CJK_RUBY,                  // 53
     0, // RES_TXTATR_UNKNOWN_CONTAINER,         // 54
    43, // RES_TXTATR_INPUTFIELD                 // 55
    44, // RES_TXTATR_CONTENTCONTROL             // 56
};

namespace CharFormat
{

/// Returns the item set associated with a character/inet/auto style
const SfxItemSet* GetItemSet( const SfxPoolItem& rAttr )
{
    const SfxItemSet* pSet = nullptr;

    if ( RES_TXTATR_AUTOFMT == rAttr.Which() )
    {
        pSet = rAttr.StaticWhichCast(RES_TXTATR_AUTOFMT).GetStyleHandle().get();
    }
    else
    {
        // Get the attributes from the template
        const SwCharFormat* pFormat = RES_TXTATR_INETFMT == rAttr.Which() ?
                        rAttr.StaticWhichCast(RES_TXTATR_INETFMT).GetTextINetFormat()->GetCharFormat() :
                        static_cast<const SwFormatCharFormat&>(rAttr).GetCharFormat();
        if( pFormat )
        {
            pSet = &pFormat->GetAttrSet();
        }
    }

    return pSet;
}

/// Extracts pool item of type nWhich from rAttr
const SfxPoolItem* GetItem( const SwTextAttr& rAttr, sal_uInt16 nWhich )
{
    if ( RES_TXTATR_INETFMT == rAttr.Which() ||
         RES_TXTATR_CHARFMT == rAttr.Which() ||
         RES_TXTATR_AUTOFMT == rAttr.Which() )
    {
        const SfxItemSet* pSet = CharFormat::GetItemSet( rAttr.GetAttr() );
        if ( !pSet ) return nullptr;

        bool bInParent = RES_TXTATR_AUTOFMT != rAttr.Which();
        const SfxPoolItem* pItem;
        bool bRet = SfxItemState::SET == pSet->GetItemState( nWhich, bInParent, &pItem );

        return bRet ? pItem : nullptr;
    }

    return ( nWhich == rAttr.Which() ) ? &rAttr.GetAttr() : nullptr;
}

/// Checks if item is included in character/inet/auto style
bool IsItemIncluded( const sal_uInt16 nWhich, const SwTextAttr *pAttr )
{
    bool bRet = false;

    const SfxItemSet* pItemSet = CharFormat::GetItemSet( pAttr->GetAttr() );
    if ( pItemSet )
        bRet = SfxItemState::SET == pItemSet->GetItemState( nWhich );

    return bRet;
}
}

/**
 * The color of hyperlinks is taken from the associated character attribute,
 * depending on its 'visited' state. There are actually two cases, which
 * should override the colors from the character attribute:
 *     1. We never take the 'visited' color during printing/pdf export/preview
 *     2. The user has chosen to override these colors in the view options
 */
static bool lcl_ChgHyperLinkColor( const SwTextAttr& rAttr,
                            const SfxPoolItem& rItem,
                            const SwViewShell* pShell,
                            Color* pColor )
{
    if ( !pShell ||
         RES_TXTATR_INETFMT != rAttr.Which() ||
         RES_CHRATR_COLOR != rItem.Which() )
        return false;

    // #i15455#
    // 1. case:
    // We do not want to show visited links:
    // (printing, pdf export, page preview)

    SwTextINetFormat & rINetAttr(const_cast<SwTextINetFormat&>(
                            static_txtattr_cast<SwTextINetFormat const&>(rAttr)));
    if ( pShell->GetOut()->GetOutDevType() == OUTDEV_PRINTER ||
         pShell->GetViewOptions()->IsPDFExport() ||
         pShell->GetViewOptions()->IsPagePreview() )
    {
        if (rINetAttr.IsVisited())
        {
            if ( pColor )
            {
                // take color from character format 'unvisited link'
                rINetAttr.SetVisited(false);
                const SwCharFormat* pTmpFormat = rINetAttr.GetCharFormat();
                if (const SvxColorItem* pItem = pTmpFormat->GetItemIfSet(RES_CHRATR_COLOR))
                    *pColor = pItem->GetValue();
                rINetAttr.SetVisited(true);
            }
            return true;
        }

        return false;
    }

    // 2. case:
    // We do not want to apply the color set in the hyperlink
    // attribute, instead we take the colors from the view options:

    if ( pShell->GetWin() &&
        (
          (rINetAttr.IsVisited() && pShell->GetViewOptions()->IsVisitedLinks()) ||
          (!rINetAttr.IsVisited() && pShell->GetViewOptions()->IsLinks())
        )
       )
    {
        if ( pColor )
        {
            if (rINetAttr.IsVisited())
            {
                // take color from view option 'visited link color'
                *pColor = pShell->GetViewOptions()->GetVisitedLinksColor();
            }
            else
            {
                // take color from view option 'unvisited link color'
                *pColor = pShell->GetViewOptions()->GetLinksColor();
            }
        }
        return true;
    }

    return false;
}

SwAttrHandler::SwAttrHandler()
    : m_pIDocumentSettingAccess(nullptr)
    , m_pShell(nullptr)
    , m_bVertLayout(false)
    , m_bVertLayoutLRBT(false)
{
    memset( m_pDefaultArray, 0, NUM_DEFAULT_VALUES * sizeof(SfxPoolItem*) );
}

SwAttrHandler::~SwAttrHandler()
{
}

void SwAttrHandler::Init( const SwAttrSet& rAttrSet,
                          const IDocumentSettingAccess& rIDocumentSettingAcces )
{
    m_pIDocumentSettingAccess = &rIDocumentSettingAcces;
    m_pShell = nullptr;

    for ( sal_uInt16 i = RES_CHRATR_BEGIN; i < RES_CHRATR_END; i++ )
        m_pDefaultArray[ StackPos[ i ] ] = &rAttrSet.Get( i );
}

void SwAttrHandler::Init( const SfxPoolItem** pPoolItem, const SwAttrSet* pAS,
                          const IDocumentSettingAccess& rIDocumentSettingAcces,
                          const SwViewShell* pSh,
                          SwFont& rFnt, bool bVL, bool bVertLayoutLRBT )
{
    // initialize default array
    memcpy( m_pDefaultArray, pPoolItem,
            NUM_DEFAULT_VALUES * sizeof(SfxPoolItem*) );

    m_pIDocumentSettingAccess = &rIDocumentSettingAcces;
    m_pShell = pSh;

    // do we have to apply additional paragraph attributes?
    m_bVertLayout = bVL;
    m_bVertLayoutLRBT = bVertLayoutLRBT;

    if ( pAS && pAS->Count() )
    {
        SfxItemIter aIter( *pAS );
        sal_uInt16 nWhich;
        const SfxPoolItem* pItem = aIter.GetCurItem();
        do
        {
            nWhich = pItem->Which();
            if (isCHRATR(nWhich))
            {
                m_pDefaultArray[ StackPos[ nWhich ] ] = pItem;
                FontChg( *pItem, rFnt, true );
            }

            pItem = aIter.NextItem();
        } while (pItem);
    }

    // It is possible, that Init is called more than once, e.g., in a
    // SwTextFrame::FormatOnceMore situation or (since sw_redlinehide)
    // from SwAttrIter::Seek(); in the latter case SwTextSizeInfo::m_pFnt
    // is an alias of m_pFnt so it must not be deleted!
    if (m_oFnt)
        *m_oFnt = rFnt;
    else
        m_oFnt.emplace(rFnt);
}

void SwAttrHandler::Reset( )
{
    for (auto& i : m_aAttrStack)
        i.clear();
}

void SwAttrHandler::PushAndChg( const SwTextAttr& rAttr, SwFont& rFnt )
{
    // these special attributes in fact represent a collection of attributes
    // they have to be pushed to each stack they belong to
    if ( RES_TXTATR_INETFMT == rAttr.Which() ||
         RES_TXTATR_CHARFMT == rAttr.Which() ||
         RES_TXTATR_AUTOFMT == rAttr.Which() )
    {
        const SfxItemSet* pSet = CharFormat::GetItemSet( rAttr.GetAttr() );
        if ( !pSet ) return;

        for ( sal_uInt16 i = RES_CHRATR_BEGIN; i < RES_CHRATR_END; i++)
        {
            const SfxPoolItem* pItem;
            bool bRet = SfxItemState::SET == pSet->GetItemState( i, rAttr.Which() != RES_TXTATR_AUTOFMT, &pItem );

            if ( bRet )
            {
                // we push rAttr onto the appropriate stack
                if ( Push( rAttr, *pItem ) )
                {
                    // we let pItem change rFnt
                    Color aColor;
                    if (lcl_ChgHyperLinkColor(rAttr, *pItem, m_pShell, &aColor))
                    {
                        SvxColorItem aItemNext( aColor, RES_CHRATR_COLOR );
                        FontChg( aItemNext, rFnt, true );
                    }
                    else
                        FontChg( *pItem, rFnt, true );
                }
            }
        }
    }
    // this is the usual case, we have a basic attribute, push it onto the
    // stack and change the font
    else
    {
        if ( Push( rAttr, rAttr.GetAttr() ) )
            // we let pItem change rFnt
            FontChg( rAttr.GetAttr(), rFnt, true );
    }
}

const SwTextAttr* SwAttrHandler::GetTop(sal_uInt16 nStack)
{
    return m_aAttrStack[nStack].empty() ? nullptr : m_aAttrStack[nStack].back();
}

bool SwAttrHandler::Push( const SwTextAttr& rAttr, const SfxPoolItem& rItem )
{
    OSL_ENSURE( rItem.Which() < RES_TXTATR_WITHEND_END,
            "I do not want this attribute, nWhich >= RES_TXTATR_WITHEND_END" );

    // robust
    if ( RES_TXTATR_WITHEND_END <= rItem.Which() )
        return false;

    const sal_uInt16 nStack = StackPos[ rItem.Which() ];

    // attributes originating from redlining have highest priority
    // second priority are hyperlink attributes, which have a color replacement
    const SwTextAttr* pTopAttr = GetTop(nStack);
    if ( !pTopAttr
         || rAttr.IsPriorityAttr()
         || ( !pTopAttr->IsPriorityAttr()
              && !lcl_ChgHyperLinkColor(*pTopAttr, rItem, m_pShell, nullptr)))
    {
        m_aAttrStack[nStack].push_back(&rAttr);
        return true;
    }

    const auto it = m_aAttrStack[nStack].end() - 1;
    m_aAttrStack[nStack].insert(it, &rAttr);
    return false;
}

void SwAttrHandler::RemoveFromStack(sal_uInt16 nWhich, const SwTextAttr& rAttr)
{
    auto& rStack = m_aAttrStack[StackPos[nWhich]];
    const auto it = std::find(rStack.begin(), rStack.end(), &rAttr);
    if (it != rStack.end())
        rStack.erase(it);
}

void SwAttrHandler::PopAndChg( const SwTextAttr& rAttr, SwFont& rFnt )
{
    if ( RES_TXTATR_WITHEND_END <= rAttr.Which() )
        return; // robust

    // these special attributes in fact represent a collection of attributes
    // they have to be removed from each stack they belong to
    if ( RES_TXTATR_INETFMT == rAttr.Which() ||
         RES_TXTATR_CHARFMT == rAttr.Which() ||
         RES_TXTATR_AUTOFMT == rAttr.Which() )
    {
        const SfxItemSet* pSet = CharFormat::GetItemSet( rAttr.GetAttr() );
        if ( !pSet ) return;

        for ( sal_uInt16 i = RES_CHRATR_BEGIN; i < RES_CHRATR_END; i++)
        {
            const SfxPoolItem* pItem;
            bool bRet = SfxItemState::SET == pSet->GetItemState( i, RES_TXTATR_AUTOFMT != rAttr.Which(), &pItem );
            if ( bRet )
            {
                // we remove rAttr from the appropriate stack
                RemoveFromStack(i, rAttr);
                // reset font according to attribute on top of stack
                // or default value
                ActivateTop( rFnt, i );
            }
        }
    }
    // this is the usual case, we have a basic attribute, remove it from the
    // stack and reset the font
    else
    {
        RemoveFromStack(rAttr.Which(), rAttr);
        // reset font according to attribute on top of stack
        // or default value
        ActivateTop( rFnt, rAttr.Which() );
    }
}

/// Only used during redlining
void SwAttrHandler::Pop( const SwTextAttr& rAttr )
{
    OSL_ENSURE( rAttr.Which() < RES_TXTATR_WITHEND_END,
            "I do not have this attribute, nWhich >= RES_TXTATR_WITHEND_END" );

    if ( rAttr.Which() < RES_TXTATR_WITHEND_END )
    {
        RemoveFromStack(rAttr.Which(), rAttr);
    }
}

void SwAttrHandler::ActivateTop( SwFont& rFnt, const sal_uInt16 nAttr )
{
    assert(nAttr < RES_TXTATR_WITHEND_END);

    const sal_uInt16 nStackPos = StackPos[ nAttr ];
    const SwTextAttr* pTopAt = GetTop(nStackPos);
    if ( pTopAt )
    {
        const SfxPoolItem* pItemNext(nullptr);

        // check if top attribute is collection of attributes
        if ( RES_TXTATR_INETFMT == pTopAt->Which() ||
             RES_TXTATR_CHARFMT == pTopAt->Which() ||
             RES_TXTATR_AUTOFMT == pTopAt->Which() )
        {
            const SfxItemSet* pSet = CharFormat::GetItemSet( pTopAt->GetAttr() );
            if (pSet)
                pSet->GetItemState( nAttr, RES_TXTATR_AUTOFMT != pTopAt->Which(), &pItemNext );
        }

        if (pItemNext)
        {
            Color aColor;
            if (lcl_ChgHyperLinkColor(*pTopAt, *pItemNext, m_pShell, &aColor))
            {
                SvxColorItem aItemNext( aColor, RES_CHRATR_COLOR );
                FontChg( aItemNext, rFnt, false );
            }
            else
                FontChg( *pItemNext, rFnt, false );
        }
        else
            FontChg( pTopAt->GetAttr(), rFnt, false );
    }

    // default value has to be set, we only have default values for char attribs
    else if ( nStackPos < NUM_DEFAULT_VALUES )
        FontChg( *m_pDefaultArray[ nStackPos ], rFnt, false );
    else if ( RES_TXTATR_REFMARK == nAttr )
        rFnt.GetRef()--;
    else if ( RES_TXTATR_TOXMARK == nAttr )
        rFnt.GetTox()--;
    else if ( (RES_TXTATR_META == nAttr) || (RES_TXTATR_METAFIELD == nAttr) )
    {
        rFnt.GetMeta()--;
    }
    else if (nAttr == RES_TXTATR_CONTENTCONTROL)
    {
        rFnt.GetContentControl()--;
    }
    else if ( RES_TXTATR_CJK_RUBY == nAttr )
    {
        // ruby stack has no more attributes
        // check, if a rotation attribute has to be applied
        const sal_uInt16 nTwoLineStack = StackPos[ RES_CHRATR_TWO_LINES ];
        bool bTwoLineAct = false;
        const SwTextAttr* pTwoLineAttr = GetTop(nTwoLineStack);

        if ( pTwoLineAttr )
        {
             const auto& rTwoLineItem = *CharFormat::GetItem( *pTwoLineAttr, RES_CHRATR_TWO_LINES );
             bTwoLineAct = rTwoLineItem.GetValue();
        }
        else
            bTwoLineAct = m_pDefaultArray[ nTwoLineStack ]->StaticWhichCast(RES_CHRATR_TWO_LINES).GetValue();

        if ( bTwoLineAct )
            return;

        // eventually, a rotate attribute has to be activated
        const sal_uInt16 nRotateStack = StackPos[ RES_CHRATR_ROTATE ];
        const SwTextAttr* pRotateAttr = GetTop(nRotateStack);

        if ( pRotateAttr )
        {
            const auto& rRotateItem = *CharFormat::GetItem( *pRotateAttr, RES_CHRATR_ROTATE );
            rFnt.SetVertical( rRotateItem.GetValue(), m_bVertLayout );
        }
        else
            rFnt.SetVertical( m_pDefaultArray[ nRotateStack ]->StaticWhichCast(RES_CHRATR_ROTATE).GetValue(), m_bVertLayout );
    }
    else if ( RES_TXTATR_INPUTFIELD == nAttr )
        rFnt.GetInputField()--;
}

/**
 * When popping an attribute from the stack, the top more remaining
 * attribute in the stack becomes valid. The following function change
 * a font depending on the stack id.
 */
void SwAttrHandler::FontChg(const SfxPoolItem& rItem, SwFont& rFnt, bool bPush )
{
    switch ( rItem.Which() )
    {
        case RES_CHRATR_CASEMAP :
            rFnt.SetCaseMap( rItem.StaticWhichCast(RES_CHRATR_CASEMAP).GetCaseMap() );
            break;
        case RES_CHRATR_COLOR :
            rFnt.SetColor( rItem.StaticWhichCast(RES_CHRATR_COLOR).GetValue() );
            break;
        case RES_CHRATR_CONTOUR :
            rFnt.SetOutline( rItem.StaticWhichCast(RES_CHRATR_CONTOUR).GetValue() );
            break;
        case RES_CHRATR_CROSSEDOUT :
            rFnt.SetStrikeout( rItem.StaticWhichCast(RES_CHRATR_CROSSEDOUT).GetStrikeout() );
            break;
        case RES_CHRATR_ESCAPEMENT :
            rFnt.SetEscapement( rItem.StaticWhichCast(RES_CHRATR_ESCAPEMENT).GetEsc() );
            rFnt.SetProportion( rItem.StaticWhichCast(RES_CHRATR_ESCAPEMENT).GetProportionalHeight() );
            break;
        case RES_CHRATR_FONT :
        {
            auto& rFontItem = rItem.StaticWhichCast(RES_CHRATR_FONT);
            rFnt.SetName( rFontItem.GetFamilyName(), SwFontScript::Latin );
            rFnt.SetStyleName( rFontItem.GetStyleName(), SwFontScript::Latin );
            rFnt.SetFamily( rFontItem.GetFamily(), SwFontScript::Latin );
            rFnt.SetPitch( rFontItem.GetPitch(), SwFontScript::Latin );
            rFnt.SetCharSet( rFontItem.GetCharSet(), SwFontScript::Latin );
            break;
        }
        case RES_CHRATR_FONTSIZE :
            rFnt.SetSize(Size(0, rItem.StaticWhichCast(RES_CHRATR_FONTSIZE).GetHeight() ), SwFontScript::Latin );
            break;
        case RES_CHRATR_KERNING :
            rFnt.SetFixKerning( rItem.StaticWhichCast(RES_CHRATR_KERNING).GetValue() );
            break;
        case RES_CHRATR_LANGUAGE :
            rFnt.SetLanguage( rItem.StaticWhichCast(RES_CHRATR_LANGUAGE).GetLanguage(), SwFontScript::Latin );
            break;
        case RES_CHRATR_POSTURE :
            rFnt.SetItalic( rItem.StaticWhichCast(RES_CHRATR_POSTURE).GetPosture(), SwFontScript::Latin );
            break;
        case RES_CHRATR_SHADOWED :
            rFnt.SetShadow( rItem.StaticWhichCast(RES_CHRATR_SHADOWED).GetValue() );
            break;
        case RES_CHRATR_UNDERLINE :
        {
            const sal_uInt16 nStackPos = StackPos[ RES_CHRATR_HIDDEN ];
            const SwTextAttr* pTopAt = GetTop(nStackPos);

            const SfxPoolItem* pTmpItem = pTopAt ?
                                          CharFormat::GetItem( *pTopAt, RES_CHRATR_HIDDEN ) :
                                          m_pDefaultArray[ nStackPos ];

            if ((m_pShell && !m_pShell->GetWin()) ||
                (pTmpItem && !pTmpItem->StaticWhichCast(RES_CHRATR_HIDDEN).GetValue()) )
            {
                rFnt.SetUnderline( rItem.StaticWhichCast(RES_CHRATR_UNDERLINE).GetLineStyle() );
                rFnt.SetUnderColor( rItem.StaticWhichCast(RES_CHRATR_UNDERLINE).GetColor() );
            }
            break;
        }
        case RES_CHRATR_BOX:
        {
            const SvxBoxItem& aBoxItem = rItem.StaticWhichCast(RES_CHRATR_BOX);
            rFnt.SetTopBorder( aBoxItem.GetTop() );
            rFnt.SetBottomBorder( aBoxItem.GetBottom() );
            rFnt.SetRightBorder( aBoxItem.GetRight() );
            rFnt.SetLeftBorder( aBoxItem.GetLeft() );
            rFnt.SetTopBorderDist( aBoxItem.GetDistance(SvxBoxItemLine::TOP) );
            rFnt.SetBottomBorderDist( aBoxItem.GetDistance(SvxBoxItemLine::BOTTOM) );
            rFnt.SetRightBorderDist( aBoxItem.GetDistance(SvxBoxItemLine::RIGHT) );
            rFnt.SetLeftBorderDist( aBoxItem.GetDistance(SvxBoxItemLine::LEFT) );
            break;
        }
        case RES_CHRATR_SHADOW:
        {
            const SvxShadowItem& aShadowItem = rItem.StaticWhichCast(RES_CHRATR_SHADOW);
            rFnt.SetShadowColor( aShadowItem.GetColor() );
            rFnt.SetShadowWidth( aShadowItem.GetWidth() );
            rFnt.SetShadowLocation( aShadowItem.GetLocation() );
            break;
        }
        case RES_CHRATR_OVERLINE :
            rFnt.SetOverline( rItem.StaticWhichCast(RES_CHRATR_OVERLINE).GetLineStyle() );
            rFnt.SetOverColor( rItem.StaticWhichCast(RES_CHRATR_OVERLINE).GetColor() );
            break;
        case RES_CHRATR_WEIGHT :
            rFnt.SetWeight( rItem.StaticWhichCast(RES_CHRATR_WEIGHT).GetWeight(), SwFontScript::Latin );
            break;
        case RES_CHRATR_WORDLINEMODE :
            rFnt.SetWordLineMode( rItem.StaticWhichCast(RES_CHRATR_WORDLINEMODE).GetValue() );
            break;
        case RES_CHRATR_AUTOKERN :
            if( rItem.StaticWhichCast(RES_CHRATR_AUTOKERN).GetValue() )
            {
                rFnt.SetAutoKern( (!m_pIDocumentSettingAccess ||
                                   !m_pIDocumentSettingAccess->get(DocumentSettingId::KERN_ASIAN_PUNCTUATION)) ?
                                     FontKerning::FontSpecific :
                                     FontKerning::Asian );
            }
            else
                rFnt.SetAutoKern( FontKerning::NONE );
            break;
        case RES_CHRATR_BACKGROUND :
            rFnt.SetBackColor(rItem.StaticWhichCast(RES_CHRATR_BACKGROUND).GetColor());
            break;
        case RES_CHRATR_HIGHLIGHT :
            rFnt.SetHighlightColor( rItem.StaticWhichCast(RES_CHRATR_HIGHLIGHT).GetColor() );
            break;
        case RES_CHRATR_CJK_FONT :
        {
            auto& rFontItem = rItem.StaticWhichCast(RES_CHRATR_CJK_FONT);
            rFnt.SetName( rFontItem.GetFamilyName(), SwFontScript::CJK );
            rFnt.SetStyleName( rFontItem.GetStyleName(), SwFontScript::CJK );
            rFnt.SetFamily( rFontItem.GetFamily(), SwFontScript::CJK );
            rFnt.SetPitch( rFontItem.GetPitch(), SwFontScript::CJK );
            rFnt.SetCharSet( rFontItem.GetCharSet(), SwFontScript::CJK );
            break;
        }
        case RES_CHRATR_CJK_FONTSIZE :
            rFnt.SetSize(Size( 0, rItem.StaticWhichCast(RES_CHRATR_CJK_FONTSIZE).GetHeight()), SwFontScript::CJK);
            break;
        case RES_CHRATR_CJK_LANGUAGE :
            rFnt.SetLanguage( rItem.StaticWhichCast(RES_CHRATR_CJK_LANGUAGE).GetLanguage(), SwFontScript::CJK );
            break;
        case RES_CHRATR_CJK_POSTURE :
            rFnt.SetItalic( rItem.StaticWhichCast(RES_CHRATR_CJK_POSTURE).GetPosture(), SwFontScript::CJK );
            break;
        case RES_CHRATR_CJK_WEIGHT :
            rFnt.SetWeight( rItem.StaticWhichCast(RES_CHRATR_CJK_WEIGHT).GetWeight(), SwFontScript::CJK );
            break;
        case RES_CHRATR_CTL_FONT :
        {
            auto& rFontItem = rItem.StaticWhichCast(RES_CHRATR_CTL_FONT);
            rFnt.SetName( rFontItem.GetFamilyName(), SwFontScript::CTL );
            rFnt.SetStyleName( rFontItem.GetStyleName(), SwFontScript::CTL );
            rFnt.SetFamily( rFontItem.GetFamily(), SwFontScript::CTL );
            rFnt.SetPitch( rFontItem.GetPitch(), SwFontScript::CTL );
            rFnt.SetCharSet( rFontItem.GetCharSet(), SwFontScript::CTL );
            break;
        }
        case RES_CHRATR_CTL_FONTSIZE :
            rFnt.SetSize(Size(0, rItem.StaticWhichCast(RES_CHRATR_CTL_FONTSIZE).GetHeight() ), SwFontScript::CTL);
            break;
        case RES_CHRATR_CTL_LANGUAGE :
            rFnt.SetLanguage( rItem.StaticWhichCast(RES_CHRATR_CTL_LANGUAGE).GetLanguage(), SwFontScript::CTL );
            break;
        case RES_CHRATR_CTL_POSTURE :
            rFnt.SetItalic( rItem.StaticWhichCast(RES_CHRATR_CTL_POSTURE).GetPosture(), SwFontScript::CTL );
            break;
        case RES_CHRATR_CTL_WEIGHT :
            rFnt.SetWeight( rItem.StaticWhichCast(RES_CHRATR_CTL_WEIGHT).GetWeight(), SwFontScript::CTL );
            break;
        case RES_CHRATR_EMPHASIS_MARK :
            rFnt.SetEmphasisMark( rItem.StaticWhichCast(RES_CHRATR_EMPHASIS_MARK).GetEmphasisMark() );
            break;
        case RES_CHRATR_SCALEW :
            rFnt.SetPropWidth( rItem.StaticWhichCast(RES_CHRATR_SCALEW).GetValue() );
            break;
        case RES_CHRATR_RELIEF :
            rFnt.SetRelief( rItem.StaticWhichCast(RES_CHRATR_RELIEF).GetValue() );
            break;
        case RES_CHRATR_HIDDEN :
            if (m_pShell && m_pShell->GetWin())
            {
                if ( rItem.StaticWhichCast(RES_CHRATR_HIDDEN).GetValue() )
                    rFnt.SetUnderline( LINESTYLE_DOTTED );
                else
                    ActivateTop( rFnt, RES_CHRATR_UNDERLINE );
            }
            break;
        case RES_CHRATR_ROTATE :
        {
            // rotate attribute is applied, when:
            // 1. ruby stack is empty and
            // 2. top of two line stack ( or default attribute )is an
            //    deactivated two line attribute
            const bool bRuby =
                0 != m_aAttrStack[ StackPos[ RES_TXTATR_CJK_RUBY ] ].size();

            if ( bRuby )
                break;

            const sal_uInt16 nTwoLineStack = StackPos[ RES_CHRATR_TWO_LINES ];
            bool bTwoLineAct = false;
            const SwTextAttr* pTwoLineAttr = GetTop(nTwoLineStack);

            if ( pTwoLineAttr )
            {
                const auto& rTwoLineItem = *CharFormat::GetItem( *pTwoLineAttr, RES_CHRATR_TWO_LINES );
                bTwoLineAct = rTwoLineItem.GetValue();
            }
            else
                bTwoLineAct = m_pDefaultArray[ nTwoLineStack ]->StaticWhichCast(RES_CHRATR_TWO_LINES).GetValue();

            if ( !bTwoLineAct )
                rFnt.SetVertical( rItem.StaticWhichCast(RES_CHRATR_ROTATE).GetValue(), m_bVertLayout, m_bVertLayoutLRBT );

            break;
        }
        case RES_CHRATR_TWO_LINES :
        {
            bool bRuby = 0 !=
                    m_aAttrStack[ StackPos[ RES_TXTATR_CJK_RUBY ] ].size();

            // two line is activated, if
            // 1. no ruby attribute is set and
            // 2. attribute is active
            if ( !bRuby && rItem.StaticWhichCast(RES_CHRATR_TWO_LINES).GetValue() )
            {
                rFnt.SetVertical( 0_deg10, m_bVertLayout );
                break;
            }

            // a deactivating two line attribute is on top of stack,
            // check if rotate attribute has to be enabled
            if ( bRuby )
                break;

            const sal_uInt16 nRotateStack = StackPos[ RES_CHRATR_ROTATE ];
            const SwTextAttr* pRotateAttr = GetTop(nRotateStack);

            if ( pRotateAttr )
            {
                const auto& rRotateItem = *CharFormat::GetItem( *pRotateAttr, RES_CHRATR_ROTATE );
                rFnt.SetVertical( rRotateItem.GetValue(), m_bVertLayout );
            }
            else
                rFnt.SetVertical(m_pDefaultArray[ nRotateStack ]->StaticWhichCast(RES_CHRATR_ROTATE).GetValue(), m_bVertLayout);
            break;
        }
        case RES_TXTATR_CJK_RUBY :
            rFnt.SetVertical( 0_deg10, m_bVertLayout );
            break;
        case RES_TXTATR_REFMARK :
            if ( bPush )
                rFnt.GetRef()++;
            else
                rFnt.GetRef()--;
            break;
        case RES_TXTATR_TOXMARK :
            if ( bPush )
                rFnt.GetTox()++;
            else
                rFnt.GetTox()--;
            break;
        case RES_TXTATR_META:
        case RES_TXTATR_METAFIELD:
            if ( bPush )
                rFnt.GetMeta()++;
            else
                rFnt.GetMeta()--;
            break;
        case RES_TXTATR_CONTENTCONTROL:
            if (bPush)
            {
                rFnt.GetContentControl()++;
            }
            else
            {
                rFnt.GetContentControl()--;
            }
            break;
        case RES_TXTATR_INPUTFIELD :
            if ( bPush )
                rFnt.GetInputField()++;
            else
                rFnt.GetInputField()--;
            break;
    }
}

/// Takes the default font and calculated the ascent and height
void SwAttrHandler::GetDefaultAscentAndHeight( SwViewShell const * pShell, OutputDevice const & rOut,
                                               sal_uInt16& nAscent, sal_uInt16& nHeight ) const
{
    OSL_ENSURE(m_oFnt, "No font available for GetDefaultAscentAndHeight");

    if (m_oFnt)
    {
        SwFont aFont( *m_oFnt );
        nHeight = aFont.GetHeight( pShell, rOut );
        nAscent = aFont.GetAscent( pShell, rOut );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
