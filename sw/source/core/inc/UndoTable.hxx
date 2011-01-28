/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SW_UNDO_TABLE_HXX
#define SW_UNDO_TABLE_HXX

#include <undobj.hxx>

#ifndef _SVSTDARR_HXX
#define _SVSTDARR_USHORTS
#define _SVSTDARR_ULONGS
#define _SVSTDARR_BOOLS
#define _SVSTDARR_BYTES
#define _SVSTDARR_USHORTSSORT
#include <svl/svstdarr.hxx>
#endif

#include <swtypes.hxx>
#include <itabenum.hxx>


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
    SvUShorts* pColWidth;
    SwRedlineData*  pRedlData;
    SwTableAutoFmt* pAutoFmt;
    ULONG nSttNode;
    USHORT nRows, nCols;
    USHORT nAdjust;

public:
    SwUndoInsTbl( const SwPosition&, USHORT nCols, USHORT nRows,
                    USHORT eAdjust, const SwInsertTableOptions& rInsTblOpts,
                    const SwTableAutoFmt* pTAFmt, const SvUShorts* pColArr,
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
    SvULongs* pDelBoxes;
    SwTableAutoFmt* pAutoFmt;
    SwHistory* pHistory;
    sal_Unicode cTrenner;
    USHORT nAdjust;
    BOOL bSplitEnd : 1;

public:
    SwUndoTxtToTbl( const SwPaM&, const SwInsertTableOptions&, sal_Unicode,
                    USHORT,
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
    ULONG nSttNd, nEndNd;
    USHORT nAdjust;
    sal_Unicode cTrenner;
    USHORT nHdlnRpt;
    BOOL bCheckNumFmt : 1;

public:
    SwUndoTblToTxt( const SwTable& rTbl, sal_Unicode cCh );

    virtual ~SwUndoTblToTxt();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );

    void SetRange( const SwNodeRange& );
    void AddBoxPos( SwDoc& rDoc, ULONG nNdIdx, ULONG nEndIdx,
                    xub_StrLen nCntntIdx = STRING_MAXLEN);
};

class SwUndoAttrTbl : public SwUndo
{
    ULONG nSttNode;
    _SaveTable* pSaveTbl;
    BOOL bClearTabCol : 1;
public:
    SwUndoAttrTbl( const SwTableNode& rTblNd, BOOL bClearTabCols = FALSE );

    virtual ~SwUndoAttrTbl();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
};

class SwUndoTblNumFmt;

class SwUndoTblAutoFmt : public SwUndo
{
    ULONG nSttNode;
    _SaveTable* pSaveTbl;
    ::std::vector< ::boost::shared_ptr<SwUndoTblNumFmt> > m_Undos;
    BOOL bSaveCntntAttr;

    void UndoRedo(bool const bUndo, ::sw::UndoRedoContext & rContext);

public:
    SwUndoTblAutoFmt( const SwTableNode& rTblNd, const SwTableAutoFmt& );

    virtual ~SwUndoTblAutoFmt();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );

    void SaveBoxCntnt( const SwTableBox& rBox );
};

class SwUndoTblNdsChg : public SwUndo
{
    _SaveTable* pSaveTbl;
    SvULongs aBoxes;

    union {
        SvULongs* pNewSttNds;
        SwUndoSaveSections* pDelSects;
    } Ptrs;
    SvBools aMvBoxes;       // for SplitRow (split Nodes of Box)
    long nMin, nMax;        // for redo of delete column
    ULONG nSttNode, nCurrBox;
    USHORT nCount, nRelDiff, nAbsDiff, nSetColType;
    BOOL bFlag;
    BOOL bSameHeight;                   // only used for SplitRow
public:
    SwUndoTblNdsChg( SwUndoId UndoId,
                    const SwSelBoxes& rBoxes,
                    const SwTableNode& rTblNd,
                    long nMn, long nMx,
                    USHORT nCnt, BOOL bFlg, BOOL bSameHeight );

    // for SetColWidth
    SwUndoTblNdsChg( SwUndoId UndoId, const SwSelBoxes& rBoxes,
                    const SwTableNode& rTblNd );

    virtual ~SwUndoTblNdsChg();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );

    void SaveNewBoxes( const SwTableNode& rTblNd, const SwTableSortBoxes& rOld );
    void SaveNewBoxes( const SwTableNode& rTblNd, const SwTableSortBoxes& rOld,
                       const SwSelBoxes& rBoxes, const SvULongs& rNodeCnts );
    void SaveSection( SwStartNode* pSttNd );
    void ReNewBoxes( const SwSelBoxes& rBoxes );


    void SetColWidthParam( ULONG nBoxIdx, USHORT nMode, USHORT nType,
                            SwTwips nAbsDif, SwTwips nRelDif )
    {
        nCurrBox = nBoxIdx;
        nCount = nMode;
        nSetColType = nType;
        nAbsDiff = (USHORT)nAbsDif;
        nRelDiff = (USHORT)nRelDif;
    }

};

class SwUndoTblMerge : public SwUndo, private SwUndRng
{
    ULONG nTblNode;
    _SaveTable* pSaveTbl;
    SvULongs aBoxes, aNewSttNds;
    SwUndoMoves* pMoves;
    SwHistory* pHistory;

public:
    SwUndoTblMerge( const SwPaM& rTblSel );

    virtual ~SwUndoTblMerge();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );

    void MoveBoxCntnt( SwDoc* pDoc, SwNodeRange& rRg, SwNodeIndex& rPos );

    void SetSelBoxes( const SwSelBoxes& rBoxes );

    void AddNewBox( ULONG nSttNdIdx )
        { aNewSttNds.Insert( nSttNdIdx, aNewSttNds.Count() ); }

    void SaveCollection( const SwTableBox& rBox );

};


class SwUndoTblNumFmt : public SwUndo
{
    SfxItemSet *pBoxSet;
    SwHistory* pHistory;
    String aStr, aNewFml;

    ULONG nFmtIdx, nNewFmtIdx;
    double fNum, fNewNum;
    ULONG nNode;
    ULONG nNdPos;

    BOOL bNewFmt : 1;
    BOOL bNewFml : 1;
    BOOL bNewValue : 1;

public:
    SwUndoTblNumFmt( const SwTableBox& rBox, const SfxItemSet* pNewSet = 0 );

    virtual ~SwUndoTblNumFmt();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );

    void SetNumFmt( ULONG nNewNumFmtIdx, const double& rNewNumber )
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

    void AddBoxBefore( const SwTableBox& rBox, BOOL bDelCntnt );
    void AddBoxAfter( const SwTableBox& rBox, const SwNodeIndex& rIdx,
                BOOL bDelCntnt );

    BOOL IsEmpty() const;
    BOOL InsertRow( SwTable& rTbl, const SwSelBoxes& rBoxes, USHORT nCnt );
};

class SwUndoCpyTbl : public SwUndo
{
    SwUndoDelete* pDel;
    ULONG nTblNode;
public:
    SwUndoCpyTbl();

    virtual ~SwUndoCpyTbl();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );

    void SetTableSttIdx( ULONG nIdx )           { nTblNode = nIdx; }
};

class SwUndoSplitTbl : public SwUndo
{
    ULONG nTblNode, nOffset;
    SwSaveRowSpan* mpSaveRowSpan; // stores row span values at the splitting row
    _SaveTable* pSavTbl;
    SwHistory* pHistory;
    USHORT nMode, nFmlEnd;
    BOOL bCalcNewSize;
public:
    SwUndoSplitTbl( const SwTableNode& rTblNd, SwSaveRowSpan* pRowSp,
            USHORT nMode, BOOL bCalcNewSize );

    virtual ~SwUndoSplitTbl();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );

    void SetTblNodeOffset( ULONG nIdx )     { nOffset = nIdx - nTblNode; }
    SwHistory* GetHistory()                 { return pHistory; }
    void SaveFormula( SwHistory& rHistory );
};

class SwUndoMergeTbl : public SwUndo
{
    String aName;
    ULONG nTblNode;
    _SaveTable* pSavTbl, *pSavHdl;
    SwHistory* pHistory;
    USHORT nMode;
    BOOL bWithPrev;
public:
    SwUndoMergeTbl( const SwTableNode& rTblNd, const SwTableNode& rDelTblNd,
                    BOOL bWithPrev, USHORT nMode );

    virtual ~SwUndoMergeTbl();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );

    void SaveFormula( SwHistory& rHistory );
};


//--------------------------------------------------------------------

class SwUndoTblHeadline : public SwUndo
{
    ULONG nTblNd;
    USHORT nOldHeadline;
    USHORT nNewHeadline;
public:
    SwUndoTblHeadline( const SwTable&, USHORT nOldHdl,  USHORT nNewHdl );

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );
};

#endif // SW_UNDO_TABLE_HXX

