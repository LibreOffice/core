/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: rolbck.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 10:41:37 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#ifndef _HINTIDS_HXX //autogen
#include <hintids.hxx>
#endif
#ifndef _SFXITEMITER_HXX //autogen
#include <svtools/itemiter.hxx>
#endif
#ifndef _FMTFTN_HXX //autogen
#include <fmtftn.hxx>
#endif
#ifndef _FCHRFMT_HXX //autogen
#include <fchrfmt.hxx>
#endif
#ifndef _FMTFLCNT_HXX //autogen
#include <fmtflcnt.hxx>
#endif
#ifndef _FMTRFMRK_HXX //autogen
#include <fmtrfmrk.hxx>
#endif
#ifndef _FMTFLD_HXX //autogen
#include <fmtfld.hxx>
#endif
#ifndef _FMTPDSC_HXX //autogen
#include <fmtpdsc.hxx>
#endif
#ifndef _TXTFLD_HXX //autogen
#include <txtfld.hxx>
#endif
#ifndef _TXTRFMRK_HXX //autogen
#include <txtrfmrk.hxx>
#endif
#ifndef _TXTTXMRK_HXX //autogen
#include <txttxmrk.hxx>
#endif
#ifndef _TXTFTN_HXX //autogen
#include <txtftn.hxx>
#endif
#ifndef _TXTFLCNT_HXX //autogen
#include <txtflcnt.hxx>
#endif
#ifndef _FMTANCHR_HXX //autogen
#include <fmtanchr.hxx>
#endif
#ifndef _FMTCNCT_HXX //autogen
#include <fmtcnct.hxx>
#endif
#ifndef _FRMFMT_HXX //autogen
#include <frmfmt.hxx>
#endif
#ifndef _FTNIDX_HXX
#include <ftnidx.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>              // SwDoc.GetNodes()
#endif
#ifndef _DOCARY_HXX
#include <docary.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>            // SwTxtNode
#endif
#ifndef _PARATR_HXX
#include <paratr.hxx>           //
#endif
#ifndef _CELLATR_HXX
#include <cellatr.hxx>          //
#endif
#ifndef _FLDBAS_HXX
#include <fldbas.hxx>           // fuer Felder
#endif
#ifndef _PAM_HXX
#include <pam.hxx>              // fuer SwPaM
#endif
#ifndef _SWTABLE_HXX
#include <swtable.hxx>
#endif
#ifndef _ROLBCK_HXX
#include <rolbck.hxx>
#endif
#ifndef _NDGRF_HXX
#include <ndgrf.hxx>            // SwGrfNode
#endif
#ifndef _UNDOBJ_HXX
#include <undobj.hxx>           // fuer UndoDelete
#endif
#ifndef _BOOKMRK_HXX
#include <bookmrk.hxx>          // fuer SwBookmark
#endif
#ifndef _CHARFMT_HXX
#include <charfmt.hxx> // #i27615#
#endif
#ifndef _COMCORE_HRC
#include <comcore.hrc>
#endif
#ifndef _TOOLS_RESID_HXX
#include <tools/resid.hxx>
#endif
#ifndef _UNDO_HRC
#include <undo.hrc>
#endif
#ifndef _SVX_BRKITEM_HXX
#include <svx/brkitem.hxx>
#endif

SV_IMPL_PTRARR( SwpHstry, SwHstryHintPtr)

String SwHstryHint::GetDescription() const
{
    return String();
}


SwSetFmtHint::SwSetFmtHint( const SfxPoolItem* pFmtHt, ULONG nNd )
    :  SwHstryHint( HSTRY_SETFMTHNT ),
       pAttr( pFmtHt->Clone() ),
       nNode( nNd ),
       // --> OD 2007-07-09 #i77372#
       mnNumLvl( NO_NUMBERING ),
       mbIsRestart( false ),
       mnRestartVal( USHRT_MAX ),
       mbIsCounted( false )
       // <--
{
    switch( pAttr->Which() )
    {
    case RES_PAGEDESC:
        ((SwFmtPageDesc*)pAttr)->ChgDefinedIn( 0 );
        break;
    case RES_PARATR_DROP:
        ((SwFmtDrop*)pAttr)->ChgDefinedIn( 0 );
        break;
    case RES_BOXATR_FORMULA:
        {
            //JP 30.07.98: Bug 54295 - Formeln immer im Klartext speichern
            SwTblBoxFormula& rNew = *(SwTblBoxFormula*)pAttr;
            if( rNew.IsIntrnlName() )
            {
                const SwTblBoxFormula& rOld = *(SwTblBoxFormula*)pFmtHt;
                const SwNode* pNd = rOld.GetNodeOfFormula();
                if( pNd && 0 != (pNd = pNd->FindTableNode() ))
                {
                    SwTableFmlUpdate aMsgHnt( &((SwTableNode*)pNd)->GetTable() );
                    aMsgHnt.eFlags = TBL_BOXNAME;
                    rNew.ChgDefinedIn( rOld.GetDefinedIn() );
                    rNew.ChangeState( &aMsgHnt );
                }
            }
            rNew.ChgDefinedIn( 0 );
        }
        break;
    case RES_PARATR_NUMRULE:
        {
            const SwModify* pMod = ((SwNumRuleItem*)pFmtHt)->GetDefinedIn();
            if( pMod && pMod->ISA( SwTxtNode ))
            {
                const SwTxtNode* pTxtNd = static_cast<const SwTxtNode *>(pMod);

                // --> OD 2007-07-09 #i77372#
                mnNumLvl = pTxtNd->GetLevel();
                mbIsRestart = pTxtNd->IsRestart();
                mnRestartVal = pTxtNd->GetStart();
                mbIsCounted = pTxtNd->IsCounted();
                // <--
            }
            ((SwNumRuleItem*)pAttr)->ChgDefinedIn( 0 );
        }
        break;
    }
}

String SwSetFmtHint::GetDescription() const
{
    String aResult ;

    USHORT nWhich = pAttr->Which();
    switch (nWhich)
    {
    case RES_BREAK:
        switch (((SvxFmtBreakItem *) pAttr)->GetBreak())
        {
        case SVX_BREAK_PAGE_BEFORE:
        case SVX_BREAK_PAGE_AFTER:
        case SVX_BREAK_PAGE_BOTH:
            aResult = SW_RES(STR_UNDO_PAGEBREAKS);

            break;
        case SVX_BREAK_COLUMN_BEFORE:
        case SVX_BREAK_COLUMN_AFTER:
        case SVX_BREAK_COLUMN_BOTH:
            aResult = SW_RES(STR_UNDO_COLBRKS);

            break;
        default:
            break;
        }
        break;
    default:
        break;
    }

    return aResult;
}

void SwSetFmtHint::SetInDoc( SwDoc* pDoc, BOOL bTmpSet )
{
    SwNode * pNode = pDoc->GetNodes()[ nNode ];
    if( pNode->IsCntntNode() )
    {
        ((SwCntntNode*)pNode)->SetAttr( *pAttr );

        if( RES_PARATR_NUMRULE == pAttr->Which() && NO_NUMBERING != mnNumLvl)
        {
            SwTxtNode * pTxtNd = static_cast<SwTxtNode *>(pNode);
            // --> OD 2007-07-09 #i77372#
            pTxtNd->SetLevel( mnNumLvl );
            pTxtNd->SetRestart( mbIsRestart );
            pTxtNd->SetStart( mnRestartVal );
            pTxtNd->SetCounted( mbIsCounted );
            // <--
        }
    }
    else if( pNode->IsTableNode() )
        ((SwTableNode*)pNode)->GetTable().GetFrmFmt()->SetAttr( *pAttr );
    else if( pNode->IsStartNode() &&
            SwTableBoxStartNode == ((SwStartNode*)pNode)->GetStartNodeType() )
    {
        SwTableNode* pTNd = pNode->FindTableNode();
        SwTableBox* pBox;
        if( pTNd && 0 != ( pBox = pTNd->GetTable().GetTblBox( nNode )))
            pBox->ClaimFrmFmt()->SetAttr( *pAttr );
    }

    if( !bTmpSet )
        DELETEZ( pAttr );
}

SwSetFmtHint::~SwSetFmtHint()
{
    delete pAttr;
}


// --> OD 2007-07-11 #i56253#
SwResetFmtHint::SwResetFmtHint( const SfxPoolItem* pFmtHt, ULONG nNodeIdx, SwDoc& rDoc )
// <--
    : SwHstryHint( HSTRY_RESETFMTHNT ),
      nNode( nNodeIdx ),
      nWhich( pFmtHt->Which() ),
      // --> OD 2007-07-11 #i56253#
      mnNumLvl( NO_NUMBERING ),
      mbIsRestart( false ),
      mnRestartVal( USHRT_MAX ),
      mbIsCounted( false )
      // <--
{
    // --> OD 2007-07-11 #i56253#
    if ( nWhich == RES_PARATR_NUMRULE )
    {
        SwTxtNode* pTxtNode = rDoc.GetNodes()[ nNode ]->GetTxtNode();
        if ( pTxtNode )
        {
            mnNumLvl = pTxtNode->GetLevel();
            mbIsRestart = pTxtNode->IsRestart();
            mnRestartVal = pTxtNode->GetStart();
            mbIsCounted = pTxtNode->IsCounted();
        }
    }
    // <--
}


void SwResetFmtHint::SetInDoc( SwDoc* pDoc, BOOL )
{
    SwNode * pNode = pDoc->GetNodes()[ nNode ];
    if ( pNode->IsCntntNode() )
    {
        ((SwCntntNode*)pNode)->ResetAttr( nWhich );
        // --> OD 2007-07-11 #i56253#
        SwTxtNode* pTxtNode = pNode->GetTxtNode();
        if ( pTxtNode &&
             mnNumLvl >= 0 && mnNumLvl < MAXLEVEL )
        {
            pTxtNode->SetLevel( mnNumLvl );
            pTxtNode->SetRestart( mbIsRestart );
            pTxtNode->SetStart( mnRestartVal );
            pTxtNode->SetCounted( mbIsCounted );
        }
        // <--
    }
    else if( pNode->IsTableNode() )
        ((SwTableNode*)pNode)->GetTable().GetFrmFmt()->ResetAttr( nWhich );
}


SwSetTxtHint::SwSetTxtHint( SwTxtAttr* pTxtHt, ULONG nNodePos )
    : SwHstryHint( HSTRY_SETTXTHNT )
{
    // !! Achtung: folgende Attribute erzeugen keine FormatAttribute:
    //  - NoLineBreak, NoHypen, Inserted, Deleted
    // Dafuer muessen Sonderbehandlungen gemacht werden !!!

    // ein bisschen kompliziert, aber ist Ok so: erst vom default
    // eine Kopie und dann die Werte aus dem Text Attribut zuweisen
    USHORT nWhich = pTxtHt->Which();
    if( RES_TXTATR_CHARFMT == nWhich )
        pAttr = new SwFmtCharFmt( pTxtHt->GetCharFmt().GetCharFmt() );
    else
        pAttr = pTxtHt->GetAttr().Clone();
    nNode = nNodePos;
    nStart = *pTxtHt->GetStart();
    nEnd = *pTxtHt->GetAnyEnd();
}


SwSetTxtHint::~SwSetTxtHint()
{
    delete pAttr;
}


void SwSetTxtHint::SetInDoc( SwDoc* pDoc, BOOL )
{
    if( !pAttr )
        return;

    if( RES_TXTATR_CHARFMT == pAttr->Which() )
    {
        // befrage das Doc, ob das CharFmt noch vorhanden ist !
        if( USHRT_MAX == pDoc->GetCharFmts()->GetPos(
                            ((SwFmtCharFmt*)pAttr)->GetCharFmt() ) )
            return;     // nicht setzen, Format nicht vorhanden
    }

    SwTxtNode * pTxtNd = pDoc->GetNodes()[ nNode ]->GetTxtNode();
    ASSERT( pTxtNd, "Undo-TxtAttr: kein TextNode" );

    if( pTxtNd )
        pTxtNd->InsertItem( *pAttr, nStart, nEnd,
                        nsSetAttrMode::SETATTR_NOTXTATRCHR | nsSetAttrMode::SETATTR_NOHINTADJUST );
}


SwSetTxtFldHint::SwSetTxtFldHint( SwTxtFld* pTxtFld, ULONG nNodePos )
    : SwHstryHint( HSTRY_SETTXTFLDHNT ), pFldType( 0 )
{
    pFld = new SwFmtFld( *pTxtFld->GetFld().GetFld() );

    // nur kopieren wenn kein Sys-FieldType
    SwDoc* pDoc = (SwDoc*)pTxtFld->GetTxtNode().GetDoc();

    nFldWhich = pFld->GetFld()->GetTyp()->Which();
    if( nFldWhich == RES_DBFLD ||
        nFldWhich == RES_USERFLD ||
        nFldWhich == RES_SETEXPFLD ||
        nFldWhich == RES_DDEFLD ||
        !pDoc->GetSysFldType( nFldWhich ))
    {
        pFldType = pFld->GetFld()->GetTyp()->Copy();
        pFld->GetFld()->ChgTyp( pFldType );     // Fieldtype umsetzen
    }
    nNode = nNodePos;
    nPos = *pTxtFld->GetStart();
}

String SwSetTxtFldHint::GetDescription() const
{
    return pFld->GetFld()->GetDescription();;
}

SwSetTxtFldHint::~SwSetTxtFldHint()
{
    delete pFld;
    delete pFldType;
}


void SwSetTxtFldHint::SetInDoc( SwDoc* pDoc, BOOL )
{
    if( !pFld )
        return;

    SwFieldType* pNewFldType = pFldType;
    if( !pNewFldType )
        pNewFldType = pDoc->GetSysFldType( nFldWhich );
    else
        // den Type am Dokuement anmelden
        pNewFldType = pDoc->InsertFldType( *pFldType );

    pFld->GetFld()->ChgTyp( pNewFldType );      // Fieldtype umsetzen

    SwTxtNode * pTxtNd = pDoc->GetNodes()[ nNode ]->GetTxtNode();
    ASSERT( pTxtNd, "Undo-TxtAttr: kein TextNode" );

    if( pTxtNd )
        pTxtNd->InsertItem( *pFld, nPos, nPos, nsSetAttrMode::SETATTR_NOTXTATRCHR );
}



SwSetRefMarkHint::SwSetRefMarkHint( SwTxtRefMark* pTxtHt, ULONG nNodePos )
    : SwHstryHint( HSTRY_SETREFMARKHNT )
{
    aRefName = pTxtHt->GetRefMark().GetRefName();
    nNode = nNodePos;
    nStart = *pTxtHt->GetStart();
    nEnd = *pTxtHt->GetAnyEnd();
}


void SwSetRefMarkHint::SetInDoc( SwDoc* pDoc, BOOL )
{
    SwTxtNode * pTxtNd = pDoc->GetNodes()[ nNode ]->GetTxtNode();
    ASSERT( pTxtNd, "Undo-TxtAttr: kein TextNode" );
    if( !pTxtNd )
        return;

    SwFmtRefMark aRefMark( aRefName );

    // existiert hier schon eine Referenz-Markierung ohne Ende, so
    // darf es nicht eingefuegt werden !!
    if( nStart != nEnd || !pTxtNd->GetTxtAttr( nStart, RES_TXTATR_REFMARK ) )
        pTxtNd->InsertItem( aRefMark, nStart, nEnd, nsSetAttrMode::SETATTR_NOTXTATRCHR );
}


SwSetTOXMarkHint::SwSetTOXMarkHint( SwTxtTOXMark* pTxtHt, ULONG nNodePos )
    : SwHstryHint( HSTRY_SETTOXMARKHNT ), aTOXMark( pTxtHt->GetTOXMark() )
{
    aTOXName = aTOXMark.GetTOXType()->GetTypeName();
    eTOXTypes = aTOXMark.GetTOXType()->GetType();
    ((SwModify*)aTOXMark.GetRegisteredIn())->Remove( &aTOXMark );

    nNode = nNodePos;
    nStart = *pTxtHt->GetStart();
    nEnd = *pTxtHt->GetAnyEnd();
}


void SwSetTOXMarkHint::SetInDoc( SwDoc* pDoc, BOOL )
{
    SwTxtNode * pTxtNd = pDoc->GetNodes()[ nNode ]->GetTxtNode();
    ASSERT( pTxtNd, "Undo-TxtAttr: kein TextNode" );
    if( !pTxtNd )
        return;

    // suche den entsprechenden Verzeichnistyp
    USHORT nCnt = pDoc->GetTOXTypeCount( eTOXTypes );
    const SwTOXType* pToxType = 0;
    for( USHORT n = 0; n < nCnt; ++n )
    {
        pToxType = pDoc->GetTOXType( eTOXTypes, n );
        if( pToxType->GetTypeName() == aTOXName )
            break;
        pToxType = 0;
    }

    if( !pToxType )     // Verzeichnistyp nicht gefunden, neu anlegen
        pToxType = pDoc->InsertTOXType( SwTOXType( eTOXTypes, aTOXName ));

    SwTOXMark aNew( aTOXMark );
    ((SwTOXType*)pToxType)->Add( &aNew );

    pTxtNd->InsertItem( aNew, nStart, nEnd, nsSetAttrMode::SETATTR_NOTXTATRCHR );
}


int SwSetTOXMarkHint::IsEqual( const SwTOXMark& rCmp ) const
{
    return aTOXName == rCmp.GetTOXType()->GetTypeName() &&
           eTOXTypes == rCmp.GetTOXType()->GetType() &&
           aTOXMark.GetAlternativeText() == rCmp.GetAlternativeText() &&
           ( TOX_INDEX == eTOXTypes
              ? ( aTOXMark.GetPrimaryKey() == rCmp.GetPrimaryKey() &&
                 aTOXMark.GetSecondaryKey() == rCmp.GetSecondaryKey() )
              : aTOXMark.GetLevel() == rCmp.GetLevel()
           );
}


SwResetTxtHint::SwResetTxtHint( USHORT nWhich, xub_StrLen nAttrStt,
                                xub_StrLen nAttrEnd, ULONG nNodePos )
    : SwHstryHint( HSTRY_RESETTXTHNT ),
    nNode( nNodePos ), nStart( nAttrStt ), nEnd( nAttrEnd ), nAttr( nWhich )
{
}


void SwResetTxtHint::SetInDoc( SwDoc* pDoc, BOOL )
{
    SwTxtNode * pTxtNd = pDoc->GetNodes()[ nNode ]->GetTxtNode();
    ASSERT( pTxtNd, "Undo-TxtAttr: kein TextNode" );
    if( pTxtNd )
        pTxtNd->Delete( nAttr, nStart, nEnd );
}


SwSetFtnHint::SwSetFtnHint( SwTxtFtn* pTxtFtn, ULONG nNodePos )
    : SwHstryHint( HSTRY_SETFTNHNT ),
    nNode( nNodePos ), nStart( *pTxtFtn->GetStart() )
{
    ASSERT( pTxtFtn->GetStartNode(), "Footnote ohne Section" );

    aFtnStr = pTxtFtn->GetFtn().GetNumStr();
    bEndNote = pTxtFtn->GetFtn().IsEndNote();

    // merke die alte NodePos, denn wer weiss was alles in der SaveSection
    // gespeichert (geloescht) wird
    SwDoc* pDoc = (SwDoc*)pTxtFtn->GetTxtNode().GetDoc();
    SwNode* pSaveNd = pDoc->GetNodes()[ nNode ];

    //Pointer auf StartNode der FtnSection merken und erstmal den Pointer im
    //Attribut zuruecksetzen -> Damit werden automatisch die Frms vernichtet.
    SwNodeIndex aSttIdx( *pTxtFtn->GetStartNode() );
    pTxtFtn->SetStartNode( 0, FALSE );

    pUndo = new SwUndoSaveSection;
    pUndo->SaveSection( pDoc, aSttIdx );
    nNode = pSaveNd->GetIndex();
}

SwSetFtnHint::SwSetFtnHint( const SwTxtFtn &rTxtFtn ) :
    SwHstryHint( HSTRY_SETFTNHNT ),
    pUndo( 0 ),
    nNode( _SwTxtFtn_GetIndex( (&rTxtFtn) ) ),
    nStart( *rTxtFtn.GetStart() )
{
    ASSERT( rTxtFtn.GetStartNode(), "Footnote ohne Section" );

    aFtnStr = rTxtFtn.GetFtn().GetNumStr();
    bEndNote = rTxtFtn.GetFtn().IsEndNote();
}

String SwSetFtnHint::GetDescription() const
{
    return SW_RES(STR_FOOTNOTE);
}

SwSetFtnHint::~SwSetFtnHint()
{
    delete pUndo;
}


void SwSetFtnHint::SetInDoc( SwDoc* pDoc, BOOL )
{
    SwTxtNode * pTxtNd = pDoc->GetNodes()[ nNode ]->GetTxtNode();
    ASSERT( pTxtNd, "Undo-TxtAttr: kein TextNode" );
    if( !pTxtNd )
        return;

    if ( pUndo )
    {
        // setze die Fussnote im TextNode
        SwFmtFtn aTemp( bEndNote );
        SwFmtFtn& rNew = (SwFmtFtn&)pDoc->GetAttrPool().Put(aTemp);
        if( aFtnStr.Len() )
            rNew.SetNumStr( aFtnStr );
        SwTxtFtn* pTxtFtn = new SwTxtFtn( rNew, nStart );

        // erzeuge schon die Section der Fussnote
        SwNodeIndex aIdx( *pTxtNd );
        pUndo->RestoreSection( pDoc, &aIdx, SwFootnoteStartNode );
        pTxtFtn->SetStartNode( &aIdx );
        if( pUndo->GetHistory() )
            // erst jetzt die Frames anlegen lassen
            pUndo->GetHistory()->Rollback( pDoc );

        pTxtNd->Insert( pTxtFtn );
    }
    else
    {
        SwTxtFtn *pFtn = (SwTxtFtn*)pTxtNd->GetTxtAttr( nStart );
        SwFmtFtn &rFtn = (SwFmtFtn&)pFtn->GetFtn();
        rFtn.SetNumStr( aFtnStr  );
        if( rFtn.IsEndNote() != bEndNote )
        {
            rFtn.SetEndNote( bEndNote );
            pFtn->CheckCondColl();
        }
    }
}


SwChgFmtColl::SwChgFmtColl( const SwFmtColl* pFmtColl, ULONG nNd,
                            BYTE nNodeWhich )
    : SwHstryHint( HSTRY_CHGFMTCOLL ),
    pColl( pFmtColl ),
    nNode( nNd ),
    nNdWhich( nNodeWhich ),
    // --> OD 2007-07-09 #i77372#
    mnNumLvl( NO_NUMBERING ),
    mbIsRestart( false ),
    mnRestartVal( USHRT_MAX ),
    mbIsCounted( false )
    // <--
{
    const SwDoc* pDoc = pFmtColl->GetDoc();
    const SwTxtNode* pTxtNd = pDoc->GetNodes()[ nNode ]->GetTxtNode();
    if( pTxtNd )
    {
        // --> OD 2007-07-09 #i77372#
        mnNumLvl = pTxtNd->GetLevel();
        mbIsRestart = pTxtNd->IsRestart();
        mnRestartVal = pTxtNd->GetStart();
        mbIsCounted = pTxtNd->IsCounted();
        // <--
    }
}


void SwChgFmtColl::SetInDoc( SwDoc* pDoc, BOOL )
{
    SwCntntNode * pCntntNd = pDoc->GetNodes()[ nNode ]->GetCntntNode();
    ASSERT( pCntntNd, "Undo-ChgFmt: kein ContentNode" );

    // prufe vor dem setzen des Formates, ob dieses ueberhaupt noch im
    // Dokument vorhanden ist. Wurde es schon geloescht, gibt es kein Undo !!
    if( pCntntNd && nNdWhich == pCntntNd->GetNodeType() )
    {
        if( ND_TEXTNODE == nNdWhich )
        {
            if( USHRT_MAX != pDoc->GetTxtFmtColls()->GetPos( (SwTxtFmtColl*)pColl ))
            {
                pCntntNd->ChgFmtColl( (SwFmtColl*)pColl );
                if( NO_NUMBERING != mnNumLvl)
                {
                    SwTxtNode * pTxtNd = static_cast<SwTxtNode *>(pCntntNd);

                    // --> OD 2007-07-09 #i77372#
                    pTxtNd->SetLevel( mnNumLvl );
                    pTxtNd->SetRestart( mbIsRestart );
                    pTxtNd->SetStart( mnRestartVal );
                    pTxtNd->SetCounted( mbIsCounted );
                    // <--
                }
            }
        }
        else if( USHRT_MAX != pDoc->GetGrfFmtColls()->GetPos( (SwGrfFmtColl*)pColl ))
            pCntntNd->ChgFmtColl( (SwFmtColl*)pColl );
    }
}


SwHstryTxtFlyCnt::SwHstryTxtFlyCnt( SwTxtFlyCnt* pTxtFly )
    : SwHstryHint( HSTRY_FLYCNT )
{
    ASSERT( pTxtFly->GetFlyCnt().GetFrmFmt(), "FlyCntnt ohne Format" );
    pUndo = new SwUndoDelLayFmt( pTxtFly->GetFlyCnt().GetFrmFmt() );
    pUndo->ChgShowSel( FALSE );
}


SwHstryTxtFlyCnt::SwHstryTxtFlyCnt( SwFlyFrmFmt* pFlyFmt )
    : SwHstryHint( HSTRY_FLYCNT )
{
    ASSERT( pFlyFmt, "kein Format" );
    pUndo = new SwUndoDelLayFmt( pFlyFmt );
    pUndo->ChgShowSel( FALSE );
}


SwHstryTxtFlyCnt::~SwHstryTxtFlyCnt()
{
    delete pUndo;
}


void SwHstryTxtFlyCnt::SetInDoc( SwDoc* pDoc, BOOL )
{
    SwPaM aPam( pDoc->GetNodes().GetEndOfPostIts() );
    SwUndoIter aUndoIter( &aPam );
    pUndo->Undo( aUndoIter );
}



// JP 21.03.94: jetzt auch die Bookmarks in die History aufnehmen
SwHstryBookmark::SwHstryBookmark( const SwBookmark& rBkmk, BYTE nType )
    : SwHstryHint( HSTRY_BOOKMARK ),
      nNode1( 0 ),
      nNode2( rBkmk.GetOtherBookmarkPos() ? 0 : ULONG_MAX ),
      nCntnt1( 0 ),
      nCntnt2( 0 ),
      nTyp( nType ),
      // --> OD 2007-10-17 #i81002#
      eBkmkType( rBkmk.GetType() )
      // <--
{
    aName = rBkmk.GetName();
    aShortName = rBkmk.GetShortName();
    nKeyCode = rBkmk.GetKeyCode().GetCode() | rBkmk.GetKeyCode().GetModifier();

    if( BKMK_POS & nTyp )
    {
        nNode1 = rBkmk.GetBookmarkPos().nNode.GetIndex();
        nCntnt1 = rBkmk.GetBookmarkPos().nContent.GetIndex();
    }
    if( BKMK_OTHERPOS & nTyp )
    {
        nNode2 = rBkmk.GetOtherBookmarkPos()->nNode.GetIndex();
        nCntnt2 = rBkmk.GetOtherBookmarkPos()->nContent.GetIndex();
    }
}


void SwHstryBookmark::SetInDoc( SwDoc* pDoc, BOOL )
{
    BOOL bDoesUndo = pDoc->DoesUndo();
    pDoc->DoUndo( FALSE );

    SwNodes& rNds = pDoc->GetNodes();

    if( ( BKMK_POS == nTyp && ULONG_MAX == nNode2 ) ||
        ( BKMK_POS | BKMK_OTHERPOS ) == nTyp )
    {
        // voellig neu setzen
        SwCntntNode * pCntntNd = rNds[ nNode1 ]->GetCntntNode();
        ASSERT( pCntntNd, "Falscher Node fuer den Bookmark" );

        // #111660# don't crash when nNode1 doesn't point to content node.
        if( pCntntNd != NULL )
        {
            SwPaM aPam( *pCntntNd, nCntnt1 );
            if( ULONG_MAX != nNode2 )
            {
                aPam.SetMark();
                aPam.GetMark()->nNode = nNode2;
                pCntntNd = rNds[ aPam.GetMark()->nNode ]->GetCntntNode();
                ASSERT( pCntntNd, "Falscher Node fuer den Bookmark" );
                aPam.GetMark()->nContent.Assign( pCntntNd, nCntnt2 );
            }
            // --> OD 2007-10-17 #i81002#
            pDoc->makeBookmark( aPam, KeyCode( nKeyCode ), aName, aShortName, eBkmkType );
            // <--
        }
    }
    else
    {
        // dann muss das noch vorhandene manipuliert werden
        SwBookmark* const* ppBkmks = pDoc->getBookmarks().GetData();
        for( USHORT n = pDoc->getBookmarks().Count(); n; --n, ++ppBkmks )
            if( (*ppBkmks)->GetName() == aName )
            {
                ULONG nNd;
                USHORT nCnt;
                if( BKMK_POS == nTyp )
                {
                    if( !nNode2 && !(*ppBkmks)->GetOtherBookmarkPos() )
                    {
                        // dann muss der neu angelegt werden.
                        SwPaM aPam( (*ppBkmks)->GetBookmarkPos() );
                        aPam.SetMark();
                        aPam.GetPoint()->nNode = nNode1;
                        aPam.GetPoint()->nContent.Assign(
                                rNds[ nNode1 ]->GetCntntNode(), nCntnt1 );

                        pDoc->deleteBookmark( pDoc->getBookmarks().Count() - n );
                        // --> OD 2007-10-17 #i81002#
                        pDoc->makeBookmark( aPam, KeyCode( nKeyCode ),
                                            aName, aShortName,
                                            eBkmkType );
                        // <--
                        break;

                    }
                    nNd = nNode1;
                    nCnt = nCntnt1;
                    // --> OD 2007-09-27 #i81002# - refactoring
                    // Do not directly manipulate member of <SwBookmark>
//                    pPos = (SwPosition*)&(*ppBkmks)->GetBookmarkPos();
                    SwPosition aNewPos( (*ppBkmks)->GetBookmarkPos() );
                    aNewPos.nNode = nNd;
                    aNewPos.nContent.Assign( rNds[ aNewPos.nNode ]->GetCntntNode(),
                                             nCnt );
                    (*ppBkmks)->SetBookmarkPos( &aNewPos );
                    // <--
                }
                else
                {
                    if( !(*ppBkmks)->GetOtherBookmarkPos() )
                    {
                        // dann muss der neu angelegt werden.
                        SwPaM aPam( (*ppBkmks)->GetBookmarkPos() );
                        aPam.SetMark();
                        aPam.GetMark()->nNode = nNode2;
                        aPam.GetMark()->nContent.Assign(
                                rNds[ nNode2 ]->GetCntntNode(), nCntnt2 );

                        pDoc->deleteBookmark( pDoc->getBookmarks().Count() - n );
                        // --> OD 2007-10-17 #i81002#
                        pDoc->makeBookmark( aPam, KeyCode( nKeyCode ),
                                            aName, aShortName, eBkmkType );
                        // <--
                        break;
                    }
                    nNd = nNode2;
                    nCnt = nCntnt2;
                    // --> OD 2007-09-27 #i81002# - refactoring
                    // Do not directly manipulate member of <SwBookmark>
//                    pPos = (SwPosition*)(*ppBkmks)->GetOtherBookmarkPos();
                    SwPosition aNewPos( *((*ppBkmks)->GetOtherBookmarkPos()) );
                    aNewPos.nNode = nNd;
                    aNewPos.nContent.Assign( rNds[ aNewPos.nNode ]->GetCntntNode(),
                                             nCnt );
                    (*ppBkmks)->SetOtherBookmarkPos( &aNewPos );
                    // <--
                }

                // --> OD 2007-10-10 #i81002# - refactoring
                // Do not directly manipulate member of <SwBookmark>
//                pPos->nNode = nNd;
//                pPos->nContent.Assign( rNds[ pPos->nNode ]->GetCntntNode(),
//                                        nCnt );
                // <--
                break;
            }
    }

    pDoc->DoUndo( bDoesUndo );
}


BOOL SwHstryBookmark::IsEqualBookmark( const SwBookmark& rBkmk )
{
    return nNode1 == rBkmk.GetBookmarkPos().nNode.GetIndex() &&
           nCntnt1 == rBkmk.GetBookmarkPos().nContent.GetIndex() &&
            aName == rBkmk.GetName() &&
            aShortName == rBkmk.GetShortName() &&
            nKeyCode == (rBkmk.GetKeyCode().GetCode() |
                        rBkmk.GetKeyCode().GetModifier())
            ? TRUE : FALSE;
}

const String & SwHstryBookmark::GetName() const
{
    return aName;
}

/*************************************************************************/


SwHstrySetAttrSet::SwHstrySetAttrSet( const SfxItemSet& rSet, ULONG nNodePos,
                                        const SvUShortsSort& rSetArr )
    : SwHstryHint( HSTRY_SETATTRSET ),
      aOldSet( rSet ),
      aResetArr( 0, 4 ),
      nNode( nNodePos ),
      // --> OD 2007-07-09 #i77372#
      mnNumLvl( NO_NUMBERING ),
      mbIsRestart( false ),
      mnRestartVal( USHRT_MAX ),
      mbIsCounted( false )
      // <--
{
    SfxItemIter aIter( aOldSet ), aOrigIter( rSet );
    const SfxPoolItem* pItem = aIter.FirstItem(),
                     * pOrigItem = aOrigIter.FirstItem();
    do {
        if( !rSetArr.Seek_Entry( pOrigItem->Which() ))
        {
            aResetArr.Insert( pOrigItem->Which(), aResetArr.Count() );
            aOldSet.ClearItem( pOrigItem->Which() );
        }
        else
        {
            switch( pItem->Which() )
            {
            case RES_PAGEDESC:
                ((SwFmtPageDesc*)pItem)->ChgDefinedIn( 0 );
                break;

            case RES_PARATR_DROP:
                ((SwFmtDrop*)pItem)->ChgDefinedIn( 0 );
                break;

            case RES_BOXATR_FORMULA:
                {
                    //JP 20.04.98: Bug 49502 - wenn eine Formel gesetzt ist, nie den
                    //              Value mit sichern. Der muss gegebenfalls neu
                    //              errechnet werden!
                    //JP 30.07.98: Bug 54295 - Formeln immer im Klartext speichern
                    aOldSet.ClearItem( RES_BOXATR_VALUE );

                    SwTblBoxFormula& rNew = *(SwTblBoxFormula*)pItem;
                    if( rNew.IsIntrnlName() )
                    {
                        const SwTblBoxFormula& rOld = (SwTblBoxFormula&)rSet.Get( RES_BOXATR_FORMULA );
                        const SwNode* pNd = rOld.GetNodeOfFormula();
                        if( pNd && 0 != (pNd = pNd->FindTableNode() ))
                        {
                            SwTableFmlUpdate aMsgHnt( &((SwTableNode*)pNd)->GetTable() );
                            aMsgHnt.eFlags = TBL_BOXNAME;
                            rNew.ChgDefinedIn( rOld.GetDefinedIn() );
                            rNew.ChangeState( &aMsgHnt );
                        }
                    }
                    rNew.ChgDefinedIn( 0 );
                }
                break;

            case RES_PARATR_NUMRULE:
                if( ((SwNumRuleItem*)pItem)->GetDefinedIn() &&
                    ((SwNumRuleItem*)pItem)->GetDefinedIn()->ISA( SwTxtNode ))
                {
                    SwTxtNode* pTNd = (SwTxtNode*)((SwNumRuleItem*)pItem)->GetDefinedIn();
                    // --> OD 2007-07-09 #i77372#
                    mnNumLvl = pTNd->GetLevel();
                    mbIsRestart = pTNd->IsRestart();
                    mnRestartVal = pTNd->GetStart();
                    mbIsCounted = pTNd->IsCounted();
                    // <--
                }
                ((SwNumRuleItem*)pItem)->ChgDefinedIn( 0 );
                break;
            }
        }

        if( aIter.IsAtEnd() )
            break;
        pItem = aIter.NextItem();
        pOrigItem = aOrigIter.NextItem();
    } while( TRUE );
}

void SwHstrySetAttrSet::SetInDoc( SwDoc* pDoc, BOOL )
{
    BOOL bDoesUndo = pDoc->DoesUndo();
    pDoc->DoUndo( FALSE );

    SwNode * pNode = pDoc->GetNodes()[ nNode ];
    if( pNode->IsCntntNode() )
    {
        ((SwCntntNode*)pNode)->SetAttr( aOldSet );
        const SfxPoolItem* pItem;
        if( ((SwCntntNode*)pNode)->HasSwAttrSet() && SFX_ITEM_SET ==
            ((SwCntntNode*)pNode)->GetpSwAttrSet()->GetItemState(
            RES_PARATR_NUMRULE, FALSE, &pItem ) &&
            NO_NUMBERING != mnNumLvl)
        {
            SwTxtNode * pTxtNd = static_cast<SwTxtNode *>(pNode);

            // --> OD 2007-07-09 #i77372#
            pTxtNd->SetLevel( mnNumLvl );
            pTxtNd->SetRestart( mbIsRestart );
            pTxtNd->SetStart( mnRestartVal );
            pTxtNd->SetCounted( mbIsCounted );
            // <--
        }
        if( aResetArr.Count() )
            ((SwCntntNode*)pNode)->ResetAttr( aResetArr );
    }
    else if( pNode->IsTableNode() )
    {
        SwFmt& rFmt = *((SwTableNode*)pNode)->GetTable().GetFrmFmt();
        rFmt.SetAttr( aOldSet );
        if( aResetArr.Count() )
            rFmt.ResetAttr( *aResetArr.GetData() );
    }

    pDoc->DoUndo( bDoesUndo );
}

/*************************************************************************/


SwHstryResetAttrSet::SwHstryResetAttrSet( const SfxItemSet& rSet,
                    ULONG nNodePos, xub_StrLen nAttrStt, xub_StrLen nAttrEnd )
    : SwHstryHint( HSTRY_RESETATTRSET ),
    nNode( nNodePos ), nStart( nAttrStt ), nEnd( nAttrEnd ),
    aArr( (BYTE)rSet.Count() )
{
    SfxItemIter aIter( rSet );
    bool bAutoStyle = true;

    while( TRUE )
    {
        const USHORT nWhich = aIter.GetCurItem()->Which();
        // Character attribute cannot be inserted into the hints array
        // anymore. Therefore we have to treat them as one RES_TXTATR_AUTOFMT:
        if ( RES_CHRATR_BEGIN <= nWhich && RES_CHRATR_END > nWhich )
            bAutoStyle = true;
        else
            aArr.Insert( aIter.GetCurItem()->Which() ,aArr.Count() );

        if( aIter.IsAtEnd() )
            break;

        aIter.NextItem();
    }

    if ( bAutoStyle )
        aArr.Insert( RES_TXTATR_AUTOFMT, aArr.Count() );
}


void SwHstryResetAttrSet::SetInDoc( SwDoc* pDoc, BOOL )
{
    BOOL bDoesUndo = pDoc->DoesUndo();
    pDoc->DoUndo( FALSE );

    SwCntntNode * pCntntNd = pDoc->GetNodes()[ nNode ]->GetCntntNode();
    ASSERT( pCntntNd, "wo ist mein CntntNode" );

    if (pCntntNd)
    {
        const USHORT* pArr = aArr.GetData();
        if( USHRT_MAX == nEnd && USHRT_MAX == nStart )
        {
            // kein Bereich also Schnittstelle zum Content-Node
            for( USHORT n = aArr.Count(); n; --n, ++pArr )
                pCntntNd->ResetAttr( *pArr );
        }
        else
        {
            // Bereich: also Schnittstelle zum Text-Node
            for( USHORT n = aArr.Count(); n; --n, ++pArr )
                ((SwTxtNode*)pCntntNd)->Delete( *pArr, nStart, nEnd );
        }
    }

    pDoc->DoUndo( bDoesUndo );
}


/*************************************************************************/


SwHstryChgFlyAnchor::SwHstryChgFlyAnchor( const SwFrmFmt& rFmt )
    : SwHstryHint( HSTRY_CHGFLYANCHOR ),
    pFmt( (SwFrmFmt*)&rFmt )
{
    const SwFmtAnchor& rAnchor = rFmt.GetAnchor();
    nOldPos = rAnchor.GetCntntAnchor()->nNode.GetIndex();
    if( FLY_AUTO_CNTNT == rAnchor.GetAnchorId() )
        nOldCnt = rAnchor.GetCntntAnchor()->nContent.GetIndex();
    else
        nOldCnt = STRING_MAXLEN;
}


void SwHstryChgFlyAnchor::SetInDoc( SwDoc* pDoc, BOOL )
{
    BOOL bDoesUndo = pDoc->DoesUndo();
    pDoc->DoUndo( FALSE );

    USHORT nPos = pDoc->GetSpzFrmFmts()->GetPos( pFmt );
    if( USHRT_MAX != nPos )         // Format noch vorhanden
    {
        SwFmtAnchor aTmp( pFmt->GetAnchor() );

        SwNode* pNd = pDoc->GetNodes()[ nOldPos  ];
        SwPosition aPos( *pNd );
        if( STRING_MAXLEN != nOldCnt )
            aPos.nContent.Assign( (SwCntntNode*)pNd, nOldCnt );
        aTmp.SetAnchor( &aPos );

        // damit das Layout nicht durcheinander kommt!
        SwCntntNode* pCNd = pNd->GetCntntNode();
        if( !pCNd || !pCNd->GetFrm( 0, 0, FALSE ) )
            pFmt->DelFrms();

        pFmt->SetAttr( aTmp );
    }
    pDoc->DoUndo( bDoesUndo );
}


/*************************************************************************/

SwHstryChgFlyChain::SwHstryChgFlyChain( const SwFlyFrmFmt& rFmt,
                                        const SwFmtChain& rAttr )
    : SwHstryHint( HSTRY_CHGFLYCHAIN ),
    pPrevFmt( rAttr.GetPrev() ),
    pNextFmt( rAttr.GetNext() ),
    pFlyFmt( (SwFlyFrmFmt*)&rFmt )
{
}


void SwHstryChgFlyChain::SetInDoc( SwDoc* pDoc, BOOL )
{
    if( USHRT_MAX != pDoc->GetSpzFrmFmts()->GetPos( pFlyFmt ) )
    {
        SwFmtChain aChain;
        if( pPrevFmt && USHRT_MAX != pDoc->GetSpzFrmFmts()->GetPos( pPrevFmt ))
        {
            aChain.SetPrev( pPrevFmt );
            SwFmtChain aTmp( pPrevFmt->GetChain() );
            aTmp.SetNext( pFlyFmt );
            pPrevFmt->SetAttr( aTmp );
        }
        if( pNextFmt && USHRT_MAX != pDoc->GetSpzFrmFmts()->GetPos( pNextFmt ))
        {
            aChain.SetNext( pNextFmt );
            SwFmtChain aTmp( pNextFmt->GetChain() );
            aTmp.SetPrev( pFlyFmt );
            pNextFmt->SetAttr( aTmp );
        }

        if( aChain.GetNext() || aChain.GetPrev()  )
            pFlyFmt->SetAttr( aChain );
    }
}


// -> #i27615#
SwHstryChgCharFmt::SwHstryChgCharFmt(const SfxItemSet & rSet,
                                     const String & _sFmt)
    : SwHstryHint(HSTRY_CHGCHARFMT), aOldSet(rSet), sFmt(_sFmt)
{
}

void SwHstryChgCharFmt::SetInDoc(SwDoc * pDoc, BOOL )
{
    SwCharFmt * pCharFmt = pDoc->FindCharFmtByName(sFmt);

    if (pCharFmt)
        pCharFmt->SetAttr(aOldSet);
}
// <- #i27615#

/*  */


SwHistory::SwHistory( USHORT nInitSz, USHORT nGrowSz )
    : SwpHstry( (BYTE)nInitSz, (BYTE)nGrowSz ), nEndDiff( 0 )
{}


SwHistory::~SwHistory()
{
    Delete( 0 );
}


/*************************************************************************
|*
|*    void SwHistory::Add()
|*
|*    Beschreibung      Dokument 1.0
|*    Ersterstellung    JP 18.02.91
|*    Letzte Aenderung  JP 18.02.91
|*
*************************************************************************/

// --> OD 2007-07-11 #i56253#
void SwHistory::Add( const SfxPoolItem* pOldValue, const SfxPoolItem* pNewValue,
                     ULONG nNodeIdx, SwDoc& rDoc )
// <--
{
    ASSERT( !nEndDiff, "nach REDO wurde die History noch nicht geloescht" );

    USHORT nWhich = pNewValue->Which();
    if( (nWhich >= POOLATTR_END) || (nWhich == RES_TXTATR_FIELD) )
        return;

    // kein default Attribut ??
    SwHstryHint * pHt;
#ifdef JP_NEWCORE
    if( pOldValue && pOldValue->GetFmt() &&
        ( !pNewValue->GetFmt() || pOldValue->GetFmt()==pNewValue->GetFmt()) )
#else
    if( pOldValue && pOldValue != GetDfltAttr( pOldValue->Which() ) )
#endif
        pHt = new SwSetFmtHint( (SfxPoolItem*)pOldValue, nNodeIdx );
    else
        pHt = new SwResetFmtHint( (SfxPoolItem*)pNewValue, nNodeIdx, rDoc );
    Insert( pHt, Count() );
}


void SwHistory::Add( const SwTxtAttr* pHint, ULONG nNodeIdx, BOOL bNewAttr )
{

    ASSERT( !nEndDiff, "nach REDO wurde die History noch nicht geloescht" );

    SwHstryHint * pHt;
    USHORT nAttrWhich = pHint->Which();

    if( !bNewAttr )
    {
        switch ( nAttrWhich )
        {
        case RES_TXTATR_FTN:
            pHt = new SwSetFtnHint( (SwTxtFtn*)pHint, nNodeIdx );
            break;
        case RES_TXTATR_FLYCNT:
            pHt = new SwHstryTxtFlyCnt( (SwTxtFlyCnt*)pHint );
            break;
        case RES_TXTATR_FIELD:
            pHt = new SwSetTxtFldHint( (SwTxtFld*)pHint, nNodeIdx );
            break;
        case RES_TXTATR_TOXMARK:
            pHt = new SwSetTOXMarkHint( (SwTxtTOXMark*)pHint, nNodeIdx );
            break;
        case RES_TXTATR_REFMARK:
            pHt = new SwSetRefMarkHint( (SwTxtRefMark*)pHint, nNodeIdx );
            break;
        default:
            pHt = new SwSetTxtHint((SwTxtAttr*)pHint, nNodeIdx );
        }
    }
    else
    {
        pHt = new SwResetTxtHint( pHint->Which(), *pHint->GetStart(),
                                    *pHint->GetAnyEnd(), nNodeIdx );
    }
    Insert( pHt, Count() );
}


void SwHistory::Add( const SwFmtColl* pColl, ULONG nNodeIdx, BYTE nWhichNd )
{
    ASSERT( !nEndDiff, "nach REDO wurde die History noch nicht geloescht" );
    SwHstryHint * pHt = new SwChgFmtColl( pColl, nNodeIdx, nWhichNd );
    Insert( pHt, Count() );
}


// JP 21.03.94: Bookmarks jetzt auch in die History mitaufnehmen
void SwHistory::Add( const SwBookmark& rBkmk, BYTE nTyp )
{
    ASSERT( !nEndDiff, "nach REDO wurde die History noch nicht geloescht" );
    SwHstryHint * pHt = new SwHstryBookmark( rBkmk, nTyp );
    Insert( pHt, Count() );
}


void SwHistory::Add( const SwFrmFmt& rFmt )
{
    SwHstryHint * pHt = new SwHstryChgFlyAnchor( rFmt );
    Insert( pHt, Count() );
}

void SwHistory::Add( const SwFlyFrmFmt& rFmt, USHORT& rSetPos )
{
    ASSERT( !nEndDiff, "nach REDO wurde die History noch nicht geloescht" );
    SwHstryHint * pHint;
    const USHORT nWh = rFmt.Which();
    if( RES_FLYFRMFMT == nWh || RES_DRAWFRMFMT == nWh )
    {
        pHint = new SwHstryTxtFlyCnt( (SwFlyFrmFmt*)&rFmt );
        Insert( pHint, Count() );

        const SwFmtChain* pChainItem;
        if( SFX_ITEM_SET == rFmt.GetItemState( RES_CHAIN, FALSE,
            (const SfxPoolItem**)&pChainItem ))
        {
            if( pChainItem->GetNext() || pChainItem->GetPrev() )
            {
                SwHstryHint * pHt = new SwHstryChgFlyChain( rFmt, *pChainItem );
                Insert( pHt, rSetPos++ );
                if ( pChainItem->GetNext() )
                {
                    SwFmtChain aTmp( pChainItem->GetNext()->GetChain() );
                    aTmp.SetPrev( 0 );
                    pChainItem->GetNext()->SetAttr( aTmp );
                }
                if ( pChainItem->GetPrev() )
                {
                    SwFmtChain aTmp( pChainItem->GetPrev()->GetChain() );
                    aTmp.SetNext( 0 );
                    pChainItem->GetPrev()->SetAttr( aTmp );
                }
            }
            ((SwFlyFrmFmt&)rFmt).ResetAttr( RES_CHAIN );
        }
    }
}

void SwHistory::Add( const SwTxtFtn& rFtn )
{
    SwHstryHint *pHt = new SwSetFtnHint( rFtn );
    Insert( pHt, Count() );
}

// #i27615#
void SwHistory::Add(const SfxItemSet & rSet, const SwCharFmt & rFmt)
{
    SwHstryHint * pHt = new SwHstryChgCharFmt(rSet, rFmt.GetName());

    Insert(pHt, Count());
}

/*************************************************************************
|*
|*    BOOL SwHistory::Rollback()
|*
|*    Beschreibung      Dokument 1.0
|*    Ersterstellung    JP 18.02.91
|*    Letzte Aenderung  JP 18.02.91
|*
*************************************************************************/


BOOL SwHistory::Rollback( SwDoc* pDoc, USHORT nStart )
{
    if( !Count() )
        return FALSE;

    SwHstryHint * pHHt;
    USHORT i;
    for( i = Count(); i > nStart ; )
    {
        pHHt = operator[]( --i );
        pHHt->SetInDoc( pDoc, FALSE );
        delete pHHt;
    }
    SwpHstry::Remove( nStart, Count() - nStart );
    nEndDiff = 0;
    return TRUE;
}



BOOL SwHistory::TmpRollback( SwDoc* pDoc, USHORT nStart, BOOL bToFirst )
{
    USHORT nEnd = Count() - nEndDiff;
    if( !Count() || !nEnd || nStart >= nEnd )
        return FALSE;

    SwHstryHint * pHHt;
    if( bToFirst )
        for( ; nEnd > nStart; ++nEndDiff )
        {
            pHHt = operator[]( --nEnd );
            pHHt->SetInDoc( pDoc, TRUE );
        }
    else
        for( ; nStart < nEnd; ++nEndDiff, ++nStart )
        {
            pHHt = operator[]( nStart );
            pHHt->SetInDoc( pDoc, TRUE );
        }
    return TRUE;
}


void SwHistory::Delete( USHORT nStart )
{
    for( USHORT n = Count(); n > nStart; )
        SwpHstry::DeleteAndDestroy( --n, 1 );
    nEndDiff = 0;
}


USHORT SwHistory::SetTmpEnd( USHORT nNewTmpEnd )
{
    ASSERT( nNewTmpEnd <= Count(),  "das temp. Ende ist zu gross" );

    USHORT nOld = Count() - nEndDiff;
    nEndDiff = Count() - nNewTmpEnd;

    // rufe bei allen SwHstryFlyCnt das Redo vom UndoObject auf. Dadurch
    // werden die Formate der Fly gesichert !!
    for( USHORT n = nOld; n < nNewTmpEnd; n++ )
        if( HSTRY_FLYCNT == (*this)[ n ]->Which() )
            ((SwHstryTxtFlyCnt*)(*this)[ n ])->GetUDelLFmt()->Redo();

    return nOld;
}

void SwHistory::CopyFmtAttr( const SfxItemSet& rSet, ULONG nNodeIdx, SwDoc& rDoc )
{
    if( rSet.Count() )
    {
        SfxItemIter aIter( rSet );
        do {
            if( (SfxPoolItem*)-1 != aIter.GetCurItem() )
            {
                const SfxPoolItem* pNew = aIter.GetCurItem();
                Add( pNew, pNew, nNodeIdx, rDoc );
            }
            if( aIter.IsAtEnd() )
                break;
            aIter.NextItem();
        } while( TRUE );
    }
}

void SwHistory::CopyAttr( const SwpHints* pHts, ULONG nNodeIdx,
                          xub_StrLen nStt, xub_StrLen nEnd, BOOL bFields )
{
    if( !pHts  )
        return;

    // kopiere alle Attribute aus dem TextNode die
    // im Bereich von Position nStt bis Len liegen
    const SwTxtAttr* pHt;
    xub_StrLen nAttrStt;
    const xub_StrLen * pEndIdx;
    for( USHORT n = 0; n < pHts->Count(); n++ )
    {
        // BP: nAttrStt muss auch bei !pEndIdx gesetzt werden
        nAttrStt = *(pHt = (*pHts)[n])->GetStart();
// JP: ???? wieso nAttrStt >= nEnd
//      if( 0 != ( pEndIdx = pHt->GetEnd() ) && nAttrStt >= nEnd )
        if( 0 != ( pEndIdx = pHt->GetEnd() ) && nAttrStt > nEnd )
            break;

        // Flys und Ftn nie kopieren !!
        BOOL bNextAttr = FALSE;
        switch( pHt->Which() )
        {
        case RES_TXTATR_FIELD:
        case RES_TXTATR_HARDBLANK:
            // keine Felder, .. kopieren ??
            if( !bFields )
                bNextAttr = TRUE;
            break;
        case RES_TXTATR_FLYCNT:
        case RES_TXTATR_FTN:
            bNextAttr = TRUE;
            break;
        }

        if( bNextAttr )
           continue;

        // alle Attribute, die irgendwie in diesem Bereich liegen speichern
        if( nStt <= nAttrStt )
        {
            if( nEnd > nAttrStt
// JP: ???? wieso nAttrStt >= nEnd
//              || (nEnd == nAttrStt && (!pEndIdx || nEnd == pEndIdx->GetIndex()))
            )
                Add( pHt, nNodeIdx, FALSE );
        }
        else if( pEndIdx && nStt < *pEndIdx )
            Add( pHt, nNodeIdx, FALSE );
    }
}


/*************************************************************************/

// Klasse zum Registrieren der History am Node, Format, HintsArray, ...

SwRegHistory::SwRegHistory( SwDoc& rDoc, SwHistory* pHst )
    : SwClient( 0 ),
      pHstry( pHst ),
      nNodeIdx( ULONG_MAX ),
      // --> OD 2007-07-11 #i56253#
      mrDoc( rDoc )
      // <--
{
    if( pHst )
        _MakeSetWhichIds();
}

SwRegHistory::SwRegHistory( SwModify* pRegIn, const SwNode& rNd,
                            SwHistory* pHst )
    : SwClient( pRegIn ),
      pHstry( pHst ),
      nNodeIdx( rNd.GetIndex() ),
      // --> OD 2007-07-11 #i56253#
      mrDoc( const_cast<SwDoc&>(*(rNd.GetDoc())) )
      // <--
{
    if( pHst )
        _MakeSetWhichIds();
}

SwRegHistory::SwRegHistory( const SwNode& rNd, SwHistory* pHst )
    : SwClient( 0 ),
      pHstry( pHst ),
      nNodeIdx( rNd.GetIndex() ),
      // --> OD 2007-07-11 #i56253#
      mrDoc( const_cast<SwDoc&>(*(rNd.GetDoc())) )
      // <--
{
    if( pHstry )
        _MakeSetWhichIds();
}

void SwRegHistory::Modify( SfxPoolItem* pOld, SfxPoolItem* pNew )
{
    // Attribut erfragen ?? ( mal sehen )
    if( pHstry && ( pOld || pNew ) )
    {
        if( pNew->Which() < POOLATTR_END )
            pHstry->Add( pOld, pNew, nNodeIdx, mrDoc );
        else if( RES_ATTRSET_CHG == pNew->Which() )
        {
            SwHstryHint* pNewHstr;
            const SfxItemSet& rSet = *((SwAttrSetChg*)pOld)->GetChgSet();
            if( 1 < rSet.Count() )
                pNewHstr = new SwHstrySetAttrSet( rSet, nNodeIdx,
                                                    aSetWhichIds );
            else
            {
                const SfxPoolItem* pItem = SfxItemIter( rSet ).FirstItem();
                if( aSetWhichIds.Seek_Entry( pItem->Which() ) )
                    pNewHstr = new SwSetFmtHint( pItem, nNodeIdx );
                else
                    pNewHstr = new SwResetFmtHint( pItem, nNodeIdx, mrDoc );
            }
            pHstry->Insert( pNewHstr, pHstry->Count() );
        }
    }
}



void SwRegHistory::Add( SwTxtAttr* pHt, const BOOL bNew )
{
    pHstry->Add( pHt, nNodeIdx, bNew );
}


SwRegHistory::SwRegHistory( SwTxtNode* pTxtNode, const SfxItemSet& rSet,
                            xub_StrLen nStart, xub_StrLen nEnd, USHORT nFlags,
                            SwHistory* pHst )
    : SwClient( pTxtNode ),
      pHstry( pHst ),
      nNodeIdx( pTxtNode->GetIndex() ),
      // --> OD 2007-07-11 #i56253#
      mrDoc( *(pTxtNode->GetDoc()) )
      // <--

{
    if( !rSet.Count() )
        return;

    BOOL bInsert;

    if( pTxtNode->GetpSwpHints() && pHst )
    {
        pTxtNode->GetpSwpHints()->Register( this );
        bInsert = pTxtNode->SetAttr( rSet, nStart, nEnd, nFlags );
        // Achtung: Durch das Einfuegen eines Attributs kann das Array
        // geloescht werden!!! Wenn das einzufuegende zunaechst ein vorhandenes
        // loescht, selbst aber nicht eingefuegt werden braucht, weil die
        // Absatzattribute identisch sind( -> bForgetAttr in SwpHints::Insert )
        if ( pTxtNode->GetpSwpHints() )
            pTxtNode->GetpSwpHints()->DeRegister();
    }
    else
        bInsert = pTxtNode->SetAttr( rSet, nStart, nEnd, nFlags );

    if( pHst && bInsert )
    {
        SwHstryHint* pNewHstr = new SwHstryResetAttrSet( rSet,
                                    pTxtNode->GetIndex(), nStart, nEnd );
        // der NodeIndex kann verschoben sein !!

        pHst->Insert( pNewHstr, pHst->Count() );
    }
}

void SwRegHistory::RegisterInModify( SwModify* pRegIn, const SwNode& rNd )
{
    if( pHstry && pRegIn )
    {
        pRegIn->Add( this );
        nNodeIdx = rNd.GetIndex();
        _MakeSetWhichIds();
    }
    else if( aSetWhichIds.Count() )
        aSetWhichIds.Remove( 0, aSetWhichIds.Count() );
}

void SwRegHistory::_MakeSetWhichIds()
{
    if( aSetWhichIds.Count() )
        aSetWhichIds.Remove( 0, aSetWhichIds.Count() );

    if( GetRegisteredIn() )
    {
        const SfxItemSet* pSet = 0;
        if( GetRegisteredIn()->ISA( SwCntntNode ) )
            pSet = ((SwCntntNode*)GetRegisteredIn())->GetpSwAttrSet();
        else if( GetRegisteredIn()->ISA( SwFmt ) )
            pSet = &((SwFmt*)GetRegisteredIn())->GetAttrSet();
        if( pSet && pSet->Count() )
        {
            SfxItemIter aIter( *pSet );
            USHORT nW = aIter.FirstItem()->Which();
            while( TRUE )
            {
                aSetWhichIds.Insert( nW );
                if( aIter.IsAtEnd() )
                    break;
                nW = aIter.NextItem()->Which();
            }
        }
    }
}
