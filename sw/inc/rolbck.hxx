/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _ROLBCK_HXX
#define _ROLBCK_HXX

#include <tools/solar.h>
#ifndef _SVSTDARR_HXX
#define _SVSTDARR_USHORTS
#define _SVSTDARR_USHORTSSORT
#include <svl/svstdarr.hxx>
#endif
#include <svl/itemset.hxx>

//Nur die History anziehen, um das docnew.cxx gegen die CLOOK's zu behaupten.

namespace sfx2 {
    class MetadatableUndo;
}

class SwDoc;
class SwFmt;
class SwFmtColl;
class SwHistoryHint;
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
#include <IDocumentMarkAccess.hxx>

#include <memory>

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
    HSTRY_CHGCHARFMT,
    HSTRY_END
};

class SwHistoryHint
{
    const HISTORY_HINT m_eWhichId;

public:
    SwHistoryHint( HISTORY_HINT eWhich ) : m_eWhichId( eWhich ) {}
    virtual ~SwHistoryHint() {}
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet ) = 0;
    HISTORY_HINT Which() const                     { return m_eWhichId; }
    virtual String GetDescription() const;
};

class SwHistorySetFmt : public SwHistoryHint
{
    ::std::auto_ptr<SfxPoolItem> m_pAttr;
    const ULONG m_nNodeIndex;

public:
    SwHistorySetFmt( const SfxPoolItem* pFmtHt, ULONG nNode );
    virtual ~SwHistorySetFmt();
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet );
    virtual String GetDescription() const;

};

class SwHistoryResetFmt : public SwHistoryHint
{
    const ULONG m_nNodeIndex;
    const USHORT m_nWhich;

public:
    SwHistoryResetFmt( const SfxPoolItem* pFmtHt, ULONG nNodeIdx );
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet );

};

class SwHistorySetTxt : public SwHistoryHint
{
    ::std::auto_ptr<SfxPoolItem> m_pAttr;
    const ULONG m_nNodeIndex;
    const xub_StrLen m_nStart;
    const xub_StrLen m_nEnd;

public:
    SwHistorySetTxt( SwTxtAttr* pTxtHt, ULONG nNode );
    virtual ~SwHistorySetTxt();
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet );

};

class SwHistorySetTxtFld : public SwHistoryHint
{
    //!! beware of the order for the declation of the auto_ptrs.
    //!! If they get destroyed in the wrong order sw may crash (namely mail-merge as well)
    ::std::auto_ptr<SwFieldType> m_pFldType;
    const ::std::auto_ptr<SwFmtFld> m_pFld;

    ULONG m_nNodeIndex;
    xub_StrLen m_nPos;
    USHORT m_nFldWhich;

public:
    SwHistorySetTxtFld( SwTxtFld* pTxtFld, ULONG nNode );
    virtual ~SwHistorySetTxtFld();
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet );

    virtual String GetDescription() const;

};

class SwHistorySetRefMark : public SwHistoryHint
{
    const String m_RefName;
    const ULONG m_nNodeIndex;
    const xub_StrLen m_nStart;
    const xub_StrLen m_nEnd;

public:
    SwHistorySetRefMark( SwTxtRefMark* pTxtHt, ULONG nNode );
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet );

};

class SwHistorySetTOXMark : public SwHistoryHint
{
    SwTOXMark m_TOXMark;
    const String m_TOXName;
    const TOXTypes m_eTOXTypes;
    const ULONG m_nNodeIndex;
    const xub_StrLen m_nStart;
    const xub_StrLen m_nEnd;

public:
    SwHistorySetTOXMark( SwTxtTOXMark* pTxtHt, ULONG nNode );
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet );
    int IsEqual( const SwTOXMark& rCmp ) const;

};

class SwHistoryResetTxt : public SwHistoryHint
{
    const ULONG m_nNodeIndex;
    const xub_StrLen m_nStart;
    const xub_StrLen m_nEnd;
    const USHORT m_nAttr;

public:
    SwHistoryResetTxt( USHORT nWhich, xub_StrLen nStt, xub_StrLen nEnd,
                       ULONG nNode );
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet );

    USHORT GetWhich() const         { return m_nAttr; }
    ULONG GetNode() const           { return m_nNodeIndex; }
    xub_StrLen GetCntnt() const     { return m_nStart; }

};

class SwHistorySetFootnote : public SwHistoryHint
{
    const ::std::auto_ptr<SwUndoSaveSection> m_pUndo;
    const String m_FootnoteNumber;
    ULONG m_nNodeIndex;
    const xub_StrLen m_nStart;
    const bool m_bEndNote;

public:
    SwHistorySetFootnote( SwTxtFtn* pTxtFtn, ULONG nNode );
    SwHistorySetFootnote( const SwTxtFtn& );
    virtual ~SwHistorySetFootnote();
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet );

    virtual String GetDescription() const;

};

class SwHistoryChangeFmtColl : public SwHistoryHint
{
    SwFmtColl * const m_pColl;
    const ULONG m_nNodeIndex;
    const BYTE m_nNodeType;

public:
    SwHistoryChangeFmtColl( SwFmtColl* pColl, ULONG nNode, BYTE nNodeWhich );
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet );

};

class SwHistoryTxtFlyCnt : public SwHistoryHint
{
    ::std::auto_ptr<SwUndoDelLayFmt> m_pUndo;

public:
    SwHistoryTxtFlyCnt( SwFrmFmt* const pFlyFmt );
    virtual ~SwHistoryTxtFlyCnt();
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet );
    SwUndoDelLayFmt* GetUDelLFmt() { return m_pUndo.get(); }

};

class SwHistoryBookmark : public SwHistoryHint
{
    public:
        SwHistoryBookmark(const ::sw::mark::IMark& rBkmk,
                        bool bSavePos, bool bSaveOtherPos);
        virtual void SetInDoc(SwDoc * pDoc, bool);

        bool IsEqualBookmark(const ::sw::mark::IMark& rBkmk);
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
        ::boost::shared_ptr< ::sfx2::MetadatableUndo > m_pMetadataUndo;
};

class SwHistorySetAttrSet : public SwHistoryHint
{
    SfxItemSet m_OldSet;
    SvUShorts m_ResetArray;
    const ULONG m_nNodeIndex;

public:
    SwHistorySetAttrSet( const SfxItemSet& rSet, ULONG nNode,
                         const SvUShortsSort& rSetArr );
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet );

};

class SwHistoryResetAttrSet : public SwHistoryHint
{
    const ULONG m_nNodeIndex;
    const xub_StrLen m_nStart;
    const xub_StrLen m_nEnd;
    SvUShorts m_Array;

public:
    SwHistoryResetAttrSet( const SfxItemSet& rSet, ULONG nNode,
                        xub_StrLen nStt = STRING_MAXLEN,
                        xub_StrLen nEnd = STRING_MAXLEN );
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet );

    const SvUShorts& GetArr() const     { return m_Array; }
    ULONG GetNode() const               { return m_nNodeIndex; }
    xub_StrLen GetCntnt() const         { return m_nStart; }

};

class SwHistoryChangeFlyAnchor : public SwHistoryHint
{
    SwFrmFmt & m_rFmt;
    const ULONG m_nOldNodeIndex;
    const xub_StrLen m_nOldContentIndex;

public:
    SwHistoryChangeFlyAnchor( SwFrmFmt& rFmt );
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet );
};

class SwHistoryChangeFlyChain : public SwHistoryHint
{
    SwFlyFrmFmt * const m_pPrevFmt;
    SwFlyFrmFmt * const m_pNextFmt;
    SwFlyFrmFmt * const m_pFlyFmt;

public:
    SwHistoryChangeFlyChain( SwFlyFrmFmt& rFmt, const SwFmtChain& rAttr );
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet );
};

class SwHistoryChangeCharFmt : public SwHistoryHint
{
    const SfxItemSet m_OldSet;
    const String m_Fmt;

public:
    SwHistoryChangeCharFmt( const SfxItemSet& rSet, const String & sFmt);
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet );

};

#endif

typedef SwHistoryHint* SwHistoryHintPtr;
SV_DECL_PTRARR_DEL( SwpHstry, SwHistoryHintPtr, 0, 2 )

class SwHistory
{
    friend class SwDoc;         // actually only SwDoc::DelUndoObj may access
    friend class SwUndoWriter;  // for Undo/Redo Writer
    friend class SwRegHistory;  // for inserting History attributes

    SwpHstry m_SwpHstry;
    USHORT m_nEndDiff;

public:
    SwHistory( USHORT nInitSz = 0, USHORT nGrowSz = 2 );
    ~SwHistory();

    // delete History from nStart to array end
    void Delete( USHORT nStart = 0 );
    // call and delete all objects between nStart and array end
    bool Rollback( SwDoc* pDoc, USHORT nStart = 0 );
    // call all objects between nStart and TmpEnd; store nStart as TmpEnd
    bool TmpRollback( SwDoc* pDoc, USHORT nStart, bool ToFirst = true );

    void Add( const SfxPoolItem* pOldValue, const SfxPoolItem* pNewValue,
              ULONG nNodeIdx );
    void Add( SwTxtAttr* pTxtHt, ULONG nNodeIdx, bool bNewAttr = true );
    void Add( SwFmtColl*, ULONG nNodeIdx, BYTE nWhichNd );
    void Add( const ::sw::mark::IMark&, bool bSavePos, bool bSaveOtherPos );
    void Add( SwFrmFmt& rFmt );
    void Add( SwFlyFrmFmt&, USHORT& rSetPos );
    void Add( const SwTxtFtn& );
    void Add( const SfxItemSet & rSet, const SwCharFmt & rCharFmt);

    USHORT Count() const { return m_SwpHstry.Count(); }
    USHORT GetTmpEnd() const { return m_SwpHstry.Count() - m_nEndDiff; }
    USHORT SetTmpEnd( USHORT nTmpEnd );        // return previous value
    SwHistoryHint      * operator[]( USHORT nPos ) { return m_SwpHstry[nPos]; }
    SwHistoryHint const* operator[]( USHORT nPos ) const
        { return m_SwpHstry[nPos]; }

    // for SwUndoDelete::Undo/Redo
    void Move( USHORT nPos, SwHistory *pIns,
               USHORT nStart = 0, USHORT nEnd = USHRT_MAX )
    {
        m_SwpHstry.Insert( &pIns->m_SwpHstry, nPos, nStart, nEnd );
        pIns->m_SwpHstry.Remove( nStart, (nEnd == USHRT_MAX)
                                            ? pIns->Count() - nStart
                                            : nEnd );
    }

    // helper methods for recording attribute in History
    // used by Undo classes (Delete/Overwrite/Inserts)
    void CopyAttr( SwpHints* pHts, ULONG nNodeIdx, xub_StrLen nStart,
                    xub_StrLen nEnd, bool bFields );
    void CopyFmtAttr( const SfxItemSet& rSet, ULONG nNodeIdx );
};

#ifndef ROLBCK_HISTORY_ONLY

class SwRegHistory : public SwClient
{
private:
    SvUShortsSort m_WhichIdSet;
    SwHistory * const m_pHistory;
    ULONG m_nNodeIndex;

    void _MakeSetWhichIds();

public:

    SwRegHistory( SwHistory* pHst );
    SwRegHistory( const SwNode& rNd, SwHistory* pHst );
    SwRegHistory( SwModify* pRegIn, const SwNode& rNd, SwHistory* pHst );

    virtual void Modify( SfxPoolItem* pOld, SfxPoolItem* pNew );

    /// @return true if at least 1 item was inserted
    bool InsertItems( const SfxItemSet& rSet,
        xub_StrLen const nStart, xub_StrLen const nEnd,
        SetAttrMode const nFlags );

    void AddHint( SwTxtAttr* pHt, const bool bNew = false );

    void RegisterInModify( SwModify* pRegIn, const SwNode& rNd );
    void ChangeNodeIndex( ULONG nNew ) { m_nNodeIndex = nNew; }
};

#endif

#endif // _ROLBCK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
