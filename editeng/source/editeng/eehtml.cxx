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
#include <eehtml.hxx>
#include <impedit.hxx>
#include <editeng/adjitem.hxx>
#include <editeng/flditem.hxx>
#include <tools/urlobj.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/wghtitem.hxx>
#include <svtools/htmltokn.h>
#include <svtools/htmlkywd.hxx>
#include <tools/tenccvt.hxx>

#define ACTION_INSERTTEXT         1
#define ACTION_INSERTPARABRK      2

#define STYLE_PRE               101

EditHTMLParser::EditHTMLParser( SvStream& rIn, const String& rBaseURL, SvKeyValueIterator* pHTTPHeaderAttrs )
    : HTMLParser( rIn, true )
    , aBaseURL( rBaseURL )
{
    pImpEditEngine = 0;
    pCurAnchor = 0;
    bInPara = FALSE;
    bWasInPara = FALSE;
    nInTable = 0;
    nInCell = 0;
    bInTitle = FALSE;
    nDefListLevel = 0;
    nBulletLevel = 0;
    nNumberingLevel = 0;
    bFieldsInserted = FALSE;

    DBG_ASSERT( RTL_TEXTENCODING_DONTKNOW == GetSrcEncoding( ), "EditHTMLParser::EditHTMLParser: Where does the encoding come from?" );
    DBG_ASSERT( !IsSwitchToUCS2(), "EditHTMLParser::::EditHTMLParser: Switch to UCS2?" );

    // Altough the real default encoding is ISO8859-1, we use MS-1252
    // als default encoding.
    SetSrcEncoding( GetExtendedCompatibilityTextEncoding(  RTL_TEXTENCODING_ISO_8859_1 ) );

    // If the file starts with a BOM, switch to UCS2.
    SetSwitchToUCS2( TRUE );

    if ( pHTTPHeaderAttrs )
        SetEncodingByHTTPHeader( pHTTPHeaderAttrs );
}

EditHTMLParser::~EditHTMLParser()
{
    delete pCurAnchor;
}

SvParserState EditHTMLParser::CallParser( ImpEditEngine* pImpEE, const EditPaM& rPaM )
{
    DBG_ASSERT( pImpEE, "CallParser: ImpEditEngine ?!" );
    pImpEditEngine = pImpEE;
    SvParserState _eState = SVPAR_NOTSTARTED;
    if ( pImpEditEngine )
    {
        // Umbrechmimik vom RTF-Import einbauen?
        aCurSel = EditSelection( rPaM, rPaM );

        if ( pImpEditEngine->aImportHdl.IsSet() )
        {
            ImportInfo aImportInfo( HTMLIMP_START, this, pImpEditEngine->CreateESel( aCurSel ) );
            pImpEditEngine->aImportHdl.Call( &aImportInfo );
        }

        ImpSetStyleSheet( 0 );
        _eState = HTMLParser::CallParser();

        if ( pImpEditEngine->aImportHdl.IsSet() )
        {
            ImportInfo aImportInfo( HTMLIMP_END, this, pImpEditEngine->CreateESel( aCurSel ) );
            pImpEditEngine->aImportHdl.Call( &aImportInfo );
        }

        if ( bFieldsInserted )
            pImpEditEngine->UpdateFields();
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
        const HTMLOptions *_pOptions = GetOptions();
        USHORT nArrLen = _pOptions->Count();
        BOOL bEquiv = FALSE;
        for ( USHORT i = 0; i < nArrLen; i++ )
        {
            const HTMLOption *pOption = (*_pOptions)[i];
            switch( pOption->GetToken() )
            {
                case HTML_O_HTTPEQUIV:
                {
                    bEquiv = TRUE;
                }
                break;
                case HTML_O_CONTENT:
                {
                    if ( bEquiv )
                    {
                        rtl_TextEncoding eEnc = GetEncodingByMIME( pOption->GetString() );
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
        bInPara = TRUE;
    break;
    case HTML_PLAINTEXT_OFF:
    case HTML_PLAINTEXT2_OFF:
        bInPara = FALSE;
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
    case HTML_TEXTTOKEN:
    {
        // #i110937# for <title> content, call aImportHdl (no SkipGroup), but don't insert the text into the EditEngine
        if (!bInTitle)
        {
            if ( !bInPara )
                StartPara( FALSE );

            // if ( bInPara || pCurAnchor )

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
                // Nur bis HTML mit 319 geschrieben ?!
                if ( IsReadPRE() )
                {
                    USHORT nTabPos = aText.Search( '\t', 0 );
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
                                USHORT nNode = pImpEditEngine->GetEditDoc().GetPos( aCurSel.Max().GetNode() );
                                SfxItemSet aItems( aCurSel.Max().GetNode()->GetContentAttribs().GetItems() );
                                aItems.ClearItem( EE_PARA_JUST );
                                if ( nToken == HTML_CENTER_ON )
                                    aItems.Put( SvxAdjustItem( SVX_ADJUST_CENTER, EE_PARA_JUST ) );
                                pImpEditEngine->SetParaAttribs( nNode, aItems );
                            }
                            break;

    case HTML_ANCHOR_ON:    AnchorStart();
                            break;
    case HTML_ANCHOR_OFF:   AnchorEnd();
                            break;

    case HTML_PARABREAK_ON:
        if( bInPara && HasTextInCurrentPara() )
            EndPara( TRUE );
        StartPara( TRUE );
        break;

    case HTML_PARABREAK_OFF:
        if( bInPara )
            EndPara( TRUE );
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
        StartPara( TRUE );
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
    case HTML_TABLE_OFF:    DBG_ASSERT( nInTable, "Nicht in Table, aber TABLE_OFF?" );
                            nInTable--;
                            break;

    case HTML_TABLEHEADER_ON:
    case HTML_TABLEDATA_ON:
        nInCell++;
    // fallthru
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
        BOOL bHasText = HasTextInCurrentPara();
        if ( bHasText )
            ImpInsertParaBreak();
        StartPara( FALSE );
    }
    break;

    case HTML_TABLEHEADER_OFF:
    case HTML_TABLEDATA_OFF:
    {
        if ( nInCell )
            nInCell--;
    }
    // fallthru
    case HTML_LISTHEADER_OFF:
    case HTML_LI_OFF:
    case HTML_DD_OFF:
    case HTML_DT_OFF:
    case HTML_ORDERLIST_OFF:
    case HTML_UNORDERLIST_OFF:  EndPara( FALSE );
                                break;

    case HTML_TABLEROW_ON:
    case HTML_TABLEROW_OFF: // Nur nach einem CELL ein RETURN, fuer Calc

    case HTML_COL_ON:
    case HTML_COLGROUP_ON:
    case HTML_COLGROUP_OFF: break;

    case HTML_FONT_ON:      // ...
                            break;
    case HTML_FONT_OFF:     // ...
                            break;


    // #58335# kein SkipGroup on/off auf inline markup etc.

    case HTML_TITLE_ON:
        bInTitle = TRUE;
        break;
    case HTML_TITLE_OFF:
        bInTitle = FALSE;
        break;

    // globals
    case HTML_HTML_ON:
    case HTML_HTML_OFF:
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
//  case HTML_BLOCKQUOTE_ON:        //! extra Behandlung
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
//  case HTML_BLOCKQUOTE30_ON:      //! extra Behandlung
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
    case HTML_FOOTNOTE_ON:          //! landen so im Text
    case HTML_FOOTNOTE_OFF:
    case HTML_MENULIST_ON:
    case HTML_MENULIST_OFF:
//  case HTML_PLAINTEXT_ON:         //! extra Behandlung
//  case HTML_PLAINTEXT_OFF:
//  case HTML_PREFORMTXT_ON:        //! extra Behandlung
//  case HTML_PREFORMTXT_OFF:
    case HTML_SPAN_ON:
    case HTML_SPAN_OFF:
    // obsolete
//  case HTML_XMP_ON:               //! extra Behandlung
//  case HTML_XMP_OFF:
//  case HTML_LISTING_ON:           //! extra Behandlung
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
//  case HTML_PLAINTEXT2_ON:        //! extra Behandlung
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
                DBG_ASSERT( !( nToken & 1 ), "Kein Start-Token ?!" );
                SkipGroup( nToken + 1 );
            }
        }
    }
    }   // SWITCH

    if ( pImpEditEngine->aImportHdl.IsSet() )
    {
        ImportInfo aImportInfo( HTMLIMP_NEXTTOKEN, this, pImpEditEngine->CreateESel( aCurSel ) );
        aImportInfo.nToken = nToken;
        aImportInfo.nTokenValue = (short)nTokenValue;
        if ( nToken == HTML_TEXTTOKEN )
            aImportInfo.aText = aToken;
        pImpEditEngine->aImportHdl.Call( &aImportInfo );
    }

}

void EditHTMLParser::ImpInsertParaBreak()
{
    if ( pImpEditEngine->aImportHdl.IsSet() )
    {
        ImportInfo aImportInfo( HTMLIMP_INSERTPARA, this, pImpEditEngine->CreateESel( aCurSel ) );
        pImpEditEngine->aImportHdl.Call( &aImportInfo );
    }
    aCurSel = pImpEditEngine->ImpInsertParaBreak( aCurSel );
    nLastAction = ACTION_INSERTPARABRK;
}

void EditHTMLParser::ImpSetAttribs( const SfxItemSet& rItems, EditSelection* pSel )
{
    // pSel, wenn Zeichenattribute, sonst Absatzattribute fuer den
    // aktuellen Absatz.
    DBG_ASSERT( pSel || ( aCurSel.Min().GetNode() == aCurSel.Max().GetNode() ), "ImpInsertAttribs: Selektion?" );

    EditPaM aStartPaM( pSel ? pSel->Min() : aCurSel.Min() );
    EditPaM aEndPaM( pSel ? pSel->Max() : aCurSel.Max() );

    if ( !pSel )
    {
        aStartPaM.SetIndex( 0 );
        aEndPaM.SetIndex( aEndPaM.GetNode()->Len() );
    }

    if ( pImpEditEngine->aImportHdl.IsSet() )
    {
        EditSelection aSel( aStartPaM, aEndPaM );
        ImportInfo aImportInfo( HTMLIMP_SETATTR, this, pImpEditEngine->CreateESel( aSel ) );
        aImportInfo.pAttrs = (void*)&rItems;
        pImpEditEngine->aImportHdl.Call( &aImportInfo );
    }

    ContentNode* pSN = aStartPaM.GetNode();
    USHORT nStartNode = pImpEditEngine->GetEditDoc().GetPos( pSN );

    // Wenn ein Attribut von 0 bis aktuelle Absatzlaenge geht,
    // soll es ein Absatz-Attribut sein!

    // Achtung: Selektion kann ueber mehrere Absaetze gehen.
    // Alle vollstaendigen Absaetze sind Absatzattribute...

    // HTML eigentlich nicht:
#ifdef DBG_UTIL
    ContentNode* pEN = aEndPaM.GetNode();
    USHORT nEndNode = pImpEditEngine->GetEditDoc().GetPos( pEN );
    DBG_ASSERT( nStartNode == nEndNode, "ImpSetAttribs: Mehrere Absaetze?" );
#endif
    if ( ( aStartPaM.GetIndex() == 0 ) && ( aEndPaM.GetIndex() == aEndPaM.GetNode()->Len() ) )
    {
        // Muesse gemergt werden:
        SfxItemSet aItems( pImpEditEngine->GetParaAttribs( nStartNode ) );
        aItems.Put( rItems );
        pImpEditEngine->SetParaAttribs( nStartNode, aItems );
    }
    else
        pImpEditEngine->SetAttribs( EditSelection( aStartPaM, aEndPaM ), rItems );
}

void EditHTMLParser::ImpSetStyleSheet( USHORT nHLevel )
{
    /*
    nHLevel:    0:          Ausschalten
              1-6:      Heading
        STYLE_PRE:  Preformatted
    */
    // Harte Attribute erzeugen...
    // Reicht fuer Calc, bei StyleSheets muesste noch geklaert werden,
    // dass diese auch in der App liegen sollten, damit sie beim
    // fuettern in eine andere Engine auch noch da sind...

    USHORT nNode = pImpEditEngine->GetEditDoc().GetPos( aCurSel.Max().GetNode() );

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

    // Fett in den ersten 3 Headings
    if ( ( nHLevel >= 1 ) && ( nHLevel <= 3 ) )
    {
        SvxWeightItem aWeightItem( WEIGHT_BOLD, EE_CHAR_WEIGHT );
        aItems.Put( aWeightItem );

        SvxWeightItem aWeightItemCJK( WEIGHT_BOLD, EE_CHAR_WEIGHT_CJK );
        aItems.Put( aWeightItem );

        SvxWeightItem aWeightItemCTL( WEIGHT_BOLD, EE_CHAR_WEIGHT_CTL );
        aItems.Put( aWeightItem );
    }

    // Fonthoehe und Abstaende, wenn LogicToLogic moeglich:
    MapUnit eUnit = pImpEditEngine->GetRefMapMode().GetMapUnit();
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

        // Absatzabstaende, wenn Heading:
        if ( !nHLevel || ((nHLevel >= 1) && (nHLevel <= 6)) )
        {
            SvxULSpaceItem aULSpaceItem( EE_PARA_ULSPACE );
            aULSpaceItem.SetUpper( (USHORT)OutputDevice::LogicToLogic( 42, MAP_10TH_MM, eUnit ) );
            aULSpaceItem.SetLower( (USHORT)OutputDevice::LogicToLogic( 35, MAP_10TH_MM, eUnit ) );
            aItems.Put( aULSpaceItem );
        }
    }

    // Bei Pre einen proportionalen Font waehlen
    if ( nHLevel == STYLE_PRE )
    {
        Font aFont = OutputDevice::GetDefaultFont( DEFAULTFONT_FIXED, LANGUAGE_SYSTEM, 0 );
        SvxFontItem aFontItem( aFont.GetFamily(), aFont.GetName(), XubString(), aFont.GetPitch(), aFont.GetCharSet(), EE_CHAR_FONTINFO );
        aItems.Put( aFontItem );

        SvxFontItem aFontItemCJK( aFont.GetFamily(), aFont.GetName(), XubString(), aFont.GetPitch(), aFont.GetCharSet(), EE_CHAR_FONTINFO_CJK );
        aItems.Put( aFontItemCJK );

        SvxFontItem aFontItemCTL( aFont.GetFamily(), aFont.GetName(), XubString(), aFont.GetPitch(), aFont.GetCharSet(), EE_CHAR_FONTINFO_CTL );
        aItems.Put( aFontItemCTL );
    }

    pImpEditEngine->SetParaAttribs( nNode, aItems );
}

void EditHTMLParser::ImpInsertText( const String& rText )
{
    String aText( rText );
    if ( pImpEditEngine->aImportHdl.IsSet() )
    {
        ImportInfo aImportInfo( HTMLIMP_INSERTTEXT, this, pImpEditEngine->CreateESel( aCurSel ) );
        aImportInfo.aText = aText;
        pImpEditEngine->aImportHdl.Call( &aImportInfo );
    }

    aCurSel = pImpEditEngine->ImpInsertText( aCurSel, aText );
    nLastAction = ACTION_INSERTTEXT;
}

void EditHTMLParser::SkipGroup( int nEndToken )
{
    // #69109# groups in cells are closed upon leaving the cell, because those
    // ******* web authors don't know their job
    // for example: <td><form></td>   lacks a closing </form>
    BYTE nCellLevel = nInCell;
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

void EditHTMLParser::StartPara( BOOL bReal )
{
    if ( bReal )
    {
        const HTMLOptions *_pOptions = GetOptions();
        USHORT nArrLen = _pOptions->Count();
        SvxAdjust eAdjust = SVX_ADJUST_LEFT;
        for ( USHORT i = 0; i < nArrLen; i++ )
        {
            const HTMLOption *pOption = (*_pOptions)[i];
            switch( pOption->GetToken() )
            {
                case HTML_O_ALIGN:
                {
                    if ( pOption->GetString().CompareIgnoreCaseToAscii( OOO_STRING_SVTOOLS_HTML_AL_right ) == COMPARE_EQUAL )
                        eAdjust = SVX_ADJUST_RIGHT;
                    else if ( pOption->GetString().CompareIgnoreCaseToAscii( OOO_STRING_SVTOOLS_HTML_AL_middle ) == COMPARE_EQUAL )
                        eAdjust = SVX_ADJUST_CENTER;
                    else if ( pOption->GetString().CompareIgnoreCaseToAscii( OOO_STRING_SVTOOLS_HTML_AL_center ) == COMPARE_EQUAL )
                        eAdjust = SVX_ADJUST_CENTER;
                    else
                        eAdjust = SVX_ADJUST_LEFT;
                }
                break;
            }
        }
        SfxItemSet aItemSet( pImpEditEngine->GetEmptyItemSet() );
        aItemSet.Put( SvxAdjustItem( eAdjust, EE_PARA_JUST ) );
        ImpSetAttribs( aItemSet );
    }
    bInPara = TRUE;
}

void EditHTMLParser::EndPara( BOOL )
{
    if ( bInPara )
    {
        BOOL bHasText = HasTextInCurrentPara();
        if ( bHasText )
            ImpInsertParaBreak();
        // Nur, wenn ohne Absatzabstaende gearbeitet wird...
//      if ( !nInTable && bReal && (nNumberingLevel<=1) && (nBulletLevel<=1) )
//          ImpInsertParaBreak();
    }
    bInPara = FALSE;
}

BOOL EditHTMLParser::ThrowAwayBlank()
{
    // Ein Blank muss weggeschmissen werden, wenn der neue Text mit einem
    // Blank beginnt und der aktuelle Absatz leer ist oder mit einem
    // Blank endet...
    ContentNode* pNode = aCurSel.Max().GetNode();
    if ( pNode->Len() && ( pNode->GetChar( pNode->Len()-1 ) != ' ' ) )
        return FALSE;
    return TRUE;
}

BOOL EditHTMLParser::HasTextInCurrentPara()
{
    return aCurSel.Max().GetNode()->Len() ? TRUE : FALSE;
}

void EditHTMLParser::AnchorStart()
{
    // Anker im Anker ignoriern
    if ( !pCurAnchor )
    {
        const HTMLOptions* _pOptions = GetOptions();
        USHORT nArrLen = _pOptions->Count();

        String aRef;

        for ( USHORT i = 0; i < nArrLen; i++ )
        {
            const HTMLOption* pOption = (*_pOptions)[i];
            switch( pOption->GetToken() )
            {
                case HTML_O_HREF:
                    aRef = pOption->GetString();
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
        // Als URL-Feld einfuegen...
        SvxFieldItem aFld( SvxURLField( pCurAnchor->aHRef, pCurAnchor->aText, SVXURLFORMAT_REPR ), EE_FEATURE_FIELD  );
        aCurSel = pImpEditEngine->InsertField( aCurSel, aFld );
        bFieldsInserted = TRUE;
        delete pCurAnchor;
        pCurAnchor = 0;

        if ( pImpEditEngine->aImportHdl.IsSet() )
        {
            ImportInfo aImportInfo( HTMLIMP_INSERTFIELD, this, pImpEditEngine->CreateESel( aCurSel ) );
            pImpEditEngine->aImportHdl.Call( &aImportInfo );
        }
    }
}

void EditHTMLParser::HeadingStart( int nToken )
{
    bWasInPara = bInPara;
    StartPara( FALSE );

    if ( bWasInPara && HasTextInCurrentPara() )
        ImpInsertParaBreak();

    USHORT nId = sal::static_int_cast< USHORT >(
        1 + ( ( nToken - HTML_HEAD1_ON ) / 2 ) );
    DBG_ASSERT( (nId >= 1) && (nId <= 9), "HeadingStart: ID kann nicht stimmen!" );
    ImpSetStyleSheet( nId );
}

void EditHTMLParser::HeadingEnd( int )
{
    EndPara( FALSE );
    ImpSetStyleSheet( 0 );

    if ( bWasInPara )
    {
        bInPara = TRUE;
        bWasInPara = FALSE;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
