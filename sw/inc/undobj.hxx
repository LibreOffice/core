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
#include <tools/solar.h>
#include <SwRewriter.hxx>
#include <swundo.hxx>
#include <o3tl/typed_flags_set.hxx>

class SwHistory;
class SwIndex;
class SwPaM;
struct SwPosition;
class SwDoc;
class SwTextFormatColl;
class SwFrameFormat;
class SwNodeIndex;
class SwNodeRange;
class SwRedlineData;
class SwRedlineSaveDatas;
enum class RedlineFlags;

namespace sw {
    class UndoRedoContext;
    class RepeatContext;
}

class SwUndo
    : public SfxUndoAction
{
    SwUndoId const m_nId;
    RedlineFlags nOrigRedlineFlags;
    sal_Int32 m_nViewShellId;

protected:
    bool bCacheComment;
    mutable std::unique_ptr<OUString> pComment;

    static void RemoveIdxFromSection( SwDoc&, sal_uLong nSttIdx, sal_uLong* pEndIdx = nullptr );
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

    // return type is sal_uInt16 because this overrides SfxUndoAction::GetId()
    virtual sal_uInt16 GetId() const override { return static_cast<sal_uInt16>(m_nId); }

    // the 4 methods that derived classes have to override
    // base implementation does nothing
    virtual void RepeatImpl( ::sw::RepeatContext & );
public: // should not be public, but ran into trouble in untbl.cxx
    virtual void UndoImpl( ::sw::UndoRedoContext & ) = 0;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) = 0;

private:
    /// Try to obtain the view shell ID of the current view.
    static sal_Int32 CreateViewShellId(const SwDoc* pDoc);
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

    /**
       Returns textual comment for this undo object.

       The textual comment is created from the resource string
       corresponding to this object's ID. The rewriter of this object
       is applied to the resource string to get the final comment.

       @return textual comment for this undo object
    */
    virtual OUString GetComment() const override;

    /// See SfxUndoAction::GetViewShellId().
    sal_Int32 GetViewShellId() const override;

    // UndoObject remembers which mode was turned on.
    // In Undo/Redo/Repeat this remembered mode is switched on.
    RedlineFlags GetRedlineFlags() const { return nOrigRedlineFlags; }
    void SetRedlineFlags( RedlineFlags eMode ) { nOrigRedlineFlags = eMode; }

    bool IsDelBox() const;

    // Save and set Redline data.
    static bool FillSaveData( const SwPaM& rRange, SwRedlineSaveDatas& rSData,
                              bool bDelRange = true, bool bCopyNext = true );
    static bool FillSaveDataForFormat( const SwPaM& , SwRedlineSaveDatas& );
    static void SetSaveData( SwDoc& rDoc, SwRedlineSaveDatas& rSData );
    static bool HasHiddenRedlines( const SwRedlineSaveDatas& rSData );
};

enum class DelContentType : sal_uInt16
{
    Ftn          = 0x01,
    Fly          = 0x02,
    Bkm          = 0x08,
    AllMask      = 0x0b,
    CheckNoCntnt = 0x80,
};
namespace o3tl {
    template<> struct typed_flags<DelContentType> : is_typed_flags<DelContentType, 0x8b> {};
}

/// will DelContentIndex destroy a frame anchored at character at rAnchorPos?
bool IsDestroyFrameAnchoredAtChar(SwPosition const & rAnchorPos,
        SwPosition const & rStart, SwPosition const & rEnd, const SwDoc* doc,
        DelContentType const nDelContentType = DelContentType::AllMask);

// This class has to be inherited into an Undo-object if it saves content
// for Redo/Undo...
class SwUndoSaveContent
{
protected:

    std::unique_ptr<SwHistory> pHistory;

    // Needed for deletion of content. For Redo content is moved into the
    // UndoNodesArray. These methods always create a new node to insert
    // content. So the attributes do not get expanded.
    // MoveTo:      moves from the NodesArray into the UndoNodesArray.
    // MoveFrom:    moves from the UndoNodesArray into the NodesArray.
    static void MoveToUndoNds( SwPaM& rPam,
                        SwNodeIndex* pNodeIdx,
                        sal_uLong* pEndNdIdx = nullptr, sal_Int32 * pEndCntIdx = nullptr );
    static void MoveFromUndoNds( SwDoc& rDoc, sal_uLong nNodeIdx,
                          SwPosition& rInsPos,
                          sal_uLong* pEndNdIdx = nullptr, sal_Int32 * pEndCntIdx = nullptr );

    // These two methods move the SPoint back/forth from PaM. With it
    // a range can be spanned for Undo/Redo. (In this case the SPoint
    // is before the manipulated range!!)
    // The flag indicates if there is content before the SPoint.
    static bool MovePtBackward( SwPaM& rPam );
    static void MovePtForward( SwPaM& rPam, bool bMvBkwrd );

    // Before moving stuff into UndoNodes-Array care has to be taken that
    // the content-bearing attributes are removed from the nodes-array.
    void DelContentIndex( const SwPosition& pMark, const SwPosition& pPoint,
                        DelContentType nDelContentType = DelContentType::AllMask );

public:
    SwUndoSaveContent();
    ~SwUndoSaveContent();
};

// Save a complete section in nodes-array.
class SwUndoSaveSection : private SwUndoSaveContent
{
    SwNodeIndex *pMvStt;
    SwRedlineSaveDatas* pRedlSaveData;
    sal_uLong nMvLen;           // Index into UndoNodes-Array.
    sal_uLong nStartPos;

protected:
    SwNodeIndex* GetMvSttIdx() const { return pMvStt; }
    sal_uLong GetMvNodeCnt() const { return nMvLen; }

public:
    SwUndoSaveSection();
    ~SwUndoSaveSection();

    void SaveSection( const SwNodeIndex& rSttIdx );
    void SaveSection( const SwNodeRange& rRange );
    void RestoreSection( SwDoc* pDoc, SwNodeIndex* pIdx, sal_uInt16 nSectType );
    void RestoreSection( SwDoc* pDoc, const SwNodeIndex& rInsPos );

    const SwHistory* GetHistory() const { return pHistory.get(); }
          SwHistory* GetHistory()       { return pHistory.get(); }
};

// This class saves the PaM as sal_uInt16's and is able to restore it
// into a PaM.
class SwUndRng
{
public:
    sal_uLong nSttNode, nEndNode;
    sal_Int32 nSttContent, nEndContent;

    SwUndRng();
    SwUndRng( const SwPaM& );

    void SetValues( const SwPaM& rPam );
    void SetPaM( SwPaM&, bool bCorrToContent = false ) const;
    SwPaM & AddUndoRedoPaM(
        ::sw::UndoRedoContext &, bool const bCorrToContent = false) const;
};

class SwUndoInsLayFormat;

// base class for insertion of Document, Glossaries and Copy
class SwUndoInserts : public SwUndo, public SwUndRng, private SwUndoSaveContent
{
    SwTextFormatColl *pTextFormatColl, *pLastNdColl;
    std::vector<SwFrameFormat*>* pFrameFormats;
    std::vector< std::shared_ptr<SwUndoInsLayFormat> > m_FlyUndos;
    SwRedlineData* pRedlData;
    bool bSttWasTextNd;
protected:
    sal_uLong nNdDiff;
    /// start of Content in UndoNodes for Redo
    std::unique_ptr<SwNodeIndex> m_pUndoNodeIndex;
    sal_uInt16 nSetPos;                 // Start in the history list.

    SwUndoInserts( SwUndoId nUndoId, const SwPaM& );
public:
    virtual ~SwUndoInserts() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;

    // Set destination range after reading.
    void SetInsertRange( const SwPaM&, bool bScanFlys = true,
                         bool bSttWasTextNd = true );
};

class SwUndoInsDoc : public SwUndoInserts
{
public:
    SwUndoInsDoc( const SwPaM& );
};

class SwUndoCpyDoc : public SwUndoInserts
{
public:
    SwUndoCpyDoc( const SwPaM& );
};

class SwUndoFlyBase : public SwUndo, private SwUndoSaveSection
{
protected:
    SwFrameFormat* pFrameFormat;          // The saved FlyFormat.
    sal_uLong nNdPgPos;
    sal_Int32 nCntPos;         // Page at/in paragraph.
    sal_uInt16 nRndId;
    bool bDelFormat;           // Delete saved format.

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
    sal_Int32 mnCursorSaveIndexPos;            // for undo
public:
    SwUndoInsLayFormat( SwFrameFormat* pFormat, sal_uLong nNodeIdx, sal_Int32 nCntIdx );

    virtual ~SwUndoInsLayFormat() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;

    virtual OUString GetComment() const override;

};

class SwUndoDelLayFormat : public SwUndoFlyBase
{
    bool bShowSelFrame;
public:
    SwUndoDelLayFormat( SwFrameFormat* pFormat );

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;

    void RedoForRollback();

    void ChgShowSel( bool bNew ) { bShowSelFrame = bNew; }

    virtual SwRewriter GetRewriter() const override;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
