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

#ifndef INCLUDED_SW_SOURCE_CORE_INC_UNDOATTRIBUTE_HXX
#define INCLUDED_SW_SOURCE_CORE_INC_UNDOATTRIBUTE_HXX

#include <undobj.hxx>
#include <memory>
#include <rtl/ustring.hxx>
#include <svl/itemset.hxx>
#include <swtypes.hxx>
#include <calbck.hxx>
#include <set>

class SvxTabStopItem;
class SwFormat;
class SwFootnoteInfo;
class SwEndNoteInfo;
class SwDoc;

class SwUndoAttr : public SwUndo, private SwUndRng
{
    SfxItemSet m_AttrSet;                           // attributes for Redo
    const std::unique_ptr<SwHistory> m_pHistory;      // History for Undo
    std::unique_ptr<SwRedlineData> m_pRedlineData;    // Redlining
    std::unique_ptr<SwRedlineSaveDatas> m_pRedlineSaveData;
    sal_uLong m_nNodeIndex;                         // Offset: for Redlining
    const SetAttrMode m_nInsertFlags;               // insert flags
    OUString m_aChrFormatName;

    void RemoveIdx( SwDoc& rDoc );

public:
    SwUndoAttr( const SwPaM&, const SfxItemSet &, const SetAttrMode nFlags );
    SwUndoAttr( const SwPaM&, const SfxPoolItem&, const SetAttrMode nFlags );

    virtual ~SwUndoAttr() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;

    void SaveRedlineData( const SwPaM& rPam, bool bInsContent );

    SwHistory& GetHistory() { return *m_pHistory; }
};

class SwUndoResetAttr : public SwUndo, private SwUndRng
{
    const std::unique_ptr<SwHistory> m_pHistory;
    std::set<sal_uInt16> m_Ids;
    const sal_uInt16 m_nFormatId;             // Format-Id for Redo

public:
    SwUndoResetAttr( const SwPaM&, sal_uInt16 nFormatId );
    SwUndoResetAttr( const SwPosition&, sal_uInt16 nFormatId );

    virtual ~SwUndoResetAttr() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;

    void SetAttrs( const std::set<sal_uInt16> &rAttrs );

    SwHistory& GetHistory() { return *m_pHistory; }
};

class SwUndoFormatAttr : public SwUndo
{
    friend class SwUndoDefaultAttr;
    OUString m_sFormatName;
    std::unique_ptr<SfxItemSet> m_pOldSet;      // old attributes
    sal_uLong m_nNodeIndex;
    const sal_uInt16 m_nFormatWhich;
    const bool m_bSaveDrawPt;

    void SaveFlyAnchor( const SwFormat * pFormat, bool bSaveDrawPt = false );
    // #i35443# - Add return value, type <bool>.
    // Return value indicates, if anchor attribute is restored.
    // Notes: - If anchor attribute is restored, all other existing attributes
    //          are also restored.
    //        - Anchor attribute isn't restored successfully, if it contains
    //          an invalid anchor position and all other existing attributes
    //          aren't restored.
    //          This situation occurs for undo of styles.
    bool RestoreFlyAnchor(::sw::UndoRedoContext & rContext);
    // --> OD 2008-02-27 #refactorlists# - removed <rAffectedItemSet>
    void Init( const SwFormat & rFormat );

public:
    // register at the Format and save old attributes
    // --> OD 2008-02-27 #refactorlists# - removed <rNewSet>
    SwUndoFormatAttr( const SfxItemSet& rOldSet,
                   SwFormat& rFormat,
                   bool bSaveDrawPt );
    SwUndoFormatAttr( const SfxPoolItem& rItem,
                   SwFormat& rFormat,
                   bool bSaveDrawPt );

    virtual ~SwUndoFormatAttr() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;

    virtual SwRewriter GetRewriter() const override;

    void PutAttr( const SfxPoolItem& rItem, const SwDoc& rDoc );
    SwFormat* GetFormat( const SwDoc& rDoc );   // checks if it is still in the Doc!
};

// --> OD 2008-02-12 #newlistlevelattrs#
class SwUndoFormatResetAttr : public SwUndo
{
    public:
        SwUndoFormatResetAttr( SwFormat& rChangedFormat,
                            const sal_uInt16 nWhichId );
        virtual ~SwUndoFormatResetAttr() override;

        virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
        virtual void RedoImpl( ::sw::UndoRedoContext & ) override;

    private:
        // format at which a certain attribute is reset.
        SwFormat * const m_pChangedFormat;
        // which ID of the reset attribute
        const sal_uInt16 m_nWhichId;
        // old attribute which has been reset - needed for undo.
        std::unique_ptr<SfxPoolItem> m_pOldItem;
};

class SwUndoDontExpandFormat : public SwUndo
{
    const sal_uLong m_nNodeIndex;
    const sal_Int32 m_nContentIndex;

public:
    SwUndoDontExpandFormat( const SwPosition& rPos );

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;
};

// helper class to receive changed attribute sets
class SwUndoFormatAttrHelper final : public SwClient
{
    SwFormat& m_rFormat;
    std::unique_ptr<SwUndoFormatAttr> m_pUndo;
    const bool m_bSaveDrawPt;

public:
    SwUndoFormatAttrHelper(SwFormat& rFormat, bool bSaveDrawPt = true);

    virtual void SwClientNotify(const SwModify&, const SfxHint&) override;

    SwUndoFormatAttr* GetUndo() const { return m_pUndo.get(); }
    // release the undo object (so it is not deleted here), and return it
    std::unique_ptr<SwUndoFormatAttr> ReleaseUndo() { return std::move(m_pUndo); }
};

class SwUndoMoveLeftMargin : public SwUndo, private SwUndRng
{
    const std::unique_ptr<SwHistory> m_pHistory;
    const bool m_bModulus;

public:
    SwUndoMoveLeftMargin( const SwPaM&, bool bRight, bool bModulus );

    virtual ~SwUndoMoveLeftMargin() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;

    SwHistory& GetHistory() { return *m_pHistory; }

};

class SwUndoDefaultAttr : public SwUndo
{
    std::unique_ptr<SfxItemSet> m_pOldSet;          // the old attributes
    std::unique_ptr<SvxTabStopItem> m_pTabStop;

public:
    // registers at the format and saves old attributes
    SwUndoDefaultAttr( const SfxItemSet& rOldSet, const SwDoc& rDoc );

    virtual ~SwUndoDefaultAttr() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
};

class SwUndoChangeFootNote : public SwUndo, private SwUndRng
{
    const std::unique_ptr<SwHistory> m_pHistory;
    const OUString m_Text;
    const bool m_bEndNote;

public:
    SwUndoChangeFootNote( const SwPaM& rRange, const OUString& rText,
                          bool bIsEndNote );
    virtual ~SwUndoChangeFootNote() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RepeatImpl( ::sw::RepeatContext & ) override;

    SwHistory& GetHistory() { return *m_pHistory; }
};

class SwUndoFootNoteInfo : public SwUndo
{
    std::unique_ptr<SwFootnoteInfo> m_pFootNoteInfo;

public:
    SwUndoFootNoteInfo( const SwFootnoteInfo &rInfo, const SwDoc& rDoc );

    virtual ~SwUndoFootNoteInfo() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
};

class SwUndoEndNoteInfo : public SwUndo
{
    std::unique_ptr<SwEndNoteInfo> m_pEndNoteInfo;

public:
    SwUndoEndNoteInfo( const SwEndNoteInfo &rInfo, const SwDoc& rDoc );

    virtual ~SwUndoEndNoteInfo() override;

    virtual void UndoImpl( ::sw::UndoRedoContext & ) override;
    virtual void RedoImpl( ::sw::UndoRedoContext & ) override;
};

#endif // INCLUDED_SW_SOURCE_CORE_INC_UNDOATTRIBUTE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
