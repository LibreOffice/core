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
#include <svl/itemset.hxx>

#include <vector>
#include <set>

//Nur die History anziehen, um das docnew.cxx gegen die CLOOK's zu behaupten.

namespace sfx2 {
    class MetadatableUndo;
}

class SwDoc;
class SwFmtColl;
class SwHistoryHint;
class SwTxtAttr;
class SfxPoolItem;
class SwUndoSaveSection;
class SwTxtFtn;
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
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<SfxPoolItem> m_pAttr;
    SAL_WNODEPRECATED_DECLARATIONS_POP
    const sal_uLong m_nNodeIndex;

public:
    SwHistorySetFmt( const SfxPoolItem* pFmtHt, sal_uLong nNode );
    virtual ~SwHistorySetFmt();
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet );
    virtual String GetDescription() const;

};

class SwHistoryResetFmt : public SwHistoryHint
{
    const sal_uLong m_nNodeIndex;
    const sal_uInt16 m_nWhich;

public:
    SwHistoryResetFmt( const SfxPoolItem* pFmtHt, sal_uLong nNodeIdx );
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet );

};

class SwHistorySetTxt : public SwHistoryHint
{
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<SfxPoolItem> m_pAttr;
    SAL_WNODEPRECATED_DECLARATIONS_POP
    const sal_uLong m_nNodeIndex;
    const xub_StrLen m_nStart;
    const xub_StrLen m_nEnd;

public:
    SwHistorySetTxt( SwTxtAttr* pTxtHt, sal_uLong nNode );
    virtual ~SwHistorySetTxt();
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet );

};

class SwHistorySetTxtFld : public SwHistoryHint
{
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    //!! beware of the order for the declation of the auto_ptrs.
    //!! If they get destroyed in the wrong order sw may crash (namely mail-merge as well)
    ::std::auto_ptr<SwFieldType> m_pFldType;
    const ::std::auto_ptr<SwFmtFld> m_pFld;
    SAL_WNODEPRECATED_DECLARATIONS_POP

    sal_uLong m_nNodeIndex;
    xub_StrLen m_nPos;
    sal_uInt16 m_nFldWhich;

public:
    SwHistorySetTxtFld( SwTxtFld* pTxtFld, sal_uLong nNode );
    virtual ~SwHistorySetTxtFld();
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet );

    virtual String GetDescription() const;

};

class SwHistorySetRefMark : public SwHistoryHint
{
    const String m_RefName;
    const sal_uLong m_nNodeIndex;
    const xub_StrLen m_nStart;
    const xub_StrLen m_nEnd;

public:
    SwHistorySetRefMark( SwTxtRefMark* pTxtHt, sal_uLong nNode );
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet );

};

class SwHistorySetTOXMark : public SwHistoryHint
{
    SwTOXMark m_TOXMark;
    const String m_TOXName;
    const TOXTypes m_eTOXTypes;
    const sal_uLong m_nNodeIndex;
    const xub_StrLen m_nStart;
    const xub_StrLen m_nEnd;

public:
    SwHistorySetTOXMark( SwTxtTOXMark* pTxtHt, sal_uLong nNode );
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet );
    int IsEqual( const SwTOXMark& rCmp ) const;

};

class SwHistoryResetTxt : public SwHistoryHint
{
    const sal_uLong m_nNodeIndex;
    const xub_StrLen m_nStart;
    const xub_StrLen m_nEnd;
    const sal_uInt16 m_nAttr;

public:
    SwHistoryResetTxt( sal_uInt16 nWhich, xub_StrLen nStt, xub_StrLen nEnd,
                       sal_uLong nNode );
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet );

    sal_uInt16 GetWhich() const         { return m_nAttr; }
    sal_uLong GetNode() const           { return m_nNodeIndex; }
    xub_StrLen GetCntnt() const     { return m_nStart; }

};

class SwHistorySetFootnote : public SwHistoryHint
{
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    const ::std::auto_ptr<SwUndoSaveSection> m_pUndo;
    SAL_WNODEPRECATED_DECLARATIONS_POP
    const String m_FootnoteNumber;
    sal_uLong m_nNodeIndex;
    const xub_StrLen m_nStart;
    const bool m_bEndNote;

public:
    SwHistorySetFootnote( SwTxtFtn* pTxtFtn, sal_uLong nNode );
    SwHistorySetFootnote( const SwTxtFtn& );
    virtual ~SwHistorySetFootnote();
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet );

    virtual String GetDescription() const;

};

class SwHistoryChangeFmtColl : public SwHistoryHint
{
    SwFmtColl * const m_pColl;
    const sal_uLong m_nNodeIndex;
    const sal_uInt8 m_nNodeType;

public:
    SwHistoryChangeFmtColl( SwFmtColl* pColl, sal_uLong nNode, sal_uInt8 nNodeWhich );
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet );

};

class SwHistoryTxtFlyCnt : public SwHistoryHint
{
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<SwUndoDelLayFmt> m_pUndo;
    SAL_WNODEPRECATED_DECLARATIONS_POP

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
        const sal_uLong m_nNode;
        const sal_uLong m_nOtherNode;
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
    std::vector<sal_uInt16> m_ResetArray;
    const sal_uLong m_nNodeIndex;

public:
    SwHistorySetAttrSet( const SfxItemSet& rSet, sal_uLong nNode,
                         const std::set<sal_uInt16> &rSetArr );
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet );

};

class SwHistoryResetAttrSet : public SwHistoryHint
{
    const sal_uLong m_nNodeIndex;
    const xub_StrLen m_nStart;
    const xub_StrLen m_nEnd;
    std::vector<sal_uInt16> m_Array;

public:
    SwHistoryResetAttrSet( const SfxItemSet& rSet, sal_uLong nNode,
                        xub_StrLen nStt = STRING_MAXLEN,
                        xub_StrLen nEnd = STRING_MAXLEN );
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet );

    const std::vector<sal_uInt16>& GetArr() const { return m_Array; }
    sal_uLong GetNode() const               { return m_nNodeIndex; }
    xub_StrLen GetCntnt() const         { return m_nStart; }

};

class SwHistoryChangeFlyAnchor : public SwHistoryHint
{
    SwFrmFmt & m_rFmt;
    const sal_uLong m_nOldNodeIndex;
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

class SwpHstry : public std::vector<SwHistoryHint*> {
public:
    // the destructor will free all objects still in the vector
    ~SwpHstry();
};

class SwHistory
{
    friend class SwDoc;         // actually only SwDoc::DelUndoObj may access
    friend class SwRegHistory;  // for inserting History attributes

    SwpHstry m_SwpHstry;
    sal_uInt16 m_nEndDiff;

public:
    SwHistory( sal_uInt16 nInitSz = 0 );
    ~SwHistory();

    // delete History from nStart to array end
    void Delete( sal_uInt16 nStart = 0 );
    // call and delete all objects between nStart and array end
    bool Rollback( SwDoc* pDoc, sal_uInt16 nStart = 0 );
    // call all objects between nStart and TmpEnd; store nStart as TmpEnd
    bool TmpRollback( SwDoc* pDoc, sal_uInt16 nStart, bool ToFirst = true );

    void Add( const SfxPoolItem* pOldValue, const SfxPoolItem* pNewValue,
              sal_uLong nNodeIdx );
    void Add( SwTxtAttr* pTxtHt, sal_uLong nNodeIdx, bool bNewAttr = true );
    void Add( SwFmtColl*, sal_uLong nNodeIdx, sal_uInt8 nWhichNd );
    void Add( const ::sw::mark::IMark&, bool bSavePos, bool bSaveOtherPos );
    void Add( SwFrmFmt& rFmt );
    void Add( SwFlyFrmFmt&, sal_uInt16& rSetPos );
    void Add( const SwTxtFtn& );
    void Add( const SfxItemSet & rSet, const SwCharFmt & rCharFmt);

    sal_uInt16 Count() const { return m_SwpHstry.size(); }
    sal_uInt16 GetTmpEnd() const { return m_SwpHstry.size() - m_nEndDiff; }
    sal_uInt16 SetTmpEnd( sal_uInt16 nTmpEnd );        // return previous value
    SwHistoryHint      * operator[]( sal_uInt16 nPos ) { return m_SwpHstry[nPos]; }
    SwHistoryHint const* operator[]( sal_uInt16 nPos ) const
        { return m_SwpHstry[nPos]; }

    // for SwUndoDelete::Undo/Redo
    void Move( sal_uInt16 nPos, SwHistory *pIns,
               sal_uInt16 const nStart = 0)
    {
        SwpHstry::iterator itSourceBegin = pIns->m_SwpHstry.begin() + nStart;
        SwpHstry::iterator itSourceEnd = pIns->m_SwpHstry.end();
        if (itSourceBegin == itSourceEnd) return;
        m_SwpHstry.insert(m_SwpHstry.begin() + nPos, itSourceBegin, itSourceEnd);
        pIns->m_SwpHstry.erase( itSourceBegin, itSourceEnd );
    }

    // helper methods for recording attribute in History
    // used by Undo classes (Delete/Overwrite/Inserts)
    void CopyAttr( SwpHints* pHts, sal_uLong nNodeIdx, xub_StrLen nStart,
                    xub_StrLen nEnd, bool bFields );
    void CopyFmtAttr( const SfxItemSet& rSet, sal_uLong nNodeIdx );
};

#ifndef ROLBCK_HISTORY_ONLY

class SwRegHistory : public SwClient
{
private:
    std::set<sal_uInt16> m_WhichIdSet;
    SwHistory * const m_pHistory;
    sal_uLong m_nNodeIndex;

    void _MakeSetWhichIds();

protected:
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew );

public:
    SwRegHistory( SwHistory* pHst );
    SwRegHistory( const SwNode& rNd, SwHistory* pHst );
    SwRegHistory( SwModify* pRegIn, const SwNode& rNd, SwHistory* pHst );

    /// @return true if at least 1 item was inserted
    bool InsertItems( const SfxItemSet& rSet,
        xub_StrLen const nStart, xub_StrLen const nEnd,
        SetAttrMode const nFlags );

    void AddHint( SwTxtAttr* pHt, const bool bNew = false );

    void RegisterInModify( SwModify* pRegIn, const SwNode& rNd );
    void ChangeNodeIndex( sal_uLong nNew ) { m_nNodeIndex = nNew; }
};

#endif

#endif // _ROLBCK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
