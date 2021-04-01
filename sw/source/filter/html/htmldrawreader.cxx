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
#include <vcl/svapp.hxx>
#include <svx/svdpage.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdotext.hxx>
#include <svx/sdtagitm.hxx>
#include <svx/sdtacitm.hxx>
#include <svx/sdtayitm.hxx>
#include <svx/sdtaaitm.hxx>
#include <svx/sdtaiitm.hxx>
#include <svx/sdtmfitm.hxx>
#include <editeng/eeitem.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <editeng/colritem.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <svl/itemiter.hxx>
#include <svtools/htmltokn.h>
#include <svtools/htmlkywd.hxx>
#include <osl/diagnose.h>

#include <charatr.hxx>
#include <drawdoc.hxx>
#include <fmtanchr.hxx>
#include <fmtornt.hxx>
#include <fmtsrnd.hxx>
#include <ndtxt.hxx>
#include <doc.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <poolfmt.hxx>
#include "swcss1.hxx"
#include "swhtml.hxx"
#include <shellio.hxx>

using namespace css;

HTMLOptionEnum<SdrTextAniKind> const aHTMLMarqBehaviorTable[] =
{
    { OOO_STRING_SVTOOLS_HTML_BEHAV_scroll,    SdrTextAniKind::Scroll       },
    { OOO_STRING_SVTOOLS_HTML_BEHAV_alternate, SdrTextAniKind::Alternate    },
    { OOO_STRING_SVTOOLS_HTML_BEHAV_slide,     SdrTextAniKind::Slide        },
    { nullptr,                                 SdrTextAniKind(0)       }
};

HTMLOptionEnum<SdrTextAniDirection> const aHTMLMarqDirectionTable[] =
{
    { OOO_STRING_SVTOOLS_HTML_AL_left,  SdrTextAniDirection::Left   },
    { OOO_STRING_SVTOOLS_HTML_AL_right, SdrTextAniDirection::Right  },
    { nullptr,                          SdrTextAniDirection(0)      }
};

void SwHTMLParser::InsertDrawObject( SdrObject* pNewDrawObj,
                                     const Size& rPixSpace,
                                     sal_Int16 eVertOri,
                                     sal_Int16 eHoriOri,
                                     SfxItemSet& rCSS1ItemSet,
                                     SvxCSS1PropertyInfo& rCSS1PropInfo )
{
    // always on top of text.
    // but in invisible layer. <ConnectToLayout> will move the object
    // to the visible layer.
    pNewDrawObj->SetLayer( m_xDoc->getIDocumentDrawModelAccess().GetInvisibleHeavenId() );

    SfxItemSet aFrameSet( m_xDoc->GetAttrPool(),
                        svl::Items<RES_FRMATR_BEGIN, RES_FRMATR_END-1>{} );
    if( !IsNewDoc() )
        Reader::ResetFrameFormatAttrs( aFrameSet );

    sal_uInt16 nLeftSpace = 0, nRightSpace = 0, nUpperSpace = 0, nLowerSpace = 0;
    if( (rPixSpace.Width() || rPixSpace.Height()) && Application::GetDefaultDevice() )
    {
        Size aTwipSpc( rPixSpace.Width(), rPixSpace.Height() );
        aTwipSpc =
            Application::GetDefaultDevice()->PixelToLogic( aTwipSpc,
                                                MapMode(MapUnit::MapTwip) );
        nLeftSpace = nRightSpace = static_cast<sal_uInt16>(aTwipSpc.Width());
        nUpperSpace = nLowerSpace = static_cast<sal_uInt16>(aTwipSpc.Height());
    }

    // set left/right border
    const SfxPoolItem *pItem;
    if( SfxItemState::SET==rCSS1ItemSet.GetItemState( RES_LR_SPACE, true, &pItem ) )
    {
        // maybe flatten the first line indentation
        const SvxLRSpaceItem *pLRItem = static_cast<const SvxLRSpaceItem *>(pItem);
        SvxLRSpaceItem aLRItem( *pLRItem );
        aLRItem.SetTextFirstLineOffset( 0 );
        if( rCSS1PropInfo.m_bLeftMargin )
        {
            nLeftSpace = static_cast< sal_uInt16 >(aLRItem.GetLeft());
            rCSS1PropInfo.m_bLeftMargin = false;
        }
        if( rCSS1PropInfo.m_bRightMargin )
        {
            nRightSpace = static_cast< sal_uInt16 >(aLRItem.GetRight());
            rCSS1PropInfo.m_bRightMargin = false;
        }
        rCSS1ItemSet.ClearItem( RES_LR_SPACE );
    }
    if( nLeftSpace || nRightSpace )
    {
        SvxLRSpaceItem aLRItem( RES_LR_SPACE );
        aLRItem.SetLeft( nLeftSpace );
        aLRItem.SetRight( nRightSpace );
        aFrameSet.Put( aLRItem );
    }

    // set top/bottom border
    if( SfxItemState::SET==rCSS1ItemSet.GetItemState( RES_UL_SPACE, true, &pItem ) )
    {
        // maybe flatten the first line indentation
        const SvxULSpaceItem *pULItem = static_cast<const SvxULSpaceItem *>(pItem);
        if( rCSS1PropInfo.m_bTopMargin )
        {
            nUpperSpace = pULItem->GetUpper();
            rCSS1PropInfo.m_bTopMargin = false;
        }
        if( rCSS1PropInfo.m_bBottomMargin )
        {
            nLowerSpace = pULItem->GetLower();
            rCSS1PropInfo.m_bBottomMargin = false;
        }

        rCSS1ItemSet.ClearItem( RES_UL_SPACE );
    }
    if( nUpperSpace || nLowerSpace )
    {
        SvxULSpaceItem aULItem( RES_UL_SPACE );
        aULItem.SetUpper( nUpperSpace );
        aULItem.SetLower( nLowerSpace );
        aFrameSet.Put( aULItem );
    }

    SwFormatAnchor aAnchor( RndStdIds::FLY_AS_CHAR );
    if( SVX_CSS1_POS_ABSOLUTE == rCSS1PropInfo.m_ePosition &&
        SVX_CSS1_LTYPE_TWIP == rCSS1PropInfo.m_eLeftType &&
        SVX_CSS1_LTYPE_TWIP == rCSS1PropInfo.m_eTopType )
    {
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
        }
        // #i26791# - direct positioning for <SwDoc::Insert(..)>
        pNewDrawObj->SetRelativePos( Point(rCSS1PropInfo.m_nLeft + nLeftSpace,
                                           rCSS1PropInfo.m_nTop + nUpperSpace) );
        aFrameSet.Put( SwFormatSurround(css::text::WrapTextMode_THROUGH) );
    }
    else if( SvxAdjust::Left == rCSS1PropInfo.m_eFloat ||
             text::HoriOrientation::LEFT == eHoriOri )
    {
        aAnchor.SetType( RndStdIds::FLY_AT_PARA );
        aFrameSet.Put( SwFormatSurround(css::text::WrapTextMode_RIGHT) );
        // #i26791# - direct positioning for <SwDoc::Insert(..)>
        pNewDrawObj->SetRelativePos( Point(nLeftSpace, nUpperSpace) );
    }
    else if( text::VertOrientation::NONE != eVertOri )
    {
        aFrameSet.Put( SwFormatVertOrient( 0, eVertOri ) );
    }

    if (RndStdIds::FLY_AT_PAGE == aAnchor.GetAnchorId())
    {
        aAnchor.SetPageNum( 1 );
    }
    else if( RndStdIds::FLY_AT_FLY != aAnchor.GetAnchorId() )
    {
        aAnchor.SetAnchor( m_pPam->GetPoint() );
    }
    aFrameSet.Put( aAnchor );

    m_xDoc->getIDocumentContentOperations().InsertDrawObj( *m_pPam, *pNewDrawObj, aFrameSet );
}

static void PutEEPoolItem( SfxItemSet &rEEItemSet,
                           const SfxPoolItem& rSwItem )
{

    sal_uInt16 nEEWhich = 0;

    switch( rSwItem.Which() )
    {
    case RES_CHRATR_COLOR:          nEEWhich = EE_CHAR_COLOR; break;
    case RES_CHRATR_CROSSEDOUT:     nEEWhich = EE_CHAR_STRIKEOUT; break;
    case RES_CHRATR_ESCAPEMENT:     nEEWhich = EE_CHAR_ESCAPEMENT; break;
    case RES_CHRATR_FONT:           nEEWhich = EE_CHAR_FONTINFO; break;
    case RES_CHRATR_CJK_FONT:       nEEWhich = EE_CHAR_FONTINFO_CJK; break;
    case RES_CHRATR_CTL_FONT:       nEEWhich = EE_CHAR_FONTINFO_CTL; break;
    case RES_CHRATR_FONTSIZE:       nEEWhich = EE_CHAR_FONTHEIGHT; break;
    case RES_CHRATR_CJK_FONTSIZE:   nEEWhich = EE_CHAR_FONTHEIGHT_CJK; break;
    case RES_CHRATR_CTL_FONTSIZE:   nEEWhich = EE_CHAR_FONTHEIGHT_CTL; break;
    case RES_CHRATR_KERNING:        nEEWhich = EE_CHAR_KERNING; break;
    case RES_CHRATR_POSTURE:        nEEWhich = EE_CHAR_ITALIC; break;
    case RES_CHRATR_CJK_POSTURE:    nEEWhich = EE_CHAR_ITALIC_CJK; break;
    case RES_CHRATR_CTL_POSTURE:    nEEWhich = EE_CHAR_ITALIC_CTL; break;
    case RES_CHRATR_UNDERLINE:      nEEWhich = EE_CHAR_UNDERLINE; break;
    case RES_CHRATR_WEIGHT:         nEEWhich = EE_CHAR_WEIGHT; break;
    case RES_CHRATR_CJK_WEIGHT:     nEEWhich = EE_CHAR_WEIGHT_CJK; break;
    case RES_CHRATR_CTL_WEIGHT:     nEEWhich = EE_CHAR_WEIGHT_CTL; break;
    case RES_BACKGROUND:
    case RES_CHRATR_BACKGROUND:
        {
            const SvxBrushItem& rBrushItem = static_cast<const SvxBrushItem&>(rSwItem);
            rEEItemSet.Put( XFillStyleItem(drawing::FillStyle_SOLID) );
            rEEItemSet.Put(XFillColorItem(OUString(),
                            rBrushItem.GetColor()) );
        }
        break;
    }

    if( nEEWhich )
        rEEItemSet.Put( rSwItem.CloneSetWhich(nEEWhich) );
}

void SwHTMLParser::NewMarquee( HTMLTable *pCurTable )
{

    OSL_ENSURE( !m_pMarquee, "Marquee in Marquee???" );
    m_aContents.clear();

    OUString aId, aStyle, aClass;

    tools::Long nWidth=0, nHeight=0;
    bool bPercentWidth = false, bDirection = false, bBGColor = false;
    Size aSpace( 0, 0 );
    sal_Int16 eVertOri = text::VertOrientation::TOP;
    sal_Int16 eHoriOri = text::HoriOrientation::NONE;
    SdrTextAniKind eAniKind = SdrTextAniKind::Scroll;
    SdrTextAniDirection eAniDir = SdrTextAniDirection::Left;
    sal_uInt16 nCount = 0, nDelay = 60;
    sal_Int16 nAmount = -6;
    Color aBGColor;

    const HTMLOptions& rHTMLOptions = GetOptions();
    for (const auto & rOption : rHTMLOptions)
    {
        switch( rOption.GetToken() )
        {
            case HtmlOptionId::ID:
                aId = rOption.GetString();
                break;
            case HtmlOptionId::STYLE:
                aStyle = rOption.GetString();
                break;
            case HtmlOptionId::CLASS:
                aClass = rOption.GetString();
                break;

            case HtmlOptionId::BEHAVIOR:
                eAniKind = rOption.GetEnum( aHTMLMarqBehaviorTable, eAniKind );
                break;

            case HtmlOptionId::BGCOLOR:
                rOption.GetColor( aBGColor );
                bBGColor = true;
                break;

            case HtmlOptionId::DIRECTION:
                eAniDir = rOption.GetEnum( aHTMLMarqDirectionTable, eAniDir );
                bDirection = true;
                break;

            case HtmlOptionId::LOOP:
                if (rOption.GetString().
                    equalsIgnoreAsciiCase(OOO_STRING_SVTOOLS_HTML_LOOP_infinite))
                {
                    nCount = 0;
                }
                else
                {
                    const sal_Int32 nLoop = rOption.GetSNumber();
                    nCount = std::max<sal_Int32>(nLoop, 0);
                }
                break;

            case HtmlOptionId::SCROLLAMOUNT:
                nAmount = - static_cast<sal_Int16>(rOption.GetNumber());
                break;

            case HtmlOptionId::SCROLLDELAY:
                nDelay = static_cast<sal_uInt16>(rOption.GetNumber());
                break;

            case HtmlOptionId::WIDTH:
                // first only save as pixel value!
                nWidth = rOption.GetNumber();
                bPercentWidth = rOption.GetString().indexOf('%') != -1;
                if( bPercentWidth && nWidth>100 )
                    nWidth = 100;
                break;

            case HtmlOptionId::HEIGHT:
                // first only save as pixel value!
                nHeight = rOption.GetNumber();
                if( rOption.GetString().indexOf('%') != -1 )
                    nHeight = 0;
                break;

            case HtmlOptionId::HSPACE:
                // first only save as pixel value!
                aSpace.setHeight( rOption.GetNumber() );
                break;

            case HtmlOptionId::VSPACE:
                // first only save as pixel value!
                aSpace.setWidth( rOption.GetNumber() );
                break;

            case HtmlOptionId::ALIGN:
                eVertOri =
                    rOption.GetEnum( aHTMLImgVAlignTable,
                                                    text::VertOrientation::TOP );
                eHoriOri =
                    rOption.GetEnum( aHTMLImgHAlignTable );
                break;
            default: break;
        }
    }

    // create a DrawTextobj
    // #i52858# - method name changed
    SwDrawModel* pModel = m_xDoc->getIDocumentDrawModelAccess().GetOrCreateDrawModel();
    SdrPage* pPg = pModel->GetPage( 0 );
    m_pMarquee = SdrObjFactory::MakeNewObject(
        *pModel,
        SdrInventor::Default,
        OBJ_TEXT);

    if( !m_pMarquee )
        return;

    pPg->InsertObject( m_pMarquee );

    if( !aId.isEmpty() )
        InsertBookmark( aId );

    // (only) Alternate runs from left to right as default
    if( SdrTextAniKind::Alternate==eAniKind && !bDirection )
        eAniDir = SdrTextAniDirection::Right;

    // re set the attributes needed for scrolling
    sal_uInt16 const aWhichMap[] { XATTR_FILL_FIRST,   XATTR_FILL_LAST,
                                   SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST,
                                   EE_CHAR_START,      EE_CHAR_END,
                                   0 };
    SfxItemSet aItemSet( pModel->GetItemPool(), aWhichMap );
    aItemSet.Put( makeSdrTextAutoGrowWidthItem( false ) );
    aItemSet.Put( makeSdrTextAutoGrowHeightItem( true ) );
    aItemSet.Put( SdrTextAniKindItem( eAniKind ) );
    aItemSet.Put( SdrTextAniDirectionItem( eAniDir ) );
    aItemSet.Put( SdrTextAniCountItem( nCount ) );
    aItemSet.Put( SdrTextAniDelayItem( nDelay ) );
    aItemSet.Put( SdrTextAniAmountItem( nAmount ) );
    if( SdrTextAniKind::Alternate==eAniKind )
    {
        // (only) Alternate starts and ends Inside as default
        aItemSet.Put( SdrTextAniStartInsideItem(true) );
        aItemSet.Put( SdrTextAniStopInsideItem(true) );
        if( SdrTextAniDirection::Left==eAniDir )
            aItemSet.Put( SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_RIGHT) );
    }

    // set the default colour (from the default template), so that a meaningful
    // colour is set at all
    const Color& rDfltColor =
        m_pCSS1Parser->GetTextCollFromPool( RES_POOLCOLL_STANDARD )
            ->GetColor().GetValue();
    aItemSet.Put( SvxColorItem( rDfltColor, EE_CHAR_COLOR ) );

    // set the attributes of the current paragraph style
    sal_uInt16 nWhichIds[] =
    {
        RES_CHRATR_COLOR,   RES_CHRATR_CROSSEDOUT, RES_CHRATR_ESCAPEMENT,
        RES_CHRATR_FONT,    RES_CHRATR_FONTSIZE,   RES_CHRATR_KERNING,
        RES_CHRATR_POSTURE, RES_CHRATR_UNDERLINE,  RES_CHRATR_WEIGHT,
        RES_CHRATR_BACKGROUND,
        RES_CHRATR_CJK_FONT, RES_CHRATR_CJK_FONTSIZE,
        RES_CHRATR_CJK_POSTURE, RES_CHRATR_CJK_WEIGHT,
        RES_CHRATR_CTL_FONT, RES_CHRATR_CTL_FONTSIZE,
        RES_CHRATR_CTL_POSTURE, RES_CHRATR_CTL_WEIGHT,
        0
    };
    SwTextNode const*const pTextNd =
        m_pPam->GetPoint()->nNode.GetNode().GetTextNode();
    if( pTextNd )
    {
        const SfxItemSet& rItemSet = pTextNd->GetAnyFormatColl().GetAttrSet();
        const SfxPoolItem *pItem;
        for( int i=0; nWhichIds[i]; ++i )
        {
            if( SfxItemState::SET == rItemSet.GetItemState( nWhichIds[i], true, &pItem ) )
                PutEEPoolItem( aItemSet, *pItem );
        }
    }

    // set attribute of environment at the Draw object
    HTMLAttr** pHTMLAttributes = reinterpret_cast<HTMLAttr**>(m_xAttrTab.get());
    for (auto nCnt = sizeof(HTMLAttrTable) / sizeof(HTMLAttr*); nCnt--; ++pHTMLAttributes)
    {
        HTMLAttr *pAttr = *pHTMLAttributes;
        if( pAttr )
            PutEEPoolItem( aItemSet, pAttr->GetItem() );
    }

    if( bBGColor )
    {
        aItemSet.Put( XFillStyleItem(drawing::FillStyle_SOLID) );
        aItemSet.Put(XFillColorItem(OUString(), aBGColor));
    }

    // parse styles (is here only possible for attributes, which also
    // can be set at character object)
    SfxItemSet aStyleItemSet( m_xDoc->GetAttrPool(),
                              m_pCSS1Parser->GetWhichMap() );
    SvxCSS1PropertyInfo aPropInfo;
    if( HasStyleOptions( aStyle, aId, aClass )  &&
        ParseStyleOptions( aStyle, aId, aClass, aStyleItemSet, aPropInfo ) )
    {
        SfxItemIter aIter( aStyleItemSet );

        for (const SfxPoolItem* pItem = aIter.GetCurItem(); pItem; pItem = aIter.NextItem())
        {
            PutEEPoolItem( aItemSet, *pItem );
        }
    }

    // now set the size
    Size aTwipSz( bPercentWidth ? 0 : nWidth, nHeight );
    if( (aTwipSz.Width() || aTwipSz.Height()) && Application::GetDefaultDevice() )
    {
        aTwipSz = Application::GetDefaultDevice()
                    ->PixelToLogic( aTwipSz, MapMode( MapUnit::MapTwip ) );
    }

    if( SVX_CSS1_LTYPE_TWIP== aPropInfo.m_eWidthType )
    {
        aTwipSz.setWidth( aPropInfo.m_nWidth );
        nWidth = 1; // != 0;
        bPercentWidth = false;
    }
    if( SVX_CSS1_LTYPE_TWIP== aPropInfo.m_eHeightType )
        aTwipSz.setHeight( aPropInfo.m_nHeight );

    m_bFixMarqueeWidth = false;
    if( !nWidth || bPercentWidth )
    {
        if( m_xTable )
        {
            if( !pCurTable )
            {
                // The marquee is in a table, but not in a cell. Since now no
                // reasonable mapping to a cell is possible, we adjust here the
                // width to the content of the marquee.
                m_bFixMarqueeWidth = true;
            }
            else if( !nWidth )
            {
                // Because we know in which cell the marquee is, we also can
                // adjust the width. No width specification is treated as
                // 100 percent.
                nWidth = 100;
                bPercentWidth = true;
            }
            aTwipSz.setWidth( MINLAY );
        }
        else
        {
            tools::Long nBrowseWidth = GetCurrentBrowseWidth();
            aTwipSz.setWidth( !nWidth ? nBrowseWidth
                                      : (nWidth*nBrowseWidth) / 100 );
        }
    }

    // The height is only minimum height
    if( aTwipSz.Height() < MINFLY )
        aTwipSz.setHeight( MINFLY );
    aItemSet.Put( makeSdrTextMinFrameHeightItem( aTwipSz.Height() ) );

    m_pMarquee->SetMergedItemSetAndBroadcast(aItemSet);

    if( aTwipSz.Width() < MINFLY )
        aTwipSz.setWidth( MINFLY );
    m_pMarquee->SetLogicRect( tools::Rectangle( 0, 0, aTwipSz.Width(), aTwipSz.Height() ) );

    // and insert the object into the document
    InsertDrawObject( m_pMarquee, aSpace, eVertOri, eHoriOri, aStyleItemSet,
                      aPropInfo );

    // Register the drawing object at the table. Is a little bit complicated,
    // because it is done via the parser, although the table is known, but
    // otherwise the table would have to be public and that also isn't pretty.
    // The global pTable also can't be used, because the marquee can also be
    // in a sub-table.
    if( pCurTable && bPercentWidth)
        RegisterDrawObjectToTable( pCurTable, m_pMarquee, static_cast<sal_uInt8>(nWidth) );
}

void SwHTMLParser::EndMarquee()
{
    OSL_ENSURE( m_pMarquee && OBJ_TEXT==m_pMarquee->GetObjIdentifier(),
            "no marquee or wrong type" );

    if( m_bFixMarqueeWidth )
    {
        // Because there is no fixed height make the text object wider then
        // the text, so that there is no line break.
        const tools::Rectangle& rOldRect = m_pMarquee->GetLogicRect();
        m_pMarquee->SetLogicRect( tools::Rectangle( rOldRect.TopLeft(),
                                           Size( USHRT_MAX, 240 ) ) );
    }

    // insert the collected text
    static_cast<SdrTextObj*>(m_pMarquee)->SetText( m_aContents );
    m_pMarquee->SetMergedItemSetAndBroadcast( m_pMarquee->GetMergedItemSet() );

    if( m_bFixMarqueeWidth )
    {
        // adjust the size to the text
        static_cast<SdrTextObj*>(m_pMarquee)->FitFrameToTextSize();
    }

    m_aContents.clear();
    m_pMarquee = nullptr;
}

void SwHTMLParser::InsertMarqueeText()
{
    OSL_ENSURE( m_pMarquee && OBJ_TEXT==m_pMarquee->GetObjIdentifier(),
            "no marquee or wrong type" );

    // append the current text part to the text
    m_aContents += aToken;
}

void SwHTMLParser::ResizeDrawObject( SdrObject* pObj, SwTwips nWidth )
{
    OSL_ENSURE( OBJ_TEXT==pObj->GetObjIdentifier(),
            "no marquee or wrong type" );

    if( OBJ_TEXT!=pObj->GetObjIdentifier() )
        return;

    // the old size
    const tools::Rectangle& rOldRect = pObj->GetLogicRect();
    Size aNewSz( nWidth, rOldRect.GetSize().Height() );
    pObj->SetLogicRect( tools::Rectangle( rOldRect.TopLeft(), aNewSz ) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
