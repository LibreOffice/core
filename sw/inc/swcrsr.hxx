/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: swcrsr.hxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-22 15:28:27 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SWCRSR_HXX
#define _SWCRSR_HXX
#ifndef _COM_SUN_STAR_I18N_WORDTYPE_HPP_
#include <com/sun/star/i18n/WordType.hpp>
#endif

#ifndef _PAM_HXX //autogen
#include <pam.hxx>
#endif
#ifndef _TBLSEL_HXX //autogen
#include <tblsel.hxx>
#endif
#ifndef _CSHTYP_HXX //autogen
#include <cshtyp.hxx>
#endif


class SwShellCrsr;
class SwShellTableCrsr;
class SwTableCursor;
class SwUnoCrsr;
class SwUnoTableCrsr;
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

    virtual operator SwShellCrsr* ();
    virtual operator SwShellTableCrsr* ();
    virtual operator SwTableCursor* ();
    virtual operator SwUnoCrsr* ();
    virtual operator SwUnoTableCrsr* ();

    inline operator const SwShellCrsr* () const;
    inline operator const SwShellTableCrsr* () const;
    inline operator const SwTableCursor* () const;
    inline operator const SwUnoCrsr* () const;
    inline operator const SwUnoTableCrsr* () const;

    virtual short MaxReplaceArived(); //returns RET_YES/RET_CANCEL/RET_NO
    virtual void SaveTblBoxCntnt( const SwPosition* pPos = 0 );

    void FillFindPos( SwDocPositions ePos, SwPosition& rPos ) const;
    SwMoveFnCollection* MakeFindRange( SwDocPositions, SwDocPositions,
                                        SwPaM* ) const;


    ULONG Find( const com::sun::star::util::SearchOptions& rSearchOpt,
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
    BOOL IsStartWord() const;
    BOOL IsEndWord() const;
    BOOL IsStartEndSentence( bool bEnd ) const;
    BOOL IsInWord() const;
    BOOL GoStartWord();
    BOOL GoEndWord();
    BOOL GoNextWord();
    BOOL GoPrevWord();
    BOOL SelectWord( const Point* pPt = 0 );

    // API versions of above functions (will be used with a different
    // WordType for the break iterator)
    BOOL IsStartWordWT( sal_Int16 nWordType ) const;
    BOOL IsEndWordWT( sal_Int16 nWordType ) const;
    BOOL IsInWordWT( sal_Int16 nWordType ) const;
    BOOL GoStartWordWT( sal_Int16 nWordType );
    BOOL GoEndWordWT( sal_Int16 nWordType );
    BOOL GoNextWordWT( sal_Int16 nWordType );
    BOOL GoPrevWordWT( sal_Int16 nWordType );
    BOOL SelectWordWT( sal_Int16 nWordType, const Point* pPt = 0 );

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

    BOOL LeftRight( BOOL bLeft, USHORT nCnt, USHORT nMode, BOOL bAllowVisual, BOOL bSkipHidden,
                        BOOL bInsertCrsr );
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
    BOOL GotoTable( const String& rName );
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
    BOOL IsReadOnlyAvailable() const;

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

public:
    SwTableCursor( const SwPosition &rPos, SwPaM* pRing = 0 );
    SwTableCursor( SwTableCursor& );
    virtual ~SwTableCursor();

    virtual operator SwTableCursor* ();

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


// --------------------------- inline Methoden ----------------------

inline SwCursor::operator const SwShellCrsr* () const
{
    return (SwShellCrsr*)*((SwCursor*)this);
}

inline SwCursor::operator const SwShellTableCrsr* () const
{
    return (SwShellTableCrsr*)*((SwCursor*)this);
}

inline SwCursor::operator const SwTableCursor* () const
{
    return (SwTableCursor*)*((SwCursor*)this);
}

inline SwCursor::operator const SwUnoCrsr* () const
{
    return (SwUnoCrsr*)*((SwCursor*)this);
}

inline SwCursor::operator const SwUnoTableCrsr* () const
{
    return (SwUnoTableCrsr*)*((SwCursor*)this);
}


#endif

