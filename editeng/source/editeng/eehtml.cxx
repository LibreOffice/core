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


#include <vcl/wrkwin.hxx>
#include <vcl/dialog.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <eehtml.hxx>
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

#include "editeng/editeng.hxx"

#define STYLE_PRE               101

EditHTMLParser::EditHTMLParser( SvStream& rIn, const String& rBaseURL, SvKeyValueIterator* pHTTPHeaderAttrs )
    : HTMLParser( rIn, true ),
    aBaseURL( rBaseURL ),
    mpEditEngine(NULL),
    pCurAnchor(NULL),
    bInPara(false),
    bWasInPara(false),
    bFieldsInserted(false),
    bInTitle(false),
    nInTable(0),
    nInCell(0),
    nDefListLevel(0)
{
    DBG_ASSERT( RTL_TEXTENCODING_DONTKNOW == GetSrcEncoding( ), "EditHTMLParser::EditHTMLParser: Where does the encoding come from?" );
    DBG_ASSERT( !IsSwitchToUCS2(), "EditHTMLParser::::EditHTMLParser: Switch to UCS2?" );

    // Altough the real default encoding is ISO8859-1, we use MS-1252
    // als default encoding.
    SetSrcEncoding( GetExtendedCompatibilityTextEncoding(  RTL_TEXTENCODING_ISO_8859_1 ) );

    // If the file starts with a BOM, switch to UCS2.
    SetSwitchToUCS2( true );

    if ( pHTTPHeaderAttrs )
        SetEncodingByHTTPHeader( pHTTPHeaderAttrs );
}

EditHTMLParser::~EditHTMLParser()
{
    delete pCurAnchor;
}

SvParserState EditHTMLParser::CallParser(EditEngine* pEE, const EditPaM& rPaM)
{
    DBG_ASSERT(pEE, "CallParser: ImpEditEngine ?!");
    mpEditEngine = pEE;
    SvParserState _eState = SVPAR_NOTSTARTED;
    if ( mpEditEngine )
    {
        // Build in wrap mimic in RTF import?
        aCurSel = EditSelection( rPaM, rPaM );

        if (mpEditEngine->IsImportHandlerSet())
        {
            ImportInfo aImportInfo(HTMLIMP_START, this, mpEditEngine->CreateESelection(aCurSel));
            mpEditEngine->CallImportHandler(aImportInfo);
        }

        ImpSetStyleSheet( 0 );
        _eState = HTMLParser::CallParser();

        if (mpEditEngine->IsImportHandlerSet())
        {
            ImportInfo aImportInfo(HTMLIMP_END, this, mpEditEngine->CreateESelection(aCurSel));
            mpEditEngine->CallImportHandler(aImportInfo);
        }

        if ( bFieldsInserted )
            mpEditEngine->UpdateFieldsOnly();
    }
    return _eState;
}

void EditHTMLParser::NextToken( int nToken )
{
    #ifdef DBG_UTIL
        HTML_TOKEN_IDS xID = (HTML_TOKEN_IDS)nToken;
        (void)xID;
    #endif

    switch( nToken )
    {
    case HTML_META:
    {
        const HTMLOptions& aOptions = GetOptions();
        size_t nArrLen = aOptions.size();
        bool bEquiv = false;
        for ( size_t i = 0; i < nArrLen; i++ )
        {
            const HTMLOption& aOption = aOptions[i];
            switch( aOption.GetToken() )
            {
                case HTML_O_HTTPEQUIV:
                {
                    bEquiv = true;
                }
                break;
                case HTML_O_CONTENT:
                {
                    if ( bEquiv )
                    {
                        rtl_TextEncoding eEnc = GetEncodingByMIME( aOption.GetString() );
                        if ( eEnc != RTL_TEXTENCODING_DONTKNOW )
                            SetSrcEncoding( eEnc );
                    }
                }
                break;
            }
        }

    }
    break;
    case HTML_PLAINTEXT_ON:
    case HTML_PLAINTEXT2_ON:
        bInPara = true;
    break;
    case HTML_PLAINTEXT_OFF:
    case HTML_PLAINTEXT2_OFF:
        bInPara = false;
    break;

    case HTML_LINEBREAK:
    case HTML_NEWPARA:
    {
        if ( ( bInPara || nInTable ) &&
            ( ( nToken == HTML_LINEBREAK ) || HasTextInCurrentPara() ) )
        {
            ImpInsertParaBreak();
        }
    }
    break;
    case HTML_HORZRULE:
    {
        if ( HasTextInCurrentPara() )
            ImpInsertParaBreak();
        ImpInsertParaBreak();
    }
    case HTML_NONBREAKSPACE:
    {
        if ( bInPara )
        {
            ImpInsertText( String( RTL_CONSTASCII_USTRINGPARAM( " " ) ) );
        }
    }
    break;
    case HTML_RAWDATA:
        if (IsReadStyle() && !aToken.isEmpty())
        {
            // Each token represents a single line.
            maStyleSource.append(aToken);
            maStyleSource.append(sal_Unicode('\n'));
        }
    break;
    case HTML_TEXTTOKEN:
    {
        // #i110937# for <title> content, call aImportHdl (no SkipGroup), but don't insert the text into the EditEngine
        if (!bInTitle)
        {
            if ( !bInPara )
                StartPara( false );

            String aText = aToken;
            if ( aText.Len() && ( aText.GetChar( 0 ) == ' ' )
                    && ThrowAwayBlank() && !IsReadPRE() )
                aText.Erase( 0, 1 );

            if ( pCurAnchor )
            {
                pCurAnchor->aText += aText;
            }
            else
            {
                // Only written until HTML with 319?
                if ( IsReadPRE() )
                {
                    sal_uInt16 nTabPos = aText.Search( '\t', 0 );
                    while ( nTabPos != STRING_NOTFOUND )
                    {
                        aText.Erase( nTabPos, 1 );
                        aText.Insert( String( RTL_CONSTASCII_USTRINGPARAM( "        " ) ), nTabPos );
                        nTabPos = aText.Search( '\t', nTabPos+8 );
                    }
                }
                ImpInsertText( aText );
            }
        }
    }
    break;

    case HTML_CENTER_ON:
    case HTML_CENTER_OFF:
                            {
                                sal_Int32 nNode = mpEditEngine->GetEditDoc().GetPos( aCurSel.Max().GetNode() );
                                SfxItemSet aItems( aCurSel.Max().GetNode()->GetContentAttribs().GetItems() );
                                aItems.ClearItem( EE_PARA_JUST );
                                if ( nToken == HTML_CENTER_ON )
                                    aItems.Put( SvxAdjustItem( SVX_ADJUST_CENTER, EE_PARA_JUST ) );
                                mpEditEngine->SetParaAttribsOnly(nNode, aItems);
                            }
                            break;

    case HTML_ANCHOR_ON:    AnchorStart();
                            break;
    case HTML_ANCHOR_OFF:   AnchorEnd();
                            break;

    case HTML_PARABREAK_ON:
        if( bInPara && HasTextInCurrentPara() )
            EndPara( true );
        StartPara( true );
        break;

    case HTML_PARABREAK_OFF:
        if( bInPara )
            EndPara( true );
        break;

    case HTML_HEAD1_ON:
    case HTML_HEAD2_ON:
    case HTML_HEAD3_ON:
    case HTML_HEAD4_ON:
    case HTML_HEAD5_ON:
    case HTML_HEAD6_ON:
    {
        HeadingStart( nToken );
    }
    break;

    case HTML_HEAD1_OFF:
    case HTML_HEAD2_OFF:
    case HTML_HEAD3_OFF:
    case HTML_HEAD4_OFF:
    case HTML_HEAD5_OFF:
    case HTML_HEAD6_OFF:
    {
        HeadingEnd( nToken );
    }
    break;

    case HTML_PREFORMTXT_ON:
    case HTML_XMP_ON:
    case HTML_LISTING_ON:
    {
        StartPara( true );
        ImpSetStyleSheet( STYLE_PRE );
    }
    break;

    case HTML_DEFLIST_ON:
    {
        nDefListLevel++;
    }
    break;

    case HTML_DEFLIST_OFF:
    {
        if( nDefListLevel )
            nDefListLevel--;
    }
    break;

    case HTML_TABLE_ON:     nInTable++;
                            break;
    case HTML_TABLE_OFF:    DBG_ASSERT( nInTable, "Not in Table, but TABLE_OFF?" );
                            nInTable--;
                            break;

    case HTML_TABLEHEADER_ON:
    case HTML_TABLEDATA_ON:
        nInCell++;
    // fall through
    case HTML_BLOCKQUOTE_ON:
    case HTML_BLOCKQUOTE_OFF:
    case HTML_BLOCKQUOTE30_ON:
    case HTML_BLOCKQUOTE30_OFF:
    case HTML_LISTHEADER_ON:
    case HTML_LI_ON:
    case HTML_DD_ON:
    case HTML_DT_ON:
    case HTML_ORDERLIST_ON:
    case HTML_UNORDERLIST_ON:
    {
        bool bHasText = HasTextInCurrentPara();
        if ( bHasText )
            ImpInsertParaBreak();
        StartPara( false );
    }
    break;

    case HTML_TABLEHEADER_OFF:
    case HTML_TABLEDATA_OFF:
    {
        if ( nInCell )
            nInCell--;
    }
    // fall through
    case HTML_LISTHEADER_OFF:
    case HTML_LI_OFF:
    case HTML_DD_OFF:
    case HTML_DT_OFF:
    case HTML_ORDERLIST_OFF:
    case HTML_UNORDERLIST_OFF:  EndPara( false );
                                break;

    case HTML_TABLEROW_ON:
    case HTML_TABLEROW_OFF: // A RETURN only after a CELL, for Calc

    case HTML_COL_ON:
    case HTML_COLGROUP_ON:
    case HTML_COLGROUP_OFF: break;

    case HTML_FONT_ON:      // ...
                            break;
    case HTML_FONT_OFF:     // ...
                            break;

    case HTML_TITLE_ON:
        bInTitle = true;
        break;
    case HTML_TITLE_OFF:
        bInTitle = false;
        break;

    // globals
    case HTML_HTML_ON:
    case HTML_HTML_OFF:
    case HTML_STYLE_ON:
    case HTML_STYLE_OFF:
    case HTML_BODY_ON:
    case HTML_BODY_OFF:
    case HTML_HEAD_ON:
    case HTML_HEAD_OFF:
    case HTML_FORM_ON:
    case HTML_FORM_OFF:
    case HTML_THEAD_ON:
    case HTML_THEAD_OFF:
    case HTML_TBODY_ON:
    case HTML_TBODY_OFF:
    // inline elements, structural markup
    // HTML 3.0
    case HTML_BANNER_ON:
    case HTML_BANNER_OFF:
    case HTML_DIVISION_ON:
    case HTML_DIVISION_OFF:
//  case HTML_LISTHEADER_ON:        //! special handling
//  case HTML_LISTHEADER_OFF:
    case HTML_NOTE_ON:
    case HTML_NOTE_OFF:
    // inline elements, logical markup
    // HTML 2.0
    case HTML_ADDRESS_ON:
    case HTML_ADDRESS_OFF:
//  case HTML_BLOCKQUOTE_ON:        //! special handling
//  case HTML_BLOCKQUOTE_OFF:
    case HTML_CITIATION_ON:
    case HTML_CITIATION_OFF:
    case HTML_CODE_ON:
    case HTML_CODE_OFF:
    case HTML_DEFINSTANCE_ON:
    case HTML_DEFINSTANCE_OFF:
    case HTML_EMPHASIS_ON:
    case HTML_EMPHASIS_OFF:
    case HTML_KEYBOARD_ON:
    case HTML_KEYBOARD_OFF:
    case HTML_SAMPLE_ON:
    case HTML_SAMPLE_OFF:
    case HTML_STRIKE_ON:
    case HTML_STRIKE_OFF:
    case HTML_STRONG_ON:
    case HTML_STRONG_OFF:
    case HTML_VARIABLE_ON:
    case HTML_VARIABLE_OFF:
    // HTML 3.0
    case HTML_ABBREVIATION_ON:
    case HTML_ABBREVIATION_OFF:
    case HTML_ACRONYM_ON:
    case HTML_ACRONYM_OFF:
    case HTML_AUTHOR_ON:
    case HTML_AUTHOR_OFF:
//  case HTML_BLOCKQUOTE30_ON:      //! special handling
//  case HTML_BLOCKQUOTE30_OFF:
    case HTML_DELETEDTEXT_ON:
    case HTML_DELETEDTEXT_OFF:
    case HTML_INSERTEDTEXT_ON:
    case HTML_INSERTEDTEXT_OFF:
    case HTML_LANGUAGE_ON:
    case HTML_LANGUAGE_OFF:
    case HTML_PERSON_ON:
    case HTML_PERSON_OFF:
    case HTML_SHORTQUOTE_ON:
    case HTML_SHORTQUOTE_OFF:
    case HTML_SUBSCRIPT_ON:
    case HTML_SUBSCRIPT_OFF:
    case HTML_SUPERSCRIPT_ON:
    case HTML_SUPERSCRIPT_OFF:
    // inline elements, visual markup
    // HTML 2.0
    case HTML_BOLD_ON:
    case HTML_BOLD_OFF:
    case HTML_ITALIC_ON:
    case HTML_ITALIC_OFF:
    case HTML_TELETYPE_ON:
    case HTML_TELETYPE_OFF:
    case HTML_UNDERLINE_ON:
    case HTML_UNDERLINE_OFF:
    // HTML 3.0
    case HTML_BIGPRINT_ON:
    case HTML_BIGPRINT_OFF:
    case HTML_STRIKETHROUGH_ON:
    case HTML_STRIKETHROUGH_OFF:
    case HTML_SMALLPRINT_ON:
    case HTML_SMALLPRINT_OFF:
    // figures
    case HTML_FIGURE_ON:
    case HTML_FIGURE_OFF:
    case HTML_CAPTION_ON:
    case HTML_CAPTION_OFF:
    case HTML_CREDIT_ON:
    case HTML_CREDIT_OFF:
    // misc
    case HTML_DIRLIST_ON:
    case HTML_DIRLIST_OFF:
    case HTML_FOOTNOTE_ON:          //! land so im Text
    case HTML_FOOTNOTE_OFF:
    case HTML_MENULIST_ON:
    case HTML_MENULIST_OFF:
//  case HTML_PLAINTEXT_ON:         //! special handling
//  case HTML_PLAINTEXT_OFF:
//  case HTML_PREFORMTXT_ON:        //! special handling
//  case HTML_PREFORMTXT_OFF:
    case HTML_SPAN_ON:
    case HTML_SPAN_OFF:
    // obsolete
//  case HTML_XMP_ON:               //! special handling
//  case HTML_XMP_OFF:
//  case HTML_LISTING_ON:           //! special handling
//  case HTML_LISTING_OFF:
    // Netscape
    case HTML_BLINK_ON:
    case HTML_BLINK_OFF:
    case HTML_NOBR_ON:
    case HTML_NOBR_OFF:
    case HTML_NOEMBED_ON:
    case HTML_NOEMBED_OFF:
    case HTML_NOFRAMES_ON:
    case HTML_NOFRAMES_OFF:
    // Internet Explorer
    case HTML_MARQUEE_ON:
    case HTML_MARQUEE_OFF:
//  case HTML_PLAINTEXT2_ON:        //! special handling
//  case HTML_PLAINTEXT2_OFF:
    break;

    default:
    {
        if ( nToken & HTML_TOKEN_ONOFF )
        {
            if ( ( nToken == HTML_UNKNOWNCONTROL_ON ) || ( nToken == HTML_UNKNOWNCONTROL_OFF ) )
            {
                ;
            }
            else if ( !(nToken & 1) )
            {
                DBG_ASSERT( !( nToken & 1 ), "No Start-Token ?!" );
                SkipGroup( nToken + 1 );
            }
        }
    }
    }   // SWITCH

    if (mpEditEngine->IsImportHandlerSet())
    {
        ImportInfo aImportInfo(HTMLIMP_NEXTTOKEN, this, mpEditEngine->CreateESelection(aCurSel));
        aImportInfo.nToken = nToken;
        aImportInfo.nTokenValue = (short)nTokenValue;
        if ( nToken == HTML_TEXTTOKEN )
            aImportInfo.aText = aToken;
        else if (nToken == HTML_STYLE_OFF)
            aImportInfo.aText = maStyleSource.makeStringAndClear();
        mpEditEngine->CallImportHandler(aImportInfo);
    }

}

void EditHTMLParser::ImpInsertParaBreak()
{
    if (mpEditEngine->IsImportHandlerSet())
    {
        ImportInfo aImportInfo(HTMLIMP_INSERTPARA, this, mpEditEngine->CreateESelection(aCurSel));
        mpEditEngine->CallImportHandler(aImportInfo);
    }
    aCurSel = mpEditEngine->InsertParaBreak(aCurSel);
}

void EditHTMLParser::ImpSetAttribs( const SfxItemSet& rItems, EditSelection* pSel )
{
    // pSel, when character attributes, otherwise paragraph attributes for
    // the current paragraph.
    DBG_ASSERT( pSel || ( aCurSel.Min().GetNode() == aCurSel.Max().GetNode() ), "ImpInsertAttribs: Selection?" );

    EditPaM aStartPaM( pSel ? pSel->Min() : aCurSel.Min() );
    EditPaM aEndPaM( pSel ? pSel->Max() : aCurSel.Max() );

    if ( !pSel )
    {
        aStartPaM.SetIndex( 0 );
        aEndPaM.SetIndex( aEndPaM.GetNode()->Len() );
    }

    if (mpEditEngine->IsImportHandlerSet())
    {
        EditSelection aSel( aStartPaM, aEndPaM );
        ImportInfo aImportInfo(HTMLIMP_SETATTR, this, mpEditEngine->CreateESelection(aSel));
        aImportInfo.pAttrs = (void*)&rItems;
        mpEditEngine->CallImportHandler(aImportInfo);
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
        aItems.Put( aWeightItem );

        SvxWeightItem aWeightItemCTL( WEIGHT_BOLD, EE_CHAR_WEIGHT_CTL );
        aItems.Put( aWeightItem );
    }

    // Font hight and margins, when LogicToLogic is possible:
    MapUnit eUnit = mpEditEngine->GetRefMapMode().GetMapUnit();
    if ( ( eUnit != MAP_PIXEL ) && ( eUnit != MAP_SYSFONT ) &&
         ( eUnit != MAP_APPFONT ) && ( eUnit != MAP_RELATIVE ) )
    {
        long nPoints = 10;
        if ( nHLevel == 1 )
            nPoints = 22;
        else if ( nHLevel == 2 )
            nPoints = 16;
        else if ( nHLevel == 3 )
            nPoints = 12;
        else if ( nHLevel == 4 )
            nPoints = 11;

        nPoints = OutputDevice::LogicToLogic( nPoints, MAP_POINT, eUnit );

        SvxFontHeightItem aHeightItem( nPoints, 100, EE_CHAR_FONTHEIGHT );
        aItems.Put( aHeightItem );

        SvxFontHeightItem aHeightItemCJK( nPoints, 100, EE_CHAR_FONTHEIGHT_CJK );
        aItems.Put( aHeightItemCJK );

        SvxFontHeightItem aHeightItemCTL( nPoints, 100, EE_CHAR_FONTHEIGHT_CTL );
        aItems.Put( aHeightItemCTL );

        // Paragraph margins, when Heading:
        if ( !nHLevel || ((nHLevel >= 1) && (nHLevel <= 6)) )
        {
            SvxULSpaceItem aULSpaceItem( EE_PARA_ULSPACE );
            aULSpaceItem.SetUpper( (sal_uInt16)OutputDevice::LogicToLogic( 42, MAP_10TH_MM, eUnit ) );
            aULSpaceItem.SetLower( (sal_uInt16)OutputDevice::LogicToLogic( 35, MAP_10TH_MM, eUnit ) );
            aItems.Put( aULSpaceItem );
        }
    }

    // Choose a proportional Font for Pre
    if ( nHLevel == STYLE_PRE )
    {
        Font aFont = OutputDevice::GetDefaultFont( DEFAULTFONT_FIXED, LANGUAGE_SYSTEM, 0 );
        SvxFontItem aFontItem( aFont.GetFamily(), aFont.GetName(), OUString(), aFont.GetPitch(), aFont.GetCharSet(), EE_CHAR_FONTINFO );
        aItems.Put( aFontItem );

        SvxFontItem aFontItemCJK( aFont.GetFamily(), aFont.GetName(), OUString(), aFont.GetPitch(), aFont.GetCharSet(), EE_CHAR_FONTINFO_CJK );
        aItems.Put( aFontItemCJK );

        SvxFontItem aFontItemCTL( aFont.GetFamily(), aFont.GetName(), OUString(), aFont.GetPitch(), aFont.GetCharSet(), EE_CHAR_FONTINFO_CTL );
        aItems.Put( aFontItemCTL );
    }

    mpEditEngine->SetParaAttribsOnly(nNode, aItems);
}

void EditHTMLParser::ImpInsertText( const String& rText )
{
    String aText( rText );
    if (mpEditEngine->IsImportHandlerSet())
    {
        ImportInfo aImportInfo(HTMLIMP_INSERTTEXT, this, mpEditEngine->CreateESelection(aCurSel));
        aImportInfo.aText = aText;
        mpEditEngine->CallImportHandler(aImportInfo);
    }

    aCurSel = mpEditEngine->InsertText(aCurSel, aText);
}

void EditHTMLParser::SkipGroup( int nEndToken )
{
    // groups in cells are closed upon leaving the cell, because those
    // ******* web authors don't know their job
    // for example: <td><form></td>   lacks a closing </form>
    sal_uInt8 nCellLevel = nInCell;
    int nToken;
    while( nCellLevel <= nInCell && ( (nToken = GetNextToken() ) != nEndToken ) && nToken )
    {
        switch ( nToken )
        {
            case HTML_TABLEHEADER_ON:
            case HTML_TABLEDATA_ON:
                nInCell++;
            break;
            case HTML_TABLEHEADER_OFF:
            case HTML_TABLEDATA_OFF:
                if ( nInCell )
                    nInCell--;
            break;
        }
    }
}

void EditHTMLParser::StartPara( bool bReal )
{
    if ( bReal )
    {
        const HTMLOptions& aOptions = GetOptions();
        SvxAdjust eAdjust = SVX_ADJUST_LEFT;
        for ( size_t i = 0, n = aOptions.size(); i < n; ++i )
        {
            const HTMLOption& aOption = aOptions[i];
            switch( aOption.GetToken() )
            {
                case HTML_O_ALIGN:
                {
                    OUString const& rTmp(aOption.GetString());
                    if (rTmp.equalsIgnoreAsciiCase(OOO_STRING_SVTOOLS_HTML_AL_right))
                        eAdjust = SVX_ADJUST_RIGHT;
                    else if (rTmp.equalsIgnoreAsciiCase(OOO_STRING_SVTOOLS_HTML_AL_middle))
                        eAdjust = SVX_ADJUST_CENTER;
                    else if (rTmp.equalsIgnoreAsciiCase(OOO_STRING_SVTOOLS_HTML_AL_center))
                        eAdjust = SVX_ADJUST_CENTER;
                    else
                        eAdjust = SVX_ADJUST_LEFT;
                }
                break;
            }
        }
        SfxItemSet aItemSet = mpEditEngine->GetEmptyItemSet();
        aItemSet.Put( SvxAdjustItem( eAdjust, EE_PARA_JUST ) );
        ImpSetAttribs( aItemSet );
    }
    bInPara = true;
}

void EditHTMLParser::EndPara( bool )
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
    if ( pNode->Len() && ( pNode->GetChar( pNode->Len()-1 ) != ' ' ) )
        return false;
    return true;
}

bool EditHTMLParser::HasTextInCurrentPara()
{
    return aCurSel.Max().GetNode()->Len() ? true : false;
}

void EditHTMLParser::AnchorStart()
{
    // ignore anchor in anchor
    if ( !pCurAnchor )
    {
        const HTMLOptions& aOptions = GetOptions();
        String aRef;

        for ( size_t i = 0, n = aOptions.size(); i < n; ++i )
        {
            const HTMLOption& aOption = aOptions[i];
            switch( aOption.GetToken() )
            {
                case HTML_O_HREF:
                    aRef = aOption.GetString();
                break;
            }
        }

        if ( aRef.Len() )
        {
            String aURL = aRef;
            if ( aURL.Len() && ( aURL.GetChar( 0 ) != '#' ) )
            {
                INetURLObject aTargetURL;
                INetURLObject aRootURL( aBaseURL );
                aRootURL.GetNewAbsURL( aRef, &aTargetURL );
                aURL = aTargetURL.GetMainURL( INetURLObject::DECODE_TO_IURI );
            }
            pCurAnchor = new AnchorInfo;
            pCurAnchor->aHRef = aURL;
        }
    }
}

void EditHTMLParser::AnchorEnd()
{
    if ( pCurAnchor )
    {
        // Insert as URL-Field...
        SvxFieldItem aFld( SvxURLField( pCurAnchor->aHRef, pCurAnchor->aText, SVXURLFORMAT_REPR ), EE_FEATURE_FIELD  );
        aCurSel = mpEditEngine->InsertField(aCurSel, aFld);
        bFieldsInserted = true;
        delete pCurAnchor;
        pCurAnchor = NULL;

        if (mpEditEngine->IsImportHandlerSet())
        {
            ImportInfo aImportInfo(HTMLIMP_INSERTFIELD, this, mpEditEngine->CreateESelection(aCurSel));
            mpEditEngine->CallImportHandler(aImportInfo);
        }
    }
}

void EditHTMLParser::HeadingStart( int nToken )
{
    bWasInPara = bInPara;
    StartPara( false );

    if ( bWasInPara && HasTextInCurrentPara() )
        ImpInsertParaBreak();

    sal_uInt16 nId = sal::static_int_cast< sal_uInt16 >(
        1 + ( ( nToken - HTML_HEAD1_ON ) / 2 ) );
    DBG_ASSERT( (nId >= 1) && (nId <= 9), "HeadingStart: ID can not be correct!" );
    ImpSetStyleSheet( nId );
}

void EditHTMLParser::HeadingEnd( int )
{
    EndPara( false );
    ImpSetStyleSheet( 0 );

    if ( bWasInPara )
    {
        bInPara = true;
        bWasInPara = false;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
