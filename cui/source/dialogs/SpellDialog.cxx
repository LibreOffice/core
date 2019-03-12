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
#include <vcl/weld.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/menu.hxx>
#include <vcl/layout.hxx>
#include <vcl/scrbar.hxx>
#include <vcl/settings.hxx>
#include "SpellAttrib.hxx"
#include <sfx2/dispatch.hxx>
#include <sfx2/bindings.hxx>
#include <svl/undo.hxx>
#include <unotools/lingucfg.hxx>
#include <vcl/textdata.hxx>
#include <vcl/textview.hxx>
#include <vcl/graphicfilter.hxx>
#include <editeng/unolingu.hxx>
#include <editeng/splwrap.hxx>
#include <linguistic/lngprops.hxx>
#include <linguistic/misc.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XServiceDisplayName.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <sfx2/app.hxx>
#include <vcl/help.hxx>
#include <vcl/graph.hxx>
#include <vcl/builderfactory.hxx>
#include <osl/file.hxx>
#include <editeng/optitems.hxx>
#include <editeng/svxenum.hxx>
#include <svx/SpellDialogChildWindow.hxx>
#include <SpellDialog.hxx>
#include <svx/dlgutil.hxx>
#include <optlingu.hxx>
#include <svx/svxerr.hxx>
#include <treeopt.hxx>
#include <svtools/langtab.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <sal/log.hxx>
#include <i18nlangtag/languagetag.hxx>

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
    long            m_nOldErrorStart;
    long            m_nOldErrorEnd;
    bool            m_bIsErrorLanguageSelected;
    //undo of AddToDictionary
    Reference<XDictionary>  m_xDictionary;
    OUString                m_sAddedWord;
    //move end of error - ::ChangeMarkedWord()
    long            m_nOffset;

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
    bool                    IsEnableChangePB(){return m_bEnableChangePB;}

    void                    SetEnableChangeAllPB(){m_bEnableChangeAllPB = true;}
    bool                    IsEnableChangeAllPB(){return m_bEnableChangeAllPB;}

    void                    SetErrorMove(long nOldStart, long nOldEnd)
                                {
                                        m_nOldErrorStart = nOldStart;
                                        m_nOldErrorEnd = nOldEnd;
                                }
    long                    GetOldErrorStart() { return m_nOldErrorStart;}
    long                    GetOldErrorEnd() { return m_nOldErrorEnd;}

    void                    SetErrorLanguageSelected(bool bSet){ m_bIsErrorLanguageSelected = bSet;}
    bool                    IsErrorLanguageSelected() const {return m_bIsErrorLanguageSelected;}

    void                    SetDictionary(const Reference<XDictionary>& xDict) { m_xDictionary = xDict; }
    const Reference<XDictionary>& GetDictionary() const { return m_xDictionary; }
    void                    SetAddedWord(const OUString& rWord) {m_sAddedWord = rWord;}
    const OUString&         GetAddedWord() const { return m_sAddedWord;}

    void                    SetOffset(long nSet) {m_nOffset = nSet;}
    long                    GetOffset() const {return m_nOffset;}
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
    vcl::Window * pParent, SfxBindings* _pBindings)
    : SfxModelessDialog (_pBindings, pChildWindow,
        pParent, "SpellingDialog", "cui/ui/spellingdialog.ui")
    , aDialogUndoLink(LINK (this, SpellDialog, DialogUndoHdl))
    , bFocusLocked(true)
    , rParent(*pChildWindow)
    , pImpl( new SpellDialog_Impl )
{
    m_sTitleSpellingGrammar = GetText();
    m_sTitleSpelling = get<FixedText>("alttitleft")->GetText();

    // fdo#68794 set initial title for cases where no text has been processed
    // yet to show its language attributes
    OUString sTitle = rParent.HasGrammarChecking() ? m_sTitleSpellingGrammar : m_sTitleSpelling;
    SetText(sTitle.replaceFirst("$LANGUAGE ($LOCATION)", ""));

    m_sResumeST = get<FixedText>("resumeft")->GetText();
    m_sNoSuggestionsST = get<FixedText>("nosuggestionsft")->GetText();

    get(m_pLanguageFT, "languageft");
    get(m_pLanguageLB, "languagelb");
    get(m_pExplainFT, "explain");
    get(m_pExplainLink, "explainlink");
    get(m_pNotInDictFT, "notindictft");
    get(m_pSentenceED, "sentence");
    Size aEdSize(LogicToPixel(Size(197, 48), MapMode(MapUnit::MapAppFont)));
    m_pSentenceED->set_width_request(aEdSize.Width());
    m_pSentenceED->set_height_request(aEdSize.Height());
    get(m_pSuggestionFT, "suggestionsft");
    get(m_pSuggestionLB, "suggestionslb");
    m_pSuggestionLB->SetDropDownLineCount(5);
    m_pSuggestionLB->set_width_request(aEdSize.Width());
    get(m_pIgnorePB, "ignore");
    m_sIgnoreOnceST = m_pIgnorePB->GetText();
    get(m_pIgnoreAllPB, "ignoreall");
    get(m_pIgnoreRulePB, "ignorerule");
    get(m_pAddToDictPB, "add");
    get(m_pAddToDictMB, "addmb");
    m_pAddToDictMB->SetHelpId(m_pAddToDictPB->GetHelpId());
    get(m_pChangePB, "change");
    get(m_pChangeAllPB, "changeall");
    get(m_pAutoCorrPB, "autocorrect");
    get(m_pCheckGrammarCB, "checkgrammar");
    get(m_pOptionsPB, "options");
    get(m_pUndoPB, "undo");
    get(m_pClosePB, "close");
    get(m_pToolbar, "toolbar");
    m_pSentenceED->Init(m_pToolbar);
    xSpell = LinguMgr::GetSpellChecker();

    const StyleSettings& rSettings = GetSettings().GetStyleSettings();
    Color aCol = rSettings.GetHelpColor();
    Wallpaper aWall( aCol );
    m_pExplainLink->SetBackground( aWall );
    m_pExplainFT->SetBackground( aWall );

    Init_Impl();

    // disable controls if service is missing
    Enable(xSpell.is());

    //InitHdl wants to use virtual methods, so it
    //can't be called during the ctor, so init
    //it on next event cycle post-ctor
    Application::PostUserEvent(
        LINK( this, SpellDialog, InitHdl ), nullptr, true );
}

SpellDialog::~SpellDialog()
{
    disposeOnce();
}

void SpellDialog::dispose()
{
    if (pImpl)
    {
        // save possibly modified user-dictionaries
        Reference< XSearchableDictionaryList >  xDicList( LinguMgr::GetDictionaryList() );
        if (xDicList.is())
            SaveDictionaries( xDicList );

        pImpl.reset();
    }
    m_pLanguageFT.clear();
    m_pLanguageLB.clear();
    m_pExplainFT.clear();
    m_pExplainLink.clear();
    m_pNotInDictFT.clear();
    m_pSentenceED.clear();
    m_pSuggestionFT.clear();
    m_pSuggestionLB.clear();
    m_pIgnorePB.clear();
    m_pIgnoreAllPB.clear();
    m_pIgnoreRulePB.clear();
    m_pAddToDictPB.clear();
    m_pAddToDictMB.clear();
    m_pChangePB.clear();
    m_pChangeAllPB.clear();
    m_pAutoCorrPB.clear();
    m_pCheckGrammarCB.clear();
    m_pOptionsPB.clear();
    m_pUndoPB.clear();
    m_pClosePB.clear();
    m_pToolbar.clear();
    SfxModelessDialog::dispose();
}

void SpellDialog::Init_Impl()
{
    // initialize handler
    m_pClosePB->SetClickHdl(LINK( this, SpellDialog, CancelHdl ) );
    m_pChangePB->SetClickHdl(LINK( this, SpellDialog, ChangeHdl ) );
    m_pChangeAllPB->SetClickHdl(LINK( this, SpellDialog, ChangeAllHdl ) );
    m_pIgnorePB->SetClickHdl(LINK( this, SpellDialog, IgnoreHdl ) );
    m_pIgnoreAllPB->SetClickHdl(LINK( this, SpellDialog, IgnoreAllHdl ) );
    m_pIgnoreRulePB->SetClickHdl(LINK( this, SpellDialog, IgnoreAllHdl ) );
    m_pUndoPB->SetClickHdl(LINK( this, SpellDialog, UndoHdl ) );

    m_pAutoCorrPB->SetClickHdl( LINK( this, SpellDialog, ExtClickHdl ) );
    m_pCheckGrammarCB->SetClickHdl( LINK( this, SpellDialog, CheckGrammarHdl ));
    m_pOptionsPB->SetClickHdl( LINK( this, SpellDialog, ExtClickHdl ) );

    m_pSuggestionLB->SetDoubleClickHdl( LINK( this, SpellDialog, DoubleClickChangeHdl ) );

    m_pSentenceED->SetModifyHdl(LINK ( this, SpellDialog, ModifyHdl) );

    m_pAddToDictMB->SetSelectHdl(LINK ( this, SpellDialog, AddToDictSelectHdl ) );
    m_pAddToDictPB->SetClickHdl(LINK ( this, SpellDialog, AddToDictClickHdl ) );

    m_pLanguageLB->SetSelectHdl(LINK( this, SpellDialog, LanguageSelectHdl ) );

    // initialize language ListBox
    m_pLanguageLB->SetLanguageList( SvxLanguageListFlags::SPELL_USED, false, true );

    m_pSentenceED->ClearModifyFlag();
    LinguMgr::GetChangeAllList()->clear();
}

void SpellDialog::UpdateBoxes_Impl(bool bCallFromSelectHdl)
{
    sal_Int32 i;
    m_pSuggestionLB->Clear();

    const SpellErrorDescription* pSpellErrorDescription = m_pSentenceED->GetAlternatives();

    LanguageType nAltLanguage = LANGUAGE_NONE;
    Sequence< OUString > aNewWords;
    bool bIsGrammarError = false;
    if( pSpellErrorDescription )
    {
        nAltLanguage    = LanguageTag::convertToLanguageType( pSpellErrorDescription->aLocale );
        aNewWords       = pSpellErrorDescription->aSuggestions;
        bIsGrammarError = pSpellErrorDescription->bIsGrammarError;
        m_pExplainLink->SetURL( pSpellErrorDescription->sExplanationURL );
        m_pExplainFT->SetText( pSpellErrorDescription->sExplanation );
    }
    if( pSpellErrorDescription && !pSpellErrorDescription->sDialogTitle.isEmpty() )
    {
        // use this function to apply the correct image to be used...
        SetTitle_Impl( nAltLanguage );
        // then change the title to the one to be actually used
        SetText( pSpellErrorDescription->sDialogTitle );
    }
    else
        SetTitle_Impl( nAltLanguage );
    if( !bCallFromSelectHdl )
        m_pLanguageLB->SelectLanguage( nAltLanguage );
    int nDicts = InitUserDicts();

    // enter alternatives
    const OUString *pNewWords = aNewWords.getConstArray();
    const sal_Int32 nSize = aNewWords.getLength();
    for ( i = 0; i < nSize; ++i )
    {
        OUString aTmp( pNewWords[i] );
        if ( LISTBOX_ENTRY_NOTFOUND == m_pSuggestionLB->GetEntryPos( aTmp ) )
        {
            m_pSuggestionLB->InsertEntry( aTmp );
            m_pSuggestionLB->SetEntryFlags(m_pSuggestionLB->GetEntryCount() - 1, ListBoxEntryFlags::MultiLine);
        }
    }
    if(!nSize)
        m_pSuggestionLB->InsertEntry(m_sNoSuggestionsST);
    m_pAutoCorrPB->Enable( nSize > 0 );

    m_pSuggestionFT->Enable(nSize > 0);
    m_pSuggestionLB->Enable(nSize > 0);
    if( nSize )
    {
        m_pSuggestionLB->SelectEntryPos(0);
    }
    m_pChangePB->Enable( nSize > 0);
    m_pChangeAllPB->Enable(nSize > 0);
    bool bShowChangeAll = !bIsGrammarError;
    m_pChangeAllPB->Show( bShowChangeAll );
    m_pExplainFT->Show( !bShowChangeAll );
    m_pLanguageLB->Enable( bShowChangeAll );
    m_pIgnoreAllPB->Show( bShowChangeAll );

    m_pAddToDictMB->Show( bShowChangeAll && nDicts > 1);
    m_pAddToDictPB->Show( bShowChangeAll && nDicts <= 1);
    m_pIgnoreRulePB->Show( !bShowChangeAll );
    m_pIgnoreRulePB->Enable(pSpellErrorDescription && !pSpellErrorDescription->sRuleId.isEmpty());
    m_pAutoCorrPB->Show( bShowChangeAll && rParent.HasAutoCorrection() );

    bool bOldShowGrammar = m_pCheckGrammarCB->IsVisible();
    bool bOldShowExplain = m_pExplainLink->IsVisible();

    m_pCheckGrammarCB->Show(rParent.HasGrammarChecking());
    m_pExplainLink->Show(!m_pExplainLink->GetURL().isEmpty());
    if (m_pExplainFT->GetText().isEmpty())
    {
        m_pExplainFT->Hide();
        m_pExplainLink->Hide();
    }

    if (bOldShowExplain != m_pExplainLink->IsVisible() || bOldShowGrammar != m_pCheckGrammarCB->IsVisible())
        setOptimalLayoutSize();
}


void SpellDialog::SpellContinue_Impl(bool bUseSavedSentence, bool bIgnoreCurrentError )
{
    //initially or after the last error of a sentence MarkNextError will fail
    //then GetNextSentence() has to be called followed again by MarkNextError()
    //MarkNextError is not initially called if the UndoEdit mode is active
    bool bNextSentence = false;
    if((!m_pSentenceED->IsUndoEditMode() && m_pSentenceED->MarkNextError( bIgnoreCurrentError, xSpell )) ||
            ( bNextSentence = GetNextSentence_Impl(bUseSavedSentence, m_pSentenceED->IsUndoEditMode()) && m_pSentenceED->MarkNextError( false, xSpell )))
    {
        const SpellErrorDescription* pSpellErrorDescription = m_pSentenceED->GetAlternatives();
        if( pSpellErrorDescription )
        {
            UpdateBoxes_Impl();
            Control* aControls[] =
            {
                m_pNotInDictFT,
                m_pSentenceED,
                m_pLanguageFT,
                nullptr
            };
            sal_Int32 nIdx = 0;
            do
            {
                aControls[nIdx]->Enable();
            }
            while(aControls[++nIdx]);

        }
        if( bNextSentence )
        {
            //remove undo if a new sentence is active
            m_pSentenceED->ResetUndo();
            m_pUndoPB->Enable(false);
        }
    }
}
/* Initialize, asynchronous to prevent virtual calls
   from a constructor
 */
IMPL_LINK_NOARG( SpellDialog, InitHdl, void*, void)
{
    SetUpdateMode( false );
    //show or hide AutoCorrect depending on the modules abilities
    m_pAutoCorrPB->Show(rParent.HasAutoCorrection());
    SpellContinue_Impl();
    m_pSentenceED->ResetUndo();
    m_pUndoPB->Enable(false);

    // get current language
    UpdateBoxes_Impl();

    // fill dictionary PopupMenu
    InitUserDicts();

    LockFocusChanges(true);
    if( m_pChangePB->IsEnabled() )
        m_pChangePB->GrabFocus();
    else if( m_pIgnorePB->IsEnabled() )
        m_pIgnorePB->GrabFocus();
    else if( m_pClosePB->IsEnabled() )
        m_pClosePB->GrabFocus();
    LockFocusChanges(false);
    //show grammar CheckBox depending on the modules abilities
    m_pCheckGrammarCB->Check( rParent.IsGrammarChecking() );
    SetUpdateMode( true );
    Show();
};


IMPL_LINK( SpellDialog, ExtClickHdl, Button *, pBtn, void )
{
    if (m_pOptionsPB == pBtn)
        StartSpellOptDlg_Impl();
    else if (m_pAutoCorrPB == pBtn)
    {
        //get the currently selected wrong word
        OUString sCurrentErrorText = m_pSentenceED->GetErrorText();
        //get the wrong word from the XSpellAlternative
        const SpellErrorDescription* pSpellErrorDescription = m_pSentenceED->GetAlternatives();
        if( pSpellErrorDescription )
        {
            OUString sWrong(pSpellErrorDescription->sErrorText);
            //if the word has not been edited in the MultiLineEdit then
            //the current suggestion should be used
            //if it's not the 'no suggestions' entry
            if(sWrong == sCurrentErrorText &&
                    m_pSuggestionLB->IsEnabled() && m_pSuggestionLB->GetSelectedEntryCount() > 0 &&
                    m_sNoSuggestionsST != m_pSuggestionLB->GetSelectedEntry())
            {
                sCurrentErrorText = m_pSuggestionLB->GetSelectedEntry();
            }
            if(sWrong != sCurrentErrorText)
            {
                SvxPrepareAutoCorrect( sWrong, sCurrentErrorText );
                LanguageType eLang = GetSelectedLang_Impl();
                rParent.AddAutoCorrection( sWrong, sCurrentErrorText, eLang );
            }
        }
    }
}

IMPL_LINK( SpellDialog, CheckGrammarHdl, Button*, pBox, void )
{
    rParent.SetGrammarChecking( static_cast<CheckBox*>(pBox)->IsChecked() );
    Impl_Restore(true);
}

void SpellDialog::StartSpellOptDlg_Impl()
{
    SfxItemSet aSet( SfxGetpApp()->GetPool(), svl::Items<SID_AUTOSPELL_CHECK,SID_AUTOSPELL_CHECK>{});
    SfxSingleTabDialogController aDlg(GetFrameWeld(), aSet, "cui/ui/spelloptionsdialog.ui", "SpellOptionsDialog");

    TabPageParent aParent(aDlg.get_content_area(), &aDlg);
    VclPtr<SfxTabPage> xPage = SvxLinguTabPage::Create(aParent, &aSet);
    static_cast<SvxLinguTabPage*>(xPage.get())->HideGroups( GROUP_MODULES );
    aDlg.SetTabPage(xPage);
    if (RET_OK == aDlg.run())
    {
        InitUserDicts();
        const SfxItemSet* pOutSet = aDlg.GetOutputItemSet();
        if(pOutSet)
            OfaTreeOptionsDialog::ApplyLanguageOptions(*pOutSet);
    }
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
    OUString sOrigString = m_pSentenceED->GetErrorText();

    OUString sReplacement(sOrigString);

    if(m_pSuggestionLB->IsEnabled() &&
            m_pSuggestionLB->GetSelectedEntryCount()>0 &&
            m_sNoSuggestionsST != m_pSuggestionLB->GetSelectedEntry())
        sReplacement = m_pSuggestionLB->GetSelectedEntry();

    return getDotReplacementString(sOrigString, sReplacement);
}


IMPL_LINK_NOARG(SpellDialog, DoubleClickChangeHdl, ListBox&, void)
{
    ChangeHdl(nullptr);
}

IMPL_LINK_NOARG(SpellDialog, ChangeHdl, Button*, void)
{
    if(m_pSentenceED->IsUndoEditMode())
    {
        SpellContinue_Impl();
    }
    else
    {
        m_pSentenceED->UndoActionStart( SPELLUNDO_CHANGE_GROUP );
        OUString aString = getReplacementString();
        m_pSentenceED->ChangeMarkedWord(aString, GetSelectedLang_Impl());
        SpellContinue_Impl();
        m_pSentenceED->UndoActionEnd();
    }
    if(!m_pChangePB->IsEnabled())
        m_pIgnorePB->GrabFocus();
}


IMPL_LINK_NOARG(SpellDialog, ChangeAllHdl, Button*, void)
{
    m_pSentenceED->UndoActionStart( SPELLUNDO_CHANGE_GROUP );
    OUString aString = getReplacementString();
    LanguageType eLang = GetSelectedLang_Impl();

    // add new word to ChangeAll list
    OUString  aOldWord( m_pSentenceED->GetErrorText() );
    SvxPrepareAutoCorrect( aOldWord, aString );
    Reference<XDictionary> aXDictionary( LinguMgr::GetChangeAllList(), UNO_QUERY );
    DictionaryError nAdded = AddEntryToDic( aXDictionary,
            aOldWord, true,
            aString );

    if(nAdded == DictionaryError::NONE)
    {
        std::unique_ptr<SpellUndoAction_Impl> pAction(new SpellUndoAction_Impl(
                        SPELLUNDO_CHANGE_ADD_TO_DICTIONARY, aDialogUndoLink));
        pAction->SetDictionary(aXDictionary);
        pAction->SetAddedWord(aOldWord);
        m_pSentenceED->AddUndoAction(std::move(pAction));
    }

    m_pSentenceED->ChangeMarkedWord(aString, eLang);
    SpellContinue_Impl();
    m_pSentenceED->UndoActionEnd();
}


IMPL_LINK( SpellDialog, IgnoreAllHdl, Button *, pButton, void )
{
    m_pSentenceED->UndoActionStart( SPELLUNDO_CHANGE_GROUP );
    // add word to IgnoreAll list
    Reference< XDictionary > aXDictionary( LinguMgr::GetIgnoreAllList(), UNO_QUERY );
    //in case the error has been changed manually it has to be restored
    m_pSentenceED->RestoreCurrentError();
    if (pButton == m_pIgnoreRulePB)
    {
        const SpellErrorDescription* pSpellErrorDescription = m_pSentenceED->GetAlternatives();
        try
        {
            if( pSpellErrorDescription && pSpellErrorDescription->xGrammarChecker.is() )
            {
                pSpellErrorDescription->xGrammarChecker->ignoreRule( pSpellErrorDescription->sRuleId,
                    pSpellErrorDescription->aLocale );
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
        OUString sErrorText(m_pSentenceED->GetErrorText());
        DictionaryError nAdded = AddEntryToDic( aXDictionary,
            sErrorText, false,
            OUString() );
        if(nAdded == DictionaryError::NONE)
        {
            std::unique_ptr<SpellUndoAction_Impl> pAction(new SpellUndoAction_Impl(
                            SPELLUNDO_CHANGE_ADD_TO_DICTIONARY, aDialogUndoLink));
            pAction->SetDictionary(aXDictionary);
            pAction->SetAddedWord(sErrorText);
            m_pSentenceED->AddUndoAction(std::move(pAction));
        }
    }

    SpellContinue_Impl();
    m_pSentenceED->UndoActionEnd();
}


IMPL_LINK_NOARG(SpellDialog, UndoHdl, Button*, void)
{
    m_pSentenceED->Undo();
    if(!m_pSentenceED->GetUndoActionCount())
        m_pUndoPB->Enable(false);
}


IMPL_LINK( SpellDialog, DialogUndoHdl, SpellUndoAction_Impl&, rAction, void )
{
    switch(rAction.GetId())
    {
        case SPELLUNDO_CHANGE_TEXTENGINE:
        {
            if(rAction.IsEnableChangePB())
                m_pChangePB->Enable(false);
            if(rAction.IsEnableChangeAllPB())
                m_pChangeAllPB->Enable(false);
        }
        break;
        case SPELLUNDO_CHANGE_NEXTERROR:
        {
            m_pSentenceED->MoveErrorMarkTo(static_cast<sal_Int32>(rAction.GetOldErrorStart()),
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
                m_pSentenceED->MoveErrorEnd(rAction.GetOffset());
        }
        break;
        case SPELLUNDO_UNDO_EDIT_MODE :
        {
            //refill the dialog with the currently spelled sentence - throw away all changes
            SpellContinue_Impl(true);
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
    m_pSentenceED->SetText(OUString());
    m_pSentenceED->ResetModified();
    //Resolves: fdo#39348 refill the dialog with the currently spelled sentence
    SpellContinue_Impl(bUseSavedSentence);
    m_pIgnorePB->SetText(m_sIgnoreOnceST);
}

IMPL_LINK_NOARG(SpellDialog, IgnoreHdl, Button*, void)
{
    if (m_sResumeST == m_pIgnorePB->GetText())
    {
        Impl_Restore(false);
    }
    else
    {
        //in case the error has been changed manually it has to be restored,
        // since the users choice now was to ignore the error
        m_pSentenceED->RestoreCurrentError();

        // the word is being ignored
        SpellContinue_Impl( false, true );
    }
}


bool SpellDialog::Close()
{
    // We have to call ToggleChildWindow directly; calling SfxDispatcher's
    // Execute() does not work here when we are in a document with protected
    // section - in that case, the cursor can move from the editable field to
    // the protected area, and the slots get disabled because of
    // SfxDisableFlags::SwOnProtectedCursor (see FN_SPELL_GRAMMAR_DIALOG in .sdi).
    SfxViewFrame::Current()->ToggleChildWindow(rParent.GetType());

    return true;
}


LanguageType SpellDialog::GetSelectedLang_Impl() const
{
    LanguageType nLang = m_pLanguageLB->GetSelectedLanguage();
    return nLang;
}


IMPL_LINK(SpellDialog, LanguageSelectHdl, ListBox&, rBox, void)
{
    //If selected language changes, then add->list should be regenerated to
    //match
    InitUserDicts();

    //if currently an error is selected then search for alternatives for
    //this word and fill the alternatives ListBox accordingly
    OUString sError = m_pSentenceED->GetErrorText();
    m_pSuggestionLB->Clear();
    if(!sError.isEmpty())
    {
        LanguageType eLanguage = static_cast<SvxLanguageBox*>(&rBox)->GetSelectedLanguage();
        Reference <XSpellAlternatives> xAlt = xSpell->spell( sError, static_cast<sal_uInt16>(eLanguage),
                                            Sequence< PropertyValue >() );
        if( xAlt.is() )
            m_pSentenceED->SetAlternatives( xAlt );
        else
        {
            m_pSentenceED->ChangeMarkedWord( sError, eLanguage );
            SpellContinue_Impl();
        }

        m_pSentenceED->AddUndoAction(std::make_unique<SpellUndoAction_Impl>(SPELLUNDO_CHANGE_LANGUAGE, aDialogUndoLink));
    }
    SpellDialog::UpdateBoxes_Impl(true);
}


void SpellDialog::SetTitle_Impl(LanguageType nLang)
{
    OUString sTitle = rParent.HasGrammarChecking() ? m_sTitleSpellingGrammar : m_sTitleSpelling;
    sTitle = sTitle.replaceFirst( "$LANGUAGE ($LOCATION)", SvtLanguageTable::GetLanguageString(nLang) );
    SetText( sTitle );
}

int SpellDialog::InitUserDicts()
{
    const LanguageType nLang = m_pLanguageLB->GetSelectedLanguage();

    const Reference< XDictionary >  *pDic = nullptr;

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
    const sal_Int32 nSize = pImpl->aDics.getLength();
    pDic = pImpl->aDics.getConstArray();
    PopupMenu* pMenu = m_pAddToDictMB->GetPopupMenu();
    assert(pMenu);
    pMenu->Clear();
    pMenu->SetMenuFlags(MenuFlags::NoAutoMnemonics);
    sal_uInt16 nItemId = 1;     // menu items should be enumerated from 1 and not 0
    for (sal_Int32 i = 0; i < nSize; ++i)
    {
        uno::Reference< linguistic2::XDictionary >  xDicTmp( pDic[i], uno::UNO_QUERY );
        if (!xDicTmp.is() || LinguMgr::GetIgnoreAllList() == xDicTmp)
            continue;

        uno::Reference< frame::XStorable > xStor( xDicTmp, uno::UNO_QUERY );
        LanguageType nActLanguage = LanguageTag( xDicTmp->getLocale() ).getLanguageType();
        if( xDicTmp->isActive()
            &&  xDicTmp->getDictionaryType() != linguistic2::DictionaryType_NEGATIVE
            && (nLang == nActLanguage || LANGUAGE_NONE == nActLanguage )
            && (!xStor.is() || !xStor->isReadonly()) )
        {
            pMenu->InsertItem( nItemId, xDicTmp->getName() );
            bEnable = true;

            uno::Reference< lang::XServiceInfo > xSvcInfo( xDicTmp, uno::UNO_QUERY );
            if (xSvcInfo.is())
            {
                OUString aDictionaryImageUrl( aCfg.GetSpellAndGrammarContextDictionaryImage(
                        xSvcInfo->getImplementationName()) );
                if (!aDictionaryImageUrl.isEmpty())
                {
                    Image aImage( aDictionaryImageUrl );
                    pMenu->SetItemImage( nItemId, aImage );
                }
            }

            ++nItemId;
        }
    }
    m_pAddToDictMB->Enable( bEnable );
    m_pAddToDictPB->Enable( bEnable );

    int nDicts = nItemId-1;

    m_pAddToDictMB->Show( nDicts > 1 );
    m_pAddToDictPB->Show( nDicts <= 1 );

    return nDicts;
}


IMPL_LINK_NOARG(SpellDialog, AddToDictClickHdl, Button*, void)
{
    AddToDictionaryExecute(1, m_pAddToDictMB->GetPopupMenu());
}


IMPL_LINK(SpellDialog, AddToDictSelectHdl, MenuButton*, pButton, void )
{
    AddToDictionaryExecute(pButton->GetCurItemId(), pButton->GetPopupMenu());
}


void SpellDialog::AddToDictionaryExecute( sal_uInt16 nItemId, PopupMenu const *pMenu )
{
    m_pSentenceED->UndoActionStart( SPELLUNDO_CHANGE_GROUP );

    //GetErrorText() returns the current error even if the text is already
    //manually changed
    const OUString aNewWord = m_pSentenceED->GetErrorText();

    OUString aDicName ( pMenu->GetItemText( nItemId ) );

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
            m_pSentenceED->AddUndoAction( std::move(pAction) );
        }
        // failed because there is already an entry?
        if (DictionaryError::NONE != nAddRes && xDic->getEntry( aNewWord ).is())
            nAddRes = DictionaryError::NONE;
    }
    if (DictionaryError::NONE != nAddRes)
    {
        SvxDicError(GetFrameWeld(), nAddRes);
        return; // don't continue
    }

    // go on
    SpellContinue_Impl();
    m_pSentenceED->UndoActionEnd();
}


IMPL_LINK(SpellDialog, ModifyHdl, Edit&, rEd, void)
{
    if (m_pSentenceED == &rEd)
    {
        m_pSuggestionLB->SetNoSelection();
        m_pSuggestionLB->Disable();
        OUString sNewText( m_pSentenceED->GetText() );
        m_pAutoCorrPB->Enable( sNewText != m_pSentenceED->GetText() );
        std::unique_ptr<SpellUndoAction_Impl> pSpellAction(new SpellUndoAction_Impl(SPELLUNDO_CHANGE_TEXTENGINE, aDialogUndoLink));
        if(!m_pChangeAllPB->IsEnabled())
        {
            m_pChangeAllPB->Enable();
            pSpellAction->SetEnableChangeAllPB();
        }
        if(!m_pChangePB->IsEnabled())
        {
            m_pChangePB->Enable();
            pSpellAction->SetEnableChangePB();
        }
        m_pSentenceED->AddUndoAction(std::move(pSpellAction));
    }
};


IMPL_LINK_NOARG(SpellDialog, CancelHdl, Button*, void)
{
    //apply changes and ignored text parts first - if there are any
    rParent.ApplyChangedSentence(m_pSentenceED->CreateSpellPortions(), false);
    Close();
}


bool SpellDialog::EventNotify( NotifyEvent& rNEvt )
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
    if( IsVisible() && !bFocusLocked )
    {
        if( rNEvt.GetType() ==  MouseNotifyEvent::GETFOCUS )
        {
            //notify the child window of the focus change
            rParent.GetFocus();
        }
        else if( rNEvt.GetType() == MouseNotifyEvent::LOSEFOCUS )
        {
            //notify the child window of the focus change
            rParent.LoseFocus();
        }
    }
    return SfxModelessDialog::EventNotify(rNEvt);
}


void SpellDialog::InvalidateDialog()
{
    if( bFocusLocked )
        return;
    m_pIgnorePB->SetText(m_sResumeST);
    vcl::Window* aDisableArr[] =
    {
        m_pNotInDictFT,
        m_pSentenceED,
        m_pSuggestionFT,
        m_pSuggestionLB,
        m_pLanguageFT,
        m_pLanguageLB,
        m_pIgnoreAllPB,
        m_pIgnoreRulePB,
        m_pAddToDictMB,
        m_pAddToDictPB,
        m_pChangePB,
        m_pChangeAllPB,
        m_pAutoCorrPB,
        m_pUndoPB,
        nullptr
    };
    sal_Int16 i = 0;
    while(aDisableArr[i])
    {
        aDisableArr[i]->Enable(false);
        i++;
    }
    SfxModelessDialog::Deactivate();
}


bool SpellDialog::GetNextSentence_Impl(bool bUseSavedSentence, bool bRecheck)
{
    bool bRet = false;
    if(!bUseSavedSentence)
    {
        //apply changes and ignored text parts
        rParent.ApplyChangedSentence(m_pSentenceED->CreateSpellPortions(), bRecheck);
    }
    m_pSentenceED->ResetIgnoreErrorsAt();
    m_pSentenceED->ResetModified();
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
        m_pSentenceED->SetText(sText.makeStringAndClear());
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
                    uno::Reference< container::XNamed > xNamed( elem.xAlternatives, uno::UNO_QUERY );
                    OUString sServiceName;
                    if( xNamed.is() )
                        sServiceName = xNamed->getName();
                    SpellErrorDescription aDesc( false, elem.xAlternatives->getWord(),
                                    elem.xAlternatives->getLocale(), elem.xAlternatives->getAlternatives(), nullptr);
                    m_pSentenceED->SetAttrib( SpellErrorAttrib(aDesc), 0, nStartPosition, nEndPosition );
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
                    m_pSentenceED->SetAttrib( SpellErrorAttrib(aDesc), 0, nStartPosition, nEndPosition );
                }
                if(elem.bIsField)
                    m_pSentenceED->SetAttrib( SpellBackgroundAttrib(COL_LIGHTGRAY), 0, nStartPosition, nEndPosition );
                m_pSentenceED->SetAttrib( SpellLanguageAttrib(elem.eLanguage), 0, nStartPosition, nEndPosition );
                nStartPosition = nEndPosition;
            }
        }
        //the edit field needs to be modified to apply the change from the ApplyChangeAllList
        if(!bHasReplaced)
            m_pSentenceED->ClearModifyFlag();
        m_pSentenceED->ResetUndo();
        m_pUndoPB->Enable(false);
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
    Reference<XDictionary> xChangeAll( LinguMgr::GetChangeAllList(), UNO_QUERY );
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


SentenceEditWindow_Impl::SentenceEditWindow_Impl(vcl::Window * pParent, WinBits nBits)
    : VclMultiLineEdit(pParent, nBits)
    , m_nErrorStart(0)
    , m_nErrorEnd(0)
    , m_bIsUndoEditMode(false)
{
    DisableSelectionOnFocus();
}

SentenceEditWindow_Impl::~SentenceEditWindow_Impl()
{
    disposeOnce();
}

extern "C" SAL_DLLPUBLIC_EXPORT void makeSentenceEditWindow(VclPtr<vcl::Window> & rRet, VclPtr<vcl::Window> & pParent, VclBuilder::stringmap &)
{
    rRet = VclPtr<SentenceEditWindow_Impl>::Create(pParent, WB_BORDER|WB_VSCROLL|WB_IGNORETAB);
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

bool SentenceEditWindow_Impl::PreNotify( NotifyEvent& rNEvt )
{
    bool bChange = false;
    if(rNEvt.GetType() == MouseNotifyEvent::KEYINPUT)
    {
        const KeyEvent& rKeyEvt = *rNEvt.GetKeyEvent();
        bChange = TextEngine::DoesKeyChangeText( rKeyEvt );
        if(bChange && !IsUndoEditMode() &&
            rKeyEvt.GetKeyCode().GetCode() != KEY_TAB)
        {
            TextEngine* pTextEngine = GetTextEngine();
            TextView* pTextView = pTextEngine->GetActiveView();
            TextSelection aCurrentSelection = pTextView->GetSelection();
            aCurrentSelection.Justify();
            //determine if the selection contains a field
            bool bHasFieldLeft = false;
            bool bHasErrorLeft = false;

            bool bHasRange = aCurrentSelection.HasRange();
            sal_uInt8 nSelectionType = 0; // invalid type!

            TextPaM aCursor(aCurrentSelection.GetStart());
            const TextCharAttrib* pBackAttr = pTextEngine->FindCharAttrib( aCursor, TEXTATTR_SPELL_BACKGROUND );
            const TextCharAttrib* pErrorAttr = pTextEngine->FindCharAttrib( aCursor, TEXTATTR_SPELL_ERROR );
            const TextCharAttrib* pBackAttrLeft = nullptr;
            const TextCharAttrib* pErrorAttrLeft = nullptr;

            bool bHasField = pBackAttr != nullptr && (bHasRange || pBackAttr->GetEnd() > aCursor.GetIndex());
            bool bHasError = pErrorAttr != nullptr && (bHasRange || pErrorAttr->GetEnd() > aCursor.GetIndex());
            if(bHasRange)
            {
                if(pBackAttr &&
                        pBackAttr->GetStart() == aCurrentSelection.GetStart().GetIndex() &&
                        pBackAttr->GetEnd() == aCurrentSelection.GetEnd().GetIndex())
                {
                    nSelectionType = FULL;
                }
                else if(pErrorAttr &&
                        pErrorAttr->GetStart() <= aCurrentSelection.GetStart().GetIndex() &&
                        pErrorAttr->GetEnd() >= aCurrentSelection.GetEnd().GetIndex())
                {
                    nSelectionType = INSIDE_YES;
                }
                else
                {
                    nSelectionType = bHasField||bHasError ? BRACE : OUTSIDE_NO;
                    while(aCursor.GetIndex() < aCurrentSelection.GetEnd().GetIndex())
                    {
                        ++aCursor.GetIndex();
                        const TextCharAttrib* pIntBackAttr = pTextEngine->FindCharAttrib( aCursor, TEXTATTR_SPELL_BACKGROUND );
                        const TextCharAttrib* pIntErrorAttr = pTextEngine->FindCharAttrib( aCursor, TEXTATTR_SPELL_ERROR );
                        //if any attr has been found then BRACE
                        if(pIntBackAttr || pIntErrorAttr)
                            nSelectionType = BRACE;
                        //the field has to be selected
                        if(pIntBackAttr && !pBackAttr)
                            pBackAttr = pIntBackAttr;
                        bHasField |= pIntBackAttr != nullptr;
                    }
                }
            }
            else
            {
                //no range selection: then 1 2 3 and 8 are possible
                const TextCharAttrib* pCurAttr = pBackAttr ? pBackAttr : pErrorAttr;
                if(pCurAttr)
                {
                    nSelectionType = pCurAttr->GetStart() == aCurrentSelection.GetStart().GetIndex() ?
                            LEFT_NO : pCurAttr->GetEnd() == aCurrentSelection.GetEnd().GetIndex() ? RIGHT_NO : INSIDE_NO;
                }
                else
                    nSelectionType = OUTSIDE_NO;

                bHasFieldLeft = pBackAttr && pBackAttr->GetEnd() == aCursor.GetIndex();
                if(bHasFieldLeft)
                {
                    pBackAttrLeft = pBackAttr;
                    pBackAttr = nullptr;
                }
                bHasErrorLeft = pErrorAttr && pErrorAttr->GetEnd() == aCursor.GetIndex();
                if(bHasErrorLeft)
                {
                    pErrorAttrLeft = pErrorAttr;
                    pErrorAttr = nullptr;
                }

                //check previous position if this exists
                //that is a redundant in the case the attribute found above already is on the left cursor side
                //but it's o.k. for two errors/fields side by side
                if(aCursor.GetIndex())
                {
                    --aCursor.GetIndex();
                    pBackAttrLeft = pTextEngine->FindCharAttrib( aCursor, TEXTATTR_SPELL_BACKGROUND );
                    pErrorAttrLeft = pTextEngine->FindCharAttrib( aCursor, TEXTATTR_SPELL_ERROR );
                    bHasFieldLeft = pBackAttrLeft !=nullptr;
                    bHasErrorLeft = pErrorAttrLeft != nullptr;
                    ++aCursor.GetIndex();
                }
            }
            //Here we have to determine if the error found is the one currently active
            bool bIsErrorActive = (pErrorAttr && pErrorAttr->GetStart() == m_nErrorStart) ||
                    (pErrorAttrLeft && pErrorAttrLeft->GetStart() == m_nErrorStart);

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
                        nAction = bHasError && !aCursor.GetIndex() ? ACTION_CONTINUE :
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
            sal_Int32 nCurrentLen = GetText().getLength();
            if(nAction != ACTION_SELECTFIELD)
                pTextView->GetWindow()->KeyInput(rKeyEvt);
            else
            {
                const TextCharAttrib* pCharAttr = pBackAttr ? pBackAttr : pBackAttrLeft;
                if(pCharAttr)
                {
                    TextPaM aStart(0, pCharAttr->GetStart());
                    TextPaM aEnd(0, pCharAttr->GetEnd());
                    TextSelection aNewSel(aStart, aEnd);
                    pTextView->SetSelection( aNewSel);
                }
            }
            if(nAction == ACTION_EXPAND)
            {
                DBG_ASSERT(pErrorAttrLeft || pErrorAttr, "where is the error");
                //text has been added on the right and only the 'error attribute has to be corrected
                if(pErrorAttrLeft)
                {
                    std::unique_ptr<TextAttrib> pNewError(pErrorAttrLeft->GetAttr().Clone());
                    const sal_Int32 nStart = pErrorAttrLeft->GetStart();
                    sal_Int32 nEnd = pErrorAttrLeft->GetEnd();
                    pTextEngine->RemoveAttrib( 0, *pErrorAttrLeft );
                    SetAttrib( *pNewError, 0, nStart, ++nEnd );
                    //only active errors move the mark
                    if(bIsErrorActive)
                    {
                        bool bGrammar = static_cast<const SpellErrorAttrib&>(*pNewError).GetErrorDescription().bIsGrammarError;
                        MoveErrorMarkTo(nStart, nEnd, bGrammar);
                    }
                }
                //text has been added on the left then the error attribute has to be expanded and the
                //field attribute on the right - if any - has to be contracted
                else if(pErrorAttr)
                {
                    //determine the change
                    sal_Int32 nAddedChars = GetText().getLength() - nCurrentLen;

                    std::unique_ptr<TextAttrib> pNewError(pErrorAttr->GetAttr().Clone());
                    sal_Int32 nStart = pErrorAttr->GetStart();
                    sal_Int32 nEnd = pErrorAttr->GetEnd();
                    pTextEngine->RemoveAttrib( 0, *pErrorAttr );
                    nStart = nStart - nAddedChars;
                    SetAttrib( *pNewError, 0, nStart - nAddedChars, nEnd );
                    //only if the error is active the mark is moved here
                    if(bIsErrorActive)
                    {
                        bool bGrammar = static_cast<const SpellErrorAttrib&>(*pNewError).GetErrorDescription().bIsGrammarError;
                        MoveErrorMarkTo(nStart, nEnd, bGrammar);
                    }
                    pNewError.reset();

                    if(pBackAttrLeft)
                    {
                        std::unique_ptr<TextAttrib> pNewBack(pBackAttrLeft->GetAttr().Clone());
                        const sal_Int32 _nStart = pBackAttrLeft->GetStart();
                        const sal_Int32 _nEnd = pBackAttrLeft->GetEnd();
                        pTextEngine->RemoveAttrib( 0, *pBackAttrLeft );
                        SetAttrib( *pNewBack, 0, _nStart, _nEnd - nAddedChars);
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
            if(!IsUndoEditMode() && bIsErrorActive)
            {
                const TextCharAttrib* pFontColor = pTextEngine->FindCharAttrib( aCursor, TEXTATTR_FONTCOLOR );
                const TextCharAttrib*  pErrorAttrib = pTextEngine->FindCharAttrib( TextPaM(0, m_nErrorStart), TEXTATTR_SPELL_ERROR );
                if(pFontColor && pErrorAttrib )
                {
                    m_nErrorStart = pFontColor->GetStart();
                    m_nErrorEnd = pFontColor->GetEnd();
                    if(pErrorAttrib->GetStart() != m_nErrorStart || pErrorAttrib->GetEnd() != m_nErrorEnd)
                    {
                        std::unique_ptr<TextAttrib> pNewError(pErrorAttrib->GetAttr().Clone());
                        pTextEngine->RemoveAttrib( 0, *pErrorAttr );
                        SetAttrib( *pNewError, 0, m_nErrorStart, m_nErrorEnd );
                    }
                }
            }
            //this is not a modification anymore
            if(nAction != ACTION_SELECTFIELD && !m_bIsUndoEditMode)
                CallModifyLink();
        }
        else
            bChange = false;
    }
    return bChange || VclMultiLineEdit::PreNotify(rNEvt);
}

void SentenceEditWindow_Impl::Init(VclPtr<ToolBox> const &rToolbar)
{
    m_xToolbar = rToolbar;
    m_xToolbar->SetSelectHdl(LINK(this,SentenceEditWindow_Impl,ToolbarHdl));
}

IMPL_LINK_NOARG(SentenceEditWindow_Impl, ToolbarHdl, ToolBox *, void)
{
    const sal_uInt16 nCurItemId = m_xToolbar->GetCurItemId();
    if (nCurItemId == m_xToolbar->GetItemId("paste"))
    {
        Paste();
        CallModifyLink();
    }
    else if (nCurItemId == m_xToolbar->GetItemId("insert"))
    {
        if (Edit::GetGetSpecialCharsFunction())
        {
            OUString aChars = Edit::GetGetSpecialCharsFunction()( this, GetFont() );
            if (!aChars.isEmpty())
            {
                ReplaceSelected(aChars);
                CallModifyLink();
            }
        }
    }
}

void SentenceEditWindow_Impl::dispose()
{
    m_xToolbar.clear();
    VclMultiLineEdit::dispose();
}

bool SentenceEditWindow_Impl::MarkNextError( bool bIgnoreCurrentError, const css::uno::Reference<css::linguistic2::XSpellChecker1>& xSpell )
{
    if (bIgnoreCurrentError)
        m_aIgnoreErrorsAt.insert( m_nErrorStart );
    ExtTextEngine* pTextEngine = GetTextEngine();
    const sal_Int32 nTextLen = pTextEngine->GetTextLen(0);
    if(m_nErrorEnd >= nTextLen - 1)
        return false;
    //if it's not already modified the modified flag has to be reset at the end of the marking
    bool bModified = IsModified();
    bool bRet = false;
    const sal_Int32 nOldErrorStart = m_nErrorStart;
    const sal_Int32 nOldErrorEnd   = m_nErrorEnd;

    //create a cursor behind the end of the last error
    //- or at 0 at the start of the sentence
    TextPaM aCursor(0, m_nErrorEnd ? m_nErrorEnd + 1 : 0);
    //search for SpellErrorAttrib

    const TextCharAttrib* pNextError = nullptr;
    //iterate over the text and search for the next error that maybe has
    //to be replace by a ChangeAllList replacement
    bool bGrammarError = false;
    while(aCursor.GetIndex() < nTextLen)
    {
        while(aCursor.GetIndex() < nTextLen &&
                nullptr == (pNextError = pTextEngine->FindCharAttrib( aCursor, TEXTATTR_SPELL_ERROR)))
        {
            ++aCursor.GetIndex();
        }
        // maybe the error found here is already in the ChangeAllList and has to be replaced

        Reference<XDictionary> xChangeAll( LinguMgr::GetChangeAllList(), UNO_QUERY );
        Reference<XDictionaryEntry> xEntry;

        const SpellErrorDescription* pSpellErrorDescription = nullptr;
        if(pNextError)
        {
            pSpellErrorDescription = &static_cast<const SpellErrorAttrib&>(pNextError->GetAttr()).GetErrorDescription();
            bGrammarError = pSpellErrorDescription->bIsGrammarError;
            m_nErrorStart = pNextError->GetStart();
            m_nErrorEnd = pNextError->GetEnd();
        }
        if(xChangeAll->getCount() && pSpellErrorDescription &&
                (xEntry = xChangeAll->getEntry( pSpellErrorDescription->sErrorText )).is())
        {

            OUString sReplacement(getDotReplacementString(GetErrorText(), xEntry->getReplacementText()));

            ChangeMarkedWord(sReplacement, LanguageTag::convertToLanguageType( pSpellErrorDescription->aLocale ));

            aCursor.GetIndex() += xEntry->getReplacementText().getLength();
        // maybe the error found here is already added to the dictionary and has to be ignored
        }
        else if(pSpellErrorDescription && !bGrammarError &&
                xSpell->isValid(GetErrorText(),
                                static_cast<sal_uInt16>(LanguageTag::convertToLanguageType( pSpellErrorDescription->aLocale )),
                                Sequence< PropertyValue >() ))
        {
            ++aCursor.GetIndex();
        }
        else
            break;
    }

    //if an attrib has been found search for the end of the error string
    if(aCursor.GetIndex() < nTextLen)
    {
        MoveErrorMarkTo(aCursor.GetIndex(), pNextError->GetEnd(), bGrammarError);
        bRet = true;
        //add an undo action
        std::unique_ptr<SpellUndoAction_Impl> pAction(new SpellUndoAction_Impl(
                SPELLUNDO_CHANGE_NEXTERROR, GetSpellDialog()->aDialogUndoLink));
        pAction->SetErrorMove(nOldErrorStart, nOldErrorEnd);
        const SpellErrorAttrib* pOldAttrib = static_cast<const SpellErrorAttrib*>(
                pTextEngine->FindAttrib( TextPaM(0, nOldErrorStart), TEXTATTR_SPELL_ERROR ));
        pAction->SetErrorLanguageSelected(pOldAttrib && pOldAttrib->GetErrorDescription().aSuggestions.getLength() &&
                LanguageTag( pOldAttrib->GetErrorDescription().aLocale).getLanguageType() ==
                                        GetSpellDialog()->m_pLanguageLB->GetSelectedLanguage());
        AddUndoAction(std::move(pAction));
    }
    else
        m_nErrorStart = m_nErrorEnd = nTextLen;
    if( !bModified )
        ClearModifyFlag();
    SpellDialog* pSpellDialog = GetSpellDialog();
    pSpellDialog->m_pIgnorePB->Enable(bRet);
    pSpellDialog->m_pIgnoreAllPB->Enable(bRet);
    pSpellDialog->m_pAutoCorrPB->Enable(bRet);
    pSpellDialog->m_pAddToDictMB->Enable(bRet);
    pSpellDialog->m_pAddToDictPB->Enable(bRet);
    return bRet;
}


void SentenceEditWindow_Impl::MoveErrorMarkTo(sal_Int32 nStart, sal_Int32 nEnd, bool bGrammarError)
{
    TextEngine* pTextEngine = GetTextEngine();
    pTextEngine->RemoveAttribs( 0, sal_uInt16(TEXTATTR_FONTCOLOR) );
    pTextEngine->RemoveAttribs( 0, sal_uInt16(TEXTATTR_FONTWEIGHT) );
    pTextEngine->SetAttrib( TextAttribFontWeight(WEIGHT_BOLD), 0, nStart, nEnd );
    pTextEngine->SetAttrib( TextAttribFontColor(bGrammarError ? COL_LIGHTBLUE : COL_LIGHTRED), 0, nStart, nEnd );
    m_nErrorStart = nStart;
    m_nErrorEnd = nEnd;
}


void SentenceEditWindow_Impl::ChangeMarkedWord(const OUString& rNewWord, LanguageType eLanguage)
{
    //calculate length changes
    long nDiffLen = rNewWord.getLength() - m_nErrorEnd + m_nErrorStart;
    TextSelection aSel(TextPaM(0, m_nErrorStart), TextPaM(0, m_nErrorEnd));
    //Remove spell error attribute
    ExtTextEngine* pTextEngine = GetTextEngine();
    pTextEngine->UndoActionStart();
    const TextCharAttrib*  pErrorAttrib = pTextEngine->FindCharAttrib( TextPaM(0, m_nErrorStart), TEXTATTR_SPELL_ERROR );
    std::unique_ptr<TextCharAttrib> pReleasedErrorAttrib;
    std::unique_ptr<TextCharAttrib> pReleasedLangAttrib;
    std::unique_ptr<TextCharAttrib> pReleasedBackAttrib;
    DBG_ASSERT(pErrorAttrib, "no error attribute found");
    const SpellErrorDescription* pSpellErrorDescription = nullptr;
    if(pErrorAttrib)
    {
        pReleasedErrorAttrib = pTextEngine->RemoveAttrib(0, *pErrorAttrib);
        pSpellErrorDescription = &static_cast<const SpellErrorAttrib&>(pErrorAttrib->GetAttr()).GetErrorDescription();
    }
    const TextCharAttrib*  pBackAttrib = pTextEngine->FindCharAttrib( TextPaM(0, m_nErrorStart), TEXTATTR_SPELL_BACKGROUND );
    pTextEngine->ReplaceText( aSel, rNewWord );

    if(!m_nErrorStart)
    {
        //attributes following an error at the start of the text are not moved but expanded from the
        //text engine - this is done to keep full-paragraph-attributes
        //in the current case that handling is not desired
        const TextCharAttrib*  pLangAttrib =
                pTextEngine->FindCharAttrib(
                    TextPaM(0, m_nErrorEnd), TEXTATTR_SPELL_LANGUAGE );
        const sal_Int32 nTextLen = pTextEngine->GetTextLen( 0 );
        if(pLangAttrib && !pLangAttrib->GetStart() && pLangAttrib->GetEnd() ==
            nTextLen)
        {
            SpellLanguageAttrib aNewLangAttrib( static_cast<const SpellLanguageAttrib&>(pLangAttrib->GetAttr()).GetLanguage());
            pReleasedLangAttrib = pTextEngine->RemoveAttrib(0, *pLangAttrib);
            pTextEngine->SetAttrib( aNewLangAttrib, 0, m_nErrorEnd + nDiffLen, nTextLen );
        }
    }
    // undo expanded attributes!
    if( pBackAttrib && pBackAttrib->GetStart() < m_nErrorStart && pBackAttrib->GetEnd() == m_nErrorEnd + nDiffLen)
    {
        std::unique_ptr<TextAttrib> pNewBackground(pBackAttrib->GetAttr().Clone());
        const sal_Int32 nStart = pBackAttrib->GetStart();
        pReleasedBackAttrib = pTextEngine->RemoveAttrib(0, *pBackAttrib);
        pTextEngine->SetAttrib(*pNewBackground, 0, nStart, m_nErrorStart);
    }
    pTextEngine->SetModified(true);

    //adjust end position
    long nEndTemp = m_nErrorEnd;
    nEndTemp += nDiffLen;
    m_nErrorEnd = static_cast<sal_Int32>(nEndTemp);

    std::unique_ptr<SpellUndoAction_Impl> pAction(new SpellUndoAction_Impl(
                    SPELLUNDO_MOVE_ERROREND, GetSpellDialog()->aDialogUndoLink));
    pAction->SetOffset(nDiffLen);
    AddUndoAction(std::move(pAction));
    if(pSpellErrorDescription)
        SetAttrib( SpellErrorAttrib(*pSpellErrorDescription), 0, m_nErrorStart, m_nErrorEnd );
    SetAttrib( SpellLanguageAttrib(eLanguage), 0, m_nErrorStart, m_nErrorEnd );
    pTextEngine->UndoActionEnd();
}


OUString SentenceEditWindow_Impl::GetErrorText() const
{
    return GetTextEngine()->GetText(TextSelection(TextPaM(0, m_nErrorStart), TextPaM(0, m_nErrorEnd) ));
}


const SpellErrorDescription* SentenceEditWindow_Impl::GetAlternatives()
{
    TextPaM aCursor(0, m_nErrorStart);
    const SpellErrorAttrib* pAttrib = static_cast<const SpellErrorAttrib*>(
            GetTextEngine()->FindAttrib( aCursor, TEXTATTR_SPELL_ERROR));
    return pAttrib ? &pAttrib->GetErrorDescription() : nullptr;
}


void SentenceEditWindow_Impl::RestoreCurrentError()
{
    TextPaM aCursor(0, m_nErrorStart);
    const SpellErrorAttrib* pAttrib = static_cast<const SpellErrorAttrib*>(
            GetTextEngine()->FindAttrib( aCursor, TEXTATTR_SPELL_ERROR));
    if( pAttrib )
    {
        const SpellErrorDescription& rDesc = pAttrib->GetErrorDescription();
        if( rDesc.sErrorText != GetErrorText() )
            ChangeMarkedWord(rDesc.sErrorText, LanguageTag::convertToLanguageType( rDesc.aLocale ));
    }
}


void SentenceEditWindow_Impl::SetAlternatives( const Reference< XSpellAlternatives>& xAlt )
{
    TextPaM aCursor(0, m_nErrorStart);
    DBG_ASSERT(static_cast<const SpellErrorAttrib*>(
            GetTextEngine()->FindAttrib( aCursor, TEXTATTR_SPELL_ERROR)), "no error set?");

    OUString aWord;
    lang::Locale    aLocale;
    uno::Sequence< OUString >    aAlts;
    OUString sServiceName;
    if (xAlt.is())
    {
        aWord   = xAlt->getWord();
        aLocale = xAlt->getLocale();
        aAlts   = xAlt->getAlternatives();
        uno::Reference< container::XNamed > xNamed( xAlt, uno::UNO_QUERY );
        if (xNamed.is())
            sServiceName = xNamed->getName();
    }
    SpellErrorDescription aDesc( false, aWord, aLocale, aAlts, nullptr);
    GetTextEngine()->SetAttrib( SpellErrorAttrib(aDesc), 0, m_nErrorStart, m_nErrorEnd );
}

void SentenceEditWindow_Impl::SetAttrib( const TextAttrib& rAttr, sal_uInt32 nPara, sal_Int32 nStart, sal_Int32 nEnd )
{
    GetTextEngine()->SetAttrib(rAttr, nPara, nStart, nEnd);
}


void SentenceEditWindow_Impl::SetText( const OUString& rStr )
{
    m_nErrorStart = m_nErrorEnd = 0;
    GetTextEngine()->SetText(rStr);
}


struct LanguagePosition_Impl
{
    sal_Int32       nPosition;
    LanguageType    eLanguage;

    LanguagePosition_Impl(sal_Int32 nPos, LanguageType eLang) :
        nPosition(nPos),
        eLanguage(eLang)
        {}
};
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
    ExtTextEngine* pTextEngine = GetTextEngine();
    const sal_Int32 nTextLen = pTextEngine->GetTextLen(0);
    if(nTextLen)
    {
        TextPaM aCursor(0, 0);
        LanguagePositions_Impl aBreakPositions;
        const TextCharAttrib* pLastLang = nullptr;
        const TextCharAttrib* pLastError = nullptr;
        LanguageType eLang = LANGUAGE_DONTKNOW;
        const TextCharAttrib* pError = nullptr;
        while(aCursor.GetIndex() < nTextLen)
        {
            const TextCharAttrib* pLang = pTextEngine->FindCharAttrib( aCursor, TEXTATTR_SPELL_LANGUAGE);
            if(pLang && pLang != pLastLang)
            {
                eLang = static_cast<const SpellLanguageAttrib&>(pLang->GetAttr()).GetLanguage();
                lcl_InsertBreakPosition_Impl(aBreakPositions, pLang->GetStart(), eLang);
                lcl_InsertBreakPosition_Impl(aBreakPositions, pLang->GetEnd(), eLang);
                pLastLang = pLang;
            }
            pError = pTextEngine->FindCharAttrib( aCursor, TEXTATTR_SPELL_ERROR);
            if(pError && pLastError != pError)
            {
                lcl_InsertBreakPosition_Impl(aBreakPositions, pError->GetStart(), eLang);
                lcl_InsertBreakPosition_Impl(aBreakPositions, pError->GetEnd(), eLang);
                pLastError = pError;

            }
            ++aCursor.GetIndex();
        }

        if (aBreakPositions.empty())
        {
            //if all content has been overwritten the attributes may have been removed, too
            svx::SpellPortion aPortion1;
            aPortion1.eLanguage = GetSpellDialog()->GetSelectedLang_Impl();
            aPortion1.sText = pTextEngine->GetText(
                        TextSelection(TextPaM(0, 0), TextPaM(0, nTextLen)));

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
                aPortion1.sText = pTextEngine->GetText(
                            TextSelection(TextPaM(0, nStart), TextPaM(0, aStart->nPosition)));
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

        // quick partly fix of #i71318. Correct fix needs to patch the TextEngine itself...
        // this one will only prevent text from disappearing. It may to not have the
        // correct language and will probably not spell checked...
        const sal_uInt32 nPara = pTextEngine->GetParagraphCount();
        if (nPara > 1)
        {
            OUStringBuffer aLeftOverText;
            for (sal_uInt32 i = 1; i < nPara; ++i)
            {
                aLeftOverText.append("\x0a");    // the manual line break...
                aLeftOverText.append(pTextEngine->GetText(i));
            }
            if (pError)
            {   // we need to add a new portion containing the left-over text
                svx::SpellPortion aPortion2;
                aPortion2.eLanguage = eLang;
                aPortion2.sText = aLeftOverText.makeStringAndClear();
                aRet.push_back( aPortion2 );
            }
            else
            {   // we just need to append the left-over text to the last portion (which had no errors)
                aRet[ aRet.size() - 1 ].sText += aLeftOverText;
            }
        }
   }
    return aRet;
}


void SentenceEditWindow_Impl::Undo()
{
    SfxUndoManager& rUndoMgr = GetTextEngine()->GetUndoManager();
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
    GetTextEngine()->ResetUndo();
}


void SentenceEditWindow_Impl::AddUndoAction( std::unique_ptr<SfxUndoAction> pAction )
{
    SfxUndoManager& rUndoMgr = GetTextEngine()->GetUndoManager();
    rUndoMgr.AddUndoAction(std::move(pAction));
    GetSpellDialog()->m_pUndoPB->Enable();
}


size_t SentenceEditWindow_Impl::GetUndoActionCount()
{
    return GetTextEngine()->GetUndoManager().GetUndoActionCount();
}


void SentenceEditWindow_Impl::UndoActionStart( sal_uInt16 nId )
{
    GetTextEngine()->UndoActionStart(nId);
}


void SentenceEditWindow_Impl::UndoActionEnd()
{
    GetTextEngine()->UndoActionEnd();
}


void SentenceEditWindow_Impl::MoveErrorEnd(long nOffset)
{
    // Shouldn't we always add the real signed value instead???
    if(nOffset > 0)
        m_nErrorEnd = m_nErrorEnd - static_cast<sal_Int32>(nOffset);
    else
        m_nErrorEnd = m_nErrorEnd - static_cast<sal_Int32>(-nOffset);
}


void  SentenceEditWindow_Impl::SetUndoEditMode(bool bSet)
{
    DBG_ASSERT(!bSet || m_bIsUndoEditMode != bSet, "SetUndoEditMode with equal values?");
    m_bIsUndoEditMode = bSet;
    //disable all buttons except the Change
    SpellDialog* pSpellDialog = GetSpellDialog();
    Control* aControls[] =
    {
        pSpellDialog->m_pChangeAllPB,
        pSpellDialog->m_pExplainFT,
        pSpellDialog->m_pIgnoreAllPB,
        pSpellDialog->m_pIgnoreRulePB,
        pSpellDialog->m_pIgnorePB,
        pSpellDialog->m_pSuggestionLB,
        pSpellDialog->m_pSuggestionFT,
        pSpellDialog->m_pLanguageFT,
        pSpellDialog->m_pLanguageLB,
        pSpellDialog->m_pAddToDictMB,
        pSpellDialog->m_pAddToDictPB,
        pSpellDialog->m_pAutoCorrPB,
        nullptr
    };
    sal_Int32 nIdx = 0;
    do
    {
        aControls[nIdx]->Enable(false);
    }
    while(aControls[++nIdx]);

    //remove error marks
    TextEngine* pTextEngine = GetTextEngine();
    pTextEngine->RemoveAttribs( 0, sal_uInt16(TEXTATTR_FONTCOLOR) );
    pTextEngine->RemoveAttribs( 0, sal_uInt16(TEXTATTR_FONTWEIGHT) );

    //put the appropriate action on the Undo-stack
    AddUndoAction( std::make_unique<SpellUndoAction_Impl>(
                        SPELLUNDO_UNDO_EDIT_MODE, GetSpellDialog()->aDialogUndoLink) );
    pSpellDialog->m_pChangePB->Enable();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
