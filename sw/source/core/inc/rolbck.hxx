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

#include <o3tl/deleter.hxx>
#include <svl/itemset.hxx>
#include <tools/solar.h>
#include <vcl/keycod.hxx>
#include <tox.hxx>

#include <SwNumberTreeTypes.hxx>
#include <IDocumentMarkAccess.hxx>

#include <memory>
#include <vector>
#include <set>

namespace sfx2 {
    class MetadatableUndo;
}

class SwDoc;
class SwFormatColl;
class SwTextAttr;
class SfxPoolItem;
class SwUndoSaveSection;
class SwTextFootnote;
class SwUndoDelLayFormat;
class SwFlyFrameFormat;
class SwFormatField;
class SwTextField;
class SwFieldType;
class SwTextTOXMark;
class SwTextRefMark;
class SwFrameFormat;
class SwpHints;
class SwFormatChain;
class SwNode;
class SwCharFormat;
enum class SwFieldIds : sal_uInt16;

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
    HSTRY_CHGFLYANCHOR,
    HSTRY_CHGFLYCHAIN,
    HSTRY_CHGCHARFMT,
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

class SwHistorySetFormat : public SwHistoryHint
{
    std::unique_ptr<SfxPoolItem> m_pAttr;
    const sal_uLong m_nNodeIndex;

public:
    SwHistorySetFormat( const SfxPoolItem* pFormatHt, sal_uLong nNode );
    virtual ~SwHistorySetFormat() override;
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet ) override;
    virtual OUString GetDescription() const override;

};

class SwHistoryResetFormat : public SwHistoryHint
{
    const sal_uLong m_nNodeIndex;
    const sal_uInt16 m_nWhich;

public:
    SwHistoryResetFormat( const SfxPoolItem* pFormatHt, sal_uLong nNodeIdx );
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet ) override;

};

class SwHistorySetText : public SwHistoryHint
{
    std::unique_ptr<SfxPoolItem> m_pAttr;
    const sal_uLong m_nNodeIndex;
    const sal_Int32 m_nStart;
    const sal_Int32 m_nEnd;
    bool m_bFormatIgnoreStart : 1;
    bool m_bFormatIgnoreEnd   : 1;

public:
    SwHistorySetText( SwTextAttr* pTextHt, sal_uLong nNode );
    virtual ~SwHistorySetText() override;
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet ) override;

};

class SwHistorySetTextField : public SwHistoryHint
{
    //!! beware of the order for the declaration of the unique_ptrs.
    //!! If they get destroyed in the wrong order sw may crash (namely mail-merge as well)
    std::unique_ptr<SwFieldType> m_pFieldType;
    const std::unique_ptr<SwFormatField> m_pField;

    sal_uLong m_nNodeIndex;
    sal_Int32 m_nPos;
    SwFieldIds m_nFieldWhich;

public:
    SwHistorySetTextField( SwTextField* pTextField, sal_uLong nNode );
    virtual ~SwHistorySetTextField() override;
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet ) override;

    virtual OUString GetDescription() const override;

};

class SwHistorySetRefMark : public SwHistoryHint
{
    const OUString m_RefName;
    const sal_uLong m_nNodeIndex;
    const sal_Int32 m_nStart;
    const sal_Int32 m_nEnd;

public:
    SwHistorySetRefMark( SwTextRefMark* pTextHt, sal_uLong nNode );
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet ) override;

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
    SwHistorySetTOXMark( SwTextTOXMark* pTextHt, sal_uLong nNode );
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet ) override;
    bool IsEqual( const SwTOXMark& rCmp ) const;

};

class SwHistoryResetText : public SwHistoryHint
{
    const sal_uLong m_nNodeIndex;
    const sal_Int32 m_nStart;
    const sal_Int32 m_nEnd;
    const sal_uInt16 m_nAttr;

public:
    SwHistoryResetText( sal_uInt16 nWhich, sal_Int32 nStt, sal_Int32 nEnd,
                       sal_uLong nNode );
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet ) override;

    sal_uInt16 GetWhich() const         { return m_nAttr; }
    sal_uLong GetNode() const           { return m_nNodeIndex; }
    sal_Int32 GetContent() const     { return m_nStart; }

};

class SwHistorySetFootnote : public SwHistoryHint
{
    const std::unique_ptr<SwUndoSaveSection, o3tl::default_delete<SwUndoSaveSection>> m_pUndo;
    const OUString m_FootnoteNumber;
    sal_uLong m_nNodeIndex;
    const sal_Int32 m_nStart;
    const bool m_bEndNote;

public:
    SwHistorySetFootnote( SwTextFootnote* pTextFootnote, sal_uLong nNode );
    SwHistorySetFootnote( const SwTextFootnote& );
    virtual ~SwHistorySetFootnote() override;
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet ) override;

    virtual OUString GetDescription() const override;

};

class SwHistoryChangeFormatColl : public SwHistoryHint
{
    SwFormatColl * const m_pColl;
    const sal_uLong m_nNodeIndex;
    const SwNodeType m_nNodeType;

public:
    SwHistoryChangeFormatColl( SwFormatColl* pColl, sal_uLong nNode, SwNodeType nNodeWhich );
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet ) override;

};

class SwHistoryTextFlyCnt : public SwHistoryHint
{
    std::unique_ptr<SwUndoDelLayFormat> m_pUndo;

public:
    SwHistoryTextFlyCnt( SwFrameFormat* const pFlyFormat );
    virtual ~SwHistoryTextFlyCnt() override;
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet ) override;
    SwUndoDelLayFormat* GetUDelLFormat() { return m_pUndo.get(); }

};

class SwHistoryBookmark : public SwHistoryHint
{
    public:
        SwHistoryBookmark(const ::sw::mark::IMark& rBkmk,
                        bool bSavePos, bool bSaveOtherPos);
        virtual void SetInDoc(SwDoc * pDoc, bool) override;

        bool IsEqualBookmark(const ::sw::mark::IMark& rBkmk);
        const OUString& GetName() const { return m_aName;}

    private:
        const OUString m_aName;
        OUString m_aShortName;
        vcl::KeyCode m_aKeycode;
        const sal_uLong m_nNode;
        const sal_uLong m_nOtherNode;
        const sal_Int32 m_nContent;
        const sal_Int32 m_nOtherContent;
        const bool m_bSavePos;
        const bool m_bSaveOtherPos;
        const bool m_bHadOtherPos;
        const IDocumentMarkAccess::MarkType m_eBkmkType;
        std::shared_ptr< ::sfx2::MetadatableUndo > m_pMetadataUndo;
};

class SwHistorySetAttrSet : public SwHistoryHint
{
    SfxItemSet m_OldSet;
    std::vector<sal_uInt16> m_ResetArray;
    const sal_uLong m_nNodeIndex;

public:
    SwHistorySetAttrSet( const SfxItemSet& rSet, sal_uLong nNode,
                         const std::set<sal_uInt16> &rSetArr );
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet ) override;

};

class SwHistoryChangeFlyAnchor : public SwHistoryHint
{
    SwFrameFormat & m_rFormat;
    const sal_uLong m_nOldNodeIndex;
    const sal_Int32 m_nOldContentIndex;

public:
    SwHistoryChangeFlyAnchor( SwFrameFormat& rFormat );
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet ) override;
};

class SwHistoryChangeFlyChain : public SwHistoryHint
{
    SwFlyFrameFormat * const m_pPrevFormat;
    SwFlyFrameFormat * const m_pNextFormat;
    SwFlyFrameFormat * const m_pFlyFormat;

public:
    SwHistoryChangeFlyChain( SwFlyFrameFormat& rFormat, const SwFormatChain& rAttr );
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet ) override;
};

class SwHistoryChangeCharFormat : public SwHistoryHint
{
    const SfxItemSet m_OldSet;
    const OUString m_Format;

public:
    SwHistoryChangeCharFormat( const SfxItemSet& rSet, const OUString & sFormat);
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet ) override;

};

class SwHistory
{
    friend class SwDoc;         // actually only SwDoc::DelUndoObj may access
    friend class SwRegHistory;  // for inserting History attributes

    std::vector<std::unique_ptr<SwHistoryHint>> m_SwpHstry;
    sal_uInt16 m_nEndDiff;

public:
    SwHistory();
    ~SwHistory();

    // call and delete all objects between nStart and array end
    bool Rollback( SwDoc* pDoc, sal_uInt16 nStart = 0 );
    // call all objects between nStart and TmpEnd; store nStart as TmpEnd
    bool TmpRollback( SwDoc* pDoc, sal_uInt16 nStart, bool ToFirst = true );

    void Add( const SfxPoolItem* pOldValue, const SfxPoolItem* pNewValue,
              sal_uLong nNodeIdx );
    void Add( SwTextAttr* pTextHt, sal_uLong nNodeIdx, bool bNewAttr );
    void Add( SwFormatColl*, sal_uLong nNodeIdx, SwNodeType nWhichNd );
    void Add( const ::sw::mark::IMark&, bool bSavePos, bool bSaveOtherPos );
    void Add( SwFrameFormat& rFormat );
    void Add( SwFlyFrameFormat&, sal_uInt16& rSetPos );
    void Add( const SwTextFootnote& );
    void Add( const SfxItemSet & rSet, const SwCharFormat & rCharFormat);

    sal_uInt16 Count() const { return m_SwpHstry.size(); }
    sal_uInt16 GetTmpEnd() const { return m_SwpHstry.size() - m_nEndDiff; }
    sal_uInt16 SetTmpEnd( sal_uInt16 nTmpEnd );        // return previous value
    SwHistoryHint      * operator[]( sal_uInt16 nPos ) { return m_SwpHstry[nPos].get(); }
    SwHistoryHint const* operator[]( sal_uInt16 nPos ) const
        { return m_SwpHstry[nPos].get(); }

    // for SwUndoDelete::Undo/Redo
    void Move( sal_uInt16 nPos, SwHistory *pIns,
               sal_uInt16 const nStart = 0)
    {
        auto itSourceBegin = pIns->m_SwpHstry.begin() + nStart;
        auto itSourceEnd = pIns->m_SwpHstry.end();
        if (itSourceBegin == itSourceEnd)
            return;
        m_SwpHstry.insert(m_SwpHstry.begin() + nPos, std::make_move_iterator(itSourceBegin), std::make_move_iterator(itSourceEnd));
        pIns->m_SwpHstry.erase( itSourceBegin, itSourceEnd );
    }

    // helper methods for recording attribute in History
    // used by Undo classes (Delete/Overwrite/Inserts)
    void CopyAttr(
        SwpHints const * pHts,
        const sal_uLong nNodeIdx,
        const sal_Int32 nStart,
        const sal_Int32 nEnd,
        const bool bCopyFields );

    void CopyFormatAttr( const SfxItemSet& rSet, sal_uLong nNodeIdx );
};

class SwRegHistory : public SwClient
{
private:
    std::set<sal_uInt16> m_WhichIdSet;
    SwHistory * const m_pHistory;
    sal_uLong m_nNodeIndex;

    void MakeSetWhichIds();

protected:
    virtual void Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew ) override;

public:
    SwRegHistory( SwHistory* pHst );
    SwRegHistory( const SwNode& rNd, SwHistory* pHst );
    SwRegHistory( SwModify* pRegIn, const SwNode& rNd, SwHistory* pHst );

    /// @return true if at least 1 item was inserted
    bool InsertItems( const SfxItemSet& rSet,
        sal_Int32 const nStart, sal_Int32 const nEnd,
        SetAttrMode const nFlags );

    void AddHint( SwTextAttr* pHt, const bool bNew );

    void RegisterInModify( SwModify* pRegIn, const SwNode& rNd );
    void ChangeNodeIndex( sal_uLong nNew ) { m_nNodeIndex = nNew; }
};

#endif // INCLUDED_SW_SOURCE_CORE_INC_ROLBCK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
