/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: rolbck.hxx,v $
 * $Revision: 1.13 $
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
#ifndef _ROLBCK_HXX
#define _ROLBCK_HXX

#include <tools/solar.h>
#ifndef _SVSTDARR_HXX
#define _SVSTDARR_USHORTS
#define _SVSTDARR_USHORTSSORT
#include <svtools/svstdarr.hxx>
#endif
#include <svtools/itemset.hxx>

//Nur die History anziehen, um das docnew.cxx gegen die CLOOK's zu behaupten.

class SwDoc;
class SwFmt;
class SwFmtColl;
class SwHstryHint;
class SwTxtAttr;
class SfxPoolItem;
class SwTxtNode;
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
class SwpHints;
class SwFmtChain;
class SwNode;
class SwCharFmt;

#ifndef ROLBCK_HISTORY_ONLY

#include <tox.hxx>

#include <SwNumberTreeTypes.hxx>
// --> OD 2007-10-17 #i81002#
#include <IDocumentMarkAccess.hxx>
// <--

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
    HSTRY_CHGCHARFMT, // #i27615#
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
    virtual String GetDescription() const;
};

class SwSetFmtHint : public SwHstryHint
{
    SfxPoolItem* pAttr;
    ULONG nNode;
public:
    SwSetFmtHint( const SfxPoolItem* pFmtHt, ULONG nNode );
    virtual ~SwSetFmtHint();
    virtual void SetInDoc( SwDoc* pDoc, BOOL bTmpSet );
    virtual String GetDescription() const;

    OUT_HSTR_HINT(SetFmtHnt)
};

class SwResetFmtHint : public SwHstryHint
{
    ULONG nNode;
    USHORT nWhich;
public:
    // --> OD 2008-02-27 #refactorlists# - removed <rDoc>
    SwResetFmtHint( const SfxPoolItem* pFmtHt, ULONG nNodeIdx );
    // <--
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

    virtual String GetDescription() const;

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

    virtual String GetDescription() const;

    OUT_HSTR_HINT(SetFtnHnt)
};

class SwChgFmtColl : public SwHstryHint
{
    const SwFmtColl* pColl;
    ULONG nNode;
    BYTE nNdWhich;
    // --> OD 2007-07-09 #i77372#
    // adjust (naming and type) and extend (add <mbIsCounted>) numbering attributes
    int mnNumLvl;
    bool mbIsRestart;
    SwNumberTree::tSwNumTreeNumber mnRestartVal;
    bool mbIsCounted;
    // <--
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
    public:
        SwHstryBookmark(const ::sw::mark::IMark& rBkmk, bool bSavePos, bool bSaveOtherPos);
        virtual void SetInDoc(SwDoc * pDoc, BOOL);
        OUT_HSTR_HINT(Bookmark)

        BOOL IsEqualBookmark(const ::sw::mark::IMark& rBkmk);
        const ::rtl::OUString& GetName() const;
    private:
        const ::rtl::OUString m_aName;
        ::rtl::OUString m_aShortName;
        KeyCode m_aKeycode;
        const ULONG m_nNode;
        const ULONG m_nOtherNode;
        const xub_StrLen m_nCntnt;
        const xub_StrLen m_nOtherCntnt;
        const bool m_bSavePos;
        const bool m_bSaveOtherPos;
        const bool m_bHadOtherPos;
        const IDocumentMarkAccess::MarkType m_eBkmkType;
};

class SwHstrySetAttrSet : public SwHstryHint
{
    SfxItemSet aOldSet;
    SvUShorts aResetArr;
    ULONG nNode;
    // --> OD 2007-07-09 #i77372#
    // adjust (naming and type) and extend (add <mbIsCounted>) numbering attributes
    int mnNumLvl;
    bool mbIsRestart;
    SwNumberTree::tSwNumTreeNumber mnRestartVal;
    bool mbIsCounted;
    // <--
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

// #i27615#
class SwHstryChgCharFmt : public SwHstryHint
{
    SfxItemSet aOldSet;
    String sFmt;
public:
    SwHstryChgCharFmt( const SfxItemSet& rSet, const String & sFmt);
    virtual void SetInDoc( SwDoc* pDoc, BOOL bTmpSet );
    OUT_HSTR_HINT(SetAttrSet)
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

    // --> OD 2008-02-27 #refactorlists# - removed <rDoc>
    void Add( const SfxPoolItem* pOldValue, const SfxPoolItem* pNewValue,
              ULONG nNodeIdx );
    // <--
    void Add( const SwTxtAttr* pTxtHt, ULONG nNodeIdx,
                BOOL bNewAttr = TRUE );
    void Add( const SwFmtColl*, ULONG nNodeIdx, BYTE nWhichNd );
    void Add( const ::sw::mark::IMark&, bool bSavePos, bool bSaveOtherPos );
    void Add( const SwFrmFmt& rFmt );
    void Add( const SwFlyFrmFmt&, USHORT& rSetPos );
    void Add( const SwTxtFtn& );
    void Add( const SfxItemSet & rSet, const SwCharFmt & rCharFmt); // #i27615#

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
    // --> OD 2008-02-27 #refactorlists# - removed <rDoc>
    void CopyFmtAttr( const SfxItemSet& rSet, ULONG nNodeIdx );
    // <--
};

#ifndef ROLBCK_HISTORY_ONLY

class SwRegHistory : public SwClient
{
    SvUShortsSort aSetWhichIds;
    SwHistory* pHstry;
    ULONG nNodeIdx;

    void _MakeSetWhichIds();
public:
    // --> OD 2008-02-27 #refactorlists# - removed <rDoc>
    SwRegHistory( SwHistory* pHst );
    // <--
    SwRegHistory( SwTxtNode* pTxtNode, const SfxItemSet& rSet,
                xub_StrLen nStart, xub_StrLen nEnd, USHORT nFlags,
                SwHistory* pHst );
    SwRegHistory( const SwNode& rNd, SwHistory* pHst );
    SwRegHistory( SwModify* pRegIn, const SwNode& rNd, SwHistory* pHst );

    virtual void Modify( SfxPoolItem* pOld, SfxPoolItem* pNew );
    void Add( SwTxtAttr* pHt, const BOOL bNew = FALSE );

    void RegisterInModify( SwModify* pRegIn, const SwNode& rNd );
    void ChangeNodeIndex( ULONG nNew ) { nNodeIdx = nNew; }
};

#endif


#endif // _ROLBCK_HXX

