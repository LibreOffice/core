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

#include <hintids.hxx>
#include <svl/itemiter.hxx>
#include <svl/whiter.hxx>
#include <svl/urihelper.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <sfx2/docfile.hxx>
#include <vcl/svapp.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/flstitem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <svtools/htmltokn.h>
#include <svtools/htmlkywd.hxx>
#include <fmtpdsc.hxx>
#include <fmtanchr.hxx>
#include <fmtornt.hxx>
#include <fmtsrnd.hxx>
#include <fmtfsize.hxx>
#include <frmatr.hxx>
#include <charfmt.hxx>
#include <docary.hxx>
#include <svx/svxids.hrc>
#include <osl/diagnose.h>

#include <doc.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <poolfmt.hxx>
#include <docsh.hxx>
#include <paratr.hxx>
#include <pagedesc.hxx>
#include "css1kywd.hxx"
#include "swcss1.hxx"
#include "htmlnum.hxx"
#include "swhtml.hxx"
#include <numrule.hxx>
#include "css1atr.hxx"

using namespace ::com::sun::star;

// How many rows/characters are allowed for DropCaps?
// (Are there maybe somewhere else corresponding values?)
#define MAX_DROPCAP_LINES 9
#define MAX_DROPCAP_CHARS 9

static void lcl_swcss1_setEncoding( SwFormat& rFormat, rtl_TextEncoding eEnc );

// Implementation of SwCSS1Parsers (actually swcss1.cxx)
static const sal_uInt16 aItemIds[] =
{
    RES_BREAK,
    RES_PAGEDESC,
    RES_KEEP,
};

void SwCSS1Parser::ChgPageDesc( const SwPageDesc *pPageDesc,
                                const SwPageDesc& rNewPageDesc )
{
    size_t pos;
    bool found = m_pDoc->ContainsPageDesc( pPageDesc, &pos );
    OSL_ENSURE( found, "style not found" );
    if (found)
        m_pDoc->ChgPageDesc( pos, rNewPageDesc );
}

SwCSS1Parser::SwCSS1Parser( SwDoc *pD, const sal_uInt32 aFHeights[7], const OUString& rBaseURL, bool bNewDoc ) :
    SvxCSS1Parser( pD->GetAttrPool(), rBaseURL,
                   aItemIds, SAL_N_ELEMENTS(aItemIds)),
    m_pDoc( pD ),
    m_nDropCapCnt( 0 ),
    m_bIsNewDoc( bNewDoc ),
    m_bBodyBGColorSet( false ),
    m_bBodyBackgroundSet( false ),
    m_bBodyTextSet( false ),
    m_bBodyLinkSet( false ),
    m_bBodyVLinkSet( false ),
    m_bSetFirstPageDesc( false ),
    m_bSetRightPageDesc( false ),
    m_bTableHeaderTextCollSet( false ),
    m_bTableTextCollSet( false ),
    m_bLinkCharFormatsSet( false )
{
    m_aFontHeights[0] = aFHeights[0];
    m_aFontHeights[1] = aFHeights[1];
    m_aFontHeights[2] = aFHeights[2];
    m_aFontHeights[3] = aFHeights[3];
    m_aFontHeights[4] = aFHeights[4];
    m_aFontHeights[5] = aFHeights[5];
    m_aFontHeights[6] = aFHeights[6];
}

SwCSS1Parser::~SwCSS1Parser()
{
}

// Feature: PrintExt
bool SwCSS1Parser::SetFormatBreak( SfxItemSet& rItemSet,
                                const SvxCSS1PropertyInfo& rPropInfo )
{
    SvxBreak eBreak = SvxBreak::NONE;
    bool bKeep = false;
    bool bSetKeep = false, bSetBreak = false, bSetPageDesc = false;
    const SwPageDesc *pPageDesc = nullptr;
    switch( rPropInfo.m_ePageBreakBefore )
    {
    case SVX_CSS1_PBREAK_ALWAYS:
        eBreak = SvxBreak::PageBefore;
        bSetBreak = true;
        break;
    case SVX_CSS1_PBREAK_LEFT:
        pPageDesc = GetLeftPageDesc( true );
        bSetPageDesc = true;
        break;
    case SVX_CSS1_PBREAK_RIGHT:
        pPageDesc = GetRightPageDesc( true );
        bSetPageDesc = true;
        break;
    case SVX_CSS1_PBREAK_AUTO:
        bSetBreak = bSetPageDesc = true;
        break;
    default:
        ;
    }
    switch( rPropInfo.m_ePageBreakAfter )
    {
    case SVX_CSS1_PBREAK_ALWAYS:
    case SVX_CSS1_PBREAK_LEFT:
    case SVX_CSS1_PBREAK_RIGHT:
        // LEFT/RIGHT also could be set in the previous paragraph
        eBreak = SvxBreak::PageAfter;
        bSetBreak = true;
        break;
    case SVX_CSS1_PBREAK_AUTO:
        bSetBreak = bSetKeep = bSetPageDesc = true;
        break;
    case SVX_CSS1_PBREAK_AVOID:
        bKeep = bSetKeep = true;
        break;
    default:
        ;
    }

    if( bSetBreak )
        rItemSet.Put( SvxFormatBreakItem( eBreak, RES_BREAK ) );
    if( bSetPageDesc )
        rItemSet.Put( SwFormatPageDesc( pPageDesc ) );
    if( bSetKeep )
        rItemSet.Put( SvxFormatKeepItem( bKeep, RES_KEEP ) );

    return bSetBreak;
}

static void SetCharFormatAttrs( SwCharFormat *pCharFormat, SfxItemSet& rItemSet )
{
    const SfxPoolItem *pItem;
    static const sal_uInt16 aWhichIds[3] = { RES_CHRATR_FONTSIZE,RES_CHRATR_CJK_FONTSIZE,
                                   RES_CHRATR_CTL_FONTSIZE };
    for(sal_uInt16 i : aWhichIds)
    {
        if( SfxItemState::SET == rItemSet.GetItemState( i, false,
                                                   &pItem ) &&
            static_cast<const SvxFontHeightItem *>(pItem)->GetProp() != 100)
        {
            // percentage values at the FontHeight item aren't supported
            rItemSet.ClearItem( i );
        }
    }

    pCharFormat->SetFormatAttr( rItemSet );

    if( SfxItemState::SET == rItemSet.GetItemState( RES_BACKGROUND, false, &pItem ) )
    {
        // A Brush-Item with RES_BACKGROUND must be converted to one
        // with RES_CHRATR_BACKGROUND

        SvxBrushItem aBrushItem( *static_cast<const SvxBrushItem *>(pItem) );
        aBrushItem.SetWhich( RES_CHRATR_BACKGROUND );
        pCharFormat->SetFormatAttr( aBrushItem );
    }

    if( SfxItemState::SET == rItemSet.GetItemState( RES_BOX, false, &pItem ) )
    {
        SvxBoxItem aBoxItem( *static_cast<const SvxBoxItem *>(pItem) );
        aBoxItem.SetWhich( RES_CHRATR_BOX );
        pCharFormat->SetFormatAttr( aBoxItem );
    }
}

void SwCSS1Parser::SetLinkCharFormats()
{
    OSL_ENSURE( !m_bLinkCharFormatsSet, "Call SetLinkCharFormats unnecessary" );

    SvxCSS1MapEntry *pStyleEntry =
        GetTag( OOO_STRING_SVTOOLS_HTML_anchor );
    SwCharFormat *pUnvisited = nullptr, *pVisited = nullptr;
    if( pStyleEntry )
    {
        SfxItemSet& rItemSet = pStyleEntry->GetItemSet();
        bool bColorSet = (SfxItemState::SET==rItemSet.GetItemState(RES_CHRATR_COLOR,
                                                              false));
        pUnvisited = GetCharFormatFromPool( RES_POOLCHR_INET_NORMAL );
        SetCharFormatAttrs( pUnvisited, rItemSet );
        m_bBodyLinkSet |= bColorSet;

        pVisited = GetCharFormatFromPool( RES_POOLCHR_INET_VISIT );
        SetCharFormatAttrs( pVisited, rItemSet );
        m_bBodyVLinkSet |= bColorSet;
    }

    OUString sTmp = OOO_STRING_SVTOOLS_HTML_anchor ":link";

    pStyleEntry = GetTag( sTmp );
    if( pStyleEntry )
    {
        SfxItemSet& rItemSet = pStyleEntry->GetItemSet();
        bool bColorSet = (SfxItemState::SET==rItemSet.GetItemState(RES_CHRATR_COLOR,
                                                              false));
        if( !pUnvisited )
            pUnvisited = GetCharFormatFromPool( RES_POOLCHR_INET_NORMAL );
        SetCharFormatAttrs( pUnvisited, rItemSet );
        m_bBodyLinkSet |= bColorSet;
    }

    sTmp = OOO_STRING_SVTOOLS_HTML_anchor ":visited";

    pStyleEntry = GetTag( sTmp );
    if( pStyleEntry )
    {
        SfxItemSet& rItemSet = pStyleEntry->GetItemSet();
        bool bColorSet = (SfxItemState::SET==rItemSet.GetItemState(RES_CHRATR_COLOR,
                                                              false));
        if( !pVisited )
            pVisited = GetCharFormatFromPool( RES_POOLCHR_INET_VISIT );
        SetCharFormatAttrs( pVisited, rItemSet );
        m_bBodyVLinkSet |= bColorSet;
    }

    m_bLinkCharFormatsSet = true;
}

static void SetTextCollAttrs( SwTextFormatColl *pColl, SfxItemSet& rItemSet,
                             SvxCSS1PropertyInfo const & rPropInfo,
                             SwCSS1Parser *pCSS1Parser )
{
    const SfxItemSet& rCollItemSet = pColl->GetAttrSet();
    const SfxPoolItem *pCollItem, *pItem;

    // left, right border and first line indentation
    if( (rPropInfo.m_bLeftMargin || rPropInfo.m_bRightMargin ||
         rPropInfo.m_bTextIndent) &&
        (!rPropInfo.m_bLeftMargin || !rPropInfo.m_bRightMargin ||
         !rPropInfo.m_bTextIndent) &&
        SfxItemState::SET == rCollItemSet.GetItemState(RES_LR_SPACE,true,&pCollItem) &&
        SfxItemState::SET == rItemSet.GetItemState(RES_LR_SPACE,false,&pItem) )
    {
        const SvxLRSpaceItem *pLRItem = static_cast<const SvxLRSpaceItem *>(pItem);

        SvxLRSpaceItem aLRItem( *static_cast<const SvxLRSpaceItem *>(pCollItem) );
        if( rPropInfo.m_bLeftMargin )
            aLRItem.SetTextLeft( pLRItem->GetTextLeft() );
        if( rPropInfo.m_bRightMargin )
            aLRItem.SetRight( pLRItem->GetRight() );
        if( rPropInfo.m_bTextIndent )
            aLRItem.SetTextFirstLineOfst( pLRItem->GetTextFirstLineOfst() );

        rItemSet.Put( aLRItem );
    }

    // top and bottom border
    if( (rPropInfo.m_bTopMargin || rPropInfo.m_bBottomMargin) &&
        (!rPropInfo.m_bTopMargin || !rPropInfo.m_bBottomMargin) &&
        SfxItemState::SET == rCollItemSet.GetItemState(RES_UL_SPACE,true,
                                                  &pCollItem) &&
        SfxItemState::SET == rItemSet.GetItemState(RES_UL_SPACE,false,&pItem) )
    {
        const SvxULSpaceItem *pULItem = static_cast<const SvxULSpaceItem *>(pItem);

        SvxULSpaceItem aULItem( *static_cast<const SvxULSpaceItem *>(pCollItem) );
        if( rPropInfo.m_bTopMargin )
            aULItem.SetUpper( pULItem->GetUpper() );
        if( rPropInfo.m_bBottomMargin )
            aULItem.SetLower( pULItem->GetLower() );

        rItemSet.Put( aULItem );
    }

    static const sal_uInt16 aWhichIds[3] = { RES_CHRATR_FONTSIZE,RES_CHRATR_CJK_FONTSIZE,
                                   RES_CHRATR_CTL_FONTSIZE };
    for(sal_uInt16 i : aWhichIds)
    {
        if( SfxItemState::SET == rItemSet.GetItemState( i, false,
                                                   &pItem ) &&
            static_cast<const SvxFontHeightItem *>(pItem)->GetProp() != 100)
        {
            // percentage values at the FontHeight item aren't supported
            rItemSet.ClearItem( i );
        }
    }

    pCSS1Parser->SetFormatBreak( rItemSet, rPropInfo );

    pColl->SetFormatAttr( rItemSet );
}

void SwCSS1Parser::SetTableTextColl( bool bHeader )
{
    OSL_ENSURE( !(bHeader ? m_bTableHeaderTextCollSet : m_bTableTextCollSet),
            "Call SetTableTextColl unnecessary" );

    sal_uInt16 nPoolId;
    OUString sTag;
    if( bHeader )
    {
        nPoolId = RES_POOLCOLL_TABLE_HDLN;
        sTag = OOO_STRING_SVTOOLS_HTML_tableheader;
    }
    else
    {
        nPoolId = RES_POOLCOLL_TABLE;
        sTag = OOO_STRING_SVTOOLS_HTML_tabledata;
    }

    SwTextFormatColl *pColl = nullptr;

    // The following entries will never be used again and may be changed.
    SvxCSS1MapEntry *pStyleEntry = GetTag( sTag );
    if( pStyleEntry )
    {
        pColl = GetTextFormatColl(nPoolId, OUString());
        SetTextCollAttrs( pColl, pStyleEntry->GetItemSet(),
                         pStyleEntry->GetPropertyInfo(), this );
    }

    OUString sTmp = sTag + " " OOO_STRING_SVTOOLS_HTML_parabreak;
    pStyleEntry = GetTag( sTmp );
    if( pStyleEntry )
    {
        if( !pColl )
            pColl = GetTextFormatColl(nPoolId, OUString());
        SetTextCollAttrs( pColl, pStyleEntry->GetItemSet(),
                         pStyleEntry->GetPropertyInfo(), this );
    }

    if( bHeader )
        m_bTableHeaderTextCollSet = true;
    else
        m_bTableTextCollSet = true;
}

void SwCSS1Parser::SetPageDescAttrs( const SvxBrushItem *pBrush,
                                     SfxItemSet *pItemSet2 )
{
    SvxBrushItem aBrushItem( RES_BACKGROUND );
    SvxBoxItem aBoxItem( RES_BOX );
    SvxFrameDirectionItem aFrameDirItem(SvxFrameDirection::Environment, RES_FRAMEDIR);
    bool bSetBrush = pBrush!=nullptr, bSetBox = false, bSetFrameDir = false;
    if( pBrush )
        aBrushItem = *pBrush;

    if( pItemSet2 )
    {
        const SfxPoolItem *pItem = nullptr;
        if( SfxItemState::SET == pItemSet2->GetItemState( RES_BACKGROUND, false,
                                                   &pItem ) )
        {
            // set a background
            aBrushItem = *static_cast<const SvxBrushItem *>(pItem);
            pItemSet2->ClearItem( RES_BACKGROUND );
            bSetBrush = true;
        }

        if( SfxItemState::SET == pItemSet2->GetItemState( RES_BOX, false, &pItem ) )
        {
            // set a border
            aBoxItem = *static_cast<const SvxBoxItem *>(pItem);
            pItemSet2->ClearItem( RES_BOX );
            bSetBox = true;
        }

        if( SfxItemState::SET == pItemSet2->GetItemState( RES_FRAMEDIR, false, &pItem ) )
        {
            // set a frame
            aFrameDirItem = *static_cast< const SvxFrameDirectionItem *>( pItem );
            pItemSet2->ClearItem( RES_FRAMEDIR );
            bSetFrameDir = true;
        }
    }

    if( bSetBrush || bSetBox || bSetFrameDir )
    {
        static sal_uInt16 aPoolIds[] = { RES_POOLPAGE_HTML, RES_POOLPAGE_FIRST,
                                     RES_POOLPAGE_LEFT, RES_POOLPAGE_RIGHT };
        for(sal_uInt16 i : aPoolIds)
        {
            const SwPageDesc *pPageDesc = GetPageDesc( i, false );
            if( pPageDesc )
            {
                SwPageDesc aNewPageDesc( *pPageDesc );
                SwFrameFormat &rMaster = aNewPageDesc.GetMaster();
                if( bSetBrush )
                    rMaster.SetFormatAttr( aBrushItem );
                if( bSetBox )
                    rMaster.SetFormatAttr( aBoxItem );
                if( bSetFrameDir )
                    rMaster.SetFormatAttr( aFrameDirItem );

                ChgPageDesc( pPageDesc, aNewPageDesc );
            }
        }
    }
}

// Feature: PrintExt
void SwCSS1Parser::SetPageDescAttrs( const SwPageDesc *pPageDesc,
                                     SfxItemSet& rItemSet,
                                     const SvxCSS1PropertyInfo& rPropInfo )
{
    if( !pPageDesc )
        return;

    SwPageDesc aNewPageDesc( *pPageDesc );
    SwFrameFormat &rMaster = aNewPageDesc.GetMaster();
    const SfxItemSet& rPageItemSet = rMaster.GetAttrSet();
    const SfxPoolItem *pPageItem, *pItem;
    bool bChanged = false;

    // left, right border and first line indentation
    if( (rPropInfo.m_bLeftMargin || rPropInfo.m_bRightMargin) &&
        SfxItemState::SET == rItemSet.GetItemState(RES_LR_SPACE,false,&pItem) )
    {
        if( (!rPropInfo.m_bLeftMargin || !rPropInfo.m_bRightMargin) &&
            SfxItemState::SET == rPageItemSet.GetItemState(RES_LR_SPACE,
                                                      true,&pPageItem) )
        {
            const SvxLRSpaceItem *pLRItem = static_cast<const SvxLRSpaceItem *>(pItem);

            SvxLRSpaceItem aLRItem( *static_cast<const SvxLRSpaceItem *>(pPageItem) );
            if( rPropInfo.m_bLeftMargin )
                aLRItem.SetLeft( pLRItem->GetLeft() );
            if( rPropInfo.m_bRightMargin )
                aLRItem.SetRight( pLRItem->GetRight() );

            rMaster.SetFormatAttr( aLRItem );
        }
        else
        {
            rMaster.SetFormatAttr( *pItem );
        }
        bChanged = true;
    }

    // top and bottom border
    if( (rPropInfo.m_bTopMargin || rPropInfo.m_bBottomMargin) &&
        SfxItemState::SET == rItemSet.GetItemState(RES_UL_SPACE,false,&pItem) )
    {
        if( (!rPropInfo.m_bTopMargin || !rPropInfo.m_bBottomMargin) &&
            SfxItemState::SET == rPageItemSet.GetItemState(RES_UL_SPACE,
                                                      true,&pPageItem) )
        {
            const SvxULSpaceItem *pULItem = static_cast<const SvxULSpaceItem *>(pItem);

            SvxULSpaceItem aULItem( *static_cast<const SvxULSpaceItem *>(pPageItem) );
            if( rPropInfo.m_bTopMargin )
                aULItem.SetUpper( pULItem->GetUpper() );
            if( rPropInfo.m_bBottomMargin )
                aULItem.SetLower( pULItem->GetLower() );

            rMaster.SetFormatAttr( aULItem );
        }
        else
        {
            rMaster.SetFormatAttr( *pItem );
        }
        bChanged = true;
    }

    // the size
    if( rPropInfo.m_eSizeType != SVX_CSS1_STYPE_NONE )
    {
        if( rPropInfo.m_eSizeType == SVX_CSS1_STYPE_TWIP )
        {
            rMaster.SetFormatAttr( SwFormatFrameSize( ATT_FIX_SIZE, rPropInfo.m_nWidth,
                                           rPropInfo.m_nHeight ) );
            bChanged = true;
        }
        else
        {
            // With "size: auto|portrait|landscape" the current size
            // of the style remains. If "landscape" and "portrait" then
            // the landscape flag will be set and maybe the width/height
            // are swapped.
            SwFormatFrameSize aFrameSz( rMaster.GetFrameSize() );
            bool bLandscape = aNewPageDesc.GetLandscape();
            if( ( bLandscape &&
                  rPropInfo.m_eSizeType == SVX_CSS1_STYPE_PORTRAIT ) ||
                ( !bLandscape &&
                  rPropInfo.m_eSizeType == SVX_CSS1_STYPE_LANDSCAPE ) )
            {
                SwTwips nTmp = aFrameSz.GetHeight();
                aFrameSz.SetHeight( aFrameSz.GetWidth() );
                aFrameSz.SetWidth( nTmp );
                rMaster.SetFormatAttr( aFrameSz );
                aNewPageDesc.SetLandscape( !bLandscape );
                bChanged = true;
            }
        }
    }

    // Is that possible?
    if( SfxItemState::SET == rItemSet.GetItemState( RES_BACKGROUND, false, &pItem ) )
    {
        // set a background
        rMaster.SetFormatAttr( *pItem );
        rItemSet.ClearItem( RES_BACKGROUND );
        bChanged = true;
    }

    if( bChanged )
        ChgPageDesc( pPageDesc, aNewPageDesc );
}

SvxBrushItem SwCSS1Parser::makePageDescBackground() const
{
    return m_pDoc->getIDocumentStylePoolAccess().GetPageDescFromPool( RES_POOLPAGE_HTML, false )
        ->GetMaster().makeBackgroundBrushItem();
}

Css1ScriptFlags SwCSS1Parser::GetScriptFromClass( OUString& rClass,
                                             bool bSubClassOnly )
{
    Css1ScriptFlags nScriptFlags = Css1ScriptFlags::AllMask;
    sal_Int32 nLen = rClass.getLength();
    sal_Int32 nPos = nLen > 4 ? rClass.lastIndexOf( '-' ) : -1;

    if( nPos == -1 )
    {
        if( bSubClassOnly )
            return nScriptFlags;
        nPos = 0;
    }
    else
    {
        nPos++;
        nLen = nLen - nPos;
    }

    switch( nLen )
    {
    case 3:
        if( rClass.matchIgnoreAsciiCase( "cjk", nPos ) )
        {
            nScriptFlags = Css1ScriptFlags::CJK;
        }
        else if( rClass.matchIgnoreAsciiCase( "ctl", nPos ) )
        {
            nScriptFlags = Css1ScriptFlags::CTL;
        }
        break;
    case 7:
        if( rClass.matchIgnoreAsciiCase( "western", nPos ) )
        {
            nScriptFlags = Css1ScriptFlags::Western;
        }
        break;
    }
    if( Css1ScriptFlags::AllMask != nScriptFlags )
    {
        if( nPos )
        {
            rClass = rClass.copy( 0, nPos-1 );
        }
        else
        {
            rClass.clear();
        }
    }

    return nScriptFlags;
}

static CSS1SelectorType GetTokenAndClass( const CSS1Selector *pSelector,
                              OUString& rToken, OUString& rClass,
                              Css1ScriptFlags& rScriptFlags )
{
    rToken = pSelector->GetString();
    rClass.clear();
    rScriptFlags = Css1ScriptFlags::AllMask;

    CSS1SelectorType eType = pSelector->GetType();
    if( CSS1_SELTYPE_ELEM_CLASS==eType  )
    {
        sal_Int32 nPos = rToken.indexOf( '.' );
        OSL_ENSURE( nPos >= 0, "No dot in Class-Selector???" );
        if( nPos >= 0 )
        {
            rClass = rToken.copy( nPos+1 );
            rToken = rToken.copy( 0, nPos );

            rScriptFlags = SwCSS1Parser::GetScriptFromClass( rClass, false );
            if( rClass.isEmpty() )
                eType = CSS1_SELTYPE_ELEMENT;
        }
    }

    rToken = rToken.toAsciiLowerCase();
    return eType;
}

static void RemoveScriptItems( SfxItemSet& rItemSet, Css1ScriptFlags nScript,
                               const SfxItemSet *pParentItemSet = nullptr )
{
    static const sal_uInt16 aWhichIds[3][5] =
    {
        { RES_CHRATR_FONT, RES_CHRATR_FONTSIZE, RES_CHRATR_LANGUAGE,
            RES_CHRATR_POSTURE, RES_CHRATR_WEIGHT },
        { RES_CHRATR_CJK_FONT, RES_CHRATR_CJK_FONTSIZE, RES_CHRATR_CJK_LANGUAGE,
            RES_CHRATR_CJK_POSTURE, RES_CHRATR_CJK_WEIGHT },
        { RES_CHRATR_CTL_FONT, RES_CHRATR_CTL_FONTSIZE, RES_CHRATR_CTL_LANGUAGE,
            RES_CHRATR_CTL_POSTURE, RES_CHRATR_CTL_WEIGHT }
    };

    bool aClearItems[3] = { false, false, false };
    switch( nScript )
    {
    case Css1ScriptFlags::Western:
        aClearItems[1] = aClearItems[2] =  true;
        break;
    case Css1ScriptFlags::CJK:
        aClearItems[0] = aClearItems[2] =  true;
        break;
    case Css1ScriptFlags::CTL:
        aClearItems[0] = aClearItems[1] =  true;
        break;
    case Css1ScriptFlags::AllMask:
        break;
    default:
        OSL_ENSURE( aClearItems[0], "unknown script type" );
        break;
       }

    for( size_t j=0; j < SAL_N_ELEMENTS(aWhichIds); ++j )
    {
        for( size_t i=0; i < SAL_N_ELEMENTS(aWhichIds[0]); ++i )
        {
            sal_uInt16 nWhich = aWhichIds[j][i];
            const SfxPoolItem *pItem;
            if( aClearItems[j] ||
                (pParentItemSet &&
                 SfxItemState::SET == rItemSet.GetItemState( nWhich, false, &pItem ) &&
                 (0==i ? swhtml_css1atr_equalFontItems( *pItem, pParentItemSet->Get(nWhich ) )
                        : *pItem == pParentItemSet->Get(nWhich ) ) ) )
            {
                rItemSet.ClearItem( nWhich );
            }
        }
    }
}

void SwCSS1Parser::StyleParsed( const CSS1Selector *pSelector,
                                SfxItemSet& rItemSet,
                                SvxCSS1PropertyInfo& rPropInfo )
{
    if( !m_bIsNewDoc )
        return;

    CSS1SelectorType eSelType = pSelector->GetType();
    const CSS1Selector *pNext = pSelector->GetNext();

    if( CSS1_SELTYPE_ID==eSelType && !pNext )
    {
        InsertId( pSelector->GetString(), rItemSet, rPropInfo );
    }
    else if( CSS1_SELTYPE_CLASS==eSelType && !pNext )
    {
        OUString aClass( pSelector->GetString() );
        Css1ScriptFlags nScript = GetScriptFromClass( aClass );
        if( Css1ScriptFlags::AllMask != nScript )
        {
            SfxItemSet aScriptItemSet( rItemSet );
            RemoveScriptItems( aScriptItemSet, nScript );
            InsertClass( aClass, aScriptItemSet, rPropInfo );
        }
        else
        {
            InsertClass( aClass, rItemSet, rPropInfo );
        }
    }
    else if( CSS1_SELTYPE_PAGE==eSelType )
    {
        if( !pNext ||
            (CSS1_SELTYPE_PSEUDO == pNext->GetType() &&
            (pNext->GetString().equalsIgnoreAsciiCase( "left" ) ||
             pNext->GetString().equalsIgnoreAsciiCase( "right" ) ||
             pNext->GetString().equalsIgnoreAsciiCase( "first" ) ) ) )
        {
            OUString aName;
            if( pNext )
                aName = pNext->GetString();
            InsertPage( aName,
                        pNext != nullptr,
                        rItemSet, rPropInfo );
        }
    }

    if( CSS1_SELTYPE_ELEMENT != eSelType &&
        CSS1_SELTYPE_ELEM_CLASS != eSelType)
        return;

    // get token and class of selector
    OUString aToken2;
    OUString aClass;
    Css1ScriptFlags nScript;
    eSelType = GetTokenAndClass( pSelector, aToken2, aClass, nScript );
    HtmlTokenId nToken2 = GetHTMLToken( aToken2 );

    // and also some information of the next element
    CSS1SelectorType eNextType = pNext ? pNext->GetType()
                                       : CSS1_SELTYPE_ELEMENT;

    // first some special cases
    if( CSS1_SELTYPE_ELEMENT==eSelType )
    {
        switch( nToken2 )
        {
        case HtmlTokenId::ANCHOR_ON:
            if( !pNext )
            {
                InsertTag( aToken2, rItemSet, rPropInfo );
                return;
            }
            else if (CSS1_SELTYPE_PSEUDO == eNextType)
            {
                // maybe A:visited or A:link

                OUString aPseudo( pNext->GetString() );
                aPseudo = aPseudo.toAsciiLowerCase();
                bool bInsert = false;
                switch( aPseudo[0] )
                {
                    case 'l':
                        if( aPseudo == "link" )
                        {
                            bInsert = true;
                        }
                        break;
                    case 'v':
                        if( aPseudo == "visited" )
                        {
                            bInsert = true;
                        }
                        break;
                }
                if( bInsert )
                {
                    OUString sTmp = aToken2 + ":" + aPseudo;
                    if( Css1ScriptFlags::AllMask != nScript )
                    {
                        SfxItemSet aScriptItemSet( rItemSet );
                        RemoveScriptItems( aScriptItemSet, nScript );
                        InsertTag( sTmp, aScriptItemSet, rPropInfo );
                    }
                    else
                    {
                        InsertTag( sTmp, rItemSet, rPropInfo );
                    }
                    return;
                }
            }
            break;
        case HtmlTokenId::BODY_ON:
            if( !pNext )
            {
                // BODY

                // We must test the background before setting, because
                // in SetPageDescAttrs it will be deleted.
                const SfxPoolItem *pItem;
                if( SfxItemState::SET==rItemSet.GetItemState(RES_BACKGROUND,false,&pItem) )
                {
                    const SvxBrushItem *pBrushItem =
                        static_cast<const SvxBrushItem *>(pItem);

                    /// Body has a background color, if it is not "no fill"/"auto fill"
                    if( pBrushItem->GetColor() != COL_TRANSPARENT )
                        m_bBodyBGColorSet = true;
                    if( GPOS_NONE != pBrushItem->GetGraphicPos() )
                        m_bBodyBackgroundSet = true;
                }

                // Border and Padding
                rPropInfo.SetBoxItem( rItemSet, MIN_BORDER_DIST );

                // Some attributes must be set at the style, the ones which
                // aren't inherited
                SetPageDescAttrs( nullptr, &rItemSet );

                // all remaining options can be set at the default style,
                // then they're the default
                if( SfxItemState::SET==rItemSet.GetItemState(RES_CHRATR_COLOR,false) )
                    m_bBodyTextSet = true;
                SetTextCollAttrs(
                    GetTextCollFromPool( RES_POOLCOLL_STANDARD ),
                    rItemSet, rPropInfo, this );

                return;
            }
            break;
        default: break;
        }
    }
    else if( CSS1_SELTYPE_ELEM_CLASS==eSelType &&  HtmlTokenId::ANCHOR_ON==nToken2 &&
             !pNext && aClass.getLength() >= 9 &&
             ('s' == aClass[0] || 'S' == aClass[0]) )
    {
        sal_uInt16 nPoolFormatId = 0;
        if( aClass.equalsIgnoreAsciiCase(OOO_STRING_SVTOOLS_HTML_sdendnote_sym) )
            nPoolFormatId = RES_POOLCHR_ENDNOTE;
        else if( aClass.equalsIgnoreAsciiCase(OOO_STRING_SVTOOLS_HTML_sdfootnote_sym) )
            nPoolFormatId = RES_POOLCHR_FOOTNOTE;
        if( nPoolFormatId )
        {
            if( Css1ScriptFlags::AllMask == nScript )
            {
                SetCharFormatAttrs( GetCharFormatFromPool(nPoolFormatId), rItemSet );
            }
            else
            {
                SfxItemSet aScriptItemSet( rItemSet );
                RemoveScriptItems( aScriptItemSet, nScript );
                SetCharFormatAttrs( GetCharFormatFromPool(nPoolFormatId),
                                 aScriptItemSet);
            }
            return;
        }
    }

    // Now the selectors are processed which belong to a paragraph style
    sal_uInt16 nPoolCollId = 0;
    switch( nToken2 )
    {
    case HtmlTokenId::HEAD1_ON:
        nPoolCollId = RES_POOLCOLL_HEADLINE1;
        break;
    case HtmlTokenId::HEAD2_ON:
        nPoolCollId = RES_POOLCOLL_HEADLINE2;
        break;
    case HtmlTokenId::HEAD3_ON:
        nPoolCollId = RES_POOLCOLL_HEADLINE3;
        break;
    case HtmlTokenId::HEAD4_ON:
        nPoolCollId = RES_POOLCOLL_HEADLINE4;
        break;
    case HtmlTokenId::HEAD5_ON:
        nPoolCollId = RES_POOLCOLL_HEADLINE5;
        break;
    case HtmlTokenId::HEAD6_ON:
        nPoolCollId = RES_POOLCOLL_HEADLINE6;
        break;
    case HtmlTokenId::PARABREAK_ON:
        if( aClass.getLength() >= 9 &&
            ('s' == aClass[0] || 'S' == aClass[0]) )
        {
            if( aClass.equalsIgnoreAsciiCase(OOO_STRING_SVTOOLS_HTML_sdendnote) )
                nPoolCollId = RES_POOLCOLL_ENDNOTE;
            else if( aClass.equalsIgnoreAsciiCase(OOO_STRING_SVTOOLS_HTML_sdfootnote) )
                nPoolCollId = RES_POOLCOLL_FOOTNOTE;

            if( nPoolCollId )
                aClass.clear();
            else
                nPoolCollId = RES_POOLCOLL_TEXT;
        }
        else
        {
            nPoolCollId = RES_POOLCOLL_TEXT;
        }
        break;
    case HtmlTokenId::ADDRESS_ON:
        nPoolCollId = RES_POOLCOLL_SENDADRESS;
        break;
    case HtmlTokenId::BLOCKQUOTE_ON:
        nPoolCollId = RES_POOLCOLL_HTML_BLOCKQUOTE;
        break;
    case HtmlTokenId::DT_ON:
        nPoolCollId = RES_POOLCOLL_HTML_DT;
        break;
    case HtmlTokenId::DD_ON:
        nPoolCollId = RES_POOLCOLL_HTML_DD;
        break;
    case HtmlTokenId::PREFORMTXT_ON:
        nPoolCollId = RES_POOLCOLL_HTML_PRE;
        break;
    case HtmlTokenId::TABLEHEADER_ON:
    case HtmlTokenId::TABLEDATA_ON:
        if( CSS1_SELTYPE_ELEMENT==eSelType && !pNext )
        {
            InsertTag( aToken2, rItemSet, rPropInfo );
            return;
        }
        else if( CSS1_SELTYPE_ELEMENT==eSelType && pNext &&
                 (CSS1_SELTYPE_ELEMENT==eNextType ||
                  CSS1_SELTYPE_ELEM_CLASS==eNextType) )
        {
            // not TH and TD, but TH P and TD P
            OUString aSubToken, aSubClass;
            GetTokenAndClass( pNext, aSubToken, aSubClass, nScript );
            if( HtmlTokenId::PARABREAK_ON == GetHTMLToken( aSubToken ) )
            {
                aClass = aSubClass;
                pNext = pNext->GetNext();
                eNextType = pNext ? pNext->GetType() : CSS1_SELTYPE_ELEMENT;

                if( !aClass.isEmpty() || pNext )
                {
                    nPoolCollId = static_cast< sal_uInt16 >(
                        HtmlTokenId::TABLEHEADER_ON == nToken2 ? RES_POOLCOLL_TABLE_HDLN
                                                      : RES_POOLCOLL_TABLE );
                }
                else
                {
                    OUString sTmp = aToken2 + " " OOO_STRING_SVTOOLS_HTML_parabreak;

                    if( Css1ScriptFlags::AllMask == nScript )
                    {
                        InsertTag( sTmp, rItemSet, rPropInfo );
                    }
                    else
                    {
                        SfxItemSet aScriptItemSet( rItemSet );
                        RemoveScriptItems( aScriptItemSet, nScript );
                        InsertTag( sTmp, aScriptItemSet, rPropInfo );
                    }

                    return;
                }
            }
        }
        break;

    default:
        ;
    }

    if( nPoolCollId )
    {
        if( !pNext ||
            (CSS1_SELTYPE_PSEUDO==eNextType &&
             pNext->GetString().equalsIgnoreAsciiCase( "first-letter" ) &&
             SvxAdjust::Left == rPropInfo.m_eFloat) )
        {
            // either not a composed selector or a X:first-line { float: left; ... }

            // search resp. create the style
            SwTextFormatColl* pColl = GetTextFormatColl(nPoolCollId, OUString());
            SwTextFormatColl* pParentColl = nullptr;
            if( !aClass.isEmpty() )
            {
                OUString aName( pColl->GetName() );
                AddClassName( aName, aClass );

                pParentColl = pColl;
                pColl = m_pDoc->FindTextFormatCollByName( aName );
                if( !pColl )
                    pColl = m_pDoc->MakeTextFormatColl( aName, pParentColl );
            }
            if( !pNext )
            {
                // set only the attributes at the style
                const SfxPoolItem *pItem;
                const SvxBoxItem *pBoxItem = nullptr;
                if( SfxItemState::SET ==
                        pColl->GetAttrSet().GetItemState(RES_BOX,true,&pItem) )
                    pBoxItem = static_cast<const SvxBoxItem *>(pItem);
                rPropInfo.SetBoxItem( rItemSet, MIN_BORDER_DIST, pBoxItem );
                if( Css1ScriptFlags::AllMask == nScript && !pParentColl )
                {
                    SetTextCollAttrs( pColl, rItemSet, rPropInfo, this );
                }
                else
                {
                    SfxItemSet aScriptItemSet( rItemSet );
                    RemoveScriptItems( aScriptItemSet, nScript,
                                       pParentColl ? &pParentColl->GetAttrSet() : nullptr );
                    SetTextCollAttrs( pColl, aScriptItemSet, rPropInfo, this );
                }
            }
            else
            {
                // create a DropCap attribute
                SwFormatDrop aDrop( pColl->GetDrop() );
                aDrop.GetChars() = 1;

                // set the attributes of the DropCap attribute
                if( Css1ScriptFlags::AllMask == nScript )
                {
                    OUString sName(pColl->GetName());
                    FillDropCap( aDrop, rItemSet, &sName );
                }
                else
                {
                    SfxItemSet aScriptItemSet( rItemSet );
                    if( Css1ScriptFlags::Western != nScript )
                    {
                        aScriptItemSet.ClearItem( RES_CHRATR_FONT );
                        aScriptItemSet.ClearItem( RES_CHRATR_LANGUAGE );
                        aScriptItemSet.ClearItem( RES_CHRATR_POSTURE );
                        aScriptItemSet.ClearItem( RES_CHRATR_WEIGHT );
                    }
                    if( Css1ScriptFlags::CJK != nScript )
                    {
                        aScriptItemSet.ClearItem( RES_CHRATR_CJK_FONT );
                        aScriptItemSet.ClearItem( RES_CHRATR_CJK_LANGUAGE );
                        aScriptItemSet.ClearItem( RES_CHRATR_CJK_POSTURE );
                        aScriptItemSet.ClearItem( RES_CHRATR_CJK_WEIGHT );
                    }
                    if( Css1ScriptFlags::CTL != nScript )
                    {
                        aScriptItemSet.ClearItem( RES_CHRATR_CTL_FONT );
                        aScriptItemSet.ClearItem( RES_CHRATR_CTL_LANGUAGE );
                        aScriptItemSet.ClearItem( RES_CHRATR_CTL_POSTURE );
                        aScriptItemSet.ClearItem( RES_CHRATR_CTL_WEIGHT );
                    }
                    OUString sName(pColl->GetName());
                    FillDropCap( aDrop, aScriptItemSet, &sName );
                }

                // Only set the attribute if "float: left" is specified and
                // the Initial is over several lines. Otherwise the maybe
                // created character style will be later searched and set
                // via name.
                if( aDrop.GetLines() > 1 &&
                    (SvxAdjust::Left == rPropInfo.m_eFloat  ||
                     Css1ScriptFlags::AllMask == nScript) )
                {
                    pColl->SetFormatAttr( aDrop );
                }
            }
        }

        return;
    }

    // Now the selectors are processed which are belonging to the character
    // template. There are no composed ones here.
    if( pNext )
        return;

    SwCharFormat* pCFormat = GetChrFormat(nToken2, OUString());
    if( pCFormat )
    {
        SwCharFormat *pParentCFormat = nullptr;
        if( !aClass.isEmpty() )
        {
            OUString aName( pCFormat->GetName() );
            AddClassName( aName, aClass );
            pParentCFormat = pCFormat;

            pCFormat = m_pDoc->FindCharFormatByName( aName );
            if( !pCFormat )
            {
                pCFormat = m_pDoc->MakeCharFormat( aName, pParentCFormat );
                pCFormat->SetAuto(false);
            }
        }

        if( Css1ScriptFlags::AllMask == nScript && !pParentCFormat )
        {
            SetCharFormatAttrs( pCFormat, rItemSet );
        }
        else
        {
            SfxItemSet aScriptItemSet( rItemSet );
            RemoveScriptItems( aScriptItemSet, nScript,
                               pParentCFormat ? &pParentCFormat->GetAttrSet() : nullptr );
            SetCharFormatAttrs( pCFormat, aScriptItemSet );
        }
    }
}

sal_uInt32 SwCSS1Parser::GetFontHeight( sal_uInt16 nSize ) const
{
    return m_aFontHeights[ std::min<sal_uInt16>(nSize,6) ];
}

const FontList *SwCSS1Parser::GetFontList() const
{
    const FontList *pFList = nullptr;
    SwDocShell *pDocSh = m_pDoc->GetDocShell();
    if( pDocSh )
    {
        const SvxFontListItem *pFListItem =
            static_cast<const SvxFontListItem *>(pDocSh->GetItem(SID_ATTR_CHAR_FONTLIST));
        if( pFListItem )
            pFList = pFListItem->GetFontList();
    }

    return pFList;
}

SwCharFormat* SwCSS1Parser::GetChrFormat( HtmlTokenId nToken2, const OUString& rClass ) const
{
    // search the corresponding style
    sal_uInt16 nPoolId = 0;
    const sal_Char* sName = nullptr;
    switch( nToken2 )
    {
    case HtmlTokenId::EMPHASIS_ON:      nPoolId = RES_POOLCHR_HTML_EMPHASIS;    break;
    case HtmlTokenId::CITIATION_ON:     nPoolId = RES_POOLCHR_HTML_CITIATION;   break;
    case HtmlTokenId::STRONG_ON:        nPoolId = RES_POOLCHR_HTML_STRONG;      break;
    case HtmlTokenId::CODE_ON:          nPoolId = RES_POOLCHR_HTML_CODE;        break;
    case HtmlTokenId::SAMPLE_ON:        nPoolId = RES_POOLCHR_HTML_SAMPLE;      break;
    case HtmlTokenId::KEYBOARD_ON:      nPoolId = RES_POOLCHR_HTML_KEYBOARD;    break;
    case HtmlTokenId::VARIABLE_ON:      nPoolId = RES_POOLCHR_HTML_VARIABLE;    break;
    case HtmlTokenId::DEFINSTANCE_ON:   nPoolId = RES_POOLCHR_HTML_DEFINSTANCE; break;
    case HtmlTokenId::TELETYPE_ON:      nPoolId = RES_POOLCHR_HTML_TELETYPE;    break;

    case HtmlTokenId::SHORTQUOTE_ON:    sName = OOO_STRING_SVTOOLS_HTML_shortquote;     break;
    case HtmlTokenId::LANGUAGE_ON:      sName = OOO_STRING_SVTOOLS_HTML_language;   break;
    case HtmlTokenId::AUTHOR_ON:        sName = OOO_STRING_SVTOOLS_HTML_author;         break;
    case HtmlTokenId::PERSON_ON:        sName = OOO_STRING_SVTOOLS_HTML_person;         break;
    case HtmlTokenId::ACRONYM_ON:       sName = OOO_STRING_SVTOOLS_HTML_acronym;        break;
    case HtmlTokenId::ABBREVIATION_ON:  sName = OOO_STRING_SVTOOLS_HTML_abbreviation;   break;
    case HtmlTokenId::INSERTEDTEXT_ON:  sName = OOO_STRING_SVTOOLS_HTML_insertedtext;   break;
    case HtmlTokenId::DELETEDTEXT_ON:   sName = OOO_STRING_SVTOOLS_HTML_deletedtext;    break;
    default: break;
    }

    // search or create the style (only possible with name)
    if( !nPoolId && !sName )
        return nullptr;

    // search or create style (without class)
    SwCharFormat *pCFormat = nullptr;
    if( nPoolId )
    {
        pCFormat = GetCharFormatFromPool( nPoolId );
    }
    else
    {
        OUString sCName( OUString::createFromAscii(sName) );
        pCFormat = m_pDoc->FindCharFormatByName( sCName );
        if( !pCFormat )
        {
            pCFormat = m_pDoc->MakeCharFormat( sCName, m_pDoc->GetDfltCharFormat() );
            pCFormat->SetAuto(false);
        }
    }

    OSL_ENSURE( pCFormat, "No character style???" );

    // If a class exists, then search for the class style but don't
    // create one.
    OUString aClass( rClass );
    GetScriptFromClass( aClass, false );
    if( !aClass.isEmpty() )
    {
        OUString aTmp( pCFormat->GetName() );
        AddClassName( aTmp, aClass );
        SwCharFormat *pClassCFormat = m_pDoc->FindCharFormatByName( aTmp );
        if( pClassCFormat )
        {
            pCFormat = pClassCFormat;
        }
        else
        {
            const SvxCSS1MapEntry *pClass = GetClass( aClass );
            if( pClass )
            {
                pCFormat = m_pDoc->MakeCharFormat( aTmp, pCFormat );
                pCFormat->SetAuto(false);
                SfxItemSet aItemSet( pClass->GetItemSet() );
                SetCharFormatAttrs( pCFormat, aItemSet );
            }
        }
    }

    return pCFormat;
}

SwTextFormatColl *SwCSS1Parser::GetTextCollFromPool( sal_uInt16 nPoolId ) const
{
    const SwTextFormatColls::size_type nOldArrLen = m_pDoc->GetTextFormatColls()->size();

    SwTextFormatColl *pColl = m_pDoc->getIDocumentStylePoolAccess().GetTextCollFromPool( nPoolId, false );

    if( m_bIsNewDoc )
    {
        const SwTextFormatColls::size_type nArrLen = m_pDoc->GetTextFormatColls()->size();
        for( SwTextFormatColls::size_type i=nOldArrLen; i<nArrLen; ++i )
            lcl_swcss1_setEncoding( *(*m_pDoc->GetTextFormatColls())[i],
                                    GetDfltEncoding() );
    }

    return pColl;
}

SwCharFormat *SwCSS1Parser::GetCharFormatFromPool( sal_uInt16 nPoolId ) const
{
    const SwCharFormats::size_type nOldArrLen = m_pDoc->GetCharFormats()->size();

    SwCharFormat *pCharFormat = m_pDoc->getIDocumentStylePoolAccess().GetCharFormatFromPool( nPoolId );

    if( m_bIsNewDoc )
    {
        const SwCharFormats::size_type nArrLen = m_pDoc->GetCharFormats()->size();

        for( SwCharFormats::size_type i=nOldArrLen; i<nArrLen; i++ )
            lcl_swcss1_setEncoding( *(*m_pDoc->GetCharFormats())[i],
                                    GetDfltEncoding() );
    }

    return pCharFormat;
}

SwTextFormatColl *SwCSS1Parser::GetTextFormatColl( sal_uInt16 nTextColl,
                                           const OUString& rClass )
{
    SwTextFormatColl* pColl = nullptr;

    OUString aClass( rClass );
    GetScriptFromClass( aClass, false );
    if( RES_POOLCOLL_TEXT == nTextColl && aClass.getLength() >= 9 &&
        ('s' == aClass[0] || 'S' == aClass[0] ) )
    {
        if( aClass.equalsIgnoreAsciiCase(OOO_STRING_SVTOOLS_HTML_sdendnote) )
        {
            nTextColl = RES_POOLCOLL_ENDNOTE;
            aClass.clear();
        }
        else if( aClass.equalsIgnoreAsciiCase(OOO_STRING_SVTOOLS_HTML_sdfootnote) )
        {
            nTextColl = RES_POOLCOLL_FOOTNOTE;
            aClass.clear();
        }
    }

    if( USER_FMT & nTextColl )       // one created by Reader
    {
        OSL_ENSURE( false, "Where does the user style comes from?" );
        pColl = GetTextCollFromPool( RES_POOLCOLL_STANDARD );
    }
    else
    {
        pColl = GetTextCollFromPool( nTextColl );
    }

    OSL_ENSURE( pColl, "No paragraph style???" );
    if( !aClass.isEmpty() )
    {
        OUString aTmp( pColl->GetName() );
        AddClassName( aTmp, aClass );
        SwTextFormatColl* pClassColl = m_pDoc->FindTextFormatCollByName( aTmp );

        if( !pClassColl &&
            (nTextColl==RES_POOLCOLL_TABLE ||
             nTextColl==RES_POOLCOLL_TABLE_HDLN) )
        {
            // In this case there was a <TD><P CLASS=foo>, but no TD.foo
            // style was found. The we must use P.foo, if available.
            SwTextFormatColl* pCollText =
                GetTextCollFromPool( RES_POOLCOLL_TEXT );
            aTmp = pCollText->GetName();
            AddClassName( aTmp, aClass );
            pClassColl = m_pDoc->FindTextFormatCollByName( aTmp );
        }

        if( pClassColl )
        {
            pColl = pClassColl;
        }
        else
        {
            const SvxCSS1MapEntry *pClass = GetClass( aClass );
            if( pClass )
            {
                pColl = m_pDoc->MakeTextFormatColl( aTmp, pColl );
                SfxItemSet aItemSet( pClass->GetItemSet() );
                SvxCSS1PropertyInfo aPropInfo( pClass->GetPropertyInfo() );
                aPropInfo.SetBoxItem( aItemSet, MIN_BORDER_DIST );
                bool bPositioned = MayBePositioned( pClass->GetPropertyInfo() );
                if( bPositioned )
                    aItemSet.ClearItem( RES_BACKGROUND );
                SetTextCollAttrs( pColl, aItemSet, aPropInfo,
                                 this );
            }
        }

    }

    if( pColl )
        lcl_swcss1_setEncoding( *pColl, GetDfltEncoding() );

    return pColl;
}

SwPageDesc *SwCSS1Parser::GetMasterPageDesc()
{
    return m_pDoc->getIDocumentStylePoolAccess().GetPageDescFromPool( RES_POOLPAGE_HTML, false );
}

static SwPageDesc *FindPageDesc(SwDoc *pDoc, sal_uInt16 nPoolId)
{
    size_t nPageDescs = pDoc->GetPageDescCnt();
    size_t nPage;
    for (nPage=0; nPage < nPageDescs &&
         pDoc->GetPageDesc(nPage).GetPoolFormatId() != nPoolId; ++nPage)
         ;

    return nPage < nPageDescs ? &pDoc->GetPageDesc(nPage) : nullptr;
}

const SwPageDesc *SwCSS1Parser::GetPageDesc( sal_uInt16 nPoolId, bool bCreate )
{
    if( RES_POOLPAGE_HTML == nPoolId )
        return m_pDoc->getIDocumentStylePoolAccess().GetPageDescFromPool( RES_POOLPAGE_HTML, false );

    const SwPageDesc *pPageDesc = FindPageDesc(m_pDoc, nPoolId);
    if( !pPageDesc && bCreate )
    {
        // The first page is created from the right page, if there is one.
        SwPageDesc *pMasterPageDesc = nullptr;
        if( RES_POOLPAGE_FIRST == nPoolId )
            pMasterPageDesc = FindPageDesc(m_pDoc, RES_POOLPAGE_RIGHT);
        if( !pMasterPageDesc )
            pMasterPageDesc = m_pDoc->getIDocumentStylePoolAccess().GetPageDescFromPool( RES_POOLPAGE_HTML, false );

        // The new page style is created by copying from master
        SwPageDesc *pNewPageDesc = m_pDoc->
            getIDocumentStylePoolAccess().GetPageDescFromPool( nPoolId, false );

        // therefore we also need the number of the new style
        OSL_ENSURE(pNewPageDesc == FindPageDesc(m_pDoc, nPoolId), "page style not found");

        m_pDoc->CopyPageDesc( *pMasterPageDesc, *pNewPageDesc, false );

        // Modify the styles for their new purpose.
        const SwPageDesc *pFollow = nullptr;
        bool bSetFollowFollow = false;
        switch( nPoolId )
        {
        case RES_POOLPAGE_FIRST:
            // If there is already a left page, then is it the follow-up
            // style, else it is the HTML style.
            pFollow = GetLeftPageDesc();
            if( !pFollow )
                pFollow = pMasterPageDesc;
            break;

        case RES_POOLPAGE_RIGHT:
            // If the left style is already created, nothing will happen here.
            // Otherwise the left style is created and ensures the link with
            // the right style.
            GetLeftPageDesc( true );
            break;

        case RES_POOLPAGE_LEFT:
            // The right style is created if none exists. No links are created.
            // If there is already a first page style, then the left style becomes
            // follow-up style of the first page.
            pFollow = GetRightPageDesc( true );
            bSetFollowFollow = true;
            {
                const SwPageDesc *pFirstPageDesc = GetFirstPageDesc();
                if( pFirstPageDesc )
                {
                    SwPageDesc aNewFirstPageDesc( *pFirstPageDesc );
                    aNewFirstPageDesc.SetFollow( pNewPageDesc );
                    ChgPageDesc( pFirstPageDesc, aNewFirstPageDesc );
                }
            }
            break;
        }

        if( pFollow )
        {
            SwPageDesc aNewPageDesc( *pNewPageDesc );
            aNewPageDesc.SetFollow( pFollow );
            ChgPageDesc( pNewPageDesc, aNewPageDesc );

            if( bSetFollowFollow )
            {
                SwPageDesc aNewFollowPageDesc( *pFollow );
                aNewFollowPageDesc.SetFollow( pNewPageDesc );
                ChgPageDesc( pFollow, aNewFollowPageDesc );
            }
        }
        pPageDesc = pNewPageDesc;
    }

    return pPageDesc;
}

bool SwCSS1Parser::MayBePositioned( const SvxCSS1PropertyInfo& rPropInfo,
                                    bool bAutoWidth )
{
    // abs-pos
    // left/top none    auto    twip    perc

    // none     Z       Z       -       -
    // auto     Z       Z       -       -
    // twip     Z       Z       S/R     -
    // perc     -       -       -       -

    // - the tag will be positioned absolutely and left/top are both
    //   present and don't contain a percentage value, or
    // - the tag should flow, and
    // - a width was specified (needed in both cases)
    return ( ( SVX_CSS1_POS_ABSOLUTE     == rPropInfo.m_ePosition &&
               SVX_CSS1_LTYPE_PERCENTAGE != rPropInfo.m_eLeftType &&
               SVX_CSS1_LTYPE_PERCENTAGE != rPropInfo.m_eTopType &&
              (SVX_CSS1_LTYPE_TWIP       == rPropInfo.m_eLeftType ||
               SVX_CSS1_LTYPE_TWIP       != rPropInfo.m_eTopType) ) ||
             ( SvxAdjust::End            != rPropInfo.m_eFloat  ) ) &&
           ( bAutoWidth ||
             SVX_CSS1_LTYPE_TWIP         == rPropInfo.m_eWidthType ||
             SVX_CSS1_LTYPE_PERCENTAGE   == rPropInfo.m_eWidthType );
}

void SwCSS1Parser::AddClassName( OUString& rFormatName, const OUString& rClass )
{
    OSL_ENSURE( !rClass.isEmpty(), "Style class without length?" );

    rFormatName += "." + rClass;
}

void SwCSS1Parser::FillDropCap( SwFormatDrop& rDrop,
                                SfxItemSet& rItemSet,
                                const OUString *pName )
{
    // the number of lines matches somehow a percentage value
    // for the height (what happens with absolute heights???)
    sal_uInt8 nLines = rDrop.GetLines();
    const SfxPoolItem *pItem;
    if( SfxItemState::SET == rItemSet.GetItemState( RES_CHRATR_FONTSIZE, false, &pItem ) )
    {
        sal_uInt16 nProp = static_cast<const SvxFontHeightItem *>(pItem)->GetProp();
        nLines = static_cast<sal_uInt8>((nProp + 50) / 100);
        if( nLines < 1 )
            nLines = 1;
        else if( nLines > MAX_DROPCAP_LINES )
            nLines = MAX_DROPCAP_LINES;

        // Only when nLines>1, then the attribute also is set. Then
        // we don't need the font height in the character style.
        if( nLines > 1 )
        {
            rItemSet.ClearItem( RES_CHRATR_FONTSIZE );
            rItemSet.ClearItem( RES_CHRATR_CJK_FONTSIZE );
            rItemSet.ClearItem( RES_CHRATR_CTL_FONTSIZE );
        }
    }

    // In case of hard attribution (pName==0) we can stop, if the Initial is
    // only one line.
    if( nLines<=1 )
        return;

    rDrop.GetLines() = nLines;

    // a right border becomes the spacing to text!
    if( SfxItemState::SET == rItemSet.GetItemState( RES_LR_SPACE, false, &pItem ) )
    {
        rDrop.GetDistance() = static_cast< sal_uInt16 >(
            static_cast<const SvxLRSpaceItem *>(pItem)->GetRight() );
        rItemSet.ClearItem( RES_LR_SPACE );
    }

    // for every other attribute create a character style
    if( rItemSet.Count() )
    {
        SwCharFormat *pCFormat = nullptr;
        OUString aName;
        if( pName )
        {
            aName = *pName;
            aName += ".FL";   // first letter
            pCFormat = m_pDoc->FindCharFormatByName( aName );
        }
        else
        {
            do
            {
                aName = "first-letter " + OUString::number( static_cast<sal_Int32>(++m_nDropCapCnt) );
            }
            while( m_pDoc->FindCharFormatByName(aName) );
        }

        if( !pCFormat )
        {
            pCFormat = m_pDoc->MakeCharFormat( aName, m_pDoc->GetDfltCharFormat() );
            pCFormat->SetAuto(false);
        }
        SetCharFormatAttrs( pCFormat, rItemSet );

        // The character style needs only be set in the attribute, when
        // the attribute also is set.
        if( nLines > 1 )
            rDrop.SetCharFormat( pCFormat );
    }
}

// specific CSS1 of SwHTMLParsers

HTMLAttr **SwHTMLParser::GetAttrTabEntry( sal_uInt16 nWhich )
{
    // find the table entry of the item ...
    HTMLAttr **ppAttr = nullptr;
    switch( nWhich )
    {
    case RES_CHRATR_BLINK:
        ppAttr = &m_xAttrTab->pBlink;
        break;
    case RES_CHRATR_CASEMAP:
        ppAttr = &m_xAttrTab->pCaseMap;
        break;
    case RES_CHRATR_COLOR:
        ppAttr = &m_xAttrTab->pFontColor;
        break;
    case RES_CHRATR_CROSSEDOUT:
        ppAttr = &m_xAttrTab->pStrike;
        break;
    case RES_CHRATR_ESCAPEMENT:
        ppAttr = &m_xAttrTab->pEscapement;
        break;
    case RES_CHRATR_FONT:
        ppAttr = &m_xAttrTab->pFont;
        break;
    case RES_CHRATR_CJK_FONT:
        ppAttr = &m_xAttrTab->pFontCJK;
        break;
    case RES_CHRATR_CTL_FONT:
        ppAttr = &m_xAttrTab->pFontCTL;
        break;
    case RES_CHRATR_FONTSIZE:
        ppAttr = &m_xAttrTab->pFontHeight;
        break;
    case RES_CHRATR_CJK_FONTSIZE:
        ppAttr = &m_xAttrTab->pFontHeightCJK;
        break;
    case RES_CHRATR_CTL_FONTSIZE:
        ppAttr = &m_xAttrTab->pFontHeightCTL;
        break;
    case RES_CHRATR_KERNING:
        ppAttr = &m_xAttrTab->pKerning;
        break;
    case RES_CHRATR_POSTURE:
        ppAttr = &m_xAttrTab->pItalic;
        break;
    case RES_CHRATR_CJK_POSTURE:
        ppAttr = &m_xAttrTab->pItalicCJK;
        break;
    case RES_CHRATR_CTL_POSTURE:
        ppAttr = &m_xAttrTab->pItalicCTL;
        break;
    case RES_CHRATR_UNDERLINE:
        ppAttr = &m_xAttrTab->pUnderline;
        break;
    case RES_CHRATR_WEIGHT:
        ppAttr = &m_xAttrTab->pBold;
        break;
    case RES_CHRATR_CJK_WEIGHT:
        ppAttr = &m_xAttrTab->pBoldCJK;
        break;
    case RES_CHRATR_CTL_WEIGHT:
        ppAttr = &m_xAttrTab->pBoldCTL;
        break;
    case RES_CHRATR_BACKGROUND:
        ppAttr = &m_xAttrTab->pCharBrush;
        break;
    case RES_CHRATR_BOX:
        ppAttr = &m_xAttrTab->pCharBox;
        break;

    case RES_PARATR_LINESPACING:
        ppAttr = &m_xAttrTab->pLineSpacing;
        break;
    case RES_PARATR_ADJUST:
        ppAttr = &m_xAttrTab->pAdjust;
        break;

    case RES_LR_SPACE:
        ppAttr = &m_xAttrTab->pLRSpace;
        break;
    case RES_UL_SPACE:
        ppAttr = &m_xAttrTab->pULSpace;
        break;
    case RES_BOX:
        ppAttr = &m_xAttrTab->pBox;
        break;
    case RES_BACKGROUND:
        ppAttr = &m_xAttrTab->pBrush;
        break;
    case RES_BREAK:
        ppAttr = &m_xAttrTab->pBreak;
        break;
    case RES_PAGEDESC:
        ppAttr = &m_xAttrTab->pPageDesc;
        break;
    case RES_PARATR_SPLIT:
        ppAttr = &m_xAttrTab->pSplit;
        break;
    case RES_PARATR_WIDOWS:
        ppAttr = &m_xAttrTab->pWidows;
        break;
    case RES_PARATR_ORPHANS:
        ppAttr = &m_xAttrTab->pOrphans;
        break;
    case RES_KEEP:
        ppAttr = &m_xAttrTab->pKeep;
        break;

    case RES_CHRATR_LANGUAGE:
        ppAttr = &m_xAttrTab->pLanguage;
        break;
    case RES_CHRATR_CJK_LANGUAGE:
        ppAttr = &m_xAttrTab->pLanguageCJK;
        break;
    case RES_CHRATR_CTL_LANGUAGE:
        ppAttr = &m_xAttrTab->pLanguageCTL;
        break;

    case RES_FRAMEDIR:
        ppAttr = &m_xAttrTab->pDirection;
        break;
    }

    return ppAttr;
}

void SwHTMLParser::NewStyle()
{
    OUString sType;

    const HTMLOptions& rOptions2 = GetOptions();
    for (size_t i = rOptions2.size(); i; )
    {
        const HTMLOption& rOption = rOptions2[--i];
        if( HtmlOptionId::TYPE == rOption.GetToken() )
            sType = rOption.GetString();
    }

    m_bIgnoreRawData = sType.getLength() &&
                     !sType.getToken(0,';').equalsAscii(sCSS_mimetype);
}

void SwHTMLParser::EndStyle()
{
    m_bIgnoreRawData = false;

    if( !m_aStyleSource.isEmpty() )
    {
        m_pCSS1Parser->ParseStyleSheet( m_aStyleSource );
        m_aStyleSource.clear();
    }
}

bool SwHTMLParser::FileDownload( const OUString& rURL,
                                 OUString& rStr )
{
    // depose view (because of reschedule)
    SwViewShell *pOldVSh = CallEndAction();

    SfxMedium aDLMedium( rURL, StreamMode::READ | StreamMode::SHARE_DENYWRITE );

    SvStream* pStream = aDLMedium.GetInStream();
    if( pStream )
    {
        SvMemoryStream aStream;
        aStream.WriteStream( *pStream );

        rStr = OUString(static_cast<const sal_Char *>(aStream.GetData()), aStream.TellEnd(),
            GetSrcEncoding());
    }

    // was aborted?
    if( ( m_xDoc->GetDocShell() && m_xDoc->GetDocShell()->IsAbortingImport() )
        || 1 == m_xDoc->getReferenceCount() )
    {
        // was the import aborted from SFX?
        eState = SvParserState::Error;
        pStream = nullptr;
    }

    // recreate View
    SwViewShell *const pVSh = CallStartAction( pOldVSh );
    OSL_ENSURE( pOldVSh == pVSh, "FileDownload: SwViewShell changed on us" );

    return pStream!=nullptr;
}

void SwHTMLParser::InsertLink()
{
    bool bFinishDownload = false;
    if( !m_vPendingStack.empty() )
    {
        OSL_ENSURE( ShouldFinishFileDownload(),
                "Pending-Stack without File-Download?" );

        m_vPendingStack.pop_back();
        assert( m_vPendingStack.empty() && "Where does the Pending-Stack come from?" );

        bFinishDownload = true;
    }
    else
    {
        OUString sRel, sHRef, sType;

        const HTMLOptions& rOptions2 = GetOptions();
        for (size_t i = rOptions2.size(); i; )
        {
            const HTMLOption& rOption = rOptions2[--i];
            switch( rOption.GetToken() )
            {
                case HtmlOptionId::REL:
                    sRel = rOption.GetString();
                    break;
                case HtmlOptionId::HREF:
                    sHRef = URIHelper::SmartRel2Abs( INetURLObject( m_sBaseURL ), rOption.GetString(), Link<OUString *, bool>(), false );
                    break;
                case HtmlOptionId::TYPE:
                    sType = rOption.GetString();
                    break;
                default: break;
            }
        }

        if( !sHRef.isEmpty() && sRel.equalsIgnoreAsciiCase( "STYLESHEET" ) &&
            ( sType.isEmpty() ||
              sType.getToken(0,';').equalsAscii(sCSS_mimetype) ) )
        {
            if( GetMedium() )
            {
                // start download of style source
                StartFileDownload(sHRef);
                if( IsParserWorking() )
                {
                    // The style was loaded synchronously and we can call it directly.
                    bFinishDownload = true;
                }
                else
                {
                    // The style was load asynchronously and is only available
                    // on the next continue call. Therefore we must create a
                    // Pending stack, so that we will return to here.
                    m_vPendingStack.emplace_back( HtmlTokenId::LINK );
                }
            }
            else
            {
                // load file synchronous
                OUString sSource;
                if( FileDownload( sHRef, sSource ) )
                    m_pCSS1Parser->ParseStyleSheet( sSource );
            }
        }
    }

    if( bFinishDownload )
    {
        OUString sSource;
        if( FinishFileDownload( sSource ) && !sSource.isEmpty() )
            m_pCSS1Parser->ParseStyleSheet( sSource );
    }
}

bool SwCSS1Parser::ParseStyleSheet( const OUString& rIn )
{
    if( !SvxCSS1Parser::ParseStyleSheet( rIn ) )
        return false;

    SwPageDesc *pMasterPageDesc =
        m_pDoc->getIDocumentStylePoolAccess().GetPageDescFromPool( RES_POOLPAGE_HTML, false );

    SvxCSS1MapEntry* pPageEntry = GetPage(OUString(), false);
    if( pPageEntry )
    {
        // @page (affects all already existing pages)

        SetPageDescAttrs( pMasterPageDesc, pPageEntry->GetItemSet(),
                          pPageEntry->GetPropertyInfo() );

        // For all other already existing page styles the attributes
        // must also be set

        SetPageDescAttrs( GetFirstPageDesc(), pPageEntry->GetItemSet(),
                          pPageEntry->GetPropertyInfo() );
        SetPageDescAttrs( GetLeftPageDesc(), pPageEntry->GetItemSet(),
                          pPageEntry->GetPropertyInfo() );
        SetPageDescAttrs( GetRightPageDesc(), pPageEntry->GetItemSet(),
                          pPageEntry->GetPropertyInfo() );

    }

    pPageEntry = GetPage( "first", true );
    if( pPageEntry )
    {
        SetPageDescAttrs( GetFirstPageDesc(true), pPageEntry->GetItemSet(),
                          pPageEntry->GetPropertyInfo() );
        m_bSetFirstPageDesc = true;
    }

    pPageEntry = GetPage( "right", true );
    if( pPageEntry )
    {
        SetPageDescAttrs( GetRightPageDesc(true), pPageEntry->GetItemSet(),
                          pPageEntry->GetPropertyInfo() );
        m_bSetRightPageDesc = true;
    }

    pPageEntry = GetPage( "left", true );
    if( pPageEntry )
        SetPageDescAttrs( GetLeftPageDesc(true), pPageEntry->GetItemSet(),
                          pPageEntry->GetPropertyInfo() );

    return true;
}

bool SwHTMLParser::ParseStyleOptions( const OUString &rStyle,
                                          const OUString &rId,
                                          const OUString &rClass,
                                          SfxItemSet &rItemSet,
                                          SvxCSS1PropertyInfo &rPropInfo,
                                          const OUString *pLang,
                                          const OUString *pDir )
{
    bool bRet = false;

    if( !rClass.isEmpty() )
    {
        OUString aClass( rClass );
        SwCSS1Parser::GetScriptFromClass( aClass );
        const SvxCSS1MapEntry *pClass = m_pCSS1Parser->GetClass( aClass );
        if( pClass )
        {
            SvxCSS1Parser::MergeStyles( pClass->GetItemSet(),
                                      pClass->GetPropertyInfo(),
                                      rItemSet, rPropInfo, false );
            bRet = true;
        }
    }

    if( !rId.isEmpty() )
    {
        const SvxCSS1MapEntry *pId = m_pCSS1Parser->GetId( rId );
        if( pId )
            SvxCSS1Parser::MergeStyles( pId->GetItemSet(),
                                      pId->GetPropertyInfo(),
                                      rItemSet, rPropInfo, !rClass.isEmpty() );
        rPropInfo.m_aId = rId;
        bRet = true;
    }

    if( !rStyle.isEmpty() )
    {
        m_pCSS1Parser->ParseStyleOption( rStyle, rItemSet, rPropInfo );
        bRet = true;
    }

    if( bRet )
        rPropInfo.SetBoxItem( rItemSet, MIN_BORDER_DIST );

    if( pLang && !pLang->isEmpty() )
    {
        LanguageType eLang = LanguageTag::convertToLanguageTypeWithFallback( *pLang );
        if( LANGUAGE_DONTKNOW != eLang )
        {
            SvxLanguageItem aLang( eLang, RES_CHRATR_LANGUAGE );
            rItemSet.Put( aLang );
            aLang.SetWhich( RES_CHRATR_CJK_LANGUAGE );
            rItemSet.Put( aLang );
            aLang.SetWhich( RES_CHRATR_CTL_LANGUAGE );
            rItemSet.Put( aLang );

            bRet = true;
        }
    }
    if( pDir && !pDir->isEmpty() )
    {
        OUString aValue( *pDir );
        SvxFrameDirection eDir = SvxFrameDirection::Environment;
        if (aValue.equalsIgnoreAsciiCase("LTR"))
            eDir = SvxFrameDirection::Horizontal_LR_TB;
        else if (aValue.equalsIgnoreAsciiCase("RTL"))
            eDir = SvxFrameDirection::Horizontal_RL_TB;

        if( SvxFrameDirection::Environment != eDir )
        {
            SvxFrameDirectionItem aDir( eDir, RES_FRAMEDIR );
            rItemSet.Put( aDir );

            bRet = true;
        }
    }

    return bRet;
}

void SwHTMLParser::SetAnchorAndAdjustment( const SvxCSS1PropertyInfo &rPropInfo,
                                           SfxItemSet &rFrameItemSet )
{
    SwFormatAnchor aAnchor;

    sal_Int16 eHoriOri = text::HoriOrientation::NONE;
    sal_Int16 eVertOri = text::VertOrientation::NONE;
    sal_Int16 eHoriRel = text::RelOrientation::FRAME;
    sal_Int16 eVertRel = text::RelOrientation::FRAME;
    SwTwips nHoriPos = 0, nVertPos = 0;
    css::text::WrapTextMode eSurround = css::text::WrapTextMode_THROUGH;
    if( SVX_CSS1_POS_ABSOLUTE == rPropInfo.m_ePosition )
    {
        if( SVX_CSS1_LTYPE_TWIP == rPropInfo.m_eLeftType &&
            SVX_CSS1_LTYPE_TWIP == rPropInfo.m_eTopType )
        {
            // Absolute positioned objects are page-bound, when they
            // aren't in a frame and otherwise frame-bound.
            const SwStartNode *pFlySttNd =
                m_pPam->GetPoint()->nNode.GetNode().FindFlyStartNode();
            if( pFlySttNd )
            {
                aAnchor.SetType( RndStdIds::FLY_AT_FLY );
                SwPosition aPos( *pFlySttNd );
                aAnchor.SetAnchor( &aPos );
            }
            else
            {
                aAnchor.SetType( RndStdIds::FLY_AT_PAGE );
                aAnchor.SetPageNum( 1 );
            }
            nHoriPos = rPropInfo.m_nLeft;
            nVertPos = rPropInfo.m_nTop;
        }
        else
        {
            aAnchor.SetType( RndStdIds::FLY_AT_PARA );
            aAnchor.SetAnchor( m_pPam->GetPoint() );
            eVertOri = text::VertOrientation::TOP;
            eVertRel = text::RelOrientation::CHAR;
            if( SVX_CSS1_LTYPE_TWIP == rPropInfo.m_eLeftType )
            {
                eHoriOri = text::HoriOrientation::NONE;
                eHoriRel = text::RelOrientation::PAGE_FRAME;
                nHoriPos = rPropInfo.m_nLeft;
            }
            else
            {
                eHoriOri = text::HoriOrientation::LEFT;
                eHoriRel = text::RelOrientation::FRAME;   // to be changed later
            }
        }
    }
    else
    {
        // Flowing object are inserted as paragraph-bound, when the paragraph is
        // still empty and otherwise auto-bound.
        // Auto-bound frames for the time being inserted at the previous position
        // and later moved.
        const sal_Int32 nContent = m_pPam->GetPoint()->nContent.GetIndex();
        if( nContent )
        {
            aAnchor.SetType( RndStdIds::FLY_AT_CHAR );
            m_pPam->Move( fnMoveBackward );
            eVertOri = text::VertOrientation::CHAR_BOTTOM;
            eVertRel = text::RelOrientation::CHAR;
        }
        else
        {
            aAnchor.SetType( RndStdIds::FLY_AT_PARA );
            eVertOri = text::VertOrientation::TOP;
            eVertRel = text::RelOrientation::PRINT_AREA;
        }

        aAnchor.SetAnchor( m_pPam->GetPoint() );

        if( nContent )
            m_pPam->Move( fnMoveForward );

        sal_uInt16 nLeftSpace = 0, nRightSpace = 0;
        short nIndent = 0;
        GetMarginsFromContextWithNumBul( nLeftSpace, nRightSpace, nIndent );

        if( SvxAdjust::Right==rPropInfo.m_eFloat )
        {
            eHoriOri = text::HoriOrientation::RIGHT;
            eHoriRel = nRightSpace ? text::RelOrientation::PRINT_AREA : text::RelOrientation::FRAME;
            eSurround = css::text::WrapTextMode_LEFT;
        }
        else
        {
            eHoriOri = text::HoriOrientation::LEFT;
            eHoriRel = nLeftSpace ? text::RelOrientation::PRINT_AREA : text::RelOrientation::FRAME;
            eSurround = css::text::WrapTextMode_RIGHT;
        }
    }
    rFrameItemSet.Put( aAnchor );

    // positioned absolutely with wrap
    rFrameItemSet.Put( SwFormatHoriOrient( nHoriPos, eHoriOri, eHoriRel ) );
    rFrameItemSet.Put( SwFormatVertOrient( nVertPos, eVertOri, eVertRel ) );
    rFrameItemSet.Put( SwFormatSurround( eSurround ) );
}

void SwHTMLParser::SetVarSize( SvxCSS1PropertyInfo const &rPropInfo,
                               SfxItemSet &rFrameItemSet,
                               SwTwips nDfltWidth, sal_uInt8 nDfltPrcWidth )
{
    SwTwips nWidth = nDfltWidth, nHeight = MINFLY;
    sal_uInt8 nPrcWidth = nDfltPrcWidth, nPrcHeight = 0;
    switch( rPropInfo.m_eWidthType )
    {
    case SVX_CSS1_LTYPE_PERCENTAGE:
        nPrcWidth = rPropInfo.m_nWidth > 0 ? static_cast<sal_uInt8>(rPropInfo.m_nWidth) : 1;
        nWidth = MINFLY;
        break;
    case SVX_CSS1_LTYPE_TWIP:
        nWidth = std::max<long>(rPropInfo.m_nWidth, MINFLY);
        nPrcWidth = 0;
        break;
    default:
        ;
    }
    switch( rPropInfo.m_eHeightType )
    {
    case SVX_CSS1_LTYPE_PERCENTAGE:
        nPrcHeight = rPropInfo.m_nHeight > 0 ? static_cast<sal_uInt8>(rPropInfo.m_nHeight) : 1;
        break;
    case SVX_CSS1_LTYPE_TWIP:
        // Netscape and MS-IE interpreting the height incorrectly as minimum height,
        // therefore we are doing the same.
        nHeight = std::max<long>(rPropInfo.m_nHeight, MINFLY);
        break;
    default:
        ;
    }

    SwFormatFrameSize aFrameSize( ATT_MIN_SIZE, nWidth, nHeight );
    aFrameSize.SetWidthPercent( nPrcWidth );
    aFrameSize.SetHeightPercent( nPrcHeight );
    rFrameItemSet.Put( aFrameSize );
}

void SwHTMLParser::SetFrameFormatAttrs( SfxItemSet &rItemSet,
                                   HtmlFrameFormatFlags nFlags,
                                   SfxItemSet &rFrameItemSet )
{
    const SfxPoolItem *pItem;
    if( (nFlags & HtmlFrameFormatFlags::Box) &&
        SfxItemState::SET==rItemSet.GetItemState( RES_BOX, true, &pItem ) )
    {
        if( nFlags & HtmlFrameFormatFlags::Padding )
        {
            SvxBoxItem aBoxItem( *static_cast<const SvxBoxItem *>(pItem) );
            // reset all 4 sides to 0
            aBoxItem.SetAllDistances(0);
            rFrameItemSet.Put( aBoxItem );
        }
        else
        {
            rFrameItemSet.Put( *pItem );
        }
        rItemSet.ClearItem( RES_BOX );
    }

    if( (nFlags & HtmlFrameFormatFlags::Background) &&
        SfxItemState::SET==rItemSet.GetItemState( RES_BACKGROUND, true, &pItem ) )
    {
        rFrameItemSet.Put( *pItem );
        rItemSet.ClearItem( RES_BACKGROUND );
    }

    if( (nFlags & HtmlFrameFormatFlags::Direction) &&
        SfxItemState::SET==rItemSet.GetItemState( RES_FRAMEDIR, true, &pItem ) )
    {
        rFrameItemSet.Put( *pItem );
        rItemSet.ClearItem( RES_FRAMEDIR );
    }
}

std::unique_ptr<HTMLAttrContext> SwHTMLParser::PopContext( HtmlTokenId nToken )
{
    std::unique_ptr<HTMLAttrContext> xCntxt;

    HTMLAttrContexts::size_type nPos = m_aContexts.size();
    if( nPos <= m_nContextStMin )
        return nullptr;

    bool bFound = HtmlTokenId::NONE == nToken;
    if( nToken != HtmlTokenId::NONE )
    {
        // search for stack entry of token ...
        while( nPos > m_nContextStMin )
        {
            HtmlTokenId nCntxtToken = m_aContexts[--nPos]->GetToken();
            if( nCntxtToken == nToken )
            {
                bFound = true;
                break;
            }
            else if( nCntxtToken == HtmlTokenId::NONE ) // zero as token doesn't occur
            {
                break;
            }
        }
    }
    else
    {
        nPos--;
    }

    if( bFound )
    {
        xCntxt = std::move(m_aContexts[nPos]);
        m_aContexts.erase( m_aContexts.begin() + nPos );
    }

    return xCntxt;
}

void SwHTMLParser::GetMarginsFromContext( sal_uInt16& nLeft,
                                          sal_uInt16& nRight,
                                          short& nIndent,
                                          bool bIgnoreTopContext ) const
{
    HTMLAttrContexts::size_type nPos = m_aContexts.size();
    if( bIgnoreTopContext )
    {
        if( !nPos )
            return;
        else
            nPos--;
    }

    while( nPos > m_nContextStAttrMin )
    {
        const HTMLAttrContext *pCntxt = m_aContexts[--nPos].get();
        if( pCntxt->IsLRSpaceChanged() )
        {
            pCntxt->GetMargins( nLeft, nRight, nIndent );
            return;
        }
    }
}

void SwHTMLParser::GetMarginsFromContextWithNumBul( sal_uInt16& nLeft,
                                                    sal_uInt16& nRight,
                                                    short& nIndent ) const
{
    GetMarginsFromContext( nLeft, nRight, nIndent );
    const SwHTMLNumRuleInfo& rInfo = const_cast<SwHTMLParser*>(this)->GetNumInfo();
    if( rInfo.GetDepth() )
    {
        sal_uInt8 nLevel = static_cast<sal_uInt8>( (rInfo.GetDepth() <= MAXLEVEL ? rInfo.GetDepth()
                                                            : MAXLEVEL) - 1 );
        const SwNumFormat& rNumFormat = rInfo.GetNumRule()->Get(nLevel);
        nLeft = nLeft + rNumFormat.GetAbsLSpace(); //TODO: overflow
        nIndent = rNumFormat.GetFirstLineOffset(); //TODO: overflow
    }
}

void SwHTMLParser::GetULSpaceFromContext( sal_uInt16& nUpper,
                                          sal_uInt16& nLower ) const
{
    sal_uInt16 nDfltColl = 0;
    OUString aDfltClass;

    HTMLAttrContexts::size_type nPos = m_aContexts.size();
    while( nPos > m_nContextStAttrMin )
    {
        const HTMLAttrContext *pCntxt = m_aContexts[--nPos].get();
        if( pCntxt->IsULSpaceChanged() )
        {
            pCntxt->GetULSpace( nUpper, nLower );
            return;
        }
        else if( !nDfltColl )
        {
            nDfltColl = pCntxt->GetDfltTextFormatColl();
            if( nDfltColl )
                aDfltClass = pCntxt->GetClass();
        }
    }

    if( !nDfltColl )
        nDfltColl = RES_POOLCOLL_TEXT;

    const SwTextFormatColl *pColl =
        m_pCSS1Parser->GetTextFormatColl( nDfltColl, aDfltClass );
    const SvxULSpaceItem& rULSpace = pColl->GetULSpace();
    nUpper = rULSpace.GetUpper();
    nLower = rULSpace.GetLower();
}

void SwHTMLParser::EndContextAttrs( HTMLAttrContext *pContext )
{
    HTMLAttrs &rAttrs = pContext->GetAttrs();
    for( auto pAttr : rAttrs )
    {
        if( RES_PARATR_DROP==pAttr->GetItem().Which() )
        {
            // Set the number of characters for DropCaps. If it's zero at the
            // end, the attribute is set to invalid and then isn't set from SetAttr.
            sal_Int32 nChars = m_pPam->GetPoint()->nContent.GetIndex();
            if( nChars < 1 )
                pAttr->Invalidate();
            else if( nChars > MAX_DROPCAP_CHARS )
                nChars = MAX_DROPCAP_CHARS;
            static_cast<SwFormatDrop&>(pAttr->GetItem()).GetChars() = static_cast<sal_uInt8>(nChars);
        }

        EndAttr( pAttr );
    }
}

void SwHTMLParser::InsertParaAttrs( const SfxItemSet& rItemSet )
{
    SfxItemIter aIter( rItemSet );

    const SfxPoolItem *pItem = aIter.FirstItem();
    while( pItem )
    {
        // search for the table entry of the item...
        sal_uInt16 nWhich = pItem->Which();
        HTMLAttr **ppAttr = GetAttrTabEntry( nWhich );

        if( ppAttr )
        {
            NewAttr(m_xAttrTab, ppAttr, *pItem);
            if( RES_PARATR_BEGIN > nWhich )
                (*ppAttr)->SetLikePara();
            m_aParaAttrs.push_back( *ppAttr );
            bool bSuccess = EndAttr( *ppAttr, false );
            if (!bSuccess)
                m_aParaAttrs.pop_back();
        }

        pItem = aIter.NextItem();
    }
}

static void lcl_swcss1_setEncoding( SwFormat& rFormat, rtl_TextEncoding eEnc )
{
    if( RTL_TEXTENCODING_DONTKNOW == eEnc )
        return;

    const SfxItemSet& rItemSet = rFormat.GetAttrSet();
    static const sal_uInt16 aWhichIds[3] = { RES_CHRATR_FONT, RES_CHRATR_CJK_FONT,
                                   RES_CHRATR_CTL_FONT };
    const SfxPoolItem *pItem;
    for(sal_uInt16 i : aWhichIds)
    {
        if( SfxItemState::SET == rItemSet.GetItemState( i, false,&pItem ) )
        {
            const SvxFontItem& rFont = *static_cast<const SvxFontItem *>(pItem);
            if( RTL_TEXTENCODING_SYMBOL != rFont.GetCharSet() )
            {
                SvxFontItem aFont( rFont.GetFamily(), rFont.GetFamilyName(),
                                   rFont.GetStyleName(), rFont.GetPitch(),
                                   eEnc, i);
                rFormat.SetFormatAttr( aFont );
            }
        }
    }
}

void SwCSS1Parser::SetDfltEncoding( rtl_TextEncoding eEnc )
{
    if( eEnc != GetDfltEncoding() )
    {
        if( m_bIsNewDoc )
        {
            // Set new encoding as pool default
            static const sal_uInt16 aWhichIds[3] = { RES_CHRATR_FONT, RES_CHRATR_CJK_FONT,
                                           RES_CHRATR_CTL_FONT };
            for(sal_uInt16 i : aWhichIds)
            {
                const SvxFontItem& rDfltFont =
                    static_cast<const SvxFontItem&>(m_pDoc->GetDefault( i));
                SvxFontItem aFont( rDfltFont.GetFamily(),
                                   rDfltFont.GetFamilyName(),
                                   rDfltFont.GetStyleName(),
                                   rDfltFont.GetPitch(),
                                   eEnc, i );
                m_pDoc->SetDefault( aFont );
            }

            // Change all paragraph styles that do specify a font.
            for( auto pTextFormatColl : *m_pDoc->GetTextFormatColls() )
                lcl_swcss1_setEncoding( *pTextFormatColl, eEnc );

            // Change all character styles that do specify a font.
            for( auto pCharFormat : *m_pDoc->GetCharFormats() )
                lcl_swcss1_setEncoding( *pCharFormat, eEnc );
        }

        SvxCSS1Parser::SetDfltEncoding( eEnc );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
