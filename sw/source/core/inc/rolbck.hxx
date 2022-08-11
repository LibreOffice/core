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
#include <o3tl/sorted_vector.hxx>
#include <svl/itemset.hxx>
#include <vcl/keycod.hxx>
#include <tox.hxx>

#include <IDocumentMarkAccess.hxx>

#include <memory>
#include <vector>

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
typedef struct _xmlTextWriter* xmlTextWriterPtr;

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
    HSTRY_NOTEXTFIELDMARK,
    HSTRY_TEXTFIELDMARK,
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
    virtual void dumpAsXml(xmlTextWriterPtr pWriter) const;
};

class SwHistorySetFormat final : public SwHistoryHint
{
    std::unique_ptr<SfxPoolItem> m_pAttr;
    const SwNodeOffset m_nNodeIndex;

public:
    SwHistorySetFormat( const SfxPoolItem* pFormatHt, SwNodeOffset nNode );
    virtual ~SwHistorySetFormat() override;
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet ) override;
    virtual OUString GetDescription() const override;

    void dumpAsXml(xmlTextWriterPtr pWriter) const override;
};

class SwHistoryResetFormat final : public SwHistoryHint
{
    const SwNodeOffset m_nNodeIndex;
    const sal_uInt16 m_nWhich;

public:
    SwHistoryResetFormat( const SfxPoolItem* pFormatHt, SwNodeOffset nNodeIdx );
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet ) override;

};

class SwHistorySetText final : public SwHistoryHint
{
    std::unique_ptr<SfxPoolItem> m_pAttr;
    const SwNodeOffset m_nNodeIndex;
    const sal_Int32 m_nStart;
    const sal_Int32 m_nEnd;
    bool m_bFormatIgnoreStart : 1;
    bool m_bFormatIgnoreEnd   : 1;

public:
    SwHistorySetText( SwTextAttr* pTextHt, SwNodeOffset nNode );
    virtual ~SwHistorySetText() override;
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet ) override;

};

class SwHistorySetTextField final : public SwHistoryHint
{
    //!! beware of the order for the declaration of the unique_ptrs.
    //!! If they get destroyed in the wrong order sw may crash (namely mail-merge as well)
    std::unique_ptr<SwFieldType> m_pFieldType;
    const std::unique_ptr<SwFormatField> m_pField;

    SwNodeOffset m_nNodeIndex;
    sal_Int32 m_nPos;
    SwFieldIds m_nFieldWhich;

public:
    SwHistorySetTextField( const SwTextField* pTextField, SwNodeOffset nNode );
    virtual ~SwHistorySetTextField() override;
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet ) override;

    virtual OUString GetDescription() const override;

};

class SwHistorySetRefMark final : public SwHistoryHint
{
    const OUString m_RefName;
    const SwNodeOffset m_nNodeIndex;
    const sal_Int32 m_nStart;
    const sal_Int32 m_nEnd;

public:
    SwHistorySetRefMark( const SwTextRefMark* pTextHt, SwNodeOffset nNode );
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet ) override;

};

class SwHistorySetTOXMark final : public SwHistoryHint
{
    SwTOXMark m_TOXMark;
    const OUString m_TOXName;
    const TOXTypes m_eTOXTypes;
    const SwNodeOffset m_nNodeIndex;
    const sal_Int32 m_nStart;
    const sal_Int32 m_nEnd;

public:
    SwHistorySetTOXMark( const SwTextTOXMark* pTextHt, SwNodeOffset nNode );
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet ) override;
    bool IsEqual( const SwTOXMark& rCmp ) const;

    static SwTOXType* GetSwTOXType(SwDoc& rDoc, TOXTypes eTOXTypes, const OUString& rTOXName);
};

class SwHistoryResetText final : public SwHistoryHint
{
    const SwNodeOffset m_nNodeIndex;
    const sal_Int32 m_nStart;
    const sal_Int32 m_nEnd;
    const sal_uInt16 m_nAttr;

public:
    SwHistoryResetText( sal_uInt16 nWhich, sal_Int32 nStt, sal_Int32 nEnd,
                       SwNodeOffset nNode );
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet ) override;

    sal_uInt16 GetWhich() const         { return m_nAttr; }
    SwNodeOffset GetNode() const     { return m_nNodeIndex; }
    sal_Int32 GetContent() const     { return m_nStart; }

};

class SwHistorySetFootnote final : public SwHistoryHint
{
    const std::unique_ptr<SwUndoSaveSection, o3tl::default_delete<SwUndoSaveSection>> m_pUndo;
    const OUString m_FootnoteNumber;
    SwNodeOffset m_nNodeIndex;
    const sal_Int32 m_nStart;
    const bool m_bEndNote;

public:
    SwHistorySetFootnote( SwTextFootnote* pTextFootnote, SwNodeOffset nNode );
    SwHistorySetFootnote( const SwTextFootnote& );
    virtual ~SwHistorySetFootnote() override;
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet ) override;

    virtual OUString GetDescription() const override;

};

class SwHistoryChangeFormatColl final : public SwHistoryHint
{
    SwFormatColl * const m_pColl;
    const SwNodeOffset m_nNodeIndex;
    const SwNodeType m_nNodeType;

public:
    SwHistoryChangeFormatColl( SwFormatColl* pColl, SwNodeOffset nNode, SwNodeType nNodeWhich );
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet ) override;

};

class SwHistoryTextFlyCnt final : public SwHistoryHint
{
    std::unique_ptr<SwUndoDelLayFormat> m_pUndo;

public:
    SwHistoryTextFlyCnt( SwFrameFormat* const pFlyFormat );
    virtual ~SwHistoryTextFlyCnt() override;
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet ) override;
    SwUndoDelLayFormat* GetUDelLFormat() { return m_pUndo.get(); }

    void dumpAsXml(xmlTextWriterPtr pWriter) const override;
};

class SwHistoryBookmark final : public SwHistoryHint
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
        bool m_bHidden;
        OUString m_aHideCondition;
        vcl::KeyCode m_aKeycode;
        const SwNodeOffset m_nNode;
        const SwNodeOffset m_nOtherNode;
        const sal_Int32 m_nContent;
        const sal_Int32 m_nOtherContent;
        const bool m_bSavePos;
        const bool m_bSaveOtherPos;
        const bool m_bHadOtherPos;
        const IDocumentMarkAccess::MarkType m_eBkmkType;
        std::shared_ptr< ::sfx2::MetadatableUndo > m_pMetadataUndo;
};

/// History object containing all information used during undo / redo
/// of checkbox and drop-down form field insertion.
class SwHistoryNoTextFieldmark final : public SwHistoryHint
{
    public:
        SwHistoryNoTextFieldmark(const ::sw::mark::IFieldmark& rFieldMark);
        virtual void SetInDoc(SwDoc* pDoc, bool) override;
        void ResetInDoc(SwDoc& rDoc);

    private:
        const OUString m_sType;
        const SwNodeOffset m_nNode;
        const sal_Int32 m_nContent;
};

/// History object containing all information used during undo / redo
/// of text form field insertion.
class SwHistoryTextFieldmark final : public SwHistoryHint
{
    public:
        SwHistoryTextFieldmark(const ::sw::mark::IFieldmark& rFieldMark);
        virtual void SetInDoc(SwDoc* pDoc, bool) override;
        void ResetInDoc(SwDoc& rDoc);

    private:
        const OUString m_sName;
        const OUString m_sType;
        const SwNodeOffset m_nStartNode;
        const sal_Int32 m_nStartContent;
        const SwNodeOffset m_nEndNode;
        const sal_Int32 m_nEndContent;
        /*const*/ SwNodeOffset m_nSepNode;
        /*const*/ sal_Int32 m_nSepContent;
};

class SwHistorySetAttrSet final : public SwHistoryHint
{
    SfxItemSet m_OldSet;
    std::vector<sal_uInt16> m_ResetArray;
    const SwNodeOffset m_nNodeIndex;

public:
    SwHistorySetAttrSet( const SfxItemSet& rSet, SwNodeOffset nNode,
                         const o3tl::sorted_vector<sal_uInt16> &rSetArr );
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet ) override;

};

class SwHistoryChangeFlyAnchor final : public SwHistoryHint
{
    SwFrameFormat & m_rFormat;
    const SwNodeOffset m_nOldNodeIndex;
    const sal_Int32 m_nOldContentIndex;

public:
    SwHistoryChangeFlyAnchor( SwFrameFormat& rFormat );
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet ) override;
};

class SwHistoryChangeFlyChain final : public SwHistoryHint
{
    SwFlyFrameFormat * const m_pPrevFormat;
    SwFlyFrameFormat * const m_pNextFormat;
    SwFlyFrameFormat * const m_pFlyFormat;

public:
    SwHistoryChangeFlyChain( SwFlyFrameFormat& rFormat, const SwFormatChain& rAttr );
    virtual void SetInDoc( SwDoc* pDoc, bool bTmpSet ) override;
};

class SwHistoryChangeCharFormat final : public SwHistoryHint
{
    const SfxItemSet m_OldSet;
    const OUString m_Format;

public:
    SwHistoryChangeCharFormat( SfxItemSet aSet, OUString  sFormat);
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
              SwNodeOffset nNodeIdx );
    void Add( SwTextAttr* pTextHt, SwNodeOffset nNodeIdx, bool bNewAttr );
    void Add( SwFormatColl*, SwNodeOffset nNodeIdx, SwNodeType nWhichNd );
    void Add( const ::sw::mark::IMark&, bool bSavePos, bool bSaveOtherPos );
    void AddChangeFlyAnchor( SwFrameFormat& rFormat );
    void AddDeleteFly( SwFrameFormat&, sal_uInt16& rSetPos );
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
        const SwNodeOffset nNodeIdx,
        const sal_Int32 nStart,
        const sal_Int32 nEnd,
        const bool bCopyFields );

    void CopyFormatAttr( const SfxItemSet& rSet, SwNodeOffset nNodeIdx );

    void dumpAsXml(xmlTextWriterPtr pWriter) const;
};

class SwRegHistory final: public SwClient
{
private:
    o3tl::sorted_vector<sal_uInt16> m_WhichIdSet;
    SwHistory * const m_pHistory;
    SwNodeOffset m_nNodeIndex;

    void MakeSetWhichIds();

public:
    SwRegHistory( SwHistory* pHst );
    SwRegHistory( const SwNode& rNd, SwHistory* pHst );
    SwRegHistory( sw::BroadcastingModify* pRegIn, const SwNode& rNd, SwHistory* pHst );
    virtual void SwClientNotify(const SwModify&, const SfxHint& rHint) override;

    /// @return true if at least 1 item was inserted
    bool InsertItems( const SfxItemSet& rSet,
        sal_Int32 const nStart, sal_Int32 const nEnd,
        SetAttrMode const nFlags,
        SwTextAttr **ppNewTextAttr );

    void AddHint( SwTextAttr* pHt, const bool bNew );

    void RegisterInModify( sw::BroadcastingModify* pRegIn, const SwNode& rNd );
    void ChangeNodeIndex( SwNodeOffset nNew ) { m_nNodeIndex = nNew; }
};

#endif // INCLUDED_SW_SOURCE_CORE_INC_ROLBCK_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
