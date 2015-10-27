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
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
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
#include <memory>

OUString SwHistoryHint::GetDescription() const
{
    return OUString();
}

SwHistorySetFormat::SwHistorySetFormat( const SfxPoolItem* pFormatHt, sal_uLong nNd )
    :  SwHistoryHint( HSTRY_SETFMTHNT )
    ,  m_pAttr( pFormatHt->Clone() )
    ,  m_nNodeIndex( nNd )
{
    switch ( m_pAttr->Which() )
    {
        case RES_PAGEDESC:
            static_cast<SwFormatPageDesc&>(*m_pAttr).ChgDefinedIn( 0 );
            break;
        case RES_PARATR_DROP:
            static_cast<SwFormatDrop&>(*m_pAttr).ChgDefinedIn( 0 );
            break;
        case RES_BOXATR_FORMULA:
        {
            // save formulas always in plain text
            SwTableBoxFormula& rNew = static_cast<SwTableBoxFormula&>(*m_pAttr);
            if ( rNew.IsIntrnlName() )
            {
                const SwTableBoxFormula& rOld =
                    *static_cast<const SwTableBoxFormula*>(pFormatHt);
                const SwNode* pNd = rOld.GetNodeOfFormula();
                if ( pNd )
                {
                    const SwTableNode* pTableNode = pNd->FindTableNode();
                    if (pTableNode)
                    {
                        SwTableFormulaUpdate aMsgHint( &pTableNode->GetTable() );
                        aMsgHint.m_eFlags = TBL_BOXNAME;
                        rNew.ChgDefinedIn( rOld.GetDefinedIn() );
                        rNew.ChangeState( &aMsgHint );
                    }
                }
            }
            rNew.ChgDefinedIn( 0 );
        }
        break;
    }
}

OUString SwHistorySetFormat::GetDescription() const
{
    OUString aResult;

    switch (m_pAttr->Which())
    {
    case RES_BREAK:
        switch ((static_cast<SvxFormatBreakItem &>(*m_pAttr)).GetBreak())
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

void SwHistorySetFormat::SetInDoc( SwDoc* pDoc, bool bTmpSet )
{
    SwNode * pNode = pDoc->GetNodes()[ m_nNodeIndex ];
    if ( pNode->IsContentNode() )
    {
        static_cast<SwContentNode*>(pNode)->SetAttr( *m_pAttr );
    }
    else if ( pNode->IsTableNode() )
    {
        static_cast<SwTableNode*>(pNode)->GetTable().GetFrameFormat()->SetFormatAttr(
                *m_pAttr );
    }
    else if ( pNode->IsStartNode() && (SwTableBoxStartNode ==
                static_cast<SwStartNode*>(pNode)->GetStartNodeType()) )
    {
        SwTableNode* pTNd = pNode->FindTableNode();
        if ( pTNd )
        {
            SwTableBox* pBox = pTNd->GetTable().GetTableBox( m_nNodeIndex );
            if (pBox)
            {
                pBox->ClaimFrameFormat()->SetFormatAttr( *m_pAttr );
            }
        }
    }

    if ( !bTmpSet )
    {
        m_pAttr.reset();
    }
}

SwHistorySetFormat::~SwHistorySetFormat()
{
}

SwHistoryResetFormat::SwHistoryResetFormat(const SfxPoolItem* pFormatHt, sal_uLong nNodeIdx)
    : SwHistoryHint( HSTRY_RESETFMTHNT )
    , m_nNodeIndex( nNodeIdx )
    , m_nWhich( pFormatHt->Which() )
{
}

void SwHistoryResetFormat::SetInDoc( SwDoc* pDoc, bool )
{
    SwNode * pNode = pDoc->GetNodes()[ m_nNodeIndex ];
    if ( pNode->IsContentNode() )
    {
        static_cast<SwContentNode*>(pNode)->ResetAttr( m_nWhich );
    }
    else if ( pNode->IsTableNode() )
    {
        static_cast<SwTableNode*>(pNode)->GetTable().GetFrameFormat()->
            ResetFormatAttr( m_nWhich );
    }
}

SwHistorySetText::SwHistorySetText( SwTextAttr* pTextHt, sal_uLong nNodePos )
    : SwHistoryHint( HSTRY_SETTXTHNT )
    , m_nNodeIndex( nNodePos )
    , m_nStart( pTextHt->GetStart() )
    , m_nEnd( *pTextHt->GetAnyEnd() )
    , m_bFormatIgnoreStart(pTextHt->IsFormatIgnoreStart())
    , m_bFormatIgnoreEnd  (pTextHt->IsFormatIgnoreEnd  ())
{
    // Caution: the following attributes generate no format attributes:
    //  - NoLineBreak, NoHypen, Inserted, Deleted
    // These cases must be handled separately !!!

    // a little bit complicated but works: first assign a copy of the
    // default value and afterwards the values from text attribute
    if ( RES_TXTATR_CHARFMT == pTextHt->Which() )
    {
        m_pAttr.reset( new SwFormatCharFormat( pTextHt->GetCharFormat().GetCharFormat() ) );
    }
    else
    {
        m_pAttr.reset( pTextHt->GetAttr().Clone() );
    }
}

SwHistorySetText::~SwHistorySetText()
{
}

void SwHistorySetText::SetInDoc( SwDoc* pDoc, bool )
{
    if ( !m_pAttr.get() )
        return;

    if ( RES_TXTATR_CHARFMT == m_pAttr->Which() )
    {
        // ask the Doc if the CharFormat still exists
        if ( !pDoc->GetCharFormats()->Contains( (static_cast<SwFormatCharFormat&>(*m_pAttr)).GetCharFormat() ) )
            return; // do not set, format does not exist
    }

    SwTextNode * pTextNd = pDoc->GetNodes()[ m_nNodeIndex ]->GetTextNode();
    OSL_ENSURE( pTextNd, "SwHistorySetText::SetInDoc: not a TextNode" );

    if ( pTextNd )
    {
        SwTextAttr *const pAttr = pTextNd->InsertItem(*m_pAttr, m_nStart, m_nEnd,
                        SetAttrMode::NOTXTATRCHR |
                        SetAttrMode::NOHINTADJUST );
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

SwHistorySetTextField::SwHistorySetTextField( SwTextField* pTextField, sal_uLong nNodePos )
    : SwHistoryHint( HSTRY_SETTXTFLDHNT )
    , m_pField( new SwFormatField( *pTextField->GetFormatField().GetField() ) )
{
    // only copy if not Sys-FieldType
    SwDoc* pDoc = pTextField->GetTextNode().GetDoc();

    m_nFieldWhich = m_pField->GetField()->GetTyp()->Which();
    if (m_nFieldWhich == RES_DBFLD ||
        m_nFieldWhich == RES_USERFLD ||
        m_nFieldWhich == RES_SETEXPFLD ||
        m_nFieldWhich == RES_DDEFLD ||
        !pDoc->getIDocumentFieldsAccess().GetSysFieldType( m_nFieldWhich ))
    {
        m_pFieldType.reset( m_pField->GetField()->GetTyp()->Copy() );
        m_pField->GetField()->ChgTyp( m_pFieldType.get() ); // change field type
    }
    m_nNodeIndex = nNodePos;
    m_nPos = pTextField->GetStart();
}

OUString SwHistorySetTextField::GetDescription() const
{
    return m_pField->GetField()->GetDescription();
}

SwHistorySetTextField::~SwHistorySetTextField()
{
}

void SwHistorySetTextField::SetInDoc( SwDoc* pDoc, bool )
{
    if ( !m_pField.get() )
        return;

    SwFieldType* pNewFieldType = m_pFieldType.get();
    if ( !pNewFieldType )
    {
        pNewFieldType = pDoc->getIDocumentFieldsAccess().GetSysFieldType( m_nFieldWhich );
    }
    else
    {
        // register type with the document
        pNewFieldType = pDoc->getIDocumentFieldsAccess().InsertFieldType( *m_pFieldType );
    }

    m_pField->GetField()->ChgTyp( pNewFieldType ); // change field type

    SwTextNode * pTextNd = pDoc->GetNodes()[ m_nNodeIndex ]->GetTextNode();
    OSL_ENSURE( pTextNd, "SwHistorySetTextField: no TextNode" );

    if ( pTextNd )
    {
        pTextNd->InsertItem( *m_pField, m_nPos, m_nPos,
                    SetAttrMode::NOTXTATRCHR );
    }
}

SwHistorySetRefMark::SwHistorySetRefMark( SwTextRefMark* pTextHt, sal_uLong nNodePos )
    : SwHistoryHint( HSTRY_SETREFMARKHNT )
    , m_RefName( pTextHt->GetRefMark().GetRefName() )
    , m_nNodeIndex( nNodePos )
    , m_nStart( pTextHt->GetStart() )
    , m_nEnd( *pTextHt->GetAnyEnd() )
{
}

void SwHistorySetRefMark::SetInDoc( SwDoc* pDoc, bool )
{
    SwTextNode * pTextNd = pDoc->GetNodes()[ m_nNodeIndex ]->GetTextNode();
    OSL_ENSURE( pTextNd, "SwHistorySetRefMark: no TextNode" );
    if ( !pTextNd )
        return;

    SwFormatRefMark aRefMark( m_RefName );

    // if a reference mark without an end already exists here: must not insert!
    if ( m_nStart != m_nEnd ||
         !pTextNd->GetTextAttrForCharAt( m_nStart, RES_TXTATR_REFMARK ) )
    {
        pTextNd->InsertItem( aRefMark, m_nStart, m_nEnd,
                            SetAttrMode::NOTXTATRCHR );
    }
}

SwHistorySetTOXMark::SwHistorySetTOXMark( SwTextTOXMark* pTextHt, sal_uLong nNodePos )
    : SwHistoryHint( HSTRY_SETTOXMARKHNT )
    , m_TOXMark( pTextHt->GetTOXMark() )
    , m_TOXName( m_TOXMark.GetTOXType()->GetTypeName() )
    , m_eTOXTypes( m_TOXMark.GetTOXType()->GetType() )
    , m_nNodeIndex( nNodePos )
    , m_nStart( pTextHt->GetStart() )
    , m_nEnd( *pTextHt->GetAnyEnd() )
{
    m_TOXMark.DeRegister();
}

void SwHistorySetTOXMark::SetInDoc( SwDoc* pDoc, bool )
{
    SwTextNode * pTextNd = pDoc->GetNodes()[ m_nNodeIndex ]->GetTextNode();
    OSL_ENSURE( pTextNd, "SwHistorySetTOXMark: no TextNode" );
    if ( !pTextNd )
        return;

    // search for respective TOX type
    const sal_uInt16 nCnt = pDoc->GetTOXTypeCount( m_eTOXTypes );
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

    pTextNd->InsertItem( aNew, m_nStart, m_nEnd,
                        SetAttrMode::NOTXTATRCHR );
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

SwHistoryResetText::SwHistoryResetText( sal_uInt16 nWhich,
            sal_Int32 nAttrStart, sal_Int32 nAttrEnd, sal_uLong nNodePos )
    : SwHistoryHint( HSTRY_RESETTXTHNT )
    , m_nNodeIndex( nNodePos ), m_nStart( nAttrStart ), m_nEnd( nAttrEnd )
    , m_nAttr( nWhich )
{
}

void SwHistoryResetText::SetInDoc( SwDoc* pDoc, bool )
{
    SwTextNode * pTextNd = pDoc->GetNodes()[ m_nNodeIndex ]->GetTextNode();
    OSL_ENSURE( pTextNd, "SwHistoryResetText: no TextNode" );
    if ( pTextNd )
    {
        pTextNd->DeleteAttributes( m_nAttr, m_nStart, m_nEnd );
    }
}

SwHistorySetFootnote::SwHistorySetFootnote( SwTextFootnote* pTextFootnote, sal_uLong nNodePos )
    : SwHistoryHint( HSTRY_SETFTNHNT )
    , m_pUndo( new SwUndoSaveSection )
    , m_FootnoteNumber( pTextFootnote->GetFootnote().GetNumStr() )
    , m_nNodeIndex( nNodePos )
    , m_nStart( pTextFootnote->GetStart() )
    , m_bEndNote( pTextFootnote->GetFootnote().IsEndNote() )
{
    OSL_ENSURE( pTextFootnote->GetStartNode(),
            "SwHistorySetFootnote: Footnote without Section" );

    // keep the old NodePos (because who knows what later will be saved/deleted
    // in SaveSection)
    SwDoc* pDoc = const_cast<SwDoc*>(pTextFootnote->GetTextNode().GetDoc());
    SwNode* pSaveNd = pDoc->GetNodes()[ m_nNodeIndex ];

    // keep pointer to StartNode of FootnoteSection and reset its attribute for now
    // (as a result, its/all Frms will be deleted automatically)
    SwNodeIndex aSttIdx( *pTextFootnote->GetStartNode() );
    pTextFootnote->SetStartNode( 0, false );

    m_pUndo->SaveSection( aSttIdx );
    m_nNodeIndex = pSaveNd->GetIndex();
}

SwHistorySetFootnote::SwHistorySetFootnote( const SwTextFootnote &rTextFootnote )
    : SwHistoryHint( HSTRY_SETFTNHNT )
    , m_FootnoteNumber( rTextFootnote.GetFootnote().GetNumStr() )
    , m_nNodeIndex( _SwTextFootnote_GetIndex( (&rTextFootnote) ) )
    , m_nStart( rTextFootnote.GetStart() )
    , m_bEndNote( rTextFootnote.GetFootnote().IsEndNote() )
{
    OSL_ENSURE( rTextFootnote.GetStartNode(),
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
    SwTextNode * pTextNd = pDoc->GetNodes()[ m_nNodeIndex ]->GetTextNode();
    OSL_ENSURE( pTextNd, "SwHistorySetFootnote: no TextNode" );
    if ( !pTextNd )
        return;

    if ( m_pUndo.get() )
    {
        // set the footnote in the TextNode
        SwFormatFootnote aTemp( m_bEndNote );
        SwFormatFootnote& rNew = const_cast<SwFormatFootnote&>(
                static_cast<const SwFormatFootnote&>(pDoc->GetAttrPool().Put(aTemp)) );
        if ( !m_FootnoteNumber.isEmpty() )
        {
            rNew.SetNumStr( m_FootnoteNumber );
        }
        SwTextFootnote* pTextFootnote = new SwTextFootnote( rNew, m_nStart );

        // create the section of the Footnote
        SwNodeIndex aIdx( *pTextNd );
        m_pUndo->RestoreSection( pDoc, &aIdx, SwFootnoteStartNode );
        pTextFootnote->SetStartNode( &aIdx );
        if ( m_pUndo->GetHistory() )
        {
            // create frames only now
            m_pUndo->GetHistory()->Rollback( pDoc );
        }

        pTextNd->InsertHint( pTextFootnote );
    }
    else
    {
        SwTextFootnote * const pFootnote =
            const_cast<SwTextFootnote*>( static_cast<const SwTextFootnote*>(
                pTextNd->GetTextAttrForCharAt( m_nStart )));
        SwFormatFootnote &rFootnote = const_cast<SwFormatFootnote&>(pFootnote->GetFootnote());
        rFootnote.SetNumStr( m_FootnoteNumber  );
        if ( rFootnote.IsEndNote() != m_bEndNote )
        {
            rFootnote.SetEndNote( m_bEndNote );
            pFootnote->CheckCondColl();
        }
    }
}

SwHistoryChangeFormatColl::SwHistoryChangeFormatColl( SwFormatColl* pFormatColl, sal_uLong nNd,
                            sal_uInt8 nNodeWhich )
    : SwHistoryHint( HSTRY_CHGFMTCOLL )
    , m_pColl( pFormatColl )
    , m_nNodeIndex( nNd )
    , m_nNodeType( nNodeWhich )
{
}

void SwHistoryChangeFormatColl::SetInDoc( SwDoc* pDoc, bool )
{
    SwContentNode * pContentNd = pDoc->GetNodes()[ m_nNodeIndex ]->GetContentNode();
    OSL_ENSURE( pContentNd, "SwHistoryChangeFormatColl: no ContentNode" );

    // before setting the format, check if it is still available in the
    // document. if it has been deleted, there is no undo!
    if ( pContentNd && m_nNodeType == pContentNd->GetNodeType() )
    {
        if ( ND_TEXTNODE == m_nNodeType )
        {
            if (pDoc->GetTextFormatColls()->Contains( static_cast<SwTextFormatColl * const>(m_pColl) ))
            {
                pContentNd->ChgFormatColl( m_pColl );
            }
        }
        else if (pDoc->GetGrfFormatColls()->Contains( static_cast<SwGrfFormatColl * const>(m_pColl) ))
        {
            pContentNd->ChgFormatColl( m_pColl );
        }
    }
}

SwHistoryTextFlyCnt::SwHistoryTextFlyCnt( SwFrameFormat* const pFlyFormat )
    : SwHistoryHint( HSTRY_FLYCNT )
    , m_pUndo( new SwUndoDelLayFormat( pFlyFormat ) )
{
    OSL_ENSURE( pFlyFormat, "SwHistoryTextFlyCnt: no Format" );
    m_pUndo->ChgShowSel( false );
}

SwHistoryTextFlyCnt::~SwHistoryTextFlyCnt()
{
}

void SwHistoryTextFlyCnt::SetInDoc( SwDoc* pDoc, bool )
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
    , m_nContent(bSavePos ?
        rBkmk.GetMarkPos().nContent.GetIndex() : 0)
    , m_nOtherContent(bSaveOtherPos ?
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
    std::unique_ptr<SwPaM> pPam;
    ::sw::mark::IMark* pMark = NULL;

    if(m_bSavePos)
    {
        SwContentNode* const pContentNd = rNds[m_nNode]->GetContentNode();
        OSL_ENSURE(pContentNd,
            "<SwHistoryBookmark::SetInDoc(..)>"
            " - wrong node for a mark");

        // #111660# don't crash when nNode1 doesn't point to content node.
        if(pContentNd)
            pPam.reset(new SwPaM(*pContentNd, m_nContent));
    }
    else
    {
        pMark = pMarkAccess->findMark(m_aName)->get();
        pPam.reset(new SwPaM(pMark->GetMarkPos()));
    }

    if(m_bSaveOtherPos)
    {
        SwContentNode* const pContentNd = rNds[m_nOtherNode]->GetContentNode();
        OSL_ENSURE(pContentNd,
            "<SwHistoryBookmark::SetInDoc(..)>"
            " - wrong node for a mark");

        if(pPam.get() != NULL && pContentNd)
        {
            pPam->SetMark();
            pPam->GetMark()->nNode = m_nOtherNode;
            pPam->GetMark()->nContent.Assign(pContentNd, m_nOtherContent);
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
        && m_nContent == rBkmk.GetMarkPos().nContent.GetIndex()
        && m_aName == rBkmk.GetName();
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
                    static_cast<SwFormatPageDesc*>(
                        const_cast<SfxPoolItem*>(pItem))->ChgDefinedIn( 0 );
                    break;

                case RES_PARATR_DROP:
                    static_cast<SwFormatDrop*>(
                        const_cast<SfxPoolItem*>(pItem))->ChgDefinedIn( 0 );
                    break;

                case RES_BOXATR_FORMULA:
                    {
                        // When a formula is set, never save the value. It
                        // possibly must be recalculated!
                        // Save formulas always in plain text
                        m_OldSet.ClearItem( RES_BOXATR_VALUE );

                        SwTableBoxFormula& rNew =
                            *static_cast<SwTableBoxFormula*>(
                                const_cast<SfxPoolItem*>(pItem));
                        if ( rNew.IsIntrnlName() )
                        {
                            const SwTableBoxFormula& rOld =
                                static_cast<const SwTableBoxFormula&>(
                                        rSet.Get( RES_BOXATR_FORMULA ));
                            const SwNode* pNd = rOld.GetNodeOfFormula();
                            if ( pNd )
                            {
                                const SwTableNode* pTableNode
                                    = pNd->FindTableNode();
                                if (pTableNode)
                                {
                                    SwTableFormulaUpdate aMsgHint(
                                        &pTableNode->GetTable() );
                                    aMsgHint.m_eFlags = TBL_BOXNAME;
                                    rNew.ChgDefinedIn( rOld.GetDefinedIn() );
                                    rNew.ChangeState( &aMsgHint );
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
    if ( pNode->IsContentNode() )
    {
        static_cast<SwContentNode*>(pNode)->SetAttr( m_OldSet );
        if ( !m_ResetArray.empty() )
        {
            static_cast<SwContentNode*>(pNode)->ResetAttr( m_ResetArray );
        }
    }
    else if ( pNode->IsTableNode() )
    {
        SwFormat& rFormat =
            *static_cast<SwTableNode*>(pNode)->GetTable().GetFrameFormat();
        rFormat.SetFormatAttr( m_OldSet );
        if ( !m_ResetArray.empty() )
        {
            rFormat.ResetFormatAttr( m_ResetArray.front() );
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

    SwContentNode * pContentNd = pDoc->GetNodes()[ m_nNodeIndex ]->GetContentNode();
    OSL_ENSURE( pContentNd, "SwHistoryResetAttrSet: no ContentNode" );

    if (pContentNd)
    {
        std::vector<sal_uInt16>::iterator it;
        if ( USHRT_MAX == m_nEnd && USHRT_MAX == m_nStart )
        {
            // no area: use ContentNode
            for ( it = m_Array.begin(); it != m_Array.end(); ++it )
            {
                pContentNd->ResetAttr( *it );
            }
        }
        else
        {
            // area: use TextNode
            for ( it = m_Array.begin(); it != m_Array.end(); ++it )
            {
                pContentNd->GetTextNode()-> DeleteAttributes( *it, m_nStart, m_nEnd );
            }
        }
    }
}

SwHistoryChangeFlyAnchor::SwHistoryChangeFlyAnchor( SwFrameFormat& rFormat )
    : SwHistoryHint( HSTRY_CHGFLYANCHOR )
    , m_rFormat( rFormat )
    , m_nOldNodeIndex( rFormat.GetAnchor().GetContentAnchor()->nNode.GetIndex() )
    , m_nOldContentIndex( (FLY_AT_CHAR == rFormat.GetAnchor().GetAnchorId())
            ?   rFormat.GetAnchor().GetContentAnchor()->nContent.GetIndex()
            :   COMPLETE_STRING )
{
}

void SwHistoryChangeFlyAnchor::SetInDoc( SwDoc* pDoc, bool )
{
    ::sw::UndoGuard const undoGuard(pDoc->GetIDocumentUndoRedo());

    if ( pDoc->GetSpzFrameFormats()->Contains( &m_rFormat ) )    // Format does still exist
    {
        SwFormatAnchor aTmp( m_rFormat.GetAnchor() );

        SwNode* pNd = pDoc->GetNodes()[ m_nOldNodeIndex ];
        SwContentNode* pCNd = pNd->GetContentNode();
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
        if ( !pCNd || !pCNd->getLayoutFrm( pDoc->getIDocumentLayoutAccess().GetCurrentLayout(), 0, 0, false ) )
        {
            m_rFormat.DelFrms();
        }

        m_rFormat.SetFormatAttr( aTmp );
    }
}

SwHistoryChangeFlyChain::SwHistoryChangeFlyChain( SwFlyFrameFormat& rFormat,
                                        const SwFormatChain& rAttr )
    : SwHistoryHint( HSTRY_CHGFLYCHAIN )
    , m_pPrevFormat( rAttr.GetPrev() )
    , m_pNextFormat( rAttr.GetNext() )
    , m_pFlyFormat( &rFormat )
{
}

void SwHistoryChangeFlyChain::SetInDoc( SwDoc* pDoc, bool )
{
    if (pDoc->GetSpzFrameFormats()->Contains( m_pFlyFormat ) )
    {
        SwFormatChain aChain;

        if ( m_pPrevFormat &&
             pDoc->GetSpzFrameFormats()->Contains( m_pPrevFormat ) )
        {
            aChain.SetPrev( m_pPrevFormat );
            SwFormatChain aTmp( m_pPrevFormat->GetChain() );
            aTmp.SetNext( m_pFlyFormat );
            m_pPrevFormat->SetFormatAttr( aTmp );
        }

        if ( m_pNextFormat &&
             pDoc->GetSpzFrameFormats()->Contains( m_pNextFormat ) )
        {
            aChain.SetNext( m_pNextFormat );
            SwFormatChain aTmp( m_pNextFormat->GetChain() );
            aTmp.SetPrev( m_pFlyFormat );
            m_pNextFormat->SetFormatAttr( aTmp );
        }

        if ( aChain.GetNext() || aChain.GetPrev() )
        {
            m_pFlyFormat->SetFormatAttr( aChain );
        }
    }
}

// -> #i27615#
SwHistoryChangeCharFormat::SwHistoryChangeCharFormat(const SfxItemSet & rSet,
                                     const OUString & sFormat)
    : SwHistoryHint(HSTRY_CHGCHARFMT)
    , m_OldSet(rSet), m_Format(sFormat)
{
}

void SwHistoryChangeCharFormat::SetInDoc(SwDoc * pDoc, bool )
{
    SwCharFormat * pCharFormat = pDoc->FindCharFormatByName(m_Format);

    if (pCharFormat)
    {
        pCharFormat->SetFormatAttr(m_OldSet);
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
    Delete();
    std::vector<SwHistoryHint*>::const_iterator it;
    for(it = m_SwpHstry.begin(); it != m_SwpHstry.end(); ++it)
        delete *it;
}

void SwHistory::Add(
    const SfxPoolItem* pOldValue,
    const SfxPoolItem* pNewValue,
    sal_uLong nNodeIdx)
{
    OSL_ENSURE( !m_nEndDiff, "History was not deleted after REDO" );
    const sal_uInt16 nWhich(pNewValue->Which());

    // excluded values
    if(nWhich == RES_TXTATR_FIELD || nWhich == RES_TXTATR_ANNOTATION)
    {
        return;
    }

    // no default Attribute?
    SwHistoryHint* pHt = 0;

    //UUUU To be able to include the DrawingLayer FillItems something more
    // general has to be done to check if an Item is default than to check
    // if its pointer equals that in Writer's global PoolDefaults (held in
    // aAttrTab and used to fill the pool defaults in Writer - looks as if
    // Writer is *older* than the SfxItemPool ?). I checked the possibility to
    // get the SfxItemPool here (works), but decided to use the SfxPoolItem's
    // global tooling aka IsDefaultItem(const SfxPoolItem*) for now
    if(pOldValue && !IsDefaultItem(pOldValue))
    {
        pHt = new SwHistorySetFormat( pOldValue, nNodeIdx );
    }
    else
    {
        pHt = new SwHistoryResetFormat( pNewValue, nNodeIdx );
    }

    m_SwpHstry.push_back( pHt );
}

// FIXME: refactor the following "Add" methods (DRY)?
void SwHistory::Add( SwTextAttr* pHint, sal_uLong nNodeIdx, bool bNewAttr )
{
    OSL_ENSURE( !m_nEndDiff, "History was not deleted after REDO" );

    SwHistoryHint * pHt = 0;
    if( !bNewAttr )
    {
        switch ( pHint->Which() )
        {
            case RES_TXTATR_FTN:
                pHt = new SwHistorySetFootnote(
                            static_cast<SwTextFootnote*>(pHint), nNodeIdx );
                break;
            case RES_TXTATR_FLYCNT:
                pHt = new SwHistoryTextFlyCnt( static_cast<SwTextFlyCnt*>(pHint)
                            ->GetFlyCnt().GetFrameFormat() );
                break;
            case RES_TXTATR_FIELD:
            case RES_TXTATR_ANNOTATION:
                pHt = new SwHistorySetTextField(
                        static_txtattr_cast<SwTextField*>(pHint), nNodeIdx);
                break;
            case RES_TXTATR_TOXMARK:
                pHt = new SwHistorySetTOXMark(
                        static_txtattr_cast<SwTextTOXMark*>(pHint), nNodeIdx);
                break;
            case RES_TXTATR_REFMARK:
                pHt = new SwHistorySetRefMark(
                        static_txtattr_cast<SwTextRefMark*>(pHint), nNodeIdx);
                break;
            default:
                pHt = new SwHistorySetText(
                            static_cast<SwTextAttr*>(pHint), nNodeIdx );
        }
    }
    else
    {
        pHt = new SwHistoryResetText( pHint->Which(), pHint->GetStart(),
                                    *pHint->GetAnyEnd(), nNodeIdx );
    }
    m_SwpHstry.push_back( pHt );
}

void SwHistory::Add( SwFormatColl* pColl, sal_uLong nNodeIdx, sal_uInt8 nWhichNd )
{
    OSL_ENSURE( !m_nEndDiff, "History was not deleted after REDO" );

    SwHistoryHint * pHt =
        new SwHistoryChangeFormatColl( pColl, nNodeIdx, nWhichNd );
    m_SwpHstry.push_back( pHt );
}

void SwHistory::Add(const ::sw::mark::IMark& rBkmk, bool bSavePos, bool bSaveOtherPos)
{
    OSL_ENSURE( !m_nEndDiff, "History was not deleted after REDO" );

    SwHistoryHint * pHt = new SwHistoryBookmark(rBkmk, bSavePos, bSaveOtherPos);
    m_SwpHstry.push_back( pHt );
}

void SwHistory::Add( SwFrameFormat& rFormat )
{
    SwHistoryHint * pHt = new SwHistoryChangeFlyAnchor( rFormat );
    m_SwpHstry.push_back( pHt );
}

void SwHistory::Add( SwFlyFrameFormat& rFormat, sal_uInt16& rSetPos )
{
    OSL_ENSURE( !m_nEndDiff, "History was not deleted after REDO" );

    const sal_uInt16 nWh = rFormat.Which();
    if( RES_FLYFRMFMT == nWh || RES_DRAWFRMFMT == nWh )
    {
        SwHistoryHint * pHint = new SwHistoryTextFlyCnt( &rFormat );
        m_SwpHstry.push_back( pHint );

        const SwFormatChain* pChainItem;
        if( SfxItemState::SET == rFormat.GetItemState( RES_CHAIN, false,
            reinterpret_cast<const SfxPoolItem**>(&pChainItem) ))
        {
            if( pChainItem->GetNext() || pChainItem->GetPrev() )
            {
                SwHistoryHint * pHt =
                    new SwHistoryChangeFlyChain( rFormat, *pChainItem );
                m_SwpHstry.insert( m_SwpHstry.begin() + rSetPos++, pHt );
                if ( pChainItem->GetNext() )
                {
                    SwFormatChain aTmp( pChainItem->GetNext()->GetChain() );
                    aTmp.SetPrev( 0 );
                    pChainItem->GetNext()->SetFormatAttr( aTmp );
                }
                if ( pChainItem->GetPrev() )
                {
                    SwFormatChain aTmp( pChainItem->GetPrev()->GetChain() );
                    aTmp.SetNext( 0 );
                    pChainItem->GetPrev()->SetFormatAttr( aTmp );
                }
            }
            rFormat.ResetFormatAttr( RES_CHAIN );
        }
    }
}

void SwHistory::Add( const SwTextFootnote& rFootnote )
{
    SwHistoryHint *pHt = new SwHistorySetFootnote( rFootnote );
    m_SwpHstry.push_back( pHt );
}

// #i27615#
void SwHistory::Add(const SfxItemSet & rSet, const SwCharFormat & rFormat)
{
    SwHistoryHint * pHt = new SwHistoryChangeCharFormat(rSet, rFormat.GetName());
    m_SwpHstry.push_back( pHt );
}

bool SwHistory::Rollback( SwDoc* pDoc, sal_uInt16 nStart )
{
    if ( !Count() )
        return false;

    for ( sal_uInt16 i = Count(); i > nStart ; )
    {
        SwHistoryHint * pHHt = m_SwpHstry[ --i ];
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

    const sal_uInt16 nOld = Count() - m_nEndDiff;
    m_nEndDiff = Count() - nNewTmpEnd;

    // for every SwHistoryFlyCnt, call the Redo of its UndoObject.
    // this saves the formats of the flys!
    for ( sal_uInt16 n = nOld; n < nNewTmpEnd; n++ )
    {
        if ( HSTRY_FLYCNT == (*this)[ n ]->Which() )
        {
            static_cast<SwHistoryTextFlyCnt*>((*this)[ n ])
                ->GetUDelLFormat()->RedoForRollback();
        }
    }

    return nOld;
}

void SwHistory::CopyFormatAttr(
    const SfxItemSet& rSet,
    sal_uLong nNodeIdx)
{
    if(rSet.Count())
    {
        SfxItemIter aIter(rSet);

        do
        {
            if(!IsInvalidItem(aIter.GetCurItem()))
            {
                const SfxPoolItem* pNew = aIter.GetCurItem();

                Add(
                    pNew,
                    pNew,
                    nNodeIdx);
            }

            if(aIter.IsAtEnd())
            {
                break;
            }

            aIter.NextItem();

        } while(true);
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
    SwTextAttr* pHt;
    for( size_t n = 0; n < pHts->Count(); ++n )
    {
        // nAttrStt must even be set when !pEndIdx
        pHt = pHts->Get(n);
        const sal_Int32 nAttrStt = pHt->GetStart();
        const sal_Int32 * pEndIdx = pHt->GetEnd();
        if( 0 !=  pEndIdx && nAttrStt > nEnd )
            break;

        // never copy Flys and Footnote !!
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
    if ( m_pHistory && ( pOld || pNew ) && pOld != pNew )
    {
        if ( pNew->Which() < POOLATTR_END )
        {
            if(RES_UPDATE_ATTR == pNew->Which())
            {
                // const SfxItemPool& rPool = static_cast< const SwUpdateAttr* >(pNew)->GetSfxItemPool();

                m_pHistory->Add(
                    // rPool,
                    pOld,
                    pNew,
                    m_nNodeIndex);
            }
            else
            {
                OSL_ENSURE(false, "Unexpected update attribute (!)");
            }
        }
        else if (pOld && RES_ATTRSET_CHG == pNew->Which())
        {
            SwHistoryHint* pNewHstr(0);
            const SfxItemSet& rSet = *static_cast< const SwAttrSetChg* >(pOld)->GetChgSet();

            if ( 1 < rSet.Count() )
            {
                pNewHstr = new SwHistorySetAttrSet( rSet, m_nNodeIndex, m_WhichIdSet );
            }
            else if (const SfxPoolItem* pItem = SfxItemIter( rSet ).FirstItem())
            {
                if ( m_WhichIdSet.count( pItem->Which() ) )
                {
                    pNewHstr = new SwHistorySetFormat( pItem, m_nNodeIndex );
                }
                else
                {
                    pNewHstr = new SwHistoryResetFormat( pItem, m_nNodeIndex );
                }
            }

            if (pNewHstr)
                m_pHistory->m_SwpHstry.push_back( pNewHstr );
        }
    }
}

void SwRegHistory::AddHint( SwTextAttr* pHt, const bool bNew )
{
    m_pHistory->Add( pHt, m_nNodeIndex, bNew );
}

bool SwRegHistory::InsertItems( const SfxItemSet& rSet,
    sal_Int32 const nStart, sal_Int32 const nEnd, SetAttrMode const nFlags )
{
    if( !rSet.Count() )
        return false;

    SwTextNode * const pTextNode =
        dynamic_cast<SwTextNode *>(GetRegisteredIn());

    OSL_ENSURE(pTextNode, "SwRegHistory not registered at text node?");
    if (!pTextNode)
        return false;

    if ( pTextNode->GetpSwpHints() && m_pHistory )
    {
        pTextNode->GetpSwpHints()->Register( this );
    }

    const bool bInserted = pTextNode->SetAttr( rSet, nStart, nEnd, nFlags );

    // Caution: The array can be deleted when inserting an attribute!
    // This can happen when the value that should be added first deletes
    // an existing attribute but does not need to be added itself because
    // the paragraph attributes are identical
    // ( -> bForgetAttr in SwpHints::Insert )
    if ( pTextNode->GetpSwpHints() && m_pHistory )
    {
        pTextNode->GetpSwpHints()->DeRegister();
    }

    if ( m_pHistory && bInserted )
    {
        SwHistoryHint* pNewHstr = new SwHistoryResetAttrSet( rSet,
                                    pTextNode->GetIndex(), nStart, nEnd );
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
        if( dynamic_cast< const SwContentNode *>( GetRegisteredIn() ) != nullptr  )
        {
            pSet = static_cast<SwContentNode*>(
                    GetRegisteredIn())->GetpSwAttrSet();
        }
        else if ( dynamic_cast< const SwFormat *>( GetRegisteredIn() ) != nullptr  )
        {
            pSet = &static_cast<SwFormat*>(
                    GetRegisteredIn())->GetAttrSet();
        }
        if( pSet && pSet->Count() )
        {
            SfxItemIter aIter( *pSet );
            const SfxPoolItem* pItem = aIter.FirstItem();
            while(pItem)
            {
                sal_uInt16 nW = pItem->Which();
                m_WhichIdSet.insert( nW );
                if( aIter.IsAtEnd() )
                    break;
                pItem = aIter.NextItem();
            }
        }
    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
