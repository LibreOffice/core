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

#pragma once
#if 1

#include <undobj.hxx>
#include <memory>
#include <svl/itemset.hxx>
#include <swtypes.hxx>
#include <calbck.hxx>
#include <set>

class SvxTabStopItem;
class SwFmt;
class SwFtnInfo;
class SwEndNoteInfo;

class SwUndoAttr : public SwUndo, private SwUndRng
{
    SfxItemSet m_AttrSet;                           // attributes for Redo
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    const ::std::auto_ptr<SwHistory> m_pHistory;    // History for Undo
    ::std::auto_ptr<SwRedlineData> m_pRedlineData;  // Redlining
    ::std::auto_ptr<SwRedlineSaveDatas> m_pRedlineSaveData;
    SAL_WNODEPRECATED_DECLARATIONS_POP
    sal_uLong m_nNodeIndex;                         // Offset: for Redlining
    const SetAttrMode m_nInsertFlags;               // insert flags

    void RemoveIdx( SwDoc& rDoc );

public:
    SwUndoAttr( const SwPaM&, const SfxItemSet &, const SetAttrMode nFlags );
    SwUndoAttr( const SwPaM&, const SfxPoolItem&, const SetAttrMode nFlags );

    virtual ~SwUndoAttr();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );

    void SaveRedlineData( const SwPaM& rPam, sal_Bool bInsCntnt );

    SwHistory& GetHistory() { return *m_pHistory; }
};

class SwUndoResetAttr : public SwUndo, private SwUndRng
{
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    const ::std::auto_ptr<SwHistory> m_pHistory;
    SAL_WNODEPRECATED_DECLARATIONS_POP
    std::set<sal_uInt16> m_Ids;
    const sal_uInt16 m_nFormatId;             // Format-Id for Redo

public:
    SwUndoResetAttr( const SwPaM&, sal_uInt16 nFmtId );
    SwUndoResetAttr( const SwPosition&, sal_uInt16 nFmtId );

    virtual ~SwUndoResetAttr();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );

    void SetAttrs( const std::set<sal_uInt16> &rAttrs );

    SwHistory& GetHistory() { return *m_pHistory; }
};

class SwUndoFmtAttr : public SwUndo
{
    friend class SwUndoDefaultAttr;
    SwFmt * m_pFmt;
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<SfxItemSet> m_pOldSet;    // old attributes
    SAL_WNODEPRECATED_DECLARATIONS_POP
    sal_uLong m_nNodeIndex;
    const sal_uInt16 m_nFmtWhich;
    const bool m_bSaveDrawPt;

    bool IsFmtInDoc( SwDoc* );   //is the attribute format still in the Doc?
    void SaveFlyAnchor( bool bSaveDrawPt = false );
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
    void Init();

public:
    // register at the Format and save old attributes
    // --> OD 2008-02-27 #refactorlists# - removed <rNewSet>
    SwUndoFmtAttr( const SfxItemSet& rOldSet,
                   SwFmt& rFmt,
                   bool bSaveDrawPt = true );
    SwUndoFmtAttr( const SfxPoolItem& rItem,
                   SwFmt& rFmt,
                   bool bSaveDrawPt = true );

    virtual ~SwUndoFmtAttr();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );

    virtual SwRewriter GetRewriter() const;

    void PutAttr( const SfxPoolItem& rItem );
    SwFmt* GetFmt( SwDoc& rDoc );   // checks if it is still in the Doc!
};

// --> OD 2008-02-12 #newlistlevelattrs#
class SwUndoFmtResetAttr : public SwUndo
{
    public:
        SwUndoFmtResetAttr( SwFmt& rChangedFormat,
                            const sal_uInt16 nWhichId );
        ~SwUndoFmtResetAttr();

        virtual void UndoImpl( ::sw::UndoRedoContext & );
        virtual void RedoImpl( ::sw::UndoRedoContext & );

    private:
        // format at which a certain attribute is reset.
        SwFmt * const m_pChangedFormat;
        // which ID of the reset attribute
        const sal_uInt16 m_nWhichId;
        // old attribute which has been reset - needed for undo.
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr<SfxPoolItem> m_pOldItem;
        SAL_WNODEPRECATED_DECLARATIONS_POP
};

class SwUndoDontExpandFmt : public SwUndo
{
    const sal_uLong m_nNodeIndex;
    const xub_StrLen m_nContentIndex;

public:
    SwUndoDontExpandFmt( const SwPosition& rPos );

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );
};

// helper class to receive changed attribute sets
class SwUndoFmtAttrHelper : public SwClient
{
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<SwUndoFmtAttr> m_pUndo;
    SAL_WNODEPRECATED_DECLARATIONS_POP
    const bool m_bSaveDrawPt;

public:
    SwUndoFmtAttrHelper( SwFmt& rFmt, bool bSaveDrawPt = true );

    virtual void Modify( const SfxPoolItem*, const SfxPoolItem* );

    SwUndoFmtAttr* GetUndo() const  { return m_pUndo.get(); }
    // release the undo object (so it is not deleted here), and return it
    SwUndoFmtAttr* ReleaseUndo()    { return m_pUndo.release(); }
};

class SwUndoMoveLeftMargin : public SwUndo, private SwUndRng
{
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    const ::std::auto_ptr<SwHistory> m_pHistory;
    SAL_WNODEPRECATED_DECLARATIONS_POP
    const bool m_bModulus;

public:
    SwUndoMoveLeftMargin( const SwPaM&, sal_Bool bRight, bool bModulus );

    virtual ~SwUndoMoveLeftMargin();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );

    SwHistory& GetHistory() { return *m_pHistory; }

};

class SwUndoDefaultAttr : public SwUndo
{
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<SfxItemSet> m_pOldSet;        // the old attributes
    ::std::auto_ptr<SvxTabStopItem> m_pTabStop;
    SAL_WNODEPRECATED_DECLARATIONS_POP

public:
    // registers at the format and saves old attributes
    SwUndoDefaultAttr( const SfxItemSet& rOldSet );

    virtual ~SwUndoDefaultAttr();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
};

class SwUndoChangeFootNote : public SwUndo, private SwUndRng
{
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    const ::std::auto_ptr<SwHistory> m_pHistory;
    SAL_WNODEPRECATED_DECLARATIONS_POP
    const String m_Text;
    const sal_uInt16 m_nNumber;
    const bool m_bEndNote;

public:
    SwUndoChangeFootNote( const SwPaM& rRange, const String& rTxt,
                          sal_uInt16 nNum, bool bIsEndNote );
    virtual ~SwUndoChangeFootNote();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
    virtual void RepeatImpl( ::sw::RepeatContext & );

    SwHistory& GetHistory() { return *m_pHistory; }
};

class SwUndoFootNoteInfo : public SwUndo
{
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<SwFtnInfo> m_pFootNoteInfo;
    SAL_WNODEPRECATED_DECLARATIONS_POP

public:
    SwUndoFootNoteInfo( const SwFtnInfo &rInfo );

    virtual ~SwUndoFootNoteInfo();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
};

class SwUndoEndNoteInfo : public SwUndo
{
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr<SwEndNoteInfo> m_pEndNoteInfo;
    SAL_WNODEPRECATED_DECLARATIONS_POP

public:
    SwUndoEndNoteInfo( const SwEndNoteInfo &rInfo );

    virtual ~SwUndoEndNoteInfo();

    virtual void UndoImpl( ::sw::UndoRedoContext & );
    virtual void RedoImpl( ::sw::UndoRedoContext & );
};

#endif // SW_UNDO_ATTRIBUTE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
