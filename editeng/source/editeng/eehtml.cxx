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


#include "eehtml.hxx"
#include <editeng/adjustitem.hxx>
#include <editeng/flditem.hxx>
#include <tools/urlobj.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/wghtitem.hxx>
#include <svtools/htmltokn.h>
#include <svtools/htmlkywd.hxx>
#include <tools/tenccvt.hxx>

#include <editeng/editeng.hxx>

#define STYLE_PRE               101

EditHTMLParser::EditHTMLParser( SvStream& rIn, const OUString& rBaseURL, SvKeyValueIterator* pHTTPHeaderAttrs )
    : HTMLParser( rIn, true ),
    aBaseURL( rBaseURL ),
    mpEditEngine(nullptr),
    bInPara(false),
    bWasInPara(false),
    bFieldsInserted(false),
    bInTitle(false),
    nInTable(0),
    nInCell(0),
    nDefListLevel(0)
{
    DBG_ASSERT( !IsSwitchToUCS2(), "EditHTMLParser::::EditHTMLParser: Switch to UCS2?" );

    // Although the real default encoding is ISO8859-1, we use MS-1252
    // as default encoding.
    SetSrcEncoding( GetExtendedCompatibilityTextEncoding(  RTL_TEXTENCODING_ISO_8859_1 ) );

    // If the file starts with a BOM, switch to UCS2.
    SetSwitchToUCS2( true );

    if ( pHTTPHeaderAttrs )
        SetEncodingByHTTPHeader( pHTTPHeaderAttrs );
}

EditHTMLParser::~EditHTMLParser()
{
}

SvParserState EditHTMLParser::CallParser(EditEngine* pEE, const EditPaM& rPaM)
{
    DBG_ASSERT(pEE, "CallParser: ImpEditEngine ?!");
    mpEditEngine = pEE;
    SvParserState _eState = SvParserState::NotStarted;
    if ( mpEditEngine )
    {
        // Build in wrap mimic in RTF import?
        aCurSel = EditSelection( rPaM, rPaM );

        if (mpEditEngine->IsHtmlImportHandlerSet())
        {
            HtmlImportInfo aImportInfo(HtmlImportState::Start, this, mpEditEngine->CreateESelection(aCurSel));
            mpEditEngine->CallHtmlImportHandler(aImportInfo);
        }

        ImpSetStyleSheet( 0 );
        _eState = HTMLParser::CallParser();

        if (mpEditEngine->IsHtmlImportHandlerSet())
        {
            HtmlImportInfo aImportInfo(HtmlImportState::End, this, mpEditEngine->CreateESelection(aCurSel));
            mpEditEngine->CallHtmlImportHandler(aImportInfo);
        }

        if ( bFieldsInserted )
            mpEditEngine->UpdateFieldsOnly();
    }
    return _eState;
}

void EditHTMLParser::NextToken( HtmlTokenId nToken )
{
    switch( nToken )
    {
    case HtmlTokenId::META:
    {
        const HTMLOptions& aOptions = GetOptions();
        size_t nArrLen = aOptions.size();
        bool bEquiv = false;
        for ( size_t i = 0; i < nArrLen; i++ )
        {
            const HTMLOption& aOption = aOptions[i];
            switch( aOption.GetToken() )
            {
                case HtmlOptionId::HTTPEQUIV:
                {
                    bEquiv = true;
                }
                break;
                case HtmlOptionId::CONTENT:
                {
                    if ( bEquiv )
                    {
                        rtl_TextEncoding eEnc = GetEncodingByMIME( aOption.GetString() );
                        if ( eEnc != RTL_TEXTENCODING_DONTKNOW )
                            SetSrcEncoding( eEnc );
                    }
                }
                break;
                default: break;
            }
        }

    }
    break;
    case HtmlTokenId::PLAINTEXT_ON:
    case HtmlTokenId::PLAINTEXT2_ON:
        bInPara = true;
    break;
    case HtmlTokenId::PLAINTEXT_OFF:
    case HtmlTokenId::PLAINTEXT2_OFF:
        bInPara = false;
    break;

    case HtmlTokenId::LINEBREAK:
    case HtmlTokenId::NEWPARA:
    {
        if ( ( bInPara || nInTable ) &&
            ( ( nToken == HtmlTokenId::LINEBREAK ) || HasTextInCurrentPara() ) )
        {
            ImpInsertParaBreak();
        }
    }
    break;
    case HtmlTokenId::HORZRULE:
    {
        if ( HasTextInCurrentPara() )
            ImpInsertParaBreak();
        ImpInsertParaBreak();
    }
    break;
    case HtmlTokenId::NONBREAKSPACE:
    {
        if ( bInPara )
        {
            ImpInsertText( " " );
        }
    }
    break;
    case HtmlTokenId::RAWDATA:
        if (IsReadStyle() && !aToken.isEmpty())
        {
            // Each token represents a single line.
            maStyleSource.append(aToken);
            maStyleSource.append('\n');
        }
    break;
    case HtmlTokenId::TEXTTOKEN:
    {
        // #i110937# for <title> content, call aImportHdl (no SkipGroup), but don't insert the text into the EditEngine
        if (!bInTitle)
        {
            if ( !bInPara )
                StartPara( false );

            OUString aText = aToken;
            if ( aText.startsWith(" ") && ThrowAwayBlank() && !IsReadPRE() )
                aText = aText.copy( 1 );

            if ( pCurAnchor )
            {
                pCurAnchor->aText += aText;
            }
            else
            {
                // Only written until HTML with 319?
                if ( IsReadPRE() )
                {
                    sal_Int32 nTabPos = aText.indexOf( '\t');
                    while ( nTabPos != -1 )
                    {
                        aText = aText.replaceAt( nTabPos, 1, "        " );
                        nTabPos = aText.indexOf( '\t', nTabPos+8 );
                    }
                }
                ImpInsertText( aText );
            }
        }
    }
    break;

    case HtmlTokenId::CENTER_ON:
    case HtmlTokenId::CENTER_OFF:
                            {
                                sal_Int32 nNode = mpEditEngine->GetEditDoc().GetPos( aCurSel.Max().GetNode() );
                                SfxItemSet aItems( aCurSel.Max().GetNode()->GetContentAttribs().GetItems() );
                                aItems.ClearItem( EE_PARA_JUST );
                                if ( nToken == HtmlTokenId::CENTER_ON )
                                    aItems.Put( SvxAdjustItem( SvxAdjust::Center, EE_PARA_JUST ) );
                                mpEditEngine->SetParaAttribsOnly(nNode, aItems);
                            }
                            break;

    case HtmlTokenId::ANCHOR_ON:    AnchorStart();
                            break;
    case HtmlTokenId::ANCHOR_OFF:   AnchorEnd();
                            break;

    case HtmlTokenId::PARABREAK_ON:
        if( bInPara && HasTextInCurrentPara() )
            EndPara();
        StartPara( true );
        break;

    case HtmlTokenId::PARABREAK_OFF:
        if( bInPara )
            EndPara();
        break;

    case HtmlTokenId::HEAD1_ON:
    case HtmlTokenId::HEAD2_ON:
    case HtmlTokenId::HEAD3_ON:
    case HtmlTokenId::HEAD4_ON:
    case HtmlTokenId::HEAD5_ON:
    case HtmlTokenId::HEAD6_ON:
    {
        HeadingStart( nToken );
    }
    break;

    case HtmlTokenId::HEAD1_OFF:
    case HtmlTokenId::HEAD2_OFF:
    case HtmlTokenId::HEAD3_OFF:
    case HtmlTokenId::HEAD4_OFF:
    case HtmlTokenId::HEAD5_OFF:
    case HtmlTokenId::HEAD6_OFF:
    {
        HeadingEnd();
    }
    break;

    case HtmlTokenId::PREFORMTXT_ON:
    case HtmlTokenId::XMP_ON:
    case HtmlTokenId::LISTING_ON:
    {
        StartPara( true );
        ImpSetStyleSheet( STYLE_PRE );
    }
    break;

    case HtmlTokenId::DEFLIST_ON:
    {
        nDefListLevel++;
    }
    break;

    case HtmlTokenId::DEFLIST_OFF:
    {
        if( nDefListLevel )
            nDefListLevel--;
    }
    break;

    case HtmlTokenId::TABLE_ON:     nInTable++;
                            break;
    case HtmlTokenId::TABLE_OFF:    DBG_ASSERT( nInTable, "Not in Table, but TABLE_OFF?" );
                            nInTable--;
                            break;

    case HtmlTokenId::TABLEHEADER_ON:
    case HtmlTokenId::TABLEDATA_ON:
        nInCell++;
        [[fallthrough]];
    case HtmlTokenId::BLOCKQUOTE_ON:
    case HtmlTokenId::BLOCKQUOTE_OFF:
    case HtmlTokenId::BLOCKQUOTE30_ON:
    case HtmlTokenId::BLOCKQUOTE30_OFF:
    case HtmlTokenId::LISTHEADER_ON:
    case HtmlTokenId::LI_ON:
    case HtmlTokenId::DD_ON:
    case HtmlTokenId::DT_ON:
    case HtmlTokenId::ORDERLIST_ON:
    case HtmlTokenId::UNORDERLIST_ON:
    {
        bool bHasText = HasTextInCurrentPara();
        if ( bHasText )
            ImpInsertParaBreak();
        StartPara( false );
    }
    break;

    case HtmlTokenId::TABLEHEADER_OFF:
    case HtmlTokenId::TABLEDATA_OFF:
    {
        if ( nInCell )
            nInCell--;
        [[fallthrough]];
    }
    case HtmlTokenId::LISTHEADER_OFF:
    case HtmlTokenId::LI_OFF:
    case HtmlTokenId::DD_OFF:
    case HtmlTokenId::DT_OFF:
    case HtmlTokenId::ORDERLIST_OFF:
    case HtmlTokenId::UNORDERLIST_OFF:  EndPara();
                                break;

    case HtmlTokenId::TABLEROW_ON:
    case HtmlTokenId::TABLEROW_OFF: // A RETURN only after a CELL, for Calc

    case HtmlTokenId::COL_ON:
    case HtmlTokenId::COLGROUP_ON:
    case HtmlTokenId::COLGROUP_OFF: break;

    case HtmlTokenId::FONT_ON:
                            break;
    case HtmlTokenId::FONT_OFF:
                            break;

    case HtmlTokenId::TITLE_ON:
        bInTitle = true;
        break;
    case HtmlTokenId::TITLE_OFF:
        bInTitle = false;
        break;

    // globals
    case HtmlTokenId::HTML_ON:
    case HtmlTokenId::HTML_OFF:
    case HtmlTokenId::STYLE_ON:
    case HtmlTokenId::STYLE_OFF:
    case HtmlTokenId::BODY_ON:
    case HtmlTokenId::BODY_OFF:
    case HtmlTokenId::HEAD_ON:
    case HtmlTokenId::HEAD_OFF:
    case HtmlTokenId::FORM_ON:
    case HtmlTokenId::FORM_OFF:
    case HtmlTokenId::THEAD_ON:
    case HtmlTokenId::THEAD_OFF:
    case HtmlTokenId::TBODY_ON:
    case HtmlTokenId::TBODY_OFF:
    // inline elements, structural markup
    // HTML 3.0
    case HtmlTokenId::BANNER_ON:
    case HtmlTokenId::BANNER_OFF:
    case HtmlTokenId::DIVISION_ON:
    case HtmlTokenId::DIVISION_OFF:
//  case HtmlTokenId::LISTHEADER_ON:        //! special handling
//  case HtmlTokenId::LISTHEADER_OFF:
    case HtmlTokenId::NOTE_ON:
    case HtmlTokenId::NOTE_OFF:
    // inline elements, logical markup
    // HTML 2.0
    case HtmlTokenId::ADDRESS_ON:
    case HtmlTokenId::ADDRESS_OFF:
//  case HtmlTokenId::BLOCKQUOTE_ON:        //! special handling
//  case HtmlTokenId::BLOCKQUOTE_OFF:
    case HtmlTokenId::CITATION_ON:
    case HtmlTokenId::CITATION_OFF:
    case HtmlTokenId::CODE_ON:
    case HtmlTokenId::CODE_OFF:
    case HtmlTokenId::DEFINSTANCE_ON:
    case HtmlTokenId::DEFINSTANCE_OFF:
    case HtmlTokenId::EMPHASIS_ON:
    case HtmlTokenId::EMPHASIS_OFF:
    case HtmlTokenId::KEYBOARD_ON:
    case HtmlTokenId::KEYBOARD_OFF:
    case HtmlTokenId::SAMPLE_ON:
    case HtmlTokenId::SAMPLE_OFF:
    case HtmlTokenId::STRIKE_ON:
    case HtmlTokenId::STRIKE_OFF:
    case HtmlTokenId::STRONG_ON:
    case HtmlTokenId::STRONG_OFF:
    case HtmlTokenId::VARIABLE_ON:
    case HtmlTokenId::VARIABLE_OFF:
    // HTML 3.0
    case HtmlTokenId::ABBREVIATION_ON:
    case HtmlTokenId::ABBREVIATION_OFF:
    case HtmlTokenId::ACRONYM_ON:
    case HtmlTokenId::ACRONYM_OFF:
    case HtmlTokenId::AUTHOR_ON:
    case HtmlTokenId::AUTHOR_OFF:
//  case HtmlTokenId::BLOCKQUOTE30_ON:      //! special handling
//  case HtmlTokenId::BLOCKQUOTE30_OFF:
    case HtmlTokenId::DELETEDTEXT_ON:
    case HtmlTokenId::DELETEDTEXT_OFF:
    case HtmlTokenId::INSERTEDTEXT_ON:
    case HtmlTokenId::INSERTEDTEXT_OFF:
    case HtmlTokenId::LANGUAGE_ON:
    case HtmlTokenId::LANGUAGE_OFF:
    case HtmlTokenId::PERSON_ON:
    case HtmlTokenId::PERSON_OFF:
    case HtmlTokenId::SHORTQUOTE_ON:
    case HtmlTokenId::SHORTQUOTE_OFF:
    case HtmlTokenId::SUBSCRIPT_ON:
    case HtmlTokenId::SUBSCRIPT_OFF:
    case HtmlTokenId::SUPERSCRIPT_ON:
    case HtmlTokenId::SUPERSCRIPT_OFF:
    // inline elements, visual markup
    // HTML 2.0
    case HtmlTokenId::BOLD_ON:
    case HtmlTokenId::BOLD_OFF:
    case HtmlTokenId::ITALIC_ON:
    case HtmlTokenId::ITALIC_OFF:
    case HtmlTokenId::TELETYPE_ON:
    case HtmlTokenId::TELETYPE_OFF:
    case HtmlTokenId::UNDERLINE_ON:
    case HtmlTokenId::UNDERLINE_OFF:
    // HTML 3.0
    case HtmlTokenId::BIGPRINT_ON:
    case HtmlTokenId::BIGPRINT_OFF:
    case HtmlTokenId::STRIKETHROUGH_ON:
    case HtmlTokenId::STRIKETHROUGH_OFF:
    case HtmlTokenId::SMALLPRINT_ON:
    case HtmlTokenId::SMALLPRINT_OFF:
    // figures
    case HtmlTokenId::FIGURE_ON:
    case HtmlTokenId::FIGURE_OFF:
    case HtmlTokenId::CAPTION_ON:
    case HtmlTokenId::CAPTION_OFF:
    case HtmlTokenId::CREDIT_ON:
    case HtmlTokenId::CREDIT_OFF:
    // misc
    case HtmlTokenId::DIRLIST_ON:
    case HtmlTokenId::DIRLIST_OFF:
    case HtmlTokenId::FOOTNOTE_ON:          //! they land so in the text
    case HtmlTokenId::FOOTNOTE_OFF:
    case HtmlTokenId::MENULIST_ON:
    case HtmlTokenId::MENULIST_OFF:
//  case HtmlTokenId::PLAINTEXT_ON:         //! special handling
//  case HtmlTokenId::PLAINTEXT_OFF:
//  case HtmlTokenId::PREFORMTXT_ON:        //! special handling
//  case HtmlTokenId::PREFORMTXT_OFF:
    case HtmlTokenId::SPAN_ON:
    case HtmlTokenId::SPAN_OFF:
    // obsolete
//  case HtmlTokenId::XMP_ON:               //! special handling
//  case HtmlTokenId::XMP_OFF:
//  case HtmlTokenId::LISTING_ON:           //! special handling
//  case HtmlTokenId::LISTING_OFF:
    // Netscape
    case HtmlTokenId::BLINK_ON:
    case HtmlTokenId::BLINK_OFF:
    case HtmlTokenId::NOBR_ON:
    case HtmlTokenId::NOBR_OFF:
    case HtmlTokenId::NOEMBED_ON:
    case HtmlTokenId::NOEMBED_OFF:
    case HtmlTokenId::NOFRAMES_ON:
    case HtmlTokenId::NOFRAMES_OFF:
    // Internet Explorer
    case HtmlTokenId::MARQUEE_ON:
    case HtmlTokenId::MARQUEE_OFF:
//  case HtmlTokenId::PLAINTEXT2_ON:        //! special handling
//  case HtmlTokenId::PLAINTEXT2_OFF:
    break;

    default:
    {
        if ( nToken >= HtmlTokenId::ONOFF_START )
        {
            if ( ( nToken == HtmlTokenId::UNKNOWNCONTROL_ON ) || ( nToken == HtmlTokenId::UNKNOWNCONTROL_OFF ) )
            {
                ;
            }
            else if ( !isOffToken(nToken) )
            {
                DBG_ASSERT( !isOffToken( nToken ), "No Start-Token ?!" );
                SkipGroup( static_cast<HtmlTokenId>(static_cast<int>(nToken) + 1) );
            }
        }
    }
    }   // SWITCH

    if (mpEditEngine->IsHtmlImportHandlerSet())
    {
        HtmlImportInfo aImportInfo(HtmlImportState::NextToken, this, mpEditEngine->CreateESelection(aCurSel));
        aImportInfo.nToken = nToken;
        if ( nToken == HtmlTokenId::TEXTTOKEN )
            aImportInfo.aText = aToken;
        else if (nToken == HtmlTokenId::STYLE_OFF)
            aImportInfo.aText = maStyleSource.makeStringAndClear();
        mpEditEngine->CallHtmlImportHandler(aImportInfo);
    }

}

void EditHTMLParser::ImpInsertParaBreak()
{
    if (mpEditEngine->IsHtmlImportHandlerSet())
    {
        HtmlImportInfo aImportInfo(HtmlImportState::InsertPara, this, mpEditEngine->CreateESelection(aCurSel));
        mpEditEngine->CallHtmlImportHandler(aImportInfo);
    }
    aCurSel = mpEditEngine->InsertParaBreak(aCurSel);
}

void EditHTMLParser::ImpSetAttribs( const SfxItemSet& rItems )
{
    // pSel, when character attributes, otherwise paragraph attributes for
    // the current paragraph.
    DBG_ASSERT( aCurSel.Min().GetNode() == aCurSel.Max().GetNode(), "ImpInsertAttribs: Selection?" );

    EditPaM aStartPaM( aCurSel.Min() );
    EditPaM aEndPaM( aCurSel.Max() );

    aStartPaM.SetIndex( 0 );
    aEndPaM.SetIndex( aEndPaM.GetNode()->Len() );

    if (mpEditEngine->IsHtmlImportHandlerSet())
    {
        EditSelection aSel( aStartPaM, aEndPaM );
        HtmlImportInfo aImportInfo(HtmlImportState::SetAttr, this, mpEditEngine->CreateESelection(aSel));
        mpEditEngine->CallHtmlImportHandler(aImportInfo);
    }

    ContentNode* pSN = aStartPaM.GetNode();
    sal_Int32 nStartNode = mpEditEngine->GetEditDoc().GetPos( pSN );

    // If an attribute goes from 0 to current Paragraph length,
    // then it should be a paragraph attribute!

    // Note: Selection can reach over several Paragraphs.
    // All complete paragraphs are paragraph attributes ...

    // not really HTML:
#ifdef DBG_UTIL
    ContentNode* pEN = aEndPaM.GetNode();
    sal_Int32 nEndNode = mpEditEngine->GetEditDoc().GetPos( pEN );
    DBG_ASSERT( nStartNode == nEndNode, "ImpSetAttribs: Several paragraphs?" );
#endif

    if ( ( aStartPaM.GetIndex() == 0 ) && ( aEndPaM.GetIndex() == aEndPaM.GetNode()->Len() ) )
    {
        // Has to be merged:
        SfxItemSet aItems = mpEditEngine->GetBaseParaAttribs(nStartNode);
        aItems.Put( rItems );
        mpEditEngine->SetParaAttribsOnly(nStartNode, aItems);
    }
    else
        mpEditEngine->SetAttribs( EditSelection( aStartPaM, aEndPaM ), rItems );
}

void EditHTMLParser::ImpSetStyleSheet( sal_uInt16 nHLevel )
{
    /*
        nHLevel:    0:          Turn off
                    1-6:        Heading
                    STYLE_PRE:  Preformatted
    */
    // Create hard attributes ...
    // Enough for Calc, would have to be clarified with StyleSheets
    // that they should also be in the app so that when they are feed
    // in a different engine still are here ...
    sal_Int32 nNode = mpEditEngine->GetEditDoc().GetPos( aCurSel.Max().GetNode() );

    SfxItemSet aItems( aCurSel.Max().GetNode()->GetContentAttribs().GetItems() );

    aItems.ClearItem( EE_PARA_ULSPACE );

    aItems.ClearItem( EE_CHAR_FONTHEIGHT );
    aItems.ClearItem( EE_CHAR_FONTINFO );
    aItems.ClearItem( EE_CHAR_WEIGHT );

    aItems.ClearItem( EE_CHAR_FONTHEIGHT_CJK );
    aItems.ClearItem( EE_CHAR_FONTINFO_CJK );
    aItems.ClearItem( EE_CHAR_WEIGHT_CJK );

    aItems.ClearItem( EE_CHAR_FONTHEIGHT_CTL );
    aItems.ClearItem( EE_CHAR_FONTINFO_CTL );
    aItems.ClearItem( EE_CHAR_WEIGHT_CTL );

    // Bold in the first 3 Headings
    if ( ( nHLevel >= 1 ) && ( nHLevel <= 3 ) )
    {
        SvxWeightItem aWeightItem( WEIGHT_BOLD, EE_CHAR_WEIGHT );
        aItems.Put( aWeightItem );

        SvxWeightItem aWeightItemCJK( WEIGHT_BOLD, EE_CHAR_WEIGHT_CJK );
        aItems.Put( aWeightItemCJK );

        SvxWeightItem aWeightItemCTL( WEIGHT_BOLD, EE_CHAR_WEIGHT_CTL );
        aItems.Put( aWeightItemCTL );
    }

    // Font height and margins, when LogicToLogic is possible:
    MapUnit eUnit = mpEditEngine->GetRefMapMode().GetMapUnit();
    if ( ( eUnit != MapUnit::MapPixel ) && ( eUnit != MapUnit::MapSysFont ) &&
         ( eUnit != MapUnit::MapAppFont ) && ( eUnit != MapUnit::MapRelative ) )
    {
        tools::Long nPoints = 10;
        if ( nHLevel == 1 )
            nPoints = 22;
        else if ( nHLevel == 2 )
            nPoints = 16;
        else if ( nHLevel == 3 )
            nPoints = 12;
        else if ( nHLevel == 4 )
            nPoints = 11;

        nPoints = OutputDevice::LogicToLogic( nPoints, MapUnit::MapPoint, eUnit );

        SvxFontHeightItem aHeightItem( nPoints, 100, EE_CHAR_FONTHEIGHT );
        aItems.Put( aHeightItem );

        SvxFontHeightItem aHeightItemCJK( nPoints, 100, EE_CHAR_FONTHEIGHT_CJK );
        aItems.Put( aHeightItemCJK );

        SvxFontHeightItem aHeightItemCTL( nPoints, 100, EE_CHAR_FONTHEIGHT_CTL );
        aItems.Put( aHeightItemCTL );

        // Paragraph margins, when Heading:
        if (nHLevel <= 6)
        {
            SvxULSpaceItem aULSpaceItem( EE_PARA_ULSPACE );
            aULSpaceItem.SetUpper( static_cast<sal_uInt16>(OutputDevice::LogicToLogic( 42, MapUnit::Map10thMM, eUnit )) );
            aULSpaceItem.SetLower( static_cast<sal_uInt16>(OutputDevice::LogicToLogic( 35, MapUnit::Map10thMM, eUnit )) );
            aItems.Put( aULSpaceItem );
        }
    }

    // Choose a proportional Font for Pre
    if ( nHLevel == STYLE_PRE )
    {
        vcl::Font aFont = OutputDevice::GetDefaultFont( DefaultFontType::FIXED, LANGUAGE_SYSTEM, GetDefaultFontFlags::NONE );
        SvxFontItem aFontItem( aFont.GetFamilyType(), aFont.GetFamilyName(), OUString(), aFont.GetPitch(), aFont.GetCharSet(), EE_CHAR_FONTINFO );
        aItems.Put( aFontItem );

        SvxFontItem aFontItemCJK( aFont.GetFamilyType(), aFont.GetFamilyName(), OUString(), aFont.GetPitch(), aFont.GetCharSet(), EE_CHAR_FONTINFO_CJK );
        aItems.Put( aFontItemCJK );

        SvxFontItem aFontItemCTL( aFont.GetFamilyType(), aFont.GetFamilyName(), OUString(), aFont.GetPitch(), aFont.GetCharSet(), EE_CHAR_FONTINFO_CTL );
        aItems.Put( aFontItemCTL );
    }

    mpEditEngine->SetParaAttribsOnly(nNode, aItems);
}

void EditHTMLParser::ImpInsertText( const OUString& rText )
{
    if (mpEditEngine->IsHtmlImportHandlerSet())
    {
        HtmlImportInfo aImportInfo(HtmlImportState::InsertText, this, mpEditEngine->CreateESelection(aCurSel));
        aImportInfo.aText = rText;
        mpEditEngine->CallHtmlImportHandler(aImportInfo);
    }

    aCurSel = mpEditEngine->InsertText(aCurSel, rText);
}

void EditHTMLParser::SkipGroup( HtmlTokenId nEndToken )
{
    // groups in cells are closed upon leaving the cell, because those
    // ******* web authors don't know their job
    // for example: <td><form></td>   lacks a closing </form>
    sal_uInt8 nCellLevel = nInCell;
    HtmlTokenId nToken;
    while( nCellLevel <= nInCell )
    {
        nToken = GetNextToken();
        if (nToken == nEndToken || nToken == HtmlTokenId::NONE)
            break;
        switch ( nToken )
        {
            case HtmlTokenId::TABLEHEADER_ON:
            case HtmlTokenId::TABLEDATA_ON:
                nInCell++;
            break;
            case HtmlTokenId::TABLEHEADER_OFF:
            case HtmlTokenId::TABLEDATA_OFF:
                if ( nInCell )
                    nInCell--;
            break;
            default: break;
        }
    }
}

void EditHTMLParser::StartPara( bool bReal )
{
    if ( bReal )
    {
        const HTMLOptions& aOptions = GetOptions();
        SvxAdjust eAdjust = SvxAdjust::Left;
        for (const auto & aOption : aOptions)
        {
            if( aOption.GetToken() == HtmlOptionId::ALIGN )
            {
                OUString const& rTmp(aOption.GetString());
                if (rTmp.equalsIgnoreAsciiCase(OOO_STRING_SVTOOLS_HTML_AL_right))
                    eAdjust = SvxAdjust::Right;
                else if (rTmp.equalsIgnoreAsciiCase(OOO_STRING_SVTOOLS_HTML_AL_middle))
                    eAdjust = SvxAdjust::Center;
                else if (rTmp.equalsIgnoreAsciiCase(OOO_STRING_SVTOOLS_HTML_AL_center))
                    eAdjust = SvxAdjust::Center;
                else
                    eAdjust = SvxAdjust::Left;
            }
        }
        SfxItemSet aItemSet = mpEditEngine->GetEmptyItemSet();
        aItemSet.Put( SvxAdjustItem( eAdjust, EE_PARA_JUST ) );
        ImpSetAttribs( aItemSet );
    }
    bInPara = true;
}

void EditHTMLParser::EndPara()
{
    if ( bInPara )
    {
        bool bHasText = HasTextInCurrentPara();
        if ( bHasText )
            ImpInsertParaBreak();
    }
    bInPara = false;
}

bool EditHTMLParser::ThrowAwayBlank()
{
    // A blank must be thrown away if the new text begins with a Blank and
    // if the current paragraph is empty or ends with a Blank...
    ContentNode* pNode = aCurSel.Max().GetNode();
    return !(pNode->Len() && ( pNode->GetChar( pNode->Len()-1 ) != ' ' ));
}

bool EditHTMLParser::HasTextInCurrentPara()
{
    return aCurSel.Max().GetNode()->Len() != 0;
}

void EditHTMLParser::AnchorStart()
{
    // ignore anchor in anchor
    if ( pCurAnchor )
        return;

    const HTMLOptions& aOptions = GetOptions();
    OUString aRef;

    for (const auto & aOption : aOptions)
    {
        if( aOption.GetToken() == HtmlOptionId::HREF)
            aRef = aOption.GetString();
    }

    if ( aRef.isEmpty() )
        return;

    OUString aURL = aRef;
    if ( !aURL.isEmpty() && ( aURL[ 0 ] != '#' ) )
    {
        INetURLObject aTargetURL;
        INetURLObject aRootURL( aBaseURL );
        aRootURL.GetNewAbsURL( aRef, &aTargetURL );
        aURL = aTargetURL.GetMainURL( INetURLObject::DecodeMechanism::ToIUri );
    }
    pCurAnchor.reset( new AnchorInfo );
    pCurAnchor->aHRef = aURL;
}

void EditHTMLParser::AnchorEnd()
{
    if ( !pCurAnchor )
        return;

    // Insert as URL-Field...
    SvxFieldItem aFld( SvxURLField( pCurAnchor->aHRef, pCurAnchor->aText, SvxURLFormat::Repr ), EE_FEATURE_FIELD  );
    aCurSel = mpEditEngine->InsertField(aCurSel, aFld);
    bFieldsInserted = true;
    pCurAnchor.reset();

    if (mpEditEngine->IsHtmlImportHandlerSet())
    {
        HtmlImportInfo aImportInfo(HtmlImportState::InsertField, this, mpEditEngine->CreateESelection(aCurSel));
        mpEditEngine->CallHtmlImportHandler(aImportInfo);
    }
}

void EditHTMLParser::HeadingStart( HtmlTokenId nToken )
{
    bWasInPara = bInPara;
    StartPara( false );

    if ( bWasInPara && HasTextInCurrentPara() )
        ImpInsertParaBreak();

    sal_uInt16 nId = sal::static_int_cast< sal_uInt16 >(
        1 + ( ( static_cast<int>(nToken) - int(HtmlTokenId::HEAD1_ON) ) / 2 ) );
    DBG_ASSERT( (nId >= 1) && (nId <= 9), "HeadingStart: ID can not be correct!" );
    ImpSetStyleSheet( nId );
}

void EditHTMLParser::HeadingEnd()
{
    EndPara();
    ImpSetStyleSheet( 0 );

    if ( bWasInPara )
    {
        bInPara = true;
        bWasInPara = false;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
