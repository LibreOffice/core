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
#ifndef INCLUDED_CUI_SOURCE_INC_SPELLDIALOG_HXX
#define INCLUDED_CUI_SOURCE_INC_SPELLDIALOG_HXX

#include <sfx2/basedlgs.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <vcl/menubtn.hxx>
#include <vcl/decoview.hxx>
#include <vcl/image.hxx>
#include <vcl/toolbox.hxx>
#include <com/sun/star/uno/Reference.hxx>


#include <svx/langbox.hxx>
#include <memory>
#include <svtools/svmedit.hxx>
#include <svl/lstner.hxx>
#include <vcl/fixedhyper.hxx>
#include <vcl/xtextedt.hxx>
#include <editeng/SpellPortions.hxx>

#include <set>

namespace svx{ class SpellUndoAction_Impl;}

// forward ---------------------------------------------------------------

struct SpellDialog_Impl;

namespace com{namespace sun{namespace star{
namespace linguistic2{
    class XSpellChecker1;
}}}}
namespace svx{
class SpellDialog;
struct SpellErrorDescription;

class SentenceEditWindow_Impl : public VclMultiLineEdit
{
    using VclMultiLineEdit::SetText;

private:
    std::set< sal_Int32 >      m_aIgnoreErrorsAt;
    VclPtr<ToolBox>     m_xToolbar;
    sal_Int32           m_nErrorStart;
    sal_Int32           m_nErrorEnd;
    bool                m_bIsUndoEditMode;

    Link<Edit&,void>    m_aModifyLink;

    void            CallModifyLink() {m_aModifyLink.Call(*this);}

    inline SpellDialog* GetSpellDialog() const;

    DECL_LINK(ToolbarHdl, ToolBox*, void);
protected:
    virtual bool    PreNotify( NotifyEvent& rNEvt ) override;

public:
    SentenceEditWindow_Impl(vcl::Window* pParent, WinBits nBits);
    virtual ~SentenceEditWindow_Impl() override;

    void            Init(VclPtr<ToolBox> const &rToolbar);
    void            SetModifyHdl(const Link<Edit&,void>& rLink) override { m_aModifyLink = rLink;}

    void            SetAttrib( const TextAttrib& rAttr, sal_uInt32 nPara, sal_Int32 nStart, sal_Int32 nEnd );
    void            SetText( const OUString& rStr ) override;

    bool            MarkNextError( bool bIgnoreCurrentError, const css::uno::Reference<css::linguistic2::XSpellChecker1>& );
    void            ChangeMarkedWord(const OUString& rNewWord, LanguageType eLanguage);
    void            MoveErrorMarkTo(sal_Int32 nErrorStart, sal_Int32 nErrorEnd, bool bGrammar);
    OUString        GetErrorText() const;
    void            RestoreCurrentError();

    void            SetAlternatives(
                        const css::uno::Reference<css::linguistic2::XSpellAlternatives>& );

    const SpellErrorDescription* GetAlternatives();


    void            ResetModified()   { GetTextEngine()->SetModified(false); m_bIsUndoEditMode = false;}
    virtual bool    IsModified() const override { return GetTextEngine()->IsModified(); }
    virtual void    dispose() override;

    bool            IsUndoEditMode() const { return m_bIsUndoEditMode;}
    void            SetUndoEditMode(bool bSet);

    svx::SpellPortions  CreateSpellPortions() const;

    void            ResetUndo();
    void            Undo();
    void            AddUndoAction( std::unique_ptr<SfxUndoAction> pAction );
    size_t          GetUndoActionCount();
    void            UndoActionStart( sal_uInt16 nId );
    void            UndoActionEnd();

    void            MoveErrorEnd(long nOffset);

    void            ResetIgnoreErrorsAt()   { m_aIgnoreErrorsAt.clear(); }
};

// class SvxSpellDialog ---------------------------------------------
class SpellDialogChildWindow;

class SpellDialog : public SfxModelessDialog
{
    using Window::Invalidate;

    friend class SentenceEditWindow_Impl;
private:

    VclPtr<FixedText>      m_pLanguageFT;
    VclPtr<SvxLanguageBox> m_pLanguageLB;

    VclPtr<FixedText>      m_pExplainFT;
    VclPtr<FixedHyperlink> m_pExplainLink;

    VclPtr<FixedText>      m_pNotInDictFT;
    VclPtr<SentenceEditWindow_Impl> m_pSentenceED;

    VclPtr<FixedText>      m_pSuggestionFT;
    VclPtr<ListBox>        m_pSuggestionLB;

    VclPtr<PushButton>     m_pIgnorePB;
    VclPtr<PushButton>     m_pIgnoreAllPB;
    VclPtr<PushButton>     m_pIgnoreRulePB;
    VclPtr<PushButton>     m_pAddToDictPB;
    VclPtr<MenuButton>     m_pAddToDictMB;

    VclPtr<PushButton>     m_pChangePB;
    VclPtr<PushButton>     m_pChangeAllPB;
    VclPtr<PushButton>     m_pAutoCorrPB;

    VclPtr<CheckBox>       m_pCheckGrammarCB;

    VclPtr<PushButton>     m_pOptionsPB;
    VclPtr<PushButton>     m_pUndoPB;
    VclPtr<CloseButton>    m_pClosePB;
    VclPtr<ToolBox>        m_pToolbar;

    OUString        m_sResumeST;
    OUString        m_sIgnoreOnceST;
    OUString        m_sNoSuggestionsST;

    OUString        m_sTitleSpelling;
    OUString        m_sTitleSpellingGrammar;

    Link<SpellUndoAction_Impl&,void> aDialogUndoLink;

    bool            bFocusLocked;

    svx::SpellDialogChildWindow& rParent;
    svx::SpellPortions           m_aSavedSentence;

    std::unique_ptr<SpellDialog_Impl> pImpl;
    css::uno::Reference<
        css::linguistic2::XSpellChecker1 >     xSpell;

    DECL_LINK(ChangeHdl, Button*, void);
    DECL_LINK(DoubleClickChangeHdl, ListBox&, void);
    DECL_LINK(ChangeAllHdl, Button*, void);
    DECL_LINK( IgnoreAllHdl, Button*, void );
    DECL_LINK(IgnoreHdl, Button*, void);
    DECL_LINK( CheckGrammarHdl, Button*, void );
    DECL_LINK( ExtClickHdl, Button*, void );
    DECL_LINK(CancelHdl, Button*, void);
    DECL_LINK( ModifyHdl, Edit&, void);
    DECL_LINK(UndoHdl, Button*, void);
    DECL_LINK( AddToDictSelectHdl, MenuButton*, void );
    DECL_LINK( AddToDictClickHdl, Button*, void );
    DECL_LINK( LanguageSelectHdl, ListBox&, void );
    DECL_LINK( DialogUndoHdl, SpellUndoAction_Impl&, void );

    DECL_LINK( InitHdl, void*, void );

    void            AddToDictionaryExecute( sal_uInt16 ItemId, PopupMenu const *pMenu );
    void            StartSpellOptDlg_Impl();
    int             InitUserDicts();
    void            UpdateBoxes_Impl(bool bCallFromSelectHdl = false);
    void            Init_Impl();
    void            SpellContinue_Impl(bool UseSavedSentence = false, bool bIgnoreCurrentError = false );
    void            LockFocusChanges( bool bLock ) {bFocusLocked = bLock;}
    void            Impl_Restore(bool bUseSavedSentence);

    LanguageType    GetSelectedLang_Impl() const;

    /** Retrieves the next sentence.
     */
    bool            GetNextSentence_Impl(bool bUseSavedSentence, bool bRechek /*for rechecking the current sentence*/);
    /** Corrects all errors that have been selected to be changed always
     */
    static bool     ApplyChangeAllList_Impl(SpellPortions& rSentence, bool& bHasReplaced);
    void            SetTitle_Impl(LanguageType nLang);

protected:
    virtual bool    EventNotify( NotifyEvent& rNEvt ) override;

    OUString getReplacementString() const;

public:
    SpellDialog(
        svx::SpellDialogChildWindow* pChildWindow,
        vcl::Window * pParent,
        SfxBindings* pBindings);
    virtual ~SpellDialog() override;
    virtual void dispose() override;

    virtual bool    Close() override;

    void            InvalidateDialog();
};

SpellDialog* SentenceEditWindow_Impl::GetSpellDialog() const {return static_cast<SpellDialog*>(GetParentDialog());}

} //namespace svx

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
