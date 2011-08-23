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


#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <horiornt.hxx>

#include "doc.hxx"

#include <errhdl.hxx>

#include "pam.hxx"

#include <fmtfsize.hxx>
#include "rdswg.hxx"
#include "swgpar.hxx"
#include "ndtxt.hxx"
#include "bookmrk.hxx"
#include "swtable.hxx"
#include "mdiexp.hxx"        // Progress
#include "swddetbl.hxx"
#include "swtblfmt.hxx"
namespace binfilter {


//////////////////////////////////////////////////////////////////////////////

// Achtung: Jede Routine nimmt an, dass der Record-Header bereits eingelesen
// ist. Sie arbeitet den Record so weit ab wie moeglich und schliesst den
// Vorgang mit end() ab.

//////////////////////////////////////////////////////////////////////////////

// Contents-Teil

// Wird ein Text in einen Absatz eingefuegt, wird der erste Absatz an den
// ersten Teil angefuegt, ohne die Formate zu uebernehmen. Alle anderen
// Absaetze uebernehmen die Formate, was auch den Rest des alten Absatzes
// betrifft.
// Falls bNode1 FALSE ist, wird ein evtl. vorhandener leerer Node nicht
// gefuellt. Dadurch laesst sich diese Methode fuer mehrere Contents
// verwenden.
// Falls bBlock TRUE ist, handelt es sich um das Einlesen eies Textblock-
// Inhalts. Hier gibt es keinen Section-ID und auch keine Registrierrung
// der Section!
// nInsFirstPara - beim Document einfuegen, muss der erste Absatz
// 					sonderbehandelt werden (Absatz Attribute!)

typedef SwTableBox* SwTableBoxPtr;

void SwSwgReader::InContents( BOOL bNode1, BOOL bBlock, USHORT nInsFirstPara )
{
    USHORT nNodes, nId = IDX_NO_VALUE;
    USHORT nOldStatus = nStatus;
    // Beim Einlesen eines TB keine Bookmarks einlesen!
    // Auch sollen alle Autoformate (AttrSets) wiederverwendbar sein!
    if( bBlock )
    {
        ReleaseAttrSets();
        nStatus |= SWGSTAT_NO_BOOKMARKS | SWGSTAT_LOCALFMTS;
    }
    if( !bBlock )
        r >> nId;
    r >> nNodes;
    r.next();

    // fuer die Sonderbehandlung des ersten/letzten Node beim "Datei/Einfuegen"
    USHORT nSaveInsFirstPara = nInsFirstPara;

    // Bei einem echten Insert muss der aktuelle Node gesplittet werden,
    // Falls mehr als 1 Node eingelesen wird.
    BOOL bIns = BOOL( !bNew );
    BOOL bSplit = FALSE;
    SwTxtNode* pLast = NULL;
    SwNodeIndex aPos( pPaM->GetPoint()->nNode );
    xub_StrLen nOffset = pPaM->GetPoint()->nContent.GetIndex();
    SwNode* pStart = pDoc->GetNodes()[ aPos ];
    SwTxtNode* pNd = bNode1 ? pStart->GetTxtNode() : NULL;
    if( !bBlock )
    {
        RegisterSection( SwNodeIndex( *pStart->StartOfSectionNode() ), nId );
    }
    for( USHORT i = 1; i <= nNodes && r.good(); i++)
    {
        ::binfilter::SetProgressState( r.tell(), pDoc->GetDocShell() );

        switch( r.cur() )
        {
            case SWG_TEXTNODE:
                // Der Node muss gesplittet werden,
                // wenn mehrere Nodes inserted werden
                if( !bSplit && bIns && nNodes > 1 )
                {
                    if( pNd )
                    {
                        // natuerlich nur TextNodes splitten
                        SwPosition aSplitPos( aPos, SwIndex( pNd, nOffset ) );
                        pDoc->SplitNode( aSplitPos );
                        pLast = pNd;
                        aPos--;
                        pNd = pDoc->GetNodes()[ aPos ]->GetTxtNode();
                        bSplit = TRUE;
                    }
                }
                else if( i == nNodes && pLast )
                {
                    // der letzte Node: nimm evtl. den gesplitteten
                    pNd = pLast;
                    if( nSaveInsFirstPara && pNd && pNd->GetTxt().Len() )
                        nInsFirstPara = 2;
                }
                FillTxtNode( pNd, aPos, nOffset, nInsFirstPara );
                pNd = NULL; nOffset = 0;
                nInsFirstPara = 0;
                break;
            case SWG_GRFNODE:
                InGrfNode( aPos );
                nInsFirstPara = 0;
                goto chknode;
            case SWG_OLENODE:
                InOleNode( aPos );
                // Falls ein TxtNode am Anfang war, weg damit!
                nInsFirstPara = 0;
            chknode:
                if( pNd )
                    pDoc->GetNodes().Delete( aPos );
                pNd = NULL;
                break;
            case SWG_TABLE: {
                // JP 20.05.94: Dok. einfuegen: wird als 1.Node eine
                //              Tabelle eingefuegt, dann immer splitten !!
                nInsFirstPara = 0;
                if( 1 == i && bIns )
                {
                    SwPosition aSplitPos( aPos, SwIndex( pNd, nOffset ) );
                    pDoc->SplitNode( aSplitPos );
                    nOffset = 0;    //  mit dem Offset hat sichs erledigt!
                }
                InTable( aPos );
                if( i != 1 ) pNd = NULL;
                } break;
            case SWG_TOXGROUP:
                InTOX( aPos, pNd );
                pNd = NULL;
                break;
            case SWG_TEXTBLOCK: {
                BYTE cFlag;
                r >> cFlag;
                if( !cFlag ) r.skipnext();
                else {
                    // TODO: unicode: check for 8 bit encoding
                    BYTE eCharSet = (BYTE) gsl_getSystemTextEncoding();
                    if( cFlag & 0x02 )
                      r >> eCharSet;
                    BYTE eSave = aHdr.cCodeSet;
                    aHdr.cCodeSet = eCharSet;
                    GetText();
                    GetText();
                    InContents( bNode1, TRUE );
                    aHdr.cCodeSet = eSave;
                    bNode1 = FALSE;
                }
            } break;
            case SWG_COMMENT:
                // Reste von TextBlocks
                r.skipnext(); break;
            case SWG_EOF:
                // sicherheitshalber
                r.undonext(); i = nNodes = 9999; break;
            default:
                // unbekannte Nodes
                Error(); r.skipnext();
        }
    }
    nStatus = nOldStatus;
}

// Einlesen des puren Textes eines Content-Bereichs

String SwSwgReader::InContentsText( BOOL bBlock )
{
    USHORT nNodes, nId;
    String aText;
    if( !bBlock )
        r >> nId;
    r >> nNodes;
    r.next();
    for( USHORT i = 1; i <= nNodes && r.good(); i++)
    {
        switch( r.cur() )
        {
            case SWG_TEXTNODE:
                FillString( aText );
                break;
            case SWG_TABLE:     // Zur Zeit keine Tabellen
            case SWG_GRFNODE:
            case SWG_OLENODE:
            case SWG_TOXGROUP:
            case SWG_COMMENT:
                r.skipnext(); break;
            case SWG_TEXTBLOCK: {
                BYTE cFlag;
                r >> cFlag;
                if( !cFlag ) r.skipnext();
                else {
                    // TODO: unicode: check for 8 bit encoding
                    BYTE eCharSet = (BYTE) gsl_getSystemTextEncoding();
                    if( cFlag & 0x02 )
                      r >> eCharSet;
                    BYTE eSave = aHdr.cCodeSet;
                    aHdr.cCodeSet = eCharSet;
                    GetText();
                    GetText();
                    String aBlock = InContentsText( TRUE );
                    if( aText.Len() ) aText += ' ';
                    aText += aBlock;
                    aHdr.cCodeSet = eSave;
                }
            } break;
            default:
                // unbekannte Nodes
                Error(); r.skipnext();
        }
    }
    return aText;
}

// Einfuegen einer nicht vorhandenen Contents-Section

SwStartNode* SwSwgReader::InSection()
{
    // Anlegen einer Section mit einem TextNode
    SwNodeIndex aStart( pDoc->GetNodes().GetEndOfAutotext() );
    SwStartNode* pSttNd = pDoc->GetNodes().MakeTextSection( aStart,
                                (SwStartNodeType)eStartNodeType,
                                (SwTxtFmtColl*) pDoc->GetDfltTxtFmtColl() );
    aStart = *pSttNd;
    FillSection( aStart );
    return pSttNd;
}

// Fuellen einer vorhandenen Contents-Section
// Die Position zeigt auf den StartNode
// Der SwIndex zeigt hinterher auf den EndNode

void SwSwgReader::FillSection( SwNodeIndex& rPos )
{
    if( r.next() != SWG_CONTENTS )
    {
        Error();
        rPos = rPos.GetIndex() + 3; // skip section
        return;
    }
    // die Anzahl Nodes einlesen
    USHORT nId, nNodes;
    r >> nId >> nNodes;
    RegisterSection( rPos, nId );
    rPos++;
    SwCntntNode* pNode = pDoc->GetNodes()[ rPos ]->GetCntntNode();
    r.next();
    for( USHORT i = 1; i <= nNodes && r.good(); i++)
    {
        ::binfilter::SetProgressState( r.tell(), pDoc->GetDocShell() );
        switch( r.cur() )
        {
            case SWG_TEXTNODE:
                FillTxtNode( (SwTxtNode*) pNode, rPos, 0 );
                pNode = NULL;
                break;
            case SWG_GRFNODE:
                InGrfNode( rPos );
                break;
            case SWG_OLENODE:
                InOleNode( rPos );
                break;
            case SWG_TABLE:
                InTable( rPos );
                break;
            case SWG_COMMENT:
                // Reste von TextBlocks
                r.skipnext(); break;
            default:
                // unbekannte Nodes
                Error(); r.skipnext();
        }
    }
    // Ueber den EndNode skippen
    rPos++;

    // Eine Tabelle und eine Section koennen u.U. alleine
    // in einer Textsection existieren, d.h. es muss ggf.
    // der ueberfluessige Node entfernt werden!
    if( pNode && r.good() )
    {
        // Falls der aktuelle PaM auf diesen Node zeigt, den PaM
        // knallhart wegschieben. Ist erlaubt laut JP!
        if( pPaM )
        {
            if( pPaM->GetBound( TRUE ).nContent.GetIdxReg() == pNode )
                pPaM->GetBound( TRUE ).nContent.Assign( 0, 0 );
            if( pPaM->GetBound( FALSE ).nContent.GetIdxReg() == pNode )
                pPaM->GetBound( FALSE ).nContent.Assign( 0, 0 );
        }
        pDoc->GetNodes().Delete( SwNodeIndex( *pNode ) );
    }

}

/////////////////////////////////////////////////////////////////////////////

// Tabellen

// Zeilen und Boxen duerfen keine Rueckwaertsreferenz in andere
// Tabellen enthalten, also: Fixen!

static long nFrmSizeAdj = 1;            // Divisionsfaktor fuer FrmSize

// Das Attribut darf im Prinzip nicht > 65535 werden, da sonst MAs
// Rechenroutinen baden gehen. Da einige alte Tabellen groessere Werte
// enthalten, setzt InTable() ggf. einen Divisionsfaktor, um den
// die gelesenen Werte heruntergeteilt werden. Normalerweise ist er 1.

static void AdjustFrmSize( SwFrmFmt* pFmt )
{
    if( pFmt && nFrmSizeAdj > 1
     && pFmt->GetAttrSet().GetItemState( RES_FRM_SIZE, FALSE ) == SFX_ITEM_SET )
    {
        SwFmtFrmSize aSz( pFmt->GetFrmSize() );
        if( aSz.GetSizeType() == ATT_VAR_SIZE )
        {
            aSz.SetWidth( aSz.GetWidth() / nFrmSizeAdj );
            aSz.SetHeight( aSz.GetHeight() / nFrmSizeAdj );
            pFmt->SetAttr( aSz );
        }
    }
}

// Das uebergebene Array ist erst mal leer.
// Der Index zeigt auf den naechsten freien StartNode und wird
// auf den wiederum naechsten vorgeschoben.

void SwSwgReader::InTableBox
     (SwTableBoxes& rBoxes, int idx,    // Array und Index
      SwTableLine* pUpper,              // uebergeordnete Zeile
      SwNodeIndex& rPos,
      const SwTable *pTable )           // Start-Index der Section
{
    short nFrmFmt, nLines;
    r >> nFrmFmt >> nLines;
    // Frame-Format evtl. einlesen
    SwTableBoxFmt* pFmt = NULL;
    r.next();
    while( r.cur() == SWG_FRAMEFMT && r.good() )
    {
        // Das Attribut SwFmtFrmSize benoetigt diese Werte, um
        // seine Werte umzudrehen
        USHORT nOldLvl = nTblBoxLvl;
        nTblBoxLvl = nFmtLvl;
        pFmt = (SwTableBoxFmt*) InFormat( pDoc->MakeTableBoxFmt() );
        RegisterFmt( *pFmt, pTable );
        AdjustFrmSize( pFmt );
        nTblBoxLvl = nOldLvl;
    }
    // Frame-Format raussuchen
    if( !pFmt )
    {
        pFmt = (SwTableBoxFmt*) FindFmt( nFrmFmt, SWG_FRAMEFMT );
        if( !pFmt )
        {
            Error(); return;
        }
        if( pTable != FindTable( nFrmFmt ) )
        {
            // es MUSS ein neues Format angelegt werden!
            SwTableBoxFmt* pNew = pDoc->MakeTableBoxFmt();
            // Attribute hart hineinkopieren
            ((SfxItemSet&)pNew->GetAttrSet()).Put( pFmt->GetAttrSet() );
            pNew->nFmtId = nFrmFmt;
            ReRegisterFmt( *pFmt, *pNew, pTable );
            pFmt = pNew;
            AdjustFrmSize( pFmt );
            aFile.nFlags |= SWGF_NO_FRAMES;
        }
    }
    SwTableBoxPtr pBox;
    if( r.cur() == SWG_CONTENTS )
    {
        r.undonext();
        pBox = new SwTableBox( pFmt, rPos, pUpper );
        long nSaveFrmSizeAdj = nFrmSizeAdj; // Tabellen in FlyFrames
        FillSection( rPos );
        nFrmSizeAdj = nSaveFrmSizeAdj;      // zerstoeren nFrmSizeAdj!
    } else
        pBox = new SwTableBox( pFmt, nLines, pUpper );
    rBoxes.C40_INSERT( SwTableBox, pBox, (USHORT)idx );
    while( r.cur() == SWG_COMMENT || r.cur() == SWG_DATA ) r.skipnext();
    // Falls Zeilen vorhanden, diese ausgeben
    for( int i = 0; i < nLines && r.good(); i++)
    {
        if( r.cur() != SWG_TABLELINE )
        {
            Error(); return;
        }
        InTableLine( pBox->GetTabLines(), pBox, i, rPos, pTable );
    }
}

// Einlesen einer Zeile

void SwSwgReader::InTableLine
    ( SwTableLines& rLines, SwTableBoxPtr pUpper, int idx, SwNodeIndex& rPos,
      const SwTable *pTable )
{
    short nFrmFmt, nBoxes;
    r >> nFrmFmt >> nBoxes;
    r.next();
    SwTableLineFmt* pFmt = NULL;
    // Frame-Format evtl. einlesen
    while( r.cur() == SWG_FRAMEFMT && r.good() )
    {
        pFmt = (SwTableLineFmt*) InFormat( pDoc->MakeTableLineFmt() );
        RegisterFmt( *pFmt, pTable );
//      AdjustFrmSize( pFmt );
    }
    // Frame-Format raussuchen
    // Frame-Format raussuchen
    if( !pFmt )
    {
        pFmt = (SwTableLineFmt*) FindFmt( nFrmFmt, SWG_FRAMEFMT );
        if( !pFmt )
        {
            Error(); return;
        }
        if( pTable != FindTable( nFrmFmt ) )
        {
            // es MUSS ein neues Format angelegt werden!
            SwTableLineFmt* pNew = pDoc->MakeTableLineFmt();
            // Attribute hart hineinkopieren
            ((SfxItemSet&)pNew->GetAttrSet()).Put( pFmt->GetAttrSet() );
            pNew->nFmtId = nFrmFmt;
            ReRegisterFmt( *pFmt, *pNew, pTable );
            AdjustFrmSize( pFmt );
            pFmt = pNew;
            aFile.nFlags |= SWGF_NO_FRAMES;
        }
    }
    // Default-Werte setzen:
    SwTableLine* pLine = new SwTableLine( pFmt, nBoxes, pUpper);
    rLines.C40_INSERT( SwTableLine, pLine, (USHORT) idx );
    SwTableBoxes& rBoxes = pLine->GetTabBoxes();
    while( r.cur() == SWG_COMMENT || r.cur() == SWG_DATA ) r.skipnext();
    // Und nun die einzelnen Zellen:
    for (int i = 0; i < nBoxes && r.good(); i++ )
    {
        if( r.cur() != SWG_TABLEBOX )
        {
            Error(); return;
        }
        InTableBox( rBoxes, i, pLine, rPos, pTable );
    }

}

// rPos zeigt anschliessend hinter die Tabelle

void SwSwgReader::InTable( SwNodeIndex& rPos )
{
    if( !bNew && pDoc->IsIdxInTbl( rPos ) )
    {
        r.skipnext(); return;
    }
    BYTE cFlags;
    USHORT nLines, nBoxes;
    r >> cFlags >> nLines >> nBoxes;
//JP 16.02.99: ueberfluessiges Flag
//  BOOL bModified = BOOL( ( cFlags & 0x01 ) != 0 );
    BOOL bHdRepeat = BOOL( ( cFlags & 0x02 ) != 0 );
    // Die Strukturen im Nodes-Array erzeugen
    // Erzeugen von:
    // TableNode..StartNode..TxtNode..EndNode.. (weitere Boxes) ..EndNode
    SwTableNode* pNd = pDoc->GetNodes().InsertTable( rPos, nBoxes,
                                (SwTxtFmtColl*)pDoc->GetDfltTxtFmtColl() );
    if( !pNd )
    {
        r.skip(); Error(); return;
    }
    rPos = *pNd;
    SwTable* pTbl = &pNd->GetTable();
//JP 16.02.99: ueberfluessiges Flag
//  if( bModified )
//      pTbl->SetModified();
    pTbl->SetHeadlineRepeat( bHdRepeat );
    // Das Frame-Format der Tabelle einlesen
    USHORT nFrmFmt;
    r >> nFrmFmt;
    r.next();
    SwFrmFmt* pFmt = pDoc->MakeTblFrmFmt( pDoc->GetUniqueTblName(),
                                        pDoc->GetDfltFrmFmt() );
    if( aHdr.nVersion >= SWG_VER_LCLFMT )
    {
        while( r.cur() == SWG_FRAMEFMT && r.good() )
        {
            InFormat( pFmt );
            RegisterFmt( *pFmt, pTbl );
            nFrmFmt = IDX_NO_VALUE;
        }
    }
    // Wenn ein Frame-Fmt angegeben wurde, muss ein neues erzeugt werden.
    BOOL bGetTblSize = FALSE;

    if( nFrmFmt != IDX_NO_VALUE )
    {
        SwFrmFmt* pLastFmt = (SwFrmFmt*) FindFmt( nFrmFmt, SWG_FRAMEFMT );
        if( !pLastFmt )
        {
//JP 25.03.97: Bug ## - wenn keine TabellenAttribute gefunden werden
//              dann setze die DefaultSize fuer eine Tabelle.
//          Error(); return;
            // die Tabelle bekommt erstmal USHRT_MAX als default Size
            // die richtige Size wird am Ende von der 1. Line besorgt
            pFmt->SetAttr( SwFmtFrmSize( ATT_VAR_SIZE, USHRT_MAX ));
            bGetTblSize = TRUE;
        }
        else
        {
            // Attribute hart hineinkopieren
            ((SfxItemSet&)pFmt->GetAttrSet()).Put( pLastFmt->GetAttrSet() );
            pFmt->nFmtId = nFrmFmt;
            ReRegisterFmt( *pLastFmt, *pFmt, pTbl );
            aFile.nFlags |= SWGF_NO_FRAMES;
        }
    }
    // Da einige FRMSIZE-Attribute zu grosse Werte enthalten,
    // muessen die Werte heruntergeteilt werden.
    nFrmSizeAdj = 1;
    if( SFX_ITEM_SET == pFmt->GetAttrSet().GetItemState(RES_FRM_SIZE, FALSE ))
    {
        SwFmtFrmSize aSz( pFmt->GetFrmSize() );
        SwTwips h = aSz.GetHeight();
        SwTwips w = aSz.GetWidth();
        while( h > 65535L || w > 65535L )
            w /= 2, h /= 2, nFrmSizeAdj *= 2;
        aSz.SetWidth( w );
        aSz.SetHeight( h );
        pFmt->SetAttr( aSz );
    }
    pFmt->Add( pTbl );

    // Ist es eine DDE-Tabelle?
    SwDDEFieldType* pDDE = 0;
    if( r.cur() == SWG_FIELDTYPES )
    {
        pDDE = (SwDDEFieldType*) InFieldType();
        r.next();
    }
    // Vordefinierte Layout-Frames loeschen
    // beim Einfuegen stoeren diese zur Zeit
    if( !bNew ) pNd->DelFrms();
    // Die einzelnen Zeilen einlesen
    // aIdx zeigt auf den Startnode der ersten Box
    rPos = pNd->GetIndex() + 1;

    while( r.cur() == SWG_COMMENT || r.cur() == SWG_DATA ) r.skipnext();
    SwTableLines& rLines = pTbl->GetTabLines();
    for (USHORT i = 0; i < nLines && r.good(); i++ )
    {
        if( r.cur() != SWG_TABLELINE )
        {
            Error(); return;
        }
        InTableLine( rLines, NULL, i, rPos, pTbl );
    }

    if( bGetTblSize )
    {
        SwTwips nAktSize = 0;
        const SwTableBoxes& rBoxes = rLines[ 0 ]->GetTabBoxes();
        for( USHORT n = rBoxes.Count(); n; )
            nAktSize += rBoxes[ --n ]->GetFrmFmt()->GetFrmSize().GetWidth();

        //
        pFmt->LockModify();
        pFmt->SetAttr( SwFmtFrmSize( ATT_VAR_SIZE, nAktSize ));
        pFmt->UnlockModify();
    }

    rPos = pNd->EndOfSectionIndex()+1;
    if( pDDE )
    {
        // tauschen am Node den Tabellen-Pointer aus
        SwDDETable* pNewTable = new SwDDETable( pNd->GetTable(),
                                                (SwDDEFieldType*) pDDE );
        pNd->SetNewTable( pNewTable, FALSE );
    }

    // Layout-Frames wieder erzeugen, falls eingefuegt
    // und falls (MA #$.!) die Tbl nicht im FlyFrm ist
    if( !bNew && !nFlyLevel && !(SWGRD_FORCE & nOptions) )
    {
        SwNodeIndex aIdx( *pNd->EndOfSectionNode() );
        pDoc->GetNodes().GoNext( &aIdx );
        pNd->MakeFrms( &aIdx );
    }
    nFrmSizeAdj = 1;
}


// Registrieren einer Section. Der SwIndex zeigt auf den StartNode.

USHORT SwSwgReader::RegisterSection( const SwNodeIndex& rIdx, USHORT nId )
{
    SectionInfo* p;
    if( nSect == nSectSize )
    {
        // kein Platz mehr: um 128 erweitern
        p = new SectionInfo [ nSectSize + 128 ];
        memcpy( p, pSects, nSectSize * sizeof( SectionInfo ) );
        delete pSects;
        pSects = p;
        nSectSize += 128;
    }
    p = &pSects[ nSect ];
    p->nId = ( nId == IDX_NO_VALUE ) ? nSect : nId;
    // Nimm den ersten Node hinter dem StartNode.
    // Dadurch laesst sich StartOfSection() anwenden.
    p->pNode1 = pDoc->GetNodes()[ rIdx.GetIndex() + 1 ];
    nSect++;
    return p->nId;

}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
