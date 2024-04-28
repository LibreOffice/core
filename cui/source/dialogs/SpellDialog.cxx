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

#include <memory>
#include "SpellAttrib.hxx"
#include <sfx2/bindings.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/viewfrm.hxx>
#include <svl/grabbagitem.hxx>
#include <svl/undo.hxx>
#include <tools/debug.hxx>
#include <unotools/lingucfg.hxx>
#include <editeng/editund2.hxx>
#include <editeng/colritem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/langitem.hxx>
#include <editeng/splwrap.hxx>
#include <editeng/unolingu.hxx>
#include <editeng/wghtitem.hxx>
#include <linguistic/misc.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/linguistic2/XDictionary.hpp>
#include <com/sun/star/linguistic2/XSpellAlternatives.hpp>
#include <com/sun/star/linguistic2/XSearchableDictionaryList.hpp>
#include <com/sun/star/linguistic2/XSpellChecker1.hpp>
#include <sfx2/app.hxx>
#include <rtl/ustrbuf.hxx>
#include <vcl/specialchars.hxx>
#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/texteng.hxx>
#include <vcl/weld.hxx>
#include <svx/SpellDialogChildWindow.hxx>
#include <SpellDialog.hxx>
#include <optlingu.hxx>
#include <treeopt.hxx>
#include <svtools/colorcfg.hxx>
#include <svtools/langtab.hxx>
#include <sal/log.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <comphelper/lok.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::linguistic2;
using namespace linguistic;


// struct SpellDialog_Impl ---------------------------------------------

struct SpellDialog_Impl
{
    Sequence< Reference< XDictionary >  >   aDics;
};


#define SPELLUNDO_START                     200

#define SPELLUNDO_CHANGE_LANGUAGE           (SPELLUNDO_START + 1)
#define SPELLUNDO_CHANGE_TEXTENGINE         (SPELLUNDO_START + 2)
#define SPELLUNDO_CHANGE_NEXTERROR          (SPELLUNDO_START + 3)
#define SPELLUNDO_CHANGE_ADD_TO_DICTIONARY  (SPELLUNDO_START + 4)
#define SPELLUNDO_CHANGE_GROUP              (SPELLUNDO_START + 5) //undo list
#define SPELLUNDO_MOVE_ERROREND             (SPELLUNDO_START + 6)
#define SPELLUNDO_UNDO_EDIT_MODE            (SPELLUNDO_START + 7)
#define SPELLUNDO_ADD_IGNORE_RULE           (SPELLUNDO_START + 8)

namespace svx{
class SpellUndoAction_Impl : public SfxUndoAction
{
    sal_uInt16          m_nId;
    const Link<SpellUndoAction_Impl&,void>& m_rActionLink;
    //undo of button enabling
    bool            m_bEnableChangePB;
    bool            m_bEnableChangeAllPB;
    //undo of MarkNextError - used in change and change all, ignore and ignore all
    tools::Long            m_nOldErrorStart;
    tools::Long            m_nOldErrorEnd;
    bool            m_bIsErrorLanguageSelected;
    //undo of AddToDictionary
    Reference<XDictionary>  m_xDictionary;
    OUString                m_sAddedWord;
    //move end of error - ::ChangeMarkedWord()
    tools::Long            m_nOffset;

public:
    SpellUndoAction_Impl(sal_uInt16 nId, const Link<SpellUndoAction_Impl&,void>& rActionLink) :
        m_nId(nId),
        m_rActionLink( rActionLink),
        m_bEnableChangePB(false),
        m_bEnableChangeAllPB(false),
        m_nOldErrorStart(-1),
        m_nOldErrorEnd(-1),
        m_bIsErrorLanguageSelected(false),
        m_nOffset(0)
        {}

    virtual void            Undo() override;
    sal_uInt16              GetId() const;

    void                    SetEnableChangePB(){m_bEnableChangePB = true;}
    bool                    IsEnableChangePB() const {return m_bEnableChangePB;}

    void                    SetEnableChangeAllPB(){m_bEnableChangeAllPB = true;}
    bool                    IsEnableChangeAllPB() const {return m_bEnableChangeAllPB;}

    void                    SetErrorMove(tools::Long nOldStart, tools::Long nOldEnd)
                                {
                                        m_nOldErrorStart = nOldStart;
                                        m_nOldErrorEnd = nOldEnd;
                                }
    tools::Long                    GetOldErrorStart() const { return m_nOldErrorStart;}
    tools::Long                    GetOldErrorEnd() const { return m_nOldErrorEnd;}

    void                    SetErrorLanguageSelected(bool bSet){ m_bIsErrorLanguageSelected = bSet;}
    bool                    IsErrorLanguageSelected() const {return m_bIsErrorLanguageSelected;}

    void                    SetDictionary(const Reference<XDictionary>& xDict) { m_xDictionary = xDict; }
    const Reference<XDictionary>& GetDictionary() const { return m_xDictionary; }
    void                    SetAddedWord(const OUString& rWord) {m_sAddedWord = rWord;}
    const OUString&         GetAddedWord() const { return m_sAddedWord;}

    void                    SetOffset(tools::Long nSet) {m_nOffset = nSet;}
    tools::Long                    GetOffset() const {return m_nOffset;}
};
}//namespace svx
using namespace ::svx;

void SpellUndoAction_Impl::Undo()
{
    m_rActionLink.Call(*this);
}


sal_uInt16 SpellUndoAction_Impl::GetId()const
{
    return m_nId;
}

// class SvxSpellCheckDialog ---------------------------------------------

SpellDialog::SpellDialog(SpellDialogChildWindow* pChildWindow,
    weld::Window * pParent, SfxBindings* _pBindings)
    : SfxModelessDialogController (_pBindings, pChildWindow,
        pParent, "cui/ui/spellingdialog.ui", "SpellingDialog")
    , aDialogUndoLink(LINK (this, SpellDialog, DialogUndoHdl))
    , m_pInitHdlEvent(nullptr)
    , bFocusLocked(true)
    , rParent(*pChildWindow)
    , pImpl( new SpellDialog_Impl )
    , m_xAltTitle(m_xBuilder->weld_label("alttitleft"))
    , m_xResumeFT(m_xBuilder->weld_label("resumeft"))
    , m_xNoSuggestionsFT(m_xBuilder->weld_label("nosuggestionsft"))
    , m_xLanguageFT(m_xBuilder->weld_label("languageft"))
    , m_xLanguageLB(new SvxLanguageBox(m_xBuilder->weld_combo_box("languagelb")))
    , m_xExplainFT(m_xBuilder->weld_label("explain"))
    , m_xExplainLink(m_xBuilder->weld_link_button("explainlink"))
    , m_xNotInDictFT(m_xBuilder->weld_label("notindictft"))
    , m_xSentenceED(new SentenceEditWindow_Impl(m_xBuilder->weld_scrolled_window("scrolledwindow", true)))
    , m_xSuggestionFT(m_xBuilder->weld_label("suggestionsft"))
    , m_xSuggestionLB(m_xBuilder->weld_tree_view("suggestionslb"))
    , m_xIgnorePB(m_xBuilder->weld_button("ignore"))
    , m_xIgnoreAllPB(m_xBuilder->weld_button("ignoreall"))
    , m_xIgnoreRulePB(m_xBuilder->weld_button("ignorerule"))
    , m_xAddToDictPB(m_xBuilder->weld_button("add"))
    , m_xAddToDictMB(m_xBuilder->weld_menu_button("addmb"))
    , m_xChangePB(m_xBuilder->weld_button("change"))
    , m_xChangeAllPB(m_xBuilder->weld_button("changeall"))
    , m_xAutoCorrPB(m_xBuilder->weld_button("autocorrect"))
    , m_xCheckGrammarCB(m_xBuilder->weld_check_button("checkgrammar"))
    , m_xOptionsPB(m_xBuilder->weld_button("options"))
    , m_xUndoPB(m_xBuilder->weld_button("undo"))
    , m_xClosePB(m_xBuilder->weld_button("close"))
    , m_xToolbar(m_xBuilder->weld_toolbar("toolbar"))
    , m_xSentenceEDWeld(new weld::CustomWeld(*m_xBuilder, "sentence", *m_xSentenceED))
{
    m_xSentenceED->SetSpellDialog(this);
    m_xSentenceED->Init(m_xToolbar.get());

    m_sTitleSpellingGrammar = m_xDialog->get_title();
    m_sTitleSpelling = m_xAltTitle->get_label();

    // fdo#68794 set initial title for cases where no text has been processed
    // yet to show its language attributes
    OUString sTitle = rParent.HasGrammarChecking() ? m_sTitleSpellingGrammar : m_sTitleSpelling;
    m_xDialog->set_title(m_xDialog->strip_mnemonic(sTitle.replaceFirst("$LANGUAGE ($LOCATION)", "")));

    m_sResumeST = m_xResumeFT->get_label();
    m_sNoSuggestionsST = m_xNoSuggestionsFT->strip_mnemonic(m_xNoSuggestionsFT->get_label());

    Size aEdSize(m_xSuggestionLB->get_approximate_digit_width() * 60,
                 m_xSuggestionLB->get_height_rows(6));
    m_xSuggestionLB->set_size_request(aEdSize.Width(), -1);
    m_sIgnoreOnceST = m_xIgnorePB->get_label();
    m_xAddToDictMB->set_help_id(m_xAddToDictPB->get_help_id());
    xSpell = LinguMgr::GetSpellChecker();

    Init_Impl();

    // disable controls if service is missing
    m_xDialog->set_sensitive(xSpell.is());

    //InitHdl wants to use virtual methods, so it
    //can't be called during the ctor, so init
    //it on next event cycle post-ctor
    m_pInitHdlEvent = Application::PostUserEvent(LINK(this, SpellDialog, InitHdl));
}

SpellDialog::~SpellDialog()
{
    if (m_xOptionsDlg)
    {
        m_xOptionsDlg->response(RET_CANCEL);
        m_xOptionsDlg.reset();
    }

    if (m_pInitHdlEvent)
        Application::RemoveUserEvent(m_pInitHdlEvent);
    if (pImpl)
    {
        // save possibly modified user-dictionaries
        Reference< XSearchableDictionaryList >  xDicList( LinguMgr::GetDictionaryList() );
        if (xDicList.is())
            SaveDictionaries( xDicList );

        pImpl.reset();
    }
}

void SpellDialog::Init_Impl()
{
    // initialize handler
    m_xClosePB->connect_clicked(LINK( this, SpellDialog, CancelHdl ) );
    m_xChangePB->connect_clicked(LINK( this, SpellDialog, ChangeHdl ) );
    m_xChangeAllPB->connect_clicked(LINK( this, SpellDialog, ChangeAllHdl ) );
    m_xIgnorePB->connect_clicked(LINK( this, SpellDialog, IgnoreHdl ) );
    m_xIgnoreAllPB->connect_clicked(LINK( this, SpellDialog, IgnoreAllHdl ) );
    m_xIgnoreRulePB->connect_clicked(LINK( this, SpellDialog, IgnoreAllHdl ) );
    m_xUndoPB->connect_clicked(LINK( this, SpellDialog, UndoHdl ) );

    m_xAutoCorrPB->connect_clicked( LINK( this, SpellDialog, ExtClickHdl ) );
    m_xCheckGrammarCB->connect_toggled( LINK( this, SpellDialog, CheckGrammarHdl ));
    m_xOptionsPB->connect_clicked( LINK( this, SpellDialog, ExtClickHdl ) );

    m_xSuggestionLB->connect_row_activated( LINK( this, SpellDialog, DoubleClickChangeHdl ) );

    m_xSentenceED->SetModifyHdl(LINK ( this, SpellDialog, ModifyHdl) );

    m_xAddToDictMB->connect_selected(LINK ( this, SpellDialog, AddToDictSelectHdl ) );
    m_xAddToDictPB->connect_clicked(LINK ( this, SpellDialog, AddToDictClickHdl ) );

    m_xLanguageLB->connect_changed(LINK( this, SpellDialog, LanguageSelectHdl ) );

    // initialize language ListBox
    m_xLanguageLB->SetLanguageList(SvxLanguageListFlags::SPELL_USED, false, false, true);

    m_xSentenceED->ClearModifyFlag();
    LinguMgr::GetChangeAllList()->clear();
}

void SpellDialog::UpdateBoxes_Impl(bool bCallFromSelectHdl)
{
    m_xSuggestionLB->clear();

    SpellErrorDescription aSpellErrorDescription;
    bool bSpellErrorDescription = m_xSentenceED->GetAlternatives(aSpellErrorDescription);

    LanguageType nAltLanguage = LANGUAGE_NONE;
    Sequence< OUString > aNewWords;
    bool bIsGrammarError = false;
    if( bSpellErrorDescription )
    {
        nAltLanguage    = LanguageTag::convertToLanguageType( aSpellErrorDescription.aLocale );
        aNewWords       = aSpellErrorDescription.aSuggestions;
        bIsGrammarError = aSpellErrorDescription.bIsGrammarError;
        m_xExplainLink->set_uri( aSpellErrorDescription.sExplanationURL );
        m_xExplainFT->set_label( aSpellErrorDescription.sExplanation );
    }
    if( bSpellErrorDescription && !aSpellErrorDescription.sDialogTitle.isEmpty() )
    {
        // use this function to apply the correct image to be used...
        SetTitle_Impl( nAltLanguage );
        // then change the title to the one to be actually used
        m_xDialog->set_title(m_xDialog->strip_mnemonic(aSpellErrorDescription.sDialogTitle));
    }
    else
        SetTitle_Impl( nAltLanguage );
    if( !bCallFromSelectHdl )
        m_xLanguageLB->set_active_id( nAltLanguage );
    int nDicts = InitUserDicts();

    // enter alternatives
    for (auto& aTmp : aNewWords)
    {
        if (m_xSuggestionLB->find_text(aTmp) == -1)
            m_xSuggestionLB->append_text(aTmp);
    }
    m_xSuggestionLB->set_sensitive(aNewWords.hasElements());
    if (aNewWords.hasElements())
        m_xSuggestionLB->select(0);
    else
        m_xSuggestionLB->append_text(m_sNoSuggestionsST);
    m_xAutoCorrPB->set_sensitive(aNewWords.hasElements());
    m_xSuggestionFT->set_sensitive(aNewWords.hasElements());
    m_xChangePB->set_sensitive(aNewWords.hasElements());
    m_xChangeAllPB->set_sensitive(aNewWords.hasElements());
    bool bShowChangeAll = !bIsGrammarError;
    m_xChangeAllPB->set_visible( bShowChangeAll );
    m_xExplainFT->set_visible( !bShowChangeAll );
    m_xLanguageLB->set_sensitive( bShowChangeAll );
    m_xIgnoreAllPB->set_visible( bShowChangeAll );

    m_xAddToDictMB->set_visible( bShowChangeAll && nDicts > 1 && !comphelper::LibreOfficeKit::isActive());
    m_xAddToDictPB->set_visible( bShowChangeAll && nDicts <= 1 && !comphelper::LibreOfficeKit::isActive());
    m_xIgnoreRulePB->set_visible( !bShowChangeAll );
    m_xIgnoreRulePB->set_sensitive(bSpellErrorDescription && !aSpellErrorDescription.sRuleId.isEmpty());
    m_xAutoCorrPB->set_visible( bShowChangeAll && rParent.HasAutoCorrection() );

    bool bOldShowGrammar = m_xCheckGrammarCB->get_visible();
    bool bOldShowExplain = m_xExplainLink->get_visible();

    m_xCheckGrammarCB->set_visible(rParent.HasGrammarChecking());
    m_xExplainLink->set_visible(!m_xExplainLink->get_uri().isEmpty());
    if (m_xExplainFT->get_label().isEmpty())
    {
        m_xExplainFT->hide();
        m_xExplainLink->hide();
    }

    if (bOldShowExplain != m_xExplainLink->get_visible() || bOldShowGrammar != m_xCheckGrammarCB->get_visible())
        m_xDialog->resize_to_request();
}

void SpellDialog::SpellContinue_Impl(std::unique_ptr<UndoChangeGroupGuard>* pGuard, bool bUseSavedSentence, bool bIgnoreCurrentError)
{
    //initially or after the last error of a sentence MarkNextError will fail
    //then GetNextSentence() has to be called followed again by MarkNextError()
    //MarkNextError is not initially called if the UndoEdit mode is active
    bool bNextSentence = false;
    if (!m_xSentenceED)
    {
        return;
    }

    if(!((!m_xSentenceED->IsUndoEditMode() && m_xSentenceED->MarkNextError( bIgnoreCurrentError, xSpell )) ||
            ( bNextSentence = GetNextSentence_Impl(pGuard, bUseSavedSentence, m_xSentenceED->IsUndoEditMode()) && m_xSentenceED->MarkNextError( false, xSpell ))))
        return;

    SpellErrorDescription aSpellErrorDescription;
    bool bSpellErrorDescription = m_xSentenceED->GetAlternatives(aSpellErrorDescription);
    if (bSpellErrorDescription)
    {
        UpdateBoxes_Impl();
        weld::Widget* aControls[] =
        {
            m_xNotInDictFT.get(),
            m_xSentenceED->GetDrawingArea(),
            m_xLanguageFT.get()
        };
        for (weld::Widget* pWidget : aControls)
            pWidget->set_sensitive(true);
    }
    if( bNextSentence )
    {
        //remove undo if a new sentence is active
        m_xSentenceED->ResetUndo();
        m_xUndoPB->set_sensitive(false);
    }
}
/* Initialize, asynchronous to prevent virtual calls
   from a constructor
 */
IMPL_LINK_NOARG( SpellDialog, InitHdl, void*, void)
{
    m_pInitHdlEvent = nullptr;
    m_xDialog->freeze();
    //show or hide AutoCorrect depending on the modules abilities
    m_xAutoCorrPB->set_visible(rParent.HasAutoCorrection());
    SpellContinue_Impl(nullptr);
    m_xSentenceED->ResetUndo();
    m_xUndoPB->set_sensitive(false);

    // get current language
    UpdateBoxes_Impl();

    // fill dictionary PopupMenu
    InitUserDicts();

    LockFocusChanges(true);
    if(m_xSentenceED->IsEnabled())
        m_xSentenceED->GrabFocus();
    else if( m_xChangePB->get_sensitive() )
        m_xChangePB->grab_focus();
    else if( m_xIgnorePB->get_sensitive() )
        m_xIgnorePB->grab_focus();
    else if( m_xClosePB->get_sensitive() )
        m_xClosePB->grab_focus();
    LockFocusChanges(false);
    //show grammar CheckBox depending on the modules abilities
    m_xCheckGrammarCB->set_active(rParent.IsGrammarChecking());
    m_xDialog->thaw();
};

IMPL_LINK( SpellDialog, ExtClickHdl, weld::Button&, rBtn, void )
{
    if (m_xOptionsPB.get() == &rBtn)
        StartSpellOptDlg_Impl();
    else if (m_xAutoCorrPB.get() == &rBtn)
    {
        //get the currently selected wrong word
        OUString sCurrentErrorText = m_xSentenceED->GetErrorText();
        //get the wrong word from the XSpellAlternative
        SpellErrorDescription aSpellErrorDescription;
        bool bSpellErrorDescription = m_xSentenceED->GetAlternatives(aSpellErrorDescription);
        if (bSpellErrorDescription)
        {
            OUString sWrong(aSpellErrorDescription.sErrorText);
            //if the word has not been edited in the MultiLineEdit then
            //the current suggestion should be used
            //if it's not the 'no suggestions' entry
            if(sWrong == sCurrentErrorText &&
                    m_xSuggestionLB->get_sensitive() && m_xSuggestionLB->get_selected_index() != -1 &&
                    m_sNoSuggestionsST != m_xSuggestionLB->get_selected_text())
            {
                sCurrentErrorText = m_xSuggestionLB->get_selected_text();
            }
            if(sWrong != sCurrentErrorText)
            {
                SvxPrepareAutoCorrect( sWrong, sCurrentErrorText );
                LanguageType eLang = GetSelectedLang_Impl();
                rParent.AddAutoCorrection( sWrong, sCurrentErrorText, eLang );
                //correct the word immediately
                ChangeHdl(*m_xAutoCorrPB);
            }
        }
    }
}

IMPL_LINK_NOARG(SpellDialog, CheckGrammarHdl, weld::Toggleable&, void)
{
    rParent.SetGrammarChecking(m_xCheckGrammarCB->get_active());
    Impl_Restore(true);
}

void SpellDialog::StartSpellOptDlg_Impl()
{
    auto xSet = std::make_shared<SfxItemSetFixed<SID_AUTOSPELL_CHECK,SID_AUTOSPELL_CHECK>>( SfxGetpApp()->GetPool() );
    m_xOptionsDlg = std::make_shared<SfxSingleTabDialogController>(
        m_xDialog.get(), xSet.get(), "content", "cui/ui/spelloptionsdialog.ui", "SpellOptionsDialog");

    std::unique_ptr<SfxTabPage> xPage = SvxLinguTabPage::Create(m_xOptionsDlg->get_content_area(), m_xOptionsDlg.get(), xSet.get());
    static_cast<SvxLinguTabPage*>(xPage.get())->HideGroups( GROUP_MODULES );
    m_xOptionsDlg->SetTabPage(std::move(xPage));
    weld::GenericDialogController::runAsync(m_xOptionsDlg, [this, xSet] (sal_uInt32 nResult) {
        if (RET_OK == nResult)
        {
            InitUserDicts();
            const SfxItemSet* pOutSet = m_xOptionsDlg->GetOutputItemSet();
            if(pOutSet)
                OfaTreeOptionsDialog::ApplyLanguageOptions(*pOutSet);
        }
    });
}

namespace
{
    OUString getDotReplacementString(const OUString &rErrorText, const OUString &rSuggestedReplacement)
    {
        OUString aString = rErrorText;

        //dots are sometimes part of the spelled word but they are not necessarily part of the replacement
        bool bDot = aString.endsWith(".");

        aString = rSuggestedReplacement;

        if(bDot && (aString.isEmpty() || !aString.endsWith(".")))
            aString += ".";

        return aString;
    }
}

OUString SpellDialog::getReplacementString() const
{
    OUString sOrigString = m_xSentenceED->GetErrorText();

    OUString sReplacement(sOrigString);

    if(m_xSuggestionLB->get_sensitive() &&
            m_xSuggestionLB->get_selected_index() != -1 &&
            m_sNoSuggestionsST != m_xSuggestionLB->get_selected_text())
        sReplacement = m_xSuggestionLB->get_selected_text();

    return getDotReplacementString(sOrigString, sReplacement);
}

IMPL_LINK_NOARG(SpellDialog, DoubleClickChangeHdl, weld::TreeView&, bool)
{
    ChangeHdl(*m_xChangePB);
    return true;
}

/* tdf#132822 start an undo group in ctor and close it in the dtor. This can
   then be passed to SpellContinue_Impl which can delete it in advance of its
   natural scope to force closing the undo group if SpellContinue_Impl needs to
   fetch a new paragraph and discard all undo information which can only be
   done properly if there are no open undo groups */
class UndoChangeGroupGuard
{
private:
    SentenceEditWindow_Impl& m_rSentenceED;
public:
    UndoChangeGroupGuard(SentenceEditWindow_Impl& rSentenceED)
        : m_rSentenceED(rSentenceED)
    {
        m_rSentenceED.UndoActionStart(SPELLUNDO_CHANGE_GROUP);
    }
    ~UndoChangeGroupGuard()
    {
        m_rSentenceED.UndoActionEnd();
    }
};

IMPL_LINK_NOARG(SpellDialog, ChangeHdl, weld::Button&, void)
{
    if (m_xSentenceED->IsUndoEditMode())
    {
        SpellContinue_Impl();
    }
    else
    {
        auto xGuard(std::make_unique<UndoChangeGroupGuard>(*m_xSentenceED));
        OUString aString = getReplacementString();
        m_xSentenceED->ChangeMarkedWord(aString, GetSelectedLang_Impl());
        SpellContinue_Impl(&xGuard);
    }
    if(!m_xChangePB->get_sensitive())
        m_xIgnorePB->grab_focus();
}

IMPL_LINK_NOARG(SpellDialog, ChangeAllHdl, weld::Button&, void)
{
    auto xGuard(std::make_unique<UndoChangeGroupGuard>(*m_xSentenceED));
    OUString aString = getReplacementString();
    LanguageType eLang = GetSelectedLang_Impl();

    // add new word to ChangeAll list
    OUString  aOldWord( m_xSentenceED->GetErrorText() );
    SvxPrepareAutoCorrect( aOldWord, aString );
    Reference<XDictionary> aXDictionary = LinguMgr::GetChangeAllList();
    DictionaryError nAdded = AddEntryToDic( aXDictionary,
            aOldWord, true,
            aString );

    if(nAdded == DictionaryError::NONE)
    {
        std::unique_ptr<SpellUndoAction_Impl> pAction(new SpellUndoAction_Impl(
                        SPELLUNDO_CHANGE_ADD_TO_DICTIONARY, aDialogUndoLink));
        pAction->SetDictionary(aXDictionary);
        pAction->SetAddedWord(aOldWord);
        m_xSentenceED->AddUndoAction(std::move(pAction));
    }

    m_xSentenceED->ChangeMarkedWord(aString, eLang);
    SpellContinue_Impl(&xGuard);
}

IMPL_LINK( SpellDialog, IgnoreAllHdl, weld::Button&, rButton, void )
{
    auto xGuard(std::make_unique<UndoChangeGroupGuard>(*m_xSentenceED));
    // add word to IgnoreAll list
    Reference< XDictionary > aXDictionary = LinguMgr::GetIgnoreAllList();
    //in case the error has been changed manually it has to be restored
    m_xSentenceED->RestoreCurrentError();
    if (&rButton == m_xIgnoreRulePB.get())
    {
        SpellErrorDescription aSpellErrorDescription;
        bool bSpellErrorDescription = m_xSentenceED->GetAlternatives(aSpellErrorDescription);
        try
        {
            if( bSpellErrorDescription && aSpellErrorDescription.xGrammarChecker.is() )
            {
                aSpellErrorDescription.xGrammarChecker->ignoreRule(aSpellErrorDescription.sRuleId,
                    aSpellErrorDescription.aLocale);
                // refresh the layout (workaround to launch a dictionary event)
                aXDictionary->setActive(false);
                aXDictionary->setActive(true);
            }
        }
        catch( const uno::Exception& )
        {
        }
    }
    else
    {
        OUString sErrorText(m_xSentenceED->GetErrorText());
        DictionaryError nAdded = AddEntryToDic( aXDictionary,
            sErrorText, false,
            OUString() );
        if (nAdded == DictionaryError::NONE)
        {
            std::unique_ptr<SpellUndoAction_Impl> pAction(new SpellUndoAction_Impl(
                            SPELLUNDO_CHANGE_ADD_TO_DICTIONARY, aDialogUndoLink));
            pAction->SetDictionary(aXDictionary);
            pAction->SetAddedWord(sErrorText);
            m_xSentenceED->AddUndoAction(std::move(pAction));
        }
    }

    SpellContinue_Impl(&xGuard);
}

IMPL_LINK_NOARG(SpellDialog, UndoHdl, weld::Button&, void)
{
    m_xSentenceED->Undo();
    if(!m_xSentenceED->GetUndoActionCount())
        m_xUndoPB->set_sensitive(false);
}


IMPL_LINK( SpellDialog, DialogUndoHdl, SpellUndoAction_Impl&, rAction, void )
{
    switch(rAction.GetId())
    {
        case SPELLUNDO_CHANGE_TEXTENGINE:
        {
            if(rAction.IsEnableChangePB())
                m_xChangePB->set_sensitive(false);
            if(rAction.IsEnableChangeAllPB())
                m_xChangeAllPB->set_sensitive(false);
        }
        break;
        case SPELLUNDO_CHANGE_NEXTERROR:
        {
            m_xSentenceED->MoveErrorMarkTo(static_cast<sal_Int32>(rAction.GetOldErrorStart()),
                                           static_cast<sal_Int32>(rAction.GetOldErrorEnd()),
                                           false);
            if(rAction.IsErrorLanguageSelected())
            {
                UpdateBoxes_Impl();
            }
        }
        break;
        case SPELLUNDO_CHANGE_ADD_TO_DICTIONARY:
        {
            if(rAction.GetDictionary().is())
                rAction.GetDictionary()->remove(rAction.GetAddedWord());
        }
        break;
        case SPELLUNDO_MOVE_ERROREND :
        {
            if(rAction.GetOffset() != 0)
                m_xSentenceED->MoveErrorEnd(rAction.GetOffset());
        }
        break;
        case SPELLUNDO_UNDO_EDIT_MODE :
        {
            //refill the dialog with the currently spelled sentence - throw away all changes
            SpellContinue_Impl(nullptr, true);
        }
        break;
        case SPELLUNDO_ADD_IGNORE_RULE:
            //undo of ignored rules is not supported
        break;
    }
}

void SpellDialog::Impl_Restore(bool bUseSavedSentence)
{
    //clear the "ChangeAllList"
    LinguMgr::GetChangeAllList()->clear();
    //get a new sentence
    m_xSentenceED->SetText(OUString());
    m_xSentenceED->ResetModified();
    //Resolves: fdo#39348 refill the dialog with the currently spelled sentence
    SpellContinue_Impl(nullptr, bUseSavedSentence);
    m_xIgnorePB->set_label(m_sIgnoreOnceST);
}

IMPL_LINK_NOARG(SpellDialog, IgnoreHdl, weld::Button&, void)
{
    if (m_sResumeST == m_xIgnorePB->get_label())
    {
        Impl_Restore(false);
    }
    else
    {
        //in case the error has been changed manually it has to be restored,
        // since the users choice now was to ignore the error
        m_xSentenceED->RestoreCurrentError();

        // the word is being ignored
        SpellContinue_Impl(nullptr, false, true);
    }
}

void SpellDialog::Close()
{
    if (IsClosing())
        return;

    // We have to call ToggleChildWindow directly; calling SfxDispatcher's
    // Execute() does not work here when we are in a document with protected
    // section - in that case, the cursor can move from the editable field to
    // the protected area, and the slots get disabled because of
    // SfxDisableFlags::SwOnProtectedCursor (see FN_SPELL_GRAMMAR_DIALOG in .sdi).
    if (SfxViewFrame* pViewFrame = SfxViewFrame::Current())
        pViewFrame->ToggleChildWindow(rParent.GetType());
}

LanguageType SpellDialog::GetSelectedLang_Impl() const
{
    LanguageType nLang = m_xLanguageLB->get_active_id();
    return nLang;
}

IMPL_LINK_NOARG(SpellDialog, LanguageSelectHdl, weld::ComboBox&, void)
{
    //If selected language changes, then add->list should be regenerated to
    //match
    InitUserDicts();

    //if currently an error is selected then search for alternatives for
    //this word and fill the alternatives ListBox accordingly
    OUString sError = m_xSentenceED->GetErrorText();
    m_xSuggestionLB->clear();
    if (!sError.isEmpty())
    {
        LanguageType eLanguage = m_xLanguageLB->get_active_id();
        Reference <XSpellAlternatives> xAlt = xSpell->spell( sError, static_cast<sal_uInt16>(eLanguage),
                                            Sequence< PropertyValue >() );
        if( xAlt.is() )
            m_xSentenceED->SetAlternatives( xAlt );
        else
        {
            m_xSentenceED->ChangeMarkedWord( sError, eLanguage );
            SpellContinue_Impl();
        }

        m_xSentenceED->AddUndoAction(std::make_unique<SpellUndoAction_Impl>(SPELLUNDO_CHANGE_LANGUAGE, aDialogUndoLink));
    }
    SpellDialog::UpdateBoxes_Impl(true);
}

void SpellDialog::SetTitle_Impl(LanguageType nLang)
{
    OUString sTitle = rParent.HasGrammarChecking() ? m_sTitleSpellingGrammar : m_sTitleSpelling;
    sTitle = sTitle.replaceFirst( "$LANGUAGE ($LOCATION)", SvtLanguageTable::GetLanguageString(nLang) );
    m_xDialog->set_title(m_xDialog->strip_mnemonic(sTitle));
}

int SpellDialog::InitUserDicts()
{
    const LanguageType nLang = m_xLanguageLB->get_active_id();

    // get list of dictionaries
    Reference< XSearchableDictionaryList >  xDicList( LinguMgr::GetDictionaryList() );
    if (xDicList.is())
    {
        // add active, positive dictionary to dic-list (if not already done).
        // This is to ensure that there is at least on dictionary to which
        // words could be added.
        Reference< XDictionary >  xDic( LinguMgr::GetStandardDic() );
        if (xDic.is())
            xDic->setActive( true );

        pImpl->aDics = xDicList->getDictionaries();
    }

    SvtLinguConfig aCfg;

    // list suitable dictionaries
    bool bEnable = false;
    m_xAddToDictMB->clear();
    sal_uInt16 nItemId = 1;     // menu items should be enumerated from 1 and not 0
    for (auto& xDicTmp : pImpl->aDics)
    {
        if (!xDicTmp.is() || LinguMgr::GetIgnoreAllList() == xDicTmp)
            continue;

        uno::Reference< frame::XStorable > xStor( xDicTmp, uno::UNO_QUERY );
        LanguageType nActLanguage = LanguageTag( xDicTmp->getLocale() ).getLanguageType();
        if( xDicTmp->isActive()
            &&  xDicTmp->getDictionaryType() != linguistic2::DictionaryType_NEGATIVE
            && (nLang == nActLanguage || LANGUAGE_NONE == nActLanguage )
            && (!xStor.is() || !xStor->isReadonly()) )
        {
            bEnable = true;

            OUString aDictionaryImageUrl;
            uno::Reference< lang::XServiceInfo > xSvcInfo( xDicTmp, uno::UNO_QUERY );
            if (xSvcInfo.is())
            {
                aDictionaryImageUrl = aCfg.GetSpellAndGrammarContextDictionaryImage(
                        xSvcInfo->getImplementationName());
            }

            m_xAddToDictMB->append_item(OUString::number(nItemId), xDicTmp->getName(), aDictionaryImageUrl);

            ++nItemId;
        }
    }
    m_xAddToDictMB->set_sensitive( bEnable );
    m_xAddToDictPB->set_sensitive( bEnable );

    int nDicts = nItemId-1;

    m_xAddToDictMB->set_visible(nDicts > 1 && !comphelper::LibreOfficeKit::isActive());
    m_xAddToDictPB->set_visible(nDicts <= 1 && !comphelper::LibreOfficeKit::isActive());

    return nDicts;
}

IMPL_LINK_NOARG(SpellDialog, AddToDictClickHdl, weld::Button&, void)
{
    AddToDictionaryExecute(OUString::number(1));
}

IMPL_LINK(SpellDialog, AddToDictSelectHdl, const OUString&, rIdent, void)
{
    AddToDictionaryExecute(rIdent);
}

void SpellDialog::AddToDictionaryExecute(const OUString& rItemId)
{
    auto xGuard(std::make_unique<UndoChangeGroupGuard>(*m_xSentenceED));

    //GetErrorText() returns the current error even if the text is already
    //manually changed
    const OUString aNewWord = m_xSentenceED->GetErrorText();

    OUString aDicName(m_xAddToDictMB->get_item_label(rItemId));

    uno::Reference< linguistic2::XDictionary >      xDic;
    uno::Reference< linguistic2::XSearchableDictionaryList >  xDicList( LinguMgr::GetDictionaryList() );
    if (xDicList.is())
        xDic = xDicList->getDictionaryByName( aDicName );

    DictionaryError nAddRes = DictionaryError::UNKNOWN;
    if (xDic.is())
    {
        nAddRes = AddEntryToDic( xDic, aNewWord, false, OUString() );
        // save modified user-dictionary if it is persistent
        uno::Reference< frame::XStorable >  xSavDic( xDic, uno::UNO_QUERY );
        if (xSavDic.is())
            xSavDic->store();

        if (nAddRes == DictionaryError::NONE)
        {
            std::unique_ptr<SpellUndoAction_Impl> pAction(new SpellUndoAction_Impl(
                            SPELLUNDO_CHANGE_ADD_TO_DICTIONARY, aDialogUndoLink));
            pAction->SetDictionary( xDic );
            pAction->SetAddedWord( aNewWord );
            m_xSentenceED->AddUndoAction( std::move(pAction) );
        }
        // failed because there is already an entry?
        if (DictionaryError::NONE != nAddRes && xDic->getEntry( aNewWord ).is())
            nAddRes = DictionaryError::NONE;
    }
    if (DictionaryError::NONE != nAddRes)
    {
        SvxDicError(m_xDialog.get(), nAddRes);
        return; // don't continue
    }

    // go on
    SpellContinue_Impl(&xGuard);
}

IMPL_LINK_NOARG(SpellDialog, ModifyHdl, LinkParamNone*, void)
{
    m_xSuggestionLB->unselect_all();
    m_xSuggestionLB->set_sensitive(false);
    m_xAutoCorrPB->set_sensitive(false);
    std::unique_ptr<SpellUndoAction_Impl> pSpellAction(new SpellUndoAction_Impl(SPELLUNDO_CHANGE_TEXTENGINE, aDialogUndoLink));
    if(!m_xChangeAllPB->get_sensitive())
    {
        m_xChangeAllPB->set_sensitive(true);
        pSpellAction->SetEnableChangeAllPB();
    }
    if(!m_xChangePB->get_sensitive())
    {
        m_xChangePB->set_sensitive(true);
        pSpellAction->SetEnableChangePB();
    }
    m_xSentenceED->AddUndoAction(std::move(pSpellAction));
}

IMPL_LINK_NOARG(SpellDialog, CancelHdl, weld::Button&, void)
{
    //apply changes and ignored text parts first - if there are any
    if (m_xSentenceED->IsModified())
    {
        rParent.ApplyChangedSentence(m_xSentenceED->CreateSpellPortions(), false);
    }
    Close();
}

void SpellDialog::ToplevelFocusChanged()
{
    /* #i38338#
    *   FIXME: LoseFocus and GetFocus are signals from vcl that
    *   a window actually got/lost the focus, it never should be
    *   forwarded from another window, that is simply wrong.
    *   FIXME: overriding the virtual methods GetFocus and LoseFocus
    *   in SpellDialogChildWindow by making them pure is at least questionable.
    *   The only sensible thing would be to call the new Method differently,
    *   e.g. DialogGot/LostFocus or so.
    */
    if (!m_xDialog->get_visible() || bFocusLocked)
        return;

    if (m_xDialog->has_toplevel_focus())
    {
        //notify the child window of the focus change
        rParent.GetFocus();
    }
    else
    {
        //notify the child window of the focus change
        rParent.LoseFocus();
    }
}

void SpellDialog::Activate()
{
    SfxModelessDialogController::Activate();
    ToplevelFocusChanged();
}

void SpellDialog::Deactivate()
{
    SfxModelessDialogController::Deactivate();
    ToplevelFocusChanged();
}

void SpellDialog::InvalidateDialog()
{
    if( bFocusLocked )
        return;
    m_xIgnorePB->set_label(m_sResumeST);
    weld::Widget* aDisableArr[] =
    {
        m_xNotInDictFT.get(),
        m_xSentenceED->GetDrawingArea(),
        m_xSuggestionFT.get(),
        m_xSuggestionLB.get(),
        m_xLanguageFT.get(),
        m_xLanguageLB->get_widget(),
        m_xIgnoreAllPB.get(),
        m_xIgnoreRulePB.get(),
        m_xAddToDictMB.get(),
        m_xAddToDictPB.get(),
        m_xChangePB.get(),
        m_xChangeAllPB.get(),
        m_xAutoCorrPB.get(),
        m_xUndoPB.get()
    };
    for (weld::Widget* pWidget : aDisableArr)
        pWidget->set_sensitive(false);

    SfxModelessDialogController::Deactivate();
}

bool SpellDialog::GetNextSentence_Impl(std::unique_ptr<UndoChangeGroupGuard>* pGuard, bool bUseSavedSentence, bool bRecheck)
{
    bool bRet = false;
    if(!bUseSavedSentence)
    {
        //apply changes and ignored text parts
        rParent.ApplyChangedSentence(m_xSentenceED->CreateSpellPortions(), bRecheck);
    }
    m_xSentenceED->ResetIgnoreErrorsAt();
    m_xSentenceED->ResetModified();
    SpellPortions aSentence = bUseSavedSentence ? m_aSavedSentence : rParent.GetNextWrongSentence( bRecheck );
    if(!bUseSavedSentence)
        m_aSavedSentence = aSentence;
    bool bHasReplaced = false;
    while(!aSentence.empty())
    {
        //apply all changes that are already part of the "ChangeAllList"
        //returns true if the list still contains errors after the changes have been applied

        if(!ApplyChangeAllList_Impl(aSentence, bHasReplaced))
        {
            rParent.ApplyChangedSentence(aSentence, bRecheck);
            aSentence = rParent.GetNextWrongSentence( bRecheck );
        }
        else
            break;
    }

    if(!aSentence.empty())
    {
        OUStringBuffer sText;
        for (auto const& elem : aSentence)
        {
            // hidden text has to be ignored
            if(!elem.bIsHidden)
                sText.append(elem.sText);
        }
        // tdf#132822 fire undo-stack UndoActionEnd to close undo stack because we're about to throw away the paragraph entirely
        if (pGuard)
            pGuard->reset();
        m_xSentenceED->SetText(sText.makeStringAndClear());
        sal_Int32 nStartPosition = 0;
        sal_Int32 nEndPosition = 0;

        for (auto const& elem : aSentence)
        {
            // hidden text has to be ignored
            if(!elem.bIsHidden)
            {
                nEndPosition += elem.sText.getLength();
                if(elem.xAlternatives.is())
                {
                    SpellErrorDescription aDesc( false, elem.xAlternatives->getWord(),
                                    elem.xAlternatives->getLocale(), elem.xAlternatives->getAlternatives(), nullptr);
                    SfxGrabBagItem aSpellErrorDescription(EE_CHAR_GRABBAG);
                    aSpellErrorDescription.GetGrabBag()["SpellErrorDescription"] <<= aDesc.toSequence();
                    m_xSentenceED->SetAttrib(aSpellErrorDescription, nStartPosition, nEndPosition);
                }
                else if(elem.bIsGrammarError )
                {
                    beans::PropertyValues  aProperties = elem.aGrammarError.aProperties;
                    OUString sFullCommentURL;
                    sal_Int32 i = 0;
                    while ( sFullCommentURL.isEmpty() && i < aProperties.getLength() )
                    {
                        if ( aProperties[i].Name == "FullCommentURL" )
                        {
                            uno::Any aValue = aProperties[i].Value;
                            aValue >>= sFullCommentURL;
                        }
                        ++i;
                    }

                    SpellErrorDescription aDesc( true,
                        elem.sText,
                        LanguageTag::convertToLocale( elem.eLanguage ),
                        elem.aGrammarError.aSuggestions,
                        elem.xGrammarChecker,
                        &elem.sDialogTitle,
                        &elem.aGrammarError.aFullComment,
                        &elem.aGrammarError.aRuleIdentifier,
                        &sFullCommentURL );

                    SfxGrabBagItem aSpellErrorDescriptionItem(EE_CHAR_GRABBAG);
                    aSpellErrorDescriptionItem.GetGrabBag()["SpellErrorDescription"] <<= aDesc.toSequence();
                    m_xSentenceED->SetAttrib(aSpellErrorDescriptionItem, nStartPosition, nEndPosition);
                }

                if (elem.bIsField)
                    m_xSentenceED->SetAttrib(SvxColorItem(COL_LIGHTGRAY, EE_CHAR_BKGCOLOR), nStartPosition, nEndPosition);
                m_xSentenceED->SetAttrib(SvxLanguageItem(elem.eLanguage, EE_CHAR_LANGUAGE), nStartPosition, nEndPosition);
                nStartPosition = nEndPosition;
            }
        }
        //the edit field needs to be modified to apply the change from the ApplyChangeAllList
        if(!bHasReplaced)
            m_xSentenceED->ClearModifyFlag();
        m_xSentenceED->ResetUndo();
        m_xUndoPB->set_sensitive(false);
        bRet = nStartPosition > 0;
    }
    return bRet;
}
/*-------------------------------------------------------------------------
    replace errors that have a replacement in the ChangeAllList
    returns false if the result doesn't contain errors after the replacement
  -----------------------------------------------------------------------*/
bool SpellDialog::ApplyChangeAllList_Impl(SpellPortions& rSentence, bool &bHasReplaced)
{
    bHasReplaced = false;
    bool bRet = true;
    Reference<XDictionary> xChangeAll = LinguMgr::GetChangeAllList();
    if(!xChangeAll->getCount())
        return bRet;
    bRet = false;
    for (auto & elem : rSentence)
    {
        if(elem.xAlternatives.is())
        {
            const OUString &rString = elem.sText;

            Reference<XDictionaryEntry> xEntry = xChangeAll->getEntry(rString);

            if(xEntry.is())
            {
                elem.sText = getDotReplacementString(rString, xEntry->getReplacementText());
                elem.xAlternatives = nullptr;
                bHasReplaced = true;
            }
            else
                bRet = true;
        }
        else if( elem.bIsGrammarError )
            bRet = true;
    }
    return bRet;
}

SentenceEditWindow_Impl::SentenceEditWindow_Impl(std::unique_ptr<weld::ScrolledWindow> xScrolledWindow)
    : m_xScrolledWindow(std::move(xScrolledWindow))
    , m_pSpellDialog(nullptr)
    , m_pToolbar(nullptr)
    , m_nErrorStart(0)
    , m_nErrorEnd(0)
    , m_bIsUndoEditMode(false)
{
    m_xScrolledWindow->connect_vadjustment_changed(LINK(this, SentenceEditWindow_Impl, ScrollHdl));
}

void SentenceEditWindow_Impl::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    Size aSize(pDrawingArea->get_approximate_digit_width() * 60,
               pDrawingArea->get_text_height() * 6);
    pDrawingArea->set_size_request(aSize.Width(), aSize.Height());
    WeldEditView::SetDrawingArea(pDrawingArea);
    // tdf#132288 don't merge equal adjacent attributes
    m_xEditEngine->DisableAttributeExpanding();

    m_xEditEngine->SetStatusEventHdl(LINK(this, SentenceEditWindow_Impl, EditStatusHdl));

    SetDocumentColor(pDrawingArea);
}

void SentenceEditWindow_Impl::SetDocumentColor(weld::DrawingArea* pDrawingArea)
{
    if (!pDrawingArea || !m_xEditView || !m_xEditEngine)
        return;
    // tdf#142631 use document background color in this widget
    Color aBgColor = svtools::ColorConfig().GetColorValue(svtools::DOCCOLOR).nColor;
    OutputDevice& rDevice = pDrawingArea->get_ref_device();
    rDevice.SetBackground(aBgColor);
    m_xEditView->SetBackgroundColor(aBgColor);
    m_xEditEngine->SetBackgroundColor(aBgColor);
}

void SentenceEditWindow_Impl::StyleUpdated()
{
    SetDocumentColor(GetDrawingArea());
    WeldEditView::StyleUpdated();
}

IMPL_LINK_NOARG(SentenceEditWindow_Impl, EditStatusHdl, EditStatus&, void)
{
    SetScrollBarRange();
    DoScroll();
}

IMPL_LINK_NOARG(SentenceEditWindow_Impl, ScrollHdl, weld::ScrolledWindow&, void)
{
    DoScroll();
}

void SentenceEditWindow_Impl::DoScroll()
{
    if (m_xEditView)
    {
        auto currentDocPos = m_xEditView->GetVisArea().Top();
        auto nDiff = currentDocPos - m_xScrolledWindow->vadjustment_get_value();
        // we expect SetScrollBarRange callback to be triggered by Scroll
        // to set where we ended up
        m_xEditView->Scroll(0, nDiff);
    }
}

void SentenceEditWindow_Impl::EditViewScrollStateChange()
{
    // editengine height has changed or editview scroll pos has changed
    SetScrollBarRange();
}

void SentenceEditWindow_Impl::SetScrollBarRange()
{
    EditEngine *pEditEngine = GetEditEngine();
    if (!pEditEngine)
        return;
    if (!m_xScrolledWindow)
        return;
    EditView* pEditView = GetEditView();
    if (!pEditView)
        return;

    int nVUpper = pEditEngine->GetTextHeight();
    int nVCurrentDocPos = pEditView->GetVisArea().Top();
    const Size aOut(pEditView->GetOutputArea().GetSize());
    int nVStepIncrement = aOut.Height() * 2 / 10;
    int nVPageIncrement = aOut.Height() * 8 / 10;
    int nVPageSize = aOut.Height();

    /* limit the page size to below nUpper because gtk's gtk_scrolled_window_start_deceleration has
       effectively...

       lower = gtk_adjustment_get_lower
       upper = gtk_adjustment_get_upper - gtk_adjustment_get_page_size

       and requires that upper > lower or the deceleration animation never ends
    */
    nVPageSize = std::min(nVPageSize, nVUpper);

    m_xScrolledWindow->vadjustment_configure(nVCurrentDocPos, 0, nVUpper,
                                             nVStepIncrement, nVPageIncrement, nVPageSize);
    m_xScrolledWindow->set_vpolicy(nVUpper > nVPageSize ? VclPolicyType::ALWAYS : VclPolicyType::NEVER);
}

SentenceEditWindow_Impl::~SentenceEditWindow_Impl()
{
}

namespace
{
    const EECharAttrib* FindCharAttrib(int nPosition, sal_uInt16 nWhich, std::vector<EECharAttrib>& rAttribList)
    {
        for (auto it = rAttribList.rbegin(); it != rAttribList.rend(); ++it)
        {
            const auto& rTextAtr = *it;
            if (rTextAtr.pAttr->Which() != nWhich)
                continue;
            if (rTextAtr.nStart <= nPosition && rTextAtr.nEnd >= nPosition)
            {
                return &rTextAtr;
            }
        }

        return nullptr;
    }

    void ExtractErrorDescription(const EECharAttrib& rEECharAttrib, SpellErrorDescription& rSpellErrorDescription)
    {
        css::uno::Sequence<css::uno::Any> aSequence;
        const auto pGrabBag = static_cast<const SfxGrabBagItem*>(rEECharAttrib.pAttr)->GetGrabBag();
        const auto iter = pGrabBag.find("SpellErrorDescription");
        assert(iter != pGrabBag.end());
        iter->second >>= aSequence;
        rSpellErrorDescription.fromSequence(aSequence);
    }
}

/*-------------------------------------------------------------------------
    The selection before inputting a key may have a range or not
    and it may be inside or outside of field or error attributes.
    A range may include the attribute partially, completely or together
    with surrounding text. It may also contain more than one attribute
    or no attribute at all.
    Depending on this starting conditions some actions are necessary:
    Attempts to delete a field are only allowed if the selection is the same
    as the field's selection. Otherwise the field has to be selected and the key
    input action has to be skipped.
    Input of text at the start of the field requires the field attribute to be
    corrected - it is not allowed to grow.

    In case of errors the appending of text should grow the error attribute because
    that is what the user usually wants to do.

    Backspace at the start of the attribute requires to find out if a field ends
    directly in front of the cursor position. In case of a field this attribute has to be
    selected otherwise the key input method is allowed.

    All changes outside of the error attributes switch the dialog mode to a "Undo edit" state that
    removes all visible attributes and switches off further attribute checks.
    Undo in this restarts the dialog with a current sentence newly presented.
    All changes to the sentence are undone including the ones before the "Undo edit state" has been reached

    We end up with 9 types of selection
    1 (LEFT_NO)     - no range, start of attribute - can also be 3 at the same time
    2 (INSIDE_NO)   - no range, inside of attribute
    3 (RIGHT_NO)    - no range, end of attribute - can also be 1 at the same time
    4 (FULL)        - range, same as attribute
    5 (INSIDE_YES)  - range, inside of the attribute
    6 (BRACE)- range, from outside of the attribute to the inside or
                including the complete attribute and something outside,
                maybe more than one attribute
    7 (OUTSIDE_NO)  - no range, not at an attribute
    8 (OUTSIDE_YES) - range, completely outside of all attributes

    What has to be done depending on the attribute type involved
    possible actions:   UE - Undo edit mode
                        CO - Continue, no additional action is required
                        FS - Field has to be completely selected
                        EX - The attribute has to be expanded to include the added text

    1 - backspace                   delete                      any other
        UE                          on field FS on error CO     on field FS on error CO

    2 - on field FS on error C
    3 - backspace                   delete                      any other
        on field FS on error CO     UE                          on field UE on error EX

    if 1 and 3 happen to apply both then backspace and other handling is 1 delete is 3

    4 - on field UE and on error CO
    5 - on field FS and on error CO
    6 - on field FS and on error UE
    7 - UE
    8 - UE
  -----------------------------------------------------------------------*/
#define     INVALID     0
#define     LEFT_NO     1
#define     INSIDE_NO   2
#define     RIGHT_NO    3
#define     FULL        4
#define     INSIDE_YES  5
#define     BRACE       6
#define     OUTSIDE_NO  7
#define     OUTSIDE_YES 8

#define ACTION_UNDOEDIT    0
#define ACTION_CONTINUE    1
#define ACTION_SELECTFIELD 2
#define ACTION_EXPAND      3

bool SentenceEditWindow_Impl::KeyInput(const KeyEvent& rKeyEvt)
{
    if (rKeyEvt.GetKeyCode().GetCode() == KEY_TAB)
        return false;

    bool bConsumed = false;

    bool bChange = TextEngine::DoesKeyChangeText( rKeyEvt );
    if (bChange && !IsUndoEditMode())
    {
        bConsumed = true;

        ESelection aCurrentSelection(m_xEditView->GetSelection());
        aCurrentSelection.Adjust();

        //determine if the selection contains a field
        bool bHasFieldLeft = false;
        bool bHasErrorLeft = false;

        bool bHasRange = aCurrentSelection.HasRange();
        sal_uInt8 nSelectionType = 0; // invalid type!

        std::vector<EECharAttrib> aAttribList;
        m_xEditEngine->GetCharAttribs(0, aAttribList);

        auto nCursor = aCurrentSelection.nStartPos;
        const EECharAttrib* pBackAttr = FindCharAttrib(nCursor, EE_CHAR_BKGCOLOR, aAttribList);
        const EECharAttrib* pErrorAttr = FindCharAttrib(nCursor, EE_CHAR_GRABBAG, aAttribList);
        const EECharAttrib* pBackAttrLeft = nullptr;
        const EECharAttrib* pErrorAttrLeft = nullptr;

        bool bHasField = pBackAttr != nullptr && (bHasRange || pBackAttr->nEnd > nCursor);
        bool bHasError = pErrorAttr != nullptr && (bHasRange || pErrorAttr->nEnd > nCursor);
        if (bHasRange)
        {
            if (pBackAttr &&
                    pBackAttr->nStart == aCurrentSelection.nStartPos &&
                    pBackAttr->nEnd == aCurrentSelection.nEndPos)
            {
                nSelectionType = FULL;
            }
            else if (pErrorAttr &&
                     pErrorAttr->nStart <= aCurrentSelection.nStartPos &&
                     pErrorAttr->nEnd >= aCurrentSelection.nEndPos)
            {
                nSelectionType = INSIDE_YES;
            }
            else
            {
                nSelectionType = bHasField||bHasError ? BRACE : OUTSIDE_NO;
                while (nCursor < aCurrentSelection.nEndPos)
                {
                    ++nCursor;
                    const EECharAttrib* pIntBackAttr = FindCharAttrib(nCursor, EE_CHAR_BKGCOLOR, aAttribList);
                    const EECharAttrib* pIntErrorAttr = FindCharAttrib(nCursor, EE_CHAR_GRABBAG, aAttribList);
                    //if any attr has been found then BRACE
                    if (pIntBackAttr || pIntErrorAttr)
                        nSelectionType = BRACE;
                    //the field has to be selected
                    if (pIntBackAttr && !pBackAttr)
                        pBackAttr = pIntBackAttr;
                    bHasField |= pIntBackAttr != nullptr;
                }
            }
        }
        else
        {
            //no range selection: then 1 2 3 and 8 are possible
            const EECharAttrib* pCurAttr = pBackAttr ? pBackAttr : pErrorAttr;
            if (pCurAttr)
            {
                nSelectionType = pCurAttr->nStart == aCurrentSelection.nStartPos ?
                        LEFT_NO : pCurAttr->nEnd == aCurrentSelection.nEndPos ? RIGHT_NO : INSIDE_NO;
            }
            else
                nSelectionType = OUTSIDE_NO;

            bHasFieldLeft = pBackAttr && pBackAttr->nEnd == nCursor;
            if(bHasFieldLeft)
            {
                pBackAttrLeft = pBackAttr;
                pBackAttr = nullptr;
            }
            bHasErrorLeft = pErrorAttr && pErrorAttr->nEnd == nCursor;
            if(bHasErrorLeft)
            {
                pErrorAttrLeft = pErrorAttr;
                pErrorAttr = nullptr;
            }

            //check previous position if this exists
            //that is a redundant in the case the attribute found above already is on the left cursor side
            //but it's o.k. for two errors/fields side by side
            if (nCursor)
            {
                --nCursor;
                pBackAttrLeft = FindCharAttrib(nCursor, EE_CHAR_BKGCOLOR, aAttribList);
                pErrorAttrLeft = FindCharAttrib(nCursor, EE_CHAR_GRABBAG, aAttribList);
                bHasFieldLeft = pBackAttrLeft !=nullptr;
                bHasErrorLeft = pErrorAttrLeft != nullptr;
                ++nCursor;
            }
        }
        //Here we have to determine if the error found is the one currently active
        bool bIsErrorActive = (pErrorAttr && pErrorAttr->nStart == m_nErrorStart) ||
                (pErrorAttrLeft && pErrorAttrLeft->nStart == m_nErrorStart);

        SAL_WARN_IF(
            nSelectionType == INVALID, "cui.dialogs",
            "selection type not set");

        const vcl::KeyCode& rKeyCode = rKeyEvt.GetKeyCode();
        bool bDelete = rKeyCode.GetCode() == KEY_DELETE;
        bool bBackspace = rKeyCode.GetCode() == KEY_BACKSPACE;

        sal_Int8 nAction = ACTION_CONTINUE;
        switch(nSelectionType)
        {
//    1 - backspace                   delete                      any other
//        UE                          on field FS on error CO     on field FS on error CO
            case LEFT_NO    :
                if(bBackspace)
                {
                    nAction = bHasFieldLeft ? ACTION_SELECTFIELD : ACTION_UNDOEDIT;
                    //to force the use of pBackAttrLeft
                    pBackAttr = nullptr;
                }
                else if(bDelete)
                    nAction = bHasField ? ACTION_SELECTFIELD : ACTION_CONTINUE;
                else
                    nAction = bHasError && !nCursor ? ACTION_CONTINUE :
                        bHasError ? ACTION_EXPAND : bHasErrorLeft ? ACTION_CONTINUE : ACTION_UNDOEDIT;
            break;
//    2 - on field FS on error C
            case INSIDE_NO  :
                nAction =  bHasField ? ACTION_SELECTFIELD :
                    bIsErrorActive ? ACTION_CONTINUE : ACTION_UNDOEDIT;
            break;
//    3 - backspace                   delete                      any other
//        on field FS on error CO     UE                          on field UE on error EX
            case RIGHT_NO   :
                if(bBackspace)
                    nAction = bHasFieldLeft ? ACTION_SELECTFIELD : ACTION_CONTINUE;
                else if(bDelete)
                    nAction = bHasFieldLeft && bHasError ? ACTION_CONTINUE : ACTION_UNDOEDIT;
                else
                    nAction = bHasFieldLeft && bHasError ? ACTION_EXPAND :
                        bHasError ? ACTION_CONTINUE : bHasErrorLeft ? ACTION_EXPAND :ACTION_UNDOEDIT;
            break;
//    4 - on field UE and on error CO
            case FULL       :
                nAction = ACTION_UNDOEDIT;
            break;
//    5 - on field FS and on error CO
            case INSIDE_YES :
                nAction = bHasField ? ACTION_SELECTFIELD : ACTION_CONTINUE;
            break;
//    6 - on field FS and on error UE
            case BRACE      :
                nAction = bHasField ? ACTION_SELECTFIELD : ACTION_UNDOEDIT;
            break;
//    7 - UE
//    8 - UE
            case OUTSIDE_NO :
            case OUTSIDE_YES:
                nAction = ACTION_UNDOEDIT;
            break;
        }
        //save the current paragraph
        sal_Int32 nCurrentLen = m_xEditEngine->GetText().getLength();
        if (nAction != ACTION_SELECTFIELD)
        {
            m_xEditView->PostKeyEvent(rKeyEvt);
        }
        else
        {
            const EECharAttrib* pCharAttr = pBackAttr ? pBackAttr : pBackAttrLeft;
            if (pCharAttr)
                m_xEditView->SetSelection(ESelection(0, pCharAttr->nStart, 0, pCharAttr->nEnd));
        }
        if(nAction == ACTION_EXPAND)
        {
            DBG_ASSERT(pErrorAttrLeft || pErrorAttr, "where is the error");
            //text has been added on the right and only the 'error attribute has to be corrected
            if (pErrorAttrLeft)
            {
                SpellErrorDescription aSpellErrorDescription;
                ExtractErrorDescription(*pErrorAttrLeft, aSpellErrorDescription);

                std::unique_ptr<SfxPoolItem> xNewError(pErrorAttrLeft->pAttr->Clone());
                sal_Int32 nStart = pErrorAttrLeft->nStart;
                sal_Int32 nEnd = pErrorAttrLeft->nEnd + 1;
                m_xEditEngine->RemoveAttribs(ESelection(0, nStart, 0, nEnd), false, EE_CHAR_GRABBAG);
                SetAttrib(*xNewError, nStart, nEnd);
                //only active errors move the mark
                if (bIsErrorActive)
                {
                    bool bGrammar = aSpellErrorDescription.bIsGrammarError;
                    MoveErrorMarkTo(nStart, nEnd, bGrammar);
                }
            }
            //text has been added on the left then the error attribute has to be expanded and the
            //field attribute on the right - if any - has to be contracted
            else if (pErrorAttr)
            {
                SpellErrorDescription aSpellErrorDescription;
                ExtractErrorDescription(*pErrorAttr, aSpellErrorDescription);

                //determine the change
                sal_Int32 nAddedChars = m_xEditEngine->GetText().getLength() - nCurrentLen;

                std::unique_ptr<SfxPoolItem> xNewError(pErrorAttr->pAttr->Clone());
                sal_Int32 nStart = pErrorAttr->nStart + nAddedChars;
                sal_Int32 nEnd = pErrorAttr->nEnd + nAddedChars;
                m_xEditEngine->RemoveAttribs(ESelection(0, nStart, 0, nEnd), false, EE_CHAR_GRABBAG);
                nStart = pErrorAttr->nStart;
                SetAttrib(*xNewError, nStart, nEnd);
                //only if the error is active the mark is moved here
                if (bIsErrorActive)
                {
                    bool bGrammar = aSpellErrorDescription.bIsGrammarError;
                    MoveErrorMarkTo(nStart, nEnd, bGrammar);
                }
                xNewError.reset();

                if (pBackAttrLeft)
                {
                    std::unique_ptr<SfxPoolItem> xNewBack(pBackAttrLeft->pAttr->Clone());
                    sal_Int32 _nStart = pBackAttrLeft->nStart + nAddedChars;
                    sal_Int32 _nEnd = pBackAttrLeft->nEnd + nAddedChars;
                    m_xEditEngine->RemoveAttribs(ESelection(0, _nStart, 0, _nEnd), false, EE_CHAR_BKGCOLOR);
                    _nStart = pBackAttrLeft->nStart;
                    SetAttrib(*xNewBack, _nStart, _nEnd);
                }
            }
        }
        else if(nAction == ACTION_UNDOEDIT)
        {
            SetUndoEditMode(true);
        }
        //make sure the error positions are correct after text changes
        //the old attribute may have been deleted
        //all changes inside of the current error leave the error attribute at the current
        //start position
        if (!IsUndoEditMode() && bIsErrorActive)
        {
            aAttribList.clear();
            m_xEditEngine->GetCharAttribs(0, aAttribList);
            const EECharAttrib* pFontColor = FindCharAttrib(nCursor, EE_CHAR_COLOR, aAttribList);
            const EECharAttrib* pErrorAttrib = FindCharAttrib(m_nErrorStart, EE_CHAR_GRABBAG, aAttribList);
            if (pFontColor && pErrorAttrib)
            {
                m_nErrorStart = pFontColor->nStart;
                m_nErrorEnd = pFontColor->nEnd;
                if (pErrorAttrib->nStart != m_nErrorStart || pErrorAttrib->nEnd != m_nErrorEnd)
                {
                    std::unique_ptr<SfxPoolItem> xNewError(pErrorAttrib->pAttr->Clone());
                    assert(pErrorAttr);
                    m_xEditEngine->RemoveAttribs(ESelection(0, pErrorAttr->nStart, 0, pErrorAttr->nEnd), false, EE_CHAR_GRABBAG);
                    SetAttrib(*xNewError, m_nErrorStart, m_nErrorEnd);
                }
            }
        }
        //this is not a modification anymore
        if(nAction != ACTION_SELECTFIELD && !m_bIsUndoEditMode)
            CallModifyLink();
    }
    else
        bConsumed = m_xEditView->PostKeyEvent(rKeyEvt);

    return bConsumed;
}

void SentenceEditWindow_Impl::Init(weld::Toolbar* pToolbar)
{
    m_pToolbar = pToolbar;
    m_pToolbar->connect_clicked(LINK(this,SentenceEditWindow_Impl,ToolbarHdl));
}

IMPL_LINK(SentenceEditWindow_Impl, ToolbarHdl, const OUString&, rCurItemId, void)
{
    if (rCurItemId == "paste")
    {
        m_xEditView->Paste();
        CallModifyLink();
    }
    else if (rCurItemId == "insert")
    {
        if (auto pImplFncGetSpecialChars = vcl::GetGetSpecialCharsFunction())
        {
            OUString aChars = pImplFncGetSpecialChars(GetDrawingArea(), m_xEditEngine->GetStandardFont(0));
            if (!aChars.isEmpty())
            {
                ESelection aCurrentSelection(m_xEditView->GetSelection());
                m_xEditEngine->QuickInsertText(aChars, aCurrentSelection);
                CallModifyLink();
            }
        }
    }
}

bool SentenceEditWindow_Impl::MarkNextError( bool bIgnoreCurrentError, const css::uno::Reference<css::linguistic2::XSpellChecker1>& xSpell )
{
    if (bIgnoreCurrentError)
        m_aIgnoreErrorsAt.insert( m_nErrorStart );

    const sal_Int32 nTextLen = m_xEditEngine->GetTextLen(0);

    if (m_nErrorEnd >= nTextLen - 1)
        return false;
    //if it's not already modified the modified flag has to be reset at the end of the marking
    bool bModified = IsModified();
    bool bRet = false;
    const sal_Int32 nOldErrorStart = m_nErrorStart;
    const sal_Int32 nOldErrorEnd   = m_nErrorEnd;

    //create a cursor behind the end of the last error
    //- or at 0 at the start of the sentence
    sal_Int32 nCursor(m_nErrorEnd ? m_nErrorEnd + 1 : 0);

    //search for SpellErrorDescription
    SpellErrorDescription aSpellErrorDescription;

    std::vector<EECharAttrib> aAttribList;
    m_xEditEngine->GetCharAttribs(0, aAttribList);

    //iterate over the text and search for the next error that maybe has
    //to be replace by a ChangeAllList replacement
    bool bGrammarError = false;
    while (nCursor < nTextLen)
    {
        const SpellErrorDescription* pSpellErrorDescription = nullptr;
        const EECharAttrib* pEECharAttrib = nullptr;

        sal_Int32 nMinPos = nTextLen + 1;
        for (const auto& rTextAtr : aAttribList)
        {
            if (rTextAtr.pAttr->Which() != EE_CHAR_GRABBAG)
                continue;
            if (rTextAtr.nEnd > nCursor && rTextAtr.nStart < nMinPos)
            {
                nMinPos = rTextAtr.nStart;
                pEECharAttrib = &rTextAtr;
            }
        }

        if (pEECharAttrib)
        {
            ExtractErrorDescription(*pEECharAttrib, aSpellErrorDescription);

            bGrammarError = aSpellErrorDescription.bIsGrammarError;
            m_nErrorStart = pEECharAttrib->nStart;
            m_nErrorEnd = pEECharAttrib->nEnd;

            pSpellErrorDescription = &aSpellErrorDescription;
        }

        nCursor = std::max(nCursor, nMinPos); // move forward if possible

        // maybe the error found here is already in the ChangeAllList and has to be replaced
        Reference<XDictionary> xChangeAll = LinguMgr::GetChangeAllList();
        Reference<XDictionaryEntry> xEntry;

        if (xChangeAll->getCount() && pSpellErrorDescription &&
                (xEntry = xChangeAll->getEntry( pSpellErrorDescription->sErrorText )).is())
        {
            OUString sReplacement(getDotReplacementString(GetErrorText(), xEntry->getReplacementText()));

            int nLenChange = ChangeMarkedWord(sReplacement, LanguageTag::convertToLanguageType(pSpellErrorDescription->aLocale));

            nCursor += sReplacement.getLength();

            if (nLenChange)
                m_xEditEngine->GetCharAttribs(0, aAttribList);
            // maybe the error found here is already added to the dictionary and has to be ignored
        }
        else if(pSpellErrorDescription && !bGrammarError &&
                xSpell->isValid(GetErrorText(),
                                static_cast<sal_uInt16>(LanguageTag::convertToLanguageType( pSpellErrorDescription->aLocale )),
                                Sequence< PropertyValue >() ))
        {
            ++nCursor;
        }
        else
            break;
    }

    //if an attrib has been found search for the end of the error string
    if (nCursor < nTextLen)
    {
        MoveErrorMarkTo(nCursor, m_nErrorEnd, bGrammarError);
        bRet = true;
        //add an undo action
        std::unique_ptr<SpellUndoAction_Impl> pAction(new SpellUndoAction_Impl(
                SPELLUNDO_CHANGE_NEXTERROR, GetSpellDialog()->aDialogUndoLink));
        pAction->SetErrorMove(nOldErrorStart, nOldErrorEnd);

        if (GetErrorDescription(aSpellErrorDescription, nOldErrorStart))
        {
            pAction->SetErrorLanguageSelected(aSpellErrorDescription.aSuggestions.hasElements() &&
                LanguageTag(aSpellErrorDescription.aLocale).getLanguageType() == GetSpellDialog()->m_xLanguageLB->get_active_id());
        }
        else
            pAction->SetErrorLanguageSelected(false);

        AddUndoAction(std::move(pAction));
    }
    else
        m_nErrorStart = m_nErrorEnd = nTextLen;
    if( !bModified )
        ClearModifyFlag();
    SpellDialog* pSpellDialog = GetSpellDialog();
    pSpellDialog->m_xIgnorePB->set_sensitive(bRet);
    pSpellDialog->m_xIgnoreAllPB->set_sensitive(bRet);
    pSpellDialog->m_xAutoCorrPB->set_sensitive(bRet);
    pSpellDialog->m_xAddToDictMB->set_sensitive(bRet);
    pSpellDialog->m_xAddToDictPB->set_sensitive(bRet);
    return bRet;
}

void SentenceEditWindow_Impl::MoveErrorMarkTo(sal_Int32 nStart, sal_Int32 nEnd, bool bGrammarError)
{
    ESelection aAll(0, 0, 0, EE_TEXTPOS_ALL);
    m_xEditEngine->RemoveAttribs(aAll, false, EE_CHAR_COLOR);
    m_xEditEngine->RemoveAttribs(aAll, false, EE_CHAR_WEIGHT);
    m_xEditEngine->RemoveAttribs(aAll, false, EE_CHAR_WEIGHT_CJK);
    m_xEditEngine->RemoveAttribs(aAll, false, EE_CHAR_WEIGHT_CTL);

    // tdf#116566 Use color defined in the current Color Scheme
    Color aSpellErrorCollor = svtools::ColorConfig().GetColorValue(svtools::SPELL).nColor;
    Color aGrammarErrorCollor = svtools::ColorConfig().GetColorValue(svtools::GRAMMAR).nColor;

    SfxItemSet aSet(m_xEditEngine->GetEmptyItemSet());
    aSet.Put(SvxColorItem(bGrammarError ? aGrammarErrorCollor : aSpellErrorCollor, EE_CHAR_COLOR));
    aSet.Put(SvxWeightItem(WEIGHT_BOLD, EE_CHAR_WEIGHT));
    aSet.Put(SvxWeightItem(WEIGHT_BOLD, EE_CHAR_WEIGHT_CJK));
    aSet.Put(SvxWeightItem(WEIGHT_BOLD, EE_CHAR_WEIGHT_CTL));

    m_xEditEngine->QuickSetAttribs(aSet, ESelection(0, nStart, 0, nEnd));

    // Set the selection so the editview will autoscroll to make this visible
    // unless (tdf#133958) the selection already overlaps this range
    ESelection aCurrentSelection = m_xEditView->GetSelection();
    aCurrentSelection.Adjust();
    bool bCurrentSelectionInRange = nStart <= aCurrentSelection.nEndPos && aCurrentSelection.nStartPos <= nEnd;
    if (!bCurrentSelectionInRange)
    {
        m_xEditView->SetSelection(ESelection(0, nStart));
        // tdf#157148 ensure current location is auto-scrolled to be visible
        m_xEditView->ShowCursor();
    }

    Invalidate();

    m_nErrorStart = nStart;
    m_nErrorEnd = nEnd;
}

int SentenceEditWindow_Impl::ChangeMarkedWord(const OUString& rNewWord, LanguageType eLanguage)
{
    std::vector<EECharAttrib> aAttribList;
    m_xEditEngine->GetCharAttribs(0, aAttribList);

    //calculate length changes
    auto nDiffLen = rNewWord.getLength() - m_nErrorEnd + m_nErrorStart;
    //Remove spell error attribute
    m_xEditEngine->UndoActionStart(SPELLUNDO_MOVE_ERROREND);
    const EECharAttrib* pErrorAttrib = FindCharAttrib(m_nErrorStart, EE_CHAR_GRABBAG, aAttribList);
    DBG_ASSERT(pErrorAttrib, "no error attribute found");
    bool bSpellErrorDescription = false;
    SpellErrorDescription aSpellErrorDescription;
    if (pErrorAttrib)
    {
        ExtractErrorDescription(*pErrorAttrib, aSpellErrorDescription);
        m_xEditEngine->RemoveAttribs(ESelection(0, pErrorAttrib->nStart, 0, pErrorAttrib->nEnd), false, EE_CHAR_GRABBAG);
        bSpellErrorDescription = true;
    }

    const EECharAttrib* pBackAttrib = FindCharAttrib(m_nErrorStart, EE_CHAR_BKGCOLOR, aAttribList);

    ESelection aSel(0, m_nErrorStart, 0, m_nErrorEnd);
    m_xEditEngine->QuickInsertText(rNewWord, aSel);

    const sal_Int32 nTextLen = m_xEditEngine->GetTextLen(0);

    if (nDiffLen)
        m_xEditEngine->GetCharAttribs(0, aAttribList);

    if (!m_nErrorStart)
    {
        //attributes following an error at the start of the text are not moved but expanded from the
        //text engine - this is done to keep full-paragraph-attributes
        //in the current case that handling is not desired
        const EECharAttrib* pLangAttrib = FindCharAttrib(m_nErrorEnd, EE_CHAR_LANGUAGE, aAttribList);

        if (pLangAttrib && !pLangAttrib->nStart && pLangAttrib->nEnd == nTextLen)
        {
            LanguageType eNewLanguage = static_cast<const SvxLanguageItem*>(pLangAttrib->pAttr)->GetLanguage();
            m_xEditEngine->RemoveAttribs(ESelection(0, pLangAttrib->nStart, 0, pLangAttrib->nEnd), false, EE_CHAR_LANGUAGE);
            SetAttrib(SvxLanguageItem(eNewLanguage, EE_CHAR_LANGUAGE), m_nErrorEnd + nDiffLen, nTextLen);
        }
    }

    // undo expanded attributes!
    if (pBackAttrib && pBackAttrib->nStart < m_nErrorStart && pBackAttrib->nEnd == m_nErrorEnd + nDiffLen)
    {
        std::unique_ptr<SfxPoolItem> xNewBackground(pBackAttrib->pAttr->Clone());
        const sal_Int32 nStart = pBackAttrib->nStart;

        m_xEditEngine->RemoveAttribs(ESelection(0, pBackAttrib->nStart, 0, pBackAttrib->nEnd), false, EE_CHAR_BKGCOLOR);

        SetAttrib(*xNewBackground, nStart, m_nErrorStart);
    }
    m_xEditEngine->SetModified();

    //adjust end position
    tools::Long nEndTemp = m_nErrorEnd;
    nEndTemp += nDiffLen;
    m_nErrorEnd = static_cast<sal_Int32>(nEndTemp);

    std::unique_ptr<SpellUndoAction_Impl> pAction(new SpellUndoAction_Impl(
                    SPELLUNDO_MOVE_ERROREND, GetSpellDialog()->aDialogUndoLink));
    pAction->SetOffset(nDiffLen);
    AddUndoAction(std::move(pAction));
    if (bSpellErrorDescription)
    {
        SfxGrabBagItem aSpellErrorDescriptionItem(EE_CHAR_GRABBAG);
        aSpellErrorDescriptionItem.GetGrabBag()["SpellErrorDescription"] <<= aSpellErrorDescription.toSequence();
        SetAttrib(aSpellErrorDescriptionItem, m_nErrorStart, m_nErrorEnd);
    }
    SetAttrib(SvxLanguageItem(eLanguage, EE_CHAR_LANGUAGE), m_nErrorStart, m_nErrorEnd);
    m_xEditEngine->UndoActionEnd();

    Invalidate();

    return nDiffLen;
}

OUString SentenceEditWindow_Impl::GetErrorText() const
{
    return m_xEditEngine->GetText(ESelection(0, m_nErrorStart, 0, m_nErrorEnd));
}

bool SentenceEditWindow_Impl::GetErrorDescription(SpellErrorDescription& rSpellErrorDescription, sal_Int32 nPosition)
{
    std::vector<EECharAttrib> aAttribList;
    m_xEditEngine->GetCharAttribs(0, aAttribList);

    if (const EECharAttrib* pEECharAttrib = FindCharAttrib(nPosition, EE_CHAR_GRABBAG, aAttribList))
    {
        ExtractErrorDescription(*pEECharAttrib, rSpellErrorDescription);
        return true;
    }

    return false;
}

bool SentenceEditWindow_Impl::GetAlternatives(SpellErrorDescription& rSpellErrorDescription)
{
    return GetErrorDescription(rSpellErrorDescription, m_nErrorStart);
}

void SentenceEditWindow_Impl::RestoreCurrentError()
{
    SpellErrorDescription aSpellErrorDescription;
    if (GetErrorDescription(aSpellErrorDescription, m_nErrorStart))
    {
        if (aSpellErrorDescription.sErrorText != GetErrorText() )
            ChangeMarkedWord(aSpellErrorDescription.sErrorText, LanguageTag::convertToLanguageType(aSpellErrorDescription.aLocale));
    }
}

void SentenceEditWindow_Impl::SetAlternatives( const Reference< XSpellAlternatives>& xAlt )
{
    OUString aWord;
    lang::Locale    aLocale;
    uno::Sequence< OUString >    aAlts;
    if (xAlt.is())
    {
        aWord   = xAlt->getWord();
        aLocale = xAlt->getLocale();
        aAlts   = xAlt->getAlternatives();
    }
    SpellErrorDescription aDesc( false, aWord, std::move(aLocale), aAlts, nullptr);
    SfxGrabBagItem aSpellErrorDescription(EE_CHAR_GRABBAG);
    aSpellErrorDescription.GetGrabBag()["SpellErrorDescription"] <<= aDesc.toSequence();
    SetAttrib(aSpellErrorDescription, m_nErrorStart, m_nErrorEnd);
}

void SentenceEditWindow_Impl::SetAttrib(const SfxPoolItem& rItem, sal_Int32 nStart, sal_Int32 nEnd)
{
    SfxItemSet aSet(m_xEditEngine->GetEmptyItemSet());
    aSet.Put(rItem);
    m_xEditEngine->QuickSetAttribs(aSet, ESelection(0, nStart, 0, nEnd));
    Invalidate();
}

void SentenceEditWindow_Impl::SetText( const OUString& rStr )
{
    m_nErrorStart = m_nErrorEnd = 0;
    m_xEditEngine->SetText(rStr);
}

namespace {

struct LanguagePosition_Impl
{
    sal_Int32       nPosition;
    LanguageType    eLanguage;

    LanguagePosition_Impl(sal_Int32 nPos, LanguageType eLang) :
        nPosition(nPos),
        eLanguage(eLang)
        {}
};

}

typedef std::vector<LanguagePosition_Impl> LanguagePositions_Impl;

static void lcl_InsertBreakPosition_Impl(
        LanguagePositions_Impl& rBreakPositions, sal_Int32 nInsert, LanguageType eLanguage)
{
    LanguagePositions_Impl::iterator aStart = rBreakPositions.begin();
    while(aStart != rBreakPositions.end())
    {
        if(aStart->nPosition == nInsert)
        {
            //the language of following starts has to overwrite
            //the one of previous ends
            aStart->eLanguage = eLanguage;
            return;
        }
        else if(aStart->nPosition > nInsert)
        {

            rBreakPositions.insert(aStart, LanguagePosition_Impl(nInsert, eLanguage));
            return;
        }
        else
            ++aStart;
    }
    rBreakPositions.emplace_back(nInsert, eLanguage);
}

/*-------------------------------------------------------------------------
    Returns the text in spell portions. Each portion contains text with an
    equal language and attribute. The spell alternatives are empty.
  -----------------------------------------------------------------------*/
svx::SpellPortions SentenceEditWindow_Impl::CreateSpellPortions() const
{
    svx::SpellPortions aRet;

    const sal_Int32 nTextLen = m_xEditEngine->GetTextLen(0);

    std::vector<EECharAttrib> aAttribList;
    m_xEditEngine->GetCharAttribs(0, aAttribList);

    if (nTextLen)
    {
        int nCursor(0);
        LanguagePositions_Impl aBreakPositions;
        const EECharAttrib* pLastLang = nullptr;
        const EECharAttrib* pLastError = nullptr;
        LanguageType eLang = LANGUAGE_DONTKNOW;
        const EECharAttrib* pError = nullptr;
        while (nCursor < nTextLen)
        {
            const EECharAttrib* pLang = FindCharAttrib(nCursor, EE_CHAR_LANGUAGE, aAttribList);
            if(pLang && pLang != pLastLang)
            {
                eLang = static_cast<const SvxLanguageItem*>(pLang->pAttr)->GetLanguage();
                lcl_InsertBreakPosition_Impl(aBreakPositions, pLang->nStart, eLang);
                lcl_InsertBreakPosition_Impl(aBreakPositions, pLang->nEnd, eLang);
                pLastLang = pLang;
            }
            pError = FindCharAttrib(nCursor, EE_CHAR_GRABBAG, aAttribList);
            if (pError && pLastError != pError)
            {
                lcl_InsertBreakPosition_Impl(aBreakPositions, pError->nStart, eLang);
                lcl_InsertBreakPosition_Impl(aBreakPositions, pError->nEnd, eLang);
                pLastError = pError;

            }
            ++nCursor;
        }

        if (aBreakPositions.empty())
        {
            //if all content has been overwritten the attributes may have been removed, too
            svx::SpellPortion aPortion1;
            aPortion1.eLanguage = GetSpellDialog()->GetSelectedLang_Impl();

            aPortion1.sText = m_xEditEngine->GetText(ESelection(0, 0, 0, nTextLen));

            aRet.push_back(aPortion1);
        }
        else
        {
            LanguagePositions_Impl::iterator aStart = aBreakPositions.begin();
            //start should always be Null
            eLang = aStart->eLanguage;
            sal_Int32 nStart = aStart->nPosition;
            DBG_ASSERT(!nStart, "invalid start position - language attribute missing?");
            ++aStart;

            while(aStart != aBreakPositions.end())
            {
                svx::SpellPortion aPortion1;
                aPortion1.eLanguage = eLang;

                aPortion1.sText = m_xEditEngine->GetText(ESelection(0, nStart, 0, aStart->nPosition));
                bool bIsIgnoreError = m_aIgnoreErrorsAt.find( nStart ) != m_aIgnoreErrorsAt.end();
                if( bIsIgnoreError )
                {
                    aPortion1.bIgnoreThisError = true;
                }
                aRet.push_back(aPortion1);
                nStart = aStart->nPosition;
                eLang = aStart->eLanguage;
                ++aStart;
            }
        }

        // quick partly fix of #i71318. Correct fix needs to patch the EditEngine itself...
        // this one will only prevent text from disappearing. It may to not have the
        // correct language and will probably not spell checked...
        const sal_uInt32 nPara = m_xEditEngine->GetParagraphCount();
        if (nPara > 1)
        {
            OUStringBuffer aLeftOverText;
            for (sal_uInt32 i = 1; i < nPara; ++i)
            {
                aLeftOverText.append("\x0a");    // the manual line break...
                aLeftOverText.append(m_xEditEngine->GetText(i));
            }
            if (pError)
            {   // we need to add a new portion containing the left-over text
                svx::SpellPortion aPortion2;
                aPortion2.eLanguage = eLang;
                aPortion2.sText = aLeftOverText.makeStringAndClear();
                aRet.push_back( aPortion2 );
            }
            else if (!aLeftOverText.isEmpty() && !aRet.empty())
            {   // we just need to append the left-over text to the last portion (which had no errors)
                aRet[ aRet.size() - 1 ].sText += aLeftOverText;
            }
        }
    }

    return aRet;
}

void SentenceEditWindow_Impl::Undo()
{
    EditUndoManager& rUndoMgr = m_xEditEngine->GetUndoManager();
    DBG_ASSERT(GetUndoActionCount(), "no undo actions available" );
    if(!GetUndoActionCount())
        return;
    bool bSaveUndoEdit = IsUndoEditMode();
    SpellUndoAction_Impl* pUndoAction;
    //if the undo edit mode is active then undo all changes until the UNDO_EDIT_MODE action has been found
    do
    {
        pUndoAction = static_cast<SpellUndoAction_Impl*>(rUndoMgr.GetUndoAction());
        rUndoMgr.Undo();
    }while(bSaveUndoEdit && SPELLUNDO_UNDO_EDIT_MODE != pUndoAction->GetId() && GetUndoActionCount());

    if(bSaveUndoEdit || SPELLUNDO_CHANGE_GROUP == pUndoAction->GetId())
        GetSpellDialog()->UpdateBoxes_Impl();
}

void SentenceEditWindow_Impl::ResetUndo()
{
    EditUndoManager& rUndo = m_xEditEngine->GetUndoManager();
    rUndo.Clear();
}

void SentenceEditWindow_Impl::AddUndoAction( std::unique_ptr<SfxUndoAction> pAction )
{
    EditUndoManager& rUndoMgr = m_xEditEngine->GetUndoManager();
    rUndoMgr.AddUndoAction(std::move(pAction));
    GetSpellDialog()->m_xUndoPB->set_sensitive(true);
}

size_t SentenceEditWindow_Impl::GetUndoActionCount() const
{
    return m_xEditEngine->GetUndoManager().GetUndoActionCount();
}

void SentenceEditWindow_Impl::UndoActionStart( sal_uInt16 nId )
{
    m_xEditEngine->UndoActionStart(nId);
}

void SentenceEditWindow_Impl::UndoActionEnd()
{
    m_xEditEngine->UndoActionEnd();
}

void SentenceEditWindow_Impl::MoveErrorEnd(tools::Long nOffset)
{
    // Shouldn't we always add the real signed value instead???
    if(nOffset > 0)
        m_nErrorEnd = m_nErrorEnd - static_cast<sal_Int32>(nOffset);
    else
        m_nErrorEnd = m_nErrorEnd - static_cast<sal_Int32>(-nOffset);
}


void SentenceEditWindow_Impl::SetUndoEditMode(bool bSet)
{
    DBG_ASSERT(!bSet || m_bIsUndoEditMode != bSet, "SetUndoEditMode with equal values?");
    m_bIsUndoEditMode = bSet;
    //disable all buttons except the Change
    SpellDialog* pSpellDialog = GetSpellDialog();
    weld::Widget* aControls[] =
    {
        pSpellDialog->m_xChangeAllPB.get(),
        pSpellDialog->m_xExplainFT.get(),
        pSpellDialog->m_xIgnoreAllPB.get(),
        pSpellDialog->m_xIgnoreRulePB.get(),
        pSpellDialog->m_xIgnorePB.get(),
        pSpellDialog->m_xSuggestionLB.get(),
        pSpellDialog->m_xSuggestionFT.get(),
        pSpellDialog->m_xLanguageFT.get(),
        pSpellDialog->m_xLanguageLB->get_widget(),
        pSpellDialog->m_xAddToDictMB.get(),
        pSpellDialog->m_xAddToDictPB.get(),
        pSpellDialog->m_xAutoCorrPB.get()
    };
    for (weld::Widget* pWidget : aControls)
        pWidget->set_sensitive(false);

    //remove error marks
    ESelection aAll(0, 0, 0, EE_TEXTPOS_ALL);
    m_xEditEngine->RemoveAttribs(aAll, false, EE_CHAR_COLOR);
    m_xEditEngine->RemoveAttribs(aAll, false, EE_CHAR_WEIGHT);
    m_xEditEngine->RemoveAttribs(aAll, false, EE_CHAR_WEIGHT_CJK);
    m_xEditEngine->RemoveAttribs(aAll, false, EE_CHAR_WEIGHT_CTL);
    Invalidate();

    //put the appropriate action on the Undo-stack
    AddUndoAction( std::make_unique<SpellUndoAction_Impl>(
                        SPELLUNDO_UNDO_EDIT_MODE, GetSpellDialog()->aDialogUndoLink) );
    pSpellDialog->m_xChangePB->set_sensitive(true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
