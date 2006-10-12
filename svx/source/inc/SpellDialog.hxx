/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SpellDialog.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 12:50:07 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _SVX_SPELLDDIALOG_HXX
#define _SVX_SPELLDDIALOG_HXX

// include ---------------------------------------------------------------

#ifndef _BASEDLGS_HXX //autogen
#include <sfx2/basedlgs.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _STDCTRL_HXX //autogen
#include <svtools/stdctrl.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _MENUBTN_HXX //autogen
#include <vcl/menubtn.hxx>
#endif

#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#ifndef _SV_DECOVIEW_HXX //autogen
#include <vcl/decoview.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif


#ifndef _SVX_BOX_HXX //autogen
#include "svxbox.hxx"
#endif
#ifndef _SVX_LANGBOX_HXX
#include <langbox.hxx>
#endif
#include <memory>
#ifndef _SVEDIT_HXX
#include <svtools/svmedit.hxx>
#endif
#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif

#ifndef _XTEXTEDT_HXX //autogen
#include <svtools/xtextedt.hxx>
#endif
#ifndef SVX_SPELL_PORTIONS_HXX
#include <SpellPortions.hxx>
#endif

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

