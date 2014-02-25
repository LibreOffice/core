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

#include <rolbck.hxx>
#include <tools/resid.hxx>
#include <svl/itemiter.hxx>
#include <editeng/formatbreakitem.hxx>
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
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <docary.hxx>
#include <ndtxt.hxx>
#include <paratr.hxx>
#include <cellatr.hxx>
#include <fldbas.hxx>
#include <pam.hxx>
#include <swtable.hxx>
#include <ndgrf.hxx>
#include <UndoCore.hxx>
#include <IMark.hxx>
#include <charfmt.hxx>
#include <comcore.hrc>
#include <undo.hrc>
#include <bookmrk.hxx>
#include <boost/scoped_ptr.hpp>

OUString SwHistoryHint::GetDescription() const
{
    return OUString();
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
            // save formulas always in plain text
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

OUString SwHistorySetFmt::GetDescription() const
{
    OUString aResult;

    sal_uInt16 nWhich = m_pAttr->Which();
    switch (nWhich)
    {
    case RES_BREAK:
        switch ((static_cast<SvxFmtBreakItem &>(*m_pAttr)).GetBreak())
        {
        case SVX_BREAK_PAGE_BEFORE:
        case SVX_BREAK_PAGE_AFTER:
        case SVX_BREAK_PAGE_BOTH:
            aResult = SW_RESSTR(STR_UNDO_PAGEBREAKS);

            break;
        case SVX_BREAK_COLUMN_BEFORE:
        case SVX_BREAK_COLUMN_AFTER:
        case SVX_BREAK_COLUMN_BOTH:
            aResult = SW_RESSTR(STR_UNDO_COLBRKS);

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

SwHistoryResetFmt::SwHistoryResetFmt(const SfxPoolItem* pFmtHt, sal_uLong nNodeIdx)
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
    , m_bFormatIgnoreStart(pTxtHt->IsFormatIgnoreStart())
    , m_bFormatIgnoreEnd  (pTxtHt->IsFormatIgnoreEnd  ())
{
    // Caution: the following attributes generate no format attributes:
    //  - NoLineBreak, NoHypen, Inserted, Deleted
    // These cases must be handled separately !!!

    // a little bit complicated but works: first assign a copy of the
    // default value and afterwards the values from text attribute
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
        if ( !pDoc->GetCharFmts()->Contains( (static_cast<SwFmtCharFmt&>(*m_pAttr)).GetCharFmt() ) )
            return; // do not set, format does not exist
    }

    SwTxtNode * pTxtNd = pDoc->GetNodes()[ m_nNodeIndex ]->GetTxtNode();
    OSL_ENSURE( pTxtNd, "SwHistorySetTxt::SetInDoc: not a TextNode" );

    if ( pTxtNd )
    {
        SwTxtAttr *const pAttr = pTxtNd->InsertItem(*m_pAttr, m_nStart, m_nEnd,
                        nsSetAttrMode::SETATTR_NOTXTATRCHR |
                        nsSetAttrMode::SETATTR_NOHINTADJUST );
        // shouldn't be possible to hit any error/merging path from here
        assert(pAttr);
        if (m_bFormatIgnoreStart)
        {
            pAttr->SetFormatIgnoreStart(true);
        }
        if (m_bFormatIgnoreEnd)
        {
            pAttr->SetFormatIgnoreEnd(true);
        }
    }
}

SwHistorySetTxtFld::SwHistorySetTxtFld( SwTxtFld* pTxtFld, sal_uLong nNodePos )
    : SwHistoryHint( HSTRY_SETTXTFLDHNT )
    , m_pFldType( 0 )
    , m_pFld( new SwFmtFld( *pTxtFld->GetFmtFld().GetField() ) )
{
    // only copy if not Sys-FieldType
    SwDoc* pDoc = pTxtFld->GetTxtNode().GetDoc();

    m_nFldWhich = m_pFld->GetField()->GetTyp()->Which();
    if (m_nFldWhich == RES_DBFLD ||
        m_nFldWhich == RES_USERFLD ||
        m_nFldWhich == RES_SETEXPFLD ||
        m_nFldWhich == RES_DDEFLD ||
        !pDoc->GetSysFldType( m_nFldWhich ))
    {
        m_pFldType.reset( m_pFld->GetField()->GetTyp()->Copy() );
        m_pFld->GetField()->ChgTyp( m_pFldType.get() ); // change field type
    }
    m_nNodeIndex = nNodePos;
    m_nPos = *pTxtFld->GetStart();
}

OUString SwHistorySetTxtFld::GetDescription() const
{
    return m_pFld->GetField()->GetDescription();
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

    m_pFld->GetField()->ChgTyp( pNewFldType ); // change field type

    SwTxtNode * pTxtNd = pDoc->GetNodes()[ m_nNodeIndex ]->GetTxtNode();
    OSL_ENSURE( pTxtNd, "SwHistorySetTxtFld: no TextNode" );

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
    OSL_ENSURE( pTxtNd, "SwHistorySetRefMark: no TextNode" );
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
    OSL_ENSURE( pTxtNd, "SwHistorySetTOXMark: no TextNode" );
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

bool SwHistorySetTOXMark::IsEqual( const SwTOXMark& rCmp ) const
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
            sal_Int32 nAttrStart, sal_Int32 nAttrEnd, sal_uLong nNodePos )
    : SwHistoryHint( HSTRY_RESETTXTHNT )
    , m_nNodeIndex( nNodePos ), m_nStart( nAttrStart ), m_nEnd( nAttrEnd )
    , m_nAttr( nWhich )
{
}

void SwHistoryResetTxt::SetInDoc( SwDoc* pDoc, bool )
{
    SwTxtNode * pTxtNd = pDoc->GetNodes()[ m_nNodeIndex ]->GetTxtNode();
    OSL_ENSURE( pTxtNd, "SwHistoryResetTxt: no TextNode" );
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
    OSL_ENSURE( pTxtFtn->GetStartNode(),
            "SwHistorySetFootnote: Footnote without Section" );

    // keep the old NodePos (because who knows what later will be saved/deleted
    // in SaveSection)
    SwDoc* pDoc = const_cast<SwDoc*>(pTxtFtn->GetTxtNode().GetDoc());
    SwNode* pSaveNd = pDoc->GetNodes()[ m_nNodeIndex ];

    // keep pointer to StartNode of FtnSection and reset its attribute for now
    // (as a result, its/all Frms will be deleted automatically)
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
    OSL_ENSURE( rTxtFtn.GetStartNode(),
            "SwHistorySetFootnote: Footnote without Section" );
}

OUString SwHistorySetFootnote::GetDescription() const
{
    return SW_RES(STR_FOOTNOTE);
}

SwHistorySetFootnote::~SwHistorySetFootnote()
{
}

void SwHistorySetFootnote::SetInDoc( SwDoc* pDoc, bool )
{
    SwTxtNode * pTxtNd = pDoc->GetNodes()[ m_nNodeIndex ]->GetTxtNode();
    OSL_ENSURE( pTxtNd, "SwHistorySetFootnote: no TextNode" );
    if ( !pTxtNd )
        return;

    if ( m_pUndo.get() )
    {
        // set the footnote in the TextNode
        SwFmtFtn aTemp( m_bEndNote );
        SwFmtFtn& rNew = const_cast<SwFmtFtn&>(
                static_cast<const SwFmtFtn&>(pDoc->GetAttrPool().Put(aTemp)) );
        if ( !m_FootnoteNumber.isEmpty() )
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
    OSL_ENSURE( pCntntNd, "SwHistoryChangeFmtColl: no ContentNode" );

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
    OSL_ENSURE( pFlyFmt, "SwHistoryTxtFlyCnt: no Format" );
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
    boost::scoped_ptr<SwPaM> pPam;
    ::sw::mark::IMark* pMark = NULL;

    if(m_bSavePos)
    {
        SwCntntNode* const pCntntNd = rNds[m_nNode]->GetCntntNode();
        OSL_ENSURE(pCntntNd,
            "<SwHistoryBookmark::SetInDoc(..)>"
            " - wrong node for a mark");

        // #111660# don't crash when nNode1 doesn't point to content node.
        if(pCntntNd)
            pPam.reset(new SwPaM(*pCntntNd, m_nCntnt));
    }
    else
    {
        pMark = pMarkAccess->findMark(m_aName)->get();
        pPam.reset(new SwPaM(pMark->GetMarkPos()));
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
        if ( pMark != NULL )
        {
            pMarkAccess->deleteMark( pMark );
        }
        ::sw::mark::IBookmark* const pBookmark =
            dynamic_cast< ::sw::mark::IBookmark* >( pMarkAccess->makeMark(*pPam, m_aName, m_eBkmkType) );
        if ( pBookmark != NULL )
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

const OUString& SwHistoryBookmark::GetName() const
{
    return m_aName;
}

SwHistorySetAttrSet::SwHistorySetAttrSet( const SfxItemSet& rSet,
                        sal_uLong nNodePos, const std::set<sal_uInt16> &rSetArr )
    : SwHistoryHint( HSTRY_SETATTRSET )
    , m_OldSet( rSet )
    , m_ResetArray( 0, 4 )
    , m_nNodeIndex( nNodePos )
{
    SfxItemIter aIter( m_OldSet ), aOrigIter( rSet );
    const SfxPoolItem* pItem = aIter.FirstItem(),
                     * pOrigItem = aOrigIter.FirstItem();
    do {
        if( !rSetArr.count( pOrigItem->Which() ))
        {
            m_ResetArray.push_back( pOrigItem->Which() );
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
                        // When a formula is set, never save the value. It
                        // possibly must be recalculated!
                        // Save formulas always in plain text
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
    } while( true );
}

void SwHistorySetAttrSet::SetInDoc( SwDoc* pDoc, bool )
{
    ::sw::UndoGuard const undoGuard(pDoc->GetIDocumentUndoRedo());

    SwNode * pNode = pDoc->GetNodes()[ m_nNodeIndex ];
    if ( pNode->IsCntntNode() )
    {
        static_cast<SwCntntNode*>(pNode)->SetAttr( m_OldSet );
        if ( !m_ResetArray.empty() )
        {
            static_cast<SwCntntNode*>(pNode)->ResetAttr( m_ResetArray );
        }
    }
    else if ( pNode->IsTableNode() )
    {
        SwFmt& rFmt =
            *static_cast<SwTableNode*>(pNode)->GetTable().GetFrmFmt();
        rFmt.SetFmtAttr( m_OldSet );
        if ( !m_ResetArray.empty() )
        {
            rFmt.ResetFmtAttr( m_ResetArray.front() );
        }
    }
}

SwHistoryResetAttrSet::SwHistoryResetAttrSet( const SfxItemSet& rSet,
                    sal_uLong nNodePos, sal_Int32 nAttrStt, sal_Int32 nAttrEnd )
    : SwHistoryHint( HSTRY_RESETATTRSET )
    , m_nNodeIndex( nNodePos ), m_nStart( nAttrStt ), m_nEnd( nAttrEnd )
    , m_Array( (sal_uInt8)rSet.Count() )
{
    SfxItemIter aIter( rSet );
    bool bAutoStyle = false;

    while( true )
    {
        const sal_uInt16 nWhich = aIter.GetCurItem()->Which();

#ifdef DBG_UTIL
        switch (nWhich)
        {
            case RES_TXTATR_REFMARK:
            case RES_TXTATR_TOXMARK:
                if (m_nStart != m_nEnd) break; // else: fall through!
            case RES_TXTATR_FIELD:
            case RES_TXTATR_ANNOTATION:
            case RES_TXTATR_FLYCNT:
            case RES_TXTATR_FTN:
            case RES_TXTATR_META:
            case RES_TXTATR_METAFIELD:
                OSL_ENSURE(rSet.Count() == 1,
                    "text attribute with CH_TXTATR, but not the only one:"
                    "\nnot such a good idea");
                break;
        }
#endif // DBG_UTIL

        // Character attribute cannot be inserted into the hints array
        // anymore. Therefore we have to treat them as one RES_TXTATR_AUTOFMT:
        if (isCHRATR(nWhich))
        {
            bAutoStyle = true;
        }
        else
        {
            m_Array.push_back( aIter.GetCurItem()->Which() );
        }

        if( aIter.IsAtEnd() )
            break;

        aIter.NextItem();
    }

    if ( bAutoStyle )
    {
        m_Array.push_back( RES_TXTATR_AUTOFMT );
    }
}

void SwHistoryResetAttrSet::SetInDoc( SwDoc* pDoc, bool )
{
    ::sw::UndoGuard const undoGuard(pDoc->GetIDocumentUndoRedo());

    SwCntntNode * pCntntNd = pDoc->GetNodes()[ m_nNodeIndex ]->GetCntntNode();
    OSL_ENSURE( pCntntNd, "SwHistoryResetAttrSet: no CntntNode" );

    if (pCntntNd)
    {
        std::vector<sal_uInt16>::iterator it;
        if ( USHRT_MAX == m_nEnd && USHRT_MAX == m_nStart )
        {
            // no area: use ContentNode
            for ( it = m_Array.begin(); it != m_Array.end(); ++it )
            {
                pCntntNd->ResetAttr( *it );
            }
        }
        else
        {
            // area: use TextNode
            for ( it = m_Array.begin(); it != m_Array.end(); ++it )
            {
                static_cast<SwTxtNode*>(pCntntNd)->
                    DeleteAttributes( *it, m_nStart, m_nEnd );
            }
        }
    }
}

SwHistoryChangeFlyAnchor::SwHistoryChangeFlyAnchor( SwFrmFmt& rFmt )
    : SwHistoryHint( HSTRY_CHGFLYANCHOR )
    , m_rFmt( rFmt )
    , m_nOldNodeIndex( rFmt.GetAnchor().GetCntntAnchor()->nNode.GetIndex() )
    , m_nOldContentIndex( (FLY_AT_CHAR == rFmt.GetAnchor().GetAnchorId())
            ?   rFmt.GetAnchor().GetCntntAnchor()->nContent.GetIndex()
            :   COMPLETE_STRING )
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
        if ( COMPLETE_STRING != m_nOldContentIndex )
        {
            OSL_ENSURE(pCNd, "SwHistoryChangeFlyAnchor: no ContentNode");
            if (pCNd)
            {
                aPos.nContent.Assign( pCNd, m_nOldContentIndex );
            }
        }
        aTmp.SetAnchor( &aPos );

        // so the Layout does not get confused
        if ( !pCNd || !pCNd->getLayoutFrm( pDoc->GetCurrentLayout(), 0, 0, sal_False ) )
        {
            m_rFmt.DelFrms();
        }

        m_rFmt.SetFmtAttr( aTmp );
    }
}

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
                                     const OUString & sFmt)
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

SwHistory::SwHistory( sal_uInt16 nInitSz )
    : m_SwpHstry()
    , m_nEndDiff( 0 )
{
    m_SwpHstry.reserve( (sal_uInt8)nInitSz );
}

SwHistory::~SwHistory()
{
    Delete( 0 );
}

void SwHistory::Add( const SfxPoolItem* pOldValue, const SfxPoolItem* pNewValue,
                     sal_uLong nNodeIdx )
{
    OSL_ENSURE( !m_nEndDiff, "History was not deleted after REDO" );

    sal_uInt16 nWhich = pNewValue->Which();
    if( (nWhich >= POOLATTR_END)
        || (nWhich == RES_TXTATR_FIELD)
        || (nWhich == RES_TXTATR_ANNOTATION) )
        return;

    // no default attribute?
    SwHistoryHint * pHt;
    if ( pOldValue && pOldValue != GetDfltAttr( pOldValue->Which() ) )
    {
        pHt = new SwHistorySetFmt( pOldValue, nNodeIdx );
    }
    else
    {
        pHt = new SwHistoryResetFmt( pNewValue, nNodeIdx );
    }
    m_SwpHstry.push_back( pHt );
}

// FIXME: refactor the following "Add" methods (DRY)?
void SwHistory::Add( SwTxtAttr* pHint, sal_uLong nNodeIdx, bool bNewAttr )
{
    OSL_ENSURE( !m_nEndDiff, "History was not deleted after REDO" );

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
            case RES_TXTATR_ANNOTATION:
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
    m_SwpHstry.push_back( pHt );
}

void SwHistory::Add( SwFmtColl* pColl, sal_uLong nNodeIdx, sal_uInt8 nWhichNd )
{
    OSL_ENSURE( !m_nEndDiff, "History was not deleted after REDO" );

    SwHistoryHint * pHt =
        new SwHistoryChangeFmtColl( pColl, nNodeIdx, nWhichNd );
    m_SwpHstry.push_back( pHt );
}

void SwHistory::Add(const ::sw::mark::IMark& rBkmk, bool bSavePos, bool bSaveOtherPos)
{
    OSL_ENSURE( !m_nEndDiff, "History was not deleted after REDO" );

    SwHistoryHint * pHt = new SwHistoryBookmark(rBkmk, bSavePos, bSaveOtherPos);
    m_SwpHstry.push_back( pHt );
}

void SwHistory::Add( SwFrmFmt& rFmt )
{
    SwHistoryHint * pHt = new SwHistoryChangeFlyAnchor( rFmt );
    m_SwpHstry.push_back( pHt );
}

void SwHistory::Add( SwFlyFrmFmt& rFmt, sal_uInt16& rSetPos )
{
    OSL_ENSURE( !m_nEndDiff, "History was not deleted after REDO" );

    const sal_uInt16 nWh = rFmt.Which();
    if( RES_FLYFRMFMT == nWh || RES_DRAWFRMFMT == nWh )
    {
        SwHistoryHint * pHint = new SwHistoryTxtFlyCnt( &rFmt );
        m_SwpHstry.push_back( pHint );

        const SwFmtChain* pChainItem;
        if( SFX_ITEM_SET == rFmt.GetItemState( RES_CHAIN, sal_False,
            (const SfxPoolItem**)&pChainItem ))
        {
            if( pChainItem->GetNext() || pChainItem->GetPrev() )
            {
                SwHistoryHint * pHt =
                    new SwHistoryChangeFlyChain( rFmt, *pChainItem );
                m_SwpHstry.insert( m_SwpHstry.begin() + rSetPos++, pHt );
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
    m_SwpHstry.push_back( pHt );
}

// #i27615#
void SwHistory::Add(const SfxItemSet & rSet, const SwCharFmt & rFmt)
{
    SwHistoryHint * pHt = new SwHistoryChangeCharFmt(rSet, rFmt.GetName());
    m_SwpHstry.push_back( pHt );
}

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
    m_SwpHstry.erase( m_SwpHstry.begin() + nStart, m_SwpHstry.end() );
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
        delete m_SwpHstry[ --n ];
        m_SwpHstry.erase( m_SwpHstry.begin() + n );
    }
    m_nEndDiff = 0;
}

sal_uInt16 SwHistory::SetTmpEnd( sal_uInt16 nNewTmpEnd )
{
    OSL_ENSURE( nNewTmpEnd <= Count(), "SwHistory::SetTmpEnd: out of bounds" );

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
        } while( true );
    }
}

void SwHistory::CopyAttr(
    SwpHints* pHts,
    const sal_uLong nNodeIdx,
    const sal_Int32 nStart,
    const sal_Int32 nEnd,
    const bool bCopyFields )
{
    if( !pHts  )
        return;

    // copy all attributes of the TextNode in the area from nStart to nEnd
    SwTxtAttr* pHt;
    const sal_Int32 * pEndIdx;
    for( sal_uInt16 n = 0; n < pHts->Count(); n++ )
    {
        // nAttrStt must even be set when !pEndIdx
        pHt = pHts->GetTextHint(n);
        const sal_Int32 nAttrStt = *pHt->GetStart();
        if( 0 != ( pEndIdx = pHt->GetEnd() ) && nAttrStt > nEnd )
            break;

        // never copy Flys and Ftn !!
        bool bNextAttr = false;
        switch( pHt->Which() )
        {
        case RES_TXTATR_FIELD:
        case RES_TXTATR_ANNOTATION:
        case RES_TXTATR_INPUTFIELD:
            if( !bCopyFields )
                bNextAttr = true;
            break;
        case RES_TXTATR_FLYCNT:
        case RES_TXTATR_FTN:
            bNextAttr = true;
            break;
        }

        if( bNextAttr )
            continue;

        // save all attributes that are somehow in this area
        if ( nStart <= nAttrStt )
        {
            if ( nEnd > nAttrStt )
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

// Class to register the history at a Node, Format, HintsArray, ...
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
    // #i114861#
    // Do not handle a "noop" modify
    // - e.g. <SwTxtNode::NumRuleChgd()> uses such a "noop" modify
    if ( m_pHistory && ( pOld || pNew ) &&
         pOld != pNew )
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
                if ( m_WhichIdSet.count( pItem->Which() ) )
                {
                    pNewHstr = new SwHistorySetFmt( pItem, m_nNodeIndex );
                }
                else
                {
                    pNewHstr = new SwHistoryResetFmt( pItem, m_nNodeIndex );
                }
            }
            m_pHistory->m_SwpHstry.push_back( pNewHstr );
        }
    }
}

void SwRegHistory::AddHint( SwTxtAttr* pHt, const bool bNew )
{
    m_pHistory->Add( pHt, m_nNodeIndex, bNew );
}

bool SwRegHistory::InsertItems( const SfxItemSet& rSet,
    sal_Int32 const nStart, sal_Int32 const nEnd, SetAttrMode const nFlags )
{
    if( !rSet.Count() )
        return false;

    SwTxtNode * const pTxtNode =
        dynamic_cast<SwTxtNode *>(const_cast<SwModify *>(GetRegisteredIn()));

    OSL_ENSURE(pTxtNode, "SwRegHistory not registered at text node?");
    if (!pTxtNode)
        return false;

    if ( pTxtNode->GetpSwpHints() && m_pHistory )
    {
        pTxtNode->GetpSwpHints()->Register( this );
    }

    const bool bInserted = pTxtNode->SetAttr( rSet, nStart, nEnd, nFlags );

    // Caution: The array can be deleted when inserting an attribute!
    // This can happen when the value that should be added first deletes
    // an existing attribute but does not need to be added itself because
    // the paragraph attributes are identical
    // ( -> bForgetAttr in SwpHints::Insert )
    if ( pTxtNode->GetpSwpHints() && m_pHistory )
    {
        pTxtNode->GetpSwpHints()->DeRegister();
    }

    if ( m_pHistory && bInserted )
    {
        SwHistoryHint* pNewHstr = new SwHistoryResetAttrSet( rSet,
                                    pTxtNode->GetIndex(), nStart, nEnd );
        // the NodeIndex might be moved!

        m_pHistory->m_SwpHstry.push_back( pNewHstr );
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
    else
    {
        m_WhichIdSet.clear();
    }
}

void SwRegHistory::_MakeSetWhichIds()
{
    if (!m_pHistory) return;

    m_WhichIdSet.clear();

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
            while( true )
            {
                m_WhichIdSet.insert( nW );
                if( aIter.IsAtEnd() )
                    break;
                nW = aIter.NextItem()->Which();
            }
        }
    }
}

SwpHstry::~SwpHstry()
{
    for(const_iterator it = begin(); it != end(); ++it)
        delete *it;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
