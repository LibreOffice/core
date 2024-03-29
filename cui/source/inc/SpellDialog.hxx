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

#include <sfx2/basedlgs.hxx>
#include <com/sun/star/uno/Reference.hxx>


#include <svx/langbox.hxx>
#include <memory>
#include <svl/undo.hxx>
#include <vcl/customweld.hxx>
#include <svx/weldeditview.hxx>
#include <editeng/SpellPortions.hxx>

#include <set>

namespace svx{ class SpellUndoAction_Impl;}
class UndoChangeGroupGuard;

// forward ---------------------------------------------------------------

struct SpellDialog_Impl;
namespace com::sun::star::linguistic2 { class XSpellChecker1; }

namespace svx{
class SpellDialog;
struct SpellErrorDescription;

class SentenceEditWindow_Impl : public WeldEditView
{
private:
    std::unique_ptr<weld::ScrolledWindow> m_xScrolledWindow;
    std::set<sal_Int32> m_aIgnoreErrorsAt;
    SpellDialog*        m_pSpellDialog;
    weld::Toolbar*      m_pToolbar;
    sal_Int32           m_nErrorStart;
    sal_Int32           m_nErrorEnd;
    bool                m_bIsUndoEditMode;

    Link<LinkParamNone*,void> m_aModifyLink;

    void            CallModifyLink() {m_aModifyLink.Call(nullptr); }

    SpellDialog* GetSpellDialog() const { return m_pSpellDialog; }

    bool GetErrorDescription(SpellErrorDescription& rSpellErrorDescription, sal_Int32 nPosition);

    DECL_LINK(ScrollHdl, weld::ScrolledWindow&, void);
    DECL_LINK(EditStatusHdl, EditStatus&, void);
    DECL_LINK(ToolbarHdl, const OUString&, void);

    void DoScroll();
    void SetScrollBarRange();

protected:
    virtual bool    KeyInput( const KeyEvent& rKEvt ) override;
    virtual void    StyleUpdated() override;

public:
    SentenceEditWindow_Impl(std::unique_ptr<weld::ScrolledWindow> xScrolledWindow);
    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;
    virtual void EditViewScrollStateChange() override;
    void SetSpellDialog(SpellDialog* pDialog) { m_pSpellDialog = pDialog; }
    virtual ~SentenceEditWindow_Impl() override;

    void            Init(weld::Toolbar* pToolbar);
    void            SetModifyHdl(const Link<LinkParamNone*,void>& rLink)
    {
        m_aModifyLink = rLink;
        m_xEditEngine->SetModifyHdl(m_aModifyLink);
    }

    void            SetAttrib(const SfxPoolItem& rItem, sal_Int32 nStart, sal_Int32 nEnd);

    void            SetText(const OUString& rStr);

    bool            MarkNextError( bool bIgnoreCurrentError, const css::uno::Reference<css::linguistic2::XSpellChecker1>& );
    int             ChangeMarkedWord(const OUString& rNewWord, LanguageType eLanguage);
    void            MoveErrorMarkTo(sal_Int32 nErrorStart, sal_Int32 nErrorEnd, bool bGrammar);
    OUString        GetErrorText() const;
    void            RestoreCurrentError();

    void            SetAlternatives(
                        const css::uno::Reference<css::linguistic2::XSpellAlternatives>& );

    bool            GetAlternatives(SpellErrorDescription& rDesc);

    void            ClearModifyFlag() { m_xEditEngine->ClearModifyFlag(); }
    void            ResetModified() { ClearModifyFlag(); m_bIsUndoEditMode = false;}
    bool            IsModified() const { return m_xEditEngine->IsModified(); }

    bool            IsUndoEditMode() const { return m_bIsUndoEditMode;}
    void            SetUndoEditMode(bool bSet);

    svx::SpellPortions  CreateSpellPortions() const;

    void            ResetUndo();
    void            Undo();
    void            AddUndoAction( std::unique_ptr<SfxUndoAction> pAction );
    size_t          GetUndoActionCount() const;
    void            UndoActionStart( sal_uInt16 nId );
    void            UndoActionEnd();

    void            MoveErrorEnd(tools::Long nOffset);

    void            ResetIgnoreErrorsAt()   { m_aIgnoreErrorsAt.clear(); }

    void            SetDocumentColor(weld::DrawingArea* pDrawingArea);
};

// class SvxSpellDialog ---------------------------------------------
class SpellDialogChildWindow;

class SpellDialog : public SfxModelessDialogController
{
    friend class SentenceEditWindow_Impl;
private:
    OUString        m_sResumeST;
    OUString        m_sIgnoreOnceST;
    OUString        m_sNoSuggestionsST;

    OUString        m_sTitleSpelling;
    OUString        m_sTitleSpellingGrammar;

    Link<SpellUndoAction_Impl&,void> aDialogUndoLink;
    ImplSVEvent *   m_pInitHdlEvent;
    bool            bFocusLocked;

    svx::SpellDialogChildWindow& rParent;
    svx::SpellPortions           m_aSavedSentence;

    std::unique_ptr<SpellDialog_Impl> pImpl;
    css::uno::Reference<
        css::linguistic2::XSpellChecker1 >     xSpell;

    std::unique_ptr<weld::Label> m_xAltTitle;
    std::unique_ptr<weld::Label> m_xResumeFT;
    std::unique_ptr<weld::Label> m_xNoSuggestionsFT;
    std::unique_ptr<weld::Label> m_xLanguageFT;
    std::unique_ptr<SvxLanguageBox> m_xLanguageLB;
    std::unique_ptr<weld::Label> m_xExplainFT;
    std::unique_ptr<weld::LinkButton> m_xExplainLink;
    std::unique_ptr<weld::Label> m_xNotInDictFT;
    std::unique_ptr<SentenceEditWindow_Impl> m_xSentenceED;
    std::unique_ptr<weld::Label> m_xSuggestionFT;
    std::unique_ptr<weld::TreeView> m_xSuggestionLB;
    std::unique_ptr<weld::Button> m_xIgnorePB;
    std::unique_ptr<weld::Button> m_xIgnoreAllPB;
    std::unique_ptr<weld::Button> m_xIgnoreRulePB;
    std::unique_ptr<weld::Button> m_xAddToDictPB;
    std::unique_ptr<weld::MenuButton> m_xAddToDictMB;
    std::unique_ptr<weld::Button> m_xChangePB;
    std::unique_ptr<weld::Button> m_xChangeAllPB;
    std::unique_ptr<weld::Button> m_xAutoCorrPB;
    std::unique_ptr<weld::CheckButton> m_xCheckGrammarCB;
    std::unique_ptr<weld::Button> m_xOptionsPB;
    std::unique_ptr<weld::Button> m_xUndoPB;
    std::unique_ptr<weld::Button> m_xClosePB;
    std::unique_ptr<weld::Toolbar> m_xToolbar;
    std::unique_ptr<weld::CustomWeld> m_xSentenceEDWeld;
    std::shared_ptr<SfxSingleTabDialogController> m_xOptionsDlg;

    DECL_LINK(ChangeHdl, weld::Button&, void);
    DECL_LINK(DoubleClickChangeHdl, weld::TreeView&, bool);
    DECL_LINK(ChangeAllHdl, weld::Button&, void);
    DECL_LINK(IgnoreAllHdl, weld::Button&, void);
    DECL_LINK(IgnoreHdl, weld::Button&, void);
    DECL_LINK(CheckGrammarHdl, weld::Toggleable&, void);
    DECL_LINK(ExtClickHdl, weld::Button&, void);
    DECL_LINK(CancelHdl, weld::Button&, void);
    DECL_LINK(ModifyHdl, LinkParamNone*, void);
    DECL_LINK(UndoHdl, weld::Button&, void);
    DECL_LINK(AddToDictSelectHdl, const OUString&, void);
    DECL_LINK(AddToDictClickHdl, weld::Button&, void);
    DECL_LINK(LanguageSelectHdl, weld::ComboBox&, void);
    DECL_LINK(DialogUndoHdl, SpellUndoAction_Impl&, void);

    DECL_LINK(InitHdl, void*, void);

    void            AddToDictionaryExecute(const OUString& rItemId);
    void            StartSpellOptDlg_Impl();
    int             InitUserDicts();
    void            UpdateBoxes_Impl(bool bCallFromSelectHdl = false);
    void            Init_Impl();
    void            SpellContinue_Impl(std::unique_ptr<UndoChangeGroupGuard>* pGuard = nullptr, bool UseSavedSentence = false, bool bIgnoreCurrentError = false );
    void            LockFocusChanges( bool bLock ) {bFocusLocked = bLock;}
    void            ToplevelFocusChanged();
    void            Impl_Restore(bool bUseSavedSentence);

    LanguageType    GetSelectedLang_Impl() const;

    /** Retrieves the next sentence.
     */
    bool            GetNextSentence_Impl(std::unique_ptr<UndoChangeGroupGuard>* pGuard, bool bUseSavedSentence, bool bRecheck /*for rechecking the current sentence*/);
    /** Corrects all errors that have been selected to be changed always
     */
    static bool     ApplyChangeAllList_Impl(SpellPortions& rSentence, bool& bHasReplaced);
    void            SetTitle_Impl(LanguageType nLang);

protected:

    OUString getReplacementString() const;

public:
    SpellDialog(
        svx::SpellDialogChildWindow* pChildWindow,
        weld::Window * pParent,
        SfxBindings* pBindings);
    virtual ~SpellDialog() override;

    virtual void    Activate() override;
    virtual void    Deactivate() override;

    virtual void    Close() override;

    void            InvalidateDialog();
};

} //namespace svx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
