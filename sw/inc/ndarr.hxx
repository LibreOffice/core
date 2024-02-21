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

#include <sal/config.h>

#include <limits>
#include <vector>
#include <memory>
#include <optional>

#include "bparr.hxx"
#include "ndtyp.hxx"
#include <rtl/ustring.hxx>
#include <o3tl/sorted_vector.hxx>
#include "nodeoffset.hxx"

class Graphic;
class GraphicObject;
class SwAttrSet;
class SfxItemSet;
class SwContentNode;
class SwDoc;
class SwGrfFormatColl;
class SwGrfNode;
class SwNode;
class SwNodeIndex;
class SwNodeRange;
class SwOLENode;
class SwPaM;
class SwRootFrame;
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
class SwTextFormatColl;
class SwTextNode;
class SwUndoTableToText;
class SwUndoTextToTable;
struct SwPosition;

namespace sw { class DocumentContentOperationsManager; }
namespace svt { class EmbeddedObjectRef; }


typedef bool (*FnForEach_SwNodes)( SwNode*, void* pArgs );
typedef struct _xmlTextWriter *xmlTextWriterPtr;

struct CompareSwOutlineNodes
{
    bool operator()(const SwNode* lhs, const SwNode* rhs) const;
};

class SwOutlineNodes : public o3tl::sorted_vector<SwNode*, CompareSwOutlineNodes>
{
public:
    static constexpr auto npos = std::numeric_limits<size_type>::max();

    bool Seek_Entry(const SwNode* rP, size_type* pnPos) const;
};

struct SwTableToTextSave;
using SwTableToTextSaves = std::vector<std::unique_ptr<SwTableToTextSave>>;

class SwNodes final
    : private BigPtrArray
{
    friend class SwDoc;
    friend class SwNode;
    friend class SwNodeIndex;
    friend class SwStartNode;
    friend class ::sw::DocumentContentOperationsManager;

    SwNodeIndex* m_vIndices; ///< ring of all indices on nodes.
    void RemoveNode( SwNodeOffset nDelPos, SwNodeOffset nLen, bool bDel );

    void InsertNode( SwNode* pNode, const SwNodeIndex& rPos );
    void InsertNode( SwNode* pNode, SwNodeOffset nPos );

    SwDoc& m_rMyDoc;                      ///< This Doc contains the nodes-array.

    SwNode *m_pEndOfPostIts, *m_pEndOfInserts,  ///< These are the fixed ranges.
           *m_pEndOfAutotext, *m_pEndOfRedlines;
    std::unique_ptr<SwNode> m_pEndOfContent;

    mutable SwOutlineNodes m_aOutlineNodes;        ///< Array of all outline nodes.

    bool m_bInNodesDel : 1;           /**< In Case of recursive calling.
                                           Do not update Num/Outline. */
    bool m_bInDelUpdOutline : 1;         ///< Flag for updating of Outline.

    // Actions on the nodes.
    static void SectionUpDown( const SwNodeIndex & aStart, const SwNodeIndex & aEnd );
    void DelNodes( const SwNodeIndex& rStart, SwNodeOffset nCnt = SwNodeOffset(1) );

    void ChgNode( SwNodeIndex const & rDelPos, SwNodeOffset nSize,
                  SwNodeIndex& rInsPos, bool bNewFrames );

    void UpdateOutlineIdx( const SwNode& );   ///< Update all OutlineNodes starting from Node.

    void CopyNodes( const SwNodeRange&, SwNode& rPos,
                    bool bNewFrames, bool bTableInsDummyNode = false ) const;
    void DelDummyNodes( const SwNodeRange& rRg );

    SwNodes(SwNodes const&) = delete;
    SwNodes& operator=(SwNodes const&) = delete;

    SwNodes(SwDoc& rDoc);

public:
    ~SwNodes();

    typedef std::vector<SwNodeRange> NodeRanges_t;
    typedef std::vector<NodeRanges_t> TableRanges_t;

    SwNode* operator[]( SwNodeOffset n ) const; // defined in node.hxx

    SwNodeOffset Count() const { return SwNodeOffset(BigPtrArray::Count()); }
    void ForEach( FnForEach_SwNodes fnForEach, void* pArgs = nullptr )
    {
        ForEach( SwNodeOffset(0), Count(), fnForEach, pArgs );
    }
    void ForEach( SwNodeOffset nStt, SwNodeOffset nEnd, FnForEach_SwNodes fnForEach, void* pArgs );
    void ForEach( SwNode& rStart, SwNode& rEnd,
                    FnForEach_SwNodes fnForEach, void* pArgs );
    void ForEach( const SwNodeIndex& rStart, const SwNodeIndex& rEnd,
                    FnForEach_SwNodes fnForEach, void* pArgs );

    /// A still empty section.
    SwNode& GetEndOfPostIts() const     { return *m_pEndOfPostIts; }
    /// Section for all footnotes.
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
    SW_DLLPUBLIC bool IsDocNodes() const;

    static sal_uInt16 GetSectionLevel(const SwNode &rIndex);
    void Delete(const SwNodeIndex &rPos, SwNodeOffset nNodes = SwNodeOffset(1));
    void Delete(const SwNode& rPos, SwNodeOffset nNodes = SwNodeOffset(1));

    bool MoveNodes( const SwNodeRange&, SwNodes& rNodes, SwNode& rPos,
                bool bNewFrames = true );
    void MoveRange( SwPaM&, SwPosition&, SwNodes& rNodes );

    void Copy_( const SwNodeRange& rRg, SwNode& rInsPos,
                bool bNewFrames = true ) const
        {   CopyNodes( rRg, rInsPos, bNewFrames ); }

    void SectionUp( SwNodeRange *);
    void SectionDown( SwNodeRange *pRange, SwStartNodeType = SwNormalStartNode );

    static void GoStartOfSection(SwNodeIndex *);
    static void GoEndOfSection(SwNodeIndex *);

    SW_DLLPUBLIC static SwContentNode* GoNext(SwNodeIndex*);
    static SwContentNode* GoNext(SwPosition*);
    static SwContentNode* GoPrevious(SwNodeIndex *, bool canCrossBoundary = false);
    static SwContentNode* GoPrevious(SwPosition *, bool canCrossBoundary = false);

    /** Go to next content-node that is not protected or hidden
       (Both set FALSE ==> GoNext/GoPrevious!!!). */
    static SwContentNode* GoNextSection( SwNodeIndex *, bool bSkipHidden  = true,
                                           bool bSkipProtect = true );
    static SwContentNode* GoNextSection( SwPosition *, bool bSkipHidden  = true,
                                           bool bSkipProtect = true );
    static SwContentNode* GoPrevSection( SwNodeIndex *, bool bSkipHidden  = true,
                                           bool bSkipProtect = true );
    static SwContentNode* GoPrevSection( SwPosition *, bool bSkipHidden  = true,
                                           bool bSkipProtect = true );

    /** Create an empty section of Start- and EndNote. It may be called
       only if a new section with content is to be created,
       e.g. at filters/Undo/... */
    static SwStartNode* MakeEmptySection( SwNode& rWhere,
                                    SwStartNodeType = SwNormalStartNode );

    /// Implementations of "Make...Node" are in the given .cxx-files.
    SW_DLLPUBLIC SwTextNode *MakeTextNode( SwNode& rWhere,
                            SwTextFormatColl *pColl,
                            bool bNewFrames = true); ///< in ndtxt.cxx
    SW_DLLPUBLIC SwStartNode* MakeTextSection( const SwNode & rWhere,
                            SwStartNodeType eSttNdTyp,
                            SwTextFormatColl *pColl );

    static SwGrfNode *MakeGrfNode( SwNode& rWhere,
                            const OUString& rGrfName,
                            const OUString& rFltName,
                            const Graphic* pGraphic,
                            SwGrfFormatColl *pColl,
                            SwAttrSet const * pAutoAttr = nullptr );    ///< in ndgrf.cxx

    SwOLENode *MakeOLENode( SwNode& rWhere,
                            const svt::EmbeddedObjectRef&,
                            SwGrfFormatColl *pColl ); ///< in ndole.cxx
    SwOLENode *MakeOLENode( SwNode& rWhere,
                            const OUString &rName,
                            sal_Int64 nAspect,
                            SwGrfFormatColl *pColl,
                            SwAttrSet const * pAutoAttr ); ///< in ndole.cxx

    /// Array of all OutlineNodes.
    const SwOutlineNodes& GetOutLineNds() const { return m_aOutlineNodes;}

    /// Update all Nodes - Rule/Format-Change.
    void UpdateOutlineNode(SwNode & rNd);

    /** Insert nodes for tables. If Lines is given, create the matrix
       from lines and boxes, else only the count of boxes.

       New parameter pAttrSet: If pAttrSet is non-null and contains an
       adjust item it is propagated to the table cells. If there is an
       adjust in pContentTextColl or pHeadlineTextColl this adjust item
       overrides the item in pAttrSet. */

    static SwTableNode* InsertTable( SwNode& rNd,
                        sal_uInt16 nBoxes, SwTextFormatColl* pContentTextColl,
                        sal_uInt16 nLines, sal_uInt16 nRepeat,
                        SwTextFormatColl* pHeadlineTextColl,
                        const SwAttrSet * pAttrSet);

    /// Create balanced table from selected range.
    SwTableNode* TextToTable( const SwNodeRange& rRange, sal_Unicode cCh,
                                SwTableFormat* pTableFormat,
                                SwTableLineFormat* pLineFormat,
                                SwTableBoxFormat* pBoxFormat,
                                SwTextFormatColl* pTextColl,
                                SwUndoTextToTable* pUndo );

    void ExpandRangeForTableBox(const SwNodeRange & rRange,
                                std::optional<SwNodeRange>& rExpandedRange);

    /// create a table from a vector of NodeRanges - API support
    SwTableNode* TextToTable( const TableRanges_t& rTableNodes,
                                SwTableFormat* pTableFormat,
                                SwTableLineFormat* pLineFormat,
                                SwTableBoxFormat* pBoxFormat );

    /// Create regular text from what was table.
    bool TableToText( const SwNodeRange& rRange, sal_Unicode cCh,
                        SwUndoTableToText* );
    /// Is in untbl.cxx and may called only by Undo-object.
    SwTableNode* UndoTableToText( SwNodeOffset nStt, SwNodeOffset nEnd,
                        const SwTableToTextSaves& rSavedData );

    /** Insert a new box in the line before InsPos. Its format
       is taken from the following one (or from the previous one if we are
       at the end). In the line there must be a box already. */
    SW_DLLPUBLIC bool InsBoxen( SwTableNode*, SwTableLine*, SwTableBoxFormat*,
                        /// Formats for TextNode of box.
                        SwTextFormatColl*, const SfxItemSet* pAutoAttr,
                        sal_uInt16 nInsPos, sal_uInt16 nCnt = 1 );
    /** Splits a table at the base-line which contains the index.
       All base lines behind it are moved to a new table/ -node.
       Is the flag bCalcNewSize set to TRUE, the new SSize for both
       tables is calculated from the Maximum of the boxes, provided
       SSize is set "absolute" (LONG_MAX).
       (Momentarily this is needed only for the RTF-parser.) */
    SwTableNode* SplitTable( SwNode& rPos, bool bAfter = true,
                                bool bCalcNewSize = false );
    /// Two Tables that are following one another are merged.
    bool MergeTable( SwNode& rPos, bool bWithPrev = true );

    /// Insert a new SwSection.
    SwSectionNode* InsertTextSection(SwNode& rNd,
                                SwSectionFormat& rSectionFormat,
                                SwSectionData const&,
                                SwTOXBase const*const pTOXBase,
                                SwNode const * pEndNd,
                                bool const bInsAtStart = true,
                                bool const bCreateFrames = true);

    /// Which Doc contains the nodes-array?
            SwDoc& GetDoc()         { return m_rMyDoc; }
    const   SwDoc& GetDoc() const   { return m_rMyDoc; }

    /** Search previous / next content node or table node with frames.
     Search is started backward with the one before rFrameNd and
     forward after pEnd.
     If no valid node is found, return nullptr. **/
    SwNode* FindPrvNxtFrameNode( const SwNode& rFrameNd,
                                const SwNode* pEnd,
                                SwRootFrame const* pLayout = nullptr) const;

    SwNode * DocumentSectionStartNode(SwNode * pNode) const;
    SwNode * DocumentSectionEndNode(SwNode * pNode) const;

    /**
     * Dumps the entire nodes structure to the given destination (file nodes.xml in the current directory by default)
     */
    void dumpAsXml( xmlTextWriterPtr pWriter ) const;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
