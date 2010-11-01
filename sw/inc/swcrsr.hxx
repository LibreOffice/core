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
#ifndef _SWCRSR_HXX
#define _SWCRSR_HXX

#include <com/sun/star/i18n/WordType.hpp>

#include <pam.hxx>
#include <tblsel.hxx>
#include <cshtyp.hxx>


struct _SwCursor_SavePos;

namespace com { namespace sun { namespace star { namespace util {
    struct SearchOptions;
} } } }


// ein Basis-Struktur fuer die Parameter der Find-Methoden
// return - Werte vom Found-Aufruf.
const int FIND_NOT_FOUND    = 0;
const int FIND_FOUND        = 1;
const int FIND_NO_RING      = 2;

struct SwFindParas
{
    virtual int Find( SwPaM*, SwMoveFn, const SwPaM*, BOOL ) = 0;
    virtual int IsReplaceMode() const = 0;
};

typedef USHORT SwCursorSelOverFlags;
namespace nsSwCursorSelOverFlags
{
    const SwCursorSelOverFlags SELOVER_NONE                = 0x00;
    const SwCursorSelOverFlags SELOVER_CHECKNODESSECTION   = 0x01;
    const SwCursorSelOverFlags SELOVER_TOGGLE              = 0x02;
    const SwCursorSelOverFlags SELOVER_ENABLEREVDIREKTION  = 0x04;
    const SwCursorSelOverFlags SELOVER_CHANGEPOS           = 0x08;
}

class SwCursor : public SwPaM
{
    friend class SwCrsrSaveState;

    _SwCursor_SavePos* pSavePos;
    long mnRowSpanOffset;        // required for travelling in tabs with rowspans
    BYTE nCursorBidiLevel;       // bidi level of the cursor
    bool mbColumnSelection;      // true: cursor is aprt of a column selection

    ULONG FindAll( SwFindParas& , SwDocPositions, SwDocPositions, FindRanges, BOOL& bCancel );

    using SwPaM::Find;

protected:
    virtual _SwCursor_SavePos* CreateNewSavePos() const;
    void SaveState();
    void RestoreState();

    const _SwCursor_SavePos* GetSavePos() const { return pSavePos; }

    virtual const SwCntntFrm* DoSetBidiLevelLeftRight(
        BOOL & io_rbLeft, BOOL bVisualAllowed, BOOL bInsertCrsr);
    virtual void DoSetBidiLevelUpDown();
    virtual bool IsSelOvrCheck(int eFlags);

public:
    // single argument ctors shall be explicit.
    SwCursor( const SwPosition &rPos, SwPaM* pRing, bool bColumnSel );
    virtual ~SwCursor();

    // @@@ semantic: no copy ctor.
    SwCursor( SwCursor& rCpy);
private:
    // forbidden and not implemented.
    //SwCursor( const SwCursor& );
    // @@@ used e.g. in core/frmedt/fetab.cxx @@@
    // SwCursor & operator= ( const SwCursor& );
public:

    virtual SwCursor* Create( SwPaM* pRing = 0 ) const;

    virtual short MaxReplaceArived(); //returns RET_YES/RET_CANCEL/RET_NO
    virtual void SaveTblBoxCntnt( const SwPosition* pPos = 0 );

    void FillFindPos( SwDocPositions ePos, SwPosition& rPos ) const;
    SwMoveFnCollection* MakeFindRange( SwDocPositions, SwDocPositions,
                                        SwPaM* ) const;


    ULONG Find( const com::sun::star::util::SearchOptions& rSearchOpt,
                BOOL bSearchInNotes,
                SwDocPositions nStart, SwDocPositions nEnde,
                BOOL& bCancel,
                FindRanges = FND_IN_BODY,
                int bReplace = FALSE );
    ULONG Find( const SwTxtFmtColl& rFmtColl,
                SwDocPositions nStart, SwDocPositions nEnde,
                BOOL& bCancel,
                FindRanges = FND_IN_BODY,
                const SwTxtFmtColl* pReplFmt = 0 );
    ULONG Find( const SfxItemSet& rSet, BOOL bNoCollections,
                SwDocPositions nStart, SwDocPositions nEnde,
                BOOL& bCancel,
                FindRanges = FND_IN_BODY,
                const com::sun::star::util::SearchOptions* pSearchOpt = 0,
                const SfxItemSet* rReplSet = 0 );

    // UI versions
    BOOL IsStartWord( sal_Int16 nWordType = com::sun::star::i18n::WordType::ANYWORD_IGNOREWHITESPACES ) const;
    BOOL IsEndWord( sal_Int16 nWordType = com::sun::star::i18n::WordType::ANYWORD_IGNOREWHITESPACES ) const;
    BOOL IsInWord( sal_Int16 nWordType = com::sun::star::i18n::WordType::ANYWORD_IGNOREWHITESPACES ) const;
    BOOL IsStartEndSentence( bool bEnd ) const;
    BOOL GoStartWord();
    BOOL GoEndWord();
    BOOL GoNextWord();
    BOOL GoPrevWord();
    BOOL SelectWord( ViewShell* pViewShell, const Point* pPt = 0 );

    // API versions of above functions (will be used with a different
    // WordType for the break iterator)
    BOOL IsStartWordWT( sal_Int16 nWordType ) const;
    BOOL IsEndWordWT( sal_Int16 nWordType ) const;
    BOOL IsInWordWT( sal_Int16 nWordType ) const;
    BOOL GoStartWordWT( sal_Int16 nWordType );
    BOOL GoEndWordWT( sal_Int16 nWordType );
    BOOL GoNextWordWT( sal_Int16 nWordType );
    BOOL GoPrevWordWT( sal_Int16 nWordType );
    BOOL SelectWordWT( ViewShell* pViewShell, sal_Int16 nWordType, const Point* pPt = 0 );

    enum SentenceMoveType
    {
        NEXT_SENT,
        PREV_SENT,
        START_SENT,
        END_SENT
    };
    BOOL GoSentence(SentenceMoveType eMoveType);
    BOOL GoNextSentence(){return GoSentence(NEXT_SENT);}
    BOOL GoEndSentence(){return GoSentence(END_SENT);}
    BOOL GoPrevSentence(){return GoSentence(PREV_SENT);}
    BOOL GoStartSentence(){return GoSentence(START_SENT);}
    BOOL ExpandToSentenceBorders();

    virtual BOOL LeftRight( BOOL bLeft, USHORT nCnt, USHORT nMode,
        BOOL bAllowVisual, BOOL bSkipHidden, BOOL bInsertCrsr );
    BOOL UpDown( BOOL bUp, USHORT nCnt, Point* pPt, long nUpDownX );
    BOOL LeftRightMargin( BOOL bLeftMargin, BOOL bAPI = FALSE );
    BOOL IsAtLeftRightMargin( BOOL bLeftMargin, BOOL bAPI = FALSE ) const;
    BOOL SttEndDoc( BOOL bSttDoc );
    BOOL GoPrevNextCell( BOOL bNext, USHORT nCnt );

    BOOL Left( USHORT nCnt, USHORT nMode, BOOL bAllowVisual, BOOL bSkipHidden )
                                    { return LeftRight( TRUE, nCnt, nMode, bAllowVisual, bSkipHidden, FALSE ); }
    BOOL Right( USHORT nCnt, USHORT nMode, BOOL bAllowVisual, BOOL bSkipHidden )
                                    { return LeftRight( FALSE, nCnt, nMode, bAllowVisual, bSkipHidden, FALSE ); }
    BOOL GoNextCell( USHORT nCnt = 1 )  { return GoPrevNextCell( TRUE, nCnt ); }
    BOOL GoPrevCell( USHORT nCnt = 1 )  { return GoPrevNextCell( FALSE, nCnt ); }
    virtual BOOL GotoTable( const String& rName );
    BOOL GotoTblBox( const String& rName );
    BOOL GotoRegion( const String& rName );
    BOOL GotoFtnAnchor();
    BOOL GotoFtnTxt();
    BOOL GotoNextFtnAnchor();
    BOOL GotoPrevFtnAnchor();
    BOOL GotoNextFtnCntnt();
    BOOL GotoPrevFtnCntnt();

    BOOL MovePara( SwWhichPara, SwPosPara );
    BOOL MoveSection( SwWhichSection, SwPosSection );
    BOOL MoveTable( SwWhichTable, SwPosTable );
    BOOL MoveRegion( SwWhichRegion, SwPosRegion );


    // gibt es eine Selection vom Content in die Tabelle
    // Return Wert gibt an, ob der Crsr auf der alten Position verbleibt
    virtual BOOL IsSelOvr( int eFlags =
                                ( nsSwCursorSelOverFlags::SELOVER_CHECKNODESSECTION |
                                  nsSwCursorSelOverFlags::SELOVER_TOGGLE |
                                  nsSwCursorSelOverFlags::SELOVER_CHANGEPOS ));
    virtual BOOL IsInProtectTable( BOOL bMove = FALSE,
                                        BOOL bChgCrsr = TRUE );
    BOOL IsNoCntnt() const;

    void RestoreSavePos();      // Point auf die SavePos setzen

    // TRUE: an die Position kann der Cursor gesetzt werden
    virtual BOOL IsAtValidPos( BOOL bPoint = TRUE ) const;

    // darf der Cursor in ReadOnlyBereiche?
    virtual bool IsReadOnlyAvailable() const;

    virtual BOOL IsSkipOverProtectSections() const;
    virtual BOOL IsSkipOverHiddenSections() const;

    BYTE GetCrsrBidiLevel() const { return nCursorBidiLevel; }
    void SetCrsrBidiLevel( BYTE nNewLevel ) { nCursorBidiLevel = nNewLevel; }

    bool IsColumnSelection() const { return mbColumnSelection; }
    void SetColumnSelection( bool bNew ) { mbColumnSelection = bNew; }

    long GetCrsrRowSpanOffset() const { return mnRowSpanOffset; }
    void SetCrsrRowSpanOffset( long nNew ) { mnRowSpanOffset = nNew; }

    DECL_FIXEDMEMPOOL_NEWDEL( SwCursor )
};


class SwCrsrSaveState
{
    SwCursor& rCrsr;
public:
    SwCrsrSaveState( SwCursor& rC ) : rCrsr( rC ) { rC.SaveState(); }
    ~SwCrsrSaveState() { rCrsr.RestoreState(); }
};

struct _SwCursor_SavePos
{
    ULONG nNode;
    xub_StrLen nCntnt;
    _SwCursor_SavePos* pNext;

    _SwCursor_SavePos( const SwCursor& rCrsr )
        : nNode( rCrsr.GetPoint()->nNode.GetIndex() ),
        nCntnt( rCrsr.GetPoint()->nContent.GetIndex() ),
        pNext( 0 )
    {}
    virtual ~_SwCursor_SavePos() {}

    DECL_FIXEDMEMPOOL_NEWDEL( _SwCursor_SavePos )
};



class SwTableCursor : public virtual SwCursor
{

protected:
    ULONG nTblPtNd, nTblMkNd;
    xub_StrLen nTblPtCnt, nTblMkCnt;
    SwSelBoxes aSelBoxes;
    BOOL bChg : 1;
    BOOL bParked : 1;       // Tabellen-Cursor wurde geparkt

    virtual bool IsSelOvrCheck(int eFlags);

public:
    SwTableCursor( const SwPosition &rPos, SwPaM* pRing = 0 );
    SwTableCursor( SwTableCursor& );
    virtual ~SwTableCursor();

    virtual BOOL LeftRight( BOOL bLeft, USHORT nCnt, USHORT nMode,
        BOOL bAllowVisual, BOOL bSkipHidden, BOOL bInsertCrsr );
    virtual BOOL GotoTable( const String& rName );

    void InsertBox( const SwTableBox& rTblBox );
    void DeleteBox( USHORT nPos ) { aSelBoxes.Remove( nPos ); bChg = TRUE; }
    USHORT GetBoxesCount() const { return aSelBoxes.Count(); }
    const SwSelBoxes& GetBoxes() const { return aSelBoxes; }

        // Baut fuer alle Boxen die Cursor auf
    SwCursor* MakeBoxSels( SwCursor* pAktCrsr );
        // sind irgendwelche Boxen mit einem Schutz versehen?
    BOOL HasReadOnlyBoxSel() const;

        // wurde der TabelleCursor veraendert ? Wenn ja speicher gleich
        // die neuen Werte.
    BOOL IsCrsrMovedUpdt();
        // wurde der TabelleCursor veraendert ?
    BOOL IsCrsrMoved() const
    {
        return  nTblMkNd != GetMark()->nNode.GetIndex() ||
                nTblPtNd != GetPoint()->nNode.GetIndex() ||
                nTblMkCnt != GetMark()->nContent.GetIndex() ||
                nTblPtCnt != GetPoint()->nContent.GetIndex();
    }

    BOOL IsChgd() const { return bChg; }

    // Parke den Tabellen-Cursor auf dem StartNode der Boxen.
    void ParkCrsr();

    bool NewTableSelection();
    void ActualizeSelection( const SwSelBoxes &rBoxes );
};

#endif

