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
#include <svtools/stdctrl.hxx>
#include <vcl/button.hxx>
#include <vcl/menubtn.hxx>
#include <vcl/group.hxx>
#include <vcl/decoview.hxx>
#include <vcl/image.hxx>
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
    std::set< sal_uInt16 >      m_aIgnoreErrorsAt;
    sal_uInt16          m_nErrorStart;
    sal_uInt16          m_nErrorEnd;
    bool            m_bIsUndoEditMode;

    Link            m_aModifyLink;

    void            CallModifyLink() {m_aModifyLink.Call(this);}

    SpellDialog*    GetSpellDialog() const {return (SpellDialog*)GetParentDialog();}
protected:
    virtual bool    PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;

public:
    SentenceEditWindow_Impl(vcl::Window* pParent, WinBits nBits);
    virtual ~SentenceEditWindow_Impl();

    void            SetModifyHdl(const Link& rLink) SAL_OVERRIDE { m_aModifyLink = rLink;}

    void            SetAttrib( const TextAttrib& rAttr, sal_uLong nPara, sal_uInt16 nStart, sal_uInt16 nEnd );
    void            SetText( const OUString& rStr ) SAL_OVERRIDE;

    bool            MarkNextError( bool bIgnoreCurrentError, com::sun::star::uno::Reference<com::sun::star::linguistic2::XSpellChecker1> );
    void            ChangeMarkedWord(const OUString& rNewWord, LanguageType eLanguage);
    void            MoveErrorMarkTo(sal_uInt16 nErrorStart, sal_uInt16 nErrorEnd, bool bGrammar);
    OUString        GetErrorText() const;
    void            RestoreCurrentError();

    void            SetAlternatives(
                        com::sun::star::uno::Reference<com::sun::star::linguistic2::XSpellAlternatives> );

    const SpellErrorDescription* GetAlternatives();


    void            ResetModified()   { GetTextEngine()->SetModified(false); m_bIsUndoEditMode = false;}
    virtual bool    IsModified() const SAL_OVERRIDE { return GetTextEngine()->IsModified(); }

    bool            IsUndoEditMode() const { return m_bIsUndoEditMode;}
    void            SetUndoEditMode(bool bSet);

    svx::SpellPortions  CreateSpellPortions( bool bSetIgnoreFlag ) const;

    void            ResetUndo();
    void            Undo();
    void            AddUndoAction( SfxUndoAction *pAction, bool bTryMerg=false );
    sal_uInt16      GetUndoActionCount();
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

    FixedText*      m_pLanguageFT;
    SvxLanguageBox* m_pLanguageLB;

    FixedText*      m_pExplainFT;
    FixedHyperlink* m_pExplainLink;

    FixedText*      m_pNotInDictFT;
    SentenceEditWindow_Impl* m_pSentenceED;

    FixedText*      m_pSuggestionFT;
    ListBox*        m_pSuggestionLB;

    PushButton*     m_pIgnorePB;
    PushButton*     m_pIgnoreAllPB;
    PushButton*     m_pIgnoreRulePB;
    PushButton*     m_pAddToDictPB;
    MenuButton*     m_pAddToDictMB;

    PushButton*     m_pChangePB;
    PushButton*     m_pChangeAllPB;
    PushButton*     m_pAutoCorrPB;

    CheckBox*       m_pCheckGrammarCB;

    PushButton*     m_pOptionsPB;
    PushButton*     m_pUndoPB;
    CloseButton*    m_pClosePB;

    OUString        m_sResumeST;
    OUString        m_sIgnoreOnceST;
    OUString        m_sNoSuggestionsST;

    OUString        m_sTitleSpelling;
    OUString        m_sTitleSpellingGrammar;

    Link            aDialogUndoLink;

    bool            bModified;
    bool            bFocusLocked;

    svx::SpellDialogChildWindow& rParent;
    svx::SpellPortions           m_aSavedSentence;

    SpellDialog_Impl* pImpl;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSpellChecker1 >     xSpell;

    DECL_LINK(ChangeHdl, void *);
    DECL_LINK(ChangeAllHdl, void *);
    DECL_LINK( IgnoreAllHdl, Button * );
    DECL_LINK(IgnoreHdl, void *);
    DECL_LINK( CheckGrammarHdl, CheckBox* );
    DECL_LINK( ExtClickHdl, Button * );
    DECL_LINK(CancelHdl, void *);
    DECL_LINK( ModifyHdl, SentenceEditWindow_Impl *);
    DECL_LINK(UndoHdl, void *);
    DECL_LINK( AddToDictSelectHdl, MenuButton* );
    DECL_LINK( AddToDictClickHdl, PushButton* );
    DECL_LINK( LanguageSelectHdl, SvxLanguageBox* );
    DECL_LINK( DialogUndoHdl, SpellUndoAction_Impl* );
    DECL_LINK( HandleHyperlink, FixedHyperlink * );

    DECL_STATIC_LINK( SpellDialog, InitHdl, SpellDialog * );

    int             AddToDictionaryExecute( sal_uInt16 ItemId, PopupMenu *pMenu );
    void            StartSpellOptDlg_Impl();
    int             InitUserDicts();
    void            UpdateBoxes_Impl();
    void            Init_Impl();
    void            SpellContinue_Impl(bool UseSavedSentence = false, bool bIgnoreCurrentError = false );
    void            LockFocusChanges( bool bLock ) {bFocusLocked = bLock;}
    void            Impl_Restore();

    void            SetSelectedLang_Impl( LanguageType nLang );
    LanguageType    GetSelectedLang_Impl() const;

    /** Retrieves the next sentence.
     */
    bool            GetNextSentence_Impl(bool bUseSavedSentence, bool bRechek /*for rechecking the current sentence*/);
    /** Corrects all errors that have been selected to be changed always
     */
    bool            ApplyChangeAllList_Impl(SpellPortions& rSentence, bool& bHasReplaced);
    void            SetTitle_Impl(LanguageType nLang);

protected:
    virtual bool    Notify( NotifyEvent& rNEvt ) SAL_OVERRIDE;

    OUString getReplacementString() const;

public:
    SpellDialog(
        svx::SpellDialogChildWindow* pChildWindow,
        vcl::Window * pParent,
        SfxBindings* pBindings);
    virtual ~SpellDialog();

    void            SetLanguage( sal_uInt16 nLang );
    virtual bool    Close() SAL_OVERRIDE;

    void            InvalidateDialog();
};
} //namespace svx

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
