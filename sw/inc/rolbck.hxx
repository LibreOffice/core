/*************************************************************************
 *
 *  $RCSfile: rolbck.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:27 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _ROLBCK_HXX
#define _ROLBCK_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _SVSTDARR_HXX
#define _SVSTDARR_USHORTS
#define _SVSTDARR_USHORTSSORT
#include <svtools/svstdarr.hxx>
#endif

#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif

//Nur die History anziehen, um das docnew.cxx gegen die CLOOK's zu behaupten.

class SwDoc;
class SwFmt;
class SwFmtColl;
class SwHstryHint;
class SwBookmark;
class SwTxtAttr;
class SfxPoolItem;
class SwTxtNode;
class SwCntntNode;
class SwUndoSaveSection;
class SwTxtFtn;
class SwTxtFlyCnt;
class SwUndoDelLayFmt;
class SwFlyFrmFmt;
class SwFmtFld;
class SwTxtFld;
class SwFieldType;
class SwTxtTOXMark;
class SwTxtRefMark;
class SwFrmFmt;
class SwNodeIndex;
class SwpHints;
class SwFmtChain;
class SwNode;

#ifndef ROLBCK_HISTORY_ONLY

#ifndef _TOX_HXX
#include <tox.hxx>
#endif

#ifndef PRODUCT
class Writer;
#define OUT_HSTR_HINT( name )   \
    friend Writer& OutUndo_Hstr_ ## name( Writer&, const SwHstryHint& );
#else
#define OUT_HSTR_HINT( name )
#endif

enum HISTORY_HINT {
    HSTRY_SETFMTHNT,
    HSTRY_RESETFMTHNT,
    HSTRY_SETTXTHNT,
    HSTRY_SETTXTFLDHNT,
    HSTRY_SETREFMARKHNT,
    HSTRY_SETTOXMARKHNT,
    HSTRY_RESETTXTHNT,
    HSTRY_SETFTNHNT,
    HSTRY_CHGFMTCOLL,
    HSTRY_FLYCNT,
    HSTRY_BOOKMARK,
    HSTRY_SETATTRSET,
    HSTRY_RESETATTRSET,
    HSTRY_CHGFLYANCHOR,
    HSTRY_CHGFLYCHAIN,
    HSTRY_END
};
class SwHstryHint
{
    HISTORY_HINT eWhichId;
public:
    SwHstryHint( HISTORY_HINT eWh ) : eWhichId( eWh ) {}
    virtual ~SwHstryHint() {}       // jetzt inline
    virtual void SetInDoc( SwDoc* pDoc, BOOL bTmpSet ) = 0;
    HISTORY_HINT Which() const                  { return eWhichId; }
};

class SwSetFmtHint : public SwHstryHint
{
    SfxPoolItem* pAttr;
    ULONG nNode;
    USHORT nSetStt;
    BYTE nNumLvl;
    BOOL bNumStt;
public:
    SwSetFmtHint( const SfxPoolItem* pFmtHt, ULONG nNode );
    virtual ~SwSetFmtHint();
    virtual void SetInDoc( SwDoc* pDoc, BOOL bTmpSet );
    OUT_HSTR_HINT(SetFmtHnt)
};

class SwResetFmtHint : public SwHstryHint
{
    ULONG nNode;
    USHORT nWhich;
public:
    SwResetFmtHint( const SfxPoolItem* pFmtHt, ULONG nNode );
    virtual void SetInDoc( SwDoc* pDoc, BOOL bTmpSet );
    OUT_HSTR_HINT(ResetFmtHnt)
};

class SwSetTxtHint : public SwHstryHint
{
    SfxPoolItem *pAttr;
    ULONG nNode;
    xub_StrLen nStart, nEnd;
public:
    SwSetTxtHint( /*const*/ SwTxtAttr* pTxtHt, ULONG nNode );
    virtual ~SwSetTxtHint();
    virtual void SetInDoc( SwDoc* pDoc, BOOL bTmpSet );
    OUT_HSTR_HINT(SetTxtHnt)
};

class SwSetTxtFldHint : public SwHstryHint
{
    SwFmtFld* pFld;
    SwFieldType* pFldType;
    ULONG nNode;
    xub_StrLen nPos;
    USHORT nFldWhich;
public:
    SwSetTxtFldHint( SwTxtFld* pTxtFld, ULONG nNode );
    virtual ~SwSetTxtFldHint();
    virtual void SetInDoc( SwDoc* pDoc, BOOL bTmpSet );
    OUT_HSTR_HINT(SetTxtFldHnt)
};

class SwSetRefMarkHint : public SwHstryHint
{
    String aRefName;
    ULONG nNode;
    xub_StrLen nStart, nEnd;
public:
    SwSetRefMarkHint( SwTxtRefMark* pTxtHt, ULONG nNode );
    virtual void SetInDoc( SwDoc* pDoc, BOOL bTmpSet );
    OUT_HSTR_HINT(SetRefMarkHnt)
};

class SwSetTOXMarkHint : public SwHstryHint
{
    String aTOXName;
    TOXTypes eTOXTypes;
    SwTOXMark aTOXMark;
    ULONG nNode;
    xub_StrLen nStart, nEnd;
public:
    SwSetTOXMarkHint( SwTxtTOXMark* pTxtHt, ULONG nNode );
    virtual void SetInDoc( SwDoc* pDoc, BOOL bTmpSet );
    int IsEqual( const SwTOXMark& rCmp ) const;
    OUT_HSTR_HINT(SetToxMarkHnt)
};

class SwResetTxtHint : public SwHstryHint
{
    ULONG nNode;
    xub_StrLen nStart, nEnd;
    USHORT nAttr;
public:
    SwResetTxtHint( USHORT nWhich, xub_StrLen nStt, xub_StrLen nEnd, ULONG nNode );
    virtual void SetInDoc( SwDoc* pDoc, BOOL bTmpSet );

    USHORT GetWhich() const         { return nAttr; }
    ULONG GetNode() const           { return nNode; }
    xub_StrLen GetCntnt() const     { return nStart; }
    OUT_HSTR_HINT(ResetTxtHnt)
};

class SwSetFtnHint : public SwHstryHint
{
    String aFtnStr;
    SwUndoSaveSection* pUndo;
    ULONG nNode;
    xub_StrLen nStart;
    BOOL   bEndNote;
public:
    SwSetFtnHint( SwTxtFtn* pTxtFtn, ULONG nNode );
    SwSetFtnHint( const SwTxtFtn& );
    ~SwSetFtnHint();
    virtual void SetInDoc( SwDoc* pDoc, BOOL bTmpSet );

    const SwUndoSaveSection* GetUndoObj() const { return pUndo; }
          SwUndoSaveSection* GetUndoObj()       { return pUndo; }
    OUT_HSTR_HINT(SetFtnHnt)
};

class SwChgFmtColl : public SwHstryHint
{
    const SwFmtColl* pColl;
    ULONG nNode;
    USHORT nSetStt;
    BYTE nNdWhich, nNumLvl;
    BOOL bNumStt;
public:
    SwChgFmtColl( const SwFmtColl* pColl, ULONG nNode, BYTE nNodeWhich );
    virtual void SetInDoc( SwDoc* pDoc, BOOL bTmpSet );
    OUT_HSTR_HINT(ChgFmtColl)
};

class SwHstryTxtFlyCnt : public SwHstryHint
{
    SwUndoDelLayFmt* pUndo;
public:
    SwHstryTxtFlyCnt( SwTxtFlyCnt* pTxtFly );       // fuer Zeichengebundene
    SwHstryTxtFlyCnt( SwFlyFrmFmt* pFlyFmt );       // fuer Absatzgebundene
    ~SwHstryTxtFlyCnt();
    virtual void SetInDoc( SwDoc* pDoc, BOOL bTmpSet );
    SwUndoDelLayFmt* GetUDelLFmt() { return pUndo; }
    OUT_HSTR_HINT(FlyCnt)
};

class SwHstryBookmark : public SwHstryHint
{
    String aName, aShortName;
    ULONG nNode1, nNode2;
    xub_StrLen nCntnt1, nCntnt2;
    USHORT nKeyCode;
    BYTE nTyp;
public:
    enum { BKMK_POS = 1, BKMK_OTHERPOS = 2 };
    SwHstryBookmark( const SwBookmark&, BYTE nTyp );
    virtual void SetInDoc( SwDoc* pDoc, BOOL bTmpSet );
    OUT_HSTR_HINT(Bookmark)

    BOOL IsEqualBookmark( const SwBookmark& );
};

class SwHstrySetAttrSet : public SwHstryHint
{
    SfxItemSet aOldSet;
    SvUShorts aResetArr;
    ULONG nNode;
    USHORT nSetStt;
    BYTE nNumLvl;
    BOOL bNumStt;
public:
    SwHstrySetAttrSet( const SfxItemSet& rSet, ULONG nNode,
                        const SvUShortsSort& rSetArr );
    virtual void SetInDoc( SwDoc* pDoc, BOOL bTmpSet );
    OUT_HSTR_HINT(SetAttrSet)
};


class SwHstryResetAttrSet : public SwHstryHint
{
    ULONG nNode;
    xub_StrLen nStart, nEnd;
    SvUShorts aArr;
public:
    SwHstryResetAttrSet( const SfxItemSet& rSet, ULONG nNode,
                        xub_StrLen nStt = STRING_MAXLEN,
                        xub_StrLen nEnd = STRING_MAXLEN );
    virtual void SetInDoc( SwDoc* pDoc, BOOL bTmpSet );

    const SvUShorts& GetArr() const     { return aArr; }
    ULONG GetNode() const               { return nNode; }
    xub_StrLen GetCntnt() const         { return nStart; }
    OUT_HSTR_HINT(ResetAttrSet)
};

class SwHstryChgFlyAnchor : public SwHstryHint
{
    SwFrmFmt* pFmt;
    ULONG nOldPos;
    xub_StrLen nOldCnt;
public:
    SwHstryChgFlyAnchor( const SwFrmFmt& rFmt );
    virtual void SetInDoc( SwDoc* pDoc, BOOL bTmpSet );
};

class SwHstryChgFlyChain : public SwHstryHint
{
    SwFlyFrmFmt *pPrevFmt, *pNextFmt, *pFlyFmt;
public:
    SwHstryChgFlyChain( const SwFlyFrmFmt& rFmt, const SwFmtChain& rAttr );
    virtual void SetInDoc( SwDoc* pDoc, BOOL bTmpSet );
};


#endif

typedef SwHstryHint* SwHstryHintPtr;
SV_DECL_PTRARR_DEL( SwpHstry, SwHstryHintPtr, 0, 2 )
class SwHistory : private SwpHstry
{
    friend class SwDoc;     // eig. darf nur SwDoc::DelUndoObj zugreifen
    friend class SwUndoWriter;  // fuer den Undo/Redo-Writer
    friend class SwRegHistory;  // fuer Insert, von Histoy-Attributen

    USHORT nEndDiff;
public:
    SwHistory( USHORT nInitSz = 0, USHORT nGrowSz = 2 );
    ~SwHistory();
    // loesche die History ab Start bis zum Ende
    void Delete( USHORT nStart = 0 );
    // alle Objecte zwischen nStart und Array-Ende aufrufen und loeschen
    BOOL Rollback( SwDoc* pDoc, USHORT nStart = 0 );
    // alle Objecte zwischen nStart und temporaeren Ende aufrufen und
    // den Start als temporaeres Ende speichern
    BOOL TmpRollback( SwDoc* pDoc, USHORT nStart, BOOL ToFirst = TRUE );

    void Add( const SfxPoolItem* pOldValue, const SfxPoolItem* pNewValue,
                ULONG nNodeIdx );
    void Add( const SwTxtAttr* pTxtHt, ULONG nNodeIdx,
                BOOL bNewAttr = TRUE );
    void Add( const SwFmtColl*, ULONG nNodeIdx, BYTE nWhichNd );
    void Add( const SwFmt*, ULONG nNodeIdx, BYTE nWhichNd );
    void Add( const SwBookmark&, BYTE );
    void Add( const SwFrmFmt& rFmt );
    void Add( const SwFlyFrmFmt&, USHORT& rSetPos );
    void Add( const SwTxtFtn& );

    USHORT Count() const { return SwpHstry::Count(); }
    USHORT GetTmpEnd() const { return SwpHstry::Count() - nEndDiff; }
    USHORT SetTmpEnd( USHORT nTmpEnd );     // returne alten Wert
    SwHstryHint* operator[]( USHORT nPos ) const { return SwpHstry::operator[](nPos); }

    // fuer SwUndoDelete::Undo/Redo
    void Move( USHORT nPos, SwHistory *pIns,
                USHORT nStart = 0, USHORT nEnd = USHRT_MAX )
    {
        SwpHstry::Insert( pIns, nPos, nStart, nEnd );
        SwpHstry* pDel = pIns;
        pDel->Remove( nStart,  nEnd == USHRT_MAX
                                    ? pDel->Count() - nStart
                                    : nEnd );
    }

        // Hilfs-Methoden zum Sichern von Attributen in der History.
        // Wird von UndoKlasse benutzt (Delete/Overwrite/Inserts)
    void CopyAttr( const SwpHints* pHts, ULONG nNodeIdx, xub_StrLen nStt,
                    xub_StrLen nEnd, BOOL bFields );
    void CopyFmtAttr( const SfxItemSet& rSet, ULONG nNodeIdx );
};

#ifndef ROLBCK_HISTORY_ONLY

class SwRegHistory : public SwClient
{
    SvUShortsSort aSetWhichIds;
    SwHistory* pHstry;
    const SwCntntNode * pNode;
    ULONG nNodeIdx;

    void _MakeSetWhichIds();
public:
    SwRegHistory( SwHistory* pHst );
    SwRegHistory( SwTxtNode* pTxtNd, SwTxtAttr* pTxtHt,
                    USHORT nFlags, SwHistory* pHst );
    SwRegHistory( SwTxtNode* pTxtNode, const SfxItemSet& rSet,
                xub_StrLen nStart, xub_StrLen nEnd, USHORT nFlags,
                SwHistory* pHst );
    SwRegHistory( const SwNode& rNd, SwHistory* pHst );
    SwRegHistory( SwModify* pRegIn, const SwNode& rNd, SwHistory* pHst );

    virtual void Modify( SfxPoolItem* pOld, SfxPoolItem* pNew );
    void Add( SwTxtAttr* pHt, const BOOL bNew = FALSE );

    void RegisterInModify( SwModify* pRegIn, const SwNode& rNd );
};

#endif


#endif // _ROLBCK_HXX

