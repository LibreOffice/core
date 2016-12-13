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

#ifndef INCLUDED_SW_INC_NDARR_HXX
#define INCLUDED_SW_INC_NDARR_HXX

#include <vector>
#include <memory>

#include <boost/noncopyable.hpp>

#include <com/sun/star/embed/XEmbeddedObject.hpp>

#include <svtools/embedhlp.hxx>

#include <bparr.hxx>
#include <ndtyp.hxx>
#include <o3tl/sorted_vector.hxx>
#include <ring.hxx>

class Graphic;
class GraphicObject;
class SwAttrSet;
class SfxItemSet;
class SwContentNode;
class SwDoc;
class SwGrfFormatColl;
class SwGrfNode;
class SwHistory;
class SwNode;
class SwNodeIndex;
class SwNodeRange;
class SwOLENode;
class SwOutlineNodes;
class SwPaM;
class SwSectionData;
class SwSectionFormat;
class SwTOXBase;
class SwSectionNode;
class SwStartNode;
class SwTableBoxFormat;
class SwTableFormat;
class SwTableLine;
class SwTableLineFormat;
class SwTableNode;
class SwTableToTextSaves;
class SwTextFormatColl;
class SwTextNode;
class SwUndoTableToText;
class SwUndoTextToTable;
struct SwPosition;

namespace sw { class DocumentContentOperationsManager; }

// class SwNodes

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
    bool Seek_Entry(SwNode* rP, sal_uInt16* pnPos) const;
};

class SW_DLLPUBLIC SwNodes
    : private BigPtrArray
    , private ::boost::noncopyable
{
    friend class SwDoc;
    friend class SwNode;
    friend class SwNodeIndex;
    friend class SwStartNode;
    friend class ::sw::DocumentContentOperationsManager;

    SwNodeIndex* m_vIndices; ///< ring of all indices on nodes.
    void RemoveNode( sal_uLong nDelPos, sal_uLong nLen, bool bDel );

    void InsertNode( const SwNodePtr pNode,
                     const SwNodeIndex& rPos );
    void InsertNode( const SwNodePtr pNode,
                     sal_uLong nPos );

    SwDoc* m_pMyDoc;                      ///< This Doc contains the nodes-array.

    SwNode *m_pEndOfPostIts, *m_pEndOfInserts,  ///< These are the fixed ranges.
           *m_pEndOfAutotext, *m_pEndOfRedlines,
           *m_pEndOfContent;

    mutable SwOutlineNodes* m_pOutlineNodes;        ///< Array of all outline nodes.

    bool m_bInNodesDel : 1;           /**< In Case of recursive calling.
                                           Do not update Num/Outline. */
    bool m_bInDelUpdOutline : 1;         ///< Flag for updating of Outline.
    bool m_bInDelUpdNum : 1;          ///< Flag for updating of Outline.

    // Actions on the nodes.
    static void SectionUpDown( const SwNodeIndex & aStart, const SwNodeIndex & aEnd );
    void DelNodes( const SwNodeIndex& rStart, sal_uLong nCnt = 1 );

    void ChgNode( SwNodeIndex& rDelPos, sal_uLong nSize,
                  SwNodeIndex& rInsPos, bool bNewFrames );

    void UpdateOutlineIdx( const SwNode& );   ///< Update all OutlineNodes starting from Node.

    void _CopyNodes( const SwNodeRange&, const SwNodeIndex&,
                    bool bNewFrames = true, bool bTableInsDummyNode = false ) const;
    void _DelDummyNodes( const SwNodeRange& rRg );

protected:
    SwNodes( SwDoc* pDoc );

public:
    ~SwNodes();

    typedef ::std::vector<SwNodeRange> NodeRanges_t;
    typedef ::std::vector<NodeRanges_t> TableRanges_t;

    SwNodePtr operator[]( sal_uLong n ) const; // defined in node.hxx

    sal_uLong Count() const { return BigPtrArray::Count(); }
    void ForEach( FnForEach_SwNodes fnForEach, void* pArgs = nullptr )
    {
        ForEach( 0, BigPtrArray::Count(), fnForEach, pArgs );
    }
    void ForEach( sal_uLong nStt, sal_uLong nEnd, FnForEach_SwNodes fnForEach, void* pArgs = nullptr );
    void ForEach( const SwNodeIndex& rStart, const SwNodeIndex& rEnd,
                    FnForEach_SwNodes fnForEach, void* pArgs = nullptr );

    /// A still empty section.
    SwNode& GetEndOfPostIts() const     { return *m_pEndOfPostIts; }
    /// Section fpr all footnotes.
    SwNode& GetEndOfInserts() const     { return *m_pEndOfInserts; }
    /// Section for all Flys/Header/Footers.
    SwNode& GetEndOfAutotext() const    { return *m_pEndOfAutotext; }
    /// Section for all Redlines.
    SwNode& GetEndOfRedlines() const    { return *m_pEndOfRedlines; }
    /** This is the last EndNode of a special section. After it
       there is only the regular ContentSection (i.e. the BodyText). */
    SwNode& GetEndOfExtras() const      { return *m_pEndOfRedlines; }
    /// Regular ContentSection (i.e. the BodyText).
    SwNode& GetEndOfContent() const     { return *m_pEndOfContent; }

    /** Is the NodesArray the regular one of Doc? (and not the UndoNds, ...)
       Implementation in doc.hxx (because one needs to know Doc for it) ! */
    bool IsDocNodes() const;

    static sal_uInt16 GetSectionLevel(const SwNodeIndex &rIndex);
    void Delete(const SwNodeIndex &rPos, sal_uLong nNodes = 1);

    bool _MoveNodes( const SwNodeRange&, SwNodes& rNodes, const SwNodeIndex&,
                bool bNewFrames = true );
    void MoveRange( SwPaM&, SwPosition&, SwNodes& rNodes );

    void _Copy( const SwNodeRange& rRg, const SwNodeIndex& rInsPos,
                bool bNewFrames = true ) const
        {   _CopyNodes( rRg, rInsPos, bNewFrames ); }

    void SectionUp( SwNodeRange *);
    void SectionDown( SwNodeRange *pRange, SwStartNodeType = SwNormalStartNode );

    bool CheckNodesRange( const SwNodeIndex& rStt, const SwNodeIndex& rEnd ) const;

    static void GoStartOfSection(SwNodeIndex *);
    static void GoEndOfSection(SwNodeIndex *);

    SwContentNode* GoNext(SwNodeIndex *) const;
    static SwContentNode* GoPrevious(SwNodeIndex *);

    /** Go to next content-node that is not protected or hidden
       (Both set FALSE ==> GoNext/GoPrevious!!!). */
    SwContentNode* GoNextSection( SwNodeIndex *, bool bSkipHidden  = true,
                                           bool bSkipProtect = true ) const;
    static SwContentNode* GoPrevSection( SwNodeIndex *, bool bSkipHidden  = true,
                                           bool bSkipProtect = true );

    /** Create an empty section of Start- and EndNote. It may be called
       only if a new section with content is to be created,
       e.g. at filters/Undo/... */
    static SwStartNode* MakeEmptySection( const SwNodeIndex& rIdx,
                                    SwStartNodeType = SwNormalStartNode );

    /// Implementations of "Make...Node" are in the given .cxx-files.
    SwTextNode *MakeTextNode( const SwNodeIndex & rWhere,
                            SwTextFormatColl *pColl,
                            SwAttrSet* pAutoAttr = nullptr ); ///< in ndtxt.cxx
    SwStartNode* MakeTextSection( const SwNodeIndex & rWhere,
                            SwStartNodeType eSttNdTyp,
                            SwTextFormatColl *pColl,
                            SwAttrSet* pAutoAttr = nullptr );

    static SwGrfNode *MakeGrfNode( const SwNodeIndex & rWhere,
                            const OUString& rGrfName,
                            const OUString& rFltName,
                            const Graphic* pGraphic,
                            SwGrfFormatColl *pColl,
                            SwAttrSet* pAutoAttr = nullptr,
                            bool bDelayed = false );    ///< in ndgrf.cxx

    static SwGrfNode *MakeGrfNode( const SwNodeIndex & rWhere,
                            const GraphicObject& rGrfObj,
                            SwGrfFormatColl *pColl,
                            SwAttrSet* pAutoAttr = nullptr ); ///< in ndgrf.cxx

    SwOLENode *MakeOLENode( const SwNodeIndex & rWhere,
                            const svt::EmbeddedObjectRef&,
                            SwGrfFormatColl *pColl,
                            SwAttrSet* pAutoAttr = nullptr ); ///< in ndole.cxx
    SwOLENode *MakeOLENode( const SwNodeIndex & rWhere,
                            const OUString &rName,
                            sal_Int64 nAspect,
                            SwGrfFormatColl *pColl,
                            SwAttrSet* pAutoAttr ); ///< in ndole.cxx

    /// Array of all OutlineNodes.
    const SwOutlineNodes& GetOutLineNds() const { return *m_pOutlineNodes;}

    /// Update all Nodes - Rule/Format-Change.
    void UpdateOutlineNode(SwNode & rNd);

    /** Insert nodes for tables. If Lines is given, create the matrix
       from lines and boxes, else only the count of boxes.

       New parameter pAttrSet: If pAttrSet is non-null and contains an
       adjust item it is propagated to the table cells. If there is an
       adjust in pContentTextColl or pHeadlineTextColl this adjust item
       overrides the item in pAttrSet. */

    static SwTableNode* InsertTable( const SwNodeIndex& rNdIdx,
                        sal_uInt16 nBoxes, SwTextFormatColl* pContentTextColl,
                        sal_uInt16 nLines = 0, sal_uInt16 nRepeat = 0,
                        SwTextFormatColl* pHeadlineTextColl = nullptr,
                        const SwAttrSet * pAttrSet = nullptr);

    /// Create balanced table from selected range.
    SwTableNode* TextToTable( const SwNodeRange& rRange, sal_Unicode cCh,
                                SwTableFormat* pTableFormat,
                                SwTableLineFormat* pLineFormat,
                                SwTableBoxFormat* pBoxFormat,
                                SwTextFormatColl* pTextColl,
                                SwUndoTextToTable* pUndo = nullptr );

    SwNodeRange * ExpandRangeForTableBox(const SwNodeRange & rRange);

    /// create a table from a vector of NodeRanges - API support
    SwTableNode* TextToTable( const TableRanges_t& rTableNodes,
                                SwTableFormat* pTableFormat,
                                SwTableLineFormat* pLineFormat,
                                SwTableBoxFormat* pBoxFormat,
                                SwTextFormatColl* pTextColl
                                /*, SwUndo... pUndo*/ );

    /// Create regular text from what was table.
    bool TableToText( const SwNodeRange& rRange, sal_Unicode cCh,
                        SwUndoTableToText* = nullptr );
    /// Is in untbl.cxx and may called only by Undo-object.
    SwTableNode* UndoTableToText( sal_uLong nStt, sal_uLong nEnd,
                        const SwTableToTextSaves& rSavedData );

    /** Insert a new box in the line before InsPos. Its format
       is taken from the following one (or from the previous one if we are
       at the end). In the line there must be a box already. */
    bool InsBoxen( SwTableNode*, SwTableLine*, SwTableBoxFormat*,
                        /// Formats for TextNode of box.
                        SwTextFormatColl*, const SfxItemSet* pAutoAttr,
                        sal_uInt16 nInsPos, sal_uInt16 nCnt = 1 );
    /** Splits a table at the base-line which contains the index.
       All base lines behind it are moved to a new table/ -node.
       Is the flag bCalcNewSize set to TRUE, the new SSize for both
       tables is calculated from the Maximum of the boxes, provided
       SSize is set "absolute" (LONG_MAX).
       (Momentarily this is needed only for the RTF-parser.) */
    SwTableNode* SplitTable( const SwNodeIndex& rPos, bool bAfter = true,
                                bool bCalcNewSize = false );
    /// Two Tables that are following one another are merged.
    bool MergeTable( const SwNodeIndex& rPos, bool bWithPrev = true,
                    sal_uInt16 nMode = 0, SwHistory* pHistory = nullptr );

    /// Insert a new SwSection.
    SwSectionNode* InsertTextSection(SwNodeIndex const& rNdIdx,
                                SwSectionFormat& rSectionFormat,
                                SwSectionData const&,
                                SwTOXBase const*const pTOXBase,
                                SwNodeIndex const*const pEnde,
                                bool const bInsAtStart = true,
                                bool const bCreateFrames = true);

    /// Which Doc contains the nodes-array?
            SwDoc* GetDoc()         { return m_pMyDoc; }
    const   SwDoc* GetDoc() const   { return m_pMyDoc; }

    /** Search previous / next content node or table node with frames.
     If no end is given begin with the FrameIndex, else start search
     with that before rFrameIdx and pEnd at the back.
     If no valid node is found, return 0. rFrameIdx points to the node with frames. **/
    SwNode* FindPrvNxtFrameNode( SwNodeIndex& rFrameIdx,
                                const SwNode* pEnd = nullptr ) const;

    SwNode * DocumentSectionStartNode(SwNode * pNode) const;
    SwNode * DocumentSectionEndNode(SwNode * pNode) const;

    /**
     * Dumps the entire nodes structure to the given destination (file nodes.xml in the current directory by default)
     * @since 3.5
     */
    void dumpAsXml( xmlTextWriterPtr pWriter ) const;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
