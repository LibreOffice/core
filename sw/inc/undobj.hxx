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
#ifndef INCLUDED_SW_INC_UNDOBJ_HXX
#define INCLUDED_SW_INC_UNDOBJ_HXX

#include <vector>
#include <memory>

#include <svl/undo.hxx>
#include "SwRewriter.hxx"
#include "swundo.hxx"
#include "nodeoffset.hxx"
#include "ndindex.hxx"
#include <o3tl/typed_flags_set.hxx>
#include <optional>

class SwHistory;
class SwPaM;
struct SwPosition;
class SwDoc;
class SwTextFormatColl;
class SwFrameFormat;
class SwFormatAnchor;
class SwNode;
class SwNodeRange;
class SwRedlineData;
class SwRedlineSaveDatas;
enum class RedlineFlags;
enum class RndStdIds;
typedef struct _xmlTextWriter* xmlTextWriterPtr;

namespace sw {
    class UndoRedoContext;
    class RepeatContext;
}

class SwUndo
    : public SfxUndoAction
{
    SwUndoId const m_nId;
    RedlineFlags   m_nOrigRedlineFlags;
    ViewShellId    m_nViewShellId;
    bool m_isRepeatIgnored; ///< for multi-selection, only repeat 1st selection

protected:
    bool m_bCacheComment;
    mutable std::optional<OUString> maComment;

    static void RemoveIdxFromSection( SwDoc&, SwNodeOffset nSttIdx, const SwNodeOffset* pEndIdx = nullptr );
    static void RemoveIdxFromRange( SwPaM& rPam, bool bMoveNext );
    static void RemoveIdxRel( SwNodeOffset, const SwPosition& );

    static bool CanRedlineGroup( SwRedlineSaveDatas& rCurr,
                                const SwRedlineSaveDatas& rCheck,
                                bool bCurrIsEnd );

    /**
       Returns the rewriter for this object.

       @return the rewriter for this object
    */
    virtual SwRewriter GetRewriter() const;

    // the 4 methods that derived classes have to override
    // base implementation does nothing
    virtual void RepeatImpl( ::sw::RepeatContext & );
public: // should not be public, but ran into trouble in untbl.cxx
    virtual void UndoImpl( ::sw::UndoRedoContext & ) = 0;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) = 0;

private:
    /// Try to obtain the view shell ID of the current view.
    static ViewShellId CreateViewShellId(const SwDoc* pDoc);
    // SfxUndoAction
    virtual void Undo() override;
    virtual void Redo() override;
    virtual void UndoWithContext(SfxUndoContext &) override;
    virtual void RedoWithContext(SfxUndoContext &) override;
    virtual void Repeat(SfxRepeatTarget &) override;
    virtual bool CanRepeat(SfxRepeatTarget &) const override;

public:
    SwUndo(SwUndoId const nId, const SwDoc* pDoc);
    virtual ~SwUndo() override;

    SwUndoId GetId() const { return m_nId; }

    /**
       Returns textual comment for this undo object.

       The textual comment is created from the resource string
       corresponding to this object's ID. The rewriter of this object
       is applied to the resource string to get the final comment.

       @return textual comment for this undo object
    */
    virtual OUString GetComment() const override;

    /// See SfxUndoAction::GetViewShellId().
    ViewShellId GetViewShellId() const override;

    // UndoObject remembers which mode was turned on.
    // In Undo/Redo/Repeat this remembered mode is switched on.
    RedlineFlags GetRedlineFlags() const { return m_nOrigRedlineFlags; }
    void SetRedlineFlags( RedlineFlags eMode ) { m_nOrigRedlineFlags = eMode; }

    bool IsDelBox() const;

    // Save and set Redline data.
    static bool FillSaveData( const SwPaM& rRange, SwRedlineSaveDatas& rSData,
                              bool bDelRange = true, bool bCopyNext = true );
    static bool FillSaveDataForFormat( const SwPaM& , SwRedlineSaveDatas& );
    static void SetSaveData( SwDoc& rDoc, SwRedlineSaveDatas& rSData );
    static bool HasHiddenRedlines( const SwRedlineSaveDatas& rSData );
    void IgnoreRepeat() { m_isRepeatIgnored = true; }
};

enum class DelContentType : sal_uInt16
{
    Ftn          = 0x01,
    Fly          = 0x02,
    Bkm          = 0x08,
    AllMask      = 0x0b,
    Replace      = 0x10,
    WriterfilterHack = 0x20,
    ExcludeFlyAtStartEnd = 0x40,
    CheckNoCntnt = 0x80,
};
namespace o3tl {
    template<> struct typed_flags<DelContentType> : is_typed_flags<DelContentType, 0xfb> {};
}

/// will DelContentIndex destroy a frame anchored at character at rAnchorPos?
bool IsDestroyFrameAnchoredAtChar(SwPosition const & rAnchorPos,
        SwPosition const & rStart, SwPosition const & rEnd,
        DelContentType const nDelContentType = DelContentType::AllMask);
/// is a fly anchored at paragraph at rAnchorPos selected?
bool IsSelectFrameAnchoredAtPara(SwPosition const & rAnchorPos,
        SwPosition const & rStart, SwPosition const & rEnd,
        DelContentType const nDelContentType = DelContentType::AllMask);
/// check at-char and at-para flys in rDoc
bool IsFlySelectedByCursor(SwDoc const & rDoc,
        SwPosition const & rStart, SwPosition const & rEnd);

// This class has to be inherited into an Undo-object if it saves content
// for Redo/Undo...
class SwUndoSaveContent
{
protected:

    std::unique_ptr<SwHistory> m_pHistory;

    // Needed for deletion of content. For Redo content is moved into the
    // UndoNodesArray. These methods always create a new node to insert
    // content. So the attributes do not get expanded.
    // MoveTo:      moves from the NodesArray into the UndoNodesArray.
    // MoveFrom:    moves from the UndoNodesArray into the NodesArray.
    static void MoveToUndoNds( SwPaM& rPam,
                        SwNodeIndex* pNodeIdx,
                        SwNodeOffset* pEndNdIdx = nullptr );
    static void MoveFromUndoNds( SwDoc& rDoc, SwNodeOffset nNodeIdx,
                          SwPosition& rInsPos,
                          const SwNodeOffset* pEndNdIdx = nullptr,
                          bool bForceCreateFrames = false);

    // These two methods save and restore the Point of PaM.
    // If the point cannot be moved, a "backup" is created on the previous node.
    // Either way, it will not be moved by inserting at its original position.
    static ::std::optional<SwNodeIndex> MovePtBackward(SwPaM& rPam);
    static void MovePtForward(SwPaM& rPam, ::std::optional<SwNodeIndex> && oMvBkwrd);

    // Before moving stuff into UndoNodes-Array care has to be taken that
    // the content-bearing attributes are removed from the nodes-array.
    void DelContentIndex( const SwPosition& pMark, const SwPosition& pPoint,
                        DelContentType nDelContentType = DelContentType::AllMask );

public:
    SwUndoSaveContent();
    virtual ~SwUndoSaveContent() COVERITY_NOEXCEPT_FALSE;
    virtual void dumpAsXml(xmlTextWriterPtr pWriter) const;
};

// Save a complete section in nodes-array.
class SwUndoSaveSection : private SwUndoSaveContent
{
    std::optional<SwNodeIndex> m_oMovedStart;
    std::unique_ptr<SwRedlineSaveDatas> m_pRedlineSaveData;
    SwNodeOffset m_nMoveLen;           // Index into UndoNodes-Array.
    SwNodeOffset m_nStartPos;

protected:
    const SwNodeIndex* GetMvSttIdx() const { return m_oMovedStart ? &*m_oMovedStart : nullptr; }
    SwNodeOffset GetMvNodeCnt() const { return m_nMoveLen; }

public:
    SwUndoSaveSection();
    ~SwUndoSaveSection();

    void SaveSection( const SwNodeIndex& rSttIdx );
    void SaveSection(const SwNodeRange& rRange, bool bExpandNodes = true);
    void RestoreSection( SwDoc* pDoc, SwNodeIndex* pIdx, sal_uInt16 nSectType );
    void RestoreSection(SwDoc* pDoc, const SwNode& rInsPos, bool bForceCreateFrames = false);

    const SwHistory* GetHistory() const { return m_pHistory.get(); }
          SwHistory* GetHistory()       { return m_pHistory.get(); }
    virtual void dumpAsXml(xmlTextWriterPtr pWriter) const;
};

// This class saves the PaM as sal_uInt16's and is able to restore it
// into a PaM.
class SwUndRng
{
public:
    SwNodeOffset m_nSttNode, m_nEndNode;
    sal_Int32 m_nSttContent, m_nEndContent;

    SwUndRng();
    SwUndRng( const SwPaM& );

    void SetValues( const SwPaM& rPam );
    void SetPaM( SwPaM&, bool bCorrToContent = false ) const;
    SwPaM & AddUndoRedoPaM(
        ::sw::UndoRedoContext &, bool const bCorrToContent = false) const;
};

class SwUndoInsLayFormat;

namespace sw {

std::optional<std::vector<SwFrameFormat*>>
GetFlysAnchoredAt(SwDoc & rDoc, SwNodeOffset nSttNode, bool isAtPageIncluded);

}

// base class for insertion of Document, Glossaries and Copy
class SwUndoInserts : public SwUndo, public SwUndRng, private SwUndoSaveContent
{
    SwTextFormatColl *m_pTextFormatColl, *m_pLastNodeColl;
    std::optional<std::vector<SwFrameFormat*>> m_pFrameFormats;
    std::vector< std::shared_ptr<SwUndoInsLayFormat> > m_FlyUndos;
    std::unique_ptr<SwRedlineData> m_pRedlineData;
    SwNodeOffset m_nDeleteTextNodes;
    SwNodeOffset m_nNodeDiff;
    /// start of Content in UndoNodes for Redo
    std::optional<SwNodeIndex> m_oUndoNodeIndex;
    sal_uInt16 m_nSetPos;                 // Start in the history list.

protected:
    SwUndoInserts( SwUndoId nUndoId, const SwPaM& );
public:
    virtual ~SwUndoInserts() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;

    // Set destination range after reading.
    void SetInsertRange( const SwPaM&, bool bScanFlys = true,
                         SwNodeOffset nDeleteTextNodes = SwNodeOffset(1));

    static bool IsCreateUndoForNewFly(SwFormatAnchor const& rAnchor,
        SwNodeOffset const nStartNode, SwNodeOffset const nEndNode);
    std::vector<SwFrameFormat*> * GetFlysAnchoredAt() { return m_pFrameFormats ? &*m_pFrameFormats : nullptr; }

    void dumpAsXml(xmlTextWriterPtr pWriter) const override;
};

/// Undo for Insert -> Text from file.
class SwUndoInsDoc final : public SwUndoInserts
{
public:
    SwUndoInsDoc( const SwPaM& );
};

/// Undo for copying from part of a document and then inserting that text, as opposed to inserting
/// it from a file or clipboard.
class SwUndoCpyDoc final : public SwUndoInserts
{
public:
    SwUndoCpyDoc( const SwPaM& );
};

class SwUndoFlyBase : public SwUndo, private SwUndoSaveSection
{
protected:
    SwFrameFormat* m_pFrameFormat;          // The saved FlyFormat.
    SwNodeOffset m_nNodePagePos;
    sal_Int32 m_nContentPos;         // Page at/in paragraph.
    RndStdIds m_nRndId;
    bool m_bDelFormat;           // Delete saved format.

    void InsFly(::sw::UndoRedoContext & rContext, bool bShowSel = true);
    void DelFly( SwDoc* );

    SwUndoFlyBase( SwFrameFormat* pFormat, SwUndoId nUndoId );

    const SwNodeIndex* GetMvSttIdx() const { return SwUndoSaveSection::GetMvSttIdx(); }
    SwNodeOffset GetMvNodeCnt() const { return SwUndoSaveSection::GetMvNodeCnt(); }

public:
    virtual ~SwUndoFlyBase() override;
    void dumpAsXml(xmlTextWriterPtr pWriter) const override;

};

class SwUndoInsLayFormat final : public SwUndoFlyBase
{
    SwNodeOffset mnCursorSaveIndexPara;        // Cursor position
    sal_Int32 mnCursorSaveIndexPos;            // for undo
public:
    SwUndoInsLayFormat( SwFrameFormat* pFormat, SwNodeOffset nNodeIdx, sal_Int32 nCntIdx );

    virtual ~SwUndoInsLayFormat() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;

    virtual OUString GetComment() const override;

};

class SwUndoDelLayFormat final : public SwUndoFlyBase
{
    bool m_bShowSelFrame;
public:
    SwUndoDelLayFormat( SwFrameFormat* pFormat );

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;

    void RedoForRollback();

    void ChgShowSel( bool bNew ) { m_bShowSelFrame = bNew; }

    virtual SwRewriter GetRewriter() const override;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
