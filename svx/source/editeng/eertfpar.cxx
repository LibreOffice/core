/*************************************************************************
 *
 *  $RCSfile: eertfpar.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:14 $
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

#include <eeng_pch.hxx>

#pragma hdrstop

#include <eertfpar.hxx>
#include <impedit.hxx>
#include <svtools/intitem.hxx>
#include "escpitem.hxx"
#include "fhgtitem.hxx"
#include "fontitem.hxx"

#ifndef _SVX_FLDITEM_HXX
#include <flditem.hxx>
#endif

#include <svtools/rtftoken.h>

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
    : SvxRTFParser( rAttrPool, rIn ), aRTFMapMode( MAP_TWIP )
{

    pImpEditEngine  = pImpEE;
    aCurSel         = aSel;
    eDestCharSet    = RTL_TEXTENCODING_DONTKNOW;
    nDefFont        = 0;
    nDefTab         = 0;
    nLastAction     = 0;
    nDefFontHeight  = 0;

    SetInsPos( EditPosition( pImpEditEngine, &aCurSel ) );

    // Umwandeln der Twips-Werte...
    SetCalcValue( TRUE );
    SetChkStyleAttr( pImpEE->GetStatus().DoImportRTFStyleSheets() );
    SetNewDoc( FALSE );     // damit die Pool-Defaults nicht
                            // ueberschrieben werden...
    aEditMapMode = MapMode( pImpEE->GetRefDevice()->GetMapMode().GetMapUnit() );

#ifdef EDITDEBUG
    SvFileStream aStream( "d:\\rtf.log", STREAM_WRITE|STREAM_TRUNC );
    ULONG nP = rIn.Tell();
    aStream << rIn;
    rIn.Seek( nP );
#endif
}

EditRTFParser::~EditRTFParser()
{
}

SvParserState __EXPORT EditRTFParser::CallParser()
{
    DBG_ASSERT( !aCurSel.HasRange(), "Selection bei CallParser!" );
    // Den Teil, in den importiert wird, vom Rest abtrennen.
    // Diese Mimik sollte fuer alle Imports verwendet werden.
    // aStart1PaM: Letzte Position vor dem importierten Inhalt
    // aEnd1PaM: Erste Position nach dem importierten Inhalt
    // aStart2PaM: Erste Position des importierten Inhaltes
    // aEnd2PaM: Letzte Position des importierten Inhaltes
    EditPaM aStart1PaM( aCurSel.Min().GetNode(), aCurSel.Min().GetIndex() );
    aCurSel = pImpEditEngine->ImpInsertParaBreak( aCurSel );
    EditPaM aStart2PaM = aCurSel.Min();
    // Sinnvoll oder nicht?:
    aStart2PaM.GetNode()->GetContentAttribs().GetItems().ClearItem();
    EditPaM aEnd1PaM( pImpEditEngine->ImpInsertParaBreak( aCurSel.Max() ) );
    // aCurCel zeigt jetzt auf den Zwischenraum

    if ( pImpEditEngine->aImportHdl.IsSet() )
    {
        ImportInfo aImportInfo( RTFIMP_START, this, pImpEditEngine->CreateESel( aCurSel ) );
        pImpEditEngine->aImportHdl.Call( &aImportInfo );
    }

    SvParserState eState = SvxRTFParser::CallParser();

    if ( pImpEditEngine->aImportHdl.IsSet() )
    {
        ImportInfo aImportInfo( RTFIMP_END, this, pImpEditEngine->CreateESel( aCurSel ) );
        pImpEditEngine->aImportHdl.Call( &aImportInfo );
    }

    if ( nLastAction == ACTION_INSERTPARABRK )
    {
        ContentNode* pCurNode = aCurSel.Max().GetNode();
        USHORT nPara = pImpEditEngine->GetEditDoc().GetPos( pCurNode );
        ContentNode* pPrevNode = pImpEditEngine->GetEditDoc().SaveGetObject( nPara-1 );
        DBG_ASSERT( pPrevNode, "Ungueltiges RTF-Dokument ?!" );
        EditSelection aSel;
        aSel.Min() = EditPaM( pPrevNode, pPrevNode->Len() );
        aSel.Max() = EditPaM( pCurNode, 0 );
        aCurSel.Max() = pImpEditEngine->ImpDeleteSelection( aSel );
    }
    EditPaM aEnd2PaM( aCurSel.Max() );
    AddRTFDefaultValues( aStart2PaM, aEnd2PaM );
    BOOL bOnlyOnePara = ( aEnd2PaM.GetNode() == aStart2PaM.GetNode() );
    // Den Brocken wieder einfuegen...
    // Problem: Absatzattribute duerfen ggf. nicht uebernommen werden
    // => Zeichenattribute machen.

    BOOL bSpecialBackward = aStart1PaM.GetNode()->Len() ? FALSE : TRUE;
    if ( bOnlyOnePara || aStart1PaM.GetNode()->Len() )
        pImpEditEngine->ParaAttribsToCharAttribs( aStart2PaM.GetNode() );
    aCurSel.Min() = pImpEditEngine->ImpConnectParagraphs(
        aStart1PaM.GetNode(), aStart2PaM.GetNode(), bSpecialBackward );
    bSpecialBackward = aEnd1PaM.GetNode()->Len() ? TRUE : FALSE;
    // wenn bOnlyOnePara, dann ist der Node beim Connect verschwunden.
    if ( !bOnlyOnePara && aEnd1PaM.GetNode()->Len() )
        pImpEditEngine->ParaAttribsToCharAttribs( aEnd2PaM.GetNode() );
    aCurSel.Max() = pImpEditEngine->ImpConnectParagraphs(
        ( bOnlyOnePara ? aStart1PaM.GetNode() : aEnd2PaM.GetNode() ),
            aEnd1PaM.GetNode(), bSpecialBackward );

    return eState;
}

void EditRTFParser::AddRTFDefaultValues( const EditPaM& rStart, const EditPaM& rEnd )
{
    // Problem: DefFont und DefFontHeight
    Size aSz( 12, 0 );
    MapMode aPntMode( MAP_POINT );
    MapMode aEditMapMode( pImpEditEngine->GetRefDevice()->GetMapMode().GetMapUnit() );
    aSz = pImpEditEngine->GetRefDevice()->LogicToLogic( aSz, &aPntMode, &aEditMapMode );
    SvxFontHeightItem aFontHeightItem( aSz.Width(), 100, EE_CHAR_FONTHEIGHT );
    Font aDefFont( GetDefFont() );
    SvxFontItem aFontItem( aDefFont.GetFamily(), aDefFont.GetName(),
                    aDefFont.GetStyleName(), aDefFont.GetPitch(), aDefFont.GetCharSet(), EE_CHAR_FONTINFO );

    USHORT nStartPara = pImpEditEngine->GetEditDoc().GetPos( rStart.GetNode() );
    USHORT nEndPara = pImpEditEngine->GetEditDoc().GetPos( rEnd.GetNode() );
    for ( USHORT nPara = nStartPara; nPara <= nEndPara; nPara++ )
    {
        ContentNode* pNode = pImpEditEngine->GetEditDoc().SaveGetObject( nPara );
        DBG_ASSERT( pNode, "AddRTFDefaultValues - Kein Absatz ?!" );
        if ( !pNode->GetContentAttribs().HasItem( EE_CHAR_FONTINFO ) )
            pNode->GetContentAttribs().GetItems().Put( aFontItem );
        if ( !pNode->GetContentAttribs().HasItem( EE_CHAR_FONTHEIGHT ) )
            pNode->GetContentAttribs().GetItems().Put( aFontHeightItem );
    }
}

void __EXPORT EditRTFParser::NextToken( int nToken )
{
    switch( nToken )
    {
        case RTF_DEFF:
        {
            nDefFont = USHORT(nTokenValue);
        }
        break;
        case RTF_DEFTAB:
        {
            nDefTab = USHORT(nTokenValue);
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

void __EXPORT EditRTFParser::UnknownAttrToken( int nToken, SfxItemSet* pSet )
{
    // fuer Tokens, die im ReadAttr nicht ausgewertet werden
    // Eigentlich nur fuer Calc (RTFTokenHdl), damit RTF_INTBL
    if ( pImpEditEngine->aImportHdl.IsSet() )
    {
        ImportInfo aImportInfo( RTFIMP_UNKNOWNATTR, this, pImpEditEngine->CreateESel( aCurSel ) );
        aImportInfo.nToken = nToken;
        aImportInfo.nTokenValue = short(nTokenValue);
        pImpEditEngine->aImportHdl.Call( &aImportInfo );
    }
}

void __EXPORT EditRTFParser::InsertText()
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

void __EXPORT EditRTFParser::InsertPara()
{
    if ( pImpEditEngine->aImportHdl.IsSet() )
    {
        ImportInfo aImportInfo( RTFIMP_INSERTPARA, this, pImpEditEngine->CreateESel( aCurSel ) );
        pImpEditEngine->aImportHdl.Call( &aImportInfo );
    }
    aCurSel = pImpEditEngine->ImpInsertParaBreak( aCurSel );
    nLastAction = ACTION_INSERTPARABRK;
}

void __EXPORT EditRTFParser::MovePos( int bForward )
{
    if( bForward )
        aCurSel = pImpEditEngine->CursorRight( aCurSel.Max() );
    else
        aCurSel = pImpEditEngine->CursorLeft( aCurSel.Max() );
}

void __EXPORT EditRTFParser::SetEndPrevPara( SvxNodeIdx*& rpNodePos,
                                    USHORT& rCntPos )
{
    //    Gewollt ist: von der aktuellen Einfuegeposition den vorherigen
    //              Absatz bestimmen und von dem das Ende setzen.
    //              Dadurch wird "\pard" immer auf den richtigen Absatz
    //              angewendet.

    ContentNode* pN = aCurSel.Max().GetNode();
    USHORT nCurPara = pImpEditEngine->GetEditDoc().GetPos( pN );
    DBG_ASSERT( nCurPara != 0, "Absatz gleich 0: SetEnfPrevPara" );
    if ( nCurPara )
        nCurPara--;
    ContentNode* pPrevNode = pImpEditEngine->GetEditDoc().SaveGetObject( nCurPara );
    DBG_ASSERT( pPrevNode, "pPrevNode = 0!" );
    rpNodePos = new EditNodeIdx( pImpEditEngine, pPrevNode );
    rCntPos = pPrevNode->Len();
}

int __EXPORT EditRTFParser::IsEndPara( SvxNodeIdx* pNd, USHORT nCnt ) const
{
    return ( nCnt == ( ((EditNodeIdx*)pNd)->GetNode()->Len()) );
}

void __EXPORT EditRTFParser::SetAttrInDoc( SvxRTFItemStackType &rSet )
{
    ContentNode* pSttNode = ((EditNodeIdx&)rSet.GetSttNode()).GetNode();
    ContentNode* pEndNode = ((EditNodeIdx&)rSet.GetEndNode()).GetNode();

    EditPaM aStartPaM( pSttNode, rSet.GetSttCnt() );
    EditPaM aEndPaM( pEndNode, rSet.GetEndCnt() );

    // ggf. noch das Escapemant-Item umbiegen:
    const SfxPoolItem* pItem;
    if( SFX_ITEM_SET == rSet.GetAttrSet().GetItemState( EE_CHAR_ESCAPEMENT, FALSE, &pItem ))
    {
        // die richtige
        long nEsc = ((SvxEscapementItem*)pItem)->GetEsc();

        if( ( DFLT_ESC_AUTO_SUPER != nEsc ) && ( DFLT_ESC_AUTO_SUB != nEsc ) )
        {
            nEsc *= 10; //HalPoints => Twips wurde in RTFITEM.CXX unterschlagen!
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
    USHORT nStartNode = pImpEditEngine->GetEditDoc().GetPos( pSN );
    USHORT nEndNode = pImpEditEngine->GetEditDoc().GetPos( pEN );
    USHORT nOutlLevel = 0xFF;

    if ( rSet.StyleNo() && pImpEditEngine->GetStyleSheetPool() && pImpEditEngine->GetStatus().DoImportRTFStyleSheets() )
    {
        SvxRTFStyleType* pS = GetStyleTbl().Get( rSet.StyleNo() );
        DBG_ASSERT( pS, "Vorlage in RTF nicht definiert!" );
        if ( pS )
        {
            pImpEditEngine->SetStyleSheet( EditSelection( aStartPaM, aEndPaM ), (SfxStyleSheet*)pImpEditEngine->GetStyleSheetPool()->Find( pS->sName, SFX_STYLE_FAMILY_ALL ) );
            nOutlLevel = pS->nOutlineNo;
        }
    }

    // Wenn ein Attribut von 0 bis aktuelle Absatzlaenge geht,
    // soll es ein Absatz-Attribut sein!

    // Achtung: Selektion kann ueber mehrere Absaetze gehen.
    // Alle vollstaendigen Absaetze sind Absatzattribute...
    for ( USHORT z = nStartNode+1; z < nEndNode; z++ )
    {
        DBG_ASSERT( pImpEditEngine->GetEditDoc().SaveGetObject( z ), "Node existiert noch nicht(RTF)" );
        pImpEditEngine->SetParaAttribs( z, rSet.GetAttrSet() );
    }

    if ( aStartPaM.GetNode() != aEndPaM.GetNode() )
    {
        // Den Rest des StartNodes...
        if ( aStartPaM.GetIndex() == 0 )
            pImpEditEngine->SetParaAttribs( nStartNode, rSet.GetAttrSet() );
        else
            pImpEditEngine->SetAttribs( EditSelection( aStartPaM, EditPaM( aStartPaM.GetNode(), aStartPaM.GetNode()->Len() ) ), rSet.GetAttrSet() );

        // Den Anfang des EndNodes....
        if ( aEndPaM.GetIndex() == aEndPaM.GetNode()->Len() )
            pImpEditEngine->SetParaAttribs( nEndNode, rSet.GetAttrSet() );
        else
            pImpEditEngine->SetAttribs( EditSelection( EditPaM( aEndPaM.GetNode(), 0 ), aEndPaM ), rSet.GetAttrSet() );
    }
    else
    {
        if ( ( aStartPaM.GetIndex() == 0 ) && ( aEndPaM.GetIndex() == aEndPaM.GetNode()->Len() ) )
            pImpEditEngine->SetParaAttribs( nStartNode, rSet.GetAttrSet() );
        else
            pImpEditEngine->SetAttribs( EditSelection( aStartPaM, aEndPaM ), rSet.GetAttrSet() );
    }

    // OutlLevel...
    if ( nOutlLevel != 0xFF )
    {
        for ( USHORT n = nStartNode; n <= nEndNode; n++ )
        {
            ContentNode* pNode = pImpEditEngine->GetEditDoc().SaveGetObject( n );
            pNode->GetContentAttribs().GetItems().Put( SfxUInt16Item( EE_PARA_OUTLLEVEL, nOutlLevel ) );
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
    // Prueffen, ob so eine Vorlage existiert....
    // dann wird sie auch nicht geaendert!
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

    // 1) Items konvertieren und uebernehmen...
    ConvertAndPutItems( pStyle->GetItemSet(), pRTFStyle->aAttrSet );

    // 2) Solange Parent nicht im Pool, auch diesen kreieren...
    if ( aParent.Len() && ( aParent != aName ) )
    {
        SfxStyleSheet* pS = (SfxStyleSheet*)pImpEditEngine->GetStyleSheetPool()->Find( aParent, SFX_STYLE_FAMILY_ALL );
        if ( !pS )
        {
            // Wenn nirgendwo gefunden, aus RTF erzeugen...
            SvxRTFStyleType* pRTFStyle = FindStyleSheet( aParent );
            if ( pRTFStyle )
                pS = CreateStyleSheet( pRTFStyle );
        }
        // 2b) ItemSet mit Parent verknuepfen...
        if ( pS )
            pStyle->GetItemSet().SetParent( &pS->GetItemSet() );
    }
    return pStyle;
}

void EditRTFParser::CreateStyleSheets()
{
    // der SvxRTFParser hat jetzt die Vorlagen erzeugt...
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

void __EXPORT EditRTFParser::CalcValue()
{
    nTokenValue = TwipsToLogic( nTokenValue );
}

void EditRTFParser::ReadField()
{
    // Aus SwRTFParser::ReadField()
    int nRet = 0;
    int nOpenBrakets = 1;       // die erste wurde schon vorher erkannt
    BOOL bFldInst = FALSE;
    BOOL bFldRslt = FALSE;
    String aFldInst;
    String aFldRslt;

    while( nOpenBrakets && IsParserWorking() )
    {
        switch( GetNextToken() )
        {
            case '}':
            {
                nOpenBrakets--;
                if ( nOpenBrakets == 1 )
                {
                    bFldInst = FALSE;
                    bFldRslt = FALSE;
                }
            }
            break;

            case '{':           nOpenBrakets++;
                                break;

            case RTF_FIELD:     SkipGroup();
                                break;

            case RTF_FLDINST:   bFldInst = TRUE;
                                break;

            case RTF_FLDRSLT:   bFldRslt = TRUE;
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

    SkipToken( -1 );        // die schliesende Klammer wird "oben" ausgewertet
}

ULONG __EXPORT EditNodeIdx::GetIdx() const
{
    return pImpEditEngine->GetEditDoc().GetPos( pNode );
}

SvxNodeIdx* __EXPORT EditNodeIdx::Clone() const
{
    return new EditNodeIdx( pImpEditEngine, pNode );
}

SvxPosition* __EXPORT EditPosition::Clone() const
{
    return new EditPosition( pImpEditEngine, pCurSel );
}

SvxNodeIdx* __EXPORT EditPosition::MakeNodeIdx() const
{
    return new EditNodeIdx( pImpEditEngine, pCurSel->Max().GetNode() );
}

ULONG __EXPORT EditPosition::GetNodeIdx() const
{
    ContentNode* pN = pCurSel->Max().GetNode();
    return pImpEditEngine->GetEditDoc().GetPos( pN );
}

USHORT __EXPORT EditPosition::GetCntIdx() const
{
    return pCurSel->Max().GetIndex();
}
