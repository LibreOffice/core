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

#ifndef SW_NDARR_HXX
#define SW_NDARR_HXX

#include <vector>

#include <boost/utility.hpp>

#include <com/sun/star/embed/XEmbeddedObject.hpp>

#include <svtools/embedhlp.hxx>

#include <bparr.hxx>
#include <ndtyp.hxx>
#include <o3tl/sorted_vector.hxx>

class Graphic;
class GraphicObject;
class SwAttrSet;
class SfxItemSet;
class SwCntntNode;
class SwDoc;
class SwGrfFmtColl;
class SwGrfNode;
class SwHistory;
class SwNode;
class SwNodeIndex;
class SwNodeRange;
class SwOLENode;
class SwOutlineNodes;
class SwPaM;
class SwSectionData;
class SwSectionFmt;
class SwTOXBase;
class SwSectionNode;
class SwStartNode;
class SwTableBoxFmt;
class SwTableFmt;
class SwTableLine;
class SwTableLineFmt;
class SwTableNode;
class SwTblToTxtSaves;
class SwTxtFmtColl;
class SwTxtNode;
class SwUndoTblToTxt;
class SwUndoTxtToTbl;
struct SwPosition;

// --------------------
// class SwNodes
// --------------------

typedef SwNode * SwNodePtr;
typedef bool (*FnForEach_SwNodes)( const SwNodePtr&, void* pArgs );
typedef struct _xmlTextWriter *xmlTextWriterPtr;

struct CompareSwOutlineNodes
{
    bool operator()( SwNode* const& lhs, SwNode* const& rhs) const;
};

class SwOutlineNodes : public o3tl::sorted_vector<SwNode*, CompareSwOutlineNodes>
{
public:
    bool Seek_Entry(SwNode* const &rP, sal_uInt16* pnPos) const;
};

class SW_DLLPUBLIC SwNodes
    : private BigPtrArray
    , private ::boost::noncopyable
{
    friend class SwDoc;
    friend class SwNode;
    friend class SwNodeIndex;

    SwNodeIndex* pRoot;                 ///< List of all indices on nodes.

    void InsertNode( const SwNodePtr pNode,
                     const SwNodeIndex& rPos );
    void InsertNode( const SwNodePtr pNode,
                     sal_uLong nPos );

    SwDoc* pMyDoc;                      ///< This Doc contains the nodes-array.

    SwNode *pEndOfPostIts, *pEndOfInserts,  ///< These are the fixed ranges.
           *pEndOfAutotext, *pEndOfRedlines,
           *pEndOfContent;

    mutable SwOutlineNodes* pOutlineNds;        ///< Array of all outline nodes.

    sal_Bool bInNodesDel : 1;           /**< In Case of recursive calling.
                                           Do not update Num/Outline. */
    sal_Bool bInDelUpdOutl : 1;         ///< Flag for updating of Outline.
    sal_Bool bInDelUpdNum : 1;          ///< Flag for updating of Outline.

    /// For administering indices.
    void RegisterIndex( SwNodeIndex& rIdx );
    void DeRegisterIndex( SwNodeIndex& rIdx );
    void RemoveNode( sal_uLong nDelPos, sal_uLong nLen, sal_Bool bDel );

    // Actions on the nodes.
    void SectionUpDown( const SwNodeIndex & aStart, const SwNodeIndex & aEnd );
    void DelNodes( const SwNodeIndex& rStart, sal_uLong nCnt = 1 );

    void ChgNode( SwNodeIndex& rDelPos, sal_uLong nSize,
                  SwNodeIndex& rInsPos, sal_Bool bNewFrms );

    void UpdtOutlineIdx( const SwNode& );   ///< Update all OutlineNodes starting from Node.

    void _CopyNodes( const SwNodeRange&, const SwNodeIndex&,
                    sal_Bool bNewFrms = sal_True, sal_Bool bTblInsDummyNode = sal_False ) const;
    void _DelDummyNodes( const SwNodeRange& rRg );

protected:
    SwNodes( SwDoc* pDoc );

public:
    ~SwNodes();

    typedef ::std::vector<SwNodeRange> NodeRanges_t;
    typedef ::std::vector<NodeRanges_t> TableRanges_t;

    SwNodePtr operator[]( sal_uLong n ) const
        { return (SwNodePtr)BigPtrArray::operator[] ( n ); }

    sal_uLong Count() const { return BigPtrArray::Count(); }
    void ForEach( FnForEach_SwNodes fnForEach, void* pArgs = 0 )
    {
        BigPtrArray::ForEach( 0, BigPtrArray::Count(),
                                (FnForEach) fnForEach, pArgs );
    }
    void ForEach( sal_uLong nStt, sal_uLong nEnd, FnForEach_SwNodes fnForEach, void* pArgs = 0 )
    {
        BigPtrArray::ForEach( nStt, nEnd, (FnForEach) fnForEach, pArgs );
    }
    void ForEach( const SwNodeIndex& rStart, const SwNodeIndex& rEnd,
                    FnForEach_SwNodes fnForEach, void* pArgs = 0 );

    /// A still empty section.
    SwNode& GetEndOfPostIts() const     { return *pEndOfPostIts; }
    /// Section fpr all footnotes.
    SwNode& GetEndOfInserts() const     { return *pEndOfInserts; }
    /// Section for all Flys/Header/Footers.
    SwNode& GetEndOfAutotext() const    { return *pEndOfAutotext; }
    /// Section for all Redlines.
    SwNode& GetEndOfRedlines() const    { return *pEndOfRedlines; }
    /** This is the last EndNode of a special section. After it
       there is only the regular ContentSection (i.e. the BodyText). */
    SwNode& GetEndOfExtras() const      { return *pEndOfRedlines; }
    /// Regular ContentSection (i.e. the BodyText).
    SwNode& GetEndOfContent() const     { return *pEndOfContent; }

    /** Is the NodesArray the regular one of Doc? (and not the UndoNds, ...)
       Implementation in doc.hxx (because one needs to know Doc for it) ! */
    sal_Bool IsDocNodes() const;

    sal_uInt16 GetSectionLevel(const SwNodeIndex &rIndex) const;
    void Delete(const SwNodeIndex &rPos, sal_uLong nNodes = 1);

    sal_Bool _MoveNodes( const SwNodeRange&, SwNodes& rNodes, const SwNodeIndex&,
                sal_Bool bNewFrms = sal_True );
    void MoveRange( SwPaM&, SwPosition&, SwNodes& rNodes );

    void _Copy( const SwNodeRange& rRg, const SwNodeIndex& rInsPos,
                sal_Bool bNewFrms = sal_True ) const
        {   _CopyNodes( rRg, rInsPos, bNewFrms ); }

    void SectionUp( SwNodeRange *);
    void SectionDown( SwNodeRange *pRange, SwStartNodeType = SwNormalStartNode );

    sal_Bool CheckNodesRange( const SwNodeIndex& rStt, const SwNodeIndex& rEnd ) const;

    void GoStartOfSection(SwNodeIndex *) const;
    void GoEndOfSection(SwNodeIndex *) const;

    SwCntntNode* GoNext(SwNodeIndex *) const;
    SwCntntNode* GoPrevious(SwNodeIndex *) const;

    /** Go to next/previous Cntnt/Table-node for which LayoutFrames exist.
     While doing this do not leave Header/Footer/Frame etc. */
    SwNode* GoNextWithFrm(SwNodeIndex *) const;
    SwNode* GoPreviousWithFrm(SwNodeIndex *) const;

    /** Go to next content-node that is not protected or hidden
       (Both set FALSE ==> GoNext/GoPrevious!!!). */
    SwCntntNode* GoNextSection( SwNodeIndex *, int bSkipHidden  = sal_True,
                                           int bSkipProtect = sal_True ) const;
    SwCntntNode* GoPrevSection( SwNodeIndex *, int bSkipHidden  = sal_True,
                                           int bSkipProtect = sal_True ) const;

    /** Create an empty section of Start- and EndNote. It may be called
       only if a new section with content is to be created,
       e.g. at filters/Undo/... */
    SwStartNode* MakeEmptySection( const SwNodeIndex& rIdx,
                                    SwStartNodeType = SwNormalStartNode );

    /// Implementations of "Make...Node" are in the given .cxx-files.
    SwTxtNode *MakeTxtNode( const SwNodeIndex & rWhere,
                            SwTxtFmtColl *pColl,
                            SwAttrSet* pAutoAttr = 0 ); ///< in ndtxt.cxx
    SwStartNode* MakeTextSection( const SwNodeIndex & rWhere,
                            SwStartNodeType eSttNdTyp,
                            SwTxtFmtColl *pColl,
                            SwAttrSet* pAutoAttr = 0 );

    SwGrfNode *MakeGrfNode( const SwNodeIndex & rWhere,
                            const OUString& rGrfName,
                            const OUString& rFltName,
                            const Graphic* pGraphic,
                            SwGrfFmtColl *pColl,
                            SwAttrSet* pAutoAttr = 0,
                            sal_Bool bDelayed = sal_False );    ///< in ndgrf.cxx

    SwGrfNode *MakeGrfNode( const SwNodeIndex & rWhere,
                            const GraphicObject& rGrfObj,
                            SwGrfFmtColl *pColl,
                            SwAttrSet* pAutoAttr = 0 ); ///< in ndgrf.cxx

    SwOLENode *MakeOLENode( const SwNodeIndex & rWhere,
                            const svt::EmbeddedObjectRef&,
                            SwGrfFmtColl *pColl,
                            SwAttrSet* pAutoAttr = 0 ); ///< in ndole.cxx
    SwOLENode *MakeOLENode( const SwNodeIndex & rWhere,
                            const OUString &rName,
                            sal_Int64 nAspect,
                            SwGrfFmtColl *pColl,
                            SwAttrSet* pAutoAttr ); ///< in ndole.cxx

    /// Array of all OutlineNodes.
    const SwOutlineNodes& GetOutLineNds() const;

    //void UpdateOutlineNode( const SwNode&, sal_uInt8 nOldLevel, sal_uInt8 nNewLevel );//#outline level,removed by zhaojianwei

    /// Update all Nodes - Rule/Format-Change.
    void UpdateOutlineNode(SwNode & rNd);

    /** Insert nodes for tables. If Lines is given, create the matrix
       from lines and boxes, else only the count of boxes.

       New parameter pAttrSet: If pAttrSet is non-null and contains an
       adjust item it is propagated to the table cells. If there is an
       adjust in pCntntTxtColl or pHeadlineTxtColl this adjust item
       overrides the item in pAttrSet. */

    SwTableNode* InsertTable( const SwNodeIndex& rNdIdx,
                        sal_uInt16 nBoxes, SwTxtFmtColl* pCntntTxtColl,
                        sal_uInt16 nLines = 0, sal_uInt16 nRepeat = 0,
                        SwTxtFmtColl* pHeadlineTxtColl = 0,
                        const SwAttrSet * pAttrSet = 0);

    /// Create balanced table from selected range.
    SwTableNode* TextToTable( const SwNodeRange& rRange, sal_Unicode cCh,
                                SwTableFmt* pTblFmt,
                                SwTableLineFmt* pLineFmt,
                                SwTableBoxFmt* pBoxFmt,
                                SwTxtFmtColl* pTxtColl,
                                SwUndoTxtToTbl* pUndo = 0 );

    SwNodeRange * ExpandRangeForTableBox(const SwNodeRange & rRange);

    /// create a table from a vector of NodeRanges - API support
    SwTableNode* TextToTable( const TableRanges_t& rTableNodes,
                                SwTableFmt* pTblFmt,
                                SwTableLineFmt* pLineFmt,
                                SwTableBoxFmt* pBoxFmt,
                                SwTxtFmtColl* pTxtColl
                                /*, SwUndo... pUndo*/ );


    /// Create regular text from what was table.
    sal_Bool TableToText( const SwNodeRange& rRange, sal_Unicode cCh,
                        SwUndoTblToTxt* = 0 );
    /// Is in untbl.cxx and may called only by Undo-object.
    SwTableNode* UndoTableToText( sal_uLong nStt, sal_uLong nEnd,
                        const SwTblToTxtSaves& rSavedData );

    /** Insert a new box in the line before InsPos. Its format
       is taken from the following one (or from the previous one if we are
       at the end). In the line there must be a box already. */
    sal_Bool InsBoxen( SwTableNode*, SwTableLine*, SwTableBoxFmt*,
                        /// Formats for TextNode of box.
                        SwTxtFmtColl*, const SfxItemSet* pAutoAttr,
                        sal_uInt16 nInsPos, sal_uInt16 nCnt = 1 );
    /** Splits a table at the base-line which contains the index.
       All base lines behind it are moved to a new table/ -node.
       Is the flag bCalcNewSize set to TRUE, the new SSize for both
       tables is calculated from the Maximum of the boxes, provided
       SSize is set "absolute" (LONG_MAX).
       (Momentarily this is needed only for the RTF-parser.) */
    SwTableNode* SplitTable( const SwNodeIndex& rPos, sal_Bool bAfter = sal_True,
                                sal_Bool bCalcNewSize = sal_False );
    /// Two Tables that are following one another are merged.
    sal_Bool MergeTable( const SwNodeIndex& rPos, sal_Bool bWithPrev = sal_True,
                    sal_uInt16 nMode = 0, SwHistory* pHistory = 0 );

    /// Insert a new SwSection.
    SwSectionNode* InsertTextSection(SwNodeIndex const& rNdIdx,
                                SwSectionFmt& rSectionFmt,
                                SwSectionData const&,
                                SwTOXBase const*const pTOXBase,
                                SwNodeIndex const*const pEnde,
                                bool const bInsAtStart = true,
                                bool const bCreateFrms = true);

    /// Which Doc contains the nodes-array?
            SwDoc* GetDoc()         { return pMyDoc; }
    const   SwDoc* GetDoc() const   { return pMyDoc; }

    /** Search previous / next content node or table node with frames.
     If no end is given begin with the FrameIndex, else start search
     with that before rFrmIdx and pEnd at the back.
     If no valid node is found, return 0. rFrmIdx points to the node with frames. **/
    SwNode* FindPrvNxtFrmNode( SwNodeIndex& rFrmIdx,
                                const SwNode* pEnd = 0 ) const;

    SwNode * DocumentSectionStartNode(SwNode * pNode) const;
    SwNode * DocumentSectionEndNode(SwNode * pNode) const;

    /**
     * Dumps the entire nodes structure to the given destination (file nodes.xml in the current directory by default)
     * @since 3.5
     */
    void dumpAsXml( xmlTextWriterPtr writer = NULL );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
