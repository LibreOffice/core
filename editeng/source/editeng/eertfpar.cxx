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

#include <comphelper/string.hxx>
#include <utility>

#include "eertfpar.hxx"
#include "impedit.hxx"
#include <svl/intitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/editeng.hxx>

#include <svtools/rtftoken.h>
#include <svtools/htmltokn.h>

using namespace com::sun::star;

HtmlImportInfo::HtmlImportInfo( HtmlImportState eSt, SvParser<HtmlTokenId>* pPrsrs, const ESelection& rSel )
    : aSelection( rSel )
{
    pParser     = pPrsrs;
    eState      = eSt;
    nToken      = HtmlTokenId::NONE;
}

HtmlImportInfo::~HtmlImportInfo()
{
}

RtfImportInfo::RtfImportInfo( RtfImportState eSt, SvParser<int>* pPrsrs, const ESelection& rSel )
    : aSelection( rSel )
{
    pParser     = pPrsrs;
    eState      = eSt;
    nToken      = 0;
    nTokenValue = 0;
}

RtfImportInfo::~RtfImportInfo()
{
}

static constexpr MapUnit gRTFMapUnit = MapUnit::MapTwip;

EditRTFParser::EditRTFParser(
    SvStream& rIn, EditSelection aSel, SfxItemPool& rAttrPool, EditEngine* pEditEngine) :
    SvxRTFParser(rAttrPool, rIn),
    aCurSel(std::move(aSel)),
    mpEditEngine(pEditEngine),
    nDefFont(0),
    bLastActionInsertParaBreak(false)
{
    SetInsPos(EditPosition(mpEditEngine, &aCurSel));

    // Convert the twips values ...
    SetCalcValue(true);
    SetChkStyleAttr(mpEditEngine->IsImportRTFStyleSheetsSet());
    SetNewDoc(false);     // So that the Pool-Defaults are not overwritten...
    aEditMapMode = MapMode(mpEditEngine->GetRefDevice()->GetMapMode().GetMapUnit());
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
    aCurSel = mpEditEngine->InsertParaBreak(aCurSel);
    EditPaM aStart2PaM = aCurSel.Min();
    // Useful or not?
    aStart2PaM.GetNode()->GetContentAttribs().GetItems().ClearItem();
    AddRTFDefaultValues( aStart2PaM, aStart2PaM );
    EditPaM aEnd1PaM = mpEditEngine->InsertParaBreak(aCurSel.Max());
    // aCurCel now points to the gap

    if (mpEditEngine->IsRtfImportHandlerSet())
    {
        RtfImportInfo aImportInfo(RtfImportState::Start, this, mpEditEngine->CreateESelection(aCurSel));
        mpEditEngine->CallRtfImportHandler(aImportInfo);
    }

    SvParserState _eState = SvxRTFParser::CallParser();

    if (mpEditEngine->IsRtfImportHandlerSet())
    {
        RtfImportInfo aImportInfo(RtfImportState::End, this, mpEditEngine->CreateESelection(aCurSel));
        mpEditEngine->CallRtfImportHandler(aImportInfo);
    }

    if (bLastActionInsertParaBreak)
    {
        ContentNode* pCurNode = aCurSel.Max().GetNode();
        sal_Int32 nPara = mpEditEngine->GetEditDoc().GetPos(pCurNode);
        ContentNode* pPrevNode = mpEditEngine->GetEditDoc().GetObject(nPara-1);
        DBG_ASSERT( pPrevNode, "Invalid RTF-Document?!" );
        EditSelection aSel;
        aSel.Min() = EditPaM( pPrevNode, pPrevNode->Len() );
        aSel.Max() = EditPaM( pCurNode, 0 );
        aCurSel.Max() = mpEditEngine->DeleteSelection(aSel);
    }
    EditPaM aEnd2PaM( aCurSel.Max() );
    //AddRTFDefaultValues( aStart2PaM, aEnd2PaM );
    bool bOnlyOnePara = ( aEnd2PaM.GetNode() == aStart2PaM.GetNode() );
    // Paste the chunk again ...
    // Problem: Paragraph attributes may not possibly be taken over
    // => Do Character attributes.

    bool bSpecialBackward = aStart1PaM.GetNode()->Len() == 0;
    if ( bOnlyOnePara || aStart1PaM.GetNode()->Len() )
        mpEditEngine->ParaAttribsToCharAttribs( aStart2PaM.GetNode() );
    aCurSel.Min() = mpEditEngine->ConnectParagraphs(
        aStart1PaM.GetNode(), aStart2PaM.GetNode(), bSpecialBackward );
    bSpecialBackward = aEnd1PaM.GetNode()->Len() != 0;
    // when bOnlyOnePara, then the node is gone on Connect.
    if ( !bOnlyOnePara && aEnd1PaM.GetNode()->Len() )
        mpEditEngine->ParaAttribsToCharAttribs( aEnd2PaM.GetNode() );
    aCurSel.Max() = mpEditEngine->ConnectParagraphs(
        ( bOnlyOnePara ? aStart1PaM.GetNode() : aEnd2PaM.GetNode() ),
            aEnd1PaM.GetNode(), bSpecialBackward );

    return _eState;
}

void EditRTFParser::AddRTFDefaultValues( const EditPaM& rStart, const EditPaM& rEnd )
{
    // Problem: DefFont and DefFontHeight
    Size aSz( 12, 0 );
    MapMode aPntMode( MapUnit::MapPoint );
    MapMode _aEditMapMode(mpEditEngine->GetRefDevice()->GetMapMode().GetMapUnit());
    aSz = mpEditEngine->GetRefDevice()->LogicToLogic(aSz, &aPntMode, &_aEditMapMode);
    SvxFontHeightItem aFontHeightItem( aSz.Width(), 100, EE_CHAR_FONTHEIGHT );
    vcl::Font aDefFont( GetFont( nDefFont ) );
    SvxFontItem aFontItem( aDefFont.GetFamilyType(), aDefFont.GetFamilyName(),
                    aDefFont.GetStyleName(), aDefFont.GetPitch(), aDefFont.GetCharSet(), EE_CHAR_FONTINFO );

    sal_Int32 nStartPara = mpEditEngine->GetEditDoc().GetPos( rStart.GetNode() );
    sal_Int32 nEndPara = mpEditEngine->GetEditDoc().GetPos( rEnd.GetNode() );
    for ( sal_Int32 nPara = nStartPara; nPara <= nEndPara; nPara++ )
    {
        ContentNode* pNode = mpEditEngine->GetEditDoc().GetObject( nPara );
        assert(pNode && "AddRTFDefaultValues - No paragraph?!");
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
        break;
        case RTF_CELL:
        {
            aCurSel = mpEditEngine->InsertParaBreak(aCurSel);
        }
        break;
        case RTF_LINE:
        {
            aCurSel = mpEditEngine->InsertLineBreak(aCurSel);
        }
        break;
        case RTF_FIELD:
        {
            ReadField();
        }
        break;
        case RTF_SHPINST:  // fdo#76776 process contents of shpinst
        break;
        case RTF_SP:       // fdo#76776 but skip SP groups
        {
            SkipGroup();
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
    if (mpEditEngine->IsRtfImportHandlerSet())
    {
        RtfImportInfo aImportInfo(RtfImportState::NextToken, this, mpEditEngine->CreateESelection(aCurSel));
        aImportInfo.nToken = nToken;
        aImportInfo.nTokenValue = short(nTokenValue);
        mpEditEngine->CallRtfImportHandler(aImportInfo);
    }
}

void EditRTFParser::UnknownAttrToken( int nToken, SfxItemSet* )
{
    // for Tokens which are not evaluated in ReadAttr
    // Actually, only for Calc (RTFTokenHdl), so that RTF_INTBL
    if (mpEditEngine->IsRtfImportHandlerSet())
    {
        RtfImportInfo aImportInfo(RtfImportState::UnknownAttr, this, mpEditEngine->CreateESelection(aCurSel));
        aImportInfo.nToken = nToken;
        aImportInfo.nTokenValue = short(nTokenValue);
        mpEditEngine->CallRtfImportHandler(aImportInfo);
    }
}

void EditRTFParser::InsertText()
{
    OUString aText( aToken );
    if (mpEditEngine->IsRtfImportHandlerSet())
    {
        RtfImportInfo aImportInfo(RtfImportState::InsertText, this, mpEditEngine->CreateESelection(aCurSel));
        mpEditEngine->CallRtfImportHandler(aImportInfo);
    }
    aCurSel = mpEditEngine->InsertText(aCurSel, aText);
    bLastActionInsertParaBreak = false;
}

void EditRTFParser::InsertPara()
{
    if (mpEditEngine->IsRtfImportHandlerSet())
    {
        RtfImportInfo aImportInfo(RtfImportState::InsertPara, this, mpEditEngine->CreateESelection(aCurSel));
        mpEditEngine->CallRtfImportHandler(aImportInfo);
    }
    aCurSel = mpEditEngine->InsertParaBreak(aCurSel);
    bLastActionInsertParaBreak = true;
}

void EditRTFParser::MovePos( bool const bForward )
{
    if( bForward )
        aCurSel = mpEditEngine->CursorRight(
            aCurSel.Max(), i18n::CharacterIteratorMode::SKIPCHARACTER);
    else
        aCurSel = mpEditEngine->CursorLeft(
            aCurSel.Max(), i18n::CharacterIteratorMode::SKIPCHARACTER);
}

void EditRTFParser::SetEndPrevPara( EditNodeIdx*& rpNodePos,
                                    sal_Int32& rCntPos )
{
    // The Intention is to: determine the current insert position of the
    //                      previous paragraph and set the end from this.
    //                      This "\pard" always apply on the right paragraph.

    ContentNode* pN = aCurSel.Max().GetNode();
    sal_Int32 nCurPara = mpEditEngine->GetEditDoc().GetPos( pN );
    DBG_ASSERT( nCurPara != 0, "Paragraph equal to 0: SetEnfPrevPara" );
    if ( nCurPara )
        nCurPara--;
    ContentNode* pPrevNode = mpEditEngine->GetEditDoc().GetObject( nCurPara );
    assert(pPrevNode && "pPrevNode = 0!");
    rpNodePos = new EditNodeIdx(mpEditEngine, pPrevNode);
    rCntPos = pPrevNode->Len();
}

bool EditRTFParser::IsEndPara( EditNodeIdx* pNd, sal_Int32 nCnt ) const
{
    return nCnt == pNd->GetNode()->Len();
}

void EditRTFParser::SetAttrInDoc( SvxRTFItemStackType &rSet )
{
    ContentNode* pSttNode = const_cast<EditNodeIdx&>(rSet.GetSttNode()).GetNode();
    ContentNode* pEndNode = const_cast<EditNodeIdx&>(rSet.GetEndNode()).GetNode();

    EditPaM aStartPaM( pSttNode, rSet.GetSttCnt() );
    EditPaM aEndPaM( pEndNode, rSet.GetEndCnt() );

    // If possible adjust the Escapement-Item:
    const SfxPoolItem* pItem;

    // #i66167# adapt font heights to destination MapUnit if necessary
    const MapUnit eDestUnit = mpEditEngine->GetEditDoc().GetItemPool().GetMetric(0);
    if (eDestUnit != gRTFMapUnit)
    {
        sal_uInt16 const aFntHeightIems[3] = { EE_CHAR_FONTHEIGHT, EE_CHAR_FONTHEIGHT_CJK, EE_CHAR_FONTHEIGHT_CTL };
        for (unsigned short aFntHeightIem : aFntHeightIems)
        {
            if (SfxItemState::SET == rSet.GetAttrSet().GetItemState( aFntHeightIem, false, &pItem ))
            {
                sal_uInt32 nHeight  = static_cast<const SvxFontHeightItem*>(pItem)->GetHeight();
                long nNewHeight;
                nNewHeight = OutputDevice::LogicToLogic( static_cast<long>(nHeight), gRTFMapUnit, eDestUnit );

                SvxFontHeightItem aFntHeightItem( nNewHeight, 100, aFntHeightIem );
                aFntHeightItem.SetProp(
                    static_cast<const SvxFontHeightItem*>(pItem)->GetProp(),
                    static_cast<const SvxFontHeightItem*>(pItem)->GetPropUnit());
                rSet.GetAttrSet().Put( aFntHeightItem );
            }
        }
    }

    if( SfxItemState::SET == rSet.GetAttrSet().GetItemState( EE_CHAR_ESCAPEMENT, false, &pItem ))
    {
        // the correct one
        long nEsc = static_cast<const SvxEscapementItem*>(pItem)->GetEsc();
        long nEscFontHeight = 0;
        if( ( DFLT_ESC_AUTO_SUPER != nEsc ) && ( DFLT_ESC_AUTO_SUB != nEsc ) )
        {
            nEsc *= 10; //HalfPoints => Twips was embezzled in RTFITEM.CXX!
            SvxFont aFont;
            mpEditEngine->SeekCursor(aStartPaM.GetNode(), aStartPaM.GetIndex()+1, aFont);
            nEscFontHeight = aFont.GetFontSize().Height();
        }
        if (nEscFontHeight)
        {
            nEsc = nEsc * 100 / nEscFontHeight;

            SvxEscapementItem aEscItem( static_cast<short>(nEsc), static_cast<const SvxEscapementItem*>(pItem)->GetProportionalHeight(), EE_CHAR_ESCAPEMENT );
            rSet.GetAttrSet().Put( aEscItem );
        }
    }

    if (mpEditEngine->IsRtfImportHandlerSet())
    {
        EditSelection aSel( aStartPaM, aEndPaM );
        RtfImportInfo aImportInfo(RtfImportState::SetAttr, this, mpEditEngine->CreateESelection(aSel));
        mpEditEngine->CallRtfImportHandler(aImportInfo);
    }

    ContentNode* pSN = aStartPaM.GetNode();
    ContentNode* pEN = aEndPaM.GetNode();
    sal_Int32 nStartNode = mpEditEngine->GetEditDoc().GetPos( pSN );
    sal_Int32 nEndNode = mpEditEngine->GetEditDoc().GetPos( pEN );
    sal_Int16 nOutlLevel = 0xff;

    if (rSet.StyleNo() && mpEditEngine->GetStyleSheetPool() && mpEditEngine->IsImportRTFStyleSheetsSet())
    {
        SvxRTFStyleTbl::iterator it = GetStyleTbl().find( rSet.StyleNo() );
        DBG_ASSERT( it != GetStyleTbl().end(), "Template not defined in RTF!" );
        if ( it != GetStyleTbl().end() )
        {
            auto const& pS = it->second;
            mpEditEngine->SetStyleSheet(
                EditSelection(aStartPaM, aEndPaM),
                static_cast<SfxStyleSheet*>(mpEditEngine->GetStyleSheetPool()->Find(pS->sName, SfxStyleFamily::All)));
            nOutlLevel = pS->nOutlineNo;
        }
    }

    // When an Attribute goes from 0 to the current paragraph length,
    // it should be a paragraph attribute!

    // Note: Selection can reach over several paragraphs.
    // All Complete paragraphs are paragraph attributes ...
    for ( sal_Int32 z = nStartNode+1; z < nEndNode; z++ )
    {
        DBG_ASSERT(mpEditEngine->GetEditDoc().GetObject(z), "Node does not exist yet(RTF)");
        mpEditEngine->SetParaAttribsOnly(z, rSet.GetAttrSet());
    }

    if ( aStartPaM.GetNode() != aEndPaM.GetNode() )
    {
        // The rest of the StartNodes...
        if ( aStartPaM.GetIndex() == 0 )
            mpEditEngine->SetParaAttribsOnly(nStartNode, rSet.GetAttrSet());
        else
            mpEditEngine->SetAttribs(
                EditSelection(aStartPaM, EditPaM(aStartPaM.GetNode(), aStartPaM.GetNode()->Len())), rSet.GetAttrSet());

        // the beginning of the EndNodes....
        if ( aEndPaM.GetIndex() == aEndPaM.GetNode()->Len() )
            mpEditEngine->SetParaAttribsOnly(nEndNode, rSet.GetAttrSet());
        else
            mpEditEngine->SetAttribs(
                EditSelection(EditPaM(aEndPaM.GetNode(), 0), aEndPaM), rSet.GetAttrSet());
    }
    else
    {
        if ( ( aStartPaM.GetIndex() == 0 ) && ( aEndPaM.GetIndex() == aEndPaM.GetNode()->Len() ) )
        {
            // When settings char attribs as para attribs, we must merge with existing attribs, not overwrite the ItemSet!
            SfxItemSet aAttrs = mpEditEngine->GetBaseParaAttribs(nStartNode);
            aAttrs.Put( rSet.GetAttrSet() );
            mpEditEngine->SetParaAttribsOnly(nStartNode, aAttrs);
        }
        else
        {
            mpEditEngine->SetAttribs(
                EditSelection(aStartPaM, aEndPaM), rSet.GetAttrSet());
        }
    }

    // OutlLevel...
    if ( nOutlLevel != 0xff )
    {
        for ( sal_Int32 n = nStartNode; n <= nEndNode; n++ )
        {
            ContentNode* pNode = mpEditEngine->GetEditDoc().GetObject( n );
            pNode->GetContentAttribs().GetItems().Put( SfxInt16Item( EE_PARA_OUTLLEVEL, nOutlLevel ) );
        }
    }
}

SvxRTFStyleType* EditRTFParser::FindStyleSheet( const OUString& rName )
{
    SvxRTFStyleTbl& rTable = GetStyleTbl();
    for (auto const& iter : rTable)
    {
        if (iter.second->sName == rName)
            return iter.second.get();
    }
    return nullptr;
}

SfxStyleSheet* EditRTFParser::CreateStyleSheet( SvxRTFStyleType const * pRTFStyle )
{
    // Check if a template exists, then it will not be changed!
    SfxStyleSheet* pStyle = static_cast<SfxStyleSheet*>(mpEditEngine->GetStyleSheetPool()->Find( pRTFStyle->sName, SfxStyleFamily::All ));
    if ( pStyle )
        return pStyle;

    OUString aName( pRTFStyle->sName );
    OUString aParent;
    if ( pRTFStyle->nBasedOn )
    {
        SvxRTFStyleTbl::iterator it = GetStyleTbl().find( pRTFStyle->nBasedOn );
        if ( it != GetStyleTbl().end())
        {
            SvxRTFStyleType *const pS = it->second.get();
            if ( pS && ( pS !=pRTFStyle ) )
                aParent = pS->sName;
        }
    }

    pStyle = static_cast<SfxStyleSheet*>( &mpEditEngine->GetStyleSheetPool()->Make( aName, SfxStyleFamily::Para ) );

    // 1) convert and take over Items ...
    ConvertAndPutItems( pStyle->GetItemSet(), pRTFStyle->aAttrSet );

    // 2) As long as Parent is not in the pool, also create this ...
    if ( !aParent.isEmpty() && ( aParent != aName ) )
    {
        SfxStyleSheet* pS = static_cast<SfxStyleSheet*>(mpEditEngine->GetStyleSheetPool()->Find( aParent, SfxStyleFamily::All ));
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
    // the SvxRTFParser has now created the template...
    if (mpEditEngine->GetStyleSheetPool() && mpEditEngine->IsImportRTFStyleSheetsSet())
    {
        for (auto const& elem : GetStyleTbl())
        {
            SvxRTFStyleType* pRTFStyle = elem.second.get();
            CreateStyleSheet( pRTFStyle );
        }
    }
}

void EditRTFParser::CalcValue()
{
    const MapUnit eDestUnit = aEditMapMode.GetMapUnit();
    if (eDestUnit != gRTFMapUnit)
        nTokenValue = OutputDevice::LogicToLogic( nTokenValue, gRTFMapUnit, eDestUnit );
}

void EditRTFParser::ReadField()
{
    // From SwRTFParser::ReadField()
    int _nOpenBrakets = 1;      // the first was already detected earlier
    bool bFldInst = false;
    bool bFldRslt = false;
    OUString aFldInst;
    OUString aFldRslt;

    while( _nOpenBrakets && IsParserWorking() )
    {
        switch( GetNextToken() )
        {
            case '}':
            {
                _nOpenBrakets--;
                if ( _nOpenBrakets == 1 )
                {
                    bFldInst = false;
                    bFldRslt = false;
                }
            }
            break;

            case '{':           _nOpenBrakets++;
                                break;

            case RTF_FIELD:     SkipGroup();
                                break;

            case RTF_FLDINST:   bFldInst = true;
                                break;

            case RTF_FLDRSLT:   bFldRslt = true;
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
    if ( !aFldInst.isEmpty() )
    {
        OUString aHyperLinkMarker( "HYPERLINK " );
        if ( aFldInst.startsWithIgnoreAsciiCase( aHyperLinkMarker ) )
        {
            aFldInst = aFldInst.copy( aHyperLinkMarker.getLength() );
            aFldInst = comphelper::string::strip(aFldInst, ' ');
            // strip start and end quotes
            aFldInst = aFldInst.copy( 1, aFldInst.getLength()-2 );

            if ( aFldRslt.isEmpty() )
                aFldRslt = aFldInst;

            SvxFieldItem aField( SvxURLField( aFldInst, aFldRslt, SvxURLFormat::Repr ), EE_FEATURE_FIELD  );
            aCurSel = mpEditEngine->InsertField(aCurSel, aField);
            mpEditEngine->UpdateFieldsOnly();
            bLastActionInsertParaBreak = false;
        }
    }

    SkipToken();        // the closing brace is evaluated "above"
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

    SkipToken();        // the closing brace is evaluated "above"
}

EditNodeIdx::EditNodeIdx(EditEngine* pEE, ContentNode* pNd) :
    mpEditEngine(pEE), mpNode(pNd) {}

sal_Int32 EditNodeIdx::GetIdx() const
{
    return mpEditEngine->GetEditDoc().GetPos(mpNode);
}

EditPosition::EditPosition(EditEngine* pEE, EditSelection* pSel) :
    mpEditEngine(pEE), mpCurSel(pSel) {}

std::unique_ptr<EditPosition> EditPosition::Clone() const
{
    return std::unique_ptr<EditPosition>(new EditPosition(mpEditEngine, mpCurSel));
}

EditNodeIdx* EditPosition::MakeNodeIdx() const
{
    return new EditNodeIdx(mpEditEngine, mpCurSel->Max().GetNode());
}

sal_Int32 EditPosition::GetNodeIdx() const
{
    ContentNode* pN = mpCurSel->Max().GetNode();
    return mpEditEngine->GetEditDoc().GetPos(pN);
}

sal_Int32 EditPosition::GetCntIdx() const
{
    return mpCurSel->Max().GetIndex();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
