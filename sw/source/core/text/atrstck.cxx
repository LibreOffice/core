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

#include <atrhndl.hxx>
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

/*
 *                      Attribute to Stack Mapping
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
const sal_uInt8 StackPos[ static_cast<sal_uInt16>(RES_TXTATR_WITHEND_END) -
                     static_cast<sal_uInt16>(RES_CHRATR_BEGIN) + 1 ] =
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
     0, // RES_CHRATR_PROPORTIONALFONTSIZE,      // 12
    11, // RES_CHRATR_SHADOWED,                  // 13
    12, // RES_CHRATR_UNDERLINE,                 // 14
    13, // RES_CHRATR_WEIGHT,                    // 15
    14, // RES_CHRATR_WORDLINEMODE,              // 16
    15, // RES_CHRATR_AUTOKERN,                  // 17
    16, // RES_CHRATR_BLINK,                     // 18
    17, // RES_CHRATR_NOHYPHEN,                  // 19
     0, // RES_CHRATR_NOLINEBREAK,               // 20
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

namespace CharFmt
{

// Returns the item set associated with an character/inet/auto style
const SfxItemSet* GetItemSet( const SfxPoolItem& rAttr )
{
    const SfxItemSet* pSet = 0;

    if ( RES_TXTATR_AUTOFMT == rAttr.Which() )
    {
        pSet = static_cast<const SwFmtAutoFmt&>(rAttr).GetStyleHandle().get();
    }
    else
    {
        // Get the attributes from the template
        SwCharFmt* pFmt = RES_TXTATR_INETFMT == rAttr.Which() ?
                        ((SwFmtINetFmt&)rAttr).GetTxtINetFmt()->GetCharFmt() :
                        ((SwFmtCharFmt&)rAttr).GetCharFmt();
        if( pFmt )
        {
            pSet = &pFmt->GetAttrSet();
        }
    }

    return pSet;
}

// Extracts pool item of type nWhich from rAttr
const SfxPoolItem* GetItem( const SwTxtAttr& rAttr, sal_uInt16 nWhich )
{
    if ( RES_TXTATR_INETFMT == rAttr.Which() ||
         RES_TXTATR_CHARFMT == rAttr.Which() ||
         RES_TXTATR_AUTOFMT == rAttr.Which() )
    {
        const SfxItemSet* pSet = CharFmt::GetItemSet( rAttr.GetAttr() );
        if ( !pSet ) return 0;

       bool bInParent = RES_TXTATR_AUTOFMT != rAttr.Which();
       const SfxPoolItem* pItem;
       bool bRet = SFX_ITEM_SET == pSet->GetItemState( nWhich, bInParent, &pItem );

       return bRet ? pItem : 0;
    }

    return ( nWhich == rAttr.Which() ) ? &rAttr.GetAttr() : 0;
}

// checks if item is included in character/inet/auto style
bool IsItemIncluded( const sal_uInt16 nWhich, const SwTxtAttr *pAttr )
{
    bool bRet = false;

    const SfxItemSet* pItemSet = CharFmt::GetItemSet( pAttr->GetAttr() );
    if ( pItemSet )
        bRet = SFX_ITEM_SET == pItemSet->GetItemState( nWhich, true );

    return bRet;
}
}

// The color of hyperlinks is taken from the associated character attribute,
// depending on its 'visited' state. There are actually two cases, which
// should override the colors from the character attribute:
// 1. We never take the 'visited' color during printing/pdf export/preview
// 2. The user has chosen to override these colors in the view options
static bool lcl_ChgHyperLinkColor( const SwTxtAttr& rAttr,
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

    if ( pShell->GetOut()->GetOutDevType() == OUTDEV_PRINTER ||
         pShell->GetViewOptions()->IsPDFExport() ||
         pShell->GetViewOptions()->IsPagePreview() )
    {
        if ( ((SwTxtINetFmt&)rAttr).IsVisited() )
        {
            if ( pColor )
            {
                // take color from character format 'unvisited link'
                SwTxtINetFmt& rInetAttr( const_cast<SwTxtINetFmt&>(
                    static_cast<const SwTxtINetFmt&>(rAttr)) );
                rInetAttr.SetVisited( false );
                const SwCharFmt* pTmpFmt = ((SwTxtINetFmt&)rAttr).GetCharFmt();
                const SfxPoolItem* pItem;
                pTmpFmt->GetItemState( RES_CHRATR_COLOR, true, &pItem );
                *pColor = ((SvxColorItem*)pItem)->GetValue();
                rInetAttr.SetVisited( true );
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
          (((SwTxtINetFmt&)rAttr).IsVisited() && SwViewOption::IsVisitedLinks()) ||
          (!((SwTxtINetFmt&)rAttr).IsVisited() && SwViewOption::IsLinks())
        )
       )
    {
        if ( pColor )
        {
            if ( ((SwTxtINetFmt&)rAttr).IsVisited() )
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
    : nCount( 0 ), nSize( INITIAL_NUM_ATTR )
{
    pArray = pInitialArray;
}

void SwAttrHandler::SwAttrStack::Insert( const SwTxtAttr& rAttr, const sal_uInt16 nPos )
{
    // do we still have enough space?
    if ( nCount >= nSize )
    {
         // we are still in our initial array
        if ( INITIAL_NUM_ATTR == nSize )
        {
            nSize += STACK_INCREMENT;
            pArray = new SwTxtAttr*[ nSize ];
            // copy from pInitArray to new Array
            memcpy( pArray, pInitialArray,
                    INITIAL_NUM_ATTR * sizeof(SwTxtAttr*)
                    );
        }
        // we are in new memory
        else
        {
            nSize += STACK_INCREMENT;
            SwTxtAttr** pTmpArray = new SwTxtAttr*[ nSize ];
            // copy from pArray to new Array
            memcpy( pTmpArray, pArray, nCount * sizeof(SwTxtAttr*) );
            // free old array
            delete [] pArray;
            pArray = pTmpArray;
        }
    }

    OSL_ENSURE( nPos <= nCount, "wrong position for insert operation");

    if ( nPos < nCount )
        memmove( pArray + nPos + 1, pArray + nPos,
                ( nCount - nPos ) * sizeof(SwTxtAttr*)
                );
    pArray[ nPos ] = (SwTxtAttr*)&rAttr;

    nCount++;
}

void SwAttrHandler::SwAttrStack::Remove( const SwTxtAttr& rAttr )
{
    sal_uInt16 nPos = Pos( rAttr );
    if ( nPos < nCount )
    {
        memmove( pArray + nPos, pArray + nPos + 1,
                ( nCount - 1 - nPos ) * sizeof(SwTxtAttr*)
                );
        nCount--;
    }
}

const SwTxtAttr* SwAttrHandler::SwAttrStack::Top() const
{
    return nCount ? pArray[ nCount - 1 ] : 0;
}

sal_uInt16 SwAttrHandler::SwAttrStack::Pos( const SwTxtAttr& rAttr ) const
{
    if ( ! nCount )
        // empty stack
        return USHRT_MAX;

    for ( sal_uInt16 nIdx = nCount; nIdx > 0; )
    {
        if ( &rAttr == pArray[ --nIdx ] )
            return nIdx;
    }

    // element not found
    return USHRT_MAX;
}

SwAttrHandler::SwAttrHandler()
    : mpIDocumentSettingAccess(NULL)
    , mpShell(NULL)
    , pFnt(NULL)
    , bVertLayout(false)
{
    memset( pDefaultArray, 0, NUM_DEFAULT_VALUES * sizeof(SfxPoolItem*) );
}

SwAttrHandler::~SwAttrHandler()
{
    delete pFnt;
}

void SwAttrHandler::Init( const SwAttrSet& rAttrSet,
                          const IDocumentSettingAccess& rIDocumentSettingAcces,
                          const SwViewShell* pSh )
{
    mpIDocumentSettingAccess = &rIDocumentSettingAcces;
    mpShell = pSh;

    for ( sal_uInt16 i = RES_CHRATR_BEGIN; i < RES_CHRATR_END; i++ )
        pDefaultArray[ StackPos[ i ] ] = &rAttrSet.Get( i, true );
}

void SwAttrHandler::Init( const SfxPoolItem** pPoolItem, const SwAttrSet* pAS,
                          const IDocumentSettingAccess& rIDocumentSettingAcces,
                          const SwViewShell* pSh,
                          SwFont& rFnt, bool bVL )
{
    // initialize default array
    memcpy( pDefaultArray, pPoolItem,
            NUM_DEFAULT_VALUES * sizeof(SfxPoolItem*) );

    mpIDocumentSettingAccess = &rIDocumentSettingAcces;
    mpShell = pSh;

    // do we have to apply additional paragraph attributes?
    bVertLayout = bVL;

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
                pDefaultArray[ StackPos[ nWhich ] ] = pItem;
                FontChg( *pItem, rFnt, true );
            }

            if( aIter.IsAtEnd() )
                break;

            pItem = aIter.NextItem();
        }
    }

    // It is possible, that Init is called more than once, e.g., in a
    // SwTxtFrm::FormatOnceMore situation.
    delete pFnt;
    pFnt = new SwFont( rFnt );
}

void SwAttrHandler::Reset( )
{
    for ( sal_uInt16 i = 0; i < NUM_ATTRIBUTE_STACKS; i++ )
        aAttrStack[ i ].Reset();
}

void SwAttrHandler::PushAndChg( const SwTxtAttr& rAttr, SwFont& rFnt )
{
    // these special attributes in fact represent a collection of attributes
    // they have to be pushed to each stack they belong to
    if ( RES_TXTATR_INETFMT == rAttr.Which() ||
         RES_TXTATR_CHARFMT == rAttr.Which() ||
         RES_TXTATR_AUTOFMT == rAttr.Which() )
    {
        const SfxItemSet* pSet = CharFmt::GetItemSet( rAttr.GetAttr() );
        if ( !pSet ) return;

        for ( sal_uInt16 i = RES_CHRATR_BEGIN; i < RES_CHRATR_END; i++)
        {
            const SfxPoolItem* pItem;
            bool bRet = SFX_ITEM_SET == pSet->GetItemState( i, rAttr.Which() != RES_TXTATR_AUTOFMT, &pItem );

            if ( bRet )
            {
                // we push rAttr onto the appropriate stack
                if ( Push( rAttr, *pItem ) )
                {
                    // we let pItem change rFnt
                    Color aColor;
                    if ( lcl_ChgHyperLinkColor( rAttr, *pItem, mpShell, &aColor ) )
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

bool SwAttrHandler::Push( const SwTxtAttr& rAttr, const SfxPoolItem& rItem )
{
    OSL_ENSURE( rItem.Which() < RES_TXTATR_WITHEND_END,
            "I do not want this attribute, nWhich >= RES_TXTATR_WITHEND_END" );

    // robust
    if ( RES_TXTATR_WITHEND_END <= rItem.Which() )
        return false;

    const sal_uInt16 nStack = StackPos[ rItem.Which() ];

    // attributes originating from redlining have highest priority
    // second priority are hyperlink attributes, which have a color replacement
    const SwTxtAttr* pTopAttr = aAttrStack[ nStack ].Top();
    if ( !pTopAttr
         || rAttr.IsPriorityAttr()
         || ( !pTopAttr->IsPriorityAttr()
              && !lcl_ChgHyperLinkColor( *pTopAttr, rItem, mpShell, 0 ) ) )
    {
        aAttrStack[ nStack ].Push( rAttr );
        return true;
    }

    const sal_uInt16 nPos = aAttrStack[ nStack ].Count();
    OSL_ENSURE( nPos, "empty stack?" );
    aAttrStack[ nStack ].Insert( rAttr, nPos - 1 );
    return false;
}

void SwAttrHandler::PopAndChg( const SwTxtAttr& rAttr, SwFont& rFnt )
{
    if ( RES_TXTATR_WITHEND_END <= rAttr.Which() )
        return; // robust

    // these special attributes in fact represent a collection of attributes
    // they have to be removed from each stack they belong to
    if ( RES_TXTATR_INETFMT == rAttr.Which() ||
         RES_TXTATR_CHARFMT == rAttr.Which() ||
         RES_TXTATR_AUTOFMT == rAttr.Which() )
    {
        const SfxItemSet* pSet = CharFmt::GetItemSet( rAttr.GetAttr() );
        if ( !pSet ) return;

        for ( sal_uInt16 i = RES_CHRATR_BEGIN; i < RES_CHRATR_END; i++)
        {
            const SfxPoolItem* pItem;
            bool bRet = SFX_ITEM_SET == pSet->GetItemState( i, RES_TXTATR_AUTOFMT != rAttr.Which(), &pItem );
            if ( bRet )
            {
                // we remove rAttr from the appropriate stack
                sal_uInt16 nStackPos = StackPos[ i ];
                aAttrStack[ nStackPos ].Remove( rAttr );
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
        aAttrStack[ StackPos[ rAttr.Which() ] ].Remove( rAttr );
        // reset font according to attribute on top of stack
        // or default value
        ActivateTop( rFnt, rAttr.Which() );
    }
}

// only used during redlining
void SwAttrHandler::Pop( const SwTxtAttr& rAttr )
{
    OSL_ENSURE( rAttr.Which() < RES_TXTATR_WITHEND_END,
            "I do not have this attribute, nWhich >= RES_TXTATR_WITHEND_END" );

    if ( rAttr.Which() < RES_TXTATR_WITHEND_END )
    {
        aAttrStack[ StackPos[ rAttr.Which() ] ].Remove( rAttr );
    }
}

void SwAttrHandler::ActivateTop( SwFont& rFnt, const sal_uInt16 nAttr )
{
    OSL_ENSURE( nAttr < RES_TXTATR_WITHEND_END,
            "I cannot activate this attribute, nWhich >= RES_TXTATR_WITHEND_END" );

    const sal_uInt16 nStackPos = StackPos[ nAttr ];
    const SwTxtAttr* pTopAt = aAttrStack[ nStackPos ].Top();
    if ( pTopAt )
    {
        const SfxPoolItem* pItemNext(NULL);

        // check if top attribute is collection of attributes
        if ( RES_TXTATR_INETFMT == pTopAt->Which() ||
             RES_TXTATR_CHARFMT == pTopAt->Which() ||
             RES_TXTATR_AUTOFMT == pTopAt->Which() )
        {
            const SfxItemSet* pSet = CharFmt::GetItemSet( pTopAt->GetAttr() );
            if (pSet)
                pSet->GetItemState( nAttr, RES_TXTATR_AUTOFMT != pTopAt->Which(), &pItemNext );
        }

        if (pItemNext)
        {
            Color aColor;
            if ( lcl_ChgHyperLinkColor( *pTopAt, *pItemNext, mpShell, &aColor ) )
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
        FontChg( *pDefaultArray[ nStackPos ], rFnt, false );
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
        sal_uInt16 nTwoLineStack = StackPos[ RES_CHRATR_TWO_LINES ];
        bool bTwoLineAct = false;
        const SwTxtAttr* pTwoLineAttr = aAttrStack[ nTwoLineStack ].Top();

        if ( pTwoLineAttr )
        {
             const SfxPoolItem* pTwoLineItem = CharFmt::GetItem( *pTwoLineAttr, RES_CHRATR_TWO_LINES );
             bTwoLineAct = ((SvxTwoLinesItem*)pTwoLineItem)->GetValue();
        }
        else
            bTwoLineAct =
                ((SvxTwoLinesItem*)pDefaultArray[ nTwoLineStack ])->GetValue();

        if ( bTwoLineAct )
            return;

        // eventually, an rotate attribute has to be activated
        sal_uInt16 nRotateStack = StackPos[ RES_CHRATR_ROTATE ];
        const SwTxtAttr* pRotateAttr = aAttrStack[ nRotateStack ].Top();

        if ( pRotateAttr )
        {
            const SfxPoolItem* pRotateItem = CharFmt::GetItem( *pRotateAttr, RES_CHRATR_ROTATE );
            rFnt.SetVertical( ((SvxCharRotateItem*)pRotateItem)->GetValue(),
                               bVertLayout );
        }
        else
            rFnt.SetVertical(
                ((SvxCharRotateItem*)pDefaultArray[ nRotateStack ])->GetValue(),
                 bVertLayout
            );
    }
    else if ( RES_TXTATR_INPUTFIELD == nAttr )
        rFnt.GetInputField()--;
}

// When popping an attribute from the stack, the top mose remaining
// attribute in the stack becomes valid. The following function change
// a font depending on the stack id.
void SwAttrHandler::FontChg(const SfxPoolItem& rItem, SwFont& rFnt, bool bPush )
{
    switch ( rItem.Which() )
    {
        case RES_CHRATR_CASEMAP :
            rFnt.SetCaseMap( ((SvxCaseMapItem&)rItem).GetCaseMap() );
            break;
        case RES_CHRATR_COLOR :
            rFnt.SetColor( ((SvxColorItem&)rItem).GetValue() );
            break;
        case RES_CHRATR_CONTOUR :
            rFnt.SetOutline( ((SvxContourItem&)rItem).GetValue() );
            break;
        case RES_CHRATR_CROSSEDOUT :
            rFnt.SetStrikeout( ((SvxCrossedOutItem&)rItem).GetStrikeout() );
            break;
        case RES_CHRATR_ESCAPEMENT :
            rFnt.SetEscapement( ((SvxEscapementItem&)rItem).GetEsc() );
            rFnt.SetProportion( ((SvxEscapementItem&)rItem).GetProp() );
            break;
        case RES_CHRATR_FONT :
            rFnt.SetName( ((SvxFontItem&)rItem).GetFamilyName(), SW_LATIN );
            rFnt.SetStyleName( ((SvxFontItem&)rItem).GetStyleName(), SW_LATIN );
            rFnt.SetFamily( ((SvxFontItem&)rItem).GetFamily(), SW_LATIN );
            rFnt.SetPitch( ((SvxFontItem&)rItem).GetPitch(), SW_LATIN );
            rFnt.SetCharSet( ((SvxFontItem&)rItem).GetCharSet(), SW_LATIN );
            break;
        case RES_CHRATR_FONTSIZE :
            rFnt.SetSize(Size(0,((SvxFontHeightItem&)rItem).GetHeight() ), SW_LATIN );
            break;
        case RES_CHRATR_KERNING :
            rFnt.SetFixKerning( ((SvxKerningItem&)rItem).GetValue() );
            break;
        case RES_CHRATR_LANGUAGE :
            rFnt.SetLanguage( ((SvxLanguageItem&)rItem).GetLanguage(), SW_LATIN );
            break;
        case RES_CHRATR_POSTURE :
            rFnt.SetItalic( ((SvxPostureItem&)rItem).GetPosture(), SW_LATIN );
            break;
        case RES_CHRATR_SHADOWED :
            rFnt.SetShadow( ((SvxShadowedItem&)rItem).GetValue() );
            break;
        case RES_CHRATR_UNDERLINE :
        {
            const sal_uInt16 nStackPos = StackPos[ RES_CHRATR_HIDDEN ];
            const SwTxtAttr* pTopAt = aAttrStack[ nStackPos ].Top();

            const SfxPoolItem* pTmpItem = pTopAt ?
                                          CharFmt::GetItem( *pTopAt, RES_CHRATR_HIDDEN ) :
                                          pDefaultArray[ nStackPos ];

            if( (mpShell && !mpShell->GetWin()) ||
                (pTmpItem && !static_cast<const SvxCharHiddenItem*>(pTmpItem)->GetValue()) )
            {
                rFnt.SetUnderline( ((SvxUnderlineItem&)rItem).GetLineStyle() );
                rFnt.SetUnderColor( ((SvxUnderlineItem&)rItem).GetColor() );
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
            rFnt.SetTopBorderDist( aBoxItem.GetDistance(BOX_LINE_TOP) );
            rFnt.SetBottomBorderDist( aBoxItem.GetDistance(BOX_LINE_BOTTOM) );
            rFnt.SetRightBorderDist( aBoxItem.GetDistance(BOX_LINE_RIGHT) );
            rFnt.SetLeftBorderDist( aBoxItem.GetDistance(BOX_LINE_LEFT) );
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
            rFnt.SetOverline( ((SvxOverlineItem&)rItem).GetLineStyle() );
            rFnt.SetOverColor( ((SvxOverlineItem&)rItem).GetColor() );
            break;
        case RES_CHRATR_WEIGHT :
            rFnt.SetWeight( ((SvxWeightItem&)rItem).GetWeight(), SW_LATIN );
            break;
        case RES_CHRATR_WORDLINEMODE :
            rFnt.SetWordLineMode( ((SvxWordLineModeItem&)rItem).GetValue() );
            break;
        case RES_CHRATR_AUTOKERN :
            if( ((SvxAutoKernItem&)rItem).GetValue() )
            {
                rFnt.SetAutoKern( ( !mpIDocumentSettingAccess ||
                                    !mpIDocumentSettingAccess->get(IDocumentSettingAccess::KERN_ASIAN_PUNCTUATION) ) ?
                                     KERNING_FONTSPECIFIC :
                                     KERNING_ASIAN );
            }
            else
                rFnt.SetAutoKern( 0 );
            break;
        case RES_CHRATR_BLINK :
            rFnt.SetBlink( ((SvxBlinkItem&)rItem).GetValue() );
            break;
        case RES_CHRATR_BACKGROUND :
            rFnt.SetBackColor(new Color( ((SvxBrushItem&)rItem).GetColor() ) );
            break;
        case RES_CHRATR_HIGHLIGHT :
            rFnt.SetHighlightColor( ((SvxBrushItem&)rItem).GetColor() );
            break;
        case RES_CHRATR_CJK_FONT :
            rFnt.SetName( ((SvxFontItem&)rItem).GetFamilyName(), SW_CJK );
            rFnt.SetStyleName( ((SvxFontItem&)rItem).GetStyleName(), SW_CJK );
            rFnt.SetFamily( ((SvxFontItem&)rItem).GetFamily(), SW_CJK );
            rFnt.SetPitch( ((SvxFontItem&)rItem).GetPitch(), SW_CJK );
            rFnt.SetCharSet( ((SvxFontItem&)rItem).GetCharSet(), SW_CJK );
            break;
        case RES_CHRATR_CJK_FONTSIZE :
            rFnt.SetSize(Size( 0, ((SvxFontHeightItem&)rItem).GetHeight()), SW_CJK);
            break;
        case RES_CHRATR_CJK_LANGUAGE :
            rFnt.SetLanguage( ((SvxLanguageItem&)rItem).GetLanguage(), SW_CJK );
            break;
        case RES_CHRATR_CJK_POSTURE :
            rFnt.SetItalic( ((SvxPostureItem&)rItem).GetPosture(), SW_CJK );
            break;
        case RES_CHRATR_CJK_WEIGHT :
            rFnt.SetWeight( ((SvxWeightItem&)rItem).GetWeight(), SW_CJK );
            break;
        case RES_CHRATR_CTL_FONT :
            rFnt.SetName( ((SvxFontItem&)rItem).GetFamilyName(), SW_CTL );
            rFnt.SetStyleName( ((SvxFontItem&)rItem).GetStyleName(), SW_CTL );
            rFnt.SetFamily( ((SvxFontItem&)rItem).GetFamily(), SW_CTL );
            rFnt.SetPitch( ((SvxFontItem&)rItem).GetPitch(), SW_CTL );
            rFnt.SetCharSet( ((SvxFontItem&)rItem).GetCharSet(), SW_CTL );
            break;
        case RES_CHRATR_CTL_FONTSIZE :
            rFnt.SetSize(Size(0, ((SvxFontHeightItem&)rItem).GetHeight() ), SW_CTL);
            break;
        case RES_CHRATR_CTL_LANGUAGE :
            rFnt.SetLanguage( ((SvxLanguageItem&)rItem).GetLanguage(), SW_CTL );
            break;
        case RES_CHRATR_CTL_POSTURE :
            rFnt.SetItalic( ((SvxPostureItem&)rItem).GetPosture(), SW_CTL );
            break;
        case RES_CHRATR_CTL_WEIGHT :
            rFnt.SetWeight( ((SvxWeightItem&)rItem).GetWeight(), SW_CTL );
            break;
        case RES_CHRATR_EMPHASIS_MARK :
            rFnt.SetEmphasisMark(
                     ((SvxEmphasisMarkItem&)rItem).GetEmphasisMark()
                     );
            break;
        case RES_CHRATR_SCALEW :
            rFnt.SetPropWidth( ((SvxCharScaleWidthItem&)rItem).GetValue() );
            break;
        case RES_CHRATR_RELIEF :
            rFnt.SetRelief( (FontRelief)((SvxCharReliefItem&)rItem).GetValue() );
            break;
        case RES_CHRATR_HIDDEN :
            if( mpShell && mpShell->GetWin())
            {
                if ( ((SvxCharHiddenItem&)rItem).GetValue() )
                    rFnt.SetUnderline( UNDERLINE_DOTTED );
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
                0 != aAttrStack[ StackPos[ RES_TXTATR_CJK_RUBY ] ].Count();

            if ( bRuby )
                break;

            sal_uInt16 nTwoLineStack = StackPos[ RES_CHRATR_TWO_LINES ];
            bool bTwoLineAct = false;
            const SwTxtAttr* pTwoLineAttr = aAttrStack[ nTwoLineStack ].Top();

            if ( pTwoLineAttr )
            {
                const SfxPoolItem* pTwoLineItem = CharFmt::GetItem( *pTwoLineAttr, RES_CHRATR_TWO_LINES );
                bTwoLineAct = ((SvxTwoLinesItem*)pTwoLineItem)->GetValue();
            }
            else
                bTwoLineAct =
                    ((SvxTwoLinesItem*)pDefaultArray[ nTwoLineStack ])->GetValue();

            if ( !bTwoLineAct )
                rFnt.SetVertical( ((SvxCharRotateItem&)rItem).GetValue(),
                                   bVertLayout );

            break;
        }
        case RES_CHRATR_TWO_LINES :
        {
            bool bRuby = 0 !=
                    aAttrStack[ StackPos[ RES_TXTATR_CJK_RUBY ] ].Count();

            // two line is activated, if
            // 1. no ruby attribute is set and
            // 2. attribute is active
            if ( !bRuby && ((SvxTwoLinesItem&)rItem).GetValue() )
            {
                rFnt.SetVertical( 0, bVertLayout );
                break;
            }

            // a deactivating two line attribute is on top of stack,
            // check if rotate attribute has to be enabled
            if ( bRuby )
                break;

            sal_uInt16 nRotateStack = StackPos[ RES_CHRATR_ROTATE ];
            const SwTxtAttr* pRotateAttr = aAttrStack[ nRotateStack ].Top();

            if ( pRotateAttr )
            {
                const SfxPoolItem* pRotateItem = CharFmt::GetItem( *pRotateAttr, RES_CHRATR_ROTATE );
                rFnt.SetVertical( ((SvxCharRotateItem*)pRotateItem)->GetValue(),
                                   bVertLayout );
            }
            else
                rFnt.SetVertical(
                    ((SvxCharRotateItem*)pDefaultArray[ nRotateStack ])->GetValue(),
                     bVertLayout
                );
            break;
        }
        case RES_TXTATR_CJK_RUBY :
            rFnt.SetVertical( 0, bVertLayout );
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

// Takes the default font and calculated the ascent and height
void SwAttrHandler::GetDefaultAscentAndHeight( SwViewShell* pShell, OutputDevice& rOut,
                                               sal_uInt16& nAscent, sal_uInt16& nHeight ) const
{
    OSL_ENSURE( pFnt, "No font available for GetDefaultAscentAndHeight" );

    if ( pFnt )
    {
        SwFont aFont( *pFnt );
        nHeight = aFont.GetHeight( pShell, rOut );
        nAscent = aFont.GetAscent( pShell, rOut );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
