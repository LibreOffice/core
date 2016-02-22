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

#include <UndoTable.hxx>
#include <UndoRedline.hxx>
#include <UndoDelete.hxx>
#include <UndoSplitMove.hxx>
#include <UndoCore.hxx>
#include <fesh.hxx>
#include <hintids.hxx>
#include <hints.hxx>
#include <editeng/formatbreakitem.hxx>
#include <fmtornt.hxx>
#include <fmtpdsc.hxx>
#include <doc.hxx>
#include <docredln.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentChartDataProviderAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <editsh.hxx>
#include <docary.hxx>
#include <ndtxt.hxx>
#include <swtable.hxx>
#include <pam.hxx>
#include <cntfrm.hxx>
#include <tblsel.hxx>
#include <swundo.hxx>
#include <rolbck.hxx>
#include <ddefld.hxx>
#include <tabcol.hxx>
#include <tabfrm.hxx>
#include <rowfrm.hxx>
#include <cellfrm.hxx>
#include <swcache.hxx>
#include <tblafmt.hxx>
#include <poolfmt.hxx>
#include <mvsave.hxx>
#include <cellatr.hxx>
#include <swtblfmt.hxx>
#include <swddetbl.hxx>
#include <redline.hxx>
#include <node2lay.hxx>
#include <tblrwcl.hxx>
#include <fmtanchr.hxx>
#include <comcore.hrc>
#include <unochart.hxx>
#include <calbck.hxx>

#include <memory>
#include <vector>

#ifdef DBG_UTIL
#define CHECK_TABLE(t) (t).CheckConsistency();
#else
#define CHECK_TABLE(t)
#endif

#ifdef DBG_UTIL
    #define _DEBUG_REDLINE( pDoc ) sw_DebugRedline( pDoc );
#else
    #define _DEBUG_REDLINE( pDoc )
#endif

typedef std::vector<std::shared_ptr<SfxItemSet> > SfxItemSets;

class SwUndoSaveSections : public std::vector<std::unique_ptr<SwUndoSaveSection>> {};

class SwUndoMoves : public std::vector<std::unique_ptr<SwUndoMove>> {};

struct SwTableToTextSave;
class SwTableToTextSaves : public std::vector<std::unique_ptr<SwTableToTextSave>> {};

struct _UndoTableCpyTable_Entry
{
    sal_uLong nBoxIdx, nOffset;
    SfxItemSet* pBoxNumAttr;
    SwUndo* pUndo;

    // Was the last paragraph of the new and the first paragraph of the old content joined?
    bool bJoin; // For redlining only

    explicit _UndoTableCpyTable_Entry( const SwTableBox& rBox );
    ~_UndoTableCpyTable_Entry();
};
class SwUndoTableCpyTable_Entries : public std::vector<std::unique_ptr<_UndoTableCpyTable_Entry>> {};

class _SaveBox;
class _SaveLine;

class _SaveTable
{
    friend class _SaveBox;
    friend class _SaveLine;
    SfxItemSet m_aTableSet;
    _SaveLine* m_pLine;
    const SwTable* m_pSwTable;
    SfxItemSets m_aSets;
    SwFrameFormats m_aFrameFormats;
    sal_uInt16 m_nLineCount;
    bool m_bModifyBox : 1;
    bool m_bSaveFormula : 1;
    bool m_bNewModel : 1;

public:
    _SaveTable( const SwTable& rTable, sal_uInt16 nLnCnt = USHRT_MAX,
                bool bSaveFormula = true );
    ~_SaveTable();

    sal_uInt16 AddFormat( SwFrameFormat* pFormat, bool bIsLine );
    void NewFrameFormat( const SwTableLine* , const SwTableBox*, sal_uInt16 nFormatPos,
                    SwFrameFormat* pOldFormat );

    void RestoreAttr( SwTable& rTable, bool bModifyBox = false );
    void SaveContentAttrs( SwDoc* pDoc );
    void CreateNew( SwTable& rTable, bool bCreateFrames = true,
                    bool bRestoreChart = true );
    bool IsNewModel() const { return m_bNewModel; }
};

class _SaveLine
{
    friend class _SaveTable;
    friend class _SaveBox;

    _SaveLine* pNext;
    _SaveBox* pBox;
    sal_uInt16 nItemSet;

public:
    _SaveLine( _SaveLine* pPrev, const SwTableLine& rLine, _SaveTable& rSTable );
    ~_SaveLine();

    void RestoreAttr( SwTableLine& rLine, _SaveTable& rSTable );
    void SaveContentAttrs( SwDoc* pDoc );

    void CreateNew( SwTable& rTable, SwTableBox& rParent, _SaveTable& rSTable  );
};

class _SaveBox
{
    friend class _SaveLine;

    _SaveBox* pNext;
    sal_uLong nSttNode;
    long nRowSpan;
    sal_uInt16 nItemSet;
    union
    {
        SfxItemSets* pContentAttrs;
        _SaveLine* pLine;
    } Ptrs;

public:
    _SaveBox( _SaveBox* pPrev, const SwTableBox& rBox, _SaveTable& rSTable );
    ~_SaveBox();

    void RestoreAttr( SwTableBox& rBox, _SaveTable& rSTable );
    void SaveContentAttrs( SwDoc* pDoc );

    void CreateNew( SwTable& rTable, SwTableLine& rParent, _SaveTable& rSTable );
};

#if OSL_DEBUG_LEVEL > 0
#include "shellio.hxx"
void CheckTable( const SwTable& );
#define CHECKTABLE(t) CheckTable( t );
#else
#define CHECKTABLE(t)
#endif

/* #130880: Crash in undo of table to text when the table has (freshly) merged cells
The order of cell content nodes in the nodes array is not given by the recursive table structure.
The algorithm must not rely on this even it holds for a fresh loaded table in odt file format.
So we need to remember not only the start node position but the end node position as well.
*/

struct SwTableToTextSave
{
    sal_uLong m_nSttNd;
    sal_uLong m_nEndNd;
    sal_Int32 m_nContent;
    SwHistory* m_pHstry;
    // metadata references for first and last paragraph in cell
    std::shared_ptr< ::sfx2::MetadatableUndo > m_pMetadataUndoStart;
    std::shared_ptr< ::sfx2::MetadatableUndo > m_pMetadataUndoEnd;

    SwTableToTextSave( SwDoc& rDoc, sal_uLong nNd, sal_uLong nEndIdx, sal_Int32 nContent );
    ~SwTableToTextSave() { delete m_pHstry; }
};

sal_uInt16 aSave_BoxContentSet[] = {
    RES_CHRATR_COLOR, RES_CHRATR_CROSSEDOUT,
    RES_CHRATR_FONT, RES_CHRATR_FONTSIZE,
    RES_CHRATR_POSTURE, RES_CHRATR_POSTURE,
    RES_CHRATR_SHADOWED, RES_CHRATR_WEIGHT,
    RES_PARATR_ADJUST, RES_PARATR_ADJUST,
    0 };

SwUndoInsTable::SwUndoInsTable( const SwPosition& rPos, sal_uInt16 nCl, sal_uInt16 nRw,
                            sal_uInt16 nAdj, const SwInsertTableOptions& rInsTableOpts,
                            const SwTableAutoFormat* pTAFormat,
                            const std::vector<sal_uInt16> *pColArr,
                            const OUString & rName)
    : SwUndo( UNDO_INSTABLE ),
    aInsTableOpts( rInsTableOpts ), pDDEFieldType( nullptr ), pColWidth( nullptr ), pRedlData( nullptr ), pAutoFormat( nullptr ),
    nSttNode( rPos.nNode.GetIndex() ), nRows( nRw ), nCols( nCl ), nAdjust( nAdj )
{
    if( pColArr )
    {
        pColWidth = new std::vector<sal_uInt16>(*pColArr);
    }
    if( pTAFormat )
        pAutoFormat = new SwTableAutoFormat( *pTAFormat );

    // consider redline
    SwDoc& rDoc = *rPos.nNode.GetNode().GetDoc();
    if( rDoc.getIDocumentRedlineAccess().IsRedlineOn() )
    {
        pRedlData = new SwRedlineData( nsRedlineType_t::REDLINE_INSERT, rDoc.getIDocumentRedlineAccess().GetRedlineAuthor() );
        SetRedlineMode( rDoc.getIDocumentRedlineAccess().GetRedlineMode() );
    }

    sTableNm = rName;
}

SwUndoInsTable::~SwUndoInsTable()
{
    delete pDDEFieldType;
    delete pColWidth;
    delete pRedlData;
    delete pAutoFormat;
}

void SwUndoInsTable::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    SwNodeIndex aIdx( rDoc.GetNodes(), nSttNode );

    SwTableNode* pTableNd = aIdx.GetNode().GetTableNode();
    OSL_ENSURE( pTableNd, "no TableNode" );
    pTableNd->DelFrames();

    if( IDocumentRedlineAccess::IsRedlineOn( GetRedlineMode() ))
        rDoc.getIDocumentRedlineAccess().DeleteRedline( *pTableNd, true, USHRT_MAX );
    RemoveIdxFromSection( rDoc, nSttNode );

    // move hard page breaks into next node
    SwContentNode* pNextNd = rDoc.GetNodes()[ pTableNd->EndOfSectionIndex()+1 ]->GetContentNode();
    if( pNextNd )
    {
        SwFrameFormat* pTableFormat = pTableNd->GetTable().GetFrameFormat();
        const SfxPoolItem *pItem;

        if( SfxItemState::SET == pTableFormat->GetItemState( RES_PAGEDESC,
            false, &pItem ) )
            pNextNd->SetAttr( *pItem );

        if( SfxItemState::SET == pTableFormat->GetItemState( RES_BREAK,
            false, &pItem ) )
            pNextNd->SetAttr( *pItem );
    }

    sTableNm = pTableNd->GetTable().GetFrameFormat()->GetName();
    if( dynamic_cast<const SwDDETable *>(&pTableNd->GetTable()) != nullptr )
        pDDEFieldType = static_cast<SwDDEFieldType*>(static_cast<SwDDETable&>(pTableNd->GetTable()).
                                        GetDDEFieldType()->Copy());

    rDoc.GetNodes().Delete( aIdx, pTableNd->EndOfSectionIndex() -
                                aIdx.GetIndex() + 1 );

    SwPaM & rPam( rContext.GetCursorSupplier().CreateNewShellCursor() );
    rPam.DeleteMark();
    rPam.GetPoint()->nNode = aIdx;
    rPam.GetPoint()->nContent.Assign( rPam.GetContentNode(), 0 );
}

void SwUndoInsTable::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();

    SwPosition const aPos(SwNodeIndex(rDoc.GetNodes(), nSttNode));
    const SwTable* pTable = rDoc.InsertTable( aInsTableOpts, aPos, nRows, nCols,
                                            nAdjust,
                                            pAutoFormat, pColWidth );
    static_cast<SwFrameFormat*>(pTable->GetFrameFormat())->SetName( sTableNm );
    SwTableNode* pTableNode = rDoc.GetNodes()[nSttNode]->GetTableNode();

    if( pDDEFieldType )
    {
        SwDDEFieldType* pNewType = static_cast<SwDDEFieldType*>(rDoc.getIDocumentFieldsAccess().InsertFieldType(
                                                            *pDDEFieldType));
        SwDDETable* pDDETable = new SwDDETable( pTableNode->GetTable(), pNewType );
        pTableNode->SetNewTable( pDDETable );
        delete pDDEFieldType;
        pDDEFieldType = nullptr;
    }

    if( (pRedlData && IDocumentRedlineAccess::IsRedlineOn( GetRedlineMode() )) ||
        ( !( nsRedlineMode_t::REDLINE_IGNORE & GetRedlineMode() ) &&
            !rDoc.getIDocumentRedlineAccess().GetRedlineTable().empty() ))
    {
        SwPaM aPam( *pTableNode->EndOfSectionNode(), *pTableNode, 1 );
        SwContentNode* pCNd = aPam.GetContentNode( false );
        if( pCNd )
            aPam.GetMark()->nContent.Assign( pCNd, 0 );

        if( pRedlData && IDocumentRedlineAccess::IsRedlineOn( GetRedlineMode() ) )
        {
            RedlineMode_t eOld = rDoc.getIDocumentRedlineAccess().GetRedlineMode();
            rDoc.getIDocumentRedlineAccess().SetRedlineMode_intern((RedlineMode_t)(eOld & ~nsRedlineMode_t::REDLINE_IGNORE));

            rDoc.getIDocumentRedlineAccess().AppendRedline( new SwRangeRedline( *pRedlData, aPam ), true);
            rDoc.getIDocumentRedlineAccess().SetRedlineMode_intern( eOld );
        }
        else
            rDoc.getIDocumentRedlineAccess().SplitRedline( aPam );
    }
}

void SwUndoInsTable::RepeatImpl(::sw::RepeatContext & rContext)
{
    rContext.GetDoc().InsertTable(
            aInsTableOpts, *rContext.GetRepeatPaM().GetPoint(),
            nRows, nCols, nAdjust, pAutoFormat, pColWidth );
}

SwRewriter SwUndoInsTable::GetRewriter() const
{
    SwRewriter aRewriter;

    aRewriter.AddRule(UndoArg1, SW_RES(STR_START_QUOTE));
    aRewriter.AddRule(UndoArg2, sTableNm);
    aRewriter.AddRule(UndoArg3, SW_RES(STR_END_QUOTE));

    return aRewriter;
}

SwTableToTextSave::SwTableToTextSave( SwDoc& rDoc, sal_uLong nNd, sal_uLong nEndIdx, sal_Int32 nCnt )
    : m_nSttNd( nNd ), m_nEndNd( nEndIdx), m_nContent( nCnt ), m_pHstry( nullptr )
{
    // keep attributes of the joined node
    SwTextNode* pNd = rDoc.GetNodes()[ nNd ]->GetTextNode();
    if( pNd )
    {
        m_pHstry = new SwHistory;

        m_pHstry->Add( pNd->GetTextColl(), nNd, ND_TEXTNODE );
        if ( pNd->GetpSwpHints() )
        {
            m_pHstry->CopyAttr( pNd->GetpSwpHints(), nNd, 0,
                        pNd->GetText().getLength(), false );
        }
        if( pNd->HasSwAttrSet() )
            m_pHstry->CopyFormatAttr( *pNd->GetpSwAttrSet(), nNd );

        if( !m_pHstry->Count() )
        {
            delete m_pHstry;
            m_pHstry = nullptr;
        }

        // METADATA: store
        m_pMetadataUndoStart = pNd->CreateUndo();
    }

    // we also need to store the metadata reference of the _last_ paragraph
    // we subtract 1 to account for the removed cell start/end node pair
    // (after SectionUp, the end of the range points to the node after the cell)
    if ( nEndIdx - 1 > nNd )
    {
        SwTextNode* pLastNode( rDoc.GetNodes()[ nEndIdx - 1 ]->GetTextNode() );
        if( pLastNode )
        {
            // METADATA: store
            m_pMetadataUndoEnd = pLastNode->CreateUndo();
        }
    }
}

SwUndoTableToText::SwUndoTableToText( const SwTable& rTable, sal_Unicode cCh )
    : SwUndo( UNDO_TABLETOTEXT ),
    sTableNm( rTable.GetFrameFormat()->GetName() ), pDDEFieldType( nullptr ), pHistory( nullptr ),
    nSttNd( 0 ), nEndNd( 0 ),
    cTrenner( cCh ), nHdlnRpt( rTable.GetRowsToRepeat() )
{
    pTableSave = new _SaveTable( rTable );
    m_pBoxSaves = new SwTableToTextSaves;
    m_pBoxSaves->reserve(rTable.GetTabSortBoxes().size());

    if( dynamic_cast<const SwDDETable *>(&rTable) != nullptr )
        pDDEFieldType = static_cast<SwDDEFieldType*>(static_cast<const SwDDETable&>(rTable).GetDDEFieldType()->Copy());

    bCheckNumFormat = rTable.GetFrameFormat()->GetDoc()->IsInsTableFormatNum();

    pHistory = new SwHistory;
    const SwTableNode* pTableNd = rTable.GetTableNode();
    sal_uLong nTableStt = pTableNd->GetIndex(), nTableEnd = pTableNd->EndOfSectionIndex();

    const SwFrameFormats& rFrameFormatTable = *pTableNd->GetDoc()->GetSpzFrameFormats();
    for( size_t n = 0; n < rFrameFormatTable.size(); ++n )
    {
        SwFrameFormat* pFormat = rFrameFormatTable[ n ];
        SwFormatAnchor const*const pAnchor = &pFormat->GetAnchor();
        SwPosition const*const pAPos = pAnchor->GetContentAnchor();
        if (pAPos &&
            ((FLY_AT_CHAR == pAnchor->GetAnchorId()) ||
             (FLY_AT_PARA == pAnchor->GetAnchorId())) &&
            nTableStt <= pAPos->nNode.GetIndex() &&
            pAPos->nNode.GetIndex() < nTableEnd )
        {
            pHistory->Add( *pFormat );
        }
    }

    if( !pHistory->Count() )
    {
        delete pHistory;
        pHistory = nullptr;
    }
}

SwUndoTableToText::~SwUndoTableToText()
{
    delete pDDEFieldType;
    delete pTableSave;
    delete m_pBoxSaves;
    delete pHistory;
}

void SwUndoTableToText::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    SwPaM *const pPam(& rContext.GetCursorSupplier().CreateNewShellCursor());

    SwNodeIndex aFrameIdx( rDoc.GetNodes(), nSttNd );
    SwNodeIndex aEndIdx( rDoc.GetNodes(), nEndNd );

    pPam->GetPoint()->nNode = aFrameIdx;
    pPam->SetMark();
    pPam->GetPoint()->nNode = aEndIdx;
    rDoc.DelNumRules( *pPam );
    pPam->DeleteMark();

    // now collect all Uppers
    SwNode2Layout aNode2Layout( aFrameIdx.GetNode() );

    // create TableNode structure
    SwTableNode* pTableNd = rDoc.GetNodes().UndoTableToText( nSttNd, nEndNd, *m_pBoxSaves );
    pTableNd->GetTable().SetTableModel( pTableSave->IsNewModel() );
    SwTableFormat* pTableFormat = rDoc.MakeTableFrameFormat( sTableNm, rDoc.GetDfltFrameFormat() );
    pTableNd->GetTable().RegisterToFormat( *pTableFormat );
    pTableNd->GetTable().SetRowsToRepeat( nHdlnRpt );

    // create old table structure
    pTableSave->CreateNew( pTableNd->GetTable() );

    if( pDDEFieldType )
    {
        SwDDEFieldType* pNewType = static_cast<SwDDEFieldType*>(rDoc.getIDocumentFieldsAccess().InsertFieldType(
                                                            *pDDEFieldType));
        SwDDETable* pDDETable = new SwDDETable( pTableNd->GetTable(), pNewType );
        pTableNd->SetNewTable( pDDETable, false );
        delete pDDEFieldType;
        pDDEFieldType = nullptr;
    }

    if( bCheckNumFormat )
    {
        SwTableSortBoxes& rBxs = pTableNd->GetTable().GetTabSortBoxes();
        for (size_t nBoxes = rBxs.size(); nBoxes; )
        {
            rDoc.ChkBoxNumFormat( *rBxs[ --nBoxes ], false );
        }
    }

    if( pHistory )
    {
        sal_uInt16 nTmpEnd = pHistory->GetTmpEnd();
        pHistory->TmpRollback( &rDoc, 0 );
        pHistory->SetTmpEnd( nTmpEnd );
    }

    aNode2Layout.RestoreUpperFrames( rDoc.GetNodes(),
                                   pTableNd->GetIndex(), pTableNd->GetIndex()+1 );

    // Is a table selection requested?
    pPam->DeleteMark();
    pPam->GetPoint()->nNode = *pTableNd->EndOfSectionNode();
    pPam->SetMark();
    pPam->GetPoint()->nNode = *pPam->GetNode().StartOfSectionNode();
    pPam->Move( fnMoveForward, fnGoContent );
    pPam->Exchange();
    pPam->Move( fnMoveBackward, fnGoContent );

    ClearFEShellTabCols();
}

// located in untbl.cxx and only an Undo object is allowed to call it
SwTableNode* SwNodes::UndoTableToText( sal_uLong nSttNd, sal_uLong nEndNd,
                                const SwTableToTextSaves& rSavedData )
{
    SwNodeIndex aSttIdx( *this, nSttNd );
    SwNodeIndex aEndIdx( *this, nEndNd+1 );

    SwTableNode * pTableNd = new SwTableNode( aSttIdx );
    SwEndNode* pEndNd = new SwEndNode( aEndIdx, *pTableNd  );

    aEndIdx = *pEndNd;

    /* Set pTableNd as start of section for all nodes in [nSttNd, nEndNd].
       Delete all Frames attached to the nodes in that range. */
    SwNode* pNd;
    {
        sal_uLong n, nTmpEnd = aEndIdx.GetIndex();
        for( n = pTableNd->GetIndex() + 1; n < nTmpEnd; ++n )
        {
            if( ( pNd = (*this)[ n ] )->IsContentNode() )
                static_cast<SwContentNode*>(pNd)->DelFrames();
            pNd->m_pStartOfSection = pTableNd;
        }
    }

    // than create table structure partially. First a single line that contains
    // all boxes. The correct structure is than taken from SaveStruct.
    SwTableBoxFormat* pBoxFormat = GetDoc()->MakeTableBoxFormat();
    SwTableLineFormat* pLineFormat = GetDoc()->MakeTableLineFormat();
    SwTableLine* pLine = new SwTableLine( pLineFormat, rSavedData.size(), nullptr );
    pTableNd->GetTable().GetTabLines().insert( pTableNd->GetTable().GetTabLines().begin(), pLine );

    const std::shared_ptr<sw::mark::ContentIdxStore> pContentStore(sw::mark::ContentIdxStore::Create());
    for( size_t n = rSavedData.size(); n; )
    {
        const SwTableToTextSave *const pSave = rSavedData[ --n ].get();
        // if the start node was merged with last from prev. cell,
        // subtract 1 from index to get the merged paragraph, and split that
        aSttIdx = pSave->m_nSttNd - ( ( SAL_MAX_INT32 != pSave->m_nContent ) ? 1 : 0);
        SwTextNode* pTextNd = aSttIdx.GetNode().GetTextNode();

        if( SAL_MAX_INT32 != pSave->m_nContent )
        {
            // split at ContentPosition, delete previous char (= separator)
            OSL_ENSURE( pTextNd, "Where is my TextNode?" );
            SwIndex aCntPos( pTextNd, pSave->m_nContent - 1 );

            pTextNd->EraseText( aCntPos, 1 );
            SwContentNode* pNewNd = pTextNd->SplitContentNode(
                                        SwPosition( aSttIdx, aCntPos ));
            if( !pContentStore->Empty() )
                pContentStore->Restore( *pNewNd, pSave->m_nContent, pSave->m_nContent + 1 );
        }
        else
        {
            pContentStore->Clear();
            if( pTextNd )
                pContentStore->Save( GetDoc(), aSttIdx.GetIndex(), pTextNd->GetText().getLength() );
        }

        if( pTextNd )
        {
            // METADATA: restore
            pTextNd->GetTextNode()->RestoreMetadata(pSave->m_pMetadataUndoStart);
            if( pTextNd->HasSwAttrSet() )
                pTextNd->ResetAllAttr();

            if( pTextNd->GetpSwpHints() )
                pTextNd->ClearSwpHintsArr( false );
        }

        if( pSave->m_pHstry )
        {
            sal_uInt16 nTmpEnd = pSave->m_pHstry->GetTmpEnd();
            pSave->m_pHstry->TmpRollback( GetDoc(), 0 );
            pSave->m_pHstry->SetTmpEnd( nTmpEnd );
        }

        // METADATA: restore
        // end points to node after cell
        if ( pSave->m_nEndNd - 1 > pSave->m_nSttNd )
        {
            SwTextNode* pLastNode = (*this)[ pSave->m_nEndNd - 1 ]->GetTextNode();
            if (pLastNode)
            {
                pLastNode->RestoreMetadata(pSave->m_pMetadataUndoEnd);
            }
        }

        aEndIdx = pSave->m_nEndNd;
        SwStartNode* pSttNd = new SwStartNode( aSttIdx, ND_STARTNODE,
                                                SwTableBoxStartNode );
        pSttNd->m_pStartOfSection = pTableNd;
        new SwEndNode( aEndIdx, *pSttNd );

        for( sal_uLong i = aSttIdx.GetIndex(); i < aEndIdx.GetIndex()-1; ++i )
        {
            pNd = (*this)[ i ];
            pNd->m_pStartOfSection = pSttNd;
            if( pNd->IsStartNode() )
                i = pNd->EndOfSectionIndex();
        }

        SwTableBox* pBox = new SwTableBox( pBoxFormat, *pSttNd, pLine );
        pLine->GetTabBoxes().insert( pLine->GetTabBoxes().begin(), pBox );
    }
    return pTableNd;
}

void SwUndoTableToText::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    SwPaM *const pPam(& rContext.GetCursorSupplier().CreateNewShellCursor());

    pPam->GetPoint()->nNode = nSttNd;
    pPam->GetPoint()->nContent.Assign( nullptr, 0 );
    SwNodeIndex aSaveIdx( pPam->GetPoint()->nNode, -1 );

    pPam->SetMark();            // log off all indices
    pPam->DeleteMark();

    SwTableNode* pTableNd = pPam->GetNode().GetTableNode();
    OSL_ENSURE( pTableNd, "Could not find any TableNode" );

    if( dynamic_cast<const SwDDETable *>(&pTableNd->GetTable()) != nullptr )
        pDDEFieldType = static_cast<SwDDEFieldType*>(static_cast<SwDDETable&>(pTableNd->GetTable()).
                                                GetDDEFieldType()->Copy());

    rDoc.TableToText( pTableNd, cTrenner );

    ++aSaveIdx;
    SwContentNode* pCNd = aSaveIdx.GetNode().GetContentNode();
    if( !pCNd && nullptr == ( pCNd = rDoc.GetNodes().GoNext( &aSaveIdx ) ) &&
        nullptr == ( pCNd = SwNodes::GoPrevious( &aSaveIdx )) )
    {
        OSL_FAIL( "Where is the TextNode now?" );
    }

    pPam->GetPoint()->nNode = aSaveIdx;
    pPam->GetPoint()->nContent.Assign( pCNd, 0 );

    pPam->SetMark();            // log off all indices
    pPam->DeleteMark();
}

void SwUndoTableToText::RepeatImpl(::sw::RepeatContext & rContext)
{
    SwPaM *const pPam = & rContext.GetRepeatPaM();
    SwTableNode *const pTableNd = pPam->GetNode().FindTableNode();
    if( pTableNd )
    {
        // move cursor out of table
        pPam->GetPoint()->nNode = *pTableNd->EndOfSectionNode();
        pPam->Move( fnMoveForward, fnGoContent );
        pPam->SetMark();
        pPam->DeleteMark();

        rContext.GetDoc().TableToText( pTableNd, cTrenner );
    }
}

void SwUndoTableToText::SetRange( const SwNodeRange& rRg )
{
    nSttNd = rRg.aStart.GetIndex();
    nEndNd = rRg.aEnd.GetIndex();
}

void SwUndoTableToText::AddBoxPos( SwDoc& rDoc, sal_uLong nNdIdx, sal_uLong nEndIdx, sal_Int32 nContentIdx )
{
    std::unique_ptr<SwTableToTextSave> pNew(
            new SwTableToTextSave(rDoc, nNdIdx, nEndIdx, nContentIdx));
    m_pBoxSaves->push_back(std::move(pNew));
}

SwUndoTextToTable::SwUndoTextToTable( const SwPaM& rRg,
                                const SwInsertTableOptions& rInsTableOpts,
                                sal_Unicode cCh, sal_uInt16 nAdj,
                                const SwTableAutoFormat* pAFormat )
    : SwUndo( UNDO_TEXTTOTABLE ), SwUndRng( rRg ), aInsTableOpts( rInsTableOpts ),
      pDelBoxes( nullptr ), pAutoFormat( nullptr ),
      pHistory( nullptr ), cTrenner( cCh ), nAdjust( nAdj )
{
    if( pAFormat )
        pAutoFormat = new SwTableAutoFormat( *pAFormat );

    const SwPosition* pEnd = rRg.End();
    SwNodes& rNds = rRg.GetDoc()->GetNodes();
    bSplitEnd = pEnd->nContent.GetIndex() && ( pEnd->nContent.GetIndex()
                        != pEnd->nNode.GetNode().GetContentNode()->Len() ||
                pEnd->nNode.GetIndex() >= rNds.GetEndOfContent().GetIndex()-1 );
}

SwUndoTextToTable::~SwUndoTextToTable()
{
    delete pDelBoxes;
    delete pAutoFormat;
}

void SwUndoTextToTable::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();

    sal_uLong nTableNd = nSttNode;
    if( nSttContent )
        ++nTableNd;       // Node was splitted previously
    SwNodeIndex aIdx( rDoc.GetNodes(), nTableNd );
    SwTableNode *const pTNd = aIdx.GetNode().GetTableNode();
    OSL_ENSURE( pTNd, "Could not find a TableNode" );

    RemoveIdxFromSection( rDoc, nTableNd );

    sTableNm = pTNd->GetTable().GetFrameFormat()->GetName();

    if( pHistory )
    {
        pHistory->TmpRollback( &rDoc, 0 );
        pHistory->SetTmpEnd( pHistory->Count() );
    }

    if( pDelBoxes )
    {
        pTNd->DelFrames();
        SwTable& rTable = pTNd->GetTable();
        for( size_t n = pDelBoxes->size(); n; )
        {
            SwTableBox* pBox = rTable.GetTableBox( (*pDelBoxes)[ --n ] );
            if( pBox )
                ::_DeleteBox( rTable, pBox, nullptr, false, false );
            else {
                OSL_ENSURE( false, "Where is my box?" );
            }
        }
    }

    SwNodeIndex aEndIdx( *pTNd->EndOfSectionNode() );
    rDoc.TableToText( pTNd, 0x0b == cTrenner ? 0x09 : cTrenner );

    // join again at start?
    SwPaM aPam(rDoc.GetNodes().GetEndOfContent());
    SwPosition *const pPos = aPam.GetPoint();
    if( nSttContent )
    {
        pPos->nNode = nTableNd;
        pPos->nContent.Assign(pPos->nNode.GetNode().GetContentNode(), 0);
        if (aPam.Move(fnMoveBackward, fnGoContent))
        {
            SwNodeIndex & rIdx = aPam.GetPoint()->nNode;

            // than move, relatively, the Cursor/etc. again
            RemoveIdxRel( rIdx.GetIndex()+1, *pPos );

            rIdx.GetNode().GetContentNode()->JoinNext();
        }
    }

    // join again at end?
    if( bSplitEnd )
    {
        SwNodeIndex& rIdx = pPos->nNode;
        rIdx = nEndNode;
        SwTextNode* pTextNd = rIdx.GetNode().GetTextNode();
        if( pTextNd && pTextNd->CanJoinNext() )
        {
            aPam.GetMark()->nContent.Assign( nullptr, 0 );
            aPam.GetPoint()->nContent.Assign( nullptr, 0 );

            // than move, relatively, the Cursor/etc. again
            pPos->nContent.Assign(pTextNd, pTextNd->GetText().getLength());
            RemoveIdxRel( nEndNode + 1, *pPos );

            pTextNd->JoinNext();
        }
    }

    AddUndoRedoPaM(rContext);
}

void SwUndoTextToTable::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwPaM & rPam( AddUndoRedoPaM(rContext) );
    RemoveIdxFromRange(rPam, false);
    SetPaM(rPam);

    SwTable const*const pTable = rContext.GetDoc().TextToTable(
                aInsTableOpts, rPam, cTrenner, nAdjust, pAutoFormat );
    static_cast<SwFrameFormat*>(pTable->GetFrameFormat())->SetName( sTableNm );
}

void SwUndoTextToTable::RepeatImpl(::sw::RepeatContext & rContext)
{
    // no Table In Table
    if (!rContext.GetRepeatPaM().GetNode().FindTableNode())
    {
        rContext.GetDoc().TextToTable( aInsTableOpts, rContext.GetRepeatPaM(),
                                        cTrenner, nAdjust,
                                        pAutoFormat );
    }
}

void SwUndoTextToTable::AddFillBox( const SwTableBox& rBox )
{
    if( !pDelBoxes )
        pDelBoxes = new std::vector<sal_uLong>;
    pDelBoxes->push_back( rBox.GetSttIdx() );
}

SwHistory& SwUndoTextToTable::GetHistory()
{
    if( !pHistory )
        pHistory = new SwHistory;
    return *pHistory;
}

SwUndoTableHeadline::SwUndoTableHeadline( const SwTable& rTable, sal_uInt16 nOldHdl,
                                      sal_uInt16 nNewHdl )
    : SwUndo( UNDO_TABLEHEADLINE ),
    nOldHeadline( nOldHdl ),
    nNewHeadline( nNewHdl )
{
    OSL_ENSURE( !rTable.GetTabSortBoxes().empty(), "Table without content" );
    const SwStartNode *pSttNd = rTable.GetTabSortBoxes()[ 0 ]->GetSttNd();
    OSL_ENSURE( pSttNd, "Box without content" );

    nTableNd = pSttNd->StartOfSectionIndex();
}

void SwUndoTableHeadline::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    SwTableNode* pTNd = rDoc.GetNodes()[ nTableNd ]->GetTableNode();
    OSL_ENSURE( pTNd, "could not find any TableNode" );

    rDoc.SetRowsToRepeat( pTNd->GetTable(), nOldHeadline );
}

void SwUndoTableHeadline::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();

    SwTableNode* pTNd = rDoc.GetNodes()[ nTableNd ]->GetTableNode();
    OSL_ENSURE( pTNd, "could not find any TableNode" );

    rDoc.SetRowsToRepeat( pTNd->GetTable(), nNewHeadline );
}

void SwUndoTableHeadline::RepeatImpl(::sw::RepeatContext & rContext)
{
    SwTableNode *const pTableNd =
        rContext.GetRepeatPaM().GetNode().FindTableNode();
    if( pTableNd )
    {
        rContext.GetDoc().SetRowsToRepeat( pTableNd->GetTable(), nNewHeadline );
    }
}

_SaveTable::_SaveTable( const SwTable& rTable, sal_uInt16 nLnCnt, bool bSaveFormula )
    : m_aTableSet(*rTable.GetFrameFormat()->GetAttrSet().GetPool(), aTableSetRange),
      m_pSwTable(&rTable), m_nLineCount(nLnCnt), m_bSaveFormula(bSaveFormula)
{
    m_bModifyBox = false;
    m_bNewModel = rTable.IsNewModel();
    m_aTableSet.Put(rTable.GetFrameFormat()->GetAttrSet());
    m_pLine = new _SaveLine( nullptr, *rTable.GetTabLines()[ 0 ], *this );

    _SaveLine* pLn = m_pLine;
    if( USHRT_MAX == nLnCnt )
        nLnCnt = rTable.GetTabLines().size();
    for( sal_uInt16 n = 1; n < nLnCnt; ++n )
        pLn = new _SaveLine( pLn, *rTable.GetTabLines()[ n ], *this );

    m_aFrameFormats.clear();
    m_pSwTable = nullptr;
}

_SaveTable::~_SaveTable()
{
    delete m_pLine;
}

sal_uInt16 _SaveTable::AddFormat( SwFrameFormat* pFormat, bool bIsLine )
{
    size_t nRet = m_aFrameFormats.GetPos(pFormat);
    if( SIZE_MAX == nRet )
    {
        // Create copy of ItemSet
        std::shared_ptr<SfxItemSet> pSet( new SfxItemSet( *pFormat->GetAttrSet().GetPool(),
            bIsLine ? aTableLineSetRange : aTableBoxSetRange ) );
        pSet->Put( pFormat->GetAttrSet() );
        // When a formula is set, never save the value. It possibly must be
        // recalculated.
        // Save formulas always in plain text.
        const SfxPoolItem* pItem;
        if( SfxItemState::SET == pSet->GetItemState( RES_BOXATR_FORMULA, true, &pItem ))
        {
            pSet->ClearItem( RES_BOXATR_VALUE );
            if (m_pSwTable && m_bSaveFormula)
            {
                SwTableFormulaUpdate aMsgHint(m_pSwTable);
                aMsgHint.m_eFlags = TBL_BOXNAME;
                SwTableBoxFormula* pFormulaItem = const_cast<SwTableBoxFormula*>(static_cast<const SwTableBoxFormula*>(pItem));
                pFormulaItem->ChgDefinedIn( pFormat );
                pFormulaItem->ChangeState( &aMsgHint );
                pFormulaItem->ChgDefinedIn( nullptr );
            }
        }
        nRet = m_aSets.size();
        m_aSets.push_back(pSet);
        m_aFrameFormats.insert(m_aFrameFormats.begin() + nRet, pFormat);
    }
    return static_cast<sal_uInt16>(nRet);
}

void _SaveTable::RestoreAttr( SwTable& rTable, bool bMdfyBox )
{
    m_bModifyBox = bMdfyBox;

    // first, get back attributes of TableFrameFormat
    SwFrameFormat* pFormat = rTable.GetFrameFormat();
    SfxItemSet& rFormatSet  = (SfxItemSet&)pFormat->GetAttrSet();
    rFormatSet.ClearItem();
    rFormatSet.Put(m_aTableSet);

    if( pFormat->IsInCache() )
    {
        SwFrame::GetCache().Delete( pFormat );
        pFormat->SetInCache( false );
    }

    // for safety, invalidate all TableFrames
    SwIterator<SwTabFrame,SwFormat> aIter( *pFormat );
    for( SwTabFrame* pLast = aIter.First(); pLast; pLast = aIter.Next() )
        if( pLast->GetTable() == &rTable )
        {
            pLast->InvalidateAll();
            pLast->SetCompletePaint();
        }

    // fill FrameFormats with defaults (0)
    pFormat = nullptr;
    for (size_t n = m_aSets.size(); n; --n)
        m_aFrameFormats.push_back(pFormat);

    const size_t nLnCnt = (USHRT_MAX == m_nLineCount)
        ? rTable.GetTabLines().size()
        : m_nLineCount;

    _SaveLine* pLn = m_pLine;
    for( size_t n = 0; n < nLnCnt; ++n, pLn = pLn->pNext )
    {
        if( !pLn )
        {
            OSL_ENSURE( false, "Number of lines changed" );
            break;
        }

        pLn->RestoreAttr( *rTable.GetTabLines()[ n ], *this );
    }

    m_aFrameFormats.clear();
    m_bModifyBox = false;
}

void _SaveTable::SaveContentAttrs( SwDoc* pDoc )
{
    m_pLine->SaveContentAttrs(pDoc);
}

void _SaveTable::CreateNew( SwTable& rTable, bool bCreateFrames,
                            bool bRestoreChart )
{
    _FndBox aTmpBox( nullptr, nullptr );
    aTmpBox.DelFrames( rTable );

    // first, get back attributes of TableFrameFormat
    SwFrameFormat* pFormat = rTable.GetFrameFormat();
    SfxItemSet& rFormatSet  = (SfxItemSet&)pFormat->GetAttrSet();
    rFormatSet.ClearItem();
    rFormatSet.Put(m_aTableSet);

    if( pFormat->IsInCache() )
    {
        SwFrame::GetCache().Delete( pFormat );
        pFormat->SetInCache( false );
    }

    // SwTableBox must have a format - the SwTableBox takes ownership of it
    SwTableBoxFormat *const pNewFormat(pFormat->GetDoc()->MakeTableBoxFormat());
    SwTableBox aParent(pNewFormat, rTable.GetTabLines().size(), nullptr);

    // fill FrameFormats with defaults (0)
    pFormat = nullptr;
    for( size_t n = m_aSets.size(); n; --n )
        m_aFrameFormats.push_back(pFormat);

    m_pLine->CreateNew(rTable, aParent, *this);
    m_aFrameFormats.clear();

    // add new lines, delete old ones
    const size_t nOldLines = (USHRT_MAX == m_nLineCount)
        ? rTable.GetTabLines().size()
        : m_nLineCount;

    SwDoc *pDoc = rTable.GetFrameFormat()->GetDoc();
    SwChartDataProvider *pPCD = pDoc->getIDocumentChartDataProviderAccess().GetChartDataProvider();
    size_t n = 0;
    for( ; n < aParent.GetTabLines().size(); ++n )
    {
        SwTableLine* pLn = aParent.GetTabLines()[ n ];
        pLn->SetUpper( nullptr );
        if( n < nOldLines )
        {
            SwTableLine* pOld = rTable.GetTabLines()[ n ];

            // TL_CHART2: notify chart about boxes to be removed
            const SwTableBoxes &rBoxes = pOld->GetTabBoxes();
            const size_t nBoxes = rBoxes.size();
            for (size_t k = 0; k < nBoxes;  ++k)
            {
                SwTableBox *pBox = rBoxes[k];
                if (pPCD)
                    pPCD->DeleteBox( &rTable, *pBox );
            }

            rTable.GetTabLines()[n] = pLn;
            delete pOld;
        }
        else
            rTable.GetTabLines().insert( rTable.GetTabLines().begin() + n, pLn );
    }

    if( n < nOldLines )
    {
        // remove remaining lines...
        for (size_t k1 = 0; k1 < nOldLines - n; ++k1)
        {
            const SwTableBoxes &rBoxes = rTable.GetTabLines()[n + k1]->GetTabBoxes();
            const size_t nBoxes = rBoxes.size();
            for (size_t k2 = 0; k2 < nBoxes; ++k2)
            {
                SwTableBox *pBox = rBoxes[k2];
                // TL_CHART2: notify chart about boxes to be removed
                if (pPCD)
                    pPCD->DeleteBox( &rTable, *pBox );
            }
        }

#ifdef _MSC_VER
// MSVC 2012 appears very confused and rambles about a declaration of "n" below
#pragma warning (push, 1)
#pragma warning (disable: 4258)
#endif
        for( SwTableLines::const_iterator it = rTable.GetTabLines().begin() + n;
             it != rTable.GetTabLines().begin() + nOldLines; ++it )
            delete *it;
        rTable.GetTabLines().erase( rTable.GetTabLines().begin() + n, rTable.GetTabLines().begin() + nOldLines );
#ifdef _MSC_VER
#pragma warning (pop)
#endif
    }

    aParent.GetTabLines().erase( aParent.GetTabLines().begin(), aParent.GetTabLines().begin() + n );
    assert(aParent.GetTabLines().empty());

    if( bCreateFrames )
        aTmpBox.MakeFrames( rTable );
    if( bRestoreChart )
    {
        // TL_CHART2: need to inform chart of probably changed cell names
        pDoc->UpdateCharts( rTable.GetFrameFormat()->GetName() );
    }
}

void _SaveTable::NewFrameFormat( const SwTableLine* pTableLn, const SwTableBox* pTableBx,
                            sal_uInt16 nFormatPos, SwFrameFormat* pOldFormat )
{
    SwDoc* pDoc = pOldFormat->GetDoc();

    SwFrameFormat* pFormat = m_aFrameFormats[ nFormatPos ];
    if( !pFormat )
    {
        if( pTableLn )
            pFormat = pDoc->MakeTableLineFormat();
        else
            pFormat = pDoc->MakeTableBoxFormat();
        pFormat->SetFormatAttr(*m_aSets[nFormatPos]);
        m_aFrameFormats[nFormatPos] = pFormat;
    }

    // first re-assign Frames
    SwIterator<SwLayoutFrame,SwFormat> aIter( *pOldFormat );
    for( SwFrame* pLast = aIter.First(); pLast; pLast = aIter.Next() )
    {
        if( pTableLn ? static_cast<SwRowFrame*>(pLast)->GetTabLine() == pTableLn
                    : static_cast<SwCellFrame*>(pLast)->GetTabBox() == pTableBx )
        {
            pLast->RegisterToFormat(*pFormat);
            pLast->InvalidateAll();
            pLast->ReinitializeFrameSizeAttrFlags();
            if ( !pTableLn )
            {
                static_cast<SwCellFrame*>(pLast)->SetDerivedVert( false );
                static_cast<SwCellFrame*>(pLast)->CheckDirChange();
            }
        }
    }

    // than re-assign myself
    if ( pTableLn )
        const_cast<SwTableLine*>(pTableLn)->RegisterToFormat( *pFormat );
    else if ( pTableBx )
        const_cast<SwTableBox*>(pTableBx)->RegisterToFormat( *pFormat );

    if (m_bModifyBox && !pTableLn)
    {
        const SfxPoolItem& rOld = pOldFormat->GetFormatAttr( RES_BOXATR_FORMAT ),
                         & rNew = pFormat->GetFormatAttr( RES_BOXATR_FORMAT );
        if( rOld != rNew )
            pFormat->ModifyNotification( &rOld, &rNew );
    }

    if( !pOldFormat->HasWriterListeners() )
        delete pOldFormat;
}

_SaveLine::_SaveLine( _SaveLine* pPrev, const SwTableLine& rLine, _SaveTable& rSTable )
    : pNext( nullptr )
{
    if( pPrev )
        pPrev->pNext = this;

    nItemSet = rSTable.AddFormat( rLine.GetFrameFormat(), true );

    pBox = new _SaveBox( nullptr, *rLine.GetTabBoxes()[ 0 ], rSTable );
    _SaveBox* pBx = pBox;
    for( size_t n = 1; n < rLine.GetTabBoxes().size(); ++n )
        pBx = new _SaveBox( pBx, *rLine.GetTabBoxes()[ n ], rSTable );
}

_SaveLine::~_SaveLine()
{
    delete pBox;
    delete pNext;
}

void _SaveLine::RestoreAttr( SwTableLine& rLine, _SaveTable& rSTable )
{
    rSTable.NewFrameFormat( &rLine, nullptr, nItemSet, rLine.GetFrameFormat() );

    _SaveBox* pBx = pBox;
    for( size_t n = 0; n < rLine.GetTabBoxes().size(); ++n, pBx = pBx->pNext )
    {
        if( !pBx )
        {
            OSL_ENSURE( false, "Number of boxes changed" );
            break;
        }
        pBx->RestoreAttr( *rLine.GetTabBoxes()[ n ], rSTable );
    }
}

void _SaveLine::SaveContentAttrs( SwDoc* pDoc )
{
    pBox->SaveContentAttrs( pDoc );
    if( pNext )
        pNext->SaveContentAttrs( pDoc );
}

void _SaveLine::CreateNew( SwTable& rTable, SwTableBox& rParent, _SaveTable& rSTable )
{
    SwTableLineFormat* pFormat = static_cast<SwTableLineFormat*>(rSTable.m_aFrameFormats[ nItemSet ]);
    if( !pFormat )
    {
        SwDoc* pDoc = rTable.GetFrameFormat()->GetDoc();
        pFormat = pDoc->MakeTableLineFormat();
        pFormat->SetFormatAttr(*rSTable.m_aSets[nItemSet]);
        rSTable.m_aFrameFormats[nItemSet] = pFormat;
    }
    SwTableLine* pNew = new SwTableLine( pFormat, 1, &rParent );

    rParent.GetTabLines().push_back( pNew );

    pBox->CreateNew( rTable, *pNew, rSTable );

    if( pNext )
        pNext->CreateNew( rTable, rParent, rSTable );
}

_SaveBox::_SaveBox( _SaveBox* pPrev, const SwTableBox& rBox, _SaveTable& rSTable )
    : pNext( nullptr ), nSttNode( ULONG_MAX ), nRowSpan(0)
{
    Ptrs.pLine = nullptr;

    if( pPrev )
        pPrev->pNext = this;

    nItemSet = rSTable.AddFormat( rBox.GetFrameFormat(), false );

    if( rBox.GetSttNd() )
    {
        nSttNode = rBox.GetSttIdx();
        nRowSpan = rBox.getRowSpan();
    }
    else
    {
        Ptrs.pLine = new _SaveLine( nullptr, *rBox.GetTabLines()[ 0 ], rSTable );

        _SaveLine* pLn = Ptrs.pLine;
        for( size_t n = 1; n < rBox.GetTabLines().size(); ++n )
            pLn = new _SaveLine( pLn, *rBox.GetTabLines()[ n ], rSTable );
    }
}

_SaveBox::~_SaveBox()
{
    if( ULONG_MAX == nSttNode )     // no EndBox
        delete Ptrs.pLine;
    else
        delete Ptrs.pContentAttrs;
    delete pNext;
}

void _SaveBox::RestoreAttr( SwTableBox& rBox, _SaveTable& rSTable )
{
    rSTable.NewFrameFormat( nullptr, &rBox, nItemSet, rBox.GetFrameFormat() );

    if( ULONG_MAX == nSttNode )     // no EndBox
    {
        if( rBox.GetTabLines().empty() )
        {
            OSL_ENSURE( false, "Number of lines changed" );
        }
        else
        {
            _SaveLine* pLn = Ptrs.pLine;
            for( size_t n = 0; n < rBox.GetTabLines().size(); ++n, pLn = pLn->pNext )
            {
                if( !pLn )
                {
                    OSL_ENSURE( false, "Number of lines changed" );
                    break;
                }

                pLn->RestoreAttr( *rBox.GetTabLines()[ n ], rSTable );
            }
        }
    }
    else if( rBox.GetSttNd() && rBox.GetSttIdx() == nSttNode )
    {
        if( Ptrs.pContentAttrs )
        {
            SwNodes& rNds = rBox.GetFrameFormat()->GetDoc()->GetNodes();
            sal_uInt16 nSet = 0;
            sal_uLong nEnd = rBox.GetSttNd()->EndOfSectionIndex();
            for( sal_uLong n = nSttNode + 1; n < nEnd; ++n )
            {
                SwContentNode* pCNd = rNds[ n ]->GetContentNode();
                if( pCNd )
                {
                    std::shared_ptr<SfxItemSet> pSet( (*Ptrs.pContentAttrs)[ nSet++ ] );
                    if( pSet )
                    {
                        sal_uInt16 *pRstAttr = aSave_BoxContentSet;
                        while( *pRstAttr )
                        {
                            pCNd->ResetAttr( *pRstAttr, *(pRstAttr+1) );
                            pRstAttr += 2;
                        }
                        pCNd->SetAttr( *pSet );
                    }
                    else
                        pCNd->ResetAllAttr();
                }
            }
        }
    }
    else
    {
        OSL_ENSURE( false, "Box not anymore at the same node" );
    }
}

void _SaveBox::SaveContentAttrs( SwDoc* pDoc )
{
    if( ULONG_MAX == nSttNode )     // no EndBox
    {
        // continue in current line
        Ptrs.pLine->SaveContentAttrs( pDoc );
    }
    else
    {
        sal_uLong nEnd = pDoc->GetNodes()[ nSttNode ]->EndOfSectionIndex();
        Ptrs.pContentAttrs = new SfxItemSets( (sal_uInt8)(nEnd - nSttNode - 1 ) );
        for( sal_uLong n = nSttNode + 1; n < nEnd; ++n )
        {
            SwContentNode* pCNd = pDoc->GetNodes()[ n ]->GetContentNode();
            if( pCNd )
            {
                std::shared_ptr<SfxItemSet> pSet;
                if( pCNd->HasSwAttrSet() )
                {
                    pSet.reset( new SfxItemSet( pDoc->GetAttrPool(),
                                            aSave_BoxContentSet ) );
                    pSet->Put( *pCNd->GetpSwAttrSet() );
                }

                Ptrs.pContentAttrs->push_back( pSet );
            }
        }
    }
    if( pNext )
        pNext->SaveContentAttrs( pDoc );
}

void _SaveBox::CreateNew( SwTable& rTable, SwTableLine& rParent, _SaveTable& rSTable )
{
    SwTableBoxFormat* pFormat = static_cast<SwTableBoxFormat*>(rSTable.m_aFrameFormats[ nItemSet ]);
    if( !pFormat )
    {
        SwDoc* pDoc = rTable.GetFrameFormat()->GetDoc();
        pFormat = pDoc->MakeTableBoxFormat();
        pFormat->SetFormatAttr(*rSTable.m_aSets[nItemSet]);
        rSTable.m_aFrameFormats[nItemSet] = pFormat;
    }

    if( ULONG_MAX == nSttNode )     // no EndBox
    {
        SwTableBox* pNew = new SwTableBox( pFormat, 1, &rParent );
        rParent.GetTabBoxes().push_back( pNew );

        Ptrs.pLine->CreateNew( rTable, *pNew, rSTable );
    }
    else
    {
        // search box for StartNode in old table
        SwTableBox* pBox = rTable.GetTableBox( nSttNode );
        if (pBox)
        {
            SwFrameFormat* pOld = pBox->GetFrameFormat();
            pBox->RegisterToFormat( *pFormat );
            if( !pOld->HasWriterListeners() )
                delete pOld;

            pBox->setRowSpan( nRowSpan );

            SwTableBoxes* pTBoxes = &pBox->GetUpper()->GetTabBoxes();
            pTBoxes->erase( std::find( pTBoxes->begin(), pTBoxes->end(), pBox ) );

            pBox->SetUpper( &rParent );
            pTBoxes = &rParent.GetTabBoxes();
            pTBoxes->push_back( pBox );
        }
    }

    if( pNext )
        pNext->CreateNew( rTable, rParent, rSTable );
}

// UndoObject for attribute changes on table
SwUndoAttrTable::SwUndoAttrTable( const SwTableNode& rTableNd, bool bClearTabCols )
    : SwUndo( UNDO_TABLE_ATTR ),
    nSttNode( rTableNd.GetIndex() )
{
    bClearTabCol = bClearTabCols;
    pSaveTable = new _SaveTable( rTableNd.GetTable() );
}

SwUndoAttrTable::~SwUndoAttrTable()
{
    delete pSaveTable;
}

void SwUndoAttrTable::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    SwTableNode* pTableNd = rDoc.GetNodes()[ nSttNode ]->GetTableNode();
    OSL_ENSURE( pTableNd, "no TableNode" );

    if (pTableNd)
    {
        _SaveTable* pOrig = new _SaveTable( pTableNd->GetTable() );
        pSaveTable->RestoreAttr( pTableNd->GetTable() );
        delete pSaveTable;
        pSaveTable = pOrig;
    }

    if( bClearTabCol )
        ClearFEShellTabCols();
}

void SwUndoAttrTable::RedoImpl(::sw::UndoRedoContext & rContext)
{
    UndoImpl(rContext);
}

// UndoObject for AutoFormat on Table
SwUndoTableAutoFormat::SwUndoTableAutoFormat( const SwTableNode& rTableNd,
                                    const SwTableAutoFormat& rAFormat )
    : SwUndo( UNDO_TABLE_AUTOFMT )
    , m_TableStyleName(rTableNd.GetTable().GetTableStyleName())
    , nSttNode( rTableNd.GetIndex() )
    , bSaveContentAttr( false )
    , m_nRepeatHeading(rTableNd.GetTable().GetRowsToRepeat())
{
    pSaveTable = new _SaveTable( rTableNd.GetTable() );

    if( rAFormat.IsFont() || rAFormat.IsJustify() )
    {
        // than also go over the ContentNodes of the EndBoxes and collect
        // all paragraph attributes
        pSaveTable->SaveContentAttrs( const_cast<SwDoc*>(rTableNd.GetDoc()) );
        bSaveContentAttr = true;
    }
}

SwUndoTableAutoFormat::~SwUndoTableAutoFormat()
{
    delete pSaveTable;
}

void SwUndoTableAutoFormat::SaveBoxContent( const SwTableBox& rBox )
{
    std::shared_ptr<SwUndoTableNumFormat> const p(new SwUndoTableNumFormat(rBox));
    m_Undos.push_back(p);
}

void
SwUndoTableAutoFormat::UndoRedo(bool const bUndo, ::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    SwTableNode* pTableNd = rDoc.GetNodes()[ nSttNode ]->GetTableNode();
    OSL_ENSURE( pTableNd, "no TableNode" );

    SwTable& table = pTableNd->GetTable();
    if (table.GetTableStyleName() != m_TableStyleName)
    {
        OUString const temp(table.GetTableStyleName());
        table.SetTableStyleName(m_TableStyleName);
        m_TableStyleName = temp;
    }
    _SaveTable* pOrig = new _SaveTable( table );
    // than go also over the ContentNodes of the EndBoxes and collect
    // all paragraph attributes
    if( bSaveContentAttr )
        pOrig->SaveContentAttrs( &rDoc );

    if (bUndo)
    {
        for (size_t n = m_Undos.size(); 0 < n; --n)
        {
            m_Undos.at(n-1)->UndoImpl(rContext);
        }

        table.SetRowsToRepeat(m_nRepeatHeading);
    }

    pSaveTable->RestoreAttr( pTableNd->GetTable(), !bUndo );
    delete pSaveTable;
    pSaveTable = pOrig;
}

void SwUndoTableAutoFormat::UndoImpl(::sw::UndoRedoContext & rContext)
{
    UndoRedo(true, rContext);
}

void SwUndoTableAutoFormat::RedoImpl(::sw::UndoRedoContext & rContext)
{
    UndoRedo(false, rContext);
}

SwUndoTableNdsChg::SwUndoTableNdsChg( SwUndoId nAction,
                                    const SwSelBoxes& rBoxes,
                                    const SwTableNode& rTableNd,
                                    long nMn, long nMx,
                                    sal_uInt16 nCnt, bool bFlg, bool bSmHght )
    : SwUndo( nAction ),
    nMin( nMn ), nMax( nMx ),
    nSttNode( rTableNd.GetIndex() ), nCurrBox( 0 ),
    nCount( nCnt ), nRelDiff( 0 ), nAbsDiff( 0 ),
    nSetColType( USHRT_MAX ),
    bFlag( bFlg ),
    bSameHeight( bSmHght )
{
    const SwTable& rTable = rTableNd.GetTable();
    pSaveTable = new _SaveTable( rTable );

    // and remember selection
    ReNewBoxes( rBoxes );
}

SwUndoTableNdsChg::SwUndoTableNdsChg( SwUndoId nAction,
                                    const SwSelBoxes& rBoxes,
                                    const SwTableNode& rTableNd )
    : SwUndo( nAction ),
    nMin( 0 ), nMax( 0 ),
    nSttNode( rTableNd.GetIndex() ), nCurrBox( 0 ),
    nCount( 0 ), nRelDiff( 0 ), nAbsDiff( 0 ),
    nSetColType( USHRT_MAX ),
    bFlag( false ),
    bSameHeight( false )
{
    const SwTable& rTable = rTableNd.GetTable();
    pSaveTable = new _SaveTable( rTable );

    // and remember selection
    ReNewBoxes( rBoxes );
}

void SwUndoTableNdsChg::ReNewBoxes( const SwSelBoxes& rBoxes )
{
    if (rBoxes.size() != m_Boxes.size())
    {
        m_Boxes.clear();
        for (size_t n = 0; n < rBoxes.size(); ++n)
        {
            m_Boxes.insert( rBoxes[n]->GetSttIdx() );
        }
    }
}

SwUndoTableNdsChg::~SwUndoTableNdsChg()
{
    delete pSaveTable;
}

void SwUndoTableNdsChg::SaveNewBoxes( const SwTableNode& rTableNd,
                                    const SwTableSortBoxes& rOld )
{
    const SwTable& rTable = rTableNd.GetTable();
    const SwTableSortBoxes& rTableBoxes = rTable.GetTabSortBoxes();

    OSL_ENSURE( ! IsDelBox(), "wrong Action" );
    pNewSttNds.reset( new std::set<_BoxMove> );

    size_t i = 0;
    for (size_t  n = 0; n < rOld.size(); ++i)
    {
        if( rOld[ n ] == rTableBoxes[ i ] )
            ++n;
        else
            // new box: insert sorted
            pNewSttNds->insert( _BoxMove(rTableBoxes[ i ]->GetSttIdx()) );
    }

    for( ; i < rTableBoxes.size(); ++i )
        // new box: insert sorted
        pNewSttNds->insert( _BoxMove(rTableBoxes[ i ]->GetSttIdx()) );
}

static SwTableLine* lcl_FindTableLine( const SwTable& rTable,
                                const SwTableBox& rBox )
{
    SwTableLine* pRet = nullptr;
    // i63949: For nested cells we have to take nLineNo - 1, too, not 0!
    const SwTableLines &rTableLines = ( rBox.GetUpper()->GetUpper() != nullptr ) ?
                                  rBox.GetUpper()->GetUpper()->GetTabLines()
                                : rTable.GetTabLines();
    const SwTableLine* pLine = rBox.GetUpper();
    sal_uInt16 nLineNo = rTableLines.GetPos( pLine );
    pRet = rTableLines[nLineNo - 1];

    return pRet;
}

static const SwTableLines& lcl_FindParentLines( const SwTable& rTable,
                                       const SwTableBox& rBox )
{
    const SwTableLines& rRet =
        ( rBox.GetUpper()->GetUpper() != nullptr ) ?
            rBox.GetUpper()->GetUpper()->GetTabLines() :
            rTable.GetTabLines();

    return rRet;
}

void SwUndoTableNdsChg::SaveNewBoxes( const SwTableNode& rTableNd,
                                    const SwTableSortBoxes& rOld,
                                    const SwSelBoxes& rBoxes,
                                    const std::vector<sal_uLong> &rNodeCnts )
{
    const SwTable& rTable = rTableNd.GetTable();
    const SwTableSortBoxes& rTableBoxes = rTable.GetTabSortBoxes();

    OSL_ENSURE( ! IsDelBox(), "wrong Action" );
    pNewSttNds.reset( new std::set<_BoxMove> );

    OSL_ENSURE( rTable.IsNewModel() || rOld.size() + nCount * rBoxes.size() == rTableBoxes.size(),
        "unexpected boxes" );
    OSL_ENSURE( rOld.size() <= rTableBoxes.size(), "more unexpected boxes" );
    for (size_t n = 0, i = 0; i < rTableBoxes.size(); ++i)
    {
        if( ( n < rOld.size() ) &&
            ( rOld[ n ] == rTableBoxes[ i ] ) )
        {
            // box already known? Then nothing to be done.
            ++n;
        }
        else
        {
            // new box found: insert (obey sort order)
            const SwTableBox* pBox = rTableBoxes[ i ];

            // find the source box. It must be one in rBoxes.
            // We found the right one if it's in the same column as pBox.
            // No, if more than one selected cell in the same column has been splitted,
            // we have to look for the nearest one (i65201)!
            const SwTableBox* pSourceBox = nullptr;
            const SwTableBox* pCheckBox = nullptr;
            const SwTableLine* pBoxLine = pBox->GetUpper();
            sal_uInt16 nLineDiff = lcl_FindParentLines(rTable,*pBox).GetPos(pBoxLine);
            sal_uInt16 nLineNo = 0;
            for (size_t j = 0; j < rBoxes.size(); ++j)
            {
                pCheckBox = rBoxes[j];
                if( pCheckBox->GetUpper()->GetUpper() == pBox->GetUpper()->GetUpper() )
                {
                    const SwTableLine* pCheckLine = pCheckBox->GetUpper();
                    sal_uInt16 nCheckLine = lcl_FindParentLines( rTable, *pCheckBox ).
                    GetPos( pCheckLine );
                    if( ( !pSourceBox || nCheckLine > nLineNo ) && nCheckLine < nLineDiff )
                    {
                        nLineNo = nCheckLine;
                        pSourceBox = pCheckBox;
                    }
                }
            }

            // find the line number difference
            // (to help determine bNodesMoved flag below)
            nLineDiff = nLineDiff - nLineNo;
            OSL_ENSURE( pSourceBox, "Splitted source box not found!" );
            // find out how many nodes the source box used to have
            // (to help determine bNodesMoved flag below)
            size_t nNdsPos = 0;
            while( rBoxes[ nNdsPos ] != pSourceBox )
                ++nNdsPos;
            sal_uLong nNodes = rNodeCnts[ nNdsPos ];

            // When a new table cell is created, it either gets a new
            // node, or it gets node(s) from elsewhere. The undo must
            // know, of course, and thus we must determine here just
            // where pBox's nodes are from:
            // If 1) the source box has lost nodes, and
            //    2) we're in the node range that got nodes
            // then pBox received nodes from elsewhere.
            // If bNodesMoved is set for pBox the undo must move the
            // boxes back, otherwise it must delete them.
            bool bNodesMoved = pSourceBox &&
                ( nNodes != ( pSourceBox->GetSttNd()->EndOfSectionIndex() -
                              pSourceBox->GetSttIdx() ) )
                && ( nNodes - 1 > nLineDiff );
            pNewSttNds->insert( _BoxMove(pBox->GetSttIdx(), bNodesMoved) );
        }
    }
}

void SwUndoTableNdsChg::SaveSection( SwStartNode* pSttNd )
{
    OSL_ENSURE( IsDelBox(), "wrong Action" );
    if (m_pDelSects.get() == nullptr)
        m_pDelSects.reset(new SwUndoSaveSections);

    SwTableNode* pTableNd = pSttNd->FindTableNode();
    std::unique_ptr<SwUndoSaveSection> pSave(new SwUndoSaveSection);
    pSave->SaveSection( SwNodeIndex( *pSttNd ));

    m_pDelSects->push_back(std::move(pSave));
    nSttNode = pTableNd->GetIndex();
}

void SwUndoTableNdsChg::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    SwNodeIndex aIdx( rDoc.GetNodes(), nSttNode );

    SwTableNode *const pTableNd = aIdx.GetNode().GetTableNode();
    OSL_ENSURE( pTableNd, "no TableNode" );

    SwTableFormulaUpdate aMsgHint( &pTableNd->GetTable() );
    aMsgHint.m_eFlags = TBL_BOXPTR;
    rDoc.getIDocumentFieldsAccess().UpdateTableFields( &aMsgHint );

    CHECK_TABLE( pTableNd->GetTable() )

    _FndBox aTmpBox( nullptr, nullptr );
    // ? TL_CHART2: notification or locking of controller required ?

    SwChartDataProvider *pPCD = rDoc.getIDocumentChartDataProviderAccess().GetChartDataProvider();
    SwSelBoxes aDelBoxes;
    std::vector< std::pair<SwTableBox *, sal_uLong> > aDelNodes;
    if( IsDelBox() )
    {
        // Trick: add missing boxes in any line, they will be connected
        // correctly when calling CreateNew
        SwTableBox* pCpyBox = pTableNd->GetTable().GetTabSortBoxes()[0];
        SwTableBoxes& rLnBoxes = pCpyBox->GetUpper()->GetTabBoxes();

        // restore sections
        for (size_t n = m_pDelSects->size(); n; )
        {
            SwUndoSaveSection *const pSave = (*m_pDelSects)[ --n ].get();
            pSave->RestoreSection( &rDoc, &aIdx, SwTableBoxStartNode );
            if( pSave->GetHistory() )
                pSave->GetHistory()->Rollback( &rDoc );
            SwTableBox* pBox = new SwTableBox( static_cast<SwTableBoxFormat*>(pCpyBox->GetFrameFormat()), aIdx,
                                                pCpyBox->GetUpper() );
            rLnBoxes.push_back( pBox );
        }
        m_pDelSects->clear();
    }
    else if( !pNewSttNds->empty() )
    {
        // Then the nodes have be moved and not deleted!
        // But for that we need a temp array.
        std::vector<_BoxMove> aTmp( pNewSttNds->begin(), pNewSttNds->end() );

        // backwards
        for (size_t n = aTmp.size(); n > 0 ; )
        {
            --n;
            // delete box from table structure
            sal_uLong nIdx = aTmp[n].index;
            SwTableBox* pBox = pTableNd->GetTable().GetTableBox( nIdx );
            OSL_ENSURE( pBox, "Where is my TableBox?" );

            // TL_CHART2: notify chart about box to be removed
            if (pPCD)
                pPCD->DeleteBox( &pTableNd->GetTable(), *pBox );

            // insert _before_ deleting the section - otherwise the box
            // has no start node so all boxes sort equal in SwSelBoxes
            aDelBoxes.insert(pBox);

            if( aTmp[n].hasMoved )
            {
                SwNodeRange aRg( *pBox->GetSttNd(), 1,
                            *pBox->GetSttNd()->EndOfSectionNode() );

                SwTableLine* pLine = lcl_FindTableLine( pTableNd->GetTable(), *pBox );
                SwNodeIndex aInsPos( *(pLine->GetTabBoxes()[0]->GetSttNd()), 2 );

                // adjust all StartNode indices
                size_t i = n;
                sal_uLong nSttIdx = aInsPos.GetIndex() - 2,
                       nNdCnt = aRg.aEnd.GetIndex() - aRg.aStart.GetIndex();
                while( i && aTmp[ --i ].index > nSttIdx )
                    aTmp[ i ].index += nNdCnt;

                // first delete box
                delete pBox;
                // than move nodes
                rDoc.GetNodes()._MoveNodes( aRg, rDoc.GetNodes(), aInsPos, false );
            }
            else
            {
                aDelNodes.push_back(std::make_pair(pBox, nIdx));
            }
        }
    }
    else
    {
        // Remove nodes from nodes array (backwards!)
        std::set<_BoxMove>::reverse_iterator it;
        for( it = pNewSttNds->rbegin(); it != pNewSttNds->rend(); ++it )
        {
            sal_uLong nIdx = (*it).index;
            SwTableBox* pBox = pTableNd->GetTable().GetTableBox( nIdx );
            OSL_ENSURE( pBox, "Where's my table box?" );
            // TL_CHART2: notify chart about box to be removed
            if (pPCD)
                pPCD->DeleteBox( &pTableNd->GetTable(), *pBox );
            aDelBoxes.insert(pBox);
            aDelNodes.push_back(std::make_pair(pBox, nIdx));
        }
    }

    // fdo#57197: before deleting the SwTableBoxes, delete the SwTabFrames
    aTmpBox.SetTableLines(aDelBoxes, pTableNd->GetTable());
    aTmpBox.DelFrames(pTableNd->GetTable());

    // do this _after_ deleting Frames because disposing SwAccessible requires
    // connection to the nodes, see SwAccessibleChild::IsAccessible()
    for (size_t i = 0; i < aDelNodes.size(); ++i)
    {
        // first disconnect box from node, otherwise ~SwTableBox would
        // access pBox->pSttNd, deleted by DeleteSection
        aDelNodes[i].first->RemoveFromTable();
        rDoc.getIDocumentContentOperations().DeleteSection(rDoc.GetNodes()[ aDelNodes[i].second ]);
    }

    // Remove boxes from table structure
    for( size_t n = 0; n < aDelBoxes.size(); ++n )
    {
        SwTableBox* pCurrBox = aDelBoxes[n];
        SwTableBoxes* pTBoxes = &pCurrBox->GetUpper()->GetTabBoxes();
        pTBoxes->erase( std::find( pTBoxes->begin(), pTBoxes->end(), pCurrBox ) );
        delete pCurrBox;
    }

    pSaveTable->CreateNew( pTableNd->GetTable(), true, false );

    // TL_CHART2: need to inform chart of probably changed cell names
    rDoc.UpdateCharts( pTableNd->GetTable().GetFrameFormat()->GetName() );

    if( IsDelBox() )
        nSttNode = pTableNd->GetIndex();
    ClearFEShellTabCols();
    CHECK_TABLE( pTableNd->GetTable() )
}

void SwUndoTableNdsChg::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();

    SwTableNode* pTableNd = rDoc.GetNodes()[ nSttNode ]->GetTableNode();
    OSL_ENSURE( pTableNd, "no TableNode" );
    CHECK_TABLE( pTableNd->GetTable() )

    SwSelBoxes aSelBoxes;
    for (std::set<sal_uLong>::iterator it = m_Boxes.begin();
            it != m_Boxes.end(); ++it)
    {
        SwTableBox* pBox = pTableNd->GetTable().GetTableBox( *it );
        aSelBoxes.insert( pBox );
    }

    // create SelBoxes and call InsertCell/-Row/SplitTable
    switch( GetId() )
    {
    case UNDO_TABLE_INSCOL:
        if( USHRT_MAX == nSetColType )
            rDoc.InsertCol( aSelBoxes, nCount, bFlag );
        else
        {
            SwTableBox* pBox = pTableNd->GetTable().GetTableBox( nCurrBox );
            rDoc.SetColRowWidthHeight( *pBox, nSetColType, nAbsDiff,
                                        nRelDiff );
        }
        break;

    case UNDO_TABLE_INSROW:
        if( USHRT_MAX == nSetColType )
            rDoc.InsertRow( aSelBoxes, nCount, bFlag );
        else
        {
            SwTable& rTable = pTableNd->GetTable();
            SwTableBox* pBox = rTable.GetTableBox( nCurrBox );
            TableChgMode eOldMode = rTable.GetTableChgMode();
            rTable.SetTableChgMode( (TableChgMode)nCount );
            rDoc.SetColRowWidthHeight( *pBox, nSetColType, nAbsDiff, nRelDiff );
            rTable.SetTableChgMode( eOldMode );
        }
        break;

    case UNDO_TABLE_SPLIT:
        rDoc.SplitTable( aSelBoxes, bFlag, nCount, bSameHeight );
        break;
    case UNDO_TABLE_DELBOX:
    case UNDO_ROW_DELETE:
    case UNDO_COL_DELETE:
        if( USHRT_MAX == nSetColType )
        {
            SwTableFormulaUpdate aMsgHint( &pTableNd->GetTable() );
            aMsgHint.m_eFlags = TBL_BOXPTR;
            rDoc.getIDocumentFieldsAccess().UpdateTableFields( &aMsgHint );
            SwTable &rTable = pTableNd->GetTable();
            if( nMax > nMin && rTable.IsNewModel() )
                rTable.PrepareDeleteCol( nMin, nMax );
            rTable.DeleteSel( &rDoc, aSelBoxes, nullptr, this, true, true );
        }
        else
        {
            SwTable& rTable = pTableNd->GetTable();

            SwTableFormulaUpdate aMsgHint( &rTable );
            aMsgHint.m_eFlags = TBL_BOXPTR;
            rDoc.getIDocumentFieldsAccess().UpdateTableFields( &aMsgHint );

            SwTableBox* pBox = rTable.GetTableBox( nCurrBox );
            TableChgMode eOldMode = rTable.GetTableChgMode();
            rTable.SetTableChgMode( (TableChgMode)nCount );

            // need the SaveSections!
            rDoc.GetIDocumentUndoRedo().DoUndo( true );
            SwUndo* pUndo = nullptr;

            switch( nSetColType & 0xff )
            {
            case nsTableChgWidthHeightType::WH_COL_LEFT:
            case nsTableChgWidthHeightType::WH_COL_RIGHT:
            case nsTableChgWidthHeightType::WH_CELL_LEFT:
            case nsTableChgWidthHeightType::WH_CELL_RIGHT:
                 rTable.SetColWidth( *pBox, nSetColType, nAbsDiff,
                                    nRelDiff, &pUndo );
                break;
            case nsTableChgWidthHeightType::WH_ROW_TOP:
            case nsTableChgWidthHeightType::WH_ROW_BOTTOM:
            case nsTableChgWidthHeightType::WH_CELL_TOP:
            case nsTableChgWidthHeightType::WH_CELL_BOTTOM:
                rTable.SetRowHeight( *pBox, nSetColType, nAbsDiff,
                                    nRelDiff, &pUndo );
                break;
            }

            if( pUndo )
            {
                m_pDelSects->insert(m_pDelSects->begin(),
                    std::make_move_iterator(
                        static_cast<SwUndoTableNdsChg *>(pUndo)->m_pDelSects->begin()),
                    std::make_move_iterator(
                        static_cast<SwUndoTableNdsChg *>(pUndo)->m_pDelSects->end()));
                static_cast<SwUndoTableNdsChg *>(pUndo)->m_pDelSects->clear();
                delete pUndo;
            }
            rDoc.GetIDocumentUndoRedo().DoUndo( false );

            rTable.SetTableChgMode( eOldMode );
        }
        nSttNode = pTableNd->GetIndex();
        break;
    default:
        ;
    }
    ClearFEShellTabCols();
    CHECK_TABLE( pTableNd->GetTable() )
}

SwUndoTableMerge::SwUndoTableMerge( const SwPaM& rTableSel )
    : SwUndo( UNDO_TABLE_MERGE ), SwUndRng( rTableSel )
    , m_pMoves(new SwUndoMoves)
    , pHistory(nullptr)
{
    const SwTableNode* pTableNd = rTableSel.GetNode().FindTableNode();
    OSL_ENSURE( pTableNd, "Where is the TableNode?" );
    pSaveTable = new _SaveTable( pTableNd->GetTable() );
    nTableNode = pTableNd->GetIndex();
}

SwUndoTableMerge::~SwUndoTableMerge()
{
    delete pSaveTable;
    delete m_pMoves;
    delete pHistory;
}

void SwUndoTableMerge::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    SwNodeIndex aIdx( rDoc.GetNodes(), nTableNode );

    SwTableNode *const pTableNd = aIdx.GetNode().GetTableNode();
    OSL_ENSURE( pTableNd, "no TableNode" );

    SwTableFormulaUpdate aMsgHint( &pTableNd->GetTable() );
    aMsgHint.m_eFlags = TBL_BOXPTR;
    rDoc.getIDocumentFieldsAccess().UpdateTableFields( &aMsgHint );

    _FndBox aTmpBox( nullptr, nullptr );
    // ? TL_CHART2: notification or locking of controller required ?

    // 1. restore deleted boxes:
    // Trick: add missing boxes in any line, they will be connected
    // correctly when calling CreateNew
    SwTableBox *pBox, *pCpyBox = pTableNd->GetTable().GetTabSortBoxes()[0];
    SwTableBoxes& rLnBoxes = pCpyBox->GetUpper()->GetTabBoxes();

CHECKTABLE(pTableNd->GetTable())

    SwSelBoxes aSelBoxes;
    SwTextFormatColl* pColl = rDoc.getIDocumentStylePoolAccess().GetTextCollFromPool( RES_POOLCOLL_STANDARD );

    std::set<sal_uLong>::iterator it;
    for (it = m_Boxes.begin(); it != m_Boxes.end(); ++it)
    {
        aIdx = *it;
        SwStartNode* pSttNd = rDoc.GetNodes().MakeTextSection( aIdx,
                                            SwTableBoxStartNode, pColl );
        pBox = new SwTableBox( static_cast<SwTableBoxFormat*>(pCpyBox->GetFrameFormat()), *pSttNd,
                                pCpyBox->GetUpper() );
        rLnBoxes.push_back( pBox );

        aSelBoxes.insert( pBox );
    }

CHECKTABLE(pTableNd->GetTable())

    SwChartDataProvider *pPCD = rDoc.getIDocumentChartDataProviderAccess().GetChartDataProvider();
    // 2. deleted the inserted boxes
    // delete nodes (from last to first)
    for( size_t n = aNewSttNds.size(); n; )
    {
        // remove box from table structure
        sal_uLong nIdx = aNewSttNds[ --n ];

        if( !nIdx && n )
        {
            nIdx = aNewSttNds[ --n ];
            pBox = pTableNd->GetTable().GetTableBox( nIdx );
            OSL_ENSURE( pBox, "Where is my TableBox?" );

            if( !pSaveTable->IsNewModel() )
                rDoc.GetNodes().MakeTextNode( SwNodeIndex(
                    *pBox->GetSttNd()->EndOfSectionNode() ), pColl );

            // this was the separator -> restore moved ones
            for (size_t i = m_pMoves->size(); i; )
            {
                SwTextNode* pTextNd = nullptr;
                sal_Int32 nDelPos = 0;
                SwUndoMove *const pUndo = (*m_pMoves)[ --i ].get();
                if( !pUndo->IsMoveRange() )
                {
                    pTextNd = rDoc.GetNodes()[ pUndo->GetDestSttNode() ]->GetTextNode();
                    nDelPos = pUndo->GetDestSttContent() - 1;
                }
                pUndo->UndoImpl(rContext);
                if( pUndo->IsMoveRange() )
                {
                    // delete the unnecessary node
                    aIdx = pUndo->GetEndNode();
                    SwContentNode *pCNd = aIdx.GetNode().GetContentNode();
                    if( pCNd )
                    {
                        SwNodeIndex aTmp( aIdx, -1 );
                        SwContentNode *pMove = aTmp.GetNode().GetContentNode();
                        if( pMove )
                            pCNd->MoveTo( *pMove );
                    }
                    rDoc.GetNodes().Delete( aIdx );
                }
                else if( pTextNd )
                {
                    // also delete not needed attributes
                    SwIndex aTmpIdx( pTextNd, nDelPos );
                    if( pTextNd->GetpSwpHints() && pTextNd->GetpSwpHints()->Count() )
                        pTextNd->RstTextAttr( aTmpIdx, pTextNd->GetText().getLength() - nDelPos + 1 );
                    // delete separator
                    pTextNd->EraseText( aTmpIdx, 1 );
                }
            }
            nIdx = pBox->GetSttIdx();
        }
        else
            pBox = pTableNd->GetTable().GetTableBox( nIdx );

        if( !pSaveTable->IsNewModel() )
        {
            // TL_CHART2: notify chart about box to be removed
            if (pPCD)
                pPCD->DeleteBox( &pTableNd->GetTable(), *pBox );

            SwTableBoxes* pTBoxes = &pBox->GetUpper()->GetTabBoxes();
            pTBoxes->erase( std::find(pTBoxes->begin(), pTBoxes->end(), pBox ) );

            // delete indices from section
            {
                SwNodeIndex aTmpIdx( *pBox->GetSttNd() );
                SwDoc::CorrAbs( SwNodeIndex( aTmpIdx, 1 ),
                            SwNodeIndex( *aTmpIdx.GetNode().EndOfSectionNode() ),
                            SwPosition( aTmpIdx, SwIndex( nullptr, 0 )), true );
            }

            delete pBox;
            rDoc.getIDocumentContentOperations().DeleteSection( rDoc.GetNodes()[ nIdx ] );
        }
    }
CHECKTABLE(pTableNd->GetTable())

    pSaveTable->CreateNew( pTableNd->GetTable(), true, false );

    // TL_CHART2: need to inform chart of probably changed cell names
    rDoc.UpdateCharts( pTableNd->GetTable().GetFrameFormat()->GetName() );

    if( pHistory )
    {
        pHistory->TmpRollback( &rDoc, 0 );
        pHistory->SetTmpEnd( pHistory->Count() );
    }
    SwPaM *const pPam(& rContext.GetCursorSupplier().CreateNewShellCursor());
    pPam->DeleteMark();
    pPam->GetPoint()->nNode = nSttNode;
    pPam->GetPoint()->nContent.Assign( pPam->GetContentNode(), nSttContent );
    pPam->SetMark();
    pPam->DeleteMark();

CHECKTABLE(pTableNd->GetTable())
    ClearFEShellTabCols();
}

void SwUndoTableMerge::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    SwPaM & rPam( AddUndoRedoPaM(rContext) );
    rDoc.MergeTable(rPam);
}

void SwUndoTableMerge::MoveBoxContent( SwDoc* pDoc, SwNodeRange& rRg, SwNodeIndex& rPos )
{
    SwNodeIndex aTmp( rRg.aStart, -1 ), aTmp2( rPos, -1 );
    std::unique_ptr<SwUndoMove> pUndo(new SwUndoMove( pDoc, rRg, rPos ));
    ::sw::UndoGuard const undoGuard(pDoc->GetIDocumentUndoRedo());
    pDoc->getIDocumentContentOperations().MoveNodeRange( rRg, rPos, pSaveTable->IsNewModel() ?
        SwMoveFlags::NO_DELFRMS :
        SwMoveFlags::DEFAULT );
    ++aTmp;
    ++aTmp2;
    pUndo->SetDestRange( aTmp2, rPos, aTmp );

    m_pMoves->push_back(std::move(pUndo));
}

void SwUndoTableMerge::SetSelBoxes( const SwSelBoxes& rBoxes )
{
    // memorize selection
    for (size_t n = 0; n < rBoxes.size(); ++n)
    {
        m_Boxes.insert(rBoxes[n]->GetSttIdx());
    }

    // as separator for inserts of new boxes after shifting
    aNewSttNds.push_back( (sal_uLong)0 );

    // The new table model does not delete overlapped cells (by row span),
    // so the rBoxes array might be empty even some cells have been merged.
    if( !rBoxes.empty() )
        nTableNode = rBoxes[ 0 ]->GetSttNd()->FindTableNode()->GetIndex();
}

void SwUndoTableMerge::SaveCollection( const SwTableBox& rBox )
{
    if( !pHistory )
        pHistory = new SwHistory;

    SwNodeIndex aIdx( *rBox.GetSttNd(), 1 );
    SwContentNode* pCNd = aIdx.GetNode().GetContentNode();
    if( !pCNd )
        pCNd = aIdx.GetNodes().GoNext( &aIdx );

    pHistory->Add( pCNd->GetFormatColl(), aIdx.GetIndex(), pCNd->GetNodeType());
    if( pCNd->HasSwAttrSet() )
        pHistory->CopyFormatAttr( *pCNd->GetpSwAttrSet(), aIdx.GetIndex() );
}

SwUndoTableNumFormat::SwUndoTableNumFormat( const SwTableBox& rBox,
                                    const SfxItemSet* pNewSet )
    : SwUndo(UNDO_TBLNUMFMT)
    , pBoxSet(nullptr)
    , pHistory(nullptr)
    , nFormatIdx(css::util::NumberFormat::TEXT)
    , nNewFormatIdx(0)
    , fNum(0.0)
    , fNewNum(0.0)
    , bNewFormat(false)
    , bNewFormula(false)
    , bNewValue(false)
{
    nNode = rBox.GetSttIdx();

    nNdPos = rBox.IsValidNumTextNd( nullptr == pNewSet );
    SwDoc* pDoc = rBox.GetFrameFormat()->GetDoc();

    if( ULONG_MAX != nNdPos )
    {
        SwTextNode* pTNd = pDoc->GetNodes()[ nNdPos ]->GetTextNode();

        pHistory = new SwHistory;
        SwRegHistory aRHst( *rBox.GetSttNd(), pHistory );
        // always save all text atttibutes because of possibly overlapping
        // areas of on/off
        pHistory->CopyAttr( pTNd->GetpSwpHints(), nNdPos, 0,
                            pTNd->GetText().getLength(), true );

        if( pTNd->HasSwAttrSet() )
            pHistory->CopyFormatAttr( *pTNd->GetpSwAttrSet(), nNdPos );

        aStr = pTNd->GetText();
        if( pTNd->GetpSwpHints() )
            pTNd->GetpSwpHints()->DeRegister();
    }

    pBoxSet = new SfxItemSet( pDoc->GetAttrPool(), aTableBoxSetRange );
    pBoxSet->Put( rBox.GetFrameFormat()->GetAttrSet() );

    if( pNewSet )
    {
        const SfxPoolItem* pItem;
        if( SfxItemState::SET == pNewSet->GetItemState( RES_BOXATR_FORMAT,
                false, &pItem ))
        {
            bNewFormat = true;
            nNewFormatIdx = static_cast<const SwTableBoxNumFormat*>(pItem)->GetValue();
        }
        if( SfxItemState::SET == pNewSet->GetItemState( RES_BOXATR_FORMULA,
                false, &pItem ))
        {
            bNewFormula = true;
            aNewFormula = static_cast<const SwTableBoxFormula*>(pItem)->GetFormula();
        }
        if( SfxItemState::SET == pNewSet->GetItemState( RES_BOXATR_VALUE,
                false, &pItem ))
        {
            bNewValue = true;
            fNewNum = static_cast<const SwTableBoxValue*>(pItem)->GetValue();
        }
    }

    // is a history needed at all?
    if( pHistory && !pHistory->Count() )
        DELETEZ( pHistory );
}

SwUndoTableNumFormat::~SwUndoTableNumFormat()
{
    delete pHistory;
    delete pBoxSet;
}

void SwUndoTableNumFormat::UndoImpl(::sw::UndoRedoContext & rContext)
{
    OSL_ENSURE( pBoxSet, "Where's the stored item set?" );

    SwDoc & rDoc = rContext.GetDoc();
    SwStartNode* pSttNd = rDoc.GetNodes()[ nNode ]->
                            FindSttNodeByType( SwTableBoxStartNode );
    OSL_ENSURE( pSttNd, "without StartNode no TableBox" );
    SwTableBox* pBox = pSttNd->FindTableNode()->GetTable().GetTableBox(
                                    pSttNd->GetIndex() );
    OSL_ENSURE( pBox, "found no TableBox" );

    SwTableBoxFormat* pFormat = rDoc.MakeTableBoxFormat();
    pFormat->SetFormatAttr( *pBoxSet );
    pBox->ChgFrameFormat( pFormat );

    if( ULONG_MAX == nNdPos )
        return;

    SwTextNode* pTextNd = rDoc.GetNodes()[ nNdPos ]->GetTextNode();
    // If more than one node was deleted than all "node" attributes were also
    // saved
    if( pTextNd->HasSwAttrSet() )
        pTextNd->ResetAllAttr();

    if( pTextNd->GetpSwpHints() && !aStr.isEmpty() )
        pTextNd->ClearSwpHintsArr( true );

    // ChgTextToNum(..) only acts when the strings are different. We need to do
    // the same here.
    if( pTextNd->GetText() != aStr )
    {
        rDoc.getIDocumentRedlineAccess().DeleteRedline( *( pBox->GetSttNd() ), false, USHRT_MAX );

        SwIndex aIdx( pTextNd, 0 );
        if( !aStr.isEmpty() )
        {
            pTextNd->EraseText( aIdx );
            pTextNd->InsertText( aStr, aIdx,
                SwInsertFlags::NOHINTEXPAND );
        }
    }

    if( pHistory )
    {
        sal_uInt16 nTmpEnd = pHistory->GetTmpEnd();
        pHistory->TmpRollback( &rDoc, 0 );
        pHistory->SetTmpEnd( nTmpEnd );
    }

    SwPaM *const pPam(& rContext.GetCursorSupplier().CreateNewShellCursor());
    pPam->DeleteMark();
    pPam->GetPoint()->nNode = nNode + 1;
    pPam->GetPoint()->nContent.Assign( pTextNd, 0 );
}

/** switch the RedlineMode on the given document, using
 * SetRedlineMode_intern. This class set the mode in the constructor,
 * and changes it back in the destructor, i.e. it uses the
 * initialization-is-resource-acquisition idiom.
 */
class RedlineModeInternGuard
{
    SwDoc& mrDoc;
    RedlineMode_t meOldRedlineMode;

public:
    RedlineModeInternGuard(
        SwDoc& rDoc,                      // change mode of this document
        RedlineMode_t eNewRedlineMode,    // new redline mode
        RedlineMode_t eRedlineModeMask  = (RedlineMode_t)(nsRedlineMode_t::REDLINE_ON | nsRedlineMode_t::REDLINE_IGNORE /*change only bits set in this mask*/));

    ~RedlineModeInternGuard();
};

RedlineModeInternGuard::RedlineModeInternGuard(
    SwDoc& rDoc,
    RedlineMode_t eNewRedlineMode,
    RedlineMode_t eRedlineModeMask )
    : mrDoc( rDoc ),
      meOldRedlineMode( rDoc.getIDocumentRedlineAccess().GetRedlineMode() )
{
    mrDoc.getIDocumentRedlineAccess().SetRedlineMode_intern((RedlineMode_t)( ( meOldRedlineMode & ~eRedlineModeMask ) |
                                     ( eNewRedlineMode & eRedlineModeMask ) ));
}

RedlineModeInternGuard::~RedlineModeInternGuard()
{
    mrDoc.getIDocumentRedlineAccess().SetRedlineMode_intern( meOldRedlineMode );
}

void SwUndoTableNumFormat::RedoImpl(::sw::UndoRedoContext & rContext)
{
    // Could the box be changed?
    if( !pBoxSet )
        return ;

    SwDoc & rDoc = rContext.GetDoc();
    SwPaM *const pPam(& rContext.GetCursorSupplier().CreateNewShellCursor());

    pPam->DeleteMark();
    pPam->GetPoint()->nNode = nNode;

    SwNode * pNd = & pPam->GetPoint()->nNode.GetNode();
    SwStartNode* pSttNd = pNd->FindSttNodeByType( SwTableBoxStartNode );
    assert(pSttNd && "without StartNode no TableBox");
    SwTableBox* pBox = pSttNd->FindTableNode()->GetTable().GetTableBox(
                                    pSttNd->GetIndex() );
    OSL_ENSURE( pBox, "found no TableBox" );

    SwFrameFormat* pBoxFormat = pBox->ClaimFrameFormat();
    if( bNewFormat || bNewFormula || bNewValue )
    {
        SfxItemSet aBoxSet( rDoc.GetAttrPool(),
                                RES_BOXATR_FORMAT, RES_BOXATR_VALUE );

        // Resetting attributes is not enough. In addition, take care that the
        // text will be also formatted correctly.
        pBoxFormat->LockModify();

        if( bNewFormula )
            aBoxSet.Put( SwTableBoxFormula( aNewFormula ));
        else
            pBoxFormat->ResetFormatAttr( RES_BOXATR_FORMULA );
        if( bNewFormat )
            aBoxSet.Put( SwTableBoxNumFormat( nNewFormatIdx ));
        else
            pBoxFormat->ResetFormatAttr( RES_BOXATR_FORMAT );
        if( bNewValue )
            aBoxSet.Put( SwTableBoxValue( fNewNum ));
        else
            pBoxFormat->ResetFormatAttr( RES_BOXATR_VALUE );
        pBoxFormat->UnlockModify();

        // dvo: When redlining is (was) enabled, setting the attribute
        // will also change the cell content. To allow this, the
        // REDLINE_IGNORE flag must be removed during Redo. #108450#
        RedlineModeInternGuard aGuard( rDoc, nsRedlineMode_t::REDLINE_NONE, nsRedlineMode_t::REDLINE_IGNORE );
        pBoxFormat->SetFormatAttr( aBoxSet );
    }
    else if( css::util::NumberFormat::TEXT != static_cast<sal_Int16>(nFormatIdx) )
    {
        SfxItemSet aBoxSet( rDoc.GetAttrPool(),
                            RES_BOXATR_FORMAT, RES_BOXATR_VALUE );

        aBoxSet.Put( SwTableBoxNumFormat( nFormatIdx ));
        aBoxSet.Put( SwTableBoxValue( fNum ));

        // Resetting attributes is not enough. In addition, take care that the
        // text will be also formatted correctly.
        pBoxFormat->LockModify();
        pBoxFormat->ResetFormatAttr( RES_BOXATR_FORMULA );
        pBoxFormat->UnlockModify();

        // dvo: When redlining is (was) enabled, setting the attribute
        // will also change the cell content. To allow this, the
        // REDLINE_IGNORE flag must be removed during Redo. #108450#
        RedlineModeInternGuard aGuard( rDoc, nsRedlineMode_t::REDLINE_NONE, nsRedlineMode_t::REDLINE_IGNORE );
        pBoxFormat->SetFormatAttr( aBoxSet );
    }
    else
    {
        // it's no number

        // Resetting attributes is not enough. In addition, take care that the
        // text will be also formatted correctly.
        pBoxFormat->SetFormatAttr( *GetDfltAttr( RES_BOXATR_FORMAT ));

        pBoxFormat->ResetFormatAttr( RES_BOXATR_FORMAT, RES_BOXATR_VALUE );
    }

    if( bNewFormula )
    {
        // No matter what was set, an update of the table is always a good idea
        SwTableFormulaUpdate aTableUpdate( &pSttNd->FindTableNode()->GetTable() );
        rDoc.getIDocumentFieldsAccess().UpdateTableFields( &aTableUpdate );
    }

    if( !pNd->IsContentNode() )
        pNd = rDoc.GetNodes().GoNext( &pPam->GetPoint()->nNode );
    pPam->GetPoint()->nContent.Assign( static_cast<SwContentNode*>(pNd), 0 );
}

void SwUndoTableNumFormat::SetBox( const SwTableBox& rBox )
{
    nNode = rBox.GetSttIdx();
}

_UndoTableCpyTable_Entry::_UndoTableCpyTable_Entry( const SwTableBox& rBox )
    : nBoxIdx( rBox.GetSttIdx() ), nOffset( 0 ),
    pBoxNumAttr( nullptr ), pUndo( nullptr ), bJoin( false )
{
}

_UndoTableCpyTable_Entry::~_UndoTableCpyTable_Entry()
{
    delete pUndo;
    delete pBoxNumAttr;
}

SwUndoTableCpyTable::SwUndoTableCpyTable()
    : SwUndo( UNDO_TBLCPYTBL )
    , m_pArr(new SwUndoTableCpyTable_Entries)
    , pInsRowUndo(nullptr)
{
}

SwUndoTableCpyTable::~SwUndoTableCpyTable()
{
    delete m_pArr;
    delete pInsRowUndo;
}

void SwUndoTableCpyTable::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    _DEBUG_REDLINE( &rDoc )

    SwTableNode* pTableNd = nullptr;
    for (size_t n = m_pArr->size(); n; )
    {
        _UndoTableCpyTable_Entry *const pEntry = (*m_pArr)[ --n ].get();
        sal_uLong nSttPos = pEntry->nBoxIdx + pEntry->nOffset;
        SwStartNode* pSNd = rDoc.GetNodes()[ nSttPos ]->StartOfSectionNode();
        if( !pTableNd )
            pTableNd = pSNd->FindTableNode();

        SwTableBox& rBox = *pTableNd->GetTable().GetTableBox( nSttPos );

        SwNodeIndex aInsIdx( *rBox.GetSttNd(), 1 );
        rDoc.GetNodes().MakeTextNode( aInsIdx, rDoc.GetDfltTextFormatColl() );

        // b62341295: Redline for copying tables
        const SwNode *pEndNode = rBox.GetSttNd()->EndOfSectionNode();
        SwPaM aPam( aInsIdx.GetNode(), *pEndNode );
        SwUndoDelete* pUndo = nullptr;

        if( IDocumentRedlineAccess::IsRedlineOn( GetRedlineMode() ) )
        {
            bool bDeleteCompleteParagraph = false;
            bool bShiftPam = false;
            // There are a couple of different situations to consider during redlining
            if( pEntry->pUndo )
            {
                SwUndoDelete *const pUndoDelete =
                    dynamic_cast<SwUndoDelete*>(pEntry->pUndo);
                SwUndoRedlineDelete *const pUndoRedlineDelete =
                    dynamic_cast<SwUndoRedlineDelete*>(pEntry->pUndo);
                OSL_ASSERT(pUndoDelete || pUndoRedlineDelete);
                if (pUndoRedlineDelete)
                {
                    // The old content was not empty or he has been merged with the new content
                    bDeleteCompleteParagraph = !pEntry->bJoin; // bJoin is set when merged
                    // Set aTmpIdx to the beginning fo the old content
                    SwNodeIndex aTmpIdx( *pEndNode,
                            pUndoRedlineDelete->NodeDiff()-1 );
                    SwTextNode *pText = aTmpIdx.GetNode().GetTextNode();
                    if( pText )
                    {
                        aPam.GetPoint()->nNode = *pText;
                        aPam.GetPoint()->nContent.Assign( pText,
                                pUndoRedlineDelete->ContentStart() );
                    }
                    else
                        *aPam.GetPoint() = SwPosition( aTmpIdx );
                }
                else if (pUndoDelete && pUndoDelete->IsDelFullPara())
                {
                    // When the old content was an empty paragraph, but could not be joined
                    // with the new content (e.g. because of a section or table)
                    // We "save" the aPam.Point, we go one step backwards (because later on the
                    // empty paragraph will be inserted by the undo) and set the "ShiftPam-flag
                    // for step forward later on.
                    bDeleteCompleteParagraph = true;
                    bShiftPam = true;
                    SwNodeIndex aTmpIdx( *pEndNode, -1 );
                    SwTextNode *pText = aTmpIdx.GetNode().GetTextNode();
                    if( pText )
                    {
                        aPam.GetPoint()->nNode = *pText;
                        aPam.GetPoint()->nContent.Assign( pText, 0 );
                    }
                    else
                        *aPam.GetPoint() = SwPosition( aTmpIdx );
                }
            }
            rDoc.getIDocumentRedlineAccess().DeleteRedline( aPam, true, USHRT_MAX );

            if( pEntry->pUndo )
            {
                pEntry->pUndo->UndoImpl(rContext);
                delete pEntry->pUndo;
                pEntry->pUndo = nullptr;
            }
            if( bShiftPam )
            {
                // The aPam.Point is at the moment at the last position of the new content and has to be
                // moved to the first position of the old content for the SwUndoDelete operation
                SwNodeIndex aTmpIdx( aPam.GetPoint()->nNode, 1 );
                SwTextNode *pText = aTmpIdx.GetNode().GetTextNode();
                if( pText )
                {
                    aPam.GetPoint()->nNode = *pText;
                    aPam.GetPoint()->nContent.Assign( pText, 0 );
                }
                else
                    *aPam.GetPoint() = SwPosition( aTmpIdx );
            }
            pUndo = new SwUndoDelete( aPam, bDeleteCompleteParagraph, true );
        }
        else
        {
            pUndo = new SwUndoDelete( aPam, true );
            if( pEntry->pUndo )
            {
                pEntry->pUndo->UndoImpl(rContext);
                delete pEntry->pUndo;
                pEntry->pUndo = nullptr;
            }
        }
        pEntry->pUndo = pUndo;

        aInsIdx = rBox.GetSttIdx() + 1;
        rDoc.GetNodes().Delete( aInsIdx );

        SfxItemSet aTmpSet( rDoc.GetAttrPool(), RES_BOXATR_FORMAT, RES_BOXATR_VALUE,
                                                RES_VERT_ORIENT, RES_VERT_ORIENT, 0 );
        aTmpSet.Put( rBox.GetFrameFormat()->GetAttrSet() );
        if( aTmpSet.Count() )
        {
            SwFrameFormat* pBoxFormat = rBox.ClaimFrameFormat();
            pBoxFormat->ResetFormatAttr( RES_BOXATR_FORMAT, RES_BOXATR_VALUE );
            pBoxFormat->ResetFormatAttr( RES_VERT_ORIENT );
        }

        if( pEntry->pBoxNumAttr )
        {
            rBox.ClaimFrameFormat()->SetFormatAttr( *pEntry->pBoxNumAttr );
            delete pEntry->pBoxNumAttr;
            pEntry->pBoxNumAttr = nullptr;
        }

        if( aTmpSet.Count() )
        {
            pEntry->pBoxNumAttr = new SfxItemSet( rDoc.GetAttrPool(),
                                    RES_BOXATR_FORMAT, RES_BOXATR_VALUE,
                                    RES_VERT_ORIENT, RES_VERT_ORIENT, 0 );
            pEntry->pBoxNumAttr->Put( aTmpSet );
        }

        pEntry->nOffset = rBox.GetSttIdx() - pEntry->nBoxIdx;
    }

    if( pInsRowUndo )
    {
        pInsRowUndo->UndoImpl(rContext);
    }
    _DEBUG_REDLINE( &rDoc )
}

void SwUndoTableCpyTable::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    _DEBUG_REDLINE( &rDoc )

    if( pInsRowUndo )
    {
        pInsRowUndo->RedoImpl(rContext);
    }

    SwTableNode* pTableNd = nullptr;
    for (size_t n = 0; n < m_pArr->size(); ++n)
    {
        _UndoTableCpyTable_Entry *const pEntry = (*m_pArr)[ n ].get();
        sal_uLong nSttPos = pEntry->nBoxIdx + pEntry->nOffset;
        SwStartNode* pSNd = rDoc.GetNodes()[ nSttPos ]->StartOfSectionNode();
        if( !pTableNd )
            pTableNd = pSNd->FindTableNode();

        SwTableBox& rBox = *pTableNd->GetTable().GetTableBox( nSttPos );

        SwNodeIndex aInsIdx( *rBox.GetSttNd(), 1 );

        // b62341295: Redline for copying tables - Start.
        rDoc.GetNodes().MakeTextNode( aInsIdx, rDoc.GetDfltTextFormatColl() );
        SwPaM aPam( aInsIdx.GetNode(), *rBox.GetSttNd()->EndOfSectionNode());
        SwUndo* pUndo = IDocumentRedlineAccess::IsRedlineOn( GetRedlineMode() ) ? nullptr : new SwUndoDelete( aPam, true );
        if( pEntry->pUndo )
        {
            pEntry->pUndo->UndoImpl(rContext);
            if( IDocumentRedlineAccess::IsRedlineOn( GetRedlineMode() ) )
            {
                // PrepareRedline has to be called with the beginning of the old content
                // When new and old content has been joined, the rIter.pAktPam has been set
                // by the Undo operation to this point.
                // Otherwise aInsIdx has been moved during the Undo operation
                if( pEntry->bJoin )
                {
                    SwPaM const& rLastPam =
                        rContext.GetCursorSupplier().GetCurrentShellCursor();
                    pUndo = PrepareRedline( &rDoc, rBox, *rLastPam.GetPoint(),
                                            pEntry->bJoin, true );
                }
                else
                {
                    SwPosition aTmpPos( aInsIdx );
                    pUndo = PrepareRedline( &rDoc, rBox, aTmpPos, pEntry->bJoin, true );
                }
            }
            delete pEntry->pUndo;
            pEntry->pUndo = nullptr;
        }
        pEntry->pUndo = pUndo;
        // b62341295: Redline for copying tables - End.

        aInsIdx = rBox.GetSttIdx() + 1;
        rDoc.GetNodes().Delete( aInsIdx );

        SfxItemSet aTmpSet( rDoc.GetAttrPool(), RES_BOXATR_FORMAT, RES_BOXATR_VALUE,
                                                RES_VERT_ORIENT, RES_VERT_ORIENT, 0 );
        aTmpSet.Put( rBox.GetFrameFormat()->GetAttrSet() );
        if( aTmpSet.Count() )
        {
            SwFrameFormat* pBoxFormat = rBox.ClaimFrameFormat();
            pBoxFormat->ResetFormatAttr( RES_BOXATR_FORMAT, RES_BOXATR_VALUE );
            pBoxFormat->ResetFormatAttr( RES_VERT_ORIENT );
        }
        if( pEntry->pBoxNumAttr )
        {
            rBox.ClaimFrameFormat()->SetFormatAttr( *pEntry->pBoxNumAttr );
            delete pEntry->pBoxNumAttr;
            pEntry->pBoxNumAttr = nullptr;
        }

        if( aTmpSet.Count() )
        {
            pEntry->pBoxNumAttr = new SfxItemSet( rDoc.GetAttrPool(),
                                    RES_BOXATR_FORMAT, RES_BOXATR_VALUE,
                                    RES_VERT_ORIENT, RES_VERT_ORIENT, 0 );
            pEntry->pBoxNumAttr->Put( aTmpSet );
        }

        pEntry->nOffset = rBox.GetSttIdx() - pEntry->nBoxIdx;
    }
    _DEBUG_REDLINE( &rDoc )
}

void SwUndoTableCpyTable::AddBoxBefore( const SwTableBox& rBox, bool bDelContent )
{
    if (!m_pArr->empty() && !bDelContent)
        return;

    _UndoTableCpyTable_Entry* pEntry = new _UndoTableCpyTable_Entry( rBox );
    m_pArr->push_back(std::unique_ptr<_UndoTableCpyTable_Entry>(pEntry));

    SwDoc* pDoc = rBox.GetFrameFormat()->GetDoc();
    _DEBUG_REDLINE( pDoc )
    if( bDelContent )
    {
        SwNodeIndex aInsIdx( *rBox.GetSttNd(), 1 );
        pDoc->GetNodes().MakeTextNode( aInsIdx, pDoc->GetDfltTextFormatColl() );
        SwPaM aPam( aInsIdx.GetNode(), *rBox.GetSttNd()->EndOfSectionNode() );

        if( !pDoc->getIDocumentRedlineAccess().IsRedlineOn() )
            pEntry->pUndo = new SwUndoDelete( aPam, true );
    }

    pEntry->pBoxNumAttr = new SfxItemSet( pDoc->GetAttrPool(),
                                    RES_BOXATR_FORMAT, RES_BOXATR_VALUE,
                                    RES_VERT_ORIENT, RES_VERT_ORIENT, 0 );
    pEntry->pBoxNumAttr->Put( rBox.GetFrameFormat()->GetAttrSet() );
    if( !pEntry->pBoxNumAttr->Count() )
    {
        delete pEntry->pBoxNumAttr;
        pEntry->pBoxNumAttr = nullptr;
    }
    _DEBUG_REDLINE( pDoc )
}

void SwUndoTableCpyTable::AddBoxAfter( const SwTableBox& rBox, const SwNodeIndex& rIdx, bool bDelContent )
{
    _UndoTableCpyTable_Entry *const pEntry = (*m_pArr).back().get();

    // If the content was deleted than remove also the temporarily created node
    if( bDelContent )
    {
        SwDoc* pDoc = rBox.GetFrameFormat()->GetDoc();
        _DEBUG_REDLINE( pDoc )

        if( pDoc->getIDocumentRedlineAccess().IsRedlineOn() )
        {
            SwPosition aTmpPos( rIdx );
            pEntry->pUndo = PrepareRedline( pDoc, rBox, aTmpPos, pEntry->bJoin, false );
        }
        SwNodeIndex aDelIdx( *rBox.GetSttNd(), 1 );
        rBox.GetFrameFormat()->GetDoc()->GetNodes().Delete( aDelIdx );
        _DEBUG_REDLINE( pDoc )
    }

    pEntry->nOffset = rBox.GetSttIdx() - pEntry->nBoxIdx;
}

// PrepareRedline is called from AddBoxAfter() and from Redo() in slightly different situations.
// bRedo is set by calling from Redo()
// rJoin is false by calling from AddBoxAfter() and will be set if the old and new content has
// been merged.
// rJoin is true if Redo() is calling and the content has already been merged

SwUndo* SwUndoTableCpyTable::PrepareRedline( SwDoc* pDoc, const SwTableBox& rBox,
    const SwPosition& rPos, bool& rJoin, bool bRedo )
{
    SwUndo *pUndo = nullptr;
    // b62341295: Redline for copying tables
    // What's to do?
    // Mark the cell content before rIdx as insertion,
    // mark the cell content behind rIdx as deletion
    // merge text nodes at rIdx if possible
    RedlineMode_t eOld = pDoc->getIDocumentRedlineAccess().GetRedlineMode();
    pDoc->getIDocumentRedlineAccess().SetRedlineMode_intern((RedlineMode_t)( ( eOld | nsRedlineMode_t::REDLINE_DONTCOMBINE_REDLINES ) &
                                     ~nsRedlineMode_t::REDLINE_IGNORE ));
    SwPosition aInsertEnd( rPos );
    SwTextNode* pText;
    if( !rJoin )
    {
        // If the content is not merged, the end of the insertion is at the end of the node
        // _before_ the given position rPos
        --aInsertEnd.nNode;
        pText = aInsertEnd.nNode.GetNode().GetTextNode();
        if( pText )
        {
            aInsertEnd.nContent.Assign(pText, pText->GetText().getLength());
            if( !bRedo && rPos.nNode.GetNode().GetTextNode() )
            {   // Try to merge, if not called by Redo()
                rJoin = true;
                pText->JoinNext();
            }
        }
        else
            aInsertEnd.nContent = SwIndex( nullptr );
    }
    // For joined (merged) contents the start of deletion and end of insertion are identical
    // otherwise adjacent nodes.
    SwPosition aDeleteStart( rJoin ? aInsertEnd : rPos );
    if( !rJoin )
    {
        pText = aDeleteStart.nNode.GetNode().GetTextNode();
        if( pText )
            aDeleteStart.nContent.Assign( pText, 0 );
    }
    SwPosition aCellEnd( SwNodeIndex( *rBox.GetSttNd()->EndOfSectionNode(), -1 ) );
    pText = aCellEnd.nNode.GetNode().GetTextNode();
    if( pText )
        aCellEnd.nContent.Assign(pText, pText->GetText().getLength());
    if( aDeleteStart != aCellEnd )
    {   // If the old (deleted) part is not empty, here we are...
        SwPaM aDeletePam( aDeleteStart, aCellEnd );
        pUndo = new SwUndoRedlineDelete( aDeletePam, UNDO_DELETE );
        pDoc->getIDocumentRedlineAccess().AppendRedline( new SwRangeRedline( nsRedlineType_t::REDLINE_DELETE, aDeletePam ), true );
    }
    else if( !rJoin ) // If the old part is empty and joined, we are finished
    {   // if it is not joined, we have to delete this empty paragraph
        aCellEnd = SwPosition(
            SwNodeIndex( *rBox.GetSttNd()->EndOfSectionNode() ));
        SwPaM aTmpPam( aDeleteStart, aCellEnd );
        pUndo = new SwUndoDelete( aTmpPam, true );
    }
    SwPosition aCellStart( SwNodeIndex( *rBox.GetSttNd(), 2 ) );
    pText = aCellStart.nNode.GetNode().GetTextNode();
    if( pText )
        aCellStart.nContent.Assign( pText, 0 );
    if( aCellStart != aInsertEnd ) // An empty insertion will not been marked
    {
        SwPaM aTmpPam( aCellStart, aInsertEnd );
        pDoc->getIDocumentRedlineAccess().AppendRedline( new SwRangeRedline( nsRedlineType_t::REDLINE_INSERT, aTmpPam ), true );
    }

    pDoc->getIDocumentRedlineAccess().SetRedlineMode_intern( eOld );
    return pUndo;
}

bool SwUndoTableCpyTable::InsertRow( SwTable& rTable, const SwSelBoxes& rBoxes,
                                sal_uInt16 nCnt )
{
    SwTableNode* pTableNd = const_cast<SwTableNode*>(rTable.GetTabSortBoxes()[0]->
                                GetSttNd()->FindTableNode());

    pInsRowUndo = new SwUndoTableNdsChg( UNDO_TABLE_INSROW, rBoxes, *pTableNd,
                                       0, 0, nCnt, true, false );
    SwTableSortBoxes aTmpLst( rTable.GetTabSortBoxes() );

    bool bRet = rTable.InsertRow( rTable.GetFrameFormat()->GetDoc(), rBoxes, nCnt );
    if( bRet )
        pInsRowUndo->SaveNewBoxes( *pTableNd, aTmpLst );
    else
    {
        delete pInsRowUndo;
        pInsRowUndo = nullptr;
    }
    return bRet;
}

bool SwUndoTableCpyTable::IsEmpty() const
{
    return !pInsRowUndo && m_pArr->empty();
}

SwUndoCpyTable::SwUndoCpyTable()
    : SwUndo( UNDO_CPYTBL ), pDel( nullptr ), nTableNode( 0 )
{
}

SwUndoCpyTable::~SwUndoCpyTable()
{
    delete pDel;
}

void SwUndoCpyTable::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    SwTableNode* pTNd = rDoc.GetNodes()[ nTableNode ]->GetTableNode();

    // move hard page breaks into next node
    SwContentNode* pNextNd = rDoc.GetNodes()[ pTNd->EndOfSectionIndex()+1 ]->GetContentNode();
    if( pNextNd )
    {
        SwFrameFormat* pTableFormat = pTNd->GetTable().GetFrameFormat();
        const SfxPoolItem *pItem;

        if( SfxItemState::SET == pTableFormat->GetItemState( RES_PAGEDESC,
            false, &pItem ) )
            pNextNd->SetAttr( *pItem );

        if( SfxItemState::SET == pTableFormat->GetItemState( RES_BREAK,
            false, &pItem ) )
            pNextNd->SetAttr( *pItem );
    }

    SwPaM aPam( *pTNd, *pTNd->EndOfSectionNode(), 0 , 1 );
    pDel = new SwUndoDelete( aPam, true );
}

void SwUndoCpyTable::RedoImpl(::sw::UndoRedoContext & rContext)
{
    pDel->UndoImpl(rContext);
    delete pDel;
    pDel = nullptr;
}

SwUndoSplitTable::SwUndoSplitTable( const SwTableNode& rTableNd,
    SwSaveRowSpan* pRowSp, sal_uInt16 eMode, bool bNewSize )
    : SwUndo( UNDO_SPLIT_TABLE ),
    nTableNode( rTableNd.GetIndex() ), nOffset( 0 ), mpSaveRowSpan( pRowSp ), pSavTable( nullptr ),
    pHistory( nullptr ), nMode( eMode ), nFormulaEnd( 0 ), bCalcNewSize( bNewSize )
{
    switch( nMode )
    {
    case HEADLINE_BOXATRCOLLCOPY:
            pHistory = new SwHistory;
            // no break
    case HEADLINE_BORDERCOPY:
    case HEADLINE_BOXATTRCOPY:
        pSavTable = new _SaveTable( rTableNd.GetTable(), 1, false );
        break;
    }
}

SwUndoSplitTable::~SwUndoSplitTable()
{
    delete pSavTable;
    delete pHistory;
    delete mpSaveRowSpan;
}

void SwUndoSplitTable::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc *const pDoc = & rContext.GetDoc();
    SwPaM *const pPam(& rContext.GetCursorSupplier().CreateNewShellCursor());

    SwNodeIndex& rIdx = pPam->GetPoint()->nNode;
    rIdx = nTableNode + nOffset;
    pPam->GetPoint()->nContent.Assign(rIdx.GetNode().GetContentNode(), 0);
    assert(rIdx.GetNode().GetContentNode()->Len() == 0); // empty para inserted

    {
        // avoid asserts from ~SwIndexReg
        SwNodeIndex const idx(pDoc->GetNodes(), nTableNode + nOffset);
        {
            SwPaM pam(idx);
            pam.Move(fnMoveBackward, fnGoContent);
            ::PaMCorrAbs(*pPam, *pam.GetPoint());
        }

        // remove implicitly created paragraph again
        pDoc->GetNodes().Delete( idx );
    }

    rIdx = nTableNode + nOffset;
    SwTableNode* pTableNd = rIdx.GetNode().GetTableNode();
    SwTable& rTable = pTableNd->GetTable();

    SwTableFormulaUpdate aMsgHint( &rTable );
    aMsgHint.m_eFlags = TBL_BOXPTR;
    pDoc->getIDocumentFieldsAccess().UpdateTableFields( &aMsgHint );

    switch( nMode )
    {
    case HEADLINE_BOXATRCOLLCOPY:
        if( pHistory )
            pHistory->TmpRollback( pDoc, nFormulaEnd );
        // no break
    case HEADLINE_BOXATTRCOPY:
    case HEADLINE_BORDERCOPY:
        {
            pSavTable->CreateNew( rTable, false );
            pSavTable->RestoreAttr( rTable );
        }
        break;

    case HEADLINE_CNTNTCOPY:
        // the created first line has to be removed again
        {
            SwSelBoxes aSelBoxes;
            SwTableBox* pBox = rTable.GetTableBox( nTableNode + nOffset + 1 );
            SwTable::SelLineFromBox( pBox, aSelBoxes );
            _FndBox aTmpBox( nullptr, nullptr );
            aTmpBox.SetTableLines( aSelBoxes, rTable );
            aTmpBox.DelFrames( rTable );
            rTable.DeleteSel( pDoc, aSelBoxes, nullptr, nullptr, false, false );
        }
        break;
    }

    pDoc->GetNodes().MergeTable( rIdx );

    if( pHistory )
    {
        pHistory->TmpRollback( pDoc, 0 );
        pHistory->SetTmpEnd( pHistory->Count() );
    }
    if( mpSaveRowSpan )
    {
        pTableNd = rIdx.GetNode().FindTableNode();
        if( pTableNd )
            pTableNd->GetTable().RestoreRowSpan( *mpSaveRowSpan );
    }
    ClearFEShellTabCols();
}

void SwUndoSplitTable::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc *const pDoc = & rContext.GetDoc();
    SwPaM *const pPam(& rContext.GetCursorSupplier().CreateNewShellCursor());

    pPam->DeleteMark();
    pPam->GetPoint()->nNode = nTableNode;
    pDoc->SplitTable( *pPam->GetPoint(), nMode, bCalcNewSize );

    ClearFEShellTabCols();
}

void SwUndoSplitTable::RepeatImpl(::sw::RepeatContext & rContext)
{
    SwPaM *const pPam = & rContext.GetRepeatPaM();
    SwDoc *const pDoc = & rContext.GetDoc();

    pDoc->SplitTable( *pPam->GetPoint(), nMode, bCalcNewSize );
    ClearFEShellTabCols();
}

void SwUndoSplitTable::SaveFormula( SwHistory& rHistory )
{
    if( !pHistory )
        pHistory = new SwHistory;

    nFormulaEnd = rHistory.Count();
    pHistory->Move( 0, &rHistory );
}

SwUndoMergeTable::SwUndoMergeTable( const SwTableNode& rTableNd,
                                const SwTableNode& rDelTableNd,
                                bool bWithPrv, sal_uInt16 nMd )
    : SwUndo( UNDO_MERGE_TABLE ), pSavTable( nullptr ),
    pHistory( nullptr ), nMode( nMd ), bWithPrev( bWithPrv )
{
    // memorize end node of the last table cell that'll stay in position
    if( bWithPrev )
        nTableNode = rDelTableNd.EndOfSectionIndex() - 1;
    else
        nTableNode = rTableNd.EndOfSectionIndex() - 1;

    aName = rDelTableNd.GetTable().GetFrameFormat()->GetName();
    pSavTable = new _SaveTable( rDelTableNd.GetTable() );

    pSavHdl = bWithPrev ? new _SaveTable( rTableNd.GetTable(), 1 ) : nullptr;
}

SwUndoMergeTable::~SwUndoMergeTable()
{
    delete pSavTable;
    delete pSavHdl;
    delete pHistory;
}

void SwUndoMergeTable::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc *const pDoc = & rContext.GetDoc();
    SwPaM *const pPam(& rContext.GetCursorSupplier().CreateNewShellCursor());

    pPam->DeleteMark();
    SwNodeIndex& rIdx = pPam->GetPoint()->nNode;
    rIdx = nTableNode;

    SwTableNode* pTableNd = rIdx.GetNode().FindTableNode();
    SwTable* pTable = &pTableNd->GetTable();

    SwTableFormulaUpdate aMsgHint( pTable );
    aMsgHint.m_eFlags = TBL_BOXPTR;
    pDoc->getIDocumentFieldsAccess().UpdateTableFields( &aMsgHint );

    // get lines for layout update
    _FndBox aFndBox( nullptr, nullptr );
    aFndBox.SetTableLines( *pTable );
    aFndBox.DelFrames( *pTable );
    // ? TL_CHART2: notification or locking of controller required ?

    SwTableNode* pNew = pDoc->GetNodes().SplitTable( rIdx );

    // update layout
    aFndBox.MakeFrames( *pTable );
    // ? TL_CHART2: notification or locking of controller required ?

    if( bWithPrev )
    {
        // move name
        pNew->GetTable().GetFrameFormat()->SetName( pTable->GetFrameFormat()->GetName() );
        pSavHdl->RestoreAttr( pNew->GetTable() );
    }
    else
        pTable = &pNew->GetTable();

    pTable->GetFrameFormat()->SetName( aName );
    pSavTable->RestoreAttr( *pTable );

    if( pHistory )
    {
        pHistory->TmpRollback( pDoc, 0 );
        pHistory->SetTmpEnd( pHistory->Count() );
    }

    // create frames for the new table
    SwNodeIndex aTmpIdx( *pNew );
    pNew->MakeFrames( &aTmpIdx );

    // position cursor somewhere in content
    SwContentNode* pCNd = pDoc->GetNodes().GoNext( &rIdx );
    pPam->GetPoint()->nContent.Assign( pCNd, 0 );

    ClearFEShellTabCols();

    // TL_CHART2: need to inform chart of probably changed cell names
    SwChartDataProvider *pPCD = pDoc->getIDocumentChartDataProviderAccess().GetChartDataProvider();
    if (pPCD)
    {
        pDoc->UpdateCharts( pTable->GetFrameFormat()->GetName() );
        pDoc->UpdateCharts( pNew->GetTable().GetFrameFormat()->GetName() );
    }
}

void SwUndoMergeTable::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc *const pDoc = & rContext.GetDoc();
    SwPaM *const pPam(& rContext.GetCursorSupplier().CreateNewShellCursor());

    pPam->DeleteMark();
    pPam->GetPoint()->nNode = nTableNode;
    if( bWithPrev )
        pPam->GetPoint()->nNode = nTableNode + 3;
    else
        pPam->GetPoint()->nNode = nTableNode;

    pDoc->MergeTable( *pPam->GetPoint(), bWithPrev, nMode );

    ClearFEShellTabCols();
}

void SwUndoMergeTable::RepeatImpl(::sw::RepeatContext & rContext)
{
    SwDoc *const pDoc = & rContext.GetDoc();
    SwPaM *const pPam = & rContext.GetRepeatPaM();

    pDoc->MergeTable( *pPam->GetPoint(), bWithPrev, nMode );
    ClearFEShellTabCols();
}

void SwUndoMergeTable::SaveFormula( SwHistory& rHistory )
{
    if( !pHistory )
        pHistory = new SwHistory;
    pHistory->Move( 0, &rHistory );
}

void InsertSort( std::vector<sal_uInt16>& rArr, sal_uInt16 nIdx, sal_uInt16* pInsPos )
{
    size_t nO = rArr.size();
    size_t nU = 0;
    if( nO > 0 )
    {
        nO--;
        while( nU <= nO )
        {
            const size_t nM = nU + ( nO - nU ) / 2;
            if ( rArr[nM] == nIdx )
            {
                OSL_FAIL( "Index already exists. This should never happen." );
                return;
            }
            if( rArr[nM] < nIdx )
                nU = nM + 1;
            else if( nM == 0 )
                break;
            else
                nO = nM - 1;
        }
    }
    rArr.insert( rArr.begin() + nU, nIdx );
    if( pInsPos )
        *pInsPos = nU;
}

#if OSL_DEBUG_LEVEL > 0
void CheckTable( const SwTable& rTable )
{
    const SwNodes& rNds = rTable.GetFrameFormat()->GetDoc()->GetNodes();
    const SwTableSortBoxes& rSrtArr = rTable.GetTabSortBoxes();
    for (size_t n = 0; n < rSrtArr.size(); ++n)
    {
        const SwTableBox* pBox = rSrtArr[ n ];
        const SwNode* pNd = pBox->GetSttNd();
        OSL_ENSURE( rNds[ pBox->GetSttIdx() ] == pNd, "Box with wrong StartNode"  );
    }
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
