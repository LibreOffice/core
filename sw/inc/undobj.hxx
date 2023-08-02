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
#include <optional>

#include <svl/undo.hxx>
#include <tools/solar.h>
#include "SwRewriter.hxx"
#include "swundo.hxx"
#include <o3tl/typed_flags_set.hxx>
#include <boost/optional.hpp>

class SwHistory;
class SwPaM;
struct SwPosition;
class SwDoc;
class SwTextFormatColl;
class SwFrameFormat;
class SwFormatAnchor;
class SwNodeIndex;
class SwNodeRange;
class SwRedlineData;
class SwRedlineSaveDatas;
enum class RedlineFlags;
enum class RndStdIds;

namespace sw {
    class UndoRedoContext;
    class RepeatContext;
}

class SwUndo
    : public SfxUndoAction
{
    SwUndoId const m_nId;
    RedlineFlags   m_nOrigRedlineFlags;
    ViewShellId const    m_nViewShellId;
    bool m_isRepeatIgnored; ///< for multi-selection, only repeat 1st selection

protected:
    bool m_bCacheComment;
    mutable boost::optional<OUString> maComment;

    static void RemoveIdxFromSection( SwDoc&, sal_uLong nSttIdx, const sal_uLong* pEndIdx = nullptr );
    static void RemoveIdxFromRange( SwPaM& rPam, bool bMoveNext );
    static void RemoveIdxRel( sal_uLong, const SwPosition& );

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
                        sal_uLong* pEndNdIdx = nullptr );
    static void MoveFromUndoNds( SwDoc& rDoc, sal_uLong nNodeIdx,
                          SwPosition& rInsPos,
                          const sal_uLong* pEndNdIdx = nullptr,
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
    ~SwUndoSaveContent() COVERITY_NOEXCEPT_FALSE;
};

// Save a complete section in nodes-array.
class SwUndoSaveSection : private SwUndoSaveContent
{
    std::unique_ptr<SwNodeIndex> m_pMovedStart;
    std::unique_ptr<SwRedlineSaveDatas> m_pRedlineSaveData;
    sal_uLong m_nMoveLen;           // Index into UndoNodes-Array.
    sal_uLong m_nStartPos;

protected:
    SwNodeIndex* GetMvSttIdx() const { return m_pMovedStart.get(); }
    sal_uLong GetMvNodeCnt() const { return m_nMoveLen; }

public:
    SwUndoSaveSection();
    ~SwUndoSaveSection();

    void SaveSection( const SwNodeIndex& rSttIdx );
    void SaveSection(const SwNodeRange& rRange, bool bExpandNodes = true);
    void RestoreSection( SwDoc* pDoc, SwNodeIndex* pIdx, sal_uInt16 nSectType );
    void RestoreSection(SwDoc* pDoc, const SwNodeIndex& rInsPos, bool bForceCreateFrames = false);

    const SwHistory* GetHistory() const { return m_pHistory.get(); }
          SwHistory* GetHistory()       { return m_pHistory.get(); }
};

// This class saves the PaM as sal_uInt16's and is able to restore it
// into a PaM.
class SwUndRng
{
public:
    sal_uLong m_nSttNode, m_nEndNode;
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

std::unique_ptr<std::vector<SwFrameFormat*>>
GetFlysAnchoredAt(SwDoc & rDoc, sal_uLong nSttNode);

}

// base class for insertion of Document, Glossaries and Copy
class SwUndoInserts : public SwUndo, public SwUndRng, private SwUndoSaveContent
{
    SwTextFormatColl *m_pTextFormatColl, *m_pLastNodeColl;
    std::unique_ptr<std::vector<SwFrameFormat*>> m_pFrameFormats;
    std::vector< std::shared_ptr<SwUndoInsLayFormat> > m_FlyUndos;
    std::unique_ptr<SwRedlineData> m_pRedlineData;
    int m_nDeleteTextNodes;

protected:
    sal_uLong m_nNodeDiff;
    /// start of Content in UndoNodes for Redo
    std::unique_ptr<SwNodeIndex> m_pUndoNodeIndex;
    sal_uInt16 m_nSetPos;                 // Start in the history list.

    SwUndoInserts( SwUndoId nUndoId, const SwPaM& );
public:
    virtual ~SwUndoInserts() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;

    // Set destination range after reading.
    void SetInsertRange( const SwPaM&, bool bScanFlys = true,
                         int nDeleteTextNodes = 1);

    static bool IsCreateUndoForNewFly(SwFormatAnchor const& rAnchor,
        sal_uLong const nStartNode, sal_uLong const nEndNode);
    std::vector<SwFrameFormat*> * GetFlysAnchoredAt() { return m_pFrameFormats.get(); }
};

class SwUndoInsDoc final : public SwUndoInserts
{
public:
    SwUndoInsDoc( const SwPaM& );
};

class SwUndoCpyDoc final : public SwUndoInserts
{
public:
    SwUndoCpyDoc( const SwPaM& );
};

class SwUndoFlyBase : public SwUndo, private SwUndoSaveSection
{
protected:
    SwFrameFormat* m_pFrameFormat;          // The saved FlyFormat.
    sal_uLong m_nNodePagePos;
    sal_Int32 m_nContentPos;         // Page at/in paragraph.
    RndStdIds m_nRndId;
    bool m_bDelFormat;           // Delete saved format.

    void InsFly(::sw::UndoRedoContext & rContext, bool bShowSel = true);
    void DelFly( SwDoc* );

    SwUndoFlyBase( SwFrameFormat* pFormat, SwUndoId nUndoId );

    SwNodeIndex* GetMvSttIdx() const { return SwUndoSaveSection::GetMvSttIdx(); }
    sal_uLong GetMvNodeCnt() const { return SwUndoSaveSection::GetMvNodeCnt(); }

public:
    virtual ~SwUndoFlyBase() override;

};

class SwUndoInsLayFormat : public SwUndoFlyBase
{
    sal_uLong mnCursorSaveIndexPara;           // Cursor position
    sal_Int32 const mnCursorSaveIndexPos;            // for undo
public:
    SwUndoInsLayFormat( SwFrameFormat* pFormat, sal_uLong nNodeIdx, sal_Int32 nCntIdx );

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
