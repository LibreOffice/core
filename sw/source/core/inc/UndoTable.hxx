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

#ifndef SW_UNDO_TABLE_HXX
#define SW_UNDO_TABLE_HXX

#include <undobj.hxx>
#include <memory>
#include <vector>
#include <set>
#include <swtypes.hxx>
#include <itabenum.hxx>
#include "boost/noncopyable.hpp"
#include "boost/scoped_ptr.hpp"
#include <tools/string.hxx>

class SfxItemSet;

struct SwSaveRowSpan;
class _SaveTable;
class SwDDEFieldType;
class SwUndoSaveSections;
class SwUndoMoves;
class SwUndoDelete;
class SwSelBoxes;
class SwTblToTxtSaves;
class SwTable;
class SwTableBox;
class SwStartNode;
class SwTableNode;
class SwTableAutoFmt;
class SwTableSortBoxes;

class SwUndoInsTbl : public SwUndo
{
    String sTblNm;
    SwInsertTableOptions aInsTblOpts;
    SwDDEFieldType* pDDEFldType;
    std::vector<sal_uInt16> *pColWidth;
    SwRedlineData*  pRedlData;
    SwTableAutoFmt* pAutoFmt;
    sal_uLong nSttNode;
    sal_uInt16 nRows, nCols;
    sal_uInt16 nAdjust;

public:
    SwUndoInsTbl( const SwPosition&, sal_uInt16 nCols, sal_uInt16 nRows,
                    sal_uInt16 eAdjust, const SwInsertTableOptions& rInsTblOpts,
                    const SwTableAutoFmt* pTAFmt, const std::vector<sal_uInt16> *pColArr,
                  const String & rName);

    virtual ~SwUndoInsTbl();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );

    virtual SwRewriter GetRewriter() const;
};

class SwUndoTxtToTbl : public SwUndo, public SwUndRng
{
    String sTblNm;
    SwInsertTableOptions aInsTblOpts;
    std::vector<sal_uLong>* pDelBoxes;
    SwTableAutoFmt* pAutoFmt;
    SwHistory* pHistory;
    sal_Unicode cTrenner;
    sal_uInt16 nAdjust;
    sal_Bool bSplitEnd : 1;

public:
    SwUndoTxtToTbl( const SwPaM&, const SwInsertTableOptions&, sal_Unicode,
                    sal_uInt16,
                    const SwTableAutoFmt* pAFmt );

    virtual ~SwUndoTxtToTbl();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );

    SwHistory& GetHistory(); // will be created if necessary
    void AddFillBox( const SwTableBox& rBox );
};

class SwUndoTblToTxt : public SwUndo
{
    String sTblNm;
    SwDDEFieldType* pDDEFldType;
    _SaveTable* pTblSave;
    SwTblToTxtSaves* pBoxSaves;
    SwHistory* pHistory;
    sal_uLong nSttNd, nEndNd;
    sal_uInt16 nAdjust;
    sal_Unicode cTrenner;
    sal_uInt16 nHdlnRpt;
    sal_Bool bCheckNumFmt : 1;

public:
    SwUndoTblToTxt( const SwTable& rTbl, sal_Unicode cCh );

    virtual ~SwUndoTblToTxt();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );

    void SetRange( const SwNodeRange& );
    void AddBoxPos( SwDoc& rDoc, sal_uLong nNdIdx, sal_uLong nEndIdx,
                    xub_StrLen nCntntIdx = STRING_MAXLEN);
};

class SwUndoAttrTbl : public SwUndo
{
    sal_uLong nSttNode;
    _SaveTable* pSaveTbl;
    sal_Bool bClearTabCol : 1;

public:
    SwUndoAttrTbl( const SwTableNode& rTblNd, sal_Bool bClearTabCols = sal_False );

    virtual ~SwUndoAttrTbl();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
};

class SwUndoTblNumFmt;

class SwUndoTblAutoFmt : public SwUndo
{
    sal_uLong nSttNode;
    _SaveTable* pSaveTbl;
    ::std::vector< ::boost::shared_ptr<SwUndoTblNumFmt> > m_Undos;
    sal_Bool bSaveCntntAttr;
    sal_uInt16 m_nRepeatHeading;

    void UndoRedo(bool const bUndo, ::sw::UndoRedoContext & rContext);

public:
    SwUndoTblAutoFmt( const SwTableNode& rTblNd, const SwTableAutoFmt& );

    virtual ~SwUndoTblAutoFmt();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );

    void SaveBoxCntnt( const SwTableBox& rBox );
};

class SwUndoTblNdsChg : public SwUndo, private boost::noncopyable
{
    _SaveTable* pSaveTbl;
    std::set<sal_uLong> m_Boxes;
    struct _BoxMove
    {
        sal_uLong index;    ///< Index of this box.
        bool      hasMoved; ///< Has this box been moved already.
        _BoxMove(sal_uLong idx, bool moved=false) : index(idx), hasMoved(moved) {};
        bool operator<(const _BoxMove other) const { return index < other.index; };
    };
    boost::scoped_ptr< std::set<_BoxMove> > pNewSttNds;
    boost::scoped_ptr< SwUndoSaveSections > pDelSects;
    long nMin, nMax;        // for redo of delete column
    sal_uLong nSttNode, nCurrBox;
    sal_uInt16 nCount, nRelDiff, nAbsDiff, nSetColType;
    sal_Bool bFlag;
    sal_Bool bSameHeight;                   // only used for SplitRow

public:
    SwUndoTblNdsChg( SwUndoId UndoId,
                    const SwSelBoxes& rBoxes,
                    const SwTableNode& rTblNd,
                    long nMn, long nMx,
                    sal_uInt16 nCnt, sal_Bool bFlg, sal_Bool bSameHeight );

    // for SetColWidth
    SwUndoTblNdsChg( SwUndoId UndoId, const SwSelBoxes& rBoxes,
                    const SwTableNode& rTblNd );

    virtual ~SwUndoTblNdsChg();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );

    void SaveNewBoxes( const SwTableNode& rTblNd, const SwTableSortBoxes& rOld );
    void SaveNewBoxes( const SwTableNode& rTblNd, const SwTableSortBoxes& rOld,
                       const SwSelBoxes& rBoxes, const std::vector<sal_uLong> &rNodeCnts );
    void SaveSection( SwStartNode* pSttNd );
    void ReNewBoxes( const SwSelBoxes& rBoxes );

    void SetColWidthParam( sal_uLong nBoxIdx, sal_uInt16 nMode, sal_uInt16 nType,
                            SwTwips nAbsDif, SwTwips nRelDif )
    {
        nCurrBox = nBoxIdx;
        nCount = nMode;
        nSetColType = nType;
        nAbsDiff = (sal_uInt16)nAbsDif;
        nRelDiff = (sal_uInt16)nRelDif;
    }
};

class SwUndoTblMerge : public SwUndo, private SwUndRng
{
    sal_uLong nTblNode;
    _SaveTable* pSaveTbl;
    std::set<sal_uLong> m_Boxes;
    std::vector<sal_uLong> aNewSttNds;
    SwUndoMoves* pMoves;
    SwHistory* pHistory;

public:
    SwUndoTblMerge( const SwPaM& rTblSel );

    virtual ~SwUndoTblMerge();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );

    void MoveBoxCntnt( SwDoc* pDoc, SwNodeRange& rRg, SwNodeIndex& rPos );

    void SetSelBoxes( const SwSelBoxes& rBoxes );

    void AddNewBox( sal_uLong nSttNdIdx )
        { aNewSttNds.push_back( nSttNdIdx ); }

    void SaveCollection( const SwTableBox& rBox );
};

class SwUndoTblNumFmt : public SwUndo
{
    SfxItemSet *pBoxSet;
    SwHistory* pHistory;
    String aStr, aNewFml;

    sal_uLong nFmtIdx, nNewFmtIdx;
    double fNum, fNewNum;
    sal_uLong nNode;
    sal_uLong nNdPos;

    sal_Bool bNewFmt : 1;
    sal_Bool bNewFml : 1;
    sal_Bool bNewValue : 1;

public:
    SwUndoTblNumFmt( const SwTableBox& rBox, const SfxItemSet* pNewSet = 0 );

    virtual ~SwUndoTblNumFmt();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );

    void SetNumFmt( sal_uLong nNewNumFmtIdx, const double& rNewNumber )
            { nFmtIdx = nNewNumFmtIdx; fNum = rNewNumber; }
    void SetBox( const SwTableBox& rBox );
};

class _UndoTblCpyTbl_Entries;

class SwUndoTblCpyTbl : public SwUndo
{
    _UndoTblCpyTbl_Entries* pArr;
    SwUndoTblNdsChg* pInsRowUndo;

    //b6341295: When redlining is active, PrepareRedline has to create the
    //redlining attributes for the new and the old table cell content
    SwUndo* PrepareRedline( SwDoc* pDoc, const SwTableBox& rBox,
                const SwPosition& rPos, bool& rJoin, bool bRedo );

public:
    SwUndoTblCpyTbl();

    virtual ~SwUndoTblCpyTbl();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );

    void AddBoxBefore( const SwTableBox& rBox, bool bDelCntnt );
    void AddBoxAfter( const SwTableBox& rBox, const SwNodeIndex& rIdx,
                bool bDelCntnt );

    sal_Bool IsEmpty() const;
    bool InsertRow( SwTable& rTbl, const SwSelBoxes& rBoxes, sal_uInt16 nCnt );
};

class SwUndoCpyTbl : public SwUndo
{
    SwUndoDelete* pDel;
    sal_uLong nTblNode;

public:
    SwUndoCpyTbl();

    virtual ~SwUndoCpyTbl();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );

    void SetTableSttIdx( sal_uLong nIdx )           { nTblNode = nIdx; }
};

class SwUndoSplitTbl : public SwUndo
{
    sal_uLong nTblNode, nOffset;
    SwSaveRowSpan* mpSaveRowSpan; // stores row span values at the splitting row
    _SaveTable* pSavTbl;
    SwHistory* pHistory;
    sal_uInt16 nMode, nFmlEnd;
    sal_Bool bCalcNewSize;

public:
    SwUndoSplitTbl( const SwTableNode& rTblNd, SwSaveRowSpan* pRowSp,
            sal_uInt16 nMode, sal_Bool bCalcNewSize );

    virtual ~SwUndoSplitTbl();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );

    void SetTblNodeOffset( sal_uLong nIdx )     { nOffset = nIdx - nTblNode; }
    SwHistory* GetHistory()                 { return pHistory; }
    void SaveFormula( SwHistory& rHistory );
};

class SwUndoMergeTbl : public SwUndo
{
    String aName;
    sal_uLong nTblNode;
    _SaveTable* pSavTbl, *pSavHdl;
    SwHistory* pHistory;
    sal_uInt16 nMode;
    sal_Bool bWithPrev;

public:
    SwUndoMergeTbl( const SwTableNode& rTblNd, const SwTableNode& rDelTblNd,
                    sal_Bool bWithPrev, sal_uInt16 nMode );

    virtual ~SwUndoMergeTbl();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );

    void SaveFormula( SwHistory& rHistory );
};

class SwUndoTblHeadline : public SwUndo
{
    sal_uLong nTblNd;
    sal_uInt16 nOldHeadline;
    sal_uInt16 nNewHeadline;

public:
    SwUndoTblHeadline( const SwTable&, sal_uInt16 nOldHdl,  sal_uInt16 nNewHdl );

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );
};

#endif // SW_UNDO_TABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
