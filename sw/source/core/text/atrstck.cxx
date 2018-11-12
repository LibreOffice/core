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
#include <editeng/blinkitem.hxx>
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

#define STACK_INCREMENT 4

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
};

namespace CharFormat
{

/// Returns the item set associated with an character/inet/auto style
const SfxItemSet* GetItemSet( const SfxPoolItem& rAttr )
{
    const SfxItemSet* pSet = nullptr;

    if ( RES_TXTATR_AUTOFMT == rAttr.Which() )
    {
        pSet = static_cast<const SwFormatAutoFormat&>(rAttr).GetStyleHandle().get();
    }
    else
    {
        // Get the attributes from the template
        const SwCharFormat* pFormat = RES_TXTATR_INETFMT == rAttr.Which() ?
                        static_cast<const SwFormatINetFormat&>(rAttr).GetTextINetFormat()->GetCharFormat() :
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
                const SfxPoolItem* pItem;
                if (SfxItemState::SET == pTmpFormat->GetItemState(RES_CHRATR_COLOR, true, &pItem))
                    *pColor = static_cast<const SvxColorItem*>(pItem)->GetValue();
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
          (rINetAttr.IsVisited() && SwViewOption::IsVisitedLinks()) ||
          (!rINetAttr.IsVisited() && SwViewOption::IsLinks())
        )
       )
    {
        if ( pColor )
        {
            if (rINetAttr.IsVisited())
            {
                // take color from view option 'visited link color'
                *pColor = SwViewOption::GetVisitedLinksColor();
            }
            else
            {
                // take color from view option 'unvisited link color'
                *pColor = SwViewOption::GetLinksColor();
            }
        }
        return true;
    }

    return false;
}

inline SwAttrHandler::SwAttrStack::SwAttrStack()
    : m_nCount( 0 ), m_nSize( INITIAL_NUM_ATTR )
{
    m_pArray = m_pInitialArray;
}

void SwAttrHandler::SwAttrStack::Insert( const SwTextAttr& rAttr, const sal_uInt16 nPos )
{
    // do we still have enough space?
    if (m_nCount >= m_nSize)
    {
         // we are still in our initial array
        if (INITIAL_NUM_ATTR == m_nSize)
        {
            m_nSize += STACK_INCREMENT;
            m_pArray = new SwTextAttr*[ m_nSize ];
            // copy from pInitArray to new Array
            memcpy( m_pArray, m_pInitialArray,
                    INITIAL_NUM_ATTR * sizeof(SwTextAttr*)
                    );
        }
        // we are in new memory
        else
        {
            m_nSize += STACK_INCREMENT;
            SwTextAttr** pTmpArray = new SwTextAttr*[ m_nSize ];
            // copy from m_pArray to new Array
            memcpy( pTmpArray, m_pArray, m_nCount * sizeof(SwTextAttr*) );
            // free old array
            delete [] m_pArray;
            m_pArray = pTmpArray;
        }
    }

    OSL_ENSURE(nPos <= m_nCount, "wrong position for insert operation");

    if (nPos < m_nCount)
        memmove( m_pArray + nPos + 1, m_pArray + nPos,
                (m_nCount - nPos) * sizeof(SwTextAttr*)
                );
    m_pArray[ nPos ] = const_cast<SwTextAttr*>(&rAttr);

    m_nCount++;
}

void SwAttrHandler::SwAttrStack::Remove( const SwTextAttr& rAttr )
{
    sal_uInt16 nPos = Pos( rAttr );
    if (nPos < m_nCount)
    {
        memmove( m_pArray + nPos, m_pArray + nPos + 1,
                (m_nCount - 1 - nPos) * sizeof(SwTextAttr*)
                );
        m_nCount--;
    }
}

const SwTextAttr* SwAttrHandler::SwAttrStack::Top() const
{
    return m_nCount ? m_pArray[ m_nCount - 1 ] : nullptr;
}

sal_uInt16 SwAttrHandler::SwAttrStack::Pos( const SwTextAttr& rAttr ) const
{
    if ( ! m_nCount )
        // empty stack
        return USHRT_MAX;

    for (sal_uInt16 nIdx = m_nCount; nIdx > 0;)
    {
        if (&rAttr == m_pArray[ --nIdx ])
            return nIdx;
    }

    // element not found
    return USHRT_MAX;
}

SwAttrHandler::SwAttrHandler()
    : m_pIDocumentSettingAccess(nullptr)
    , m_pShell(nullptr)
    , m_bVertLayout(false)
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
                          SwFont& rFnt, bool bVL )
{
    // initialize default array
    memcpy( m_pDefaultArray, pPoolItem,
            NUM_DEFAULT_VALUES * sizeof(SfxPoolItem*) );

    m_pIDocumentSettingAccess = &rIDocumentSettingAcces;
    m_pShell = pSh;

    // do we have to apply additional paragraph attributes?
    m_bVertLayout = bVL;

    if ( pAS && pAS->Count() )
    {
        SfxItemIter aIter( *pAS );
        sal_uInt16 nWhich;
        const SfxPoolItem* pItem = aIter.GetCurItem();
        while( true )
        {
            nWhich = pItem->Which();
            if (isCHRATR(nWhich))
            {
                m_pDefaultArray[ StackPos[ nWhich ] ] = pItem;
                FontChg( *pItem, rFnt, true );
            }

            if( aIter.IsAtEnd() )
                break;

            pItem = aIter.NextItem();
        }
    }

    // It is possible, that Init is called more than once, e.g., in a
    // SwTextFrame::FormatOnceMore situation or (since sw_redlinehide)
    // from SwAttrIter::Seek(); in the latter case SwTextSizeInfo::m_pFnt
    // is an alias of m_pFnt so it must not be deleted!
    if (m_pFnt)
    {
        *m_pFnt = rFnt;
    }
    else
    {
        m_pFnt.reset(new SwFont(rFnt));
    }
}

void SwAttrHandler::Reset( )
{
    for (SwAttrStack & i : m_aAttrStack)
        i.Reset();
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
    const SwTextAttr* pTopAttr = m_aAttrStack[ nStack ].Top();
    if ( !pTopAttr
         || rAttr.IsPriorityAttr()
         || ( !pTopAttr->IsPriorityAttr()
              && !lcl_ChgHyperLinkColor(*pTopAttr, rItem, m_pShell, nullptr)))
    {
        m_aAttrStack[ nStack ].Push( rAttr );
        return true;
    }

    const sal_uInt16 nPos = m_aAttrStack[ nStack ].Count();
    OSL_ENSURE( nPos, "empty stack?" );
    m_aAttrStack[ nStack ].Insert( rAttr, nPos - 1 );
    return false;
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
                const sal_uInt16 nStackPos = StackPos[ i ];
                m_aAttrStack[ nStackPos ].Remove( rAttr );
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
        m_aAttrStack[ StackPos[ rAttr.Which() ] ].Remove( rAttr );
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
        m_aAttrStack[ StackPos[ rAttr.Which() ] ].Remove( rAttr );
    }
}

void SwAttrHandler::ActivateTop( SwFont& rFnt, const sal_uInt16 nAttr )
{
    OSL_ENSURE( nAttr < RES_TXTATR_WITHEND_END,
            "I cannot activate this attribute, nWhich >= RES_TXTATR_WITHEND_END" );

    const sal_uInt16 nStackPos = StackPos[ nAttr ];
    const SwTextAttr* pTopAt = m_aAttrStack[ nStackPos ].Top();
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
    else if ( RES_TXTATR_CJK_RUBY == nAttr )
    {
        // ruby stack has no more attributes
        // check, if an rotation attribute has to be applied
        const sal_uInt16 nTwoLineStack = StackPos[ RES_CHRATR_TWO_LINES ];
        bool bTwoLineAct = false;
        const SwTextAttr* pTwoLineAttr = m_aAttrStack[ nTwoLineStack ].Top();

        if ( pTwoLineAttr )
        {
             const SfxPoolItem* pTwoLineItem = CharFormat::GetItem( *pTwoLineAttr, RES_CHRATR_TWO_LINES );
             bTwoLineAct = static_cast<const SvxTwoLinesItem*>(pTwoLineItem)->GetValue();
        }
        else
            bTwoLineAct =
                static_cast<const SvxTwoLinesItem*>(m_pDefaultArray[ nTwoLineStack ])->GetValue();

        if ( bTwoLineAct )
            return;

        // eventually, an rotate attribute has to be activated
        const sal_uInt16 nRotateStack = StackPos[ RES_CHRATR_ROTATE ];
        const SwTextAttr* pRotateAttr = m_aAttrStack[ nRotateStack ].Top();

        if ( pRotateAttr )
        {
            const SfxPoolItem* pRotateItem = CharFormat::GetItem( *pRotateAttr, RES_CHRATR_ROTATE );
            rFnt.SetVertical( static_cast<const SvxCharRotateItem*>(pRotateItem)->GetValue(),
                               m_bVertLayout );
        }
        else
            rFnt.SetVertical(
                static_cast<const SvxCharRotateItem*>(m_pDefaultArray[ nRotateStack ])->GetValue(),
                 m_bVertLayout
            );
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
            rFnt.SetCaseMap( static_cast<const SvxCaseMapItem&>(rItem).GetCaseMap() );
            break;
        case RES_CHRATR_COLOR :
            rFnt.SetColor( static_cast<const SvxColorItem&>(rItem).GetValue() );
            break;
        case RES_CHRATR_CONTOUR :
            rFnt.SetOutline( static_cast<const SvxContourItem&>(rItem).GetValue() );
            break;
        case RES_CHRATR_CROSSEDOUT :
            rFnt.SetStrikeout( static_cast<const SvxCrossedOutItem&>(rItem).GetStrikeout() );
            break;
        case RES_CHRATR_ESCAPEMENT :
            rFnt.SetEscapement( static_cast<const SvxEscapementItem&>(rItem).GetEsc() );
            rFnt.SetProportion( static_cast<const SvxEscapementItem&>(rItem).GetProportionalHeight() );
            break;
        case RES_CHRATR_FONT :
            rFnt.SetName( static_cast<const SvxFontItem&>(rItem).GetFamilyName(), SwFontScript::Latin );
            rFnt.SetStyleName( static_cast<const SvxFontItem&>(rItem).GetStyleName(), SwFontScript::Latin );
            rFnt.SetFamily( static_cast<const SvxFontItem&>(rItem).GetFamily(), SwFontScript::Latin );
            rFnt.SetPitch( static_cast<const SvxFontItem&>(rItem).GetPitch(), SwFontScript::Latin );
            rFnt.SetCharSet( static_cast<const SvxFontItem&>(rItem).GetCharSet(), SwFontScript::Latin );
            break;
        case RES_CHRATR_FONTSIZE :
            rFnt.SetSize(Size(0,static_cast<const SvxFontHeightItem&>(rItem).GetHeight() ), SwFontScript::Latin );
            break;
        case RES_CHRATR_KERNING :
            rFnt.SetFixKerning( static_cast<const SvxKerningItem&>(rItem).GetValue() );
            break;
        case RES_CHRATR_LANGUAGE :
            rFnt.SetLanguage( static_cast<const SvxLanguageItem&>(rItem).GetLanguage(), SwFontScript::Latin );
            break;
        case RES_CHRATR_POSTURE :
            rFnt.SetItalic( static_cast<const SvxPostureItem&>(rItem).GetPosture(), SwFontScript::Latin );
            break;
        case RES_CHRATR_SHADOWED :
            rFnt.SetShadow( static_cast<const SvxShadowedItem&>(rItem).GetValue() );
            break;
        case RES_CHRATR_UNDERLINE :
        {
            const sal_uInt16 nStackPos = StackPos[ RES_CHRATR_HIDDEN ];
            const SwTextAttr* pTopAt = m_aAttrStack[ nStackPos ].Top();

            const SfxPoolItem* pTmpItem = pTopAt ?
                                          CharFormat::GetItem( *pTopAt, RES_CHRATR_HIDDEN ) :
                                          m_pDefaultArray[ nStackPos ];

            if ((m_pShell && !m_pShell->GetWin()) ||
                (pTmpItem && !static_cast<const SvxCharHiddenItem*>(pTmpItem)->GetValue()) )
            {
                rFnt.SetUnderline( static_cast<const SvxUnderlineItem&>(rItem).GetLineStyle() );
                rFnt.SetUnderColor( static_cast<const SvxUnderlineItem&>(rItem).GetColor() );
            }
            break;
        }
        case RES_CHRATR_BOX:
        {
            const SvxBoxItem& aBoxItem = static_cast<const SvxBoxItem&>(rItem);
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
            const SvxShadowItem& aShadowItem = static_cast<const SvxShadowItem&>(rItem);
            rFnt.SetShadowColor( aShadowItem.GetColor() );
            rFnt.SetShadowWidth( aShadowItem.GetWidth() );
            rFnt.SetShadowLocation( aShadowItem.GetLocation() );
            break;
        }
        case RES_CHRATR_OVERLINE :
            rFnt.SetOverline( static_cast<const SvxOverlineItem&>(rItem).GetLineStyle() );
            rFnt.SetOverColor( static_cast<const SvxOverlineItem&>(rItem).GetColor() );
            break;
        case RES_CHRATR_WEIGHT :
            rFnt.SetWeight( static_cast<const SvxWeightItem&>(rItem).GetWeight(), SwFontScript::Latin );
            break;
        case RES_CHRATR_WORDLINEMODE :
            rFnt.SetWordLineMode( static_cast<const SvxWordLineModeItem&>(rItem).GetValue() );
            break;
        case RES_CHRATR_AUTOKERN :
            if( static_cast<const SvxAutoKernItem&>(rItem).GetValue() )
            {
                rFnt.SetAutoKern( (!m_pIDocumentSettingAccess ||
                                   !m_pIDocumentSettingAccess->get(DocumentSettingId::KERN_ASIAN_PUNCTUATION)) ?
                                     FontKerning::FontSpecific :
                                     FontKerning::Asian );
            }
            else
                rFnt.SetAutoKern( FontKerning::NONE );
            break;
        case RES_CHRATR_BLINK :
            rFnt.SetBlink( static_cast<const SvxBlinkItem&>(rItem).GetValue() );
            break;
        case RES_CHRATR_BACKGROUND :
            rFnt.SetBackColor(new Color( static_cast<const SvxBrushItem&>(rItem).GetColor() ) );
            break;
        case RES_CHRATR_HIGHLIGHT :
            rFnt.SetHighlightColor( static_cast<const SvxBrushItem&>(rItem).GetColor() );
            break;
        case RES_CHRATR_CJK_FONT :
            rFnt.SetName( static_cast<const SvxFontItem&>(rItem).GetFamilyName(), SwFontScript::CJK );
            rFnt.SetStyleName( static_cast<const SvxFontItem&>(rItem).GetStyleName(), SwFontScript::CJK );
            rFnt.SetFamily( static_cast<const SvxFontItem&>(rItem).GetFamily(), SwFontScript::CJK );
            rFnt.SetPitch( static_cast<const SvxFontItem&>(rItem).GetPitch(), SwFontScript::CJK );
            rFnt.SetCharSet( static_cast<const SvxFontItem&>(rItem).GetCharSet(), SwFontScript::CJK );
            break;
        case RES_CHRATR_CJK_FONTSIZE :
            rFnt.SetSize(Size( 0, static_cast<const SvxFontHeightItem&>(rItem).GetHeight()), SwFontScript::CJK);
            break;
        case RES_CHRATR_CJK_LANGUAGE :
            rFnt.SetLanguage( static_cast<const SvxLanguageItem&>(rItem).GetLanguage(), SwFontScript::CJK );
            break;
        case RES_CHRATR_CJK_POSTURE :
            rFnt.SetItalic( static_cast<const SvxPostureItem&>(rItem).GetPosture(), SwFontScript::CJK );
            break;
        case RES_CHRATR_CJK_WEIGHT :
            rFnt.SetWeight( static_cast<const SvxWeightItem&>(rItem).GetWeight(), SwFontScript::CJK );
            break;
        case RES_CHRATR_CTL_FONT :
            rFnt.SetName( static_cast<const SvxFontItem&>(rItem).GetFamilyName(), SwFontScript::CTL );
            rFnt.SetStyleName( static_cast<const SvxFontItem&>(rItem).GetStyleName(), SwFontScript::CTL );
            rFnt.SetFamily( static_cast<const SvxFontItem&>(rItem).GetFamily(), SwFontScript::CTL );
            rFnt.SetPitch( static_cast<const SvxFontItem&>(rItem).GetPitch(), SwFontScript::CTL );
            rFnt.SetCharSet( static_cast<const SvxFontItem&>(rItem).GetCharSet(), SwFontScript::CTL );
            break;
        case RES_CHRATR_CTL_FONTSIZE :
            rFnt.SetSize(Size(0, static_cast<const SvxFontHeightItem&>(rItem).GetHeight() ), SwFontScript::CTL);
            break;
        case RES_CHRATR_CTL_LANGUAGE :
            rFnt.SetLanguage( static_cast<const SvxLanguageItem&>(rItem).GetLanguage(), SwFontScript::CTL );
            break;
        case RES_CHRATR_CTL_POSTURE :
            rFnt.SetItalic( static_cast<const SvxPostureItem&>(rItem).GetPosture(), SwFontScript::CTL );
            break;
        case RES_CHRATR_CTL_WEIGHT :
            rFnt.SetWeight( static_cast<const SvxWeightItem&>(rItem).GetWeight(), SwFontScript::CTL );
            break;
        case RES_CHRATR_EMPHASIS_MARK :
            rFnt.SetEmphasisMark(
                     static_cast<const SvxEmphasisMarkItem&>(rItem).GetEmphasisMark()
                     );
            break;
        case RES_CHRATR_SCALEW :
            rFnt.SetPropWidth( static_cast<const SvxCharScaleWidthItem&>(rItem).GetValue() );
            break;
        case RES_CHRATR_RELIEF :
            rFnt.SetRelief( static_cast<const SvxCharReliefItem&>(rItem).GetValue() );
            break;
        case RES_CHRATR_HIDDEN :
            if (m_pShell && m_pShell->GetWin())
            {
                if ( static_cast<const SvxCharHiddenItem&>(rItem).GetValue() )
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
                0 != m_aAttrStack[ StackPos[ RES_TXTATR_CJK_RUBY ] ].Count();

            if ( bRuby )
                break;

            const sal_uInt16 nTwoLineStack = StackPos[ RES_CHRATR_TWO_LINES ];
            bool bTwoLineAct = false;
            const SwTextAttr* pTwoLineAttr = m_aAttrStack[ nTwoLineStack ].Top();

            if ( pTwoLineAttr )
            {
                const SfxPoolItem* pTwoLineItem = CharFormat::GetItem( *pTwoLineAttr, RES_CHRATR_TWO_LINES );
                bTwoLineAct = static_cast<const SvxTwoLinesItem*>(pTwoLineItem)->GetValue();
            }
            else
                bTwoLineAct =
                    static_cast<const SvxTwoLinesItem*>(m_pDefaultArray[ nTwoLineStack ])->GetValue();

            if ( !bTwoLineAct )
                rFnt.SetVertical( static_cast<const SvxCharRotateItem&>(rItem).GetValue(),
                                   m_bVertLayout );

            break;
        }
        case RES_CHRATR_TWO_LINES :
        {
            bool bRuby = 0 !=
                    m_aAttrStack[ StackPos[ RES_TXTATR_CJK_RUBY ] ].Count();

            // two line is activated, if
            // 1. no ruby attribute is set and
            // 2. attribute is active
            if ( !bRuby && static_cast<const SvxTwoLinesItem&>(rItem).GetValue() )
            {
                rFnt.SetVertical( 0, m_bVertLayout );
                break;
            }

            // a deactivating two line attribute is on top of stack,
            // check if rotate attribute has to be enabled
            if ( bRuby )
                break;

            const sal_uInt16 nRotateStack = StackPos[ RES_CHRATR_ROTATE ];
            const SwTextAttr* pRotateAttr = m_aAttrStack[ nRotateStack ].Top();

            if ( pRotateAttr )
            {
                const SfxPoolItem* pRotateItem = CharFormat::GetItem( *pRotateAttr, RES_CHRATR_ROTATE );
                rFnt.SetVertical( static_cast<const SvxCharRotateItem*>(pRotateItem)->GetValue(),
                                   m_bVertLayout );
            }
            else
                rFnt.SetVertical(
                    static_cast<const SvxCharRotateItem*>(m_pDefaultArray[ nRotateStack ])->GetValue(),
                     m_bVertLayout
                );
            break;
        }
        case RES_TXTATR_CJK_RUBY :
            rFnt.SetVertical( 0, m_bVertLayout );
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
    OSL_ENSURE(m_pFnt, "No font available for GetDefaultAscentAndHeight");

    if (m_pFnt)
    {
        SwFont aFont( *m_pFnt );
        nHeight = aFont.GetHeight( pShell, rOut );
        nAscent = aFont.GetAscent( pShell, rOut );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
