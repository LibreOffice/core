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

#include <eertfpar.hxx>
#include <impedit.hxx>
#include <svl/intitem.hxx>
#include <editeng/escpitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/flditem.hxx>

#include <svtools/rtftoken.h>

// Set all values to defaultt; is called after loading the bitmap!
void SvxRTFPictureType::ResetValues()
{   // Set all values RTF-Defaults
    eStyle = RTF_BITMAP;
    nMode = HEX_MODE;
    nType = nGoalWidth = nGoalHeight = 0;
    nWidth = nHeight = nWidthBytes = 0;
    uPicLen = 0;
    nBitsPerPixel = nPlanes = 1;
    nScalX = nScalY = 100;      // Scale in percent
    nCropT = nCropB = nCropL = nCropR = 0;
    aPropertyPairs.clear();
}

ImportInfo::ImportInfo( ImportState eSt, SvParser* pPrsrs, const ESelection& rSel )
    : aSelection( rSel )
{
    pParser     = pPrsrs,
    eState      = eSt;

    nToken      = 0;
    nTokenValue = 0;
    pAttrs      = NULL;
}

ImportInfo::~ImportInfo()
{
}

EditRTFParser::EditRTFParser( SvStream& rIn, EditSelection aSel, SfxItemPool& rAttrPool, ImpEditEngine* pImpEE  )
    : SvxRTFParser( rAttrPool, rIn, 0 ), aRTFMapMode( MAP_TWIP )
{

    pImpEditEngine  = pImpEE;
    aCurSel         = aSel;
    eDestCharSet    = RTL_TEXTENCODING_DONTKNOW;
    nDefFont        = 0;
    nDefTab         = 0;
    nLastAction     = 0;
    nDefFontHeight  = 0;

    SetInsPos( EditPosition( pImpEditEngine, &aCurSel ) );

    // Convert the twips values ...
    SetCalcValue( sal_True );
    SetChkStyleAttr( pImpEE->GetStatus().DoImportRTFStyleSheets() );
    SetNewDoc( sal_False );     // So that the Pool-Defaults are not overwritten...
    aEditMapMode = MapMode( pImpEE->GetRefDevice()->GetMapMode().GetMapUnit() );
}

EditRTFParser::~EditRTFParser()
{
}

SvParserState EditRTFParser::CallParser()
{
    DBG_ASSERT( !aCurSel.HasRange(), "Selection for CallParser!" );
    // Separate the part that is imported from the rest.
    // This expression should be used for all imports.
    // aStart1PaM: Last position before the imported content
    // aEnd1PaM: First position after the imported content
    // aStart2PaM: First position of the imported content
    // aEnd2PaM: Last position of the imported content
    EditPaM aStart1PaM( aCurSel.Min().GetNode(), aCurSel.Min().GetIndex() );
    aCurSel = pImpEditEngine->ImpInsertParaBreak( aCurSel );
    EditPaM aStart2PaM = aCurSel.Min();
    // Useful or not?
    aStart2PaM.GetNode()->GetContentAttribs().GetItems().ClearItem();
    AddRTFDefaultValues( aStart2PaM, aStart2PaM );
    EditPaM aEnd1PaM( pImpEditEngine->ImpInsertParaBreak( aCurSel.Max() ) );
    // aCurCel now points to the gap

    if ( pImpEditEngine->aImportHdl.IsSet() )
    {
        ImportInfo aImportInfo( RTFIMP_START, this, pImpEditEngine->CreateESel( aCurSel ) );
        pImpEditEngine->aImportHdl.Call( &aImportInfo );
    }

    SvParserState _eState = SvxRTFParser::CallParser();

    if ( pImpEditEngine->aImportHdl.IsSet() )
    {
        ImportInfo aImportInfo( RTFIMP_END, this, pImpEditEngine->CreateESel( aCurSel ) );
        pImpEditEngine->aImportHdl.Call( &aImportInfo );
    }

    if ( nLastAction == ACTION_INSERTPARABRK )
    {
        ContentNode* pCurNode = aCurSel.Max().GetNode();
        sal_uInt16 nPara = pImpEditEngine->GetEditDoc().GetPos( pCurNode );
        ContentNode* pPrevNode = pImpEditEngine->GetEditDoc().SaveGetObject( nPara-1 );
        DBG_ASSERT( pPrevNode, "Invalid RTF-Document?!" );
        EditSelection aSel;
        aSel.Min() = EditPaM( pPrevNode, pPrevNode->Len() );
        aSel.Max() = EditPaM( pCurNode, 0 );
        aCurSel.Max() = pImpEditEngine->ImpDeleteSelection( aSel );
    }
    EditPaM aEnd2PaM( aCurSel.Max() );
    //AddRTFDefaultValues( aStart2PaM, aEnd2PaM );
    sal_Bool bOnlyOnePara = ( aEnd2PaM.GetNode() == aStart2PaM.GetNode() );
    // Paste the chunk again ...
    // Problem: Paragraph attributes may not possibly be taken over
    // => Do Character attributes.

    sal_Bool bSpecialBackward = aStart1PaM.GetNode()->Len() ? sal_False : sal_True;
    if ( bOnlyOnePara || aStart1PaM.GetNode()->Len() )
        pImpEditEngine->ParaAttribsToCharAttribs( aStart2PaM.GetNode() );
    aCurSel.Min() = pImpEditEngine->ImpConnectParagraphs(
        aStart1PaM.GetNode(), aStart2PaM.GetNode(), bSpecialBackward );
    bSpecialBackward = aEnd1PaM.GetNode()->Len() ? sal_True : sal_False;
    // when bOnlyOnePara, then the node is gone on Connect.
    if ( !bOnlyOnePara && aEnd1PaM.GetNode()->Len() )
        pImpEditEngine->ParaAttribsToCharAttribs( aEnd2PaM.GetNode() );
    aCurSel.Max() = pImpEditEngine->ImpConnectParagraphs(
        ( bOnlyOnePara ? aStart1PaM.GetNode() : aEnd2PaM.GetNode() ),
            aEnd1PaM.GetNode(), bSpecialBackward );

    return _eState;
}

void EditRTFParser::AddRTFDefaultValues( const EditPaM& rStart, const EditPaM& rEnd )
{
    // Problem: DefFont and DefFontHeight
    Size aSz( 12, 0 );
    MapMode aPntMode( MAP_POINT );
    MapMode _aEditMapMode( pImpEditEngine->GetRefDevice()->GetMapMode().GetMapUnit() );
    aSz = pImpEditEngine->GetRefDevice()->LogicToLogic( aSz, &aPntMode, &_aEditMapMode );
    SvxFontHeightItem aFontHeightItem( aSz.Width(), 100, EE_CHAR_FONTHEIGHT );
    Font aDefFont( GetDefFont() );
    SvxFontItem aFontItem( aDefFont.GetFamily(), aDefFont.GetName(),
                    aDefFont.GetStyleName(), aDefFont.GetPitch(), aDefFont.GetCharSet(), EE_CHAR_FONTINFO );

    sal_uInt16 nStartPara = pImpEditEngine->GetEditDoc().GetPos( rStart.GetNode() );
    sal_uInt16 nEndPara = pImpEditEngine->GetEditDoc().GetPos( rEnd.GetNode() );
    for ( sal_uInt16 nPara = nStartPara; nPara <= nEndPara; nPara++ )
    {
        ContentNode* pNode = pImpEditEngine->GetEditDoc().SaveGetObject( nPara );
        DBG_ASSERT( pNode, "AddRTFDefaultValues - No paragraph?!" );
        if ( !pNode->GetContentAttribs().HasItem( EE_CHAR_FONTINFO ) )
            pNode->GetContentAttribs().GetItems().Put( aFontItem );
        if ( !pNode->GetContentAttribs().HasItem( EE_CHAR_FONTHEIGHT ) )
            pNode->GetContentAttribs().GetItems().Put( aFontHeightItem );
    }
}

void EditRTFParser::NextToken( int nToken )
{
    switch( nToken )
    {
        case RTF_DEFF:
        {
            nDefFont = sal_uInt16(nTokenValue);
        }
        break;
        case RTF_DEFTAB:
        {
            nDefTab = sal_uInt16(nTokenValue);
        }
        break;
        case RTF_CELL:
        {
            aCurSel = pImpEditEngine->ImpInsertParaBreak( aCurSel );
        }
        break;
        case RTF_LINE:
        {
            aCurSel = pImpEditEngine->InsertLineBreak( aCurSel );
        }
        break;
        case RTF_FIELD:
        {
            ReadField();
        }
        break;
        case RTF_PGDSCTBL: // #i29453# ignore \*\pgdsctbl destination
        case RTF_LISTTEXT:
        {
            SkipGroup();
        }
        break;
        default:
        {
            SvxRTFParser::NextToken( nToken );
            if ( nToken == RTF_STYLESHEET )
                CreateStyleSheets();
        }
        break;
    }
    if ( pImpEditEngine->aImportHdl.IsSet() )
    {
        ImportInfo aImportInfo( RTFIMP_NEXTTOKEN, this, pImpEditEngine->CreateESel( aCurSel ) );
        aImportInfo.nToken = nToken;
        aImportInfo.nTokenValue = short(nTokenValue);
        pImpEditEngine->aImportHdl.Call( &aImportInfo );
    }
}

void EditRTFParser::UnknownAttrToken( int nToken, SfxItemSet* )
{
    // for Tokens which are not evaluated in ReadAttr
    // Actually, only for Calc (RTFTokenHdl), so that RTF_INTBL
    if ( pImpEditEngine->aImportHdl.IsSet() )
    {
        ImportInfo aImportInfo( RTFIMP_UNKNOWNATTR, this, pImpEditEngine->CreateESel( aCurSel ) );
        aImportInfo.nToken = nToken;
        aImportInfo.nTokenValue = short(nTokenValue);
        pImpEditEngine->aImportHdl.Call( &aImportInfo );
    }
}

void EditRTFParser::InsertText()
{
    String aText( aToken );
    if ( pImpEditEngine->aImportHdl.IsSet() )
    {
        ImportInfo aImportInfo( RTFIMP_INSERTTEXT, this, pImpEditEngine->CreateESel( aCurSel ) );
        aImportInfo.aText = aText;
        pImpEditEngine->aImportHdl.Call( &aImportInfo );
    }
    aCurSel = pImpEditEngine->ImpInsertText( aCurSel, aText );
    nLastAction = ACTION_INSERTTEXT;
}

void EditRTFParser::InsertPara()
{
    if ( pImpEditEngine->aImportHdl.IsSet() )
    {
        ImportInfo aImportInfo( RTFIMP_INSERTPARA, this, pImpEditEngine->CreateESel( aCurSel ) );
        pImpEditEngine->aImportHdl.Call( &aImportInfo );
    }
    aCurSel = pImpEditEngine->ImpInsertParaBreak( aCurSel );
    nLastAction = ACTION_INSERTPARABRK;
}

void EditRTFParser::MovePos( int bForward )
{
    if( bForward )
        aCurSel = pImpEditEngine->CursorRight( aCurSel.Max(), ::com::sun::star::i18n::CharacterIteratorMode::SKIPCHARACTER );
    else
        aCurSel = pImpEditEngine->CursorLeft( aCurSel.Max(), ::com::sun::star::i18n::CharacterIteratorMode::SKIPCHARACTER );
}

void EditRTFParser::SetEndPrevPara( SvxNodeIdx*& rpNodePos,
                                    sal_uInt16& rCntPos )
{
    // The Intention is to: determine the current insert position of the
    //                      previous paragraph and set the end from this.
    //                      This "\pard" always apply on the right paragraph.

    ContentNode* pN = aCurSel.Max().GetNode();
    sal_uInt16 nCurPara = pImpEditEngine->GetEditDoc().GetPos( pN );
    DBG_ASSERT( nCurPara != 0, "Paragraph equal to 0: SetEnfPrevPara" );
    if ( nCurPara )
        nCurPara--;
    ContentNode* pPrevNode = pImpEditEngine->GetEditDoc().SaveGetObject( nCurPara );
    DBG_ASSERT( pPrevNode, "pPrevNode = 0!" );
    rpNodePos = new EditNodeIdx( pImpEditEngine, pPrevNode );
    rCntPos = pPrevNode->Len();
}

int EditRTFParser::IsEndPara( SvxNodeIdx* pNd, sal_uInt16 nCnt ) const
{
    return ( nCnt == ( ((EditNodeIdx*)pNd)->GetNode()->Len()) );
}

void EditRTFParser::SetAttrInDoc( SvxRTFItemStackType &rSet )
{
    ContentNode* pSttNode = ((EditNodeIdx&)rSet.GetSttNode()).GetNode();
    ContentNode* pEndNode = ((EditNodeIdx&)rSet.GetEndNode()).GetNode();

    EditPaM aStartPaM( pSttNode, rSet.GetSttCnt() );
    EditPaM aEndPaM( pEndNode, rSet.GetEndCnt() );

    // If possible adjust the Escapemant-Item:
    const SfxPoolItem* pItem;

    // #i66167# adapt font heights to destination MapUnit if necessary
    const MapUnit eDestUnit = ( MapUnit )( pImpEditEngine->GetEditDoc().GetItemPool().GetMetric(0) );
    const MapUnit eSrcUnit  = aRTFMapMode.GetMapUnit();
    if (eDestUnit != eSrcUnit)
    {
        sal_uInt16 aFntHeightIems[3] = { EE_CHAR_FONTHEIGHT, EE_CHAR_FONTHEIGHT_CJK, EE_CHAR_FONTHEIGHT_CTL };
        for (int i = 0; i < 2; ++i)
        {
            if (SFX_ITEM_SET == rSet.GetAttrSet().GetItemState( aFntHeightIems[i], sal_False, &pItem ))
            {
                sal_uInt32 nHeight  = ((SvxFontHeightItem*)pItem)->GetHeight();
                long nNewHeight;
                nNewHeight = pImpEditEngine->GetRefDevice()->LogicToLogic( (long)nHeight, eSrcUnit, eDestUnit );

                SvxFontHeightItem aFntHeightItem( nNewHeight, ((SvxFontHeightItem*)pItem)->GetProp(), aFntHeightIems[i] );
                rSet.GetAttrSet().Put( aFntHeightItem );
            }
        }
    }

    if( SFX_ITEM_SET == rSet.GetAttrSet().GetItemState( EE_CHAR_ESCAPEMENT, sal_False, &pItem ))
    {
        // die richtige
        long nEsc = ((SvxEscapementItem*)pItem)->GetEsc();

        if( ( DFLT_ESC_AUTO_SUPER != nEsc ) && ( DFLT_ESC_AUTO_SUB != nEsc ) )
        {
            nEsc *= 10; //HalPoints => Twips was embezzled in RTFITEM.CXX!
            SvxFont aFont;
            pImpEditEngine->SeekCursor( aStartPaM.GetNode(), aStartPaM.GetIndex()+1, aFont );
            nEsc = nEsc * 100 / aFont.GetSize().Height();

            SvxEscapementItem aEscItem( (short) nEsc, ((SvxEscapementItem*)pItem)->GetProp(), EE_CHAR_ESCAPEMENT );
            rSet.GetAttrSet().Put( aEscItem );
        }
    }

    if ( pImpEditEngine->aImportHdl.IsSet() )
    {
        EditSelection aSel( aStartPaM, aEndPaM );
        ImportInfo aImportInfo( RTFIMP_SETATTR, this, pImpEditEngine->CreateESel( aSel ) );
        aImportInfo.pAttrs = &rSet;
        pImpEditEngine->aImportHdl.Call( &aImportInfo );
    }

    ContentNode* pSN = aStartPaM.GetNode();
    ContentNode* pEN = aEndPaM.GetNode();
    sal_uInt16 nStartNode = pImpEditEngine->GetEditDoc().GetPos( pSN );
    sal_uInt16 nEndNode = pImpEditEngine->GetEditDoc().GetPos( pEN );
    sal_Int16 nOutlLevel = 0xff;

    if ( rSet.StyleNo() && pImpEditEngine->GetStyleSheetPool() && pImpEditEngine->GetStatus().DoImportRTFStyleSheets() )
    {
        SvxRTFStyleType* pS = GetStyleTbl().Get( rSet.StyleNo() );
        DBG_ASSERT( pS, "Template not defined in RTF!" );
        if ( pS )
        {
            pImpEditEngine->SetStyleSheet( EditSelection( aStartPaM, aEndPaM ), (SfxStyleSheet*)pImpEditEngine->GetStyleSheetPool()->Find( pS->sName, SFX_STYLE_FAMILY_ALL ) );
            nOutlLevel = pS->nOutlineNo;
        }
    }

    // When an Attribute goes from 0 to the current paragraph length,
    // it should be a paragraph attribute!

    // Note: Selection can reach over several paragraphs.
    // All Complete paragraphs are paragraph attributes ...
    for ( sal_uInt16 z = nStartNode+1; z < nEndNode; z++ )
    {
        DBG_ASSERT( pImpEditEngine->GetEditDoc().SaveGetObject( z ), "Node does not exist yet(RTF)" );
        pImpEditEngine->SetParaAttribs( z, rSet.GetAttrSet() );
    }

    if ( aStartPaM.GetNode() != aEndPaM.GetNode() )
    {
        // The rest dof the StartNodes...
        if ( aStartPaM.GetIndex() == 0 )
            pImpEditEngine->SetParaAttribs( nStartNode, rSet.GetAttrSet() );
        else
            pImpEditEngine->SetAttribs( EditSelection( aStartPaM, EditPaM( aStartPaM.GetNode(), aStartPaM.GetNode()->Len() ) ), rSet.GetAttrSet() );

        // the beginning of the EndNodes....
        if ( aEndPaM.GetIndex() == aEndPaM.GetNode()->Len() )
            pImpEditEngine->SetParaAttribs( nEndNode, rSet.GetAttrSet() );
        else
            pImpEditEngine->SetAttribs( EditSelection( EditPaM( aEndPaM.GetNode(), 0 ), aEndPaM ), rSet.GetAttrSet() );
    }
    else
    {
        if ( ( aStartPaM.GetIndex() == 0 ) && ( aEndPaM.GetIndex() == aEndPaM.GetNode()->Len() ) )
        {
            // When settings char attribs as para attribs, we must merge with existing attribs, not overwrite the ItemSet!
            SfxItemSet aAttrs = pImpEditEngine->GetParaAttribs( nStartNode );
            aAttrs.Put( rSet.GetAttrSet() );
            pImpEditEngine->SetParaAttribs( nStartNode, aAttrs );
        }
        else
        {
            pImpEditEngine->SetAttribs( EditSelection( aStartPaM, aEndPaM ), rSet.GetAttrSet() );
        }
    }

    // OutlLevel...
    if ( nOutlLevel != 0xff )
    {
        for ( sal_uInt16 n = nStartNode; n <= nEndNode; n++ )
        {
            ContentNode* pNode = pImpEditEngine->GetEditDoc().SaveGetObject( n );
            pNode->GetContentAttribs().GetItems().Put( SfxInt16Item( EE_PARA_OUTLLEVEL, nOutlLevel ) );
        }
    }
}

SvxRTFStyleType* EditRTFParser::FindStyleSheet( const XubString& rName )
{
    SvxRTFStyleType* pS = GetStyleTbl().First();
    while ( pS && ( pS->sName != rName ) )
        pS = GetStyleTbl().Next();

    return pS;
}

SfxStyleSheet* EditRTFParser::CreateStyleSheet( SvxRTFStyleType* pRTFStyle )
{
    // Check if a template exists, then it will not be changed!
    SfxStyleSheet* pStyle = (SfxStyleSheet*)pImpEditEngine->GetStyleSheetPool()->Find( pRTFStyle->sName, SFX_STYLE_FAMILY_ALL );
    if ( pStyle )
        return pStyle;

    String aName( pRTFStyle->sName );
    String aParent;
    if ( pRTFStyle->nBasedOn )
    {
        SvxRTFStyleType* pS = GetStyleTbl().Get( pRTFStyle->nBasedOn );
        if ( pS && ( pS !=pRTFStyle ) )
            aParent = pS->sName;
    }

    pStyle = (SfxStyleSheet*) &pImpEditEngine->GetStyleSheetPool()->Make( aName, SFX_STYLE_FAMILY_PARA );

    // 1) convert and take over Items ...
    ConvertAndPutItems( pStyle->GetItemSet(), pRTFStyle->aAttrSet );

    // 2) As long as Parent is not in the pool, also create this ...
    if ( aParent.Len() && ( aParent != aName ) )
    {
        SfxStyleSheet* pS = (SfxStyleSheet*)pImpEditEngine->GetStyleSheetPool()->Find( aParent, SFX_STYLE_FAMILY_ALL );
        if ( !pS )
        {
            // If not found anywhere, create from RTF ...
            SvxRTFStyleType* _pRTFStyle = FindStyleSheet( aParent );
            if ( _pRTFStyle )
                pS = CreateStyleSheet( _pRTFStyle );
        }
        // 2b) Link Itemset with Parent ...
        if ( pS )
            pStyle->GetItemSet().SetParent( &pS->GetItemSet() );
    }
    return pStyle;
}

void EditRTFParser::CreateStyleSheets()
{
    // the SvxRTFParser haa  now created the template...
    if ( pImpEditEngine->GetStyleSheetPool() && pImpEditEngine->GetStatus().DoImportRTFStyleSheets() )
    {
        SvxRTFStyleType* pRTFStyle = GetStyleTbl().First();
        while ( pRTFStyle )
        {
            CreateStyleSheet( pRTFStyle );

            pRTFStyle = GetStyleTbl().Next();
        }
    }
}

void EditRTFParser::CalcValue()
{
    const MapUnit eDestUnit = static_cast< MapUnit >( aEditMapMode.GetMapUnit() );
    const MapUnit eSrcUnit  = aRTFMapMode.GetMapUnit();
    if (eDestUnit != eSrcUnit)
        nTokenValue = OutputDevice::LogicToLogic( (long)nTokenValue, eSrcUnit, eDestUnit );
}

void EditRTFParser::ReadField()
{
    // From SwRTFParser::ReadField()
    int _nOpenBrakets = 1;      // the first was already detected earlier
    sal_Bool bFldInst = sal_False;
    sal_Bool bFldRslt = sal_False;
    String aFldInst;
    String aFldRslt;

    while( _nOpenBrakets && IsParserWorking() )
    {
        switch( GetNextToken() )
        {
            case '}':
            {
                _nOpenBrakets--;
                if ( _nOpenBrakets == 1 )
                {
                    bFldInst = sal_False;
                    bFldRslt = sal_False;
                }
            }
            break;

            case '{':           _nOpenBrakets++;
                                break;

            case RTF_FIELD:     SkipGroup();
                                break;

            case RTF_FLDINST:   bFldInst = sal_True;
                                break;

            case RTF_FLDRSLT:   bFldRslt = sal_True;
                                break;

            case RTF_TEXTTOKEN:
            {
                if ( bFldInst )
                    aFldInst += aToken;
                else if ( bFldRslt )
                    aFldRslt += aToken;
            }
            break;
        }
    }
    if ( aFldInst.Len() )
    {
        String aHyperLinkMarker( RTL_CONSTASCII_USTRINGPARAM( "HYPERLINK " ) );
        if ( aFldInst.CompareIgnoreCaseToAscii( aHyperLinkMarker, aHyperLinkMarker.Len() ) == COMPARE_EQUAL )
        {
            aFldInst.Erase( 0, aHyperLinkMarker.Len() );
            aFldInst.EraseLeadingChars();
            aFldInst.EraseTrailingChars();
            aFldInst.Erase( 0, 1 ); // "
            aFldInst.Erase( aFldInst.Len()-1, 1 );  // "

            if ( !aFldRslt.Len() )
                aFldRslt = aFldInst;

            SvxFieldItem aField( SvxURLField( aFldInst, aFldRslt, SVXURLFORMAT_REPR ), EE_FEATURE_FIELD  );
            aCurSel = pImpEditEngine->InsertField( aCurSel, aField );
            pImpEditEngine->UpdateFields();
            nLastAction = ACTION_INSERTTEXT;
        }
    }

    SkipToken( -1 );        // the closing brace is evaluated "above"
}

void EditRTFParser::SkipGroup()
{
    int _nOpenBrakets = 1;      // the first was already detected earlier

    while( _nOpenBrakets && IsParserWorking() )
    {
        switch( GetNextToken() )
        {
            case '}':
            {
                _nOpenBrakets--;
            }
            break;

            case '{':
            {
                _nOpenBrakets++;
            }
            break;
        }
    }

    SkipToken( -1 );        // the closing brace is evaluated "above"
}

sal_uLong EditNodeIdx::GetIdx() const
{
    return pImpEditEngine->GetEditDoc().GetPos( pNode );
}

SvxNodeIdx* EditNodeIdx::Clone() const
{
    return new EditNodeIdx( pImpEditEngine, pNode );
}

SvxPosition* EditPosition::Clone() const
{
    return new EditPosition( pImpEditEngine, pCurSel );
}

SvxNodeIdx* EditPosition::MakeNodeIdx() const
{
    return new EditNodeIdx( pImpEditEngine, pCurSel->Max().GetNode() );
}

sal_uLong EditPosition::GetNodeIdx() const
{
    ContentNode* pN = pCurSel->Max().GetNode();
    return pImpEditEngine->GetEditDoc().GetPos( pN );
}

sal_uInt16 EditPosition::GetCntIdx() const
{
    return pCurSel->Max().GetIndex();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
