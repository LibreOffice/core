/*************************************************************************
 *
 *  $RCSfile: atrstck.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: ama $ $Date: 2001-03-19 15:57:05 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifdef PRECOMPILED
#include "core_pch.hxx"
#endif

#pragma hdrstop

#ifndef _ERRHDL_HXX
#include <errhdl.hxx>   // ASSERT
#endif
#ifndef _ATRHNDL_HXX
#include <atrhndl.hxx>
#endif
#ifndef _SWFONT_HXX
#include <swfont.hxx>
#endif
#ifndef _SVX_CMAPITEM_HXX
#include <svx/cmapitem.hxx>
#endif
#ifndef _SVX_COLRITEM_HXX
#include <svx/colritem.hxx>
#endif
#ifndef _SVX_ITEM_HXX
#include <svx/cntritem.hxx>
#endif
#ifndef _SVX_CRSDITEM_HXX
#include <svx/crsditem.hxx>
#endif
#ifndef _SVX_ESCPITEM_HXX
#include <svx/escpitem.hxx>
#endif
#ifndef _SVX_FONTITEM_HXX
#include <svx/fontitem.hxx>
#endif
#ifndef _SVX_FHGTITEM_HXX
#include <svx/fhgtitem.hxx>
#endif
#ifndef _SVX_KERNITEM_HXX
#include <svx/kernitem.hxx>
#endif
#ifndef _SVX_CHARRELIEFITEM_HXX
#include <svx/charreliefitem.hxx>
#endif
#ifndef _SVX_LANGITEM_HXX
#include <svx/langitem.hxx>
#endif
#ifndef _SVX_POSTITEM_HXX
#include <svx/postitem.hxx>
#endif
#ifndef _SVX_SHDDITEM_HXX
#include <svx/shdditem.hxx>
#endif
#ifndef _SVX_UDLNITEM_HXX
#include <svx/udlnitem.hxx>
#endif
#ifndef _SVX_WGHTITEM_HXX
#include <svx/wghtitem.hxx>
#endif
#ifndef _SVX_WRLMITEM_HXX
#include <svx/wrlmitem.hxx>
#endif
#ifndef _SVX_AKRNITEM_HXX
#include <svx/akrnitem.hxx>
#endif
#ifndef _SVX_BLNKITEM_HXX
#include <svx/blnkitem.hxx>
#endif
#ifndef _SVX_CHARROTATEITEM_HXX
#include <svx/charrotateitem.hxx>
#endif
#ifndef _SVX_EMPHITEM_HXX
#include <svx/emphitem.hxx>
#endif
#ifndef _SVX_CHARSCALEITEM_HXX
#include <svx/charscaleitem.hxx>
#endif
#ifndef _SVX_TWOLINESITEM_HXX
#include <svx/twolinesitem.hxx>
#endif
#ifndef _CHARFMT_HXX
#include <charfmt.hxx>
#endif
#ifndef _FCHRFMT_HXX
#include <fchrfmt.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX
#include <svx/brshitem.hxx>
#endif
#ifndef _FMTINFMT_HXX
#include <fmtinfmt.hxx>
#endif
#ifndef _TXTINET_HXX
#include <txtinet.hxx>
#endif

#define NUM_OBJECTS (RES_TXTATR_END - RES_CHRATR_BEGIN + 1)
#define STACK_INCREMENT 4

/*************************************************************************
 *                      Attribute to Stack Mapping
 *
 * Attributes applied to a text are pushed on different stacks. For each
 * stack, the top most attribute on the stack is valid. Because some
 * kinds of attributes have to be pushed to the same stacks we map their
 * ids to stack ids
 * Attention: The first NUM_DEFAULT_VALUES ( defined in swfntcch.hxx == 34 )
 * are stored in the defaultitem-cache, if you add one, you have to increase
 * NUM_DEFAULT_VALUES.
 *************************************************************************/

USHORT StackPos[ NUM_OBJECTS ] = {
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
     0, // RES_CHRATR_DUMMY1,                    // 37
     0, // RES_TXTATR_INETFMT                    // 38
     0, // RES_TXTATR_DUMMY4,                    // 39
    34, // RES_TXTATR_REFMARK,                   // 40
    35, // RES_TXTATR_TOXMARK,                   // 41
     0, // RES_TXTATR_CHARFMT,                   // 42
     0, // RES_TXTATR_DUMMY5,                    // 43
    36, // RES_TXTATR_CJK_RUBY,                  // 44
     0, // RES_TXTATR_UNKNOWN_CONTAINER,         // 45
     0, // RES_TXTATR_DUMMY6,                    // 46
     0  // RES_TXTATR_DUMMY7,                    // 47
};

/*************************************************************************
 *                      lcl_GetItem
 * extracts pool item of type nWhich from rAttr
 *************************************************************************/

const SfxPoolItem* lcl_GetItem( const SwTxtAttr& rAttr, USHORT nWhich )
{
    if ( RES_TXTATR_INETFMT == rAttr.Which() ||
         RES_TXTATR_CHARFMT == rAttr.Which() )
    {
       SwCharFmt* pFmt;
       if( RES_TXTATR_INETFMT == rAttr.Which() )
          pFmt = ((SwTxtINetFmt&)rAttr).GetCharFmt();
       else
          pFmt = rAttr.GetCharFmt().GetCharFmt();

       if ( !pFmt )
           return 0;;

       const SfxPoolItem* pItem;
       BOOL bRet = SFX_ITEM_SET ==
             pFmt->GetItemState( RES_CHRATR_TWO_LINES, TRUE, &pItem );

       if ( bRet )
             return pItem;
       else return 0;
    }

    return ( nWhich == rAttr.Which() ) ? &rAttr.GetAttr() : 0;
}

/*************************************************************************
 *                      SwAttrHandler::SwAttrStack::SwAttrStack()
 *************************************************************************/

inline SwAttrHandler::SwAttrStack::SwAttrStack()
    : nCount( 0 ), nSize( INITIAL_NUM_ATTR )
{
    pArray = pInitialArray;
}

/*************************************************************************
 *                      SwAttrHandler::SwAttrStack::Insert()
 *************************************************************************/

void SwAttrHandler::SwAttrStack::Insert( const SwTxtAttr& rAttr, const USHORT nPos )
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

    ASSERT( nPos <= nCount, "wrong position for insert operation");

    if ( nPos < nCount )
        memmove( pArray + nPos + 1, pArray + nPos,
                ( nCount - nPos ) * sizeof(SwTxtAttr*)
                );
    pArray[ nPos ] = (SwTxtAttr*)&rAttr;

    nCount++;
}

/*************************************************************************
 *                      SwAttrHandler::SwAttrStack::Remove()
 *************************************************************************/

void SwAttrHandler::SwAttrStack::Remove( const SwTxtAttr& rAttr )
{
    USHORT nPos = Pos( rAttr );
    if ( nPos < nCount )
    {
        memmove( pArray + nPos, pArray + nPos + 1,
                ( nCount - 1 - nPos ) * sizeof(SwTxtAttr*)
                );
        nCount--;
    }
}

/*************************************************************************
 *                      SwAttrHandler::SwAttrStack::Top()
 *************************************************************************/

const SwTxtAttr* SwAttrHandler::SwAttrStack::Top() const
{
    return nCount ? pArray[ nCount - 1 ] : 0;
}

/*************************************************************************
 *                      SwAttrHandler::SwAttrStack::Pos()
 *************************************************************************/

USHORT SwAttrHandler::SwAttrStack::Pos( const SwTxtAttr& rAttr ) const
{
    if ( ! nCount )
        // empty stack
        return USHRT_MAX;

    for ( USHORT nIdx = nCount; nIdx > 0; )
    {
        if ( &rAttr == pArray[ --nIdx ] )
            return nIdx;
    }

    // element not found
    return USHRT_MAX;
}

/*************************************************************************
 *                      SwAttrHandler::SwAttrHandler::SwAttrHandler()
 *************************************************************************/

SwAttrHandler::SwAttrHandler()
{
    pAttrSet = 0;
    memset( pDefaultArray, 0, NUM_DEFAULT_VALUES * sizeof(SfxPoolItem*) );
}

/*************************************************************************
 *                      SwAttrHandler::Init()
 *************************************************************************/

void SwAttrHandler::Init( const SwAttrSet& rAttrSet )
{
    pAttrSet = &rAttrSet;
    for ( USHORT i = RES_CHRATR_BEGIN; i < RES_CHRATR_END; i++ )
        pDefaultArray[ StackPos[ i ] ] = &rAttrSet.Get( i, TRUE );
}

void SwAttrHandler::Init( const SfxPoolItem** pPoolItem, const SwAttrSet& rAS )
{
    pAttrSet = &rAS;
    memcpy( pDefaultArray, pPoolItem,
            NUM_DEFAULT_VALUES * sizeof(SfxPoolItem*) );
}

void SwAttrHandler::Reset( )
{
    for ( USHORT i = 0; i < NUM_ATTRIBUTE_STACKS; i++ )
        aAttrStack[ i ].Reset();
}

/*************************************************************************
 *                      SwAttrHandler::PushAndChg()
 *************************************************************************/

void SwAttrHandler::PushAndChg( const SwTxtAttr& rAttr, SwFont& rFnt )
{
    // these special attributes in fact represent a collection of attributes
    // they have to be pushed to each stack they belong to
    if ( RES_TXTATR_INETFMT == rAttr.Which() ||
         RES_TXTATR_CHARFMT == rAttr.Which() )
    {
        SwCharFmt* pFmt;
        if( RES_TXTATR_INETFMT == rAttr.Which() )
            pFmt = ((SwTxtINetFmt&)rAttr).GetCharFmt();
        else
            pFmt = rAttr.GetCharFmt().GetCharFmt();

        if ( !pFmt )
            return;

        for ( USHORT i = 0; i < NUM_OBJECTS; i++)
        {
            const SfxPoolItem* pItem;
            BOOL bRet = SFX_ITEM_SET ==
                 pFmt->GetItemState( i, TRUE, &pItem );
            if ( bRet )
            {
                // we push rAttr onto the appropriate stack
                if ( Push( rAttr, *pItem , rFnt ) )
                    // we let pItem change rFnt
                    FontChg( *pItem, rFnt, sal_True );
            }
        }
    }
    // this is the usual case, we have a basic attribute, push it onto the
    // stack and change the font
    else
    {
        if ( Push( rAttr, rAttr.GetAttr(), rFnt ) )
            // we let pItem change rFnt
            FontChg( rAttr.GetAttr(), rFnt, sal_True );
    }
}

/*************************************************************************
 *                      SwAttrHandler::Push()
 *************************************************************************/

sal_Bool SwAttrHandler::Push( const SwTxtAttr& rAttr, const SfxPoolItem& rItem, SwFont& rFnt )
{
    USHORT nStack = StackPos[ rItem.Which() ];

    // attributes originating from redlining have highest priority
    const SwTxtAttr* pTopAttr = aAttrStack[ nStack ].Top();
    if ( !pTopAttr || !pTopAttr->IsPriorityAttr() )
    {
        aAttrStack[ nStack ].Push( rAttr );
        return sal_True;
    }

    USHORT nPos = aAttrStack[ nStack ].Count();
    ASSERT( nPos, "empty stack?" );
    aAttrStack[ nStack ].Insert( rAttr, nPos - 1 );
    return sal_False;
}

/*************************************************************************
 *                      SwAttrHandler::PopAndChg()
 *************************************************************************/

void SwAttrHandler::PopAndChg( const SwTxtAttr& rAttr, SwFont& rFnt )
{
    // these special attributes in fact represent a collection of attributes
    // they have to be removed from each stack they belong to
    if ( RES_TXTATR_INETFMT == rAttr.Which() ||
         RES_TXTATR_CHARFMT == rAttr.Which() )
    {
        SwCharFmt* pFmt;
        if( RES_TXTATR_INETFMT == rAttr.Which() )
            pFmt = ((SwTxtINetFmt&)rAttr).GetCharFmt();
        else
            pFmt = rAttr.GetCharFmt().GetCharFmt();

        if ( !pFmt )
            return;

        for ( USHORT i = 0; i < NUM_OBJECTS; i++)
        {
            const SfxPoolItem* pItem;
            BOOL bRet = SFX_ITEM_SET ==
                 pFmt->GetItemState( i, TRUE, &pItem );
            if ( bRet )
            {
                // we remove rAttr from the appropriate stack
                USHORT nStackPos = StackPos[ i ];
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

/*************************************************************************
 *                      SwAttrHandler::Pop()
 *
 * only used during redlining
 *************************************************************************/

void SwAttrHandler::Pop( const SwTxtAttr& rAttr )
{
    aAttrStack[ StackPos[ rAttr.Which() ] ].Remove( rAttr );
}

/*************************************************************************
 *                      SwAttrHandler::ActivateTop()
 *************************************************************************/
void SwAttrHandler::ActivateTop( SwFont& rFnt, const USHORT nAttr )
{
    const USHORT nStackPos = StackPos[ nAttr ];
    const SwTxtAttr* pTopAt = aAttrStack[ nStackPos ].Top();
    if ( pTopAt )
    {
        // check if top attribute is collection of attributes
        if ( RES_TXTATR_INETFMT == pTopAt->Which() ||
             RES_TXTATR_CHARFMT == pTopAt->Which() )
        {
            SwCharFmt* pFmtNext;
            if( RES_TXTATR_INETFMT == pTopAt->Which() )
                pFmtNext = ((SwTxtINetFmt*)pTopAt)->GetCharFmt();
            else
                pFmtNext = pTopAt->GetCharFmt().GetCharFmt();

            const SfxPoolItem* pItemNext;
            pFmtNext->GetItemState( nAttr, TRUE, &pItemNext );
            FontChg( *pItemNext, rFnt, sal_False );
        }
        else
            FontChg( pTopAt->GetAttr(), rFnt, sal_False );
    }
    // default value has to be set, we only have default values for char attribs
    else if ( nStackPos < NUM_DEFAULT_VALUES )
        FontChg( *pDefaultArray[ nStackPos ], rFnt, sal_False );

    else if ( RES_TXTATR_REFMARK == nAttr )
        rFnt.GetRef()--;
    else if ( RES_TXTATR_TOXMARK == nAttr )
        rFnt.GetTox()--;
    else if ( RES_TXTATR_CJK_RUBY == nAttr )
    {
        // ruby stack has no more attributes
        // check, if an rotation attribute has to be applied
        USHORT nTwoLineStack = StackPos[ RES_CHRATR_TWO_LINES ];
        sal_Bool bTwoLineAct = sal_False;
        const SfxPoolItem* pTwoLineItem = 0;
        const SwTxtAttr* pTwoLineAttr = aAttrStack[ nTwoLineStack ].Top();

        if ( pTwoLineAttr )
        {
             pTwoLineItem = lcl_GetItem( *pTwoLineAttr, RES_CHRATR_TWO_LINES );
             bTwoLineAct = ((SvxTwoLinesItem*)pTwoLineItem)->GetValue();
        }
        else
            bTwoLineAct =
                ((SvxTwoLinesItem*)pDefaultArray[ nTwoLineStack ])->GetValue();

        if ( bTwoLineAct )
            return;

        // eventually, an rotate attribute has to be activated
        USHORT nRotateStack = StackPos[ RES_CHRATR_ROTATE ];
        const SfxPoolItem* pRotateItem = 0;
        const SwTxtAttr* pRotateAttr = aAttrStack[ nRotateStack ].Top();

        if ( pRotateAttr )
        {
            pRotateItem = lcl_GetItem( *pRotateAttr, RES_CHRATR_ROTATE );
            rFnt.SetVertical( ((SvxCharRotateItem*)pRotateItem)->GetValue() );
        }
        else
            rFnt.SetVertical(
                ((SvxCharRotateItem*)pDefaultArray[ nRotateStack ])->GetValue()
            );
    }
}

/**************************************************************************
 *                      SwAttrHandler::GetDefault()
 *************************************************************************/

const SfxPoolItem& SwAttrHandler::GetDefault( const USHORT nAttribID ) const
{
    ASSERT( 0 <= nAttribID && nAttribID < RES_TXTATR_END,
            "this attrib does not ex."
            );
    ASSERT( pDefaultArray[ StackPos[ nAttribID ] ], "array not initialized" );
    return *pDefaultArray[ StackPos[ nAttribID ] ];
}

/*************************************************************************
 *                      SwAttrHandler::ChangeScript()
 * when changing swfont so that is only uses one internal font, the
 * following code can be useful
 *************************************************************************/

//void SwAttrHandler::ChangeScript( SwFont& rFnt, const BYTE nScr )
//{
//    USHORT i;

//    switch ( nScr ) {
//        case SW_LATIN :
//            ActivateTop( rFnt, RES_CHRATR_FONT );
//            ActivateTop( rFnt, RES_CHRATR_FONTSIZE );
//            ActivateTop( rFnt, RES_CHRATR_LANGUAGE );
//            ActivateTop( rFnt, RES_CHRATR_POSTURE );
//            ActivateTop( rFnt, RES_CHRATR_WEIGHT );
//            break;
//        case SW_CJK :
//            for ( i = RES_CHRATR_CJK_FONT; i <= RES_CHRATR_CJK_WEIGHT; i++ )
//                ActivateTop( rFnt, i );
//            break;
//        case SW_CTL :
//            for ( i = RES_CHRATR_CTL_FONT; i <= RES_CHRATR_CTL_WEIGHT; i++ )
//                ActivateTop( rFnt, i );
//            break;
//    }
//}

/*************************************************************************
 *                      Font Changing Function
 *
 * When popping an attribute from the stack, the top mose remaining
 * attribute in the stack becomes valid. The following function change
 * a font depending on the stack id.
 *************************************************************************/

void SwAttrHandler::FontChg(const SfxPoolItem& rItem, SwFont& rFnt, sal_Bool bPush )
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
            rFnt.SetUnderline( ((SvxUnderlineItem&)rItem).GetUnderline() );
            rFnt.SetUnderColor( ((SvxUnderlineItem&)rItem).GetColor() );
            break;
        case RES_CHRATR_WEIGHT :
            rFnt.SetWeight( ((SvxWeightItem&)rItem).GetWeight(), SW_LATIN );
            break;
        case RES_CHRATR_WORDLINEMODE :
            rFnt.SetWordLineMode( ((SvxWordLineModeItem&)rItem).GetValue() );
            break;
        case RES_CHRATR_AUTOKERN :
            rFnt.SetAutoKern( ((SvxAutoKernItem&)rItem).GetValue() );
            break;
        case RES_CHRATR_BLINK :
            rFnt.SetBlink( ((SvxBlinkItem&)rItem).GetValue() );
            break;
        case RES_CHRATR_BACKGROUND :
            rFnt.SetBackColor(new Color( ((SvxBrushItem&)rItem).GetColor() ) );
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

        case RES_CHRATR_ROTATE :
        {
            // rotate attribute is applied, when:
            // 1. ruby stack is empty and
            // 2. top of two line stack ( or default attribute )is an
            //    deactivated two line attribute
            sal_Bool bRuby = 0 !=
                             aAttrStack[ StackPos[ RES_TXTATR_CJK_RUBY ] ].Count();

            if ( bRuby )
                break;

            USHORT nTwoLineStack = StackPos[ RES_CHRATR_TWO_LINES ];
            sal_Bool bTwoLineAct = sal_False;
            const SfxPoolItem* pTwoLineItem = 0;
            const SwTxtAttr* pTwoLineAttr = aAttrStack[ nTwoLineStack ].Top();

            if ( pTwoLineAttr )
            {
                pTwoLineItem = lcl_GetItem( *pTwoLineAttr, RES_CHRATR_TWO_LINES );
                bTwoLineAct = ((SvxTwoLinesItem*)pTwoLineItem)->GetValue();
            }
            else
                bTwoLineAct =
                    ((SvxTwoLinesItem*)pDefaultArray[ nTwoLineStack ])->GetValue();

            if ( !bTwoLineAct )
                rFnt.SetVertical( ((SvxCharRotateItem&)rItem).GetValue() );

            break;
        }
        case RES_CHRATR_TWO_LINES :
        {
            sal_Bool bRuby = 0 !=
                    aAttrStack[ StackPos[ RES_TXTATR_CJK_RUBY ] ].Count();
            sal_Bool bTwoLineAct = sal_False;

            // two line is activated, if
            // 1. no ruby attribute is set and
            // 2. attribute is active
            bTwoLineAct = ((SvxTwoLinesItem&)rItem).GetValue();

            if ( !bRuby && bTwoLineAct )
            {
                rFnt.SetVertical( 0 );
                break;
            }

            // a deactivating two line attribute is on top of stack,
            // check if rotate attribute has to be enabled
            if ( bRuby )
                break;

            USHORT nRotateStack = StackPos[ RES_CHRATR_ROTATE ];
            const SfxPoolItem* pRotateItem = 0;
            const SwTxtAttr* pRotateAttr = aAttrStack[ nRotateStack ].Top();

            if ( pRotateAttr )
            {
                pRotateItem = lcl_GetItem( *pRotateAttr, RES_CHRATR_ROTATE );
                rFnt.SetVertical( ((SvxCharRotateItem*)pRotateItem)->GetValue() );
            }
            else
                rFnt.SetVertical(
                    ((SvxCharRotateItem*)pDefaultArray[ nRotateStack ])->GetValue()
                );

            break;
        }
        case RES_TXTATR_CJK_RUBY :
            rFnt.SetVertical( 0 );
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
    }
}

