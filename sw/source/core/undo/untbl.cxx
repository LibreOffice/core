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

#include <libxml/xmlwriter.h>

#include <UndoTable.hxx>
#include <UndoRedline.hxx>
#include <UndoDelete.hxx>
#include <UndoSplitMove.hxx>
#include <UndoCore.hxx>
#include <fesh.hxx>
#include <fmtpdsc.hxx>
#include <hintids.hxx>
#include <hints.hxx>
#include <doc.hxx>
#include <docredln.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentChartDataProviderAccess.hxx>
#include <IDocumentRedlineAccess.hxx>
#include <IDocumentFieldsAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <rootfrm.hxx>
#include <editsh.hxx>
#include <docary.hxx>
#include <ndtxt.hxx>
#include <swtable.hxx>
#include <pam.hxx>
#include <tblsel.hxx>
#include <swundo.hxx>
#include <rolbck.hxx>
#include <ddefld.hxx>
#include <tabfrm.hxx>
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
#include <strings.hrc>
#include <unochart.hxx>
#include <calbck.hxx>
#include <frameformats.hxx>
#include <editeng/formatbreakitem.hxx>
#include <osl/diagnose.h>
#include <docsh.hxx>

#include <memory>
#include <utility>
#include <vector>

#ifdef DBG_UTIL
#define CHECK_TABLE(t) (t).CheckConsistency();
#else
#define CHECK_TABLE(t)
#endif

#ifdef DBG_UTIL
    #define DEBUG_REDLINE( pDoc ) sw_DebugRedline( pDoc );
#else
    #define DEBUG_REDLINE( pDoc )
#endif

typedef std::vector<std::shared_ptr<SfxItemSet> > SfxItemSets;

struct UndoTableCpyTable_Entry
{
    SwNodeOffset nBoxIdx, nOffset;
    std::unique_ptr<SfxItemSet> pBoxNumAttr;
    std::unique_ptr<SwUndo> pUndo;

    // Was the last paragraph of the new and the first paragraph of the old content joined?
    bool bJoin; // For redlining only

    explicit UndoTableCpyTable_Entry( const SwTableBox& rBox );

    void dumpAsXml(xmlTextWriterPtr pWriter) const;
};

namespace {

class SaveBox;
class SaveLine;

void KillEmptyFrameFormat(SwFrameFormat& rFormat)
{
    if(!rFormat.HasWriterListeners())
        delete &rFormat;
};

}

class SaveTable
{
    friend SaveBox;
    friend SaveLine;
    SfxItemSet m_aTableSet;
    std::unique_ptr<SaveLine> m_pLine;
    const SwTable* m_pSwTable;
    SfxItemSets m_aSets;
    SwFrameFormatsV m_aFrameFormats;
    sal_uInt16 m_nLineCount;
    bool m_bModifyBox : 1;
    bool m_bSaveFormula : 1;
    bool m_bNewModel : 1;

    SaveTable(const SaveTable&) = delete;
    SaveTable& operator=(const SaveTable&) = delete;
    SwFrameFormat& CreateNewFormat(SwFrameFormat& rFormat, sal_uInt16 nFormatPos);

public:
    SaveTable( const SwTable& rTable, sal_uInt16 nLnCnt = USHRT_MAX,
                bool bSaveFormula = true );

    sal_uInt16 AddFormat( SwFrameFormat* pFormat, bool bIsLine );
    void NewFrameFormatForLine(const SwTableLine&, sal_uInt16 nFormatPos, SwFrameFormat* pOldFormat);
    void NewFrameFormatForBox(const SwTableBox&, sal_uInt16 nFormatPos, SwFrameFormat* pOldFormat);

    void RestoreAttr( SwTable& rTable, bool bModifyBox = false );
    void SaveContentAttrs( SwDoc* pDoc );
    void CreateNew( SwTable& rTable, bool bCreateFrames = true,
                    bool bRestoreChart = true );
    bool IsNewModel() const { return m_bNewModel; }
};

namespace {

class SaveLine
{
    friend SaveTable;
    friend class SaveBox;

    SaveLine* m_pNext;
    SaveBox* m_pBox;
    sal_uInt16 m_nItemSet;

    SaveLine(const SaveLine&) = delete;
    SaveLine& operator=(const SaveLine&) = delete;

public:
    SaveLine( SaveLine* pPrev, const SwTableLine& rLine, SaveTable& rSTable );
    ~SaveLine();

    void RestoreAttr( SwTableLine& rLine, SaveTable& rSTable );
    void SaveContentAttrs( SwDoc* pDoc );

    void CreateNew( SwTable& rTable, SwTableBox& rParent, SaveTable& rSTable  );
};

class SaveBox
{
    friend class SaveLine;

    SaveBox* m_pNext;
    SwNodeOffset m_nStartNode;
    sal_Int32 m_nRowSpan;
    sal_uInt16 m_nItemSet;
    union
    {
        SfxItemSets* pContentAttrs;
        SaveLine* pLine;
    } m_Ptrs;

public:
    SaveBox( SaveBox* pPrev, const SwTableBox& rBox, SaveTable& rSTable );
    ~SaveBox();

    void RestoreAttr( SwTableBox& rBox, SaveTable& rSTable );
    void SaveContentAttrs( SwDoc* pDoc );

    void CreateNew( SwTable& rTable, SwTableLine& rParent, SaveTable& rSTable );
};

}

#if OSL_DEBUG_LEVEL > 0
static void CheckTable( const SwTable& );
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
    SwNodeOffset m_nSttNd;
    SwNodeOffset m_nEndNd;
    sal_Int32 m_nContent;
    std::unique_ptr<SwHistory> m_pHstry;
    // metadata references for first and last paragraph in cell
    std::shared_ptr< ::sfx2::MetadatableUndo > m_pMetadataUndoStart;
    std::shared_ptr< ::sfx2::MetadatableUndo > m_pMetadataUndoEnd;

    SwTableToTextSave( SwDoc& rDoc, SwNodeOffset nNd, SwNodeOffset nEndIdx, sal_Int32 nContent );

private:
    SwTableToTextSave(const SwTableToTextSave&) = delete;
    SwTableToTextSave& operator=(const SwTableToTextSave&) = delete;

};

WhichRangesContainer const aSave_BoxContentSet(svl::Items<
    RES_CHRATR_COLOR, RES_CHRATR_CROSSEDOUT,
    RES_CHRATR_FONT, RES_CHRATR_FONTSIZE,
    RES_CHRATR_POSTURE, RES_CHRATR_POSTURE,
    RES_CHRATR_SHADOWED, RES_CHRATR_WEIGHT,
    RES_PARATR_ADJUST, RES_PARATR_ADJUST>);

SwUndoInsTable::SwUndoInsTable( const SwPosition& rPos, sal_uInt16 nCl, sal_uInt16 nRw,
                            sal_uInt16 nAdj, const SwInsertTableOptions& rInsTableOpts,
                            const SwTableAutoFormat* pTAFormat,
                            const std::vector<sal_uInt16> *pColArr,
                            const OUString & rName)
    : SwUndo( SwUndoId::INSTABLE, &rPos.GetDoc() ),
    m_aInsTableOptions( rInsTableOpts ),
    m_nStartNode( rPos.GetNodeIndex() ), m_nRows( nRw ), m_nColumns( nCl ), m_nAdjust( nAdj )
{
    if( pColArr )
    {
        m_oColumnWidth.emplace( *pColArr );
    }
    if( pTAFormat )
        m_pAutoFormat.reset( new SwTableAutoFormat( *pTAFormat ) );

    // consider redline
    SwDoc& rDoc = rPos.GetNode().GetDoc();
    if( rDoc.getIDocumentRedlineAccess().IsRedlineOn() )
    {
        m_pRedlineData.reset( new SwRedlineData( RedlineType::Insert, rDoc.getIDocumentRedlineAccess().GetRedlineAuthor() ) );
        SetRedlineFlags( rDoc.getIDocumentRedlineAccess().GetRedlineFlags() );
    }

    m_sTableName = rName;
}

SwUndoInsTable::~SwUndoInsTable()
{
    m_pDDEFieldType.reset();
    m_oColumnWidth.reset();
    m_pRedlineData.reset();
    m_pAutoFormat.reset();
}

void SwUndoInsTable::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    SwNodeIndex aIdx( rDoc.GetNodes(), m_nStartNode );

    SwTableNode* pTableNd = aIdx.GetNode().GetTableNode();
    // tdf#159025 skip undo if SwTableNode is a nullptr
    // I don't know what causes the SwTableNode to be a nullptr in the
    // case of tdf#159025, but at least stop the crashing by skipping
    // this undo request.
    SAL_WARN_IF( !pTableNd, "sw.core", "no TableNode" );
    if( !pTableNd )
        return;

    pTableNd->DelFrames();

    if( IDocumentRedlineAccess::IsRedlineOn( GetRedlineFlags() ))
        rDoc.getIDocumentRedlineAccess().DeleteRedline( *pTableNd, true, RedlineType::Any );
    RemoveIdxFromSection( rDoc, m_nStartNode );

    // move hard page breaks into next node
    SwContentNode* pNextNd = rDoc.GetNodes()[ pTableNd->EndOfSectionIndex()+1 ]->GetContentNode();
    if( pNextNd )
    {
        SwFrameFormat* pTableFormat = pTableNd->GetTable().GetFrameFormat();

        if( const SwFormatPageDesc* pItem = pTableFormat->GetItemIfSet( RES_PAGEDESC,
            false ) )
            pNextNd->SetAttr( *pItem );

        if( const SvxFormatBreakItem* pItem = pTableFormat->GetItemIfSet( RES_BREAK,
            false ) )
            pNextNd->SetAttr( *pItem );

        ::sw::NotifyTableCollapsedParagraph(pNextNd, nullptr);
    }

    m_sTableName = pTableNd->GetTable().GetFrameFormat()->GetName();
    if( auto pDDETable = dynamic_cast<const SwDDETable *>(&pTableNd->GetTable()) )
        m_pDDEFieldType.reset(static_cast<SwDDEFieldType*>(pDDETable->GetDDEFieldType()->Copy().release()));

    rDoc.GetNodes().Delete( aIdx, pTableNd->EndOfSectionIndex() -
                                aIdx.GetIndex() + 1 );

    SwPaM & rPam( rContext.GetCursorSupplier().CreateNewShellCursor() );
    rPam.DeleteMark();
    rPam.GetPoint()->Assign(aIdx);
}

void SwUndoInsTable::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();

    SwEditShell *const pEditShell(rDoc.GetEditShell());
    OSL_ENSURE(pEditShell, "SwUndoInsTable::RedoImpl needs a SwEditShell!");
    if (!pEditShell)
    {
        throw uno::RuntimeException();
    }

    SwPosition const aPos(rDoc.GetNodes(), m_nStartNode);
    const SwTable* pTable = rDoc.InsertTable( m_aInsTableOptions, aPos, m_nRows, m_nColumns,
                                            m_nAdjust,
                                            m_pAutoFormat.get(),
                                            m_oColumnWidth ? &*m_oColumnWidth : nullptr );
    pEditShell->MoveTable( GotoPrevTable, fnTableStart );
    static_cast<SwFrameFormat*>(pTable->GetFrameFormat())->SetFormatName( m_sTableName );
    SwTableNode* pTableNode = rDoc.GetNodes()[m_nStartNode]->GetTableNode();

    if( m_pDDEFieldType )
    {
        SwDDEFieldType* pNewType = static_cast<SwDDEFieldType*>(rDoc.getIDocumentFieldsAccess().InsertFieldType(
                                                            *m_pDDEFieldType));
        std::unique_ptr<SwDDETable> pDDETable(new SwDDETable( pTableNode->GetTable(), pNewType ));
        pTableNode->SetNewTable( std::move(pDDETable) );
        m_pDDEFieldType.reset();
    }

    if( !((m_pRedlineData && IDocumentRedlineAccess::IsRedlineOn( GetRedlineFlags() )) ||
        ( !( RedlineFlags::Ignore & GetRedlineFlags() ) &&
            !rDoc.getIDocumentRedlineAccess().GetRedlineTable().empty() )))
        return;

    SwPaM aPam( *pTableNode->EndOfSectionNode(), *pTableNode, SwNodeOffset(1) );

    if( m_pRedlineData && IDocumentRedlineAccess::IsRedlineOn( GetRedlineFlags() ) )
    {
        RedlineFlags eOld = rDoc.getIDocumentRedlineAccess().GetRedlineFlags();
        rDoc.getIDocumentRedlineAccess().SetRedlineFlags_intern(eOld & ~RedlineFlags::Ignore);

        rDoc.getIDocumentRedlineAccess().AppendRedline( new SwRangeRedline( *m_pRedlineData, aPam ), true);
        rDoc.getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
    }
    else
        rDoc.getIDocumentRedlineAccess().SplitRedline( aPam );
}

void SwUndoInsTable::RepeatImpl(::sw::RepeatContext & rContext)
{
    rContext.GetDoc().InsertTable(
            m_aInsTableOptions, *rContext.GetRepeatPaM().GetPoint(),
            m_nRows, m_nColumns, m_nAdjust, m_pAutoFormat.get(),
            m_oColumnWidth ? &*m_oColumnWidth : nullptr );
}

SwRewriter SwUndoInsTable::GetRewriter() const
{
    SwRewriter aRewriter;

    aRewriter.AddRule(UndoArg1, SwResId(STR_START_QUOTE));
    aRewriter.AddRule(UndoArg2, m_sTableName);
    aRewriter.AddRule(UndoArg3, SwResId(STR_END_QUOTE));

    return aRewriter;
}

SwTableToTextSave::SwTableToTextSave( SwDoc& rDoc, SwNodeOffset nNd, SwNodeOffset nEndIdx, sal_Int32 nCnt )
    : m_nSttNd( nNd ), m_nEndNd( nEndIdx), m_nContent( nCnt )
{
    // keep attributes of the joined node
    SwTextNode* pNd = rDoc.GetNodes()[ nNd ]->GetTextNode();
    if( pNd )
    {
        m_pHstry.reset( new SwHistory );

        m_pHstry->AddColl(pNd->GetTextColl(), nNd, SwNodeType::Text);
        if ( pNd->GetpSwpHints() )
        {
            m_pHstry->CopyAttr( pNd->GetpSwpHints(), nNd, 0,
                        pNd->GetText().getLength(), false );
        }
        if( pNd->HasSwAttrSet() )
            m_pHstry->CopyFormatAttr( *pNd->GetpSwAttrSet(), nNd );

        if( !m_pHstry->Count() )
        {
            m_pHstry.reset();
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
    : SwUndo( SwUndoId::TABLETOTEXT, rTable.GetFrameFormat()->GetDoc() ),
    m_sTableName( rTable.GetFrameFormat()->GetName() ),
    m_nStartNode( 0 ), m_nEndNode( 0 ),
    m_cSeparator( cCh ), m_nHeadlineRepeat( rTable.GetRowsToRepeat() )
{
    m_pTableSave.reset( new SaveTable( rTable ) );
    m_vBoxSaves.reserve(rTable.GetTabSortBoxes().size());

    if( auto pDDETable = dynamic_cast<const SwDDETable *>(&rTable) )
        m_pDDEFieldType.reset(static_cast<SwDDEFieldType*>(pDDETable->GetDDEFieldType()->Copy().release()));

    m_bCheckNumFormat = rTable.GetFrameFormat()->GetDoc()->IsInsTableFormatNum();

    m_pHistory.reset(new SwHistory);
    const SwTableNode* pTableNd = rTable.GetTableNode();
    SwNodeOffset nTableStt = pTableNd->GetIndex(), nTableEnd = pTableNd->EndOfSectionIndex();

    for(sw::SpzFrameFormat* pFormat: *pTableNd->GetDoc().GetSpzFrameFormats())
    {
        SwFormatAnchor const*const pAnchor = &pFormat->GetAnchor();
        SwNode const*const pAnchorNode = pAnchor->GetAnchorNode();
        if (pAnchorNode &&
            ((RndStdIds::FLY_AT_CHAR == pAnchor->GetAnchorId()) ||
             (RndStdIds::FLY_AT_PARA == pAnchor->GetAnchorId())) &&
            nTableStt <= pAnchorNode->GetIndex() &&
            pAnchorNode->GetIndex() < nTableEnd )
        {
            m_pHistory->AddChangeFlyAnchor(*pFormat);
        }
    }

    if( !m_pHistory->Count() )
    {
        m_pHistory.reset();
    }
}

SwUndoTableToText::~SwUndoTableToText()
{
    m_pDDEFieldType.reset();
    m_pTableSave.reset();
    m_vBoxSaves.clear();
    m_pHistory.reset();
}

void SwUndoTableToText::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    SwPaM *const pPam(& rContext.GetCursorSupplier().CreateNewShellCursor());

    SwNodeIndex aFrameIdx( rDoc.GetNodes(), m_nStartNode );
    SwNodeIndex aEndIdx( rDoc.GetNodes(), m_nEndNode );

    pPam->GetPoint()->Assign( aFrameIdx );
    pPam->SetMark();
    pPam->GetPoint()->Assign( aEndIdx );
    rDoc.DelNumRules( *pPam );
    pPam->DeleteMark();

    // now collect all Uppers
    SwNode2LayoutSaveUpperFrames aNode2Layout(aFrameIdx.GetNode());

    // create TableNode structure
    SwTableNode* pTableNd = rDoc.GetNodes().UndoTableToText( m_nStartNode, m_nEndNode, m_vBoxSaves );
    pTableNd->GetTable().SetTableModel( m_pTableSave->IsNewModel() );
    SwTableFormat* pTableFormat = rDoc.MakeTableFrameFormat( m_sTableName, rDoc.GetDfltFrameFormat() );
    pTableNd->GetTable().RegisterToFormat( *pTableFormat );
    pTableNd->GetTable().SetRowsToRepeat( m_nHeadlineRepeat );

    // create old table structure
    m_pTableSave->CreateNew( pTableNd->GetTable() );

    if( m_pDDEFieldType )
    {
        SwDDEFieldType* pNewType = static_cast<SwDDEFieldType*>(rDoc.getIDocumentFieldsAccess().InsertFieldType(
                                                            *m_pDDEFieldType));
        std::unique_ptr<SwDDETable> pDDETable( new SwDDETable( pTableNd->GetTable(), pNewType ) );
        pTableNd->SetNewTable( std::move(pDDETable), false );
        m_pDDEFieldType.reset();
    }

    if( m_bCheckNumFormat )
    {
        SwTableSortBoxes& rBxs = pTableNd->GetTable().GetTabSortBoxes();
        for (size_t nBoxes = rBxs.size(); nBoxes; )
        {
            rDoc.ChkBoxNumFormat( *rBxs[ --nBoxes ], false );
        }
    }

    if( m_pHistory )
    {
        sal_uInt16 nTmpEnd = m_pHistory->GetTmpEnd();
        m_pHistory->TmpRollback( &rDoc, 0 );
        m_pHistory->SetTmpEnd( nTmpEnd );
    }

    aNode2Layout.RestoreUpperFrames( rDoc.GetNodes(),
                                   pTableNd->GetIndex(), pTableNd->GetIndex()+1 );

    // Is a table selection requested?
    pPam->DeleteMark();
    pPam->GetPoint()->Assign( *pTableNd->EndOfSectionNode() );
    pPam->SetMark();
    pPam->GetPoint()->Assign( *pPam->GetPointNode().StartOfSectionNode() );
    pPam->Move( fnMoveForward, GoInContent );
    pPam->Exchange();
    pPam->Move( fnMoveBackward, GoInContent );

    ClearFEShellTabCols(rDoc, nullptr);
}

// located in untbl.cxx and only an Undo object is allowed to call it
SwTableNode* SwNodes::UndoTableToText( SwNodeOffset nSttNd, SwNodeOffset nEndNd,
                                const SwTableToTextSaves& rSavedData )
{
    SwNodeIndex aSttIdx( *this, nSttNd );
    SwNodeIndex aEndIdx( *this, nEndNd+1 );

    SwTableNode * pTableNd = new SwTableNode( aSttIdx.GetNode() );
    SwEndNode* pEndNd = new SwEndNode( aEndIdx.GetNode(), *pTableNd  );

    aEndIdx = *pEndNd;

    /* Set pTableNd as start of section for all nodes in [nSttNd, nEndNd].
       Delete all Frames attached to the nodes in that range. */
    SwNode* pNd;
    {
        SwNodeOffset n, nTmpEnd = aEndIdx.GetIndex();
        for( n = pTableNd->GetIndex() + 1; n < nTmpEnd; ++n )
        {
            pNd = (*this)[n];
            if (pNd->IsContentNode())
            {
                static_cast<SwContentNode*>(pNd)->DelFrames(nullptr);
            }
            // tdf#147938 reset merge flag in nodes
            pNd->SetRedlineMergeFlag(SwNode::Merge::None);
            pNd->m_pStartOfSection = pTableNd;
        }
    }

    // than create table structure partially. First a single line that contains
    // all boxes. The correct structure is then taken from SaveStruct.
    SwTableBoxFormat* pBoxFormat = GetDoc().MakeTableBoxFormat();
    SwTableLineFormat* pLineFormat = GetDoc().MakeTableLineFormat();
    SwTableLine* pLine = new SwTableLine( pLineFormat, rSavedData.size(), nullptr );
    pTableNd->GetTable().GetTabLines().insert( pTableNd->GetTable().GetTabLines().begin(), pLine );

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
            SwContentIndex aCntPos( pTextNd, pSave->m_nContent - 1 );

            const std::shared_ptr<sw::mark::ContentIdxStore> pContentStore(sw::mark::ContentIdxStore::Create());
            pContentStore->Save(GetDoc(), aSttIdx.GetIndex(), aCntPos.GetIndex());

            pTextNd->EraseText( aCntPos, 1 );

            std::function<void (SwTextNode *, sw::mark::RestoreMode, bool)> restoreFunc(
                [&](SwTextNode *const pNewNode, sw::mark::RestoreMode const eMode, bool)
                {
                    if (!pContentStore->Empty())
                    {
                        pContentStore->Restore(*pNewNode, pSave->m_nContent, pSave->m_nContent + 1, eMode);
                    }
                });
            pTextNd->SplitContentNode(
                        SwPosition(aSttIdx, aCntPos), &restoreFunc);
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
            pSave->m_pHstry->TmpRollback( &GetDoc(), 0 );
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
        SwStartNode* pSttNd = new SwStartNode( aSttIdx.GetNode(), SwNodeType::Start,
                                                SwTableBoxStartNode );
        pSttNd->m_pStartOfSection = pTableNd;
        new SwEndNode( aEndIdx.GetNode(), *pSttNd );

        for( SwNodeOffset i = aSttIdx.GetIndex(); i < aEndIdx.GetIndex()-1; ++i )
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

    pPam->GetPoint()->Assign( m_nStartNode );
    SwNodeIndex aSaveIdx( pPam->GetPoint()->GetNode(), -1 );

    pPam->SetMark();            // log off all indices
    pPam->DeleteMark();

    SwTableNode* pTableNd = pPam->GetPointNode().GetTableNode();
    OSL_ENSURE( pTableNd, "Could not find any TableNode" );

    if( auto pDDETable = dynamic_cast<const SwDDETable *>(&pTableNd->GetTable()) )
        m_pDDEFieldType.reset(static_cast<SwDDEFieldType*>(pDDETable->GetDDEFieldType()->Copy().release()));

    rDoc.TableToText( pTableNd, m_cSeparator );

    ++aSaveIdx;
    SwContentNode* pCNd = aSaveIdx.GetNode().GetContentNode();
    if( !pCNd && nullptr == ( pCNd = rDoc.GetNodes().GoNext( &aSaveIdx ) ) &&
        nullptr == ( pCNd = SwNodes::GoPrevious( &aSaveIdx )) )
    {
        OSL_FAIL( "Where is the TextNode now?" );
    }

    pPam->GetPoint()->Assign( aSaveIdx );

    pPam->SetMark();            // log off all indices
    pPam->DeleteMark();
}

void SwUndoTableToText::RepeatImpl(::sw::RepeatContext & rContext)
{
    SwPaM *const pPam = & rContext.GetRepeatPaM();
    SwTableNode *const pTableNd = pPam->GetPointNode().FindTableNode();
    if( pTableNd )
    {
        // move cursor out of table
        pPam->GetPoint()->Assign( *pTableNd->EndOfSectionNode() );
        pPam->Move( fnMoveForward, GoInContent );
        pPam->SetMark();
        pPam->DeleteMark();

        rContext.GetDoc().TableToText( pTableNd, m_cSeparator );
    }
}

void SwUndoTableToText::SetRange( const SwNodeRange& rRg )
{
    m_nStartNode = rRg.aStart.GetIndex();
    m_nEndNode = rRg.aEnd.GetIndex();
}

void SwUndoTableToText::AddBoxPos( SwDoc& rDoc, SwNodeOffset nNdIdx, SwNodeOffset nEndIdx, sal_Int32 nContentIdx )
{
    m_vBoxSaves.push_back(std::make_unique<SwTableToTextSave>(rDoc, nNdIdx, nEndIdx, nContentIdx));
}

SwUndoTextToTable::SwUndoTextToTable( const SwPaM& rRg,
                                const SwInsertTableOptions& rInsTableOpts,
                                sal_Unicode cCh, sal_uInt16 nAdj,
                                const SwTableAutoFormat* pAFormat )
    : SwUndo( SwUndoId::TEXTTOTABLE, &rRg.GetDoc() ), SwUndRng( rRg ), m_aInsertTableOpts( rInsTableOpts ),
      m_pHistory( nullptr ), m_cSeparator( cCh ), m_nAdjust( nAdj )
{
    if( pAFormat )
        m_pAutoFormat.reset( new SwTableAutoFormat( *pAFormat ) );

    const SwPosition* pEnd = rRg.End();
    SwNodes& rNds = rRg.GetDoc().GetNodes();
    m_bSplitEnd = pEnd->GetContentIndex() && ( pEnd->GetContentIndex()
                        != pEnd->GetNode().GetContentNode()->Len() ||
                pEnd->GetNodeIndex() >= rNds.GetEndOfContent().GetIndex()-1 );
}

SwUndoTextToTable::~SwUndoTextToTable()
{
    m_pAutoFormat.reset();
}

void SwUndoTextToTable::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();

    SwNodeOffset nTableNd = m_nSttNode;
    if( m_nSttContent )
        ++nTableNd;       // Node was split previously
    SwNodeIndex aIdx( rDoc.GetNodes(), nTableNd );
    SwTableNode *const pTNd = aIdx.GetNode().GetTableNode();
    OSL_ENSURE( pTNd, "Could not find a TableNode" );

    RemoveIdxFromSection( rDoc, nTableNd );

    m_sTableName = pTNd->GetTable().GetFrameFormat()->GetName();

    if( m_pHistory )
    {
        m_pHistory->TmpRollback( &rDoc, 0 );
        m_pHistory->SetTmpEnd( m_pHistory->Count() );
    }

    if( !mvDelBoxes.empty() )
    {
        pTNd->DelFrames();
        SwTable& rTable = pTNd->GetTable();
        for( size_t n = mvDelBoxes.size(); n; )
        {
            SwTableBox* pBox = rTable.GetTableBox( mvDelBoxes[ --n ] );
            if( pBox )
                ::DeleteBox_( rTable, pBox, nullptr, false, false );
            else {
                OSL_ENSURE( false, "Where is my box?" );
            }
        }
    }

    rDoc.TableToText( pTNd, 0x0b == m_cSeparator ? 0x09 : m_cSeparator );

    // join again at start?
    if( m_nSttContent )
    {
        SwPaM aPam(rDoc.GetNodes(), nTableNd);
        if (aPam.Move(fnMoveBackward, GoInContent))
        {
            SwNode & rIdx = aPam.GetPoint()->GetNode();

            // than move, relatively, the Cursor/etc. again
            RemoveIdxRel( rIdx.GetIndex()+1, *aPam.GetPoint() );

            rIdx.GetContentNode()->JoinNext();
        }
    }

    // join again at end?
    if( m_bSplitEnd )
    {
        SwPosition aEndPos( rDoc.GetNodes(), m_nEndNode );
        SwTextNode* pTextNd = aEndPos.GetNode().GetTextNode();
        if( pTextNd && pTextNd->CanJoinNext() )
        {
            aEndPos.nContent.Assign( nullptr, 0 );

            // than move, relatively, the Cursor/etc. again
            aEndPos.SetContent(pTextNd->GetText().getLength());
            RemoveIdxRel( m_nEndNode + 1, aEndPos );

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
                m_aInsertTableOpts, rPam, m_cSeparator, m_nAdjust, m_pAutoFormat.get() );
    static_cast<SwFrameFormat*>(pTable->GetFrameFormat())->SetFormatName( m_sTableName );
}

void SwUndoTextToTable::RepeatImpl(::sw::RepeatContext & rContext)
{
    // no Table In Table
    if (!rContext.GetRepeatPaM().GetPointNode().FindTableNode())
    {
        rContext.GetDoc().TextToTable( m_aInsertTableOpts, rContext.GetRepeatPaM(),
                                        m_cSeparator, m_nAdjust,
                                        m_pAutoFormat.get() );
    }
}

void SwUndoTextToTable::AddFillBox( const SwTableBox& rBox )
{
    mvDelBoxes.push_back( rBox.GetSttIdx() );
}

SwHistory& SwUndoTextToTable::GetHistory()
{
    if( !m_pHistory )
        m_pHistory = new SwHistory;
    return *m_pHistory;
}

SwUndoTableHeadline::SwUndoTableHeadline( const SwTable& rTable, sal_uInt16 nOldHdl,
                                      sal_uInt16 nNewHdl )
    : SwUndo( SwUndoId::TABLEHEADLINE, rTable.GetFrameFormat()->GetDoc() ),
    m_nOldHeadline( nOldHdl ),
    m_nNewHeadline( nNewHdl )
{
    OSL_ENSURE( !rTable.GetTabSortBoxes().empty(), "Table without content" );
    const SwStartNode *pSttNd = rTable.GetTabSortBoxes()[ 0 ]->GetSttNd();
    OSL_ENSURE( pSttNd, "Box without content" );

    m_nTableNode = pSttNd->StartOfSectionIndex();
}

void SwUndoTableHeadline::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    SwTableNode* pTNd = rDoc.GetNodes()[ m_nTableNode ]->GetTableNode();
    OSL_ENSURE( pTNd, "could not find any TableNode" );

    rDoc.SetRowsToRepeat( pTNd->GetTable(), m_nOldHeadline );
}

void SwUndoTableHeadline::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();

    SwTableNode* pTNd = rDoc.GetNodes()[ m_nTableNode ]->GetTableNode();
    OSL_ENSURE( pTNd, "could not find any TableNode" );

    rDoc.SetRowsToRepeat( pTNd->GetTable(), m_nNewHeadline );
}

void SwUndoTableHeadline::RepeatImpl(::sw::RepeatContext & rContext)
{
    SwTableNode *const pTableNd =
        rContext.GetRepeatPaM().GetPointNode().FindTableNode();
    if( pTableNd )
    {
        rContext.GetDoc().SetRowsToRepeat( pTableNd->GetTable(), m_nNewHeadline );
    }
}

SaveTable::SaveTable( const SwTable& rTable, sal_uInt16 nLnCnt, bool bSaveFormula )
    : m_aTableSet(*rTable.GetFrameFormat()->GetAttrSet().GetPool(), aTableSetRange),
      m_pSwTable(&rTable), m_nLineCount(nLnCnt), m_bSaveFormula(bSaveFormula)
{
    m_bModifyBox = false;
    m_bNewModel = rTable.IsNewModel();
    m_aTableSet.Put(rTable.GetFrameFormat()->GetAttrSet());
    m_pLine.reset( new SaveLine( nullptr, *rTable.GetTabLines()[ 0 ], *this ) );

    SaveLine* pLn = m_pLine.get();
    if( USHRT_MAX == nLnCnt )
        nLnCnt = rTable.GetTabLines().size();
    for( sal_uInt16 n = 1; n < nLnCnt; ++n )
        pLn = new SaveLine( pLn, *rTable.GetTabLines()[ n ], *this );

    m_aFrameFormats.clear();
    m_pSwTable = nullptr;
}

sal_uInt16 SaveTable::AddFormat( SwFrameFormat* pFormat, bool bIsLine )
{
    size_t nRet = m_aFrameFormats.GetPos(pFormat);
    if( SIZE_MAX == nRet )
    {
        // Create copy of ItemSet
        auto pSet = std::make_shared<SfxItemSet>( *pFormat->GetAttrSet().GetPool(),
            bIsLine ? aTableLineSetRange : aTableBoxSetRange );
        pSet->Put( pFormat->GetAttrSet() );
        // When a formula is set, never save the value. It possibly must be
        // recalculated.
        // Save formulas always in plain text.
        if( const SwTableBoxFormula* pItem = pSet->GetItemIfSet( RES_BOXATR_FORMULA ))
        {
            pSet->ClearItem( RES_BOXATR_VALUE );
            if (m_pSwTable && m_bSaveFormula)
            {
                const_cast<SwTable*>(m_pSwTable)->SwitchFormulasToExternalRepresentation();
                SwTableBoxFormula* pFormulaItem = const_cast<SwTableBoxFormula*>(pItem);
                pFormulaItem->ChgDefinedIn(pFormat);
                pFormulaItem->ToRelBoxNm(m_pSwTable);
                pFormulaItem->ChgDefinedIn(nullptr);
            }
        }
        nRet = m_aSets.size();
        m_aSets.push_back(pSet);
        m_aFrameFormats.insert(m_aFrameFormats.begin() + nRet, pFormat);
    }
    return o3tl::narrowing<sal_uInt16>(nRet);
}

void SaveTable::RestoreAttr( SwTable& rTable, bool bMdfyBox )
{
    m_bModifyBox = bMdfyBox;

    FndBox_ aTmpBox( nullptr, nullptr );
    bool bHideChanges = rTable.GetFrameFormat()->GetDoc()->getIDocumentLayoutAccess().GetCurrentLayout()->IsHideRedlines();
    // TODO delete/make frames only at changing line attribute TextChangesOnly (RES_PRINT) to true again
    if ( bHideChanges )
        aTmpBox.DelFrames( rTable );

    // first, get back attributes of TableFrameFormat
    SwFrameFormat* pFormat = rTable.GetFrameFormat();
    SfxItemSet& rFormatSet  = const_cast<SfxItemSet&>(static_cast<SfxItemSet const &>(pFormat->GetAttrSet()));
    rFormatSet.ClearItem();
    rFormatSet.Put(m_aTableSet);

    pFormat->InvalidateInSwCache(RES_ATTRSET_CHG);

    // table without table frame
    bool bHiddenTable = true;

    // for safety, invalidate all TableFrames
    SwIterator<SwTabFrame,SwFormat> aIter( *pFormat );
    for( SwTabFrame* pLast = aIter.First(); pLast; pLast = aIter.Next() )
    {
        if( pLast->GetTable() == &rTable )
        {
            pLast->InvalidateAll();
            pLast->SetCompletePaint();
            bHiddenTable = false;
        }
    }

    // fill FrameFormats with defaults (0)
    pFormat = nullptr;
    for (size_t n = m_aSets.size(); n; --n)
        m_aFrameFormats.push_back(pFormat);

    const size_t nLnCnt = (USHRT_MAX == m_nLineCount)
        ? rTable.GetTabLines().size()
        : m_nLineCount;

    SaveLine* pLn = m_pLine.get();
    for (size_t n = 0; n < nLnCnt; ++n, pLn = pLn->m_pNext)
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

    if ( bHideChanges )
    {
        if ( bHiddenTable )
        {
            SwTableNode* pTableNode = rTable.GetTableNode();
            pTableNode->DelFrames();
            pTableNode->MakeOwnFrames();
        }
        else
        {
            aTmpBox.MakeFrames( rTable );
        }
    }
}

void SaveTable::SaveContentAttrs( SwDoc* pDoc )
{
    m_pLine->SaveContentAttrs(pDoc);
}

void SaveTable::CreateNew( SwTable& rTable, bool bCreateFrames,
                            bool bRestoreChart )
{
    FndBox_ aTmpBox( nullptr, nullptr );
    aTmpBox.DelFrames( rTable );

    // first, get back attributes of TableFrameFormat
    SwFrameFormat* pFormat = rTable.GetFrameFormat();
    SfxItemSet& rFormatSet  = const_cast<SfxItemSet&>(static_cast<SfxItemSet const &>(pFormat->GetAttrSet()));
    rFormatSet.ClearItem();
    rFormatSet.Put(m_aTableSet);

    pFormat->InvalidateInSwCache(RES_ATTRSET_CHG);

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

        for( SwTableLines::const_iterator it = rTable.GetTabLines().begin() + n;
             it != rTable.GetTabLines().begin() + nOldLines; ++it )
            delete *it;
        rTable.GetTabLines().erase( rTable.GetTabLines().begin() + n, rTable.GetTabLines().begin() + nOldLines );
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

SwFrameFormat& SaveTable::CreateNewFormat(SwFrameFormat& rFormat, sal_uInt16 nFormatPos)
{
    rFormat.SetFormatAttr(*m_aSets[nFormatPos]);
    m_aFrameFormats[nFormatPos] = &rFormat;
    return rFormat;
}

void SaveTable::NewFrameFormatForLine(const SwTableLine& rTableLn, sal_uInt16 nFormatPos, SwFrameFormat* pOldFormat)
{
    SwFrameFormat* pFormat = m_aFrameFormats[nFormatPos];
    if(!pFormat)
        pFormat = &CreateNewFormat(*pOldFormat->GetDoc()->MakeTableLineFormat(), nFormatPos);
    pOldFormat->CallSwClientNotify(sw::MoveTableLineHint(*pFormat, rTableLn));
    pFormat->Add(const_cast<SwTableLine*>(&rTableLn));
    KillEmptyFrameFormat(*pOldFormat);
}

void SaveTable::NewFrameFormatForBox(const SwTableBox& rTableBx, sal_uInt16 nFormatPos, SwFrameFormat* pOldFormat)
{
    SwFrameFormat* pFormat = m_aFrameFormats[nFormatPos];
    if(!pFormat)
        pFormat = &CreateNewFormat(*pOldFormat->GetDoc()->MakeTableBoxFormat(), nFormatPos);
    pOldFormat->CallSwClientNotify(sw::MoveTableBoxHint(*pFormat, rTableBx));
    pFormat->MoveTableBox(*const_cast<SwTableBox*>(&rTableBx), m_bModifyBox ? pOldFormat : nullptr);
    KillEmptyFrameFormat(*pOldFormat);
}

SaveLine::SaveLine(SaveLine* pPrev, const SwTableLine& rLine, SaveTable& rSTable)
    : m_pNext(nullptr)
{
    if( pPrev )
        pPrev->m_pNext = this;

    m_nItemSet = rSTable.AddFormat(rLine.GetFrameFormat(), true);

    m_pBox = new SaveBox(nullptr, *rLine.GetTabBoxes()[0], rSTable);
    SaveBox* pBx = m_pBox;
    for( size_t n = 1; n < rLine.GetTabBoxes().size(); ++n )
        pBx = new SaveBox( pBx, *rLine.GetTabBoxes()[ n ], rSTable );
}

SaveLine::~SaveLine()
{
    delete m_pBox;
    delete m_pNext;
}

void SaveLine::RestoreAttr( SwTableLine& rLine, SaveTable& rSTable )
{
    rSTable.NewFrameFormatForLine(rLine, m_nItemSet, rLine.GetFrameFormat());

    SaveBox* pBx = m_pBox;
    for (size_t n = 0; n < rLine.GetTabBoxes().size(); ++n, pBx = pBx->m_pNext)
    {
        if( !pBx )
        {
            OSL_ENSURE( false, "Number of boxes changed" );
            break;
        }
        pBx->RestoreAttr( *rLine.GetTabBoxes()[ n ], rSTable );
    }
}

void SaveLine::SaveContentAttrs( SwDoc* pDoc )
{
    m_pBox->SaveContentAttrs(pDoc);
    if (m_pNext)
        m_pNext->SaveContentAttrs(pDoc);
}

void SaveLine::CreateNew( SwTable& rTable, SwTableBox& rParent, SaveTable& rSTable )
{
    SwTableLineFormat* pFormat
        = static_cast<SwTableLineFormat*>(rSTable.m_aFrameFormats[m_nItemSet]);
    if( !pFormat )
    {
        SwDoc* pDoc = rTable.GetFrameFormat()->GetDoc();
        pFormat = pDoc->MakeTableLineFormat();
        pFormat->SetFormatAttr(*rSTable.m_aSets[m_nItemSet]);
        rSTable.m_aFrameFormats[m_nItemSet] = pFormat;
    }
    SwTableLine* pNew = new SwTableLine( pFormat, 1, &rParent );

    rParent.GetTabLines().push_back( pNew );

    m_pBox->CreateNew(rTable, *pNew, rSTable);

    if (m_pNext)
        m_pNext->CreateNew(rTable, rParent, rSTable);
}

SaveBox::SaveBox(SaveBox* pPrev, const SwTableBox& rBox, SaveTable& rSTable)
    : m_pNext(nullptr)
    , m_nStartNode(NODE_OFFSET_MAX)
    , m_nRowSpan(0)
{
    m_Ptrs.pLine = nullptr;

    if( pPrev )
        pPrev->m_pNext = this;

    m_nItemSet = rSTable.AddFormat(rBox.GetFrameFormat(), false);

    if( rBox.GetSttNd() )
    {
        m_nStartNode = rBox.GetSttIdx();
        m_nRowSpan = rBox.getRowSpan();
    }
    else
    {
        m_Ptrs.pLine = new SaveLine(nullptr, *rBox.GetTabLines()[0], rSTable);

        SaveLine* pLn = m_Ptrs.pLine;
        for( size_t n = 1; n < rBox.GetTabLines().size(); ++n )
            pLn = new SaveLine( pLn, *rBox.GetTabLines()[ n ], rSTable );
    }
}

SaveBox::~SaveBox()
{
    if (NODE_OFFSET_MAX == m_nStartNode) // no EndBox
        delete m_Ptrs.pLine;
    else
        delete m_Ptrs.pContentAttrs;
    delete m_pNext;
}

void SaveBox::RestoreAttr( SwTableBox& rBox, SaveTable& rSTable )
{
    rSTable.NewFrameFormatForBox(rBox, m_nItemSet, rBox.GetFrameFormat());

    if (NODE_OFFSET_MAX == m_nStartNode) // no EndBox
    {
        if( rBox.GetTabLines().empty() )
        {
            OSL_ENSURE( false, "Number of lines changed" );
        }
        else
        {
            SaveLine* pLn = m_Ptrs.pLine;
            for (size_t n = 0; n < rBox.GetTabLines().size(); ++n, pLn = pLn->m_pNext)
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
    else if (rBox.GetSttNd() && rBox.GetSttIdx() == m_nStartNode)
    {
        if (m_Ptrs.pContentAttrs)
        {
            SwNodes& rNds = rBox.GetFrameFormat()->GetDoc()->GetNodes();
            sal_uInt16 nSet = 0;
            SwNodeOffset nEnd = rBox.GetSttNd()->EndOfSectionIndex();
            for (SwNodeOffset n = m_nStartNode + 1; n < nEnd; ++n)
            {
                SwContentNode* pCNd = rNds[ n ]->GetContentNode();
                if( pCNd )
                {
                    std::shared_ptr<SfxItemSet> pSet((*m_Ptrs.pContentAttrs)[nSet++]);
                    if( pSet )
                    {
                        for( const WhichPair& rPair : aSave_BoxContentSet )
                            pCNd->ResetAttr( rPair.first, rPair.second );
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

void SaveBox::SaveContentAttrs( SwDoc* pDoc )
{
    if (NODE_OFFSET_MAX == m_nStartNode) // no EndBox
    {
        // continue in current line
        m_Ptrs.pLine->SaveContentAttrs(pDoc);
    }
    else
    {
        SwNodeOffset nEnd = pDoc->GetNodes()[m_nStartNode]->EndOfSectionIndex();
        m_Ptrs.pContentAttrs = new SfxItemSets;
        for (SwNodeOffset n = m_nStartNode + 1; n < nEnd; ++n)
        {
            SwContentNode* pCNd = pDoc->GetNodes()[ n ]->GetContentNode();
            if( pCNd )
            {
                std::shared_ptr<SfxItemSet> pSet;
                if( pCNd->HasSwAttrSet() )
                {
                    pSet = std::make_shared<SfxItemSet>( pDoc->GetAttrPool(),
                                            aSave_BoxContentSet );
                    pSet->Put( *pCNd->GetpSwAttrSet() );
                }

                m_Ptrs.pContentAttrs->push_back(pSet);
            }
        }
    }
    if (m_pNext)
        m_pNext->SaveContentAttrs(pDoc);
}

void SaveBox::CreateNew( SwTable& rTable, SwTableLine& rParent, SaveTable& rSTable )
{
    SwTableBoxFormat* pFormat = static_cast<SwTableBoxFormat*>(rSTable.m_aFrameFormats[m_nItemSet]);
    if( !pFormat )
    {
        SwDoc* pDoc = rTable.GetFrameFormat()->GetDoc();
        pFormat = pDoc->MakeTableBoxFormat();
        pFormat->SetFormatAttr(*rSTable.m_aSets[m_nItemSet]);
        rSTable.m_aFrameFormats[m_nItemSet] = pFormat;
    }

    if (NODE_OFFSET_MAX == m_nStartNode) // no EndBox
    {
        SwTableBox* pNew = new SwTableBox( pFormat, 1, &rParent );
        rParent.GetTabBoxes().push_back( pNew );

        m_Ptrs.pLine->CreateNew(rTable, *pNew, rSTable);
    }
    else
    {
        // search box for StartNode in old table
        SwTableBox* pBox = rTable.GetTableBox(m_nStartNode);
        if (pBox)
        {
            SwFrameFormat* pOld = pBox->GetFrameFormat();
            pBox->RegisterToFormat( *pFormat );
            if( !pOld->HasWriterListeners() )
                delete pOld;

            pBox->setRowSpan(m_nRowSpan);

            SwTableBoxes* pTBoxes = &pBox->GetUpper()->GetTabBoxes();
            pTBoxes->erase( std::find( pTBoxes->begin(), pTBoxes->end(), pBox ) );

            pBox->SetUpper( &rParent );
            pTBoxes = &rParent.GetTabBoxes();
            pTBoxes->push_back( pBox );
        }
    }

    if (m_pNext)
        m_pNext->CreateNew(rTable, rParent, rSTable);
}

// UndoObject for attribute changes on table
SwUndoAttrTable::SwUndoAttrTable( const SwTableNode& rTableNd, bool bClearTabCols )
    : SwUndo( SwUndoId::TABLE_ATTR, &rTableNd.GetDoc() ),
    m_nStartNode( rTableNd.GetIndex() )
{
    m_bClearTableCol = bClearTabCols;
    m_pSaveTable.reset( new SaveTable( rTableNd.GetTable() ) );
}

SwUndoAttrTable::~SwUndoAttrTable()
{
}

void SwUndoAttrTable::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    SwTableNode* pTableNd = rDoc.GetNodes()[ m_nStartNode ]->GetTableNode();
    OSL_ENSURE( pTableNd, "no TableNode" );

    if (pTableNd)
    {
        SaveTable* pOrig = new SaveTable( pTableNd->GetTable() );
        m_pSaveTable->RestoreAttr( pTableNd->GetTable() );
        m_pSaveTable.reset( pOrig );
    }

    if( m_bClearTableCol )
    {
        ClearFEShellTabCols(rDoc, nullptr);
    }
}

void SwUndoAttrTable::RedoImpl(::sw::UndoRedoContext & rContext)
{
    UndoImpl(rContext);
}

// UndoObject for AutoFormat on Table
SwUndoTableAutoFormat::SwUndoTableAutoFormat( const SwTableNode& rTableNd,
                                    const SwTableAutoFormat& rAFormat )
    : SwUndo( SwUndoId::TABLE_AUTOFMT, &rTableNd.GetDoc() )
    , m_TableStyleName(rTableNd.GetTable().GetTableStyleName())
    , m_nStartNode( rTableNd.GetIndex() )
    , m_bSaveContentAttr( false )
    , m_nRepeatHeading(rTableNd.GetTable().GetRowsToRepeat())
{
    m_pSaveTable.reset( new SaveTable( rTableNd.GetTable() ) );

    if( rAFormat.IsFont() || rAFormat.IsJustify() )
    {
        // then also go over the ContentNodes of the EndBoxes and collect
        // all paragraph attributes
        m_pSaveTable->SaveContentAttrs( &const_cast<SwDoc&>(rTableNd.GetDoc()) );
        m_bSaveContentAttr = true;
    }
}

SwUndoTableAutoFormat::~SwUndoTableAutoFormat()
{
}

void SwUndoTableAutoFormat::SaveBoxContent( const SwTableBox& rBox )
{
    m_Undos.push_back(std::make_shared<SwUndoTableNumFormat>(rBox));
}

void
SwUndoTableAutoFormat::UndoRedo(bool const bUndo, ::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    SwTableNode* pTableNd = rDoc.GetNodes()[ m_nStartNode ]->GetTableNode();
    OSL_ENSURE( pTableNd, "no TableNode" );

    SwTable& table = pTableNd->GetTable();
    if (table.GetTableStyleName() != m_TableStyleName)
    {
        OUString const temp(table.GetTableStyleName());
        table.SetTableStyleName(m_TableStyleName);
        m_TableStyleName = temp;
    }
    SaveTable* pOrig = new SaveTable( table );
    // then go also over the ContentNodes of the EndBoxes and collect
    // all paragraph attributes
    if( m_bSaveContentAttr )
        pOrig->SaveContentAttrs( &rDoc );

    if (bUndo)
    {
        for (size_t n = m_Undos.size(); 0 < n; --n)
        {
            m_Undos.at(n-1)->UndoImpl(rContext);
        }

        table.SetRowsToRepeat(m_nRepeatHeading);
    }

    m_pSaveTable->RestoreAttr( pTableNd->GetTable(), !bUndo );
    m_pSaveTable.reset( pOrig );
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
                                    tools::Long nMn, tools::Long nMx,
                                    sal_uInt16 nCnt, bool bFlg, bool bSmHght )
    : SwUndo( nAction, &rTableNd.GetDoc() ),
    m_nMin( nMn ), m_nMax( nMx ),
    m_nSttNode( rTableNd.GetIndex() ),
    m_nCount( nCnt ),
    m_bFlag( bFlg ),
    m_bSameHeight( bSmHght )
{
    const SwTable& rTable = rTableNd.GetTable();
    m_pSaveTable.reset( new SaveTable( rTable ) );

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
}

void SwUndoTableNdsChg::SaveNewBoxes( const SwTableNode& rTableNd,
                                    const SwTableSortBoxes& rOld )
{
    const SwTable& rTable = rTableNd.GetTable();
    const SwTableSortBoxes& rTableBoxes = rTable.GetTabSortBoxes();

    OSL_ENSURE( ! IsDelBox(), "wrong Action" );
    m_xNewSttNds.emplace();

    size_t i = 0;
    for (size_t  n = 0; n < rOld.size(); ++i)
    {
        if( rOld[ n ] == rTableBoxes[ i ] )
            ++n;
        else
            // new box: insert sorted
            m_xNewSttNds->insert( BoxMove(rTableBoxes[ i ]->GetSttIdx()) );
    }

    for( ; i < rTableBoxes.size(); ++i )
        // new box: insert sorted
        m_xNewSttNds->insert( BoxMove(rTableBoxes[ i ]->GetSttIdx()) );
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
                                    const std::vector<SwNodeOffset> &rNodeCnts )
{
    const SwTable& rTable = rTableNd.GetTable();
    const SwTableSortBoxes& rTableBoxes = rTable.GetTabSortBoxes();

    OSL_ENSURE( ! IsDelBox(), "wrong Action" );
    m_xNewSttNds.emplace();

    OSL_ENSURE( rTable.IsNewModel() || rOld.size() + m_nCount * rBoxes.size() == rTableBoxes.size(),
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
            // No, if more than one selected cell in the same column has been split,
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
            OSL_ENSURE( pSourceBox, "Split source box not found!" );
            // find out how many nodes the source box used to have
            // (to help determine bNodesMoved flag below)
            size_t nNdsPos = 0;
            while( rBoxes[ nNdsPos ] != pSourceBox )
                ++nNdsPos;
            SwNodeOffset nNodes = rNodeCnts[ nNdsPos ];

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
                && ( nNodes - 1 > SwNodeOffset(nLineDiff) );
            m_xNewSttNds->insert( BoxMove(pBox->GetSttIdx(), bNodesMoved) );
        }
    }
}

void SwUndoTableNdsChg::SaveSection( SwStartNode* pSttNd )
{
    OSL_ENSURE( IsDelBox(), "wrong Action" );
    if (m_pDelSects == nullptr)
        m_pDelSects.reset(new SwUndoSaveSections);

    SwTableNode* pTableNd = pSttNd->FindTableNode();
    std::unique_ptr<SwUndoSaveSection, o3tl::default_delete<SwUndoSaveSection>> pSave(new SwUndoSaveSection);
    pSave->SaveSection( SwNodeIndex( *pSttNd ));

    m_pDelSects->push_back(std::move(pSave));
    m_nSttNode = pTableNd->GetIndex();
}

void SwUndoTableNdsChg::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    SwNodeIndex aIdx( rDoc.GetNodes(), m_nSttNode );

    SwTableNode *const pTableNd = aIdx.GetNode().GetTableNode();
    OSL_ENSURE( pTableNd, "no TableNode" );
    pTableNd->GetTable().SwitchFormulasToInternalRepresentation();

    CHECK_TABLE( pTableNd->GetTable() )

    FndBox_ aTmpBox( nullptr, nullptr );
    // ? TL_CHART2: notification or locking of controller required ?

    SwChartDataProvider *pPCD = rDoc.getIDocumentChartDataProviderAccess().GetChartDataProvider();
    SwSelBoxes aDelBoxes;
    std::vector< std::pair<SwTableBox *, SwNodeOffset> > aDelNodes;
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
    else if( !m_xNewSttNds->empty() )
    {
        // Then the nodes have be moved and not deleted!
        // But for that we need a temp array.
        std::vector<BoxMove> aTmp( m_xNewSttNds->begin(), m_xNewSttNds->end() );

        // backwards
        for (size_t n = aTmp.size(); n > 0 ; )
        {
            --n;
            // delete box from table structure
            SwNodeOffset nIdx = aTmp[n].index;
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
                SwNodeRange aRg( *pBox->GetSttNd(), SwNodeOffset(1),
                            *pBox->GetSttNd()->EndOfSectionNode() );

                SwTableLine* pLine = lcl_FindTableLine( pTableNd->GetTable(), *pBox );
                SwNodeIndex aInsPos( *(pLine->GetTabBoxes()[0]->GetSttNd()), 2 );

                // adjust all StartNode indices
                size_t i = n;
                SwNodeOffset nSttIdx = aInsPos.GetIndex() - 2,
                       nNdCnt = aRg.aEnd.GetIndex() - aRg.aStart.GetIndex();
                while( i && aTmp[ --i ].index > nSttIdx )
                    aTmp[ i ].index += nNdCnt;

                // first delete box
                delete pBox;
                // than move nodes
                rDoc.GetNodes().MoveNodes( aRg, rDoc.GetNodes(), aInsPos.GetNode(), false );
            }
            else
            {
                aDelNodes.emplace_back(pBox, nIdx);
            }
        }
    }
    else
    {
        // Remove nodes from nodes array (backwards!)
        std::set<BoxMove>::reverse_iterator it;
        for( it = m_xNewSttNds->rbegin(); it != m_xNewSttNds->rend(); ++it )
        {
            SwNodeOffset nIdx = (*it).index;
            SwTableBox* pBox = pTableNd->GetTable().GetTableBox( nIdx );
            OSL_ENSURE( pBox, "Where's my table box?" );
            // TL_CHART2: notify chart about box to be removed
            if (pPCD)
                pPCD->DeleteBox( &pTableNd->GetTable(), *pBox );
            aDelBoxes.insert(pBox);
            aDelNodes.emplace_back(pBox, nIdx);
        }
    }

    // fdo#57197: before deleting the SwTableBoxes, delete the SwTabFrames
    aTmpBox.SetTableLines(aDelBoxes, pTableNd->GetTable());
    aTmpBox.DelFrames(pTableNd->GetTable());

    // do this _after_ deleting Frames because disposing SwAccessible requires
    // connection to the nodes, see SwAccessibleChild::IsAccessible()
    for (const std::pair<SwTableBox *, SwNodeOffset> & rDelNode : aDelNodes)
    {
        // first disconnect box from node, otherwise ~SwTableBox would
        // access pBox->pSttNd, deleted by DeleteSection
        rDelNode.first->RemoveFromTable();
        rDoc.getIDocumentContentOperations().DeleteSection(rDoc.GetNodes()[ rDelNode.second ]);
    }

    // Remove boxes from table structure
    for( size_t n = 0; n < aDelBoxes.size(); ++n )
    {
        SwTableBox* pCurrBox = aDelBoxes[n];
        SwTableBoxes* pTBoxes = &pCurrBox->GetUpper()->GetTabBoxes();
        pTBoxes->erase( std::find( pTBoxes->begin(), pTBoxes->end(), pCurrBox ) );
        delete pCurrBox;
    }

    m_pSaveTable->CreateNew( pTableNd->GetTable(), true, false );

    // TL_CHART2: need to inform chart of probably changed cell names
    rDoc.UpdateCharts( pTableNd->GetTable().GetFrameFormat()->GetName() );
    if (SwFEShell* pFEShell = rDoc.GetDocShell()->GetFEShell())
        pFEShell->UpdateTableStyleFormatting(pTableNd);
    if( IsDelBox() )
        m_nSttNode = pTableNd->GetIndex();
    ClearFEShellTabCols(rDoc, nullptr);
    CHECK_TABLE( pTableNd->GetTable() )
}

void SwUndoTableNdsChg::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();

    SwTableNode* pTableNd = rDoc.GetNodes()[ m_nSttNode ]->GetTableNode();
    OSL_ENSURE( pTableNd, "no TableNode" );
    CHECK_TABLE( pTableNd->GetTable() )

    SwSelBoxes aSelBoxes;
    for (const auto& rBox : m_Boxes)
    {
        SwTableBox* pBox = pTableNd->GetTable().GetTableBox( rBox );
        aSelBoxes.insert( pBox );
    }

    // create SelBoxes and call InsertCell/-Row/SplitTable
    switch( GetId() )
    {
    case SwUndoId::TABLE_INSCOL:
        rDoc.InsertCol( aSelBoxes, m_nCount, m_bFlag );
        break;

    case SwUndoId::TABLE_INSROW:
        rDoc.InsertRow( aSelBoxes, m_nCount, m_bFlag );
        break;

    case SwUndoId::TABLE_SPLIT:
        rDoc.SplitTable( aSelBoxes, m_bFlag, m_nCount, m_bSameHeight );
        break;
    case SwUndoId::TABLE_DELBOX:
    case SwUndoId::ROW_DELETE:
    case SwUndoId::COL_DELETE:
        {
            SwTable &rTable = pTableNd->GetTable();
            rTable.SwitchFormulasToInternalRepresentation();
            if( m_nMax > m_nMin && rTable.IsNewModel() )
                rTable.PrepareDeleteCol( m_nMin, m_nMax );
            rTable.DeleteSel( &rDoc, aSelBoxes, nullptr, this, true, true );
            m_nSttNode = pTableNd->GetIndex();
        }
        break;
    default:
        ;
    }
    ClearFEShellTabCols(rDoc, nullptr);
    CHECK_TABLE( pTableNd->GetTable() )
}

SwUndoTableMerge::SwUndoTableMerge( const SwPaM& rTableSel )
    : SwUndo( SwUndoId::TABLE_MERGE, &rTableSel.GetDoc() ), SwUndRng( rTableSel )
{
    const SwTableNode* pTableNd = rTableSel.GetPointNode().FindTableNode();
    OSL_ENSURE( pTableNd, "Where is the TableNode?" );
    m_pSaveTable.reset( new SaveTable( pTableNd->GetTable() ) );
    m_nTableNode = pTableNd->GetIndex();
}

SwUndoTableMerge::~SwUndoTableMerge()
{
    m_pSaveTable.reset();
    m_vMoves.clear();
    m_pHistory.reset();
}

void SwUndoTableMerge::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    SwNodeIndex aIdx( rDoc.GetNodes(), m_nTableNode );

    SwTableNode *const pTableNd = aIdx.GetNode().GetTableNode();
    OSL_ENSURE( pTableNd, "no TableNode" );

    pTableNd->GetTable().SwitchFormulasToInternalRepresentation();

    // ? TL_CHART2: notification or locking of controller required ?

    // 1. restore deleted boxes:
    // Trick: add missing boxes in any line, they will be connected
    // correctly when calling CreateNew
    SwTableBox *pBox, *pCpyBox = pTableNd->GetTable().GetTabSortBoxes()[0];
    SwTableBoxes& rLnBoxes = pCpyBox->GetUpper()->GetTabBoxes();

    CHECKTABLE(pTableNd->GetTable())

    SwSelBoxes aSelBoxes;
    SwTextFormatColl* pColl = rDoc.getIDocumentStylePoolAccess().GetTextCollFromPool( RES_POOLCOLL_STANDARD );

    for (const auto& rBox : m_Boxes)
    {
        aIdx = rBox;
        SwStartNode* pSttNd = rDoc.GetNodes().MakeTextSection( aIdx.GetNode(),
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
    for( size_t n = m_aNewStartNodes.size(); n; )
    {
        // remove box from table structure
        SwNodeOffset nIdx = m_aNewStartNodes[ --n ];

        if( !nIdx && n )
        {
            nIdx = m_aNewStartNodes[ --n ];
            pBox = pTableNd->GetTable().GetTableBox( nIdx );
            OSL_ENSURE( pBox, "Where is my TableBox?" );

            if( !m_pSaveTable->IsNewModel() )
                rDoc.GetNodes().MakeTextNode(
                    const_cast<SwEndNode&>(*pBox->GetSttNd()->EndOfSectionNode()), pColl );

            // this was the separator -> restore moved ones
            for (size_t i = m_vMoves.size(); i; )
            {
                SwTextNode* pTextNd = nullptr;
                sal_Int32 nDelPos = 0;
                SwUndoMove *const pUndo = m_vMoves[ --i ].get();
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
                    SwContentIndex aTmpIdx( pTextNd, nDelPos );
                    if( pTextNd->GetpSwpHints() && pTextNd->GetpSwpHints()->Count() )
                        pTextNd->RstTextAttr( nDelPos, pTextNd->GetText().getLength() - nDelPos + 1 );
                    // delete separator
                    pTextNd->EraseText( aTmpIdx, 1 );
                }
            }
            nIdx = pBox->GetSttIdx();
        }
        else
            pBox = pTableNd->GetTable().GetTableBox( nIdx );

        if( !m_pSaveTable->IsNewModel() )
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
                            SwPosition( aTmpIdx, nullptr, 0 ), true );
            }

            delete pBox;
            rDoc.getIDocumentContentOperations().DeleteSection( rDoc.GetNodes()[ nIdx ] );
        }
    }
    CHECKTABLE(pTableNd->GetTable())

    m_pSaveTable->CreateNew( pTableNd->GetTable(), true, false );

    // TL_CHART2: need to inform chart of probably changed cell names
    rDoc.UpdateCharts( pTableNd->GetTable().GetFrameFormat()->GetName() );

    if( m_pHistory )
    {
        m_pHistory->TmpRollback( &rDoc, 0 );
        m_pHistory->SetTmpEnd( m_pHistory->Count() );
    }
    SwPaM *const pPam(& rContext.GetCursorSupplier().CreateNewShellCursor());
    pPam->DeleteMark();
    pPam->GetPoint()->Assign(m_nSttNode, m_nSttContent );
    pPam->SetMark();
    pPam->DeleteMark();

    CHECKTABLE(pTableNd->GetTable())
    ClearFEShellTabCols(rDoc, nullptr);
}

void SwUndoTableMerge::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    SwPaM & rPam( AddUndoRedoPaM(rContext) );
    rDoc.MergeTable(rPam);
}

void SwUndoTableMerge::MoveBoxContent( SwDoc& rDoc, SwNodeRange& rRg, SwNode& rPos )
{
    SwNodeIndex aTmp( rRg.aStart, -1 ), aTmp2( rPos, -1 );
    std::unique_ptr<SwUndoMove> pUndo(new SwUndoMove( rDoc, rRg, rPos ));
    ::sw::UndoGuard const undoGuard(rDoc.GetIDocumentUndoRedo());
    rDoc.getIDocumentContentOperations().MoveNodeRange( rRg, rPos, m_pSaveTable->IsNewModel() ?
        SwMoveFlags::NO_DELFRMS :
        SwMoveFlags::DEFAULT );
    ++aTmp;
    ++aTmp2;
    pUndo->SetDestRange( aTmp2.GetNode(), rPos, aTmp );

    m_vMoves.push_back(std::move(pUndo));
}

void SwUndoTableMerge::SetSelBoxes( const SwSelBoxes& rBoxes )
{
    // memorize selection
    for (size_t n = 0; n < rBoxes.size(); ++n)
    {
        m_Boxes.insert(rBoxes[n]->GetSttIdx());
    }

    // as separator for inserts of new boxes after shifting
    m_aNewStartNodes.push_back( SwNodeOffset(0) );

    // The new table model does not delete overlapped cells (by row span),
    // so the rBoxes array might be empty even some cells have been merged.
    if( !rBoxes.empty() )
        m_nTableNode = rBoxes[ 0 ]->GetSttNd()->FindTableNode()->GetIndex();
}

void SwUndoTableMerge::SaveCollection( const SwTableBox& rBox )
{
    if( !m_pHistory )
        m_pHistory.reset(new SwHistory);

    SwNodeIndex aIdx( *rBox.GetSttNd(), 1 );
    SwContentNode* pCNd = aIdx.GetNode().GetContentNode();
    if( !pCNd )
        pCNd = aIdx.GetNodes().GoNext( &aIdx );

    m_pHistory->AddColl(pCNd->GetFormatColl(), aIdx.GetIndex(), pCNd->GetNodeType());
    if( pCNd->HasSwAttrSet() )
        m_pHistory->CopyFormatAttr( *pCNd->GetpSwAttrSet(), aIdx.GetIndex() );
}

SwUndoTableNumFormat::SwUndoTableNumFormat( const SwTableBox& rBox,
                                    const SfxItemSet* pNewSet )
    : SwUndo(SwUndoId::TBLNUMFMT, rBox.GetFrameFormat()->GetDoc())
    , m_nFormatIdx(getSwDefaultTextFormat())
    , m_nNewFormatIdx(0)
    , m_fNum(0.0)
    , m_fNewNum(0.0)
    , m_bNewFormat(false)
    , m_bNewFormula(false)
    , m_bNewValue(false)
{
    m_nNode = rBox.GetSttIdx();

    m_nNodePos = rBox.IsValidNumTextNd( nullptr == pNewSet );
    SwDoc* pDoc = rBox.GetFrameFormat()->GetDoc();

    if( NODE_OFFSET_MAX != m_nNodePos )
    {
        SwTextNode* pTNd = pDoc->GetNodes()[ m_nNodePos ]->GetTextNode();

        m_pHistory.reset(new SwHistory);
        SwRegHistory aRHst( *rBox.GetSttNd(), m_pHistory.get() );
        // always save all text attributes because of possibly overlapping
        // areas of on/off
        m_pHistory->CopyAttr( pTNd->GetpSwpHints(), m_nNodePos, 0,
                            pTNd->GetText().getLength(), true );

        if( pTNd->HasSwAttrSet() )
            m_pHistory->CopyFormatAttr( *pTNd->GetpSwAttrSet(), m_nNodePos );

        m_aStr = pTNd->GetText();
        if( pTNd->GetpSwpHints() )
            pTNd->GetpSwpHints()->DeRegister();
    }

    m_pBoxSet.reset( new SfxItemSet( pDoc->GetAttrPool(), aTableBoxSetRange ) );
    m_pBoxSet->Put( rBox.GetFrameFormat()->GetAttrSet() );

    if( pNewSet )
    {
        if( const SwTableBoxNumFormat* pItem = pNewSet->GetItemIfSet( RES_BOXATR_FORMAT,
                false ))
        {
            m_bNewFormat = true;
            m_nNewFormatIdx = pItem->GetValue();
        }
        if( const SwTableBoxFormula* pItem = pNewSet->GetItemIfSet( RES_BOXATR_FORMULA,
                false ))
        {
            m_bNewFormula = true;
            m_aNewFormula = pItem->GetFormula();
        }
        if( const SwTableBoxValue* pItem = pNewSet->GetItemIfSet( RES_BOXATR_VALUE,
                false ))
        {
            m_bNewValue = true;
            m_fNewNum = pItem->GetValue();
        }
    }

    // is a history needed at all?
    if (m_pHistory && !m_pHistory->Count())
    {
        m_pHistory.reset();
    }
}

SwUndoTableNumFormat::~SwUndoTableNumFormat()
{
    m_pHistory.reset();
    m_pBoxSet.reset();
}

void SwUndoTableNumFormat::UndoImpl(::sw::UndoRedoContext & rContext)
{
    OSL_ENSURE( m_pBoxSet, "Where's the stored item set?" );

    SwDoc & rDoc = rContext.GetDoc();
    SwStartNode* pSttNd = rDoc.GetNodes()[ m_nNode ]->
                            FindSttNodeByType( SwTableBoxStartNode );
    OSL_ENSURE( pSttNd, "without StartNode no TableBox" );
    SwTableBox* pBox = pSttNd->FindTableNode()->GetTable().GetTableBox(
                                    pSttNd->GetIndex() );
    OSL_ENSURE( pBox, "found no TableBox" );

    SwTableBoxFormat* pFormat = rDoc.MakeTableBoxFormat();
    pFormat->SetFormatAttr( *m_pBoxSet );
    pBox->ChgFrameFormat( pFormat );

    if( NODE_OFFSET_MAX == m_nNodePos )
        return;

    SwTextNode* pTextNd = rDoc.GetNodes()[ m_nNodePos ]->GetTextNode();
    // If more than one node was deleted then all "node" attributes were also
    // saved
    if( pTextNd->HasSwAttrSet() )
        pTextNd->ResetAllAttr();

    if( pTextNd->GetpSwpHints() && !m_aStr.isEmpty() )
        pTextNd->ClearSwpHintsArr( true );

    // ChgTextToNum(..) only acts when the strings are different. We need to do
    // the same here.
    if( pTextNd->GetText() != m_aStr )
    {
        rDoc.getIDocumentRedlineAccess().DeleteRedline( *( pBox->GetSttNd() ), false, RedlineType::Any );

        SwContentIndex aIdx( pTextNd, 0 );
        if( !m_aStr.isEmpty() )
        {
            pTextNd->EraseText( aIdx );
            pTextNd->InsertText( m_aStr, aIdx,
                SwInsertFlags::NOHINTEXPAND );
        }
    }

    if( m_pHistory )
    {
        sal_uInt16 nTmpEnd = m_pHistory->GetTmpEnd();
        m_pHistory->TmpRollback( &rDoc, 0 );
        m_pHistory->SetTmpEnd( nTmpEnd );
    }

    SwPaM *const pPam(& rContext.GetCursorSupplier().CreateNewShellCursor());
    pPam->DeleteMark();
    pPam->GetPoint()->Assign( m_nNode + 1 );
}

namespace {

/** switch the RedlineFlags on the given document, using
 * SetRedlineFlags_intern. This class set the mode in the constructor,
 * and changes it back in the destructor, i.e. it uses the
 * initialization-is-resource-acquisition idiom.
 */
class RedlineFlagsInternGuard
{
    SwDoc& mrDoc;
    RedlineFlags meOldRedlineFlags;

public:
    RedlineFlagsInternGuard(
        SwDoc& rDoc,                      // change mode of this document
        RedlineFlags eNewRedlineFlags,    // new redline mode
        RedlineFlags eRedlineFlagsMask   /*change only bits set in this mask*/);

    ~RedlineFlagsInternGuard();
};

}

RedlineFlagsInternGuard::RedlineFlagsInternGuard(
    SwDoc& rDoc,
    RedlineFlags eNewRedlineFlags,
    RedlineFlags eRedlineFlagsMask )
    : mrDoc( rDoc ),
      meOldRedlineFlags( rDoc.getIDocumentRedlineAccess().GetRedlineFlags() )
{
    mrDoc.getIDocumentRedlineAccess().SetRedlineFlags_intern( ( meOldRedlineFlags & ~eRedlineFlagsMask ) |
                                     ( eNewRedlineFlags & eRedlineFlagsMask ) );
}

RedlineFlagsInternGuard::~RedlineFlagsInternGuard()
{
    mrDoc.getIDocumentRedlineAccess().SetRedlineFlags_intern( meOldRedlineFlags );
}

void SwUndoTableNumFormat::RedoImpl(::sw::UndoRedoContext & rContext)
{
    // Could the box be changed?
    if( !m_pBoxSet )
        return ;

    SwDoc & rDoc = rContext.GetDoc();
    SwPaM *const pPam(& rContext.GetCursorSupplier().CreateNewShellCursor());

    pPam->DeleteMark();
    pPam->GetPoint()->Assign( m_nNode );

    SwNode * pNd = & pPam->GetPoint()->GetNode();
    SwStartNode* pSttNd = pNd->FindSttNodeByType( SwTableBoxStartNode );
    assert(pSttNd && "without StartNode no TableBox");
    SwTableBox* pBox = pSttNd->FindTableNode()->GetTable().GetTableBox(
                                    pSttNd->GetIndex() );
    OSL_ENSURE( pBox, "found no TableBox" );

    SwFrameFormat* pBoxFormat = pBox->ClaimFrameFormat();
    if( m_bNewFormat || m_bNewFormula || m_bNewValue )
    {
        SfxItemSetFixed<RES_BOXATR_FORMAT, RES_BOXATR_VALUE> aBoxSet( rDoc.GetAttrPool() );

        // Resetting attributes is not enough. In addition, take care that the
        // text will be also formatted correctly.
        pBoxFormat->LockModify();

        if( m_bNewFormula )
            aBoxSet.Put( SwTableBoxFormula( m_aNewFormula ));
        else
            pBoxFormat->ResetFormatAttr( RES_BOXATR_FORMULA );
        if( m_bNewFormat )
            aBoxSet.Put( SwTableBoxNumFormat( m_nNewFormatIdx ));
        else
            pBoxFormat->ResetFormatAttr( RES_BOXATR_FORMAT );
        if( m_bNewValue )
            aBoxSet.Put( SwTableBoxValue( m_fNewNum ));
        else
            pBoxFormat->ResetFormatAttr( RES_BOXATR_VALUE );
        pBoxFormat->UnlockModify();

        // dvo: When redlining is (was) enabled, setting the attribute
        // will also change the cell content. To allow this, the
        // RedlineFlags::Ignore flag must be removed during Redo. #108450#
        RedlineFlagsInternGuard aGuard( rDoc, RedlineFlags::NONE, RedlineFlags::Ignore );
        pBoxFormat->SetFormatAttr( aBoxSet );
    }
    else if( getSwDefaultTextFormat() != m_nFormatIdx )
    {
        SfxItemSetFixed<RES_BOXATR_FORMAT, RES_BOXATR_VALUE> aBoxSet( rDoc.GetAttrPool() );

        aBoxSet.Put( SwTableBoxNumFormat( m_nFormatIdx ));
        aBoxSet.Put( SwTableBoxValue( m_fNum ));

        // Resetting attributes is not enough. In addition, take care that the
        // text will be also formatted correctly.
        pBoxFormat->LockModify();
        pBoxFormat->ResetFormatAttr( RES_BOXATR_FORMULA );
        pBoxFormat->UnlockModify();

        // dvo: When redlining is (was) enabled, setting the attribute
        // will also change the cell content. To allow this, the
        // RedlineFlags::Ignore flag must be removed during Redo. #108450#
        RedlineFlagsInternGuard aGuard( rDoc, RedlineFlags::NONE, RedlineFlags::Ignore );
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

    if( m_bNewFormula )
    {
        // No matter what was set, an update of the table is always a good idea
        rDoc.getIDocumentFieldsAccess().UpdateTableFields(&pSttNd->FindTableNode()->GetTable());
    }

    if( !pNd->IsContentNode() )
        pNd = rDoc.GetNodes().GoNext( pPam->GetPoint() );
}

void SwUndoTableNumFormat::SetBox( const SwTableBox& rBox )
{
    m_nNode = rBox.GetSttIdx();
}

UndoTableCpyTable_Entry::UndoTableCpyTable_Entry( const SwTableBox& rBox )
    : nBoxIdx( rBox.GetSttIdx() ), nOffset( 0 ),
    bJoin( false )
{
}

void UndoTableCpyTable_Entry::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("UndoTableCpyTable_Entry"));

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("nBoxIdx"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::number(sal_Int32(nBoxIdx)).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("nOffset"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::number(sal_Int32(nOffset)).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    if (pBoxNumAttr)
    {
        (void)xmlTextWriterStartElement(pWriter, BAD_CAST("pBoxNumAttr"));
        pBoxNumAttr->dumpAsXml(pWriter);
        (void)xmlTextWriterEndElement(pWriter);
    }

    if (pUndo)
    {
        (void)xmlTextWriterStartElement(pWriter, BAD_CAST("pUndo"));
        pUndo->dumpAsXml(pWriter);
        (void)xmlTextWriterEndElement(pWriter);
    }

    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("bJoin"));
    (void)xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"),
                                BAD_CAST(OString::boolean(bJoin).getStr()));
    (void)xmlTextWriterEndElement(pWriter);

    (void)xmlTextWriterEndElement(pWriter);
}

SwUndoTableCpyTable::SwUndoTableCpyTable(const SwDoc& rDoc)
    : SwUndo( SwUndoId::TBLCPYTBL, &rDoc )
{
}

SwUndoTableCpyTable::~SwUndoTableCpyTable()
{
    m_vArr.clear();
    m_pInsRowUndo.reset();
}

void SwUndoTableCpyTable::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    DEBUG_REDLINE( &rDoc )

    SwTableNode* pTableNd = nullptr;
    for (size_t n = m_vArr.size(); n; )
    {
        UndoTableCpyTable_Entry *const pEntry = m_vArr[ --n ].get();
        SwNodeOffset nSttPos = pEntry->nBoxIdx + pEntry->nOffset;
        SwStartNode* pSNd = rDoc.GetNodes()[ nSttPos ]->StartOfSectionNode();
        if( !pTableNd )
            pTableNd = pSNd->FindTableNode();

        SwTableBox* pBox = pTableNd->GetTable().GetTableBox( nSttPos );
        if (!pBox)
        {
            SAL_WARN("sw.core",
                     "SwUndoTableCpyTable::UndoImpl: invalid start node index for table box");
            continue;
        }

        SwTableBox& rBox = *pBox;

        SwNodeIndex aInsIdx( *rBox.GetSttNd(), 1 );
        rDoc.GetNodes().MakeTextNode( aInsIdx.GetNode(), rDoc.GetDfltTextFormatColl() );

        // b62341295: Redline for copying tables
        const SwNode *pEndNode = rBox.GetSttNd()->EndOfSectionNode();
        SwPaM aPam( aInsIdx.GetNode(), *pEndNode );
        std::unique_ptr<SwUndoDelete> pUndo;

        if( IDocumentRedlineAccess::IsRedlineOn( GetRedlineFlags() ) )
        {
            bool bDeleteCompleteParagraph = false;
            bool bShiftPam = false;
            // There are a couple of different situations to consider during redlining
            if( pEntry->pUndo )
            {
                SwUndoDelete *const pUndoDelete =
                    dynamic_cast<SwUndoDelete*>(pEntry->pUndo.get());
                SwUndoRedlineDelete *const pUndoRedlineDelete =
                    dynamic_cast<SwUndoRedlineDelete*>(pEntry->pUndo.get());
                assert(pUndoDelete || pUndoRedlineDelete);
                if (pUndoRedlineDelete)
                {
                    // The old content was not empty or he has been merged with the new content
                    bDeleteCompleteParagraph = !pEntry->bJoin; // bJoin is set when merged
                    // Set aTmpIdx to the beginning of the old content
                    SwNodeIndex aTmpIdx( *pEndNode,
                            pUndoRedlineDelete->NodeDiff()-1 );
                    SwTextNode *pText = aTmpIdx.GetNode().GetTextNode();
                    if( pText )
                    {
                        aPam.GetPoint()->Assign(*pText,
                                pUndoRedlineDelete->ContentStart() );
                    }
                    else
                        aPam.GetPoint()->Assign( aTmpIdx );
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
                    aPam.GetPoint()->Assign(*pEndNode, SwNodeOffset(-1));
                }
            }
            rDoc.getIDocumentRedlineAccess().DeleteRedline( aPam, true, RedlineType::Any );

            if( pEntry->pUndo )
            {
                pEntry->pUndo->UndoImpl(rContext);
                pEntry->pUndo.reset();
            }
            if( bShiftPam )
            {
                // The aPam.Point is at the moment at the last position of the new content and has to be
                // moved to the first position of the old content for the SwUndoDelete operation
                aPam.GetPoint()->Assign(aPam.GetPoint()->GetNode(), SwNodeOffset(1));
            }
            pUndo = std::make_unique<SwUndoDelete>(aPam, SwDeleteFlags::Default, bDeleteCompleteParagraph, true);
        }
        else
        {
            pUndo = std::make_unique<SwUndoDelete>(aPam, SwDeleteFlags::Default, true);
            if( pEntry->pUndo )
            {
                pEntry->pUndo->UndoImpl(rContext);
                pEntry->pUndo.reset();
            }
        }
        pEntry->pUndo = std::move(pUndo);

        aInsIdx = rBox.GetSttIdx() + 1;
        rDoc.GetNodes().Delete( aInsIdx );

        SfxItemSetFixed<
                RES_VERT_ORIENT, RES_VERT_ORIENT,
                RES_BOXATR_FORMAT, RES_BOXATR_VALUE>
             aTmpSet(rDoc.GetAttrPool());
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
            pEntry->pBoxNumAttr.reset();
        }

        if( aTmpSet.Count() )
        {
            pEntry->pBoxNumAttr = std::make_unique<SfxItemSetFixed<
                    RES_VERT_ORIENT, RES_VERT_ORIENT,
                    RES_BOXATR_FORMAT, RES_BOXATR_VALUE>>(rDoc.GetAttrPool());
            pEntry->pBoxNumAttr->Put( aTmpSet );
        }

        pEntry->nOffset = rBox.GetSttIdx() - pEntry->nBoxIdx;
    }

    if( m_pInsRowUndo )
    {
        m_pInsRowUndo->UndoImpl(rContext);
    }
    DEBUG_REDLINE( &rDoc )
}

void SwUndoTableCpyTable::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    DEBUG_REDLINE( &rDoc )

    if( m_pInsRowUndo )
    {
        m_pInsRowUndo->RedoImpl(rContext);
    }

    SwTableNode* pTableNd = nullptr;
    for (size_t n = 0; n < m_vArr.size(); ++n)
    {
        UndoTableCpyTable_Entry *const pEntry = m_vArr[ n ].get();
        SwNodeOffset nSttPos = pEntry->nBoxIdx + pEntry->nOffset;
        SwStartNode* pSNd = rDoc.GetNodes()[ nSttPos ]->StartOfSectionNode();
        if( !pTableNd )
            pTableNd = pSNd->FindTableNode();

        SwTableBox& rBox = *pTableNd->GetTable().GetTableBox( nSttPos );

        SwNodeIndex aInsIdx( *rBox.GetSttNd(), 1 );

        // b62341295: Redline for copying tables - Start.
        rDoc.GetNodes().MakeTextNode( aInsIdx.GetNode(), rDoc.GetDfltTextFormatColl() );
        SwPaM aPam( aInsIdx.GetNode(), *rBox.GetSttNd()->EndOfSectionNode());
        std::unique_ptr<SwUndo> pUndo(IDocumentRedlineAccess::IsRedlineOn(GetRedlineFlags())
            ? nullptr
            : std::make_unique<SwUndoDelete>(aPam, SwDeleteFlags::Default, true));
        if( pEntry->pUndo )
        {
            pEntry->pUndo->UndoImpl(rContext);
            if( IDocumentRedlineAccess::IsRedlineOn( GetRedlineFlags() ) )
            {
                // PrepareRedline has to be called with the beginning of the old content
                // When new and old content has been joined, the rIter.pAktPam has been set
                // by the Undo operation to this point.
                // Otherwise aInsIdx has been moved during the Undo operation
                if( pEntry->bJoin )
                {
                    SwPaM& rLastPam =
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
            pEntry->pUndo.reset();
        }
        pEntry->pUndo = std::move(pUndo);
        // b62341295: Redline for copying tables - End.

        aInsIdx = rBox.GetSttIdx() + 1;
        rDoc.GetNodes().Delete( aInsIdx );

        SfxItemSetFixed<
                RES_VERT_ORIENT, RES_VERT_ORIENT,
                RES_BOXATR_FORMAT, RES_BOXATR_VALUE> aTmpSet(rDoc.GetAttrPool());
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
            pEntry->pBoxNumAttr.reset();
        }

        if( aTmpSet.Count() )
        {
            pEntry->pBoxNumAttr = std::make_unique<SfxItemSetFixed<
                    RES_VERT_ORIENT, RES_VERT_ORIENT,
                    RES_BOXATR_FORMAT, RES_BOXATR_VALUE>>(rDoc.GetAttrPool());
            pEntry->pBoxNumAttr->Put( aTmpSet );
        }

        pEntry->nOffset = rBox.GetSttIdx() - pEntry->nBoxIdx;
    }
    DEBUG_REDLINE( &rDoc )
}

void SwUndoTableCpyTable::AddBoxBefore( const SwTableBox& rBox, bool bDelContent )
{
    if (!m_vArr.empty() && !bDelContent)
        return;

    UndoTableCpyTable_Entry* pEntry = new UndoTableCpyTable_Entry( rBox );
    m_vArr.push_back(std::unique_ptr<UndoTableCpyTable_Entry>(pEntry));

    SwDoc* pDoc = rBox.GetFrameFormat()->GetDoc();
    DEBUG_REDLINE( pDoc )
    if( bDelContent )
    {
        SwNodeIndex aInsIdx( *rBox.GetSttNd(), 1 );
        SwTextNode *const pNewNode(pDoc->GetNodes().MakeTextNode(aInsIdx.GetNode(), pDoc->GetDfltTextFormatColl()));
        SwPaM aPam( aInsIdx.GetNode(), *rBox.GetSttNd()->EndOfSectionNode() );

        if( !pDoc->getIDocumentRedlineAccess().IsRedlineOn() )
        {
            {   // move cursors to new node which precedes aPam
                SwPosition const pos(*pNewNode, 0);
                ::PaMCorrAbs(aPam, pos);
            }
            pEntry->pUndo = std::make_unique<SwUndoDelete>(aPam, SwDeleteFlags::Default, true);
        }
    }

    pEntry->pBoxNumAttr = std::make_unique<SfxItemSetFixed<
            RES_VERT_ORIENT, RES_VERT_ORIENT,
            RES_BOXATR_FORMAT, RES_BOXATR_VALUE>>(pDoc->GetAttrPool());
    pEntry->pBoxNumAttr->Put( rBox.GetFrameFormat()->GetAttrSet() );
    if( !pEntry->pBoxNumAttr->Count() )
    {
        pEntry->pBoxNumAttr.reset();
    }
    DEBUG_REDLINE( pDoc )
}

void SwUndoTableCpyTable::AddBoxAfter( const SwTableBox& rBox, const SwNodeIndex& rIdx, bool bDelContent )
{
    UndoTableCpyTable_Entry *const pEntry = m_vArr.back().get();

    // If the content was deleted then remove also the temporarily created node
    if( bDelContent )
    {
        SwDoc* pDoc = rBox.GetFrameFormat()->GetDoc();
        DEBUG_REDLINE( pDoc )

        {   // move cursors to first node which was inserted
            SwPaM pam(SwNodeIndex(*rBox.GetSttNd(), 1));
            assert(pam.GetPoint()->GetNode().IsTextNode());
            pam.SetMark();
            pam.Move(fnMoveForward, GoInContent);
            ::PaMCorrAbs(pam, *pam.GetPoint());
        }
        if( pDoc->getIDocumentRedlineAccess().IsRedlineOn() )
        {
            SwPosition aTmpPos( rIdx );
            pEntry->pUndo = PrepareRedline( pDoc, rBox, aTmpPos, pEntry->bJoin, false );
        }
        SwNodeIndex aDelIdx( *rBox.GetSttNd(), 1 );
        rBox.GetFrameFormat()->GetDoc()->GetNodes().Delete( aDelIdx );
        DEBUG_REDLINE( pDoc )
    }

    pEntry->nOffset = rBox.GetSttIdx() - pEntry->nBoxIdx;
}

// PrepareRedline is called from AddBoxAfter() and from Redo() in slightly different situations.
// bRedo is set by calling from Redo()
// rJoin is false by calling from AddBoxAfter() and will be set if the old and new content has
// been merged.
// rJoin is true if Redo() is calling and the content has already been merged

std::unique_ptr<SwUndo> SwUndoTableCpyTable::PrepareRedline( SwDoc* pDoc, const SwTableBox& rBox,
    SwPosition& rPos, bool& rJoin, bool bRedo )
{
    std::unique_ptr<SwUndo> pUndo;
    // b62341295: Redline for copying tables
    // What's to do?
    // Mark the cell content before rIdx as insertion,
    // mark the cell content behind rIdx as deletion
    // merge text nodes at rIdx if possible
    RedlineFlags eOld = pDoc->getIDocumentRedlineAccess().GetRedlineFlags();
    pDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( ( eOld | RedlineFlags::DontCombineRedlines ) & ~RedlineFlags::Ignore );
    SwPosition aInsertEnd( rPos );
    SwTextNode* pText;
    if( !rJoin )
    {
        // If the content is not merged, the end of the insertion is at the end of the node
        // _before_ the given position rPos
        aInsertEnd.Adjust(SwNodeOffset(-1));
        pText = aInsertEnd.GetNode().GetTextNode();
        if( pText )
        {
            aInsertEnd.SetContent(pText->GetText().getLength());
            if( !bRedo && rPos.GetNode().GetTextNode() )
            {   // Try to merge, if not called by Redo()
                rJoin = true;

                // Park this somewhere else so nothing points to the to-be-deleted node.
                rPos.nContent.Assign(pText, 0);

                pText->JoinNext();
            }
        }
        else
            aInsertEnd.nContent.Assign(nullptr, 0);
    }
    // For joined (merged) contents the start of deletion and end of insertion are identical
    // otherwise adjacent nodes.
    SwPosition aDeleteStart( rJoin ? aInsertEnd : rPos );
    if( !rJoin )
    {
        pText = aDeleteStart.GetNode().GetTextNode();
        if( pText )
            aDeleteStart.SetContent( 0 );
    }
    SwPosition aCellEnd( *rBox.GetSttNd()->EndOfSectionNode(), SwNodeOffset(-1) );
    pText = aCellEnd.GetNode().GetTextNode();
    if( pText )
        aCellEnd.SetContent(pText->GetText().getLength());
    if( aDeleteStart != aCellEnd )
    {   // If the old (deleted) part is not empty, here we are...
        SwPaM aDeletePam( aDeleteStart, aCellEnd );
        pUndo = std::make_unique<SwUndoRedlineDelete>( aDeletePam, SwUndoId::DELETE );
        pDoc->getIDocumentRedlineAccess().AppendRedline( new SwRangeRedline( RedlineType::Delete, aDeletePam ), true );
    }
    else if( !rJoin ) // If the old part is empty and joined, we are finished
    {   // if it is not joined, we have to delete this empty paragraph
        aCellEnd.Assign(*rBox.GetSttNd()->EndOfSectionNode());
        SwPaM aTmpPam( aDeleteStart, aCellEnd );
        pUndo = std::make_unique<SwUndoDelete>(aTmpPam, SwDeleteFlags::Default, true);
    }
    SwPosition aCellStart( *rBox.GetSttNd(), SwNodeOffset(2) );
    pText = aCellStart.GetNode().GetTextNode();
    if( pText )
        aCellStart.SetContent( 0 );
    if( aCellStart != aInsertEnd ) // An empty insertion will not been marked
    {
        SwPaM aTmpPam( aCellStart, aInsertEnd );
        pDoc->getIDocumentRedlineAccess().AppendRedline( new SwRangeRedline( RedlineType::Insert, aTmpPam ), true );
    }

    pDoc->getIDocumentRedlineAccess().SetRedlineFlags_intern( eOld );
    return pUndo;
}

bool SwUndoTableCpyTable::InsertRow( SwTable& rTable, const SwSelBoxes& rBoxes,
                                sal_uInt16 nCnt )
{
    SwTableNode* pTableNd = const_cast<SwTableNode*>(rTable.GetTabSortBoxes()[0]->
                                GetSttNd()->FindTableNode());

    m_pInsRowUndo.reset( new SwUndoTableNdsChg( SwUndoId::TABLE_INSROW, rBoxes, *pTableNd,
                                       0, 0, nCnt, true, false ) );
    SwTableSortBoxes aTmpLst( rTable.GetTabSortBoxes() );

    bool bRet = rTable.InsertRow( rTable.GetFrameFormat()->GetDoc(), rBoxes, nCnt, /*bBehind*/true );
    if( bRet )
        m_pInsRowUndo->SaveNewBoxes( *pTableNd, aTmpLst );
    else
    {
        m_pInsRowUndo.reset();
    }
    return bRet;
}

void SwUndoTableCpyTable::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    (void)xmlTextWriterStartElement(pWriter, BAD_CAST("SwUndoTableCpyTable"));

    for (const auto& pEntry : m_vArr)
    {
        pEntry->dumpAsXml(pWriter);
    }

    if (m_pInsRowUndo)
    {
        m_pInsRowUndo->dumpAsXml(pWriter);
    }

    (void)xmlTextWriterEndElement(pWriter);
}

bool SwUndoTableCpyTable::IsEmpty() const
{
    return !m_pInsRowUndo && m_vArr.empty();
}

SwUndoCpyTable::SwUndoCpyTable(const SwDoc& rDoc)
    : SwUndo( SwUndoId::CPYTBL, &rDoc ), m_nTableNode( 0 )
{
}

SwUndoCpyTable::~SwUndoCpyTable()
{
}

void SwUndoCpyTable::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc & rDoc = rContext.GetDoc();
    SwTableNode* pTNd = rDoc.GetNodes()[ m_nTableNode ]->GetTableNode();

    // move hard page breaks into next node
    SwContentNode* pNextNd = rDoc.GetNodes()[ pTNd->EndOfSectionIndex()+1 ]->GetContentNode();
    if( pNextNd )
    {
        SwFrameFormat* pTableFormat = pTNd->GetTable().GetFrameFormat();

        if( const SwFormatPageDesc* pItem = pTableFormat->GetItemIfSet( RES_PAGEDESC,
            false ) )
            pNextNd->SetAttr( *pItem );

        if( const SvxFormatBreakItem* pItem = pTableFormat->GetItemIfSet( RES_BREAK,
            false ) )
            pNextNd->SetAttr( *pItem );
    }

    SwPaM aPam( *pTNd, *pTNd->EndOfSectionNode(), SwNodeOffset(0) , SwNodeOffset(1) );
    m_pDelete.reset(new SwUndoDelete(aPam, SwDeleteFlags::Default, true));
}

void SwUndoCpyTable::RedoImpl(::sw::UndoRedoContext & rContext)
{
    m_pDelete->UndoImpl(rContext);
    m_pDelete.reset();
}

SwUndoSplitTable::SwUndoSplitTable( const SwTableNode& rTableNd,
    std::unique_ptr<SwSaveRowSpan> pRowSp, SplitTable_HeadlineOption eMode, bool bNewSize )
    : SwUndo( SwUndoId::SPLIT_TABLE, &rTableNd.GetDoc() ),
    m_nTableNode( rTableNd.GetIndex() ), m_nOffset( 0 ), mpSaveRowSpan( std::move(pRowSp) ),
    m_nMode( eMode ), m_nFormulaEnd( 0 ), m_bCalcNewSize( bNewSize )
{
    switch( m_nMode )
    {
    case SplitTable_HeadlineOption::BoxAttrAllCopy:
            m_pHistory.reset(new SwHistory);
            [[fallthrough]];
    case SplitTable_HeadlineOption::BorderCopy:
    case SplitTable_HeadlineOption::BoxAttrCopy:
        m_pSavedTable.reset(new SaveTable( rTableNd.GetTable(), 1, false ));
        break;
    default: break;
    }
}

SwUndoSplitTable::~SwUndoSplitTable()
{
    m_pSavedTable.reset();
    m_pHistory.reset();
    mpSaveRowSpan.reset();
}

void SwUndoSplitTable::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc *const pDoc = & rContext.GetDoc();
    SwPaM *const pPam(& rContext.GetCursorSupplier().CreateNewShellCursor());

    SwPosition& rPtPos = *pPam->GetPoint();
    rPtPos.Assign( m_nTableNode + m_nOffset );
    assert(rPtPos.GetNode().GetContentNode()->Len() == 0); // empty para inserted

    {
        // avoid asserts from ~SwContentIndexReg
        SwNodeIndex const idx(pDoc->GetNodes(), m_nTableNode + m_nOffset);
        {
            SwPaM pam(idx);
            pam.Move(fnMoveBackward, GoInContent);
            ::PaMCorrAbs(*pPam, *pam.GetPoint());
        }

        // remove implicitly created paragraph again
        pDoc->GetNodes().Delete( idx );
    }

    rPtPos.Assign( m_nTableNode + m_nOffset );
    SwTableNode* pTableNd = rPtPos.GetNode().GetTableNode();
    SwTable& rTable = pTableNd->GetTable();
    rTable.SwitchFormulasToInternalRepresentation();

    switch( m_nMode )
    {
    case SplitTable_HeadlineOption::BoxAttrAllCopy:
        if( m_pHistory )
            m_pHistory->TmpRollback( pDoc, m_nFormulaEnd );
        [[fallthrough]];
    case SplitTable_HeadlineOption::BoxAttrCopy:
    case SplitTable_HeadlineOption::BorderCopy:
        {
            m_pSavedTable->CreateNew( rTable, false );
            m_pSavedTable->RestoreAttr( rTable );
        }
        break;

    case SplitTable_HeadlineOption::ContentCopy:
        // the created first line has to be removed again
        {
            SwSelBoxes aSelBoxes;
            SwTableBox* pBox = rTable.GetTableBox( m_nTableNode + m_nOffset + 1 );
            SwTable::SelLineFromBox( pBox, aSelBoxes );
            FndBox_ aTmpBox( nullptr, nullptr );
            aTmpBox.SetTableLines( aSelBoxes, rTable );
            aTmpBox.DelFrames( rTable );
            rTable.DeleteSel( pDoc, aSelBoxes, nullptr, nullptr, false, false );
        }
        break;
    default: break;
    }

    pDoc->GetNodes().MergeTable( rPtPos.GetNode() );

    if( m_pHistory )
    {
        m_pHistory->TmpRollback( pDoc, 0 );
        m_pHistory->SetTmpEnd( m_pHistory->Count() );
    }
    if( mpSaveRowSpan )
    {
        pTableNd = rPtPos.GetNode().FindTableNode();
        if( pTableNd )
            pTableNd->GetTable().RestoreRowSpan( *mpSaveRowSpan );
    }
    ClearFEShellTabCols(*pDoc, nullptr);
}

void SwUndoSplitTable::RedoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc *const pDoc = & rContext.GetDoc();
    SwPaM *const pPam(& rContext.GetCursorSupplier().CreateNewShellCursor());

    pPam->DeleteMark();
    pPam->GetPoint()->Assign( m_nTableNode );
    pDoc->SplitTable( *pPam->GetPoint(), m_nMode, m_bCalcNewSize );

    ClearFEShellTabCols(*pDoc, nullptr);
}

void SwUndoSplitTable::RepeatImpl(::sw::RepeatContext & rContext)
{
    SwPaM *const pPam = & rContext.GetRepeatPaM();
    SwDoc *const pDoc = & rContext.GetDoc();

    pDoc->SplitTable( *pPam->GetPoint(), m_nMode, m_bCalcNewSize );
    ClearFEShellTabCols(*pDoc, nullptr);
}

void SwUndoSplitTable::SaveFormula( SwHistory& rHistory )
{
    if( !m_pHistory )
        m_pHistory.reset(new SwHistory);

    m_nFormulaEnd = rHistory.Count();
    m_pHistory->Move( 0, &rHistory );
}

SwUndoMergeTable::SwUndoMergeTable( const SwTableNode& rTableNd,
                                const SwTableNode& rDelTableNd,
                                bool bWithPrv )
    : SwUndo( SwUndoId::MERGE_TABLE, &rTableNd.GetDoc() ),
    m_bWithPrev( bWithPrv )
{
    // memorize end node of the last table cell that'll stay in position
    if( m_bWithPrev )
        m_nTableNode = rDelTableNd.EndOfSectionIndex() - 1;
    else
        m_nTableNode = rTableNd.EndOfSectionIndex() - 1;

    m_aName = rDelTableNd.GetTable().GetFrameFormat()->GetName();
    m_pSaveTable.reset(new SaveTable( rDelTableNd.GetTable() ));

    if (m_bWithPrev)
        m_pSaveHdl.reset( new SaveTable( rTableNd.GetTable(), 1 ) );
}

SwUndoMergeTable::~SwUndoMergeTable()
{
    m_pSaveTable.reset();
    m_pSaveHdl.reset();
    m_pHistory.reset();
}

void SwUndoMergeTable::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwDoc *const pDoc = & rContext.GetDoc();
    SwPaM *const pPam(& rContext.GetCursorSupplier().CreateNewShellCursor());

    pPam->DeleteMark();
    SwPosition& rPtPos = *pPam->GetPoint();
    rPtPos.Assign( m_nTableNode);

    SwTableNode* pTableNd = rPtPos.GetNode().FindTableNode();
    SwTable* pTable = &pTableNd->GetTable();
    pTable->SwitchFormulasToInternalRepresentation();

    // get lines for layout update
    FndBox_ aFndBox( nullptr, nullptr );
    aFndBox.SetTableLines( *pTable );
    aFndBox.DelFrames( *pTable );
    // ? TL_CHART2: notification or locking of controller required ?

    SwTableNode* pNew = pDoc->GetNodes().SplitTable( rPtPos.GetNode() );

    // update layout
    aFndBox.MakeFrames( *pTable );
    // ? TL_CHART2: notification or locking of controller required ?

    if( m_bWithPrev )
    {
        // move name
        pNew->GetTable().GetFrameFormat()->SetFormatName( pTable->GetFrameFormat()->GetName() );
        m_pSaveHdl->RestoreAttr( pNew->GetTable() );
    }
    else
        pTable = &pNew->GetTable();

    pTable->GetFrameFormat()->SetFormatName( m_aName );
    m_pSaveTable->RestoreAttr( *pTable );

    if( m_pHistory )
    {
        m_pHistory->TmpRollback( pDoc, 0 );
        m_pHistory->SetTmpEnd( m_pHistory->Count() );
    }

    // create frames for the new table
    pNew->MakeOwnFrames();

    // position cursor somewhere in content
    pDoc->GetNodes().GoNext( &rPtPos );

    ClearFEShellTabCols(*pDoc, nullptr);

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
    if( m_bWithPrev )
        pPam->GetPoint()->Assign( m_nTableNode + 3 );
    else
        pPam->GetPoint()->Assign( m_nTableNode );

    pDoc->MergeTable( *pPam->GetPoint(), m_bWithPrev );

    ClearFEShellTabCols(*pDoc, nullptr);
}

void SwUndoMergeTable::RepeatImpl(::sw::RepeatContext & rContext)
{
    SwDoc *const pDoc = & rContext.GetDoc();
    SwPaM *const pPam = & rContext.GetRepeatPaM();

    pDoc->MergeTable( *pPam->GetPoint(), m_bWithPrev );
    ClearFEShellTabCols(*pDoc, nullptr);
}

void SwUndoMergeTable::SaveFormula( SwHistory& rHistory )
{
    if( !m_pHistory )
        m_pHistory.reset( new SwHistory );
    m_pHistory->Move( 0, &rHistory );
}

void InsertSort( std::vector<sal_uInt16>& rArr, sal_uInt16 nIdx )
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

SwUndoTableStyleMake::SwUndoTableStyleMake(OUString aName, const SwDoc& rDoc)
    : SwUndo(SwUndoId::TBLSTYLE_CREATE, &rDoc),
    m_sName(std::move(aName))
{ }

SwUndoTableStyleMake::~SwUndoTableStyleMake()
{ }

void SwUndoTableStyleMake::UndoImpl(::sw::UndoRedoContext & rContext)
{
    m_pAutoFormat = rContext.GetDoc().DelTableStyle(m_sName, true);
}

void SwUndoTableStyleMake::RedoImpl(::sw::UndoRedoContext & rContext)
{
    if (m_pAutoFormat)
    {
        SwTableAutoFormat* pFormat = rContext.GetDoc().MakeTableStyle(m_sName, true);
        if (pFormat)
        {
            *pFormat = *m_pAutoFormat;
            m_pAutoFormat.reset();
        }
    }
}

SwRewriter SwUndoTableStyleMake::GetRewriter() const
{
    SwRewriter aResult;
    aResult.AddRule(UndoArg1, m_sName);
    return aResult;
}

SwUndoTableStyleDelete::SwUndoTableStyleDelete(std::unique_ptr<SwTableAutoFormat> pAutoFormat, std::vector<SwTable*>&& rAffectedTables, const SwDoc& rDoc)
    : SwUndo(SwUndoId::TBLSTYLE_DELETE, &rDoc),
    m_pAutoFormat(std::move(pAutoFormat)),
    m_rAffectedTables(std::move(rAffectedTables))
{ }

SwUndoTableStyleDelete::~SwUndoTableStyleDelete()
{ }

void SwUndoTableStyleDelete::UndoImpl(::sw::UndoRedoContext & rContext)
{
    SwTableAutoFormat* pNewFormat = rContext.GetDoc().MakeTableStyle(m_pAutoFormat->GetName(), true);
    *pNewFormat = *m_pAutoFormat;
    for (size_t i=0; i < m_rAffectedTables.size(); i++)
        m_rAffectedTables[i]->SetTableStyleName(m_pAutoFormat->GetName());
}

void SwUndoTableStyleDelete::RedoImpl(::sw::UndoRedoContext & rContext)
{
    // Don't need to remember deleted table style nor affected tables, because they must be the same as these already known.
    rContext.GetDoc().DelTableStyle(m_pAutoFormat->GetName());
}

SwRewriter SwUndoTableStyleDelete::GetRewriter() const
{
    SwRewriter aResult;
    aResult.AddRule(UndoArg1, m_pAutoFormat->GetName());
    return aResult;
}

SwUndoTableStyleUpdate::SwUndoTableStyleUpdate(const SwTableAutoFormat& rNewFormat, const SwTableAutoFormat& rOldFormat, const SwDoc& rDoc)
    : SwUndo(SwUndoId::TBLSTYLE_UPDATE, &rDoc)
    , m_pOldFormat(new SwTableAutoFormat(rOldFormat))
    , m_pNewFormat(new SwTableAutoFormat(rNewFormat))
{ }

SwUndoTableStyleUpdate::~SwUndoTableStyleUpdate()
{ }

void SwUndoTableStyleUpdate::UndoImpl(::sw::UndoRedoContext & rContext)
{
    rContext.GetDoc().ChgTableStyle(m_pNewFormat->GetName(), *m_pOldFormat);
}

void SwUndoTableStyleUpdate::RedoImpl(::sw::UndoRedoContext & rContext)
{
    rContext.GetDoc().ChgTableStyle(m_pNewFormat->GetName(), *m_pNewFormat);
}

SwRewriter SwUndoTableStyleUpdate::GetRewriter() const
{
    SwRewriter aResult;
    aResult.AddRule(UndoArg1, m_pNewFormat->GetName());
    return aResult;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
