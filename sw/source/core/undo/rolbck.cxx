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
#include "precompiled_sw.hxx"

#include <rolbck.hxx>

#include <tools/resid.hxx>

#include <svl/itemiter.hxx>

#include <editeng/brkitem.hxx>

#include <hints.hxx>
#include <hintids.hxx>
#include <fmtftn.hxx>
#include <fchrfmt.hxx>
#include <fmtflcnt.hxx>
#include <fmtrfmrk.hxx>
#include <fmtfld.hxx>
#include <fmtpdsc.hxx>
#include <txtfld.hxx>
#include <txtrfmrk.hxx>
#include <txttxmrk.hxx>
#include <txtftn.hxx>
#include <txtflcnt.hxx>
#include <fmtanchr.hxx>
#include <fmtcnct.hxx>
#include <frmfmt.hxx>
#include <ftnidx.hxx>
#include <doc.hxx>              // SwDoc.GetNodes()
#include <IDocumentUndoRedo.hxx>
#include <docary.hxx>
#include <ndtxt.hxx>            // SwTxtNode
#include <paratr.hxx>           //
#include <cellatr.hxx>          //
#include <fldbas.hxx>           // fuer Felder
#include <pam.hxx>              // fuer SwPaM
#include <swtable.hxx>
#include <ndgrf.hxx>            // SwGrfNode
#include <UndoCore.hxx>
#include <IMark.hxx>            // fuer SwBookmark
#include <charfmt.hxx> // #i27615#
#include <comcore.hrc>
#include <undo.hrc>
#include <bookmrk.hxx>

SV_IMPL_PTRARR( SwpHstry, SwHistoryHintPtr)

String SwHistoryHint::GetDescription() const
{
    return String();
}


SwHistorySetFmt::SwHistorySetFmt( const SfxPoolItem* pFmtHt, sal_uLong nNd )
    :  SwHistoryHint( HSTRY_SETFMTHNT )
    ,  m_pAttr( pFmtHt->Clone() )
    ,  m_nNodeIndex( nNd )
{
    switch ( m_pAttr->Which() )
    {
        case RES_PAGEDESC:
            static_cast<SwFmtPageDesc&>(*m_pAttr).ChgDefinedIn( 0 );
            break;
        case RES_PARATR_DROP:
            static_cast<SwFmtDrop&>(*m_pAttr).ChgDefinedIn( 0 );
            break;
        case RES_BOXATR_FORMULA:
        {
            //JP 30.07.98: Bug 54295 - Formeln immer im Klartext speichern
            SwTblBoxFormula& rNew = static_cast<SwTblBoxFormula&>(*m_pAttr);
            if ( rNew.IsIntrnlName() )
            {
                const SwTblBoxFormula& rOld =
                    *static_cast<const SwTblBoxFormula*>(pFmtHt);
                const SwNode* pNd = rOld.GetNodeOfFormula();
                if ( pNd )
                {
                    const SwTableNode* pTableNode = pNd->FindTableNode();
                    if (pTableNode)
                    {
                        SwTableFmlUpdate aMsgHnt( &pTableNode->GetTable() );
                        aMsgHnt.eFlags = TBL_BOXNAME;
                        rNew.ChgDefinedIn( rOld.GetDefinedIn() );
                        rNew.ChangeState( &aMsgHnt );
                    }
                }
            }
            rNew.ChgDefinedIn( 0 );
        }
        break;
    }
}

String SwHistorySetFmt::GetDescription() const
{
    String aResult ;

    sal_uInt16 nWhich = m_pAttr->Which();
    switch (nWhich)
    {
    case RES_BREAK:
        switch ((static_cast<SvxFmtBreakItem &>(*m_pAttr)).GetBreak())
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

void SwHistorySetFmt::SetInDoc( SwDoc* pDoc, bool bTmpSet )
{
    SwNode * pNode = pDoc->GetNodes()[ m_nNodeIndex ];
    if ( pNode->IsCntntNode() )
    {
        static_cast<SwCntntNode*>(pNode)->SetAttr( *m_pAttr );
    }
    else if ( pNode->IsTableNode() )
    {
        static_cast<SwTableNode*>(pNode)->GetTable().GetFrmFmt()->SetFmtAttr(
                *m_pAttr );
    }
    else if ( pNode->IsStartNode() && (SwTableBoxStartNode ==
                static_cast<SwStartNode*>(pNode)->GetStartNodeType()) )
    {
        SwTableNode* pTNd = pNode->FindTableNode();
        if ( pTNd )
        {
            SwTableBox* pBox = pTNd->GetTable().GetTblBox( m_nNodeIndex );
            if (pBox)
            {
                pBox->ClaimFrmFmt()->SetFmtAttr( *m_pAttr );
            }
        }
    }

    if ( !bTmpSet )
    {
        m_pAttr.reset();
    }
}

SwHistorySetFmt::~SwHistorySetFmt()
{
}


// --> OD 2008-02-27 #refactorlists# - removed <rDoc>
SwHistoryResetFmt::SwHistoryResetFmt(const SfxPoolItem* pFmtHt, sal_uLong nNodeIdx)
// <--
    : SwHistoryHint( HSTRY_RESETFMTHNT )
    , m_nNodeIndex( nNodeIdx )
    , m_nWhich( pFmtHt->Which() )
{
}


void SwHistoryResetFmt::SetInDoc( SwDoc* pDoc, bool )
{
    SwNode * pNode = pDoc->GetNodes()[ m_nNodeIndex ];
    if ( pNode->IsCntntNode() )
    {
        static_cast<SwCntntNode*>(pNode)->ResetAttr( m_nWhich );
    }
    else if ( pNode->IsTableNode() )
    {
        static_cast<SwTableNode*>(pNode)->GetTable().GetFrmFmt()->
            ResetFmtAttr( m_nWhich );
    }
}


SwHistorySetTxt::SwHistorySetTxt( SwTxtAttr* pTxtHt, sal_uLong nNodePos )
    : SwHistoryHint( HSTRY_SETTXTHNT )
    , m_nNodeIndex( nNodePos )
    , m_nStart( *pTxtHt->GetStart() )
    , m_nEnd( *pTxtHt->GetAnyEnd() )
{
    // !! Achtung: folgende Attribute erzeugen keine FormatAttribute:
    //  - NoLineBreak, NoHypen, Inserted, Deleted
    // Dafuer muessen Sonderbehandlungen gemacht werden !!!

    // ein bisschen kompliziert, aber ist Ok so: erst vom default
    // eine Kopie und dann die Werte aus dem Text Attribut zuweisen
    sal_uInt16 nWhich = pTxtHt->Which();
    if ( RES_TXTATR_CHARFMT == nWhich )
    {
        m_pAttr.reset( new SwFmtCharFmt( pTxtHt->GetCharFmt().GetCharFmt() ) );
    }
    else
    {
        m_pAttr.reset( pTxtHt->GetAttr().Clone() );
    }
}


SwHistorySetTxt::~SwHistorySetTxt()
{
}


void SwHistorySetTxt::SetInDoc( SwDoc* pDoc, bool )
{
    if ( !m_pAttr.get() )
        return;

    if ( RES_TXTATR_CHARFMT == m_pAttr->Which() )
    {
        // ask the Doc if the CharFmt still exists
        if ( USHRT_MAX == pDoc->GetCharFmts()->GetPos(
                        (static_cast<SwFmtCharFmt&>(*m_pAttr)).GetCharFmt() ) )
            return; // do not set, format does not exist
    }

    SwTxtNode * pTxtNd = pDoc->GetNodes()[ m_nNodeIndex ]->GetTxtNode();
    ASSERT( pTxtNd, "SwHistorySetTxt::SetInDoc: not a TextNode" );

    if ( pTxtNd )
    {
        pTxtNd->InsertItem( *m_pAttr, m_nStart, m_nEnd,
                        nsSetAttrMode::SETATTR_NOTXTATRCHR |
                        nsSetAttrMode::SETATTR_NOHINTADJUST );
    }
}


SwHistorySetTxtFld::SwHistorySetTxtFld( SwTxtFld* pTxtFld, sal_uLong nNodePos )
    : SwHistoryHint( HSTRY_SETTXTFLDHNT )
    , m_pFldType( 0 )
    , m_pFld( new SwFmtFld( *pTxtFld->GetFld().GetFld() ) )
{
    // only copy if not Sys-FieldType
    SwDoc* pDoc = pTxtFld->GetTxtNode().GetDoc();

    m_nFldWhich = m_pFld->GetFld()->GetTyp()->Which();
    if (m_nFldWhich == RES_DBFLD ||
        m_nFldWhich == RES_USERFLD ||
        m_nFldWhich == RES_SETEXPFLD ||
        m_nFldWhich == RES_DDEFLD ||
        !pDoc->GetSysFldType( m_nFldWhich ))
    {
        m_pFldType.reset( m_pFld->GetFld()->GetTyp()->Copy() );
        m_pFld->GetFld()->ChgTyp( m_pFldType.get() ); // change field type
    }
    m_nNodeIndex = nNodePos;
    m_nPos = *pTxtFld->GetStart();
}

String SwHistorySetTxtFld::GetDescription() const
{
    return m_pFld->GetFld()->GetDescription();;
}

SwHistorySetTxtFld::~SwHistorySetTxtFld()
{
}


void SwHistorySetTxtFld::SetInDoc( SwDoc* pDoc, bool )
{
    if ( !m_pFld.get() )
        return;

    SwFieldType* pNewFldType = m_pFldType.get();
    if ( !pNewFldType )
    {
        pNewFldType = pDoc->GetSysFldType( m_nFldWhich );
    }
    else
    {
        // register type with the document
        pNewFldType = pDoc->InsertFldType( *m_pFldType );
    }

    m_pFld->GetFld()->ChgTyp( pNewFldType ); // change field type

    SwTxtNode * pTxtNd = pDoc->GetNodes()[ m_nNodeIndex ]->GetTxtNode();
    ASSERT( pTxtNd, "SwHistorySetTxtFld: no TextNode" );

    if ( pTxtNd )
    {
        pTxtNd->InsertItem( *m_pFld, m_nPos, m_nPos,
                    nsSetAttrMode::SETATTR_NOTXTATRCHR );
    }
}



SwHistorySetRefMark::SwHistorySetRefMark( SwTxtRefMark* pTxtHt, sal_uLong nNodePos )
    : SwHistoryHint( HSTRY_SETREFMARKHNT )
    , m_RefName( pTxtHt->GetRefMark().GetRefName() )
    , m_nNodeIndex( nNodePos )
    , m_nStart( *pTxtHt->GetStart() )
    , m_nEnd( *pTxtHt->GetAnyEnd() )
{
}


void SwHistorySetRefMark::SetInDoc( SwDoc* pDoc, bool )
{
    SwTxtNode * pTxtNd = pDoc->GetNodes()[ m_nNodeIndex ]->GetTxtNode();
    ASSERT( pTxtNd, "SwHistorySetRefMark: no TextNode" );
    if ( !pTxtNd )
        return;

    SwFmtRefMark aRefMark( m_RefName );

    // if a reference mark without an end already exists here: must not insert!
    if ( m_nStart != m_nEnd ||
         !pTxtNd->GetTxtAttrForCharAt( m_nStart, RES_TXTATR_REFMARK ) )
    {
        pTxtNd->InsertItem( aRefMark, m_nStart, m_nEnd,
                            nsSetAttrMode::SETATTR_NOTXTATRCHR );
    }
}


SwHistorySetTOXMark::SwHistorySetTOXMark( SwTxtTOXMark* pTxtHt, sal_uLong nNodePos )
    : SwHistoryHint( HSTRY_SETTOXMARKHNT )
    , m_TOXMark( pTxtHt->GetTOXMark() )
    , m_TOXName( m_TOXMark.GetTOXType()->GetTypeName() )
    , m_eTOXTypes( m_TOXMark.GetTOXType()->GetType() )
    , m_nNodeIndex( nNodePos )
    , m_nStart( *pTxtHt->GetStart() )
    , m_nEnd( *pTxtHt->GetAnyEnd() )
{
    m_TOXMark.DeRegister();
}


void SwHistorySetTOXMark::SetInDoc( SwDoc* pDoc, bool )
{
    SwTxtNode * pTxtNd = pDoc->GetNodes()[ m_nNodeIndex ]->GetTxtNode();
    ASSERT( pTxtNd, "SwHistorySetTOXMark: no TextNode" );
    if ( !pTxtNd )
        return;

    // search for respective TOX type
    sal_uInt16 nCnt = pDoc->GetTOXTypeCount( m_eTOXTypes );
    SwTOXType* pToxType = 0;
    for ( sal_uInt16 n = 0; n < nCnt; ++n )
    {
        pToxType = const_cast<SwTOXType*>(pDoc->GetTOXType( m_eTOXTypes, n ));
        if ( pToxType->GetTypeName() == m_TOXName )
            break;
        pToxType = 0;
    }

    if ( !pToxType )  // TOX type not found, create new
    {
        pToxType = const_cast<SwTOXType*>(
                pDoc->InsertTOXType( SwTOXType( m_eTOXTypes, m_TOXName )));
    }

    SwTOXMark aNew( m_TOXMark );
    aNew.RegisterToTOXType( *pToxType );

    pTxtNd->InsertItem( aNew, m_nStart, m_nEnd,
                        nsSetAttrMode::SETATTR_NOTXTATRCHR );
}


int SwHistorySetTOXMark::IsEqual( const SwTOXMark& rCmp ) const
{
    return m_TOXName   == rCmp.GetTOXType()->GetTypeName() &&
           m_eTOXTypes == rCmp.GetTOXType()->GetType() &&
           m_TOXMark.GetAlternativeText() == rCmp.GetAlternativeText() &&
           ( (TOX_INDEX == m_eTOXTypes)
              ?   ( m_TOXMark.GetPrimaryKey()   == rCmp.GetPrimaryKey()  &&
                    m_TOXMark.GetSecondaryKey() == rCmp.GetSecondaryKey()   )
              :   m_TOXMark.GetLevel() == rCmp.GetLevel()
           );
}


SwHistoryResetTxt::SwHistoryResetTxt( sal_uInt16 nWhich,
            xub_StrLen nAttrStart, xub_StrLen nAttrEnd, sal_uLong nNodePos )
    : SwHistoryHint( HSTRY_RESETTXTHNT )
    , m_nNodeIndex( nNodePos ), m_nStart( nAttrStart ), m_nEnd( nAttrEnd )
    , m_nAttr( nWhich )
{
}


void SwHistoryResetTxt::SetInDoc( SwDoc* pDoc, bool )
{
    SwTxtNode * pTxtNd = pDoc->GetNodes()[ m_nNodeIndex ]->GetTxtNode();
    ASSERT( pTxtNd, "SwHistoryResetTxt: no TextNode" );
    if ( pTxtNd )
    {
        pTxtNd->DeleteAttributes( m_nAttr, m_nStart, m_nEnd );
    }
}


SwHistorySetFootnote::SwHistorySetFootnote( SwTxtFtn* pTxtFtn, sal_uLong nNodePos )
    : SwHistoryHint( HSTRY_SETFTNHNT )
    , m_pUndo( new SwUndoSaveSection )
    , m_FootnoteNumber( pTxtFtn->GetFtn().GetNumStr() )
    , m_nNodeIndex( nNodePos )
    , m_nStart( *pTxtFtn->GetStart() )
    , m_bEndNote( pTxtFtn->GetFtn().IsEndNote() )
{
    ASSERT( pTxtFtn->GetStartNode(),
            "SwHistorySetFootnote: Footnote without Section" );

    // merke die alte NodePos, denn wer weiss was alles in der SaveSection
    // gespeichert (geloescht) wird
    SwDoc* pDoc = const_cast<SwDoc*>(pTxtFtn->GetTxtNode().GetDoc());
    SwNode* pSaveNd = pDoc->GetNodes()[ m_nNodeIndex ];

    //Pointer auf StartNode der FtnSection merken und erstmal den Pointer im
    //Attribut zuruecksetzen -> Damit werden automatisch die Frms vernichtet.
    SwNodeIndex aSttIdx( *pTxtFtn->GetStartNode() );
    pTxtFtn->SetStartNode( 0, sal_False );

    m_pUndo->SaveSection( pDoc, aSttIdx );
    m_nNodeIndex = pSaveNd->GetIndex();
}

SwHistorySetFootnote::SwHistorySetFootnote( const SwTxtFtn &rTxtFtn )
    : SwHistoryHint( HSTRY_SETFTNHNT )
    , m_pUndo( 0 )
    , m_FootnoteNumber( rTxtFtn.GetFtn().GetNumStr() )
    , m_nNodeIndex( _SwTxtFtn_GetIndex( (&rTxtFtn) ) )
    , m_nStart( *rTxtFtn.GetStart() )
    , m_bEndNote( rTxtFtn.GetFtn().IsEndNote() )
{
    ASSERT( rTxtFtn.GetStartNode(),
            "SwHistorySetFootnote: Footnote without Section" );
}

String SwHistorySetFootnote::GetDescription() const
{
    return SW_RES(STR_FOOTNOTE);
}

SwHistorySetFootnote::~SwHistorySetFootnote()
{
}


void SwHistorySetFootnote::SetInDoc( SwDoc* pDoc, bool )
{
    SwTxtNode * pTxtNd = pDoc->GetNodes()[ m_nNodeIndex ]->GetTxtNode();
    ASSERT( pTxtNd, "SwHistorySetFootnote: no TextNode" );
    if ( !pTxtNd )
        return;

    if ( m_pUndo.get() )
    {
        // set the footnote in the TextNode
        SwFmtFtn aTemp( m_bEndNote );
        SwFmtFtn& rNew = const_cast<SwFmtFtn&>(
                static_cast<const SwFmtFtn&>(pDoc->GetAttrPool().Put(aTemp)) );
        if ( m_FootnoteNumber.Len() )
        {
            rNew.SetNumStr( m_FootnoteNumber );
        }
        SwTxtFtn* pTxtFtn = new SwTxtFtn( rNew, m_nStart );

        // create the section of the Footnote
        SwNodeIndex aIdx( *pTxtNd );
        m_pUndo->RestoreSection( pDoc, &aIdx, SwFootnoteStartNode );
        pTxtFtn->SetStartNode( &aIdx );
        if ( m_pUndo->GetHistory() )
        {
            // create frames only now
            m_pUndo->GetHistory()->Rollback( pDoc );
        }

        pTxtNd->InsertHint( pTxtFtn );
    }
    else
    {
        SwTxtFtn * const pFtn =
            const_cast<SwTxtFtn*>( static_cast<const SwTxtFtn*>(
                pTxtNd->GetTxtAttrForCharAt( m_nStart )));
        SwFmtFtn &rFtn = const_cast<SwFmtFtn&>(pFtn->GetFtn());
        rFtn.SetNumStr( m_FootnoteNumber  );
        if ( rFtn.IsEndNote() != m_bEndNote )
        {
            rFtn.SetEndNote( m_bEndNote );
            pFtn->CheckCondColl();
        }
    }
}


SwHistoryChangeFmtColl::SwHistoryChangeFmtColl( SwFmtColl* pFmtColl, sal_uLong nNd,
                            sal_uInt8 nNodeWhich )
    : SwHistoryHint( HSTRY_CHGFMTCOLL )
    , m_pColl( pFmtColl )
    , m_nNodeIndex( nNd )
    , m_nNodeType( nNodeWhich )
{
}

void SwHistoryChangeFmtColl::SetInDoc( SwDoc* pDoc, bool )
{
    SwCntntNode * pCntntNd = pDoc->GetNodes()[ m_nNodeIndex ]->GetCntntNode();
    ASSERT( pCntntNd, "SwHistoryChangeFmtColl: no ContentNode" );

    // before setting the format, check if it is still available in the
    // document. if it has been deleted, there is no undo!
    if ( pCntntNd && m_nNodeType == pCntntNd->GetNodeType() )
    {
        if ( ND_TEXTNODE == m_nNodeType )
        {
            if ( USHRT_MAX != pDoc->GetTxtFmtColls()->GetPos(
                            static_cast<SwTxtFmtColl * const>(m_pColl) ))
            {
                pCntntNd->ChgFmtColl( m_pColl );
            }
        }
        else if ( USHRT_MAX != pDoc->GetGrfFmtColls()->GetPos(
                            static_cast<SwGrfFmtColl * const>(m_pColl) ))
        {
            pCntntNd->ChgFmtColl( m_pColl );
        }
    }
}


SwHistoryTxtFlyCnt::SwHistoryTxtFlyCnt( SwFrmFmt* const pFlyFmt )
    : SwHistoryHint( HSTRY_FLYCNT )
    , m_pUndo( new SwUndoDelLayFmt( pFlyFmt ) )
{
    ASSERT( pFlyFmt, "SwHistoryTxtFlyCnt: no Format" );
    m_pUndo->ChgShowSel( sal_False );
}


SwHistoryTxtFlyCnt::~SwHistoryTxtFlyCnt()
{
}


void SwHistoryTxtFlyCnt::SetInDoc( SwDoc* pDoc, bool )
{
    ::sw::IShellCursorSupplier *const pISCS(pDoc->GetIShellCursorSupplier());
    OSL_ASSERT(pISCS);
    ::sw::UndoRedoContext context(*pDoc, *pISCS);
    m_pUndo->UndoImpl(context);
}



SwHistoryBookmark::SwHistoryBookmark(
    const ::sw::mark::IMark& rBkmk,
    bool bSavePos,
    bool bSaveOtherPos)
    : SwHistoryHint(HSTRY_BOOKMARK)
    , m_aName(rBkmk.GetName())
    , m_aShortName()
    , m_aKeycode()
    , m_nNode(bSavePos ?
        rBkmk.GetMarkPos().nNode.GetIndex() : 0)
    , m_nOtherNode(bSaveOtherPos ?
        rBkmk.GetOtherMarkPos().nNode.GetIndex() : 0)
    , m_nCntnt(bSavePos ?
        rBkmk.GetMarkPos().nContent.GetIndex() : 0)
    , m_nOtherCntnt(bSaveOtherPos ?
        rBkmk.GetOtherMarkPos().nContent.GetIndex() :0)
    , m_bSavePos(bSavePos)
    , m_bSaveOtherPos(bSaveOtherPos)
    , m_bHadOtherPos(rBkmk.IsExpanded())
    , m_eBkmkType(IDocumentMarkAccess::GetType(rBkmk))
{
    const ::sw::mark::IBookmark* const pBookmark = dynamic_cast< const ::sw::mark::IBookmark* >(&rBkmk);
    if(pBookmark)
    {
        m_aKeycode = pBookmark->GetKeyCode();
        m_aShortName = pBookmark->GetShortName();

        ::sfx2::Metadatable const*const pMetadatable(
                dynamic_cast< ::sfx2::Metadatable const* >(pBookmark));
        if (pMetadatable)
        {
            m_pMetadataUndo = pMetadatable->CreateUndo();
        }
    }
}


void SwHistoryBookmark::SetInDoc( SwDoc* pDoc, bool )
{
    ::sw::UndoGuard const undoGuard(pDoc->GetIDocumentUndoRedo());

    SwNodes& rNds = pDoc->GetNodes();
    IDocumentMarkAccess* pMarkAccess = pDoc->getIDocumentMarkAccess();
    ::std::auto_ptr<SwPaM> pPam;
    ::sw::mark::IMark* pMark = NULL;

    if(m_bSavePos)
    {
        SwCntntNode* const pCntntNd = rNds[m_nNode]->GetCntntNode();
        OSL_ENSURE(pCntntNd,
            "<SwHistoryBookmark::SetInDoc(..)>"
            " - wrong node for a mark");

        // #111660# don't crash when nNode1 doesn't point to content node.
        if(pCntntNd)
            pPam = ::std::auto_ptr<SwPaM>(new SwPaM(*pCntntNd, m_nCntnt));
    }
    else
    {
        pMark = pMarkAccess->findMark(m_aName)->get();
        pPam = ::std::auto_ptr<SwPaM>(new SwPaM(pMark->GetMarkPos()));
    }

    if(m_bSaveOtherPos)
    {
        SwCntntNode* const pCntntNd = rNds[m_nOtherNode]->GetCntntNode();
        OSL_ENSURE(pCntntNd,
            "<SwHistoryBookmark::SetInDoc(..)>"
            " - wrong node for a mark");

        if(pPam.get() != NULL && pCntntNd)
        {
            pPam->SetMark();
            pPam->GetMark()->nNode = m_nOtherNode;
            pPam->GetMark()->nContent.Assign(pCntntNd, m_nOtherCntnt);
        }
    }
    else if(m_bHadOtherPos)
    {
        if(!pMark)
            pMark = pMarkAccess->findMark(m_aName)->get();
        OSL_ENSURE(pMark->IsExpanded(),
            "<SwHistoryBookmark::SetInDoc(..)>"
            " - missing pos on old mark");
        pPam->SetMark();
        *pPam->GetMark() = pMark->GetOtherMarkPos();
    }

    if(pPam.get())
    {
        if(pMark)
            pMarkAccess->deleteMark(pMark);
        ::sw::mark::IBookmark* const pBookmark = dynamic_cast< ::sw::mark::IBookmark* >(
            pMarkAccess->makeMark(*pPam, m_aName, m_eBkmkType));
        if(pBookmark)
        {
            pBookmark->SetKeyCode(m_aKeycode);
            pBookmark->SetShortName(m_aShortName);
            if (m_pMetadataUndo)
            {
                ::sfx2::Metadatable * const pMeta(
                    dynamic_cast< ::sfx2::Metadatable* >(pBookmark));
                OSL_ENSURE(pMeta, "metadata undo, but not metadatable?");
                if (pMeta)
                {
                    pMeta->RestoreMetadata(m_pMetadataUndo);
                }
            }
        }
    }
}


bool SwHistoryBookmark::IsEqualBookmark(const ::sw::mark::IMark& rBkmk)
{
    return m_nNode == rBkmk.GetMarkPos().nNode.GetIndex()
        && m_nCntnt == rBkmk.GetMarkPos().nContent.GetIndex()
        && m_aName == rBkmk.GetName();
}

const ::rtl::OUString& SwHistoryBookmark::GetName() const
{
    return m_aName;
}

/*************************************************************************/


SwHistorySetAttrSet::SwHistorySetAttrSet( const SfxItemSet& rSet,
                        sal_uLong nNodePos, const SvUShortsSort& rSetArr )
    : SwHistoryHint( HSTRY_SETATTRSET )
    , m_OldSet( rSet )
    , m_ResetArray( 0, 4 )
    , m_nNodeIndex( nNodePos )
{
    SfxItemIter aIter( m_OldSet ), aOrigIter( rSet );
    const SfxPoolItem* pItem = aIter.FirstItem(),
                     * pOrigItem = aOrigIter.FirstItem();
    do {
        if( !rSetArr.Seek_Entry( pOrigItem->Which() ))
        {
            m_ResetArray.Insert( pOrigItem->Which(), m_ResetArray.Count() );
            m_OldSet.ClearItem( pOrigItem->Which() );
        }
        else
        {
            switch ( pItem->Which() )
            {
                case RES_PAGEDESC:
                    static_cast<SwFmtPageDesc*>(
                        const_cast<SfxPoolItem*>(pItem))->ChgDefinedIn( 0 );
                    break;

                case RES_PARATR_DROP:
                    static_cast<SwFmtDrop*>(
                        const_cast<SfxPoolItem*>(pItem))->ChgDefinedIn( 0 );
                    break;

                case RES_BOXATR_FORMULA:
                    {
                    //JP 20.04.98: Bug 49502 - wenn eine Formel gesetzt ist, nie den
                    //              Value mit sichern. Der muss gegebenfalls neu
                    //              errechnet werden!
                    //JP 30.07.98: Bug 54295 - Formeln immer im Klartext speichern
                        m_OldSet.ClearItem( RES_BOXATR_VALUE );

                        SwTblBoxFormula& rNew =
                            *static_cast<SwTblBoxFormula*>(
                                const_cast<SfxPoolItem*>(pItem));
                        if ( rNew.IsIntrnlName() )
                        {
                            const SwTblBoxFormula& rOld =
                                static_cast<const SwTblBoxFormula&>(
                                        rSet.Get( RES_BOXATR_FORMULA ));
                            const SwNode* pNd = rOld.GetNodeOfFormula();
                            if ( pNd )
                            {
                                const SwTableNode* pTableNode
                                    = pNd->FindTableNode();
                                if (pTableNode)
                                {
                                    SwTableFmlUpdate aMsgHnt(
                                        &pTableNode->GetTable() );
                                    aMsgHnt.eFlags = TBL_BOXNAME;
                                    rNew.ChgDefinedIn( rOld.GetDefinedIn() );
                                    rNew.ChangeState( &aMsgHnt );
                                }
                            }
                        }
                        rNew.ChgDefinedIn( 0 );
                    }
                    break;
            }
        }

        if( aIter.IsAtEnd() )
            break;
        pItem = aIter.NextItem();
        pOrigItem = aOrigIter.NextItem();
    } while( sal_True );
}

void SwHistorySetAttrSet::SetInDoc( SwDoc* pDoc, bool )
{
    ::sw::UndoGuard const undoGuard(pDoc->GetIDocumentUndoRedo());

    SwNode * pNode = pDoc->GetNodes()[ m_nNodeIndex ];
    if ( pNode->IsCntntNode() )
    {
        static_cast<SwCntntNode*>(pNode)->SetAttr( m_OldSet );
        if ( m_ResetArray.Count() )
        {
            static_cast<SwCntntNode*>(pNode)->ResetAttr( m_ResetArray );
        }
    }
    else if ( pNode->IsTableNode() )
    {
        SwFmt& rFmt =
            *static_cast<SwTableNode*>(pNode)->GetTable().GetFrmFmt();
        rFmt.SetFmtAttr( m_OldSet );
        if ( m_ResetArray.Count() )
        {
            rFmt.ResetFmtAttr( *m_ResetArray.GetData() );
        }
    }
}

/*************************************************************************/


SwHistoryResetAttrSet::SwHistoryResetAttrSet( const SfxItemSet& rSet,
                    sal_uLong nNodePos, xub_StrLen nAttrStt, xub_StrLen nAttrEnd )
    : SwHistoryHint( HSTRY_RESETATTRSET )
    , m_nNodeIndex( nNodePos ), m_nStart( nAttrStt ), m_nEnd( nAttrEnd )
    , m_Array( (sal_uInt8)rSet.Count() )
{
    SfxItemIter aIter( rSet );
    bool bAutoStyle = false;

    while( sal_True )
    {
        const sal_uInt16 nWhich = aIter.GetCurItem()->Which();

#ifndef PRODUCT
        switch (nWhich)
        {
            case RES_TXTATR_REFMARK:
            case RES_TXTATR_TOXMARK:
                if (m_nStart != m_nEnd) break; // else: fall through!
            case RES_TXTATR_FIELD:
            case RES_TXTATR_FLYCNT:
            case RES_TXTATR_FTN:
            case RES_TXTATR_META:
            case RES_TXTATR_METAFIELD:
                ASSERT(rSet.Count() == 1,
                    "text attribute with CH_TXTATR, but not the only one:"
                    "\nnot such a good idea");
                break;
        }
#endif

        // Character attribute cannot be inserted into the hints array
        // anymore. Therefore we have to treat them as one RES_TXTATR_AUTOFMT:
        if (isCHRATR(nWhich))
        {
            bAutoStyle = true;
        }
        else
        {
            m_Array.Insert( aIter.GetCurItem()->Which(), m_Array.Count() );
        }

        if( aIter.IsAtEnd() )
            break;

        aIter.NextItem();
    }

    if ( bAutoStyle )
    {
        m_Array.Insert( RES_TXTATR_AUTOFMT, m_Array.Count() );
    }
}


void SwHistoryResetAttrSet::SetInDoc( SwDoc* pDoc, bool )
{
    ::sw::UndoGuard const undoGuard(pDoc->GetIDocumentUndoRedo());

    SwCntntNode * pCntntNd = pDoc->GetNodes()[ m_nNodeIndex ]->GetCntntNode();
    ASSERT( pCntntNd, "SwHistoryResetAttrSet: no CntntNode" );

    if (pCntntNd)
    {
        const sal_uInt16* pArr = m_Array.GetData();
        if ( USHRT_MAX == m_nEnd && USHRT_MAX == m_nStart )
        {
            // no area: use ContentNode
            for ( sal_uInt16 n = m_Array.Count(); n; --n, ++pArr )
            {
                pCntntNd->ResetAttr( *pArr );
            }
        }
        else
        {
            // area: use TextNode
            for ( sal_uInt16 n = m_Array.Count(); n; --n, ++pArr )
            {
                static_cast<SwTxtNode*>(pCntntNd)->
                    DeleteAttributes( *pArr, m_nStart, m_nEnd );
            }
        }
    }
}


/*************************************************************************/


SwHistoryChangeFlyAnchor::SwHistoryChangeFlyAnchor( SwFrmFmt& rFmt )
    : SwHistoryHint( HSTRY_CHGFLYANCHOR )
    , m_rFmt( rFmt )
    , m_nOldNodeIndex( rFmt.GetAnchor().GetCntntAnchor()->nNode.GetIndex() )
    , m_nOldContentIndex( (FLY_AT_CHAR == rFmt.GetAnchor().GetAnchorId())
            ?   rFmt.GetAnchor().GetCntntAnchor()->nContent.GetIndex()
            :   STRING_MAXLEN )
{
}


void SwHistoryChangeFlyAnchor::SetInDoc( SwDoc* pDoc, bool )
{
    ::sw::UndoGuard const undoGuard(pDoc->GetIDocumentUndoRedo());

    sal_uInt16 nPos = pDoc->GetSpzFrmFmts()->GetPos( &m_rFmt );
    if ( USHRT_MAX != nPos )    // Format does still exist
    {
        SwFmtAnchor aTmp( m_rFmt.GetAnchor() );

        SwNode* pNd = pDoc->GetNodes()[ m_nOldNodeIndex ];
        SwCntntNode* pCNd = pNd->GetCntntNode();
        SwPosition aPos( *pNd );
        if ( STRING_MAXLEN != m_nOldContentIndex )
        {
            ASSERT(pCNd, "SwHistoryChangeFlyAnchor: no ContentNode");
            if (pCNd)
            {
                aPos.nContent.Assign( pCNd, m_nOldContentIndex );
            }
        }
        aTmp.SetAnchor( &aPos );

        // so the Layout does not get confused
<<<<<<< local
        if ( !pCNd || !pCNd->getLayoutFrm( pDoc->GetCurrentLayout(), 0, 0, FALSE ) )
=======
        if ( !pCNd || !pCNd->GetFrm( 0, 0, sal_False ) )
>>>>>>> other
        {
            m_rFmt.DelFrms();
        }

        m_rFmt.SetFmtAttr( aTmp );
    }
}


/*************************************************************************/

SwHistoryChangeFlyChain::SwHistoryChangeFlyChain( SwFlyFrmFmt& rFmt,
                                        const SwFmtChain& rAttr )
    : SwHistoryHint( HSTRY_CHGFLYCHAIN )
    , m_pPrevFmt( rAttr.GetPrev() )
    , m_pNextFmt( rAttr.GetNext() )
    , m_pFlyFmt( &rFmt )
{
}


void SwHistoryChangeFlyChain::SetInDoc( SwDoc* pDoc, bool )
{
    if ( USHRT_MAX != pDoc->GetSpzFrmFmts()->GetPos( m_pFlyFmt ) )
    {
        SwFmtChain aChain;

        if ( m_pPrevFmt &&
             USHRT_MAX != pDoc->GetSpzFrmFmts()->GetPos( m_pPrevFmt ) )
        {
            aChain.SetPrev( m_pPrevFmt );
            SwFmtChain aTmp( m_pPrevFmt->GetChain() );
            aTmp.SetNext( m_pFlyFmt );
            m_pPrevFmt->SetFmtAttr( aTmp );
        }

        if ( m_pNextFmt &&
             USHRT_MAX != pDoc->GetSpzFrmFmts()->GetPos( m_pNextFmt ) )
        {
            aChain.SetNext( m_pNextFmt );
            SwFmtChain aTmp( m_pNextFmt->GetChain() );
            aTmp.SetPrev( m_pFlyFmt );
            m_pNextFmt->SetFmtAttr( aTmp );
        }

        if ( aChain.GetNext() || aChain.GetPrev() )
        {
            m_pFlyFmt->SetFmtAttr( aChain );
        }
    }
}


// -> #i27615#
SwHistoryChangeCharFmt::SwHistoryChangeCharFmt(const SfxItemSet & rSet,
                                     const String & sFmt)
    : SwHistoryHint(HSTRY_CHGCHARFMT)
    , m_OldSet(rSet), m_Fmt(sFmt)
{
}

void SwHistoryChangeCharFmt::SetInDoc(SwDoc * pDoc, bool )
{
    SwCharFmt * pCharFmt = pDoc->FindCharFmtByName(m_Fmt);

    if (pCharFmt)
    {
        pCharFmt->SetFmtAttr(m_OldSet);
    }
}
// <- #i27615#

/*  */


SwHistory::SwHistory( sal_uInt16 nInitSz, sal_uInt16 nGrowSz )
    : m_SwpHstry( (sal_uInt8)nInitSz, (sal_uInt8)nGrowSz )
    , m_nEndDiff( 0 )
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

// --> OD 2008-02-27 #refactorlists# - removed <rDoc>
void SwHistory::Add( const SfxPoolItem* pOldValue, const SfxPoolItem* pNewValue,
                     sal_uLong nNodeIdx )
// <--
{
    ASSERT( !m_nEndDiff, "History was not deleted after REDO" );

    sal_uInt16 nWhich = pNewValue->Which();
    if( (nWhich >= POOLATTR_END) || (nWhich == RES_TXTATR_FIELD) )
        return;

    // no default Attribute?
    SwHistoryHint * pHt;
    if ( pOldValue && pOldValue != GetDfltAttr( pOldValue->Which() ) )
    {
        pHt = new SwHistorySetFmt( pOldValue, nNodeIdx );
    }
    else
    {
        pHt = new SwHistoryResetFmt( pNewValue, nNodeIdx );
    }
    m_SwpHstry.Insert( pHt, Count() );
}


void SwHistory::Add( SwTxtAttr* pHint, sal_uLong nNodeIdx, bool bNewAttr )
{
    ASSERT( !m_nEndDiff, "History was not deleted after REDO" );

    SwHistoryHint * pHt;
    sal_uInt16 nAttrWhich = pHint->Which();

    if( !bNewAttr )
    {
        switch ( nAttrWhich )
        {
            case RES_TXTATR_FTN:
                pHt = new SwHistorySetFootnote(
                            static_cast<SwTxtFtn*>(pHint), nNodeIdx );
                break;
            case RES_TXTATR_FLYCNT:
                pHt = new SwHistoryTxtFlyCnt( static_cast<SwTxtFlyCnt*>(pHint)
                            ->GetFlyCnt().GetFrmFmt() );
                break;
            case RES_TXTATR_FIELD:
                pHt = new SwHistorySetTxtFld(
                            static_cast<SwTxtFld*>(pHint), nNodeIdx );
                break;
            case RES_TXTATR_TOXMARK:
                pHt = new SwHistorySetTOXMark(
                            static_cast<SwTxtTOXMark*>(pHint), nNodeIdx );
                break;
            case RES_TXTATR_REFMARK:
                pHt = new SwHistorySetRefMark(
                            static_cast<SwTxtRefMark*>(pHint), nNodeIdx );
                break;
            default:
                pHt = new SwHistorySetTxt(
                            static_cast<SwTxtAttr*>(pHint), nNodeIdx );
        }
    }
    else
    {
        pHt = new SwHistoryResetTxt( pHint->Which(), *pHint->GetStart(),
                                    *pHint->GetAnyEnd(), nNodeIdx );
    }
    m_SwpHstry.Insert( pHt, Count() );
}


void SwHistory::Add( SwFmtColl* pColl, sal_uLong nNodeIdx, sal_uInt8 nWhichNd )
{
    ASSERT( !m_nEndDiff, "History was not deleted after REDO" );

    SwHistoryHint * pHt =
        new SwHistoryChangeFmtColl( pColl, nNodeIdx, nWhichNd );
    m_SwpHstry.Insert( pHt, Count() );
}


void SwHistory::Add(const ::sw::mark::IMark& rBkmk, bool bSavePos, bool bSaveOtherPos)
{
    ASSERT( !m_nEndDiff, "History was not deleted after REDO" );

    SwHistoryHint * pHt = new SwHistoryBookmark(rBkmk, bSavePos, bSaveOtherPos);
    m_SwpHstry.Insert( pHt, Count() );
}


void SwHistory::Add( SwFrmFmt& rFmt )
{
    SwHistoryHint * pHt = new SwHistoryChangeFlyAnchor( rFmt );
    m_SwpHstry.Insert( pHt, Count() );
}

void SwHistory::Add( SwFlyFrmFmt& rFmt, sal_uInt16& rSetPos )
{
    ASSERT( !m_nEndDiff, "History was not deleted after REDO" );

    SwHistoryHint * pHint;
    const sal_uInt16 nWh = rFmt.Which();
    if( RES_FLYFRMFMT == nWh || RES_DRAWFRMFMT == nWh )
    {
        pHint = new SwHistoryTxtFlyCnt( &rFmt );
        m_SwpHstry.Insert( pHint, Count() );

        const SwFmtChain* pChainItem;
        if( SFX_ITEM_SET == rFmt.GetItemState( RES_CHAIN, sal_False,
            (const SfxPoolItem**)&pChainItem ))
        {
            if( pChainItem->GetNext() || pChainItem->GetPrev() )
            {
                SwHistoryHint * pHt =
                    new SwHistoryChangeFlyChain( rFmt, *pChainItem );
                m_SwpHstry.Insert( pHt, rSetPos++ );
                if ( pChainItem->GetNext() )
                {
                    SwFmtChain aTmp( pChainItem->GetNext()->GetChain() );
                    aTmp.SetPrev( 0 );
                    pChainItem->GetNext()->SetFmtAttr( aTmp );
                }
                if ( pChainItem->GetPrev() )
                {
                    SwFmtChain aTmp( pChainItem->GetPrev()->GetChain() );
                    aTmp.SetNext( 0 );
                    pChainItem->GetPrev()->SetFmtAttr( aTmp );
                }
            }
            rFmt.ResetFmtAttr( RES_CHAIN );
        }
    }
}

void SwHistory::Add( const SwTxtFtn& rFtn )
{
    SwHistoryHint *pHt = new SwHistorySetFootnote( rFtn );
    m_SwpHstry.Insert( pHt, Count() );
}

// #i27615#
void SwHistory::Add(const SfxItemSet & rSet, const SwCharFmt & rFmt)
{
    SwHistoryHint * pHt = new SwHistoryChangeCharFmt(rSet, rFmt.GetName());
    m_SwpHstry.Insert(pHt, Count());
}

/*************************************************************************
|*
|*    sal_Bool SwHistory::Rollback()
|*
|*    Beschreibung      Dokument 1.0
|*    Ersterstellung    JP 18.02.91
|*    Letzte Aenderung  JP 18.02.91
|*
*************************************************************************/


bool SwHistory::Rollback( SwDoc* pDoc, sal_uInt16 nStart )
{
    if ( !Count() )
        return false;

    SwHistoryHint * pHHt;
    sal_uInt16 i;
    for ( i = Count(); i > nStart ; )
    {
        pHHt = m_SwpHstry[ --i ];
        pHHt->SetInDoc( pDoc, false );
        delete pHHt;
    }
    m_SwpHstry.Remove( nStart, Count() - nStart );
    m_nEndDiff = 0;
    return true;
}



bool SwHistory::TmpRollback( SwDoc* pDoc, sal_uInt16 nStart, bool bToFirst )
{
    sal_uInt16 nEnd = Count() - m_nEndDiff;
    if ( !Count() || !nEnd || nStart >= nEnd )
        return false;

    SwHistoryHint * pHHt;
    if ( bToFirst )
    {
        for ( ; nEnd > nStart; ++m_nEndDiff )
        {
            pHHt = m_SwpHstry[ --nEnd ];
            pHHt->SetInDoc( pDoc, true );
        }
    }
    else
    {
        for ( ; nStart < nEnd; ++m_nEndDiff, ++nStart )
        {
            pHHt = m_SwpHstry[ nStart ];
            pHHt->SetInDoc( pDoc, true );
        }
    }
    return true;
}


void SwHistory::Delete( sal_uInt16 nStart )
{
    for ( sal_uInt16 n = Count(); n > nStart; )
    {
        m_SwpHstry.DeleteAndDestroy( --n, 1 );
    }
    m_nEndDiff = 0;
}


sal_uInt16 SwHistory::SetTmpEnd( sal_uInt16 nNewTmpEnd )
{
    ASSERT( nNewTmpEnd <= Count(),  "SwHistory::SetTmpEnd: out of bounds" );

    sal_uInt16 nOld = Count() - m_nEndDiff;
    m_nEndDiff = Count() - nNewTmpEnd;

    // for every SwHistoryFlyCnt, call the Redo of its UndoObject.
    // this saves the formats of the flys!
    for ( sal_uInt16 n = nOld; n < nNewTmpEnd; n++ )
    {
        if ( HSTRY_FLYCNT == (*this)[ n ]->Which() )
        {
            static_cast<SwHistoryTxtFlyCnt*>((*this)[ n ])
                ->GetUDelLFmt()->RedoForRollback();
        }
    }

    return nOld;
}

void SwHistory::CopyFmtAttr( const SfxItemSet& rSet, sal_uLong nNodeIdx )
{
    if( rSet.Count() )
    {
        SfxItemIter aIter( rSet );
        do {
            if( (SfxPoolItem*)-1 != aIter.GetCurItem() )
            {
                const SfxPoolItem* pNew = aIter.GetCurItem();
                Add( pNew, pNew, nNodeIdx );
            }
            if( aIter.IsAtEnd() )
                break;
            aIter.NextItem();
        } while( sal_True );
    }
}

void SwHistory::CopyAttr( SwpHints* pHts, sal_uLong nNodeIdx,
                          xub_StrLen nStart, xub_StrLen nEnd, bool bFields )
{
    if( !pHts  )
        return;

    // copy all attributes of the TextNode in the area from nStart to nEnd
    SwTxtAttr* pHt;
    xub_StrLen nAttrStt;
    const xub_StrLen * pEndIdx;
    for( sal_uInt16 n = 0; n < pHts->Count(); n++ )
    {
        // BP: nAttrStt muss auch bei !pEndIdx gesetzt werden
        pHt = pHts->GetTextHint(n);
        nAttrStt = *pHt->GetStart();
// JP: ???? wieso nAttrStt >= nEnd
//      if( 0 != ( pEndIdx = pHt->GetEnd() ) && nAttrStt >= nEnd )
        if( 0 != ( pEndIdx = pHt->GetEnd() ) && nAttrStt > nEnd )
            break;

        // Flys und Ftn nie kopieren !!
        sal_Bool bNextAttr = sal_False;
        switch( pHt->Which() )
        {
        case RES_TXTATR_FIELD:
            // keine Felder, .. kopieren ??
            if( !bFields )
                bNextAttr = sal_True;
            break;
        case RES_TXTATR_FLYCNT:
        case RES_TXTATR_FTN:
            bNextAttr = sal_True;
            break;
        }

        if( bNextAttr )
           continue;

        // save all attributes that are somehow in this area
        if ( nStart <= nAttrStt )
        {
            if ( nEnd > nAttrStt
// JP: ???? wieso nAttrStt >= nEnd
//              || (nEnd == nAttrStt && (!pEndIdx || nEnd == pEndIdx->GetIndex()))
            )
            {
                Add( pHt, nNodeIdx, false );
            }
        }
        else if ( pEndIdx && nStart < *pEndIdx )
        {
            Add( pHt, nNodeIdx, false );
        }
    }
}


/*************************************************************************/

// Klasse zum Registrieren der History am Node, Format, HintsArray, ...

SwRegHistory::SwRegHistory( SwHistory* pHst )
    : SwClient( 0 )
    , m_pHistory( pHst )
    , m_nNodeIndex( ULONG_MAX )
{
    _MakeSetWhichIds();
}

SwRegHistory::SwRegHistory( SwModify* pRegIn, const SwNode& rNd,
                            SwHistory* pHst )
    : SwClient( pRegIn )
    , m_pHistory( pHst )
    , m_nNodeIndex( rNd.GetIndex() )
{
    _MakeSetWhichIds();
}

SwRegHistory::SwRegHistory( const SwNode& rNd, SwHistory* pHst )
    : SwClient( 0 )
    , m_pHistory( pHst )
    , m_nNodeIndex( rNd.GetIndex() )
{
    _MakeSetWhichIds();
}

void SwRegHistory::Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
{
    // --> OD 2010-10-05 #i114861#
    // Do not handle a "noop" modify
    // - e.g. <SwTxtNode::NumRuleChgd()> uses such a "noop" modify
//    if ( m_pHistory && ( pOld || pNew ) )
    if ( m_pHistory && ( pOld || pNew ) &&
         pOld != pNew )
    // <--
    {
        if ( pNew->Which() < POOLATTR_END )
        {
            m_pHistory->Add( pOld, pNew, m_nNodeIndex );
        }
        else if ( RES_ATTRSET_CHG == pNew->Which() )
        {
            SwHistoryHint* pNewHstr;
            const SfxItemSet& rSet =
                *static_cast<const SwAttrSetChg*>(pOld)->GetChgSet();
            if ( 1 < rSet.Count() )
            {
                pNewHstr =
                    new SwHistorySetAttrSet( rSet, m_nNodeIndex, m_WhichIdSet );
            }
            else
            {
                const SfxPoolItem* pItem = SfxItemIter( rSet ).FirstItem();
                if ( m_WhichIdSet.Seek_Entry( pItem->Which() ) )
                {
                    pNewHstr = new SwHistorySetFmt( pItem, m_nNodeIndex );
                }
                else
                {
                    pNewHstr = new SwHistoryResetFmt( pItem, m_nNodeIndex );
                }
            }
            m_pHistory->m_SwpHstry.Insert( pNewHstr, m_pHistory->Count() );
        }
    }
}



void SwRegHistory::AddHint( SwTxtAttr* pHt, const bool bNew )
{
    m_pHistory->Add( pHt, m_nNodeIndex, bNew );
}


bool SwRegHistory::InsertItems( const SfxItemSet& rSet,
    xub_StrLen const nStart, xub_StrLen const nEnd, SetAttrMode const nFlags )
{
    if( !rSet.Count() )
        return false;

    SwTxtNode * const pTxtNode =
        dynamic_cast<SwTxtNode *>(const_cast<SwModify *>(GetRegisteredIn()));

    ASSERT(pTxtNode, "SwRegHistory not registered at text node?");
    if (!pTxtNode)
        return false;

    if ( pTxtNode->GetpSwpHints() && m_pHistory )
    {
        pTxtNode->GetpSwpHints()->Register( this );
    }

    const bool bInserted = pTxtNode->SetAttr( rSet, nStart, nEnd, nFlags );

        // Achtung: Durch das Einfuegen eines Attributs kann das Array
        // geloescht werden!!! Wenn das einzufuegende zunaechst ein vorhandenes
        // loescht, selbst aber nicht eingefuegt werden braucht, weil die
        // Absatzattribute identisch sind( -> bForgetAttr in SwpHints::Insert )
    if ( pTxtNode->GetpSwpHints() && m_pHistory )
    {
        pTxtNode->GetpSwpHints()->DeRegister();
    }

    if ( m_pHistory && bInserted )
    {
        SwHistoryHint* pNewHstr = new SwHistoryResetAttrSet( rSet,
                                    pTxtNode->GetIndex(), nStart, nEnd );
        // der NodeIndex kann verschoben sein !!

        m_pHistory->m_SwpHstry.Insert( pNewHstr, m_pHistory->Count() );
    }

    return bInserted;
}

void SwRegHistory::RegisterInModify( SwModify* pRegIn, const SwNode& rNd )
{
    if ( m_pHistory && pRegIn )
    {
        pRegIn->Add( this );
        m_nNodeIndex = rNd.GetIndex();
        _MakeSetWhichIds();
    }
    else if ( m_WhichIdSet.Count() )
    {
        m_WhichIdSet.Remove( 0, m_WhichIdSet.Count() );
    }
}

void SwRegHistory::_MakeSetWhichIds()
{
    if (!m_pHistory) return;

    if ( m_WhichIdSet.Count() )
    {
        m_WhichIdSet.Remove( 0, m_WhichIdSet.Count() );
    }

    if( GetRegisteredIn() )
    {
        const SfxItemSet* pSet = 0;
        if( GetRegisteredIn()->ISA( SwCntntNode ) )
        {
            pSet = static_cast<SwCntntNode*>(
                    const_cast<SwModify*>(GetRegisteredIn()))->GetpSwAttrSet();
        }
        else if ( GetRegisteredIn()->ISA( SwFmt ) )
        {
            pSet = &static_cast<SwFmt*>(
                    const_cast<SwModify*>(GetRegisteredIn()))->GetAttrSet();
        }
        if( pSet && pSet->Count() )
        {
            SfxItemIter aIter( *pSet );
            sal_uInt16 nW = aIter.FirstItem()->Which();
            while( sal_True )
            {
                m_WhichIdSet.Insert( nW );
                if( aIter.IsAtEnd() )
                    break;
                nW = aIter.NextItem()->Which();
            }
        }
    }
}

