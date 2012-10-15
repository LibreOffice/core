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


#include <hintids.hxx>
#include <vcl/cmdevt.hxx>
#include <unotools/charclass.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <swwait.hxx>
#include <fmtsrnd.hxx>
#include <fmtinfmt.hxx>
#include <txtinet.hxx>
#include <frmfmt.hxx>
#include <charfmt.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <docary.hxx>
#include <editsh.hxx>
#include <frame.hxx>
#include <cntfrm.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>            // fuer SwTxtNode
#include <grfatr.hxx>
#include <flyfrm.hxx>
#include <swtable.hxx>
#include <swundo.hxx>           // UNDO_START, UNDO_END
#include <calc.hxx>
#include <edimp.hxx>
#include <ndgrf.hxx>
#include <ndole.hxx>
#include <txtfrm.hxx>
#include <rootfrm.hxx>
#include <extinput.hxx>
#include <crsskip.hxx>
#include <scriptinfo.hxx>
#include <unocrsrhelper.hxx>
#include <section.hxx>
#include <unochart.hxx>
#include <numrule.hxx>
#include <SwNodeNum.hxx>
#include <unocrsr.hxx>
#include <switerator.hxx>

using namespace com::sun::star;


/******************************************************************************
 *                      void SwEditShell::Insert(char c)
 ******************************************************************************/


void SwEditShell::Insert( sal_Unicode c, sal_Bool bOnlyCurrCrsr )
{
    StartAllAction();
    FOREACHPAM_START(this)

        const bool bSuccess = GetDoc()->InsertString(*PCURCRSR, rtl::OUString(c));
        OSL_ENSURE( bSuccess, "Doc->Insert() failed." );
        (void) bSuccess;

        SaveTblBoxCntnt( PCURCRSR->GetPoint() );
        if( bOnlyCurrCrsr )
            break;

    FOREACHPAM_END()

    EndAllAction();
}


/******************************************************************************
 *                void SwEditShell::Insert(const String &rStr)
 ******************************************************************************/


void SwEditShell::Insert2(const String &rStr, const bool bForceExpandHints )
{
    StartAllAction();
    {
        const enum IDocumentContentOperations::InsertFlags nInsertFlags =
            (bForceExpandHints)
            ? static_cast<IDocumentContentOperations::InsertFlags>(
                    IDocumentContentOperations::INS_FORCEHINTEXPAND |
                    IDocumentContentOperations::INS_EMPTYEXPAND)
            : IDocumentContentOperations::INS_EMPTYEXPAND;

        SwPaM *_pStartCrsr = getShellCrsr( true ), *__pStartCrsr = _pStartCrsr;
        do {
            //OPT: GetSystemCharSet
            const bool bSuccess =
                GetDoc()->InsertString(*_pStartCrsr, rStr, nInsertFlags);
            OSL_ENSURE( bSuccess, "Doc->Insert() failed." );
            (void) bSuccess;

            GetDoc()->UpdateRsid( *_pStartCrsr, rStr.Len() );

            // Set paragraph rsid if beginning of paragraph
            SwTxtNode *pTxtNode = _pStartCrsr->GetPoint()->nNode.GetNode().GetTxtNode();
            if( pTxtNode && pTxtNode->Len() == 1)
                GetDoc()->UpdateParRsid( pTxtNode );

            SaveTblBoxCntnt( _pStartCrsr->GetPoint() );

        } while( (_pStartCrsr=(SwPaM *)_pStartCrsr->GetNext()) != __pStartCrsr );
    }

    // calculate cursor bidi level
    SwCursor* pTmpCrsr = _GetCrsr();
    const sal_Bool bDoNotSetBidiLevel = ! pTmpCrsr ||
                                ( 0 != dynamic_cast<SwUnoCrsr*>(pTmpCrsr) );

    if ( ! bDoNotSetBidiLevel )
    {
        SwNode& rNode = pTmpCrsr->GetPoint()->nNode.GetNode();
        if ( rNode.IsTxtNode() )
        {
            SwIndex& rIdx = pTmpCrsr->GetPoint()->nContent;
            xub_StrLen nPos = rIdx.GetIndex();
            xub_StrLen nPrevPos = nPos;
            if ( nPrevPos )
                --nPrevPos;

            SwScriptInfo* pSI = SwScriptInfo::GetScriptInfo( ((SwTxtNode&)rNode),
                                                              sal_True );

            sal_uInt8 nLevel = 0;
            if ( ! pSI )
            {
                // seems to be an empty paragraph.
                Point aPt;
                SwCntntFrm* pFrm =
                        ((SwTxtNode&)rNode).getLayoutFrm( GetLayout(), &aPt, pTmpCrsr->GetPoint(),
                                                    sal_False );

                SwScriptInfo aScriptInfo;
                aScriptInfo.InitScriptInfo( (SwTxtNode&)rNode, pFrm->IsRightToLeft() );
                nLevel = aScriptInfo.DirType( nPrevPos );
            }
            else
            {
                if ( STRING_LEN != pSI->GetInvalidity() )
                    pSI->InitScriptInfo( (SwTxtNode&)rNode );
                nLevel = pSI->DirType( nPrevPos );
            }

            pTmpCrsr->SetCrsrBidiLevel( nLevel );
        }
    }

    SetInFrontOfLabel( false ); // #i27615#

    EndAllAction();
}


/******************************************************************************
 *              void SwEditShell::Overwrite(const String &rStr)
 ******************************************************************************/


void SwEditShell::Overwrite(const String &rStr)
{
    StartAllAction();
    FOREACHPAM_START(this)
        if( !GetDoc()->Overwrite(*PCURCRSR, rStr ) )
        {
            OSL_FAIL( "Doc->Overwrite(Str) failed." );
        }
        SaveTblBoxCntnt( PCURCRSR->GetPoint() );
    FOREACHPAM_END()
    EndAllAction();
}


/******************************************************************************
 *                      long SwEditShell::SplitNode()
 ******************************************************************************/

long SwEditShell::SplitNode( sal_Bool bAutoFormat, sal_Bool bCheckTableStart )
{
    StartAllAction();
    GetDoc()->GetIDocumentUndoRedo().StartUndo(UNDO_EMPTY, NULL);

    FOREACHPAM_START(this)
        // eine Tabellen Zelle wird jetzt zu einer normalen Textzelle!
        GetDoc()->ClearBoxNumAttrs( PCURCRSR->GetPoint()->nNode );
        GetDoc()->SplitNode( *PCURCRSR->GetPoint(), bCheckTableStart );
    FOREACHPAM_END()

    GetDoc()->GetIDocumentUndoRedo().EndUndo(UNDO_EMPTY, NULL);

    if( bAutoFormat )
        AutoFmtBySplitNode();

    ClearTblBoxCntnt();

    EndAllAction();
    return(1L);
}

sal_Bool SwEditShell::AppendTxtNode()
{
    sal_Bool bRet = sal_False;
    StartAllAction();
    GetDoc()->GetIDocumentUndoRedo().StartUndo(UNDO_EMPTY, NULL);

    FOREACHPAM_START(this)
        GetDoc()->ClearBoxNumAttrs( PCURCRSR->GetPoint()->nNode );
        bRet = GetDoc()->AppendTxtNode( *PCURCRSR->GetPoint()) || bRet;
    FOREACHPAM_END()

    GetDoc()->GetIDocumentUndoRedo().EndUndo(UNDO_EMPTY, NULL);

    ClearTblBoxCntnt();

    EndAllAction();
    return bRet;
}

/******************************************************************************
 *        liefert einen Pointer auf einen SwGrfNode; dieser wird von
 *              GetGraphic() und GetGraphicSize() verwendet.
 ******************************************************************************/


SwGrfNode * SwEditShell::_GetGrfNode() const
{
    SwGrfNode *pGrfNode = 0;
    SwPaM* pCrsr = GetCrsr();
    if( !pCrsr->HasMark() ||
        pCrsr->GetPoint()->nNode == pCrsr->GetMark()->nNode )
        pGrfNode = pCrsr->GetPoint()->nNode.GetNode().GetGrfNode();

    return pGrfNode;
}
/******************************************************************************
 *      liefert Pointer auf eine Graphic, wenn CurCrsr->GetPoint() auf
 *           einen SwGrfNode zeigt (und GetMark nicht gesetzt ist
 *                   oder auf die gleiche Graphic zeigt)
 ******************************************************************************/

const Graphic* SwEditShell::GetGraphic( sal_Bool bWait ) const
{
    SwGrfNode* pGrfNode = _GetGrfNode();
    const Graphic* pGrf( 0L );
    if ( pGrfNode )
    {
        pGrf = &(pGrfNode->GetGrf());
        // --> #i73788#
        // no load of linked graphic, if its not needed now (bWait = sal_False).
        if ( bWait )
        {
            if( pGrf->IsSwapOut() ||
                ( pGrfNode->IsLinkedFile() && GRAPHIC_DEFAULT == pGrf->GetType() ) )
            {
                bool const bResult = pGrfNode->SwapIn(bWait);
                OSL_ENSURE(bResult || !bWait, "Graphic could not be loaded" );
                (void) bResult; // unused in non-debug
            }
        }
        else
        {
            if ( pGrf->IsSwapOut() && !pGrfNode->IsLinkedFile() )
            {
                bool const bResult = pGrfNode->SwapIn(bWait);
                OSL_ENSURE(bResult || !bWait, "Graphic could not be loaded" );
                (void) bResult; // unused in non-debug
            }
        }
    }
    return pGrf;
}

sal_Bool SwEditShell::IsGrfSwapOut( sal_Bool bOnlyLinked ) const
{
    SwGrfNode *pGrfNode = _GetGrfNode();
    return pGrfNode &&
        (bOnlyLinked ? ( pGrfNode->IsLinkedFile() &&
                        ( GRAPHIC_DEFAULT == pGrfNode->GetGrfObj().GetType()||
                          pGrfNode->GetGrfObj().IsSwappedOut()))
                     : pGrfNode->GetGrfObj().IsSwappedOut());
}

const GraphicObject* SwEditShell::GetGraphicObj() const
{
    SwGrfNode* pGrfNode = _GetGrfNode();
    return pGrfNode ? &(pGrfNode->GetGrfObj()) : 0L;
}

sal_uInt16 SwEditShell::GetGraphicType() const
{
    SwGrfNode *pGrfNode = _GetGrfNode();
    return static_cast<sal_uInt16>(pGrfNode ? pGrfNode->GetGrfObj().GetType() : GRAPHIC_NONE);
}

/******************************************************************************
 *      liefert die Groesse der Graphic, wenn CurCrsr->GetPoint() auf
 *          einen SwGrfNode zeigt (und GetMark nicht gesetzt ist
 *                  oder auf die gleiche Graphic zeigt)
 ******************************************************************************/

sal_Bool SwEditShell::GetGrfSize(Size& rSz) const
{
    SwNoTxtNode* pNoTxtNd;
    SwPaM* pCurrentCrsr = GetCrsr();
    if( ( !pCurrentCrsr->HasMark()
         || pCurrentCrsr->GetPoint()->nNode == pCurrentCrsr->GetMark()->nNode )
         && 0 != ( pNoTxtNd = pCurrentCrsr->GetNode()->GetNoTxtNode() ) )
    {
        rSz = pNoTxtNd->GetTwipSize();
        return sal_True;
    }
    return sal_False;

}
/******************************************************************************
 *      erneutes Einlesen, falls Graphic nicht Ok ist. Die
 *      aktuelle wird durch die neue ersetzt.
 ******************************************************************************/

void SwEditShell::ReRead( const String& rGrfName, const String& rFltName,
                    const Graphic* pGraphic, const GraphicObject* pGrfObj )
{
    StartAllAction();
    pDoc->ReRead( *GetCrsr(), rGrfName, rFltName, pGraphic, pGrfObj );
    EndAllAction();
}


/******************************************************************************
 *  liefert den Namen und den FilterNamen einer Graphic, wenn der Cursor
 *  auf einer Graphic steht
 *  Ist ein String-Ptr != 0 dann returne den entsp. Namen
 ******************************************************************************/


void SwEditShell::GetGrfNms( String* pGrfName, String* pFltName,
                            const SwFlyFrmFmt* pFmt ) const
{
    OSL_ENSURE( pGrfName || pFltName, "was wird denn nun erfragt?" );
    if( pFmt )
        GetDoc()->GetGrfNms( *pFmt, pGrfName, pFltName );
    else
    {
        SwGrfNode *pGrfNode = _GetGrfNode();
        if( pGrfNode && pGrfNode->IsLinkedFile() )
            pGrfNode->GetFileFilterNms( pGrfName, pFltName );
    }
}


const PolyPolygon *SwEditShell::GetGraphicPolygon() const
{
    SwNoTxtNode *pNd = GetCrsr()->GetNode()->GetNoTxtNode();
    return pNd->HasContour();
}


void SwEditShell::SetGraphicPolygon( const PolyPolygon *pPoly )
{
    SwNoTxtNode *pNd = GetCrsr()->GetNode()->GetNoTxtNode();
    StartAllAction();
    pNd->SetContour( pPoly );
    SwFlyFrm *pFly = (SwFlyFrm*)pNd->getLayoutFrm(GetLayout())->GetUpper();
    const SwFmtSurround &rSur = pFly->GetFmt()->GetSurround();
    pFly->GetFmt()->NotifyClients( (SwFmtSurround*)&rSur, (SwFmtSurround*)&rSur );
    GetDoc()->SetModified();
    EndAllAction();
}

void SwEditShell::ClearAutomaticContour()
{
    SwNoTxtNode *pNd = GetCrsr()->GetNode()->GetNoTxtNode();
    OSL_ENSURE( pNd, "is no NoTxtNode!" );
    if( pNd->HasAutomaticContour() )
    {
        StartAllAction();
        pNd->SetContour( NULL, sal_False );
        SwFlyFrm *pFly = (SwFlyFrm*)pNd->getLayoutFrm(GetLayout())->GetUpper();
        const SwFmtSurround &rSur = pFly->GetFmt()->GetSurround();
        pFly->GetFmt()->NotifyClients( (SwFmtSurround*)&rSur, (SwFmtSurround*)&rSur );
        GetDoc()->SetModified();
        EndAllAction();
    }
}

/******************************************************************************
 *      liefert Pointer auf ein SvInPlaceObjectRef, wenn CurCrsr->GetPoint() auf
 *          einen SwOLENode zeigt (und GetMark nicht gesetzt ist
 *                  oder auf das gleiche SvInPlaceObjectRef zeigt)
 *      besorgt den Pointer vom Doc wenn das Objekt per Namen gesucht werden
 *      soll
 ******************************************************************************/

svt::EmbeddedObjectRef& SwEditShell::GetOLEObject() const
{
    OSL_ENSURE(  CNT_OLE == GetCntType(), "GetOLEObj: kein OLENode." );
    OSL_ENSURE( !GetCrsr()->HasMark() ||
            (GetCrsr()->HasMark() &&
                GetCrsr()->GetPoint()->nNode == GetCrsr()->GetMark()->nNode),
            "GetOLEObj: kein OLENode." );

    SwOLENode *pOLENode = GetCrsr()->GetNode()->GetOLENode();
    OSL_ENSURE( pOLENode, "GetOLEObj: kein OLENode." );
    SwOLEObj& rOObj = pOLENode->GetOLEObj();
    return rOObj.GetObject();
}


sal_Bool SwEditShell::HasOLEObj( const String &rName ) const
{
    SwStartNode *pStNd;
    SwNodeIndex aIdx( *GetNodes().GetEndOfAutotext().StartOfSectionNode(), 1 );
    while ( 0 != (pStNd = aIdx.GetNode().GetStartNode()) )
    {
        ++aIdx;
        SwNode& rNd = aIdx.GetNode();
        if( rNd.IsOLENode() &&
            rName == ((SwOLENode&)rNd).GetChartTblName() &&
            ((SwOLENode&)rNd).getLayoutFrm( GetLayout() ) )
            return sal_True;

        aIdx.Assign( *pStNd->EndOfSectionNode(), + 1 );
    }
    return sal_False;
}


void SwEditShell::SetChartName( const String &rName )
{
    SwOLENode *pONd = GetCrsr()->GetNode()->GetOLENode();
    OSL_ENSURE( pONd, "ChartNode not found" );
    pONd->SetChartTblName( rName );
}

void SwEditShell::UpdateCharts( const String &rName )
{
    GetDoc()->UpdateCharts( rName );
}


/******************************************************************************
 *      Aenderung des Tabellennamens
 ******************************************************************************/

void SwEditShell::SetTableName( SwFrmFmt& rTblFmt, const String &rNewName )
{
    GetDoc()->SetTableName( rTblFmt, rNewName );
}

// erfragen des akt. Wortes

String SwEditShell::GetCurWord()
{
    const SwPaM& rPaM = *GetCrsr();
    const SwTxtNode* pNd = rPaM.GetNode()->GetTxtNode();
    String aString = pNd ?
                     pNd->GetCurWord(rPaM.GetPoint()->nContent.GetIndex()) :
                     aEmptyStr;
    return aString;
}

/****************************************************************************
 *           void SwEditShell::UpdateDocStat()
 ****************************************************************************/


void SwEditShell::UpdateDocStat( )
{
    StartAllAction();
    GetDoc()->UpdateDocStat( );
    EndAllAction();
}

const SwDocStat& SwEditShell::GetUpdatedDocStat()
{
    StartAllAction();
    const SwDocStat &rRet = GetDoc()->GetUpdatedDocStat();
    EndAllAction();
    return rRet;
}

// OPT: eddocinl.cxx


    // returne zum Namen die im Doc gesetzte Referenz
const SwFmtRefMark* SwEditShell::GetRefMark( const String& rName ) const
{
    return GetDoc()->GetRefMark( rName );
}

    // returne die Namen aller im Doc gesetzten Referenzen
sal_uInt16 SwEditShell::GetRefMarks( std::vector<rtl::OUString>* pStrings ) const
{
    return GetDoc()->GetRefMarks( pStrings );
}

/******************************************************************************
 *          DropCap-SS
 ******************************************************************************/


String SwEditShell::GetDropTxt( const sal_uInt16 nChars ) const
{
    /**
     * pb: made changes for #i74939#
     *
     * always return a string even though there is a selection
     */

    String aTxt;
    SwPaM* pCrsr = GetCrsr();
    if ( IsMultiSelection() )
    {
        // if a multi selection exists, search for the first line
        // -> it is the cursor with the lowest index
        sal_uLong nIndex = pCrsr->GetMark()->nNode.GetIndex();
        bool bPrev = true;
        SwPaM* pLast = pCrsr;
        SwPaM* pTemp = pCrsr;
        while ( bPrev )
        {
            SwPaM* pPrev2 = dynamic_cast< SwPaM* >( pTemp->GetPrev() );
            bPrev = ( pPrev2 && pPrev2 != pLast );
            if ( bPrev )
            {
                pTemp = pPrev2;
                sal_uLong nTemp = pPrev2->GetMark()->nNode.GetIndex();
                if ( nTemp < nIndex )
                {
                    nIndex = nTemp;
                    pCrsr = pPrev2;
                }
            }
        }
    }

    SwTxtNode* pTxtNd = pCrsr->GetNode( !pCrsr->HasMark() )->GetTxtNode();
    if( pTxtNd )
    {
        xub_StrLen nDropLen = pTxtNd->GetDropLen( nChars );
        if( nDropLen )
            aTxt = pTxtNd->GetTxt().Copy( 0, nDropLen );
    }

    return aTxt;
}

void SwEditShell::ReplaceDropTxt( const String &rStr, SwPaM* pPaM )
{
    SwPaM* pCrsr = pPaM ? pPaM : GetCrsr();
    if( pCrsr->GetPoint()->nNode == pCrsr->GetMark()->nNode &&
        pCrsr->GetNode()->GetTxtNode()->IsTxtNode() )
    {
        StartAllAction();

        const SwNodeIndex& rNd = pCrsr->GetPoint()->nNode;
        SwPaM aPam( rNd, rStr.Len(), rNd, 0 );
        if( !GetDoc()->Overwrite( aPam, rStr ) )
        {
            OSL_FAIL( "Doc->Overwrite(Str) failed." );
        }

        EndAllAction();
    }
}


String SwEditShell::Calculate()
{
    String  aFormel;                    // die entgueltige Formel
    SwPaM   *pPaMLast = (SwPaM*)GetCrsr()->GetNext(),
            *pPaM = pPaMLast;           // die Pointer auf Cursor
    SwCalc  aCalc( *GetDoc() );
    const CharClass& rCC = GetAppCharClass();

    do {
        SwTxtNode* pTxtNd = pPaM->GetNode()->GetTxtNode();
        if(pTxtNd)
        {
            const SwPosition *pStart = pPaM->Start(), *pEnd = pPaM->End();
            xub_StrLen nStt = pStart->nContent.GetIndex();
            String aStr = pTxtNd->GetExpandTxt( nStt, pEnd->nContent.
                                                GetIndex() - nStt );

            aStr = rCC.lowercase( aStr );

            sal_Unicode ch;
            sal_Bool    bValidFlds = sal_False;
            xub_StrLen nPos = 0;

            while( nPos < aStr.Len() )
            {
                ch = aStr.GetChar( nPos++ );
                if( rCC.isLetter( aStr, nPos-1 ) || ch == '_' )
                {
                    xub_StrLen nTmpStt = nPos-1;
                    while(  nPos < aStr.Len() &&
                            0 != ( ch = aStr.GetChar( nPos++ )) &&
                           (rCC.isLetterNumeric( aStr, nPos - 1 ) ||
                               ch == '_'|| ch == '.' ))
                        ;

                    if( nPos < aStr.Len() )
                        --nPos;

                    String sVar( aStr.Copy( nTmpStt, nPos - nTmpStt ));
                    if( !::FindOperator( sVar ) &&
                        (::Find( sVar, aCalc.GetVarTable(),TBLSZ) ||
                         aCalc.VarLook( sVar )) )
                    {
                        if( !bValidFlds )
                        {
                            GetDoc()->FldsToCalc( aCalc,
                                                  pStart->nNode.GetIndex(),
                                                  pStart->nContent.GetIndex() );
                            bValidFlds = sal_True;
                        }
                        (( aFormel += '(' ) +=
                                aCalc.GetStrResult( aCalc.VarLook( sVar )
                                                        ->nValue )) += ')';
                    }
                    else
                        aFormel += sVar;
                }
                else
                    aFormel += ch;
            }
        }
    } while( pPaMLast != (pPaM = (SwPaM*)pPaM->GetNext()) );

    return aCalc.GetStrResult( aCalc.Calculate(aFormel) );
}


sfx2::LinkManager& SwEditShell::GetLinkManager()
{
    return pDoc->GetLinkManager();
}


void *SwEditShell::GetIMapInventor() const
{
    //Als eindeutige Identifikation sollte der Node, auf dem der Crsr steht
    //genuegen.
    return (void*)GetCrsr()->GetNode();
}

// #i73788#
// remove default parameter, because method is always called this default value
Graphic SwEditShell::GetIMapGraphic() const
{
    //Liefert immer eine Graphic, wenn der Crsr in einem Fly steht.
    SET_CURR_SHELL( (ViewShell*)this );
    Graphic aRet;
    SwPaM* pCrsr = GetCrsr();
    if ( !pCrsr->HasMark() )
    {
        SwNode *pNd =pCrsr->GetNode();
        if( pNd->IsGrfNode() )
        {
            SwGrfNode & rGrfNode(*static_cast<SwGrfNode*>(pNd));
            const Graphic& rGrf = rGrfNode.GetGrf();
            if( rGrf.IsSwapOut() || ( rGrfNode.IsLinkedFile() &&
                                    GRAPHIC_DEFAULT == rGrf.GetType() ) )
            {
                bool const bResult = rGrfNode.SwapIn(true);
                OSL_ENSURE(bResult, "Graphic could not be loaded" );
                (void) bResult; // unused in non-debug
            }
            aRet = rGrf;
        }
        else if ( pNd->IsOLENode() )
        {
            aRet = *((SwOLENode*)pNd)->GetGraphic();
        }
        else
        {
            SwFlyFrm* pFlyFrm = pNd->GetCntntNode()->getLayoutFrm( GetLayout() )->FindFlyFrm();
            if(pFlyFrm)
                aRet = pFlyFrm->GetFmt()->MakeGraphic();
        }
    }
    return aRet;
}


sal_Bool SwEditShell::InsertURL( const SwFmtINetFmt& rFmt, const String& rStr, sal_Bool bKeepSelection )
{
    // URL und Hinweistext (direkt oder via Selektion) notwendig
    if( !rFmt.GetValue().Len() ||   ( !rStr.Len() && !HasSelection() ) )
        return sal_False;
    StartAllAction();
    GetDoc()->GetIDocumentUndoRedo().StartUndo( UNDO_UI_INSERT_URLTXT, NULL);
    sal_Bool bInsTxt = sal_True;

    if( rStr.Len() )
    {
        SwPaM* pCrsr = GetCrsr();
        if( pCrsr->HasMark() && *pCrsr->GetPoint() != *pCrsr->GetMark() )
        {
            // Selection vorhanden, MehrfachSelektion?
            sal_Bool bDelTxt = sal_True;
            if( pCrsr->GetNext() == pCrsr )
            {
                // einfach Selection -> Text ueberpruefen
                String sTxt(comphelper::string::stripEnd(GetSelTxt(), ' '));
                if( sTxt == rStr )
                    bDelTxt = bInsTxt = sal_False;
            }
            else if( rFmt.GetValue() == rStr )      // Name und URL gleich?
                bDelTxt = bInsTxt = sal_False;

            if( bDelTxt )
                Delete();
        }
        else if( pCrsr->GetNext() != pCrsr && rFmt.GetValue() == rStr )
            bInsTxt = sal_False;

        if( bInsTxt )
        {
            Insert2( rStr );
            SetMark();
            ExtendSelection( sal_False, rStr.Len() );
        }
    }
    else
        bInsTxt = sal_False;

    SetAttr( rFmt );
    if (bInsTxt && !IsCrsrPtAtEnd())
        SwapPam();
    if(!bKeepSelection)
        ClearMark();
    if( bInsTxt )
        DontExpandFmt();
    GetDoc()->GetIDocumentUndoRedo().EndUndo( UNDO_UI_INSERT_URLTXT, NULL );
    EndAllAction();
    return sal_True;
}


sal_uInt16 SwEditShell::GetINetAttrs( SwGetINetAttrs& rArr )
{
    rArr.clear();

    const SwTxtNode* pTxtNd;
    const SwCharFmts* pFmts = GetDoc()->GetCharFmts();
    for( sal_uInt16 n = pFmts->size(); 1 < n; )
    {
        SwIterator<SwTxtINetFmt,SwCharFmt> aIter(*(*pFmts)[--n]);
        for( SwTxtINetFmt* pFnd = aIter.First(); pFnd; pFnd = aIter.Next() )
        {
            if( 0 != ( pTxtNd = pFnd->GetpTxtNode()) &&
                pTxtNd->GetNodes().IsDocNodes() )
            {
                SwTxtINetFmt& rAttr = *pFnd;
                String sTxt( pTxtNd->GetExpandTxt( *rAttr.GetStart(),
                                    *rAttr.GetEnd() - *rAttr.GetStart() ) );

                sTxt = comphelper::string::remove(sTxt, 0x0a);
                sTxt = comphelper::string::strip(sTxt, ' ');

                if( sTxt.Len() )
                {
                    SwGetINetAttr* pNew = new SwGetINetAttr( sTxt, rAttr );
                    rArr.push_back( pNew );
                }
            }
    }
    }
    return rArr.size();
}


    // ist der Cursor in eine INetAttribut, dann wird das komplett
    // geloescht; inclusive des Hinweistextes (wird beim Drag&Drop gebraucht)
sal_Bool SwEditShell::DelINetAttrWithText()
{
    sal_Bool bRet = SelectTxtAttr( RES_TXTATR_INETFMT, sal_False );
    if( bRet )
        DeleteSel( *GetCrsr() );
    return bRet;
}


// setzen an den Textzeichenattributen das DontExpand-Flag
sal_Bool SwEditShell::DontExpandFmt()
{
    sal_Bool bRet = sal_False;
    if( !IsTableMode() && GetDoc()->DontExpandFmt( *GetCrsr()->GetPoint() ))
    {
        bRet = sal_True;
        CallChgLnk();
    }
    return bRet;
}

SvNumberFormatter* SwEditShell::GetNumberFormatter()
{
    return GetDoc()->GetNumberFormatter();
}

sal_Bool SwEditShell::RemoveInvisibleContent()
{
    StartAllAction();
    sal_Bool bRet = GetDoc()->RemoveInvisibleContent();
    EndAllAction();
    return bRet;
}
bool SwEditShell::ConvertFieldsToText()
{
    StartAllAction();
    bool bRet = GetDoc()->ConvertFieldsToText();
    EndAllAction();
    return bRet;
}
void SwEditShell::SetNumberingRestart()
{
    StartAllAction();
    Push();
    //iterate over all text contents - body, frames, header, footer, footnote text
    SwPaM* pCrsr = GetCrsr();
    for(sal_uInt16 i = 0; i < 2; i++)
    {
        if(!i)
            MakeFindRange(DOCPOS_START, DOCPOS_END, pCrsr); //body content
        else
            MakeFindRange(DOCPOS_OTHERSTART, DOCPOS_OTHEREND, pCrsr); //extra content
        SwPosition* pSttPos = pCrsr->Start(), *pEndPos = pCrsr->End();
        sal_uLong nCurrNd = pSttPos->nNode.GetIndex();
        sal_uLong nEndNd = pEndPos->nNode.GetIndex();
        if( nCurrNd <= nEndNd )
        {
            SwCntntFrm* pCntFrm;
            sal_Bool bGoOn = sal_True;
            //iterate over all paragraphs
            while( bGoOn )
            {
                SwNode* pNd = GetDoc()->GetNodes()[ nCurrNd ];
                switch( pNd->GetNodeType() )
                {
                case ND_TEXTNODE:
                    if( 0 != ( pCntFrm = ((SwTxtNode*)pNd)->getLayoutFrm( GetLayout() )) )
                    {
                        //jump over hidden frames - ignore protection!
                        if( !((SwTxtFrm*)pCntFrm)->IsHiddenNow() )
                        {
                            //if the node is numbered and the starting value of the numbering equals the
                            //start value of the numbering rule then set this value as hard starting value

                            //get the node num
                            // OD 2005-11-09
                            SwTxtNode* pTxtNd( static_cast<SwTxtNode*>(pNd) );
                            SwNumRule* pNumRule( pTxtNd->GetNumRule() );

                            if ( pNumRule && pTxtNd->GetNum() &&
                                 ( pTxtNd->HasNumber() || pTxtNd->HasBullet() ) &&
                                 pTxtNd->IsCountedInList() &&
                                 !pTxtNd->IsListRestart() &&
                                 pTxtNd->GetNum()->GetNumber() ==
                                    pNumRule->Get( static_cast<sal_uInt16>(pTxtNd->GetActualListLevel()) ).GetStart() )
                            {
                                //now set a the start value as attribute
                                SwPosition aCurrentNode(*pNd);
                                GetDoc()->SetNumRuleStart( aCurrentNode, sal_True );
                            }
                        }
                    }
                    break;
                case ND_SECTIONNODE:
                    // jump over hidden sections  - ignore protection!
                    if(((SwSectionNode*)pNd)->GetSection().IsHidden() )
                        nCurrNd = pNd->EndOfSectionIndex();
                    break;
                case ND_ENDNODE:
                    {
                        break;
                    }
                }

                bGoOn = nCurrNd < nEndNd;
                ++nCurrNd;
            }
        }
    }


    Pop(sal_False);
    EndAllAction();
}

sal_uInt16 SwEditShell::GetLineCount( sal_Bool bActPos )
{
    sal_uInt16 nRet = 0;
    CalcLayout();
    SwPaM* pPam = GetCrsr();
    SwNodeIndex& rPtIdx = pPam->GetPoint()->nNode;
    SwNodeIndex aStart( rPtIdx );
    SwCntntNode* pCNd;
    SwCntntFrm *pCntFrm = 0;
    sal_uLong nTmpPos;

    if( !bActPos )
        aStart = 0;
    else if( rPtIdx > ( nTmpPos = GetDoc()->GetNodes().GetEndOfExtras().GetIndex()) )
        // BodyBereich => Start ist EndOfIcons + 1
        aStart = nTmpPos + 1;
    else
    {
        if( 0 != ( pCNd = pPam->GetCntntNode() ) &&
            0 != ( pCntFrm = pCNd->getLayoutFrm( GetLayout() ) ) )
        {
            const SwStartNode *pTmp;
            if( pCntFrm->IsInFly() )                        // Fly
                pTmp = pCNd->FindFlyStartNode();
            else if( pCntFrm->IsInFtn() )                   // Footnote
                pTmp = pCNd->FindFootnoteStartNode();
            else
            {                                               // Footer/Header
                const sal_uInt16 nTyp = FRM_HEADER | FRM_FOOTER;
                SwFrm* pFrm = pCntFrm;
                while( pFrm && !(pFrm->GetType() & nTyp) )
                    pFrm = pFrm->GetUpper();
                OSL_ENSURE( pFrm, "Wo bin ich?" );
                if( pFrm && ( pFrm->GetType() & FRM_FOOTER ) )
                    pTmp = pCNd->FindFooterStartNode();
                else
                    pTmp = pCNd->FindHeaderStartNode();
            }
            OSL_ENSURE( pTmp, "Missing StartNode" );
            aStart  = *pTmp;
        }
        OSL_ENSURE( pCNd && pCntFrm, "Missing Layout-Information" );
    }

    while( 0 != ( pCNd = GetDoc()->GetNodes().GoNextSection(
                &aStart, sal_True, sal_False )) && ( !bActPos || aStart <= rPtIdx ) )
    {
        if( 0 != ( pCntFrm = pCNd->getLayoutFrm( GetLayout() ) ) && pCntFrm->IsTxtFrm() )
        {
            xub_StrLen nActPos = bActPos && aStart == rPtIdx ?
                pPam->GetPoint()->nContent.GetIndex() : USHRT_MAX;
            nRet = nRet + ((SwTxtFrm*)pCntFrm)->GetLineCount( nActPos );
        }
    }
    return nRet;
}

long SwEditShell::CompareDoc( const SwDoc& rDoc )
{
    StartAllAction();
    long nRet = GetDoc()->CompareDoc( rDoc );
    EndAllAction();
    return nRet;
}

long SwEditShell::MergeDoc( const SwDoc& rDoc )
{
    StartAllAction();
    long nRet = GetDoc()->MergeDoc( rDoc );
    EndAllAction();
    return nRet;
}


const SwFtnInfo& SwEditShell::GetFtnInfo() const
{
    return GetDoc()->GetFtnInfo();
}

void SwEditShell::SetFtnInfo(const SwFtnInfo& rInfo)
{
    StartAllAction();
    SET_CURR_SHELL( this );
    GetDoc()->SetFtnInfo(rInfo);
    CallChgLnk();
    EndAllAction();
}

const SwEndNoteInfo& SwEditShell::GetEndNoteInfo() const
{
    return GetDoc()->GetEndNoteInfo();
}

void SwEditShell::SetEndNoteInfo(const SwEndNoteInfo& rInfo)
{
    StartAllAction();
    SET_CURR_SHELL( this );
    GetDoc()->SetEndNoteInfo(rInfo);
    EndAllAction();
}

const SwLineNumberInfo& SwEditShell::GetLineNumberInfo() const
{
    return GetDoc()->GetLineNumberInfo();
}

void SwEditShell::SetLineNumberInfo(const SwLineNumberInfo& rInfo)
{
    StartAllAction();
    SET_CURR_SHELL( this );
    GetDoc()->SetLineNumberInfo(rInfo);
    AddPaintRect( GetLayout()->Frm() );
    EndAllAction();
}

sal_uInt16 SwEditShell::GetLinkUpdMode(sal_Bool bDocSettings) const
{
    return getIDocumentSettingAccess()->getLinkUpdateMode( !bDocSettings );
}

void SwEditShell::SetLinkUpdMode( sal_uInt16 nMode )
{
    getIDocumentSettingAccess()->setLinkUpdateMode( nMode );
}


// Schnittstelle fuer die TextInputDaten - ( fuer die Texteingabe
// von japanischen/chinesischen Zeichen)
SwExtTextInput* SwEditShell::CreateExtTextInput(LanguageType eInputLanguage)
{
    SwExtTextInput* pRet = GetDoc()->CreateExtTextInput( *GetCrsr() );
    pRet->SetLanguage(eInputLanguage);
    pRet->SetOverwriteCursor( SwCrsrShell::IsOverwriteCrsr() );
    return pRet;
}

String SwEditShell::DeleteExtTextInput( SwExtTextInput* pDel, sal_Bool bInsText )
{
    if( !pDel )
    {
        const SwPosition& rPos = *GetCrsr()->GetPoint();
        pDel = GetDoc()->GetExtTextInput( rPos.nNode.GetNode(),
                                          rPos.nContent.GetIndex() );
        if( !pDel )
        {
            //JP 25.10.2001: under UNIX the cursor is moved before the Input-
            //              Engine event comes in. So take any - normally there
            //              exist only one at the time. -- Task 92016
            pDel = GetDoc()->GetExtTextInput();
        }
    }
    String sRet;
    if( pDel )
    {
        rtl::OUString sTmp;
        SwUnoCursorHelper::GetTextFromPam(*pDel, sTmp);
        sRet = sTmp;
        SET_CURR_SHELL( this );
        StartAllAction();
        pDel->SetInsText( bInsText );
        SetOverwriteCrsr( pDel->IsOverwriteCursor() );
        const SwPosition aPos( *pDel->GetPoint() );
        GetDoc()->DeleteExtTextInput( pDel );

        // In this case, the "replace" function did not set the cursor
        // to the original position. Therefore we have to do this manually.
        if ( ! bInsText && IsOverwriteCrsr() )
            *GetCrsr()->GetPoint() = aPos;

        EndAllAction();
    }
    return sRet;
}

void SwEditShell::SetExtTextInputData( const CommandExtTextInputData& rData )
{
    const SwPosition& rPos = *GetCrsr()->GetPoint();
    SwExtTextInput* pInput = GetDoc()->GetExtTextInput( rPos.nNode.GetNode()
                                                /*, rPos.nContent.GetIndex()*/ );
    if( pInput )
    {
        StartAllAction();
        SET_CURR_SHELL( this );

        if( !rData.IsOnlyCursorChanged() )
            pInput->SetInputData( rData );
        // Cursor positionieren:
        const SwPosition& rStt = *pInput->Start();
        xub_StrLen nNewCrsrPos = rStt.nContent.GetIndex() + rData.GetCursorPos();

        // zwar unschoen aber was hilfts
        ShowCrsr();
        long nDiff = nNewCrsrPos - rPos.nContent.GetIndex();
        if( 0 > nDiff )
            Left( (xub_StrLen)-nDiff, CRSR_SKIP_CHARS );
        else if( 0 < nDiff )
            Right( (xub_StrLen)nDiff, CRSR_SKIP_CHARS );

        SetOverwriteCrsr( rData.IsCursorOverwrite() );

        EndAllAction();

        if( !rData.IsCursorVisible() )  // must be called after the EndAction
            HideCrsr();
    }
}

void SwEditShell::TransliterateText( sal_uInt32 nType )
{
    utl::TransliterationWrapper aTrans( ::comphelper::getProcessServiceFactory(), nType );
    StartAllAction();
    SET_CURR_SHELL( this );

    SwPaM* pCrsr = GetCrsr();
    if( pCrsr->GetNext() != pCrsr )
    {
        GetDoc()->GetIDocumentUndoRedo().StartUndo(UNDO_EMPTY, NULL);
        FOREACHPAM_START( this )

        if( PCURCRSR->HasMark() )
            GetDoc()->TransliterateText( *PCURCRSR, aTrans );

        FOREACHPAM_END()
        GetDoc()->GetIDocumentUndoRedo().EndUndo(UNDO_EMPTY, NULL);
    }
    else
        GetDoc()->TransliterateText( *pCrsr, aTrans );

    EndAllAction();
}

void SwEditShell::CountWords( SwDocStat& rStat ) const
{
    FOREACHPAM_START( this )

        if( PCURCRSR->HasMark() )
            GetDoc()->CountWords( *PCURCRSR, rStat );

    FOREACHPAM_END()
}

void SwEditShell::ApplyViewOptions( const SwViewOption &rOpt )
{
    SwCrsrShell::StartAction();
    ViewShell::ApplyViewOptions( rOpt );
    SwEditShell::EndAction();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
