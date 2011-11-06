/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _SVX_SPELLDDIALOG_HXX
#define _SVX_SPELLDDIALOG_HXX

// include ---------------------------------------------------------------

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


#include <svtools/svxbox.hxx>
#include <svx/langbox.hxx>
#include <memory>
#include <svtools/svmedit.hxx>
#include <svl/lstner.hxx>
#include <svtools/xtextedt.hxx>
#include <editeng/SpellPortions.hxx>

#include <set>

class ScrollBar;
class TextEngine;
class ExtTextView;
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
// ------------------------------------------------------------------
class SentenceEditWindow_Impl : public MultiLineEdit/*, public SfxListener*/
{
    using MultiLineEdit::SetText;

private:
    std::set< sal_uInt16 >      m_aIgnoreErrorsAt;
    sal_uInt16          m_nErrorStart;
    sal_uInt16          m_nErrorEnd;
    bool            m_bIsUndoEditMode;

    Link            m_aModifyLink;

    void            CallModifyLink() {m_aModifyLink.Call(this);}

    SpellDialog*    GetSpellDialog() const {return (SpellDialog*)GetParent();}
protected:
    virtual long    PreNotify( NotifyEvent& rNEvt );

public:
                    SentenceEditWindow_Impl( SpellDialog* pParent, const ResId& rResId );
                    ~SentenceEditWindow_Impl();

    void            SetModifyHdl(const Link& rLink) { m_aModifyLink = rLink;}

    void            SetAttrib( const TextAttrib& rAttr, sal_uLong nPara, sal_uInt16 nStart, sal_uInt16 nEnd );
    void            SetText( const String& rStr );

    bool            MarkNextError( bool bIgnoreCurrentError );
    void            ChangeMarkedWord(const String& rNewWord, LanguageType eLanguage);
    void            MoveErrorMarkTo(sal_uInt16 nErrorStart, sal_uInt16 nErrorEnd, bool bGrammar);
    String          GetErrorText() const;
    void            RestoreCurrentError();

    void            SetAlternatives(
                        com::sun::star::uno::Reference<com::sun::star::linguistic2::XSpellAlternatives> );

    const SpellErrorDescription* GetAlternatives();


    void            ResetModified()   { GetTextEngine()->SetModified(sal_False); m_bIsUndoEditMode = false;}
    sal_Bool            IsModified() const              { return GetTextEngine()->IsModified(); }

    bool            IsUndoEditMode() const { return m_bIsUndoEditMode;}
    void            SetUndoEditMode(bool bSet);

    svx::SpellPortions  CreateSpellPortions( bool bSetIgnoreFlag ) const;

    void            ResetUndo();
    void            Undo();
    void            AddUndoAction( SfxUndoAction *pAction, sal_Bool bTryMerg=sal_False );
    sal_uInt16      GetUndoActionCount();
    void            UndoActionStart( sal_uInt16 nId );
    void            UndoActionEnd();

    void            MoveErrorEnd(long nOffset);

    void            ResetIgnoreErrorsAt()   { m_aIgnoreErrorsAt.clear(); }
};


// class SvxSpellDialog ---------------------------------------------
class SpellDialogChildWindow;
class ExplainButton : public PushButton
{
    String              m_sExplanation;

    virtual void        RequestHelp( const HelpEvent& rHEvt );
    virtual void        Click();
public:
    ExplainButton( Window* pParent, const ResId& rResId ) : PushButton( pParent, rResId ){}
    ~ExplainButton();
    void                SetExplanation( const String& rText ) {m_sExplanation = rText;}
    bool                HasExplanation() { return m_sExplanation.Len() > 0;}

};

class SpellDialog : public SfxModelessDialog
{
    using Window::Invalidate;

    friend class SentenceEditWindow_Impl;
private:

    FixedImage      aVendorImageFI;

    FixedText       aLanguageFT;
    SvxLanguageBox  aLanguageLB;

    FixedText           aNotInDictFT;
    SentenceEditWindow_Impl  aSentenceED;

    FixedText       aSuggestionFT;
    ListBox         aSuggestionLB;

    PushButton      aIgnorePB;
    PushButton      aIgnoreAllPB;
    PushButton      aIgnoreRulePB;
    MenuButton      aAddToDictMB;

    PushButton      aChangePB;
    PushButton      aChangeAllPB;
    ExplainButton   aExplainPB;
    PushButton      aAutoCorrPB;

    CheckBox        aCheckGrammarCB;

    HelpButton      aHelpPB;
    PushButton      aOptionsPB;
    PushButton      aUndoPB;
    PushButton      aClosePB;

    GroupBox        aBackgroundGB;

    Image           aVendorImage;
    Image           aVendorImageHC;

    String          aResumeST;
    String          aIgnoreOnceST;
    String          aNoSuggestionsST;

    const String    m_sTitleSpelling;
    const String    m_sTitleSpellingGrammar;
    const String    m_sTitleSpellingGrammarVendor;

    Size            aOldWordEDSize;
    Link            aDialogUndoLink;

    bool            bModified;
    bool            bFocusLocked;

    svx::SpellDialogChildWindow& rParent;
    svx::SpellPortions           m_aSavedSentence;

    SpellDialog_Impl* pImpl;
    ::com::sun::star::uno::Reference<
        ::com::sun::star::linguistic2::XSpellChecker1 >     xSpell;
    LanguageType        nOldLang;

    DECL_LINK( ChangeHdl, Button * );
    DECL_LINK( ChangeAllHdl, Button * );
    DECL_LINK( IgnoreAllHdl, Button * );
    DECL_LINK( IgnoreHdl, Button * );
    DECL_LINK( CheckGrammarHdl, CheckBox* );
    DECL_LINK( ExtClickHdl, Button * );
    DECL_LINK( CancelHdl, Button * );
    DECL_LINK( ModifyHdl, SentenceEditWindow_Impl *);
    DECL_LINK( UndoHdl, Button * );
    DECL_LINK( AddToDictionaryHdl, MenuButton* );
    DECL_LINK( LanguageSelectHdl, SvxLanguageBox* );
    DECL_LINK( DialogUndoHdl, SpellUndoAction_Impl* );

    DECL_STATIC_LINK( SpellDialog, InitHdl, SpellDialog * );

    void            StartSpellOptDlg_Impl();
    void            InitUserDicts();
    void            UpdateBoxes_Impl();
    void            Init_Impl();
    void            SpellContinue_Impl(bool UseSavedSentence = false, bool bIgnoreCurrentError = false );
    void            LockFocusChanges( bool bLock ) {bFocusLocked = bLock;}
    void            Impl_Restore();

    void            SetSelectedLang_Impl( LanguageType nLang );
    LanguageType    GetSelectedLang_Impl() const;

    /** Retrieves the next sentence.
     */
    bool            GetNextSentence_Impl(bool bUseSavedSentence, bool bRechek /*for rechecking the curretn sentence*/);
    /** Corrects all errors that have been selected to be changed always
     */
    bool            ApplyChangeAllList_Impl(SpellPortions& rSentence, bool& bHasReplaced);
    void            SetTitle_Impl(LanguageType nLang);

protected:
    virtual void    Paint( const Rectangle& rRect );
    virtual long    Notify( NotifyEvent& rNEvt );

public:
    SpellDialog(
        svx::SpellDialogChildWindow* pChildWindow,
        Window * pParent,
        SfxBindings* pBindings);
    ~SpellDialog();

    void            SetLanguage( sal_uInt16 nLang );
    virtual sal_Bool    Close();

    void            InvalidateDialog();
};
} //namespace svx

#endif

