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

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include <svl/undo.hxx>

#include <SwRewriter.hxx>
#include <swundo.hxx>

class SwHistory;
class SwIndex;
class SwPaM;
struct SwPosition;
class SwDoc;
class SwTxtFmtColl;
class SwFrmFmt;
class SwNodeIndex;
class SwNodeRange;
class SwRedlineData;
class SwRedlineSaveDatas;

namespace sw {
    class UndoRedoContext;
    class RepeatContext;
}

class SwUndo
    : public SfxUndoAction
{
    SwUndoId const m_nId;
    sal_uInt16 nOrigRedlineMode;

protected:
    bool bCacheComment;
    mutable OUString * pComment;

    void RemoveIdxFromSection( SwDoc&, sal_uLong nSttIdx, sal_uLong* pEndIdx = 0 );
    void RemoveIdxFromRange( SwPaM& rPam, bool bMoveNext );
    void RemoveIdxRel( sal_uLong, const SwPosition& );

    static bool CanRedlineGroup( SwRedlineSaveDatas& rCurr,
                                const SwRedlineSaveDatas& rCheck,
                                bool bCurrIsEnd );

    /**
       Returns the rewriter for this object.

       @return the rewriter for this object
    */
    virtual SwRewriter GetRewriter() const;

    // return type is sal_uInt16 because this overrides SfxUndoAction::GetId()
    virtual sal_uInt16 GetId() const SAL_OVERRIDE { return static_cast<sal_uInt16>(m_nId); }

    // the 4 methods that derived classes have to override
    // base implementation does nothing
    virtual void RepeatImpl( ::sw::RepeatContext & );
    virtual bool CanRepeatImpl( ::sw::RepeatContext & ) const;
public: // should not be public, but ran into trouble in untbl.cxx
    virtual void UndoImpl( ::sw::UndoRedoContext & ) = 0;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) = 0;

private:
    // SfxUndoAction
    virtual void Undo() SAL_OVERRIDE;
    virtual void Redo() SAL_OVERRIDE;
    virtual void UndoWithContext(SfxUndoContext &) SAL_OVERRIDE;
    virtual void RedoWithContext(SfxUndoContext &) SAL_OVERRIDE;
    virtual void Repeat(SfxRepeatTarget &) SAL_OVERRIDE;
    virtual bool CanRepeat(SfxRepeatTarget &) const SAL_OVERRIDE;

public:
    SwUndo(SwUndoId const nId);
    virtual ~SwUndo();

    /**
       Returns textual comment for this undo object.

       The textual comment is created from the resource string
       corresponding to this object's ID. The rewriter of this object
       is applied to the resource string to get the final comment.

       @return textual comment for this undo object
    */
    virtual OUString GetComment() const SAL_OVERRIDE;

    // UndoObject remembers which mode was turned on.
    // In Undo/Redo/Repeat this remembered mode is switched on.
    sal_uInt16 GetRedlineMode() const { return nOrigRedlineMode; }
    void SetRedlineMode( sal_uInt16 eMode ) { nOrigRedlineMode = eMode; }

    bool IsDelBox() const;

    // Save and set Redline data.
    static bool FillSaveData( const SwPaM& rRange, SwRedlineSaveDatas& rSData,
                              bool bDelRange = true, bool bCopyNext = true );
    static bool FillSaveDataForFmt( const SwPaM& , SwRedlineSaveDatas& );
    static void SetSaveData( SwDoc& rDoc, SwRedlineSaveDatas& rSData );
    static bool HasHiddenRedlines( const SwRedlineSaveDatas& rSData );
};

typedef sal_uInt16 DelCntntType;
namespace nsDelCntntType
{
    const DelCntntType DELCNT_FTN = 0x01;
    const DelCntntType DELCNT_FLY = 0x02;
    const DelCntntType DELCNT_TOC = 0x04;
    const DelCntntType DELCNT_BKM = 0x08;
    const DelCntntType DELCNT_ALL = 0x0F;
    const DelCntntType DELCNT_CHKNOCNTNT = 0x80;
}

/// will DelCntntIndex destroy a frame anchored at character at rAnchorPos?
bool IsDestroyFrameAnchoredAtChar(SwPosition const & rAnchorPos,
        SwPosition const & rStart, SwPosition const & rEnd, const SwDoc* doc,
        DelCntntType const nDelCntntType = nsDelCntntType::DELCNT_ALL);

// This class has to be inherited into an Undo-object if it saves content
// for Redo/Undo...
class SwUndoSaveCntnt
{
protected:

    SwHistory* pHistory;

    // Needed for deletion of content. For Redo content is moved into the
    // UndoNodesArray. These methods always create a new node to insert
    // content. So the attributes do not get expanded.
    // MoveTo:      moves from the NodesArray into the UndoNodesArray.
    // MoveFrom:    moves from the UndoNodesArray into the NodesArray.
    void MoveToUndoNds( SwPaM& rPam,
                        SwNodeIndex* pNodeIdx = 0,
                        sal_uLong* pEndNdIdx = 0, sal_Int32 * pEndCntIdx = 0 );
    void MoveFromUndoNds( SwDoc& rDoc, sal_uLong nNodeIdx,
                          SwPosition& rInsPos,
                          sal_uLong* pEndNdIdx = 0, sal_Int32 * pEndCntIdx = 0 );

    // These two methods move the SPoint back/forth from PaM. With it
    // a range can be spanned for Undo/Redo. (In this case the SPoint
    // is before the manipulated range!!)
    // The flag indicates if there is content before the SPoint.
    bool MovePtBackward( SwPaM& rPam );
    void MovePtForward( SwPaM& rPam, bool bMvBkwrd );

    // Before moving stuff into UndoNodes-Array care has to be taken that
    // the content-bearing attributes are removed from the nodes-array.
    void DelCntntIndex( const SwPosition& pMark, const SwPosition& pPoint,
                        DelCntntType nDelCntntType = nsDelCntntType::DELCNT_ALL );

public:
    SwUndoSaveCntnt();
    ~SwUndoSaveCntnt();
};

// Save a complete section in nodes-array.
class SwUndoSaveSection : private SwUndoSaveCntnt
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

    void SaveSection( SwDoc* pDoc, const SwNodeIndex& rSttIdx );
    void SaveSection( SwDoc* pDoc, const SwNodeRange& rRange );
    void RestoreSection( SwDoc* pDoc, SwNodeIndex* pIdx, sal_uInt16 nSectType );
    void RestoreSection( SwDoc* pDoc, const SwNodeIndex& rInsPos );

    const SwHistory* GetHistory() const { return pHistory; }
          SwHistory* GetHistory()       { return pHistory; }
};

// This class saves the PaM as sal_uInt16's and is able to restore it
// into a PaM.
class SwUndRng
{
public:
    sal_uLong nSttNode, nEndNode;
    sal_Int32 nSttCntnt, nEndCntnt;

    SwUndRng();
    SwUndRng( const SwPaM& );

    void SetValues( const SwPaM& rPam );
    void SetPaM( SwPaM&, bool bCorrToCntnt = false ) const;
    SwPaM & AddUndoRedoPaM(
        ::sw::UndoRedoContext &, bool const bCorrToCntnt = false) const;
};

class SwUndoInsLayFmt;

// base class for insertion of Document, Glossaries and Copy
class SwUndoInserts : public SwUndo, public SwUndRng, private SwUndoSaveCntnt
{
    SwTxtFmtColl *pTxtFmtColl, *pLastNdColl;
    std::vector<SwFrmFmt*>* pFrmFmts;
    ::std::vector< ::boost::shared_ptr<SwUndoInsLayFmt> > m_FlyUndos;
    SwRedlineData* pRedlData;
    bool bSttWasTxtNd;
protected:
    sal_uLong nNdDiff;
    /// start of Content in UndoNodes for Redo
    ::boost::scoped_ptr<SwNodeIndex> m_pUndoNodeIndex;
    sal_uInt16 nSetPos;                 // Start in the history list.

    SwUndoInserts( SwUndoId nUndoId, const SwPaM& );
public:
    virtual ~SwUndoInserts();

    virtual void UndoImpl( ::sw::UndoRedoContext & ) SAL_OVERRIDE;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) SAL_OVERRIDE;
    virtual void RepeatImpl( ::sw::RepeatContext & ) SAL_OVERRIDE;

    // Set destination range after reading.
    void SetInsertRange( const SwPaM&, bool bScanFlys = true,
                         bool bSttWasTxtNd = true );
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
    SwFrmFmt* pFrmFmt;          // The saved FlyFormat.
    sal_uLong nNdPgPos;
    sal_Int32 nCntPos;         // Page at/in paragraph.
    sal_uInt16 nRndId;
    bool bDelFmt;           // Delete saved format.

    void InsFly(::sw::UndoRedoContext & rContext, bool bShowSel = true);
    void DelFly( SwDoc* );

    SwUndoFlyBase( SwFrmFmt* pFormat, SwUndoId nUndoId );

    SwNodeIndex* GetMvSttIdx() const { return SwUndoSaveSection::GetMvSttIdx(); }
    sal_uLong GetMvNodeCnt() const { return SwUndoSaveSection::GetMvNodeCnt(); }

public:
    virtual ~SwUndoFlyBase();

};

class SwUndoInsLayFmt : public SwUndoFlyBase
{
    sal_uLong mnCrsrSaveIndexPara;           // Cursor position
    sal_Int32 mnCrsrSaveIndexPos;            // for undo
public:
    SwUndoInsLayFmt( SwFrmFmt* pFormat, sal_uLong nNodeIdx, sal_Int32 nCntIdx );

    virtual ~SwUndoInsLayFmt();

    virtual void UndoImpl( ::sw::UndoRedoContext & ) SAL_OVERRIDE;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) SAL_OVERRIDE;
    virtual void RepeatImpl( ::sw::RepeatContext & ) SAL_OVERRIDE;

    virtual OUString GetComment() const SAL_OVERRIDE;

};

class SwUndoDelLayFmt : public SwUndoFlyBase
{
    bool bShowSelFrm;
public:
    SwUndoDelLayFmt( SwFrmFmt* pFormat );

    virtual void UndoImpl( ::sw::UndoRedoContext & ) SAL_OVERRIDE;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) SAL_OVERRIDE;

    void RedoForRollback();

    void ChgShowSel( bool bNew ) { bShowSelFrm = bNew; }

    virtual SwRewriter GetRewriter() const SAL_OVERRIDE;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
