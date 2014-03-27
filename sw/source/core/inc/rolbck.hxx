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
#ifndef INCLUDED_SW_SOURCE_CORE_INC_ROLBCK_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_ROLBCK_HXX

#include <tools/solar.h>
#include <svl/itemset.hxx>

#include <vector>
#include <set>

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
    virtual OUString GetDescription() const;
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
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet ) SAL_OVERRIDE;
    virtual OUString GetDescription() const SAL_OVERRIDE;

};

class SwHistoryResetFmt : public SwHistoryHint
{
    const sal_uLong m_nNodeIndex;
    const sal_uInt16 m_nWhich;

public:
    SwHistoryResetFmt( const SfxPoolItem* pFmtHt, sal_uLong nNodeIdx );
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet ) SAL_OVERRIDE;

};

class SwHistorySetTxt : public SwHistoryHint
{
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<SfxPoolItem> m_pAttr;
    SAL_WNODEPRECATED_DECLARATIONS_POP
    const sal_uLong m_nNodeIndex;
    const sal_Int32 m_nStart;
    const sal_Int32 m_nEnd;
    bool m_bFormatIgnoreStart : 1;
    bool m_bFormatIgnoreEnd   : 1;

public:
    SwHistorySetTxt( SwTxtAttr* pTxtHt, sal_uLong nNode );
    virtual ~SwHistorySetTxt();
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet ) SAL_OVERRIDE;

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
    sal_Int32 m_nPos;
    sal_uInt16 m_nFldWhich;

public:
    SwHistorySetTxtFld( SwTxtFld* pTxtFld, sal_uLong nNode );
    virtual ~SwHistorySetTxtFld();
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet ) SAL_OVERRIDE;

    virtual OUString GetDescription() const SAL_OVERRIDE;

};

class SwHistorySetRefMark : public SwHistoryHint
{
    const OUString m_RefName;
    const sal_uLong m_nNodeIndex;
    const sal_Int32 m_nStart;
    const sal_Int32 m_nEnd;

public:
    SwHistorySetRefMark( SwTxtRefMark* pTxtHt, sal_uLong nNode );
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet ) SAL_OVERRIDE;

};

class SwHistorySetTOXMark : public SwHistoryHint
{
    SwTOXMark m_TOXMark;
    const OUString m_TOXName;
    const TOXTypes m_eTOXTypes;
    const sal_uLong m_nNodeIndex;
    const sal_Int32 m_nStart;
    const sal_Int32 m_nEnd;

public:
    SwHistorySetTOXMark( SwTxtTOXMark* pTxtHt, sal_uLong nNode );
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet ) SAL_OVERRIDE;
    bool IsEqual( const SwTOXMark& rCmp ) const;

};

class SwHistoryResetTxt : public SwHistoryHint
{
    const sal_uLong m_nNodeIndex;
    const sal_Int32 m_nStart;
    const sal_Int32 m_nEnd;
    const sal_uInt16 m_nAttr;

public:
    SwHistoryResetTxt( sal_uInt16 nWhich, sal_Int32 nStt, sal_Int32 nEnd,
                       sal_uLong nNode );
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet ) SAL_OVERRIDE;

    sal_uInt16 GetWhich() const         { return m_nAttr; }
    sal_uLong GetNode() const           { return m_nNodeIndex; }
    sal_Int32 GetCntnt() const     { return m_nStart; }

};

class SwHistorySetFootnote : public SwHistoryHint
{
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    const ::std::auto_ptr<SwUndoSaveSection> m_pUndo;
    SAL_WNODEPRECATED_DECLARATIONS_POP
    const OUString m_FootnoteNumber;
    sal_uLong m_nNodeIndex;
    const sal_Int32 m_nStart;
    const bool m_bEndNote;

public:
    SwHistorySetFootnote( SwTxtFtn* pTxtFtn, sal_uLong nNode );
    SwHistorySetFootnote( const SwTxtFtn& );
    virtual ~SwHistorySetFootnote();
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet ) SAL_OVERRIDE;

    virtual OUString GetDescription() const SAL_OVERRIDE;

};

class SwHistoryChangeFmtColl : public SwHistoryHint
{
    SwFmtColl * const m_pColl;
    const sal_uLong m_nNodeIndex;
    const sal_uInt8 m_nNodeType;

public:
    SwHistoryChangeFmtColl( SwFmtColl* pColl, sal_uLong nNode, sal_uInt8 nNodeWhich );
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet ) SAL_OVERRIDE;

};

class SwHistoryTxtFlyCnt : public SwHistoryHint
{
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<SwUndoDelLayFmt> m_pUndo;
    SAL_WNODEPRECATED_DECLARATIONS_POP

public:
    SwHistoryTxtFlyCnt( SwFrmFmt* const pFlyFmt );
    virtual ~SwHistoryTxtFlyCnt();
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet ) SAL_OVERRIDE;
    SwUndoDelLayFmt* GetUDelLFmt() { return m_pUndo.get(); }

};

class SwHistoryBookmark : public SwHistoryHint
{
    public:
        SwHistoryBookmark(const ::sw::mark::IMark& rBkmk,
                        bool bSavePos, bool bSaveOtherPos);
        virtual void SetInDoc(SwDoc * pDoc, bool) SAL_OVERRIDE;

        bool IsEqualBookmark(const ::sw::mark::IMark& rBkmk);
        const OUString& GetName() const;

    private:
        const OUString m_aName;
        OUString m_aShortName;
        KeyCode m_aKeycode;
        const sal_uLong m_nNode;
        const sal_uLong m_nOtherNode;
        const sal_Int32 m_nCntnt;
        const sal_Int32 m_nOtherCntnt;
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
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet ) SAL_OVERRIDE;

};

class SwHistoryResetAttrSet : public SwHistoryHint
{
    const sal_uLong m_nNodeIndex;
    const sal_Int32 m_nStart;
    const sal_Int32 m_nEnd;
    std::vector<sal_uInt16> m_Array;

public:
    SwHistoryResetAttrSet( const SfxItemSet& rSet, sal_uLong nNode,
                        sal_Int32 nStt,
                        sal_Int32 nEnd);
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet ) SAL_OVERRIDE;

    const std::vector<sal_uInt16>& GetArr() const { return m_Array; }
    sal_uLong GetNode() const               { return m_nNodeIndex; }
    sal_Int32 GetCntnt() const         { return m_nStart; }

};

class SwHistoryChangeFlyAnchor : public SwHistoryHint
{
    SwFrmFmt & m_rFmt;
    const sal_uLong m_nOldNodeIndex;
    const sal_Int32 m_nOldContentIndex;

public:
    SwHistoryChangeFlyAnchor( SwFrmFmt& rFmt );
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet ) SAL_OVERRIDE;
};

class SwHistoryChangeFlyChain : public SwHistoryHint
{
    SwFlyFrmFmt * const m_pPrevFmt;
    SwFlyFrmFmt * const m_pNextFmt;
    SwFlyFrmFmt * const m_pFlyFmt;

public:
    SwHistoryChangeFlyChain( SwFlyFrmFmt& rFmt, const SwFmtChain& rAttr );
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet ) SAL_OVERRIDE;
};

class SwHistoryChangeCharFmt : public SwHistoryHint
{
    const SfxItemSet m_OldSet;
    const OUString m_Fmt;

public:
    SwHistoryChangeCharFmt( const SfxItemSet& rSet, const OUString & sFmt);
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet ) SAL_OVERRIDE;

};

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
    void CopyAttr(
        SwpHints* pHts,
        const sal_uLong nNodeIdx,
        const sal_Int32 nStart,
        const sal_Int32 nEnd,
        const bool bCopyFields );

    void CopyFmtAttr( const SfxItemSet& rSet, sal_uLong nNodeIdx );
};

class SwRegHistory : public SwClient
{
private:
    std::set<sal_uInt16> m_WhichIdSet;
    SwHistory * const m_pHistory;
    sal_uLong m_nNodeIndex;

    void _MakeSetWhichIds();

protected:
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew ) SAL_OVERRIDE;

public:
    SwRegHistory( SwHistory* pHst );
    SwRegHistory( const SwNode& rNd, SwHistory* pHst );
    SwRegHistory( SwModify* pRegIn, const SwNode& rNd, SwHistory* pHst );

    /// @return true if at least 1 item was inserted
    bool InsertItems( const SfxItemSet& rSet,
        sal_Int32 const nStart, sal_Int32 const nEnd,
        SetAttrMode const nFlags );

    void AddHint( SwTxtAttr* pHt, const bool bNew = false );

    void RegisterInModify( SwModify* pRegIn, const SwNode& rNd );
    void ChangeNodeIndex( sal_uLong nNew ) { m_nNodeIndex = nNew; }
};

#endif // INCLUDED_SW_SOURCE_CORE_INC_ROLBCK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
