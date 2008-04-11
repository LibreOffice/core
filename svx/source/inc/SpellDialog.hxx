/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SpellDialog.hxx,v $
 * $Revision: 1.8 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SVX_SPELLDDIALOG_HXX
#define _SVX_SPELLDDIALOG_HXX

// include ---------------------------------------------------------------

#include <sfx2/basedlgs.hxx>
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#include <svtools/stdctrl.hxx>

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _MENUBTN_HXX //autogen
#include <vcl/menubtn.hxx>
#endif

#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#include <vcl/decoview.hxx>
#include <com/sun/star/uno/Reference.hxx>


#include <svx/svxbox.hxx>
#include <svx/langbox.hxx>
#include <memory>
#include <svtools/svmedit.hxx>
#include <svtools/lstner.hxx>
#include <svtools/xtextedt.hxx>
#include <svx/SpellPortions.hxx>

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
// ------------------------------------------------------------------
class SentenceEditWindow_Impl : public MultiLineEdit/*, public SfxListener*/
{
    using MultiLineEdit::SetText;

private:
    USHORT          m_nErrorStart;
    USHORT          m_nErrorEnd;
    bool            m_bIsUndoEditMode;

    Link                    m_aModifyLink;

    void            CallModifyLink() {m_aModifyLink.Call(this);}

    SpellDialog*    GetSpellDialog() const {return (SpellDialog*)GetParent();}
protected:
    virtual long    PreNotify( NotifyEvent& rNEvt );

public:
                    SentenceEditWindow_Impl( SpellDialog* pParent, const ResId& rResId );
                    ~SentenceEditWindow_Impl();

    void            SetModifyHdl(const Link& rLink) { m_aModifyLink = rLink;}

    void            SetAttrib( const TextAttrib& rAttr, ULONG nPara, USHORT nStart, USHORT nEnd );
    void            SetText( const String& rStr );

    bool            MarkNextError();
    void            ChangeMarkedWord(const String& rNewWord, LanguageType eLanguage);
    void            MoveErrorMarkTo(USHORT nErrorStart, USHORT nErrorEnd);
    String          GetErrorText() const;
    void            RestoreCurrentError();

    com::sun::star::uno::Reference<com::sun::star::linguistic2::XSpellAlternatives> GetAlternatives();
    void            SetAlternatives(
                        com::sun::star::uno::Reference<com::sun::star::linguistic2::XSpellAlternatives> );

    void            ResetModified()   { GetTextEngine()->SetModified(FALSE); m_bIsUndoEditMode = false;}
    BOOL            IsModified() const              { return GetTextEngine()->IsModified(); }

    bool            IsUndoEditMode() const { return m_bIsUndoEditMode;}
    void            SetUndoEditMode(bool bSet);

    svx::SpellPortions  CreateSpellPortions() const;

    void            ResetUndo();
    void            Undo();
    void            AddUndoAction( SfxUndoAction *pAction, BOOL bTryMerg=FALSE );
    USHORT          GetUndoActionCount();
    void            UndoActionStart( USHORT nId );
    void            UndoActionEnd( USHORT nId );

    void            MoveErrorEnd(long nOffset);
};


// class SvxSpellDialog ---------------------------------------------
class SpellDialogChildWindow;
class SpellDialog : public SfxModelessDialog
{
    using Window::Invalidate;

    friend class SentenceEditWindow_Impl;
private:
    FixedText           aNotInDictFT;
    SentenceEditWindow_Impl  aSentenceED;

    FixedText       aSuggestionFT;
    ListBox         aSuggestionLB;

    FixedText       aLanguageFT;
    SvxLanguageBox  aLanguageLB;

    PushButton      aIgnorePB;
    PushButton      aIgnoreAllPB;
    MenuButton      aAddToDictMB;

    PushButton      aChangePB;
    PushButton      aChangeAllPB;
    PushButton      aAutoCorrPB;

    PushButton      aOptionsPB;
    HelpButton      aHelpPB;
    PushButton      aUndoPB;
    PushButton      aClosePB;

    GroupBox        aBackgroundGB;

    String          aTitel;
    String          aResumeST;
    String          aIgnoreOnceST;
    String          aNoSuggestionsST;

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
    void            SpellContinue_Impl(bool UseSavedSentence = false);
    void            LockFocusChanges( bool bLock ) {bFocusLocked = bLock;}

    void            SetSelectedLang_Impl( LanguageType nLang );
    LanguageType    GetSelectedLang_Impl() const;

    /** Retrieves the next sentence.
     */
    bool            GetNextSentence_Impl(bool bUseSavedSentence);
    /** Corrects all errors that have been selected to be changed always
     */
    bool            ApplyChangeAllList_Impl(SpellPortions& rSentence, bool& bHasReplaced);

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

