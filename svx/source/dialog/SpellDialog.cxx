/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SpellDialog.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: obo $ $Date: 2008-03-25 16:41:32 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#ifdef SVX_DLLIMPLEMENTATION
#undef SVX_DLLIMPLEMENTATION
#endif

// include ---------------------------------------------------------------

#include <tools/ref.hxx>

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _SV_WRKWIN_HXX
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SV_MENU_HXX
#include <vcl/menu.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _SCRBAR_HXX //autogen
#include <vcl/scrbar.hxx>
#endif
#ifndef _SVX_SPELL_ATTRIB
#include <SpellAttrib.hxx>
#endif
#ifndef _SFXDISPATCH_HXX
#include <sfx2/dispatch.hxx>
#endif
#ifndef _SFX_BINDINGS_HXX
#include <sfx2/bindings.hxx>
#endif
#ifndef _UNDO_HXX
#include <svtools/undo.hxx>
#endif
#ifndef _TEXTDATA_HXX
#include <svtools/textdata.hxx>
#endif
#ifndef _UNO_LINGU_HXX
#include <unolingu.hxx>
#endif

#ifndef _SVX_SPLWRAP_HXX
#include <svx/splwrap.hxx>
#endif
#ifndef _LINGUISTIC_LNGPROPS_HHX_
#include <linguistic/lngprops.hxx>
#endif
#ifndef _LINGUISTIC_MISC_HHX_
#include <linguistic/misc.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LINGUISTIC2_SPELLFAILURE_HPP_
#include <com/sun/star/linguistic2/SpellFailure.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTORABLE_HPP_
#include <com/sun/star/frame/XStorable.hpp>
#endif

#ifndef _SFXAPP_HXX //autogen
#include <sfx2/app.hxx>
#endif


#include <svx/dialogs.hrc>
#include <helpid.hrc>
#include "SpellDialog.hrc"



#include "optitems.hxx"
#include <svx/svxenum.hxx>
#include <svx/SpellDialogChildWindow.hxx>
#include "SpellDialog.hxx"
//#include <svx/splwrap.hxx>      // Der Wrapper
#include "dlgutil.hxx"      // language
#include <svx/optlingu.hxx>
#include <svx/dialmgr.hxx>
#include "svxerr.hxx"
#include "treeopt.hxx"

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::linguistic2;

#define C2U(cChar)                  OUString::createFromAscii(cChar)
// struct SpellDialog_Impl ---------------------------------------------

struct SpellDialog_Impl
{
    Sequence< Reference< XDictionary >  >   aDics;
};
// -----------------------------------------------------------------------
#define SPELLUNDO_CHANGE_LANGUAGE           (TEXTUNDO_USER + 1)
#define SPELLUNDO_CHANGE_TEXTENGINE         (TEXTUNDO_USER + 2)
#define SPELLUNDO_CHANGE_NEXTERROR          (TEXTUNDO_USER + 3)
#define SPELLUNDO_CHANGE_ADD_TO_DICTIONARY  (TEXTUNDO_USER + 4)
#define SPELLUNDO_CHANGE_GROUP              (TEXTUNDO_USER + 5) //undo list
#define SPELLUNDO_MOVE_ERROREND             (TEXTUNDO_USER + 6)
#define SPELLUNDO_UNDO_EDIT_MODE            (TEXTUNDO_USER + 7)

namespace svx{
class SpellUndoAction_Impl : public SfxUndoAction
{
    USHORT          m_nId;
    const Link&     m_rActionLink;
    //undo of button enabling
    bool            m_bEnableChangePB;
    bool            m_bEnableChangeAllPB;
    //undo of MarkNextError - used in change and change all, ignore and ignore all
    long            m_nNewErrorStart;
    long            m_nNewErrorEnd;
    long            m_nOldErrorStart;
    long            m_nOldErrorEnd;
    bool            m_bIsErrorLanguageSelected;
    //undo of AddToDictionary
    Reference<XDictionary>  m_xDictionary;
    OUString                m_sAddedWord;
    //move end of error - ::ChangeMarkedWord()
    long            m_nOffset;

public:
    SpellUndoAction_Impl(USHORT nId, const Link& rActionLink) :
        m_nId(nId),
        m_rActionLink( rActionLink),
        m_bEnableChangePB(false),
        m_bEnableChangeAllPB(false),
        m_nNewErrorStart(-1),
        m_nNewErrorEnd(-1),
        m_nOldErrorStart(-1),
        m_nOldErrorEnd(-1),
        m_bIsErrorLanguageSelected(false),
        m_nOffset(0)
            {}

    ~SpellUndoAction_Impl();

    virtual void            Undo();
    virtual USHORT          GetId() const;

    void                    SetEnableChangePB(){m_bEnableChangePB = true;}
    bool                    IsEnableChangePB(){return m_bEnableChangePB;}

    void                    SetEnableChangeAllPB(){m_bEnableChangeAllPB = true;}
    bool                    IsEnableChangeAllPB(){return m_bEnableChangeAllPB;}

    void                    SetErrorMove(long nNewStart, long nNewEnd, long nOldStart, long nOldEnd)
                                {
                                        m_nNewErrorStart = nNewStart;
                                        m_nNewErrorEnd  = nNewEnd;
                                        m_nOldErrorStart = nOldStart;
                                        m_nOldErrorEnd = nOldEnd;
                                }
    long                    GetNewErrorStart() { return m_nNewErrorStart;}
    long                    GetNewErrorEnd() { return m_nNewErrorEnd;}
    long                    GetOldErrorStart() { return m_nOldErrorStart;}
    long                    GetOldErrorEnd() { return m_nOldErrorEnd;}

    void                    SetErrorLanguageSelected(bool bSet){ m_bIsErrorLanguageSelected = bSet;}
    bool                    IsErrorLanguageSelected() const {return m_bIsErrorLanguageSelected;}


    void                    SetDictionary(Reference<XDictionary> xDict) { m_xDictionary = xDict; }
    Reference<XDictionary>  GetDictionary() const {return m_xDictionary;}
    void                    SetAddedWord(const OUString& rWord) {m_sAddedWord = rWord;}
    const OUString&         GetAddedWord() const { return m_sAddedWord;}

    void                    SetOffset(long nSet) {m_nOffset = nSet;}
    long                    GetOffset() const {return m_nOffset;}

};
}//namespace svx
using namespace ::svx;
/*-- 06.11.2003 12:16:02---------------------------------------------------

  -----------------------------------------------------------------------*/
SpellUndoAction_Impl::~SpellUndoAction_Impl()
{
}
/*-- 06.11.2003 12:16:02---------------------------------------------------

  -----------------------------------------------------------------------*/
void SpellUndoAction_Impl::Undo()
{
    m_rActionLink.Call(this);
}
/*-- 06.11.2003 12:16:02---------------------------------------------------

  -----------------------------------------------------------------------*/
USHORT SpellUndoAction_Impl::GetId()const
{
    return m_nId;
}

// class SvxSpellCheckDialog ---------------------------------------------

SpellDialog::SpellDialog(
        SpellDialogChildWindow* pChildWindow,
        Window * pParent,
        SfxBindings* _pBindings)
            : SfxModelessDialog (_pBindings,
                                    pChildWindow,
                                    pParent,
                                    SVX_RES(RID_SVXDLG_SPELLCHECK)),

    aNotInDictFT    ( this, SVX_RES( FT_NOTINDICT ) ),
    aSentenceED      ( this, SVX_RES( ED_NEWWORD ) ),
    aSuggestionFT   ( this, SVX_RES( FT_SUGGESTION ) ),
    aSuggestionLB   ( this, SVX_RES( LB_SUGGESTION ) ),
    aLanguageFT     ( this, SVX_RES( FT_LANGUAGE ) ),
    aLanguageLB     ( this, SVX_RES( LB_LANGUAGE ) ),

    aIgnorePB       ( this, SVX_RES( PB_IGNORE ) ),
    aIgnoreAllPB    ( this, SVX_RES( PB_IGNOREALL ) ),
    aAddToDictMB    ( this, SVX_RES( MB_ADDTODICT ) ),

    aChangePB       ( this, SVX_RES( PB_CHANGE ) ),
    aChangeAllPB    ( this, SVX_RES( PB_CHANGEALL ) ),
    aAutoCorrPB     ( this, SVX_RES( PB_AUTOCORR ) ),

    aOptionsPB      ( this, SVX_RES( PB_OPTIONS ) ),
    aHelpPB         ( this, SVX_RES( PB_HELP ) ),
    aUndoPB         ( this, SVX_RES( PB_UNDO ) ),
    aClosePB        ( this, SVX_RES( PB_CLOSE ) ),
    aBackgroundGB   ( this, SVX_RES( GB_BACKGROUND ) ),

    aResumeST       ( SVX_RES(ST_RESUME )),
    aIgnoreOnceST   ( aIgnorePB.GetText()),
    aNoSuggestionsST( SVX_RES(ST_NOSUGGESTIONS)),
    aDialogUndoLink( LINK (this, SpellDialog, DialogUndoHdl)),
    bModified( false ),
    bFocusLocked( false ),
    rParent         ( *pChildWindow ),
    nOldLang        ( LANGUAGE_NONE )
{
    FreeResource();

    xSpell = LinguMgr::GetSpellChecker();
    pImpl = new SpellDialog_Impl;

    //HelpIds
    aClosePB.       SetHelpId(HID_SPLDLG_BUTTON_CLOSE    );
    aIgnorePB.      SetHelpId(HID_SPLDLG_BUTTON_IGNORE   );
    aIgnoreAllPB.   SetHelpId(HID_SPLDLG_BUTTON_IGNOREALL);
    aChangePB.      SetHelpId(HID_SPLDLG_BUTTON_CHANGE   );
    aChangeAllPB.   SetHelpId(HID_SPLDLG_BUTTON_CHANGEALL);

    Init_Impl();

    // disable controls if service is missing
    if (!xSpell.is())
        Enable( sal_False );

    Application::PostUserEvent( STATIC_LINK(
                        this, SpellDialog, InitHdl ) );
}

// -----------------------------------------------------------------------

SpellDialog::~SpellDialog()
{
    // save possibly modified user-dictionaries
    Reference< XDictionaryList >  xDicList( SvxGetDictionaryList() );
    if (xDicList.is())
    {
        linguistic::SaveDictionaries( xDicList );
    }

    delete aAddToDictMB.GetPopupMenu();
    delete pImpl;
}

// -----------------------------------------------------------------------

void SpellDialog::Init_Impl()
{
    // Handler initialisieren
    aClosePB.SetClickHdl(LINK( this, SpellDialog, CancelHdl ) );
    aChangePB.SetClickHdl(LINK( this, SpellDialog, ChangeHdl ) );
    aChangeAllPB.SetClickHdl(LINK( this, SpellDialog, ChangeAllHdl ) );
    aIgnorePB.SetClickHdl(LINK( this, SpellDialog, IgnoreHdl ) );
    aIgnoreAllPB.SetClickHdl(LINK( this, SpellDialog, IgnoreAllHdl ) );
    aUndoPB.SetClickHdl(LINK( this, SpellDialog, UndoHdl ) );

    aAutoCorrPB.SetClickHdl( LINK( this, SpellDialog, ExtClickHdl ) );
    aOptionsPB .SetClickHdl( LINK( this, SpellDialog, ExtClickHdl ) );

    aSuggestionLB.SetDoubleClickHdl( LINK( this, SpellDialog, ChangeHdl ) );

    aSentenceED.SetModifyHdl(LINK ( this, SpellDialog, ModifyHdl) );
    aAddToDictMB.SetSelectHdl(LINK ( this, SpellDialog, AddToDictionaryHdl ) );
    aLanguageLB.SetSelectHdl(LINK( this, SpellDialog, LanguageSelectHdl ) );
    // Save heading
    aTitel = GetText();

    // initialize language ListBox
    aLanguageLB.SetLanguageList( LANG_LIST_SPELL_USED, FALSE, FALSE, TRUE );

    // get current language
    UpdateBoxes_Impl();

    // fill dictionary PopupMenu
    InitUserDicts();

    aSentenceED.ClearModifyFlag();
    SvxGetChangeAllList()->clear();
}

// -----------------------------------------------------------------------

void SpellDialog::UpdateBoxes_Impl()
{
    sal_Int32 i;
    aSuggestionLB.Clear();

    Reference< XSpellAlternatives >  xAlt( aSentenceED.GetAlternatives(), UNO_QUERY );

    LanguageType nAltLanguage = LANGUAGE_NONE;
    String       aAltWord;
    Sequence< OUString >    aNewWords;
    if (xAlt.is())
    {
        nAltLanguage    = SvxLocaleToLanguage( xAlt->getLocale() );
        aAltWord        = String( xAlt->getWord() );
        aNewWords       = xAlt->getAlternatives();
    }


    String aStr( aTitel );
    aStr.Append( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( " (" ) ) );
    if (xAlt.is())
        aStr.Append( ::GetLanguageString( nAltLanguage ) );
    aStr.Append( sal_Unicode( ')' ) );
    SetText( aStr );

    SetSelectedLang_Impl( nAltLanguage );


    // Alternativen eintragen
    const OUString *pNewWords = aNewWords.getConstArray();
    const sal_Int32 nSize = aNewWords.getLength();
    for ( i = 0; i < nSize; ++i )
    {
        String aTmp( pNewWords[i] );
        if ( LISTBOX_ENTRY_NOTFOUND == aSuggestionLB.GetEntryPos( aTmp ) )
            aSuggestionLB.InsertEntry( aTmp );
    }
    if(!nSize)
        aSuggestionLB.InsertEntry( aNoSuggestionsST );
    aAutoCorrPB.Enable( nSize > 0 );
    //aSentenceED.GrabFocus();

    aSuggestionFT.Enable(nSize > 0);
    aSuggestionLB.Enable(nSize > 0);
    if( nSize )
    {
        aSuggestionLB.SelectEntryPos(0);
    }
    aChangePB.Enable( nSize > 0);
    aChangeAllPB.Enable(nSize > 0);
}
// -----------------------------------------------------------------------

void SpellDialog::SpellContinue_Impl(bool bUseSavedSentence)
{
    //initially or after the last error of a sentence MarkNextError will fail
    //then GetNextSentence() has to be called followed again by MarkNextError()
    //MarkNextError is not initally called if the UndoEdit mode is active
    if((!aSentenceED.IsUndoEditMode() && aSentenceED.MarkNextError()) ||
            GetNextSentence_Impl(bUseSavedSentence) && aSentenceED.MarkNextError())
    {
        Reference< XSpellAlternatives > xAlt = aSentenceED.GetAlternatives();
        if(xAlt.is())
        {
            UpdateBoxes_Impl();
            Control* aControls[] =
            {
                &aNotInDictFT,
                &aSentenceED,
                &aLanguageFT,
                &aLanguageLB,
                0
            };
            sal_Int32 nIdx = 0;
            do
            {
                aControls[nIdx]->Enable(sal_True);
            }
            while(aControls[++nIdx]);


        }
    }
}
/* -----------------10.09.2003 14:04-----------------
    Initialize, asynchronous to prevent virtial calls
    from a constructor
 --------------------------------------------------*/
IMPL_STATIC_LINK( SpellDialog, InitHdl, SpellDialog *, EMPTYARG )
{
    //show or hide AutoCorrect depending on the modules abilities
    pThis->aAutoCorrPB.Show(pThis->rParent.HasAutoCorrection());
    pThis->SpellContinue_Impl();
    pThis->aSentenceED.ResetUndo();
    pThis->aUndoPB.Enable(FALSE);

    pThis->LockFocusChanges(true);
    if( pThis->aChangePB.IsEnabled() )
        pThis->aChangePB.GrabFocus();
    else if( pThis->aIgnorePB.IsEnabled() )
        pThis->aIgnorePB.GrabFocus();
    else if( pThis->aClosePB.IsEnabled() )
        pThis->aClosePB.GrabFocus();
    pThis->LockFocusChanges(false);

    return 0;
};

// -----------------------------------------------------------------------

IMPL_LINK( SpellDialog, ExtClickHdl, Button *, pBtn )
{
    if (&aOptionsPB == pBtn)
        StartSpellOptDlg_Impl();
    else if(&aAutoCorrPB == pBtn)
    {
        //get the currently selected wrong word
        String sCurrentErrorText = aSentenceED.GetErrorText();
        //get the wrong word from the XSpellAlternative
        Reference< XSpellAlternatives > xAlt = aSentenceED.GetAlternatives();
        if(xAlt.is())
        {
            String sWrong(xAlt->getWord());
            //if the word has not been edited in the MultiLineEdit then
            //the current suggestion should be used
            //if it's not the 'no suggestions' entry
            if(sWrong == sCurrentErrorText &&
                    aSuggestionLB.IsEnabled() && aSuggestionLB.GetSelectEntryCount() > 0 &&
                    aNoSuggestionsST != aSuggestionLB.GetSelectEntry())
            {
                sCurrentErrorText = aSuggestionLB.GetSelectEntry();
            }
            if(sWrong != sCurrentErrorText)
            {
                SvxPrepareAutoCorrect( sWrong, sCurrentErrorText );
                LanguageType eLang = GetSelectedLang_Impl();
                rParent.AddAutoCorrection( sWrong, sCurrentErrorText, eLang );
            }
        }
    }
    return 0;
}
// -----------------------------------------------------------------------

void SpellDialog::StartSpellOptDlg_Impl()
{
    sal_uInt16 aSpellInfos[] =
    {
        SID_ATTR_SPELL,SID_ATTR_SPELL,
        SID_SPELL_MODIFIED, SID_SPELL_MODIFIED,
        SID_AUTOSPELL_CHECK, SID_AUTOSPELL_CHECK,
        SID_AUTOSPELL_MARKOFF, SID_AUTOSPELL_MARKOFF,
        0
    };
    SfxItemSet aSet( SFX_APP()->GetPool(), aSpellInfos);
    aSet.Put(SfxSpellCheckItem( xSpell, SID_ATTR_SPELL ));
    SfxSingleTabDialog* pDlg =
        new SfxSingleTabDialog( this, aSet, RID_SFXPAGE_LINGU );
    SfxTabPage* pPage = SvxLinguTabPage::Create( pDlg, aSet );
    ( (SvxLinguTabPage*)pPage )->HideGroups( GROUP_MODULES );
    pDlg->SetTabPage( pPage );
    if(RET_OK == pDlg->Execute())
    {

        // Benutzerb"ucher anzeigen
        InitUserDicts();
        const SfxItemSet* pOutSet = pDlg->GetOutputItemSet();
        if(pOutSet)
            OfaTreeOptionsDialog::ApplyLanguageOptions(*pOutSet);
    }
    delete pDlg;

}

// -----------------------------------------------------------------------

IMPL_LINK( SpellDialog, ChangeHdl, Button *, EMPTYARG )
{
    if(aSentenceED.IsUndoEditMode())
    {
        SpellContinue_Impl();
    }
    else
    {
        aSentenceED.UndoActionStart( SPELLUNDO_CHANGE_GROUP );
        String aString = aSentenceED.GetErrorText();
        //dots are sometimes part of the spelled word but they are not necessarily part of the replacement
        bool bDot = aString.Len() && aString.GetChar(aString.Len() - 1 ) == '.';
        if(aSuggestionLB.IsEnabled() &&
                aSuggestionLB.GetSelectEntryCount()>0 &&
                aNoSuggestionsST != aSuggestionLB.GetSelectEntry())
            aString = aSuggestionLB.GetSelectEntry();
        if(bDot && (!aString.Len() || aString.GetChar(aString.Len() - 1 ) != '.'))
            aString += '.';

        aSentenceED.ChangeMarkedWord(aString, GetSelectedLang_Impl());
        SpellContinue_Impl();
        bModified = false;
        aSentenceED.UndoActionEnd( SPELLUNDO_CHANGE_GROUP );
    }
    if(!aChangePB.IsEnabled())
        aIgnorePB.GrabFocus();
    return 1;
}


// -----------------------------------------------------------------------

IMPL_LINK( SpellDialog, ChangeAllHdl, Button *, EMPTYARG )
{
    aSentenceED.UndoActionStart( SPELLUNDO_CHANGE_GROUP );
    // change the current word first
    String aString = aSentenceED.GetErrorText();
    if(aSuggestionLB.IsEnabled() &&
            aSuggestionLB.GetSelectEntryCount()>0 &&
            aNoSuggestionsST != aSuggestionLB.GetSelectEntry())
        aString = aSuggestionLB.GetSelectEntry();

    LanguageType eLang = GetSelectedLang_Impl();

    // add new word to ChangeAll list
    String  aOldWord( aSentenceED.GetErrorText() );
    SvxPrepareAutoCorrect( aOldWord, aString );
    Reference<XDictionary> aXDictionary( SvxGetChangeAllList(), UNO_QUERY );
    sal_uInt8 nAdded = linguistic::AddEntryToDic( aXDictionary,
            aOldWord , sal_True,
            aString, eLang );

    if(nAdded == DIC_ERR_NONE)
    {
        SpellUndoAction_Impl* pAction = new SpellUndoAction_Impl(
                        SPELLUNDO_CHANGE_ADD_TO_DICTIONARY, aDialogUndoLink);
        pAction->SetDictionary(aXDictionary);
        pAction->SetAddedWord(aOldWord);
        aSentenceED.AddUndoAction(pAction);
    }

    aSentenceED.ChangeMarkedWord(aString, eLang);
    SpellContinue_Impl();
    bModified = false;
    aSentenceED.UndoActionEnd( SPELLUNDO_CHANGE_GROUP );
    return 1;
}
// -----------------------------------------------------------------------

IMPL_LINK( SpellDialog, IgnoreAllHdl, Button *, EMPTYARG )
{
    aSentenceED.UndoActionStart( SPELLUNDO_CHANGE_GROUP );
    // add word to IgnoreAll list
    Reference< XDictionary > aXDictionary( SvxGetIgnoreAllList(), UNO_QUERY );
    //in case the error has been changed manually it has to be restored
    aSentenceED.RestoreCurrentError();
    String sErrorText(aSentenceED.GetErrorText());
    sal_uInt8 nAdded = linguistic::AddEntryToDic( aXDictionary,
        sErrorText, sal_False,
        ::rtl::OUString(), LANGUAGE_NONE );
    if(nAdded == DIC_ERR_NONE)
    {
        SpellUndoAction_Impl* pAction = new SpellUndoAction_Impl(
                        SPELLUNDO_CHANGE_ADD_TO_DICTIONARY, aDialogUndoLink);
        pAction->SetDictionary(aXDictionary);
        pAction->SetAddedWord(sErrorText);
        aSentenceED.AddUndoAction(pAction);
    }

    SpellContinue_Impl();
    bModified = false;
    aSentenceED.UndoActionEnd( SPELLUNDO_CHANGE_GROUP );
    return 1;
}
/*-- 06.11.2003 11:24:08---------------------------------------------------

  -----------------------------------------------------------------------*/
IMPL_LINK( SpellDialog, UndoHdl, Button*, EMPTYARG )
{
    aSentenceED.Undo();
    if(!aSentenceED.GetUndoActionCount())
        aUndoPB.Enable(FALSE);
    return 0;
}
/*-- 06.11.2003 12:19:15---------------------------------------------------

  -----------------------------------------------------------------------*/
IMPL_LINK( SpellDialog, DialogUndoHdl, SpellUndoAction_Impl*, pAction )
{
    switch(pAction->GetId())
    {
        case SPELLUNDO_CHANGE_TEXTENGINE:
        {
            if(pAction->IsEnableChangePB())
                aChangePB.Enable(FALSE);
            if(pAction->IsEnableChangeAllPB())
                aChangeAllPB.Enable(FALSE);
        }
        break;
        case SPELLUNDO_CHANGE_NEXTERROR:
        {
            aSentenceED.MoveErrorMarkTo((USHORT)pAction->GetOldErrorStart(), (USHORT)pAction->GetOldErrorEnd());
            if(pAction->IsErrorLanguageSelected())
            {
                UpdateBoxes_Impl();
            }
        }
        break;
        case SPELLUNDO_CHANGE_ADD_TO_DICTIONARY:
        {
            if(pAction->GetDictionary().is())
                pAction->GetDictionary()->remove(pAction->GetAddedWord());
        }
        break;
        case SPELLUNDO_MOVE_ERROREND :
        {
            if(pAction->GetOffset() != 0)
                aSentenceED.MoveErrorEnd(pAction->GetOffset());
        }
        break;
        case SPELLUNDO_UNDO_EDIT_MODE :
        {
            //refill the dialog with the currently spelled sentence - throw away all changes
            SpellContinue_Impl(true);
        }
        break;
    }

    return 0;
}
// -----------------------------------------------------------------------

IMPL_LINK( SpellDialog, IgnoreHdl, Button *, EMPTYARG )
{
    if(aIgnorePB.GetText() == aResumeST)
    {
        //clear the "ChangeAllList"
        SvxGetChangeAllList()->clear();
        //get a new sentence
        aSentenceED.SetText(rtl::OUString());
        aSentenceED.ResetModified();
        SpellContinue_Impl();
        aIgnorePB.SetText(aIgnoreOnceST);
    }
    else
    {
        //in case the error has been changed manually it has to be restored
        aSentenceED.RestoreCurrentError();
        // the word is being ignored
        SpellContinue_Impl();
        bModified = false;
    }
    return 1;
}


// -----------------------------------------------------------------------

sal_Bool SpellDialog::Close()
{
    GetBindings().GetDispatcher()->
        Execute(SID_SPELL_DIALOG,
        SFX_CALLMODE_ASYNCHRON|SFX_CALLMODE_RECORD);
    return sal_True;
}
// -----------------------------------------------------------------------

void SpellDialog::SetSelectedLang_Impl( LanguageType nLang )
{
    aLanguageLB.SelectLanguage( nLang );
}

// -----------------------------------------------------------------------

LanguageType SpellDialog::GetSelectedLang_Impl() const
{
    INT16 nLang = aLanguageLB.GetSelectLanguage();
    return nLang;
}
/* -----------------28.10.2003 14:27-----------------

 --------------------------------------------------*/
IMPL_LINK(SpellDialog, LanguageSelectHdl, SvxLanguageBox*, pBox)
{
    //if currently an error is selected then search for alternatives for
    //this word and fill the alternatives ListBox accordingly
    String sError = aSentenceED.GetErrorText();
    aSuggestionLB.Clear();
    if(sError.Len())
    {
         aSentenceED.SetAlternatives(xSpell->spell( sError, pBox->GetSelectLanguage(),
                                            Sequence< PropertyValue >() ));

         aSentenceED.AddUndoAction(new SpellUndoAction_Impl(SPELLUNDO_CHANGE_LANGUAGE, aDialogUndoLink));
    }
    SpellDialog::UpdateBoxes_Impl();
    return 0;
}
// -----------------------------------------------------------------------

void SpellDialog::SetLanguage( sal_uInt16 nLang )

/*  [Beschreibung]

    wenn die Sprache im Thesaurus umgestellt wurde,
    muss auch hier die Sprache umgestellt werden.
*/

{
    String aStr( aTitel );
    aStr.Append( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( " (" ) ) );
    aStr.Append( ::GetLanguageString( (LanguageType)nLang ) );
    aStr.Append( sal_Unicode( ')' ) );;
    SetText( aStr );

    // den richtigen Eintrag finden, da sortiert
    aLanguageLB.SelectLanguage( nLang );
}
/*-------------------------------------------------------------------------

  -----------------------------------------------------------------------*/
void SpellDialog::InitUserDicts()
{
    sal_uInt16 nLang = aLanguageLB.GetSelectLanguage();

    const Reference< XDictionary >  *pDic = 0;

    // get list of dictionaries
    Reference< XDictionaryList >  xDicList( SvxGetDictionaryList() );
    if (xDicList.is())
    {
        // add active, positive dictionary to dic-list (if not already done).
        // This is to ensure that there is at least on dictionary to which
        // words could be added.
        Reference< XDictionary1 >  xDic( SvxGetOrCreatePosDic( xDicList ) );
        if (xDic.is())
            xDic->setActive( sal_True );

        pImpl->aDics = xDicList->getDictionaries();
    }

    // Benutzerbuecher anzeigen
    const sal_Int32 nSize = pImpl->aDics.getLength();
    pDic = pImpl->aDics.getConstArray();
    sal_Int32 i;
    delete aAddToDictMB.GetPopupMenu();
    PopupMenu* pMenu = new PopupMenu;
    for (i = 0; i < nSize; ++i )
    {
        Reference< XDictionary1 >  xDic( pDic[i], UNO_QUERY );
        if (!xDic.is() || SvxGetIgnoreAllList() == xDic)
            continue;

        // add only active and not read-only dictionaries to list
        // from which to choose from
        Reference< frame::XStorable > xStor( xDic, UNO_QUERY );
        if ( xDic->isActive()  &&  (!xStor.is() || !xStor->isReadonly()) )
        {
            sal_Bool bNegativ = xDic->getDictionaryType() == DictionaryType_NEGATIVE;
            pMenu->InsertItem( (USHORT)i + 1, ::GetDicInfoStr( xDic->getName(),
                                xDic->getLanguage(), bNegativ ) );
        }
    }
    aAddToDictMB.SetPopupMenu(pMenu);

    aAddToDictMB.Disable();

    sal_uInt16 k;
    for ( k = 0; k < pMenu->GetItemCount(); ++k )
    {
        sal_uInt16 nId = pMenu->GetItemId(k) - 1;
        sal_Bool bFound = sal_False;

        const sal_uInt16 nDicLang  = SvxLocaleToLanguage( pDic[nId]->getLocale() );
        const sal_Bool bDicNegativ =
            pDic[nId]->getDictionaryType() == DictionaryType_NEGATIVE;
        // Stimmt die Sprache "uberein, dann enable
        if ((nDicLang == nLang || nDicLang == LANGUAGE_NONE) && !bDicNegativ)
            bFound = sal_True;

        if (bFound)
        {
            aAddToDictMB.Enable();
            break;
        }
    }
}
/*-- 20.10.2003 15:31:06---------------------------------------------------

  -----------------------------------------------------------------------*/
IMPL_LINK(SpellDialog, AddToDictionaryHdl, MenuButton*, pButton )
{
    aSentenceED.UndoActionStart( SPELLUNDO_CHANGE_GROUP );
    USHORT nItem = pButton->GetCurItemId();

    //GetErrorText() returns the current error even if the text is already
    //manually changed
    String sNewWord= aSentenceED.GetErrorText();

    Reference< XDictionary >  xDic( pImpl->aDics.getConstArray()[ nItem - 1 ], UNO_QUERY );
    sal_Int16 nAddRes = DIC_ERR_UNKNOWN;
    if (xDic.is())
    {
        String sTmpTxt( sNewWord );
        sal_Bool bNegEntry = xDic->getDictionaryType() == DictionaryType_NEGATIVE;
        nAddRes = linguistic::AddEntryToDic( xDic, sTmpTxt, bNegEntry,
                OUString(), LANGUAGE_NONE );

        if(nAddRes == DIC_ERR_NONE)
        {
            SpellUndoAction_Impl* pAction = new SpellUndoAction_Impl(
                            SPELLUNDO_CHANGE_ADD_TO_DICTIONARY, aDialogUndoLink);
            pAction->SetDictionary(xDic);
            pAction->SetAddedWord(sTmpTxt);
            aSentenceED.AddUndoAction(pAction);
        }
        // failed because there is already an entry?
        if (DIC_ERR_NONE != nAddRes && xDic->getEntry( sTmpTxt ).is())
            nAddRes = DIC_ERR_NONE;
    }
    if (DIC_ERR_NONE != nAddRes)
    {
        SvxDicError( this, nAddRes );
        return 0;   // Nicht weitermachen
    }
    // nach dem Aufnehmen ggf. '='-Zeichen entfernen
    sNewWord.EraseAllChars( sal_Unicode( '=' ) );

    // go on
    SpellContinue_Impl();
    aSentenceED.UndoActionEnd( SPELLUNDO_CHANGE_GROUP );
    return 0;
}
/*-------------------------------------------------------------------------

  -----------------------------------------------------------------------*/
IMPL_LINK(SpellDialog, ModifyHdl, SentenceEditWindow_Impl*, pEd)
{
    if (&aSentenceED == pEd)
    {
        bModified = true;
        aSuggestionLB.SetNoSelection();
        aSuggestionLB.Disable();
        String sNewText( aSentenceED.GetText() );
        aAutoCorrPB.Enable( sNewText != aSentenceED.GetText() );
        SpellUndoAction_Impl* pSpellAction = new SpellUndoAction_Impl(SPELLUNDO_CHANGE_TEXTENGINE, aDialogUndoLink);
        if(!aChangeAllPB.IsEnabled())
        {
            aChangeAllPB.Enable();
            pSpellAction->SetEnableChangeAllPB();
        }
        if(!aChangePB.IsEnabled())
        {
            aChangePB.Enable();
            pSpellAction->SetEnableChangePB();
        }
        aSentenceED.AddUndoAction(pSpellAction);
    }
    return 0;
};
/*-------------------------------------------------------------------------

  -----------------------------------------------------------------------*/
IMPL_LINK(SpellDialog, CancelHdl, Button *, EMPTYARG )
{
    //apply changes first - if there are any
    if(aSentenceED.IsModified())
    {
        rParent.ApplyChangedSentence(aSentenceED.CreateSpellPortions());
    }
    Close();
    return 0;
}
/*-------------------------------------------------------------------------

  -----------------------------------------------------------------------*/
void SpellDialog::Paint( const Rectangle& rRect )
{
    ModelessDialog::Paint(rRect );
    Rectangle aRect(aBackgroundGB.GetPosPixel(), aBackgroundGB.GetSizePixel());
    DecorationView aDecoView( this );
    aDecoView.DrawButton( aRect, BUTTON_DRAW_NOFILL);
}
/*-- 28.10.2003 13:26:39---------------------------------------------------

  -----------------------------------------------------------------------*/
long SpellDialog::Notify( NotifyEvent& rNEvt )
{
    /* #i38338#
    *   FIXME: LoseFocus and GetFocus are signals from vcl that
    *   a window actually got/lost the focus, it never should be
    *   forwarded from another window, that is simply wrong.
    *   FIXME: overloading the virtual methods GetFocus and LoseFocus
    *   in SpellDialogChildWindow by making them pure is at least questionable.
    *   The only sensible thing would be to call the new Method differently,
    *   e.g. DialogGot/LostFocus or so.
    */
    if( IsVisible() && !bFocusLocked )
    {
        if( rNEvt.GetType() ==  EVENT_GETFOCUS )
        {
            //notify the child window of the focus change
            rParent.GetFocus();
        }
        else if( rNEvt.GetType() == EVENT_LOSEFOCUS )
        {
            //notify the child window of the focus change
            rParent.LoseFocus();
        }
    }
    return SfxModelessDialog::Notify(rNEvt);
}
/* -----------------10.09.2003 08:26-----------------

 --------------------------------------------------*/
void SpellDialog::InvalidateDialog()
{
    aIgnorePB.SetText(aResumeST);
    Window* aDisableArr[] =
            {
                &aNotInDictFT,
                &aSentenceED,
                &aSuggestionFT,
                &aSuggestionLB,
                &aLanguageFT,
                &aLanguageLB,
                &aIgnoreAllPB,
                &aAddToDictMB,
                &aChangePB,
                &aChangeAllPB,
                &aAutoCorrPB,
                &aUndoPB,
                0
            };
    sal_Int16 i = 0;
    while(aDisableArr[i])
    {
        aDisableArr[i]->Enable(FALSE);
        i++;
    }
    SfxModelessDialog::Deactivate();
}

/*-- 10.09.2003 08:35:56---------------------------------------------------

  -----------------------------------------------------------------------*/
bool SpellDialog::GetNextSentence_Impl(bool bUseSavedSentence)
{
    bool bRet = false;
    if(!bUseSavedSentence && aSentenceED.IsModified())
    {
        rParent.ApplyChangedSentence(aSentenceED.CreateSpellPortions());
    }
    aSentenceED.ResetModified();
    SpellPortions aSentence = bUseSavedSentence ? m_aSavedSentence : rParent.GetNextWrongSentence();
    if(!bUseSavedSentence)
        m_aSavedSentence = aSentence;
    bool bHasReplaced = false;
    while(aSentence.size())
    {
        //apply all changes that are already part of the "ChangeAllList"
        //returns true if the list still contains errors after the changes have been applied

        if(!ApplyChangeAllList_Impl(aSentence, bHasReplaced))
        {
            rParent.ApplyChangedSentence(aSentence);
            aSentence = rParent.GetNextWrongSentence();
        }
        else
            break;
    }

    if(aSentence.size())
    {
        SpellPortions::iterator aStart = aSentence.begin();
        rtl::OUString sText;
        while(aStart != aSentence.end())
        {
            // hidden text has to be ignored
            if(!aStart->bIsHidden)
                sText += aStart->sText;
            aStart++;
        }
        aSentenceED.SetText(sText);
        aStart = aSentence.begin();
        sal_Int32 nStartPosition = 0;
        sal_Int32 nEndPosition = 0;

        while(aStart != aSentence.end())
        {
            // hidden text has to be ignored
            if(!aStart->bIsHidden)
            {
                nEndPosition += aStart->sText.getLength();
                if(aStart->xAlternatives.is())
                    aSentenceED.SetAttrib( SpellErrorAttrib(aStart->xAlternatives), 0, (USHORT) nStartPosition, (USHORT) nEndPosition );
                if(aStart->bIsField)
                    aSentenceED.SetAttrib( SpellBackgroundAttrib(COL_LIGHTGRAY), 0, (USHORT) nStartPosition, (USHORT) nEndPosition );
                aSentenceED.SetAttrib( SpellLanguageAttrib(aStart->eLanguage), 0, (USHORT) nStartPosition, (USHORT) nEndPosition );
                nStartPosition = nEndPosition;
            }
            aStart++;
        }
        //the edit field needs to be modified to apply the change from the ApplyChangeAllList
        if(!bHasReplaced)
            aSentenceED.ClearModifyFlag();
        aSentenceED.ResetUndo();
        aUndoPB.Enable(FALSE);
        bRet = nStartPosition > 0;
    }
    return bRet;
}
/*-- 12.11.2003 15:21:25---------------------------------------------------
    replace errrors that have a replacement in the ChangeAllList
    returns false if the result doesn't contain errors after the replacement
  -----------------------------------------------------------------------*/
bool SpellDialog::ApplyChangeAllList_Impl(SpellPortions& rSentence, bool &bHasReplaced)
{
    bHasReplaced = false;
    bool bRet = true;
    SpellPortions::iterator aStart = rSentence.begin();
    Reference<XDictionary> xChangeAll( SvxGetChangeAllList(), UNO_QUERY );
    if(!xChangeAll->getCount())
        return bRet;
    bRet = false;
    while(aStart != rSentence.end())
    {
        if(aStart->xAlternatives.is())
        {
            Reference<XDictionaryEntry> xEntry = xChangeAll->getEntry( aStart->sText );
            if(xEntry.is())
            {
                aStart->sText = xEntry->getReplacementText();
                aStart->xAlternatives = 0;
                bHasReplaced = true;
            }
            else
                bRet = true;
        }
        aStart++;
    }
    return bRet;
}
/*-- 10.09.2003 10:40:21---------------------------------------------------

  -----------------------------------------------------------------------*/
SentenceEditWindow_Impl::SentenceEditWindow_Impl( SpellDialog* pParent, const ResId& rResId ) :
    MultiLineEdit( pParent, rResId ),
    m_nErrorStart(0),
    m_nErrorEnd(0),
    m_bIsUndoEditMode(false)
{
    DisableSelectionOnFocus();
}
/*-- 10.09.2003 10:40:11---------------------------------------------------

  -----------------------------------------------------------------------*/
SentenceEditWindow_Impl::~SentenceEditWindow_Impl()
{
}
/*-- 20.10.2003 13:42:34---------------------------------------------------
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

long SentenceEditWindow_Impl::PreNotify( NotifyEvent& rNEvt )
{
    bool bChange = false;
    const TextCharAttrib*  pErrorAttrib = 0;
    if(rNEvt.GetType() == EVENT_KEYINPUT)
    {
        const KeyEvent& rKeyEvt = *rNEvt.GetKeyEvent();
        bChange = TextEngine::DoesKeyChangeText( rKeyEvt );
        if(bChange && !IsUndoEditMode() &&
            rKeyEvt.GetKeyCode().GetCode() != KEY_TAB)
        {
            TextEngine* pTextEngine = GetTextEngine();
            TextView* pTextView = pTextEngine->GetActiveView();
            const TextSelection& rCurrentSelection = pTextView->GetSelection();
            //determine if the selection contains a field
            bool bHasField = false;
            bool bHasError = false;
            bool bHasFieldLeft = false;
            bool bHasErrorLeft = false;
//            bool bInsideAttr = false;

            bool bHasRange = rCurrentSelection.HasRange();
            sal_uInt8 nSelectionType = 0; // invalid type!

            TextPaM aCursor(rCurrentSelection.GetStart());
            const TextCharAttrib* pBackAttr = pTextEngine->FindCharAttrib( aCursor, TEXTATTR_SPELL_BACKGROUND );
            const TextCharAttrib* pErrorAttr = pTextEngine->FindCharAttrib( aCursor, TEXTATTR_SPELL_ERROR );
            const TextCharAttrib* pBackAttrLeft = 0;
            const TextCharAttrib* pErrorAttrLeft = 0;

            bHasField = pBackAttr != 0 && (bHasRange || pBackAttr->GetEnd() > aCursor.GetIndex());
            bHasError = pErrorAttr != 0 && (bHasRange || pErrorAttr->GetEnd() > aCursor.GetIndex());
            if(bHasRange)
            {
                if(pBackAttr &&
                        pBackAttr->GetStart() == rCurrentSelection.GetStart().GetIndex() &&
                        pBackAttr->GetEnd() == rCurrentSelection.GetEnd().GetIndex())
                {
                    nSelectionType = FULL;
                }
                else if(pErrorAttr &&
                        pErrorAttr->GetStart() <= rCurrentSelection.GetStart().GetIndex() &&
                        pErrorAttr->GetEnd() >= rCurrentSelection.GetEnd().GetIndex())
                {
                    nSelectionType = INSIDE_YES;
                }
                else
                {
                    nSelectionType = bHasField||bHasError ? BRACE : OUTSIDE_NO;
                    while(aCursor.GetIndex() < rCurrentSelection.GetEnd().GetIndex())
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
                        bHasField |= pIntBackAttr != 0;
                    }
                }
            }
            else
            {
                //no range selection: then 1 2 3 and 8 are possible
                const TextCharAttrib* pCurAttr = pBackAttr ? pBackAttr : pErrorAttr;
                if(pCurAttr)
                {
                    nSelectionType = pCurAttr->GetStart() == rCurrentSelection.GetStart().GetIndex() ?
                            LEFT_NO : pCurAttr->GetEnd() == rCurrentSelection.GetEnd().GetIndex() ? RIGHT_NO : INSIDE_NO;
                }
                else
                    nSelectionType = OUTSIDE_NO;

                bHasFieldLeft = pBackAttr && pBackAttr->GetEnd() == aCursor.GetIndex();
                if(bHasFieldLeft)
                {
                    pBackAttrLeft = pBackAttr;
                    pBackAttr = 0;
                }
                bHasErrorLeft = pErrorAttr && pErrorAttr->GetEnd() == aCursor.GetIndex();
                if(bHasErrorLeft)
                {
                    pErrorAttrLeft = pErrorAttr;
                    pErrorAttr = 0;
                }

                //check previous position if this exists
                //that is a redundant in the case the the attribute found above already is on the left cursor side
                //but it's o.k. for two errors/fields side by side
                if(aCursor.GetIndex())
                {
                    --aCursor.GetIndex();
                    pBackAttrLeft = pTextEngine->FindCharAttrib( aCursor, TEXTATTR_SPELL_BACKGROUND );
                    pErrorAttrLeft = pTextEngine->FindCharAttrib( aCursor, TEXTATTR_SPELL_ERROR );
                    bHasFieldLeft = pBackAttrLeft !=0;
                    bHasErrorLeft = pErrorAttrLeft != 0;
//                    bInsideAttr = (bHasField || bHasError) && (bHasFieldLeft || bHasErrorLeft);
                    ++aCursor.GetIndex();
                }
            }
            //Here we have to determine if the error found is the one currently active
            bool bIsErrorActive = pErrorAttr && pErrorAttr->GetStart() == m_nErrorStart ||
                    pErrorAttrLeft && pErrorAttrLeft->GetStart() == m_nErrorStart;

            DBG_ASSERT(nSelectionType != INVALID, "selection type not set!")

            const KeyCode& rKeyCode = rKeyEvt.GetKeyCode();
            bool bDelete = rKeyCode.GetCode() == KEY_DELETE;
            bool bBackspace = rKeyCode.GetCode() == KEY_BACKSPACE;

            sal_Int8 nAction = ACTION_CONTINUE;
//            nAction = ACTION_UNDOEDIT
//            nAction = ACTION_SELECTFIELD
//            nAction = ACTION_EXPAND
            switch(nSelectionType)
            {
//    1 - backspace                   delete                      any other
//        UE                          on field FS on error CO     on field FS on error CO
                case LEFT_NO    :
                    if(bBackspace)
                    {
                        nAction = bHasFieldLeft ? ACTION_SELECTFIELD : ACTION_UNDOEDIT;
                        //to force the use of pBackAttrLeft
                        pBackAttr = 0;
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
                    nAction = bHasField ? ACTION_UNDOEDIT : ACTION_CONTINUE;
                break;
//    5 - on field FS and on error CO
                case INSIDE_YES :
                    nAction = bHasField ? ACTION_SELECTFIELD : ACTION_CONTINUE;
                break;
//    6 - on field FS and on error UE
                case BRACE      :
                    nAction = bHasField ? ACTION_SELECTFIELD : ACTION_UNDOEDIT;;
                break;
//    7 - UE
//    8 - UE
                case OUTSIDE_NO :
                case OUTSIDE_YES:
                    nAction = ACTION_UNDOEDIT;
                break;
            }
            //save the current paragraph
            USHORT nCurrentLen = GetText().Len();
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
                DBG_ASSERT(pErrorAttrLeft || pErrorAttr, "where is the error")
                //text has been added on the right and only the 'error attribute has to be corrected
                if(pErrorAttrLeft)
                {
                    TextAttrib* pNewError =  pErrorAttrLeft->GetAttr().Clone();
                    USHORT nStart = pErrorAttrLeft->GetStart();
                    USHORT nEnd = pErrorAttrLeft->GetEnd();
                    pTextEngine->RemoveAttrib( 0, *pErrorAttrLeft );
                    SetAttrib( *pNewError, 0, nStart, ++nEnd );
                    //only active errors move the mark
                    if(bIsErrorActive)
                        MoveErrorMarkTo(nStart, nEnd);
                    delete pNewError;
                }
                //text has been added on the left then the error attribute has to be expanded and the
                //field attribute on the right - if any - has to be contracted
                else if(pErrorAttr)
                {
                    //determine the change
                    USHORT nAddedChars = GetText().Len() - nCurrentLen;

                    TextAttrib* pNewError =  pErrorAttr->GetAttr().Clone();
                    USHORT nStart = pErrorAttr->GetStart();
                    USHORT nEnd = pErrorAttr->GetEnd();
                    pTextEngine->RemoveAttrib( 0, *pErrorAttr );
                    nStart = nStart - (USHORT)nAddedChars;
                    SetAttrib( *pNewError, 0, nStart - nAddedChars, nEnd );
                    //only if the error is active the mark is moved here
                    if(bIsErrorActive)
                        MoveErrorMarkTo(nStart, nEnd);
                    delete pNewError;

                    if(pBackAttrLeft)
                    {
                        TextAttrib* pNewBack =  pBackAttrLeft->GetAttr().Clone();
                        USHORT _nStart = pBackAttrLeft->GetStart();
                        USHORT _nEnd = pBackAttrLeft->GetEnd();
                        pTextEngine->RemoveAttrib( 0, *pBackAttrLeft );
                        SetAttrib( *pNewBack, 0, _nStart, _nEnd - nAddedChars);
                        delete pNewBack;
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
                pErrorAttrib = pTextEngine->FindCharAttrib( TextPaM(0, m_nErrorStart), TEXTATTR_SPELL_ERROR );
                if(pFontColor && pErrorAttrib )
                {
                    m_nErrorStart = pFontColor->GetStart();
                    m_nErrorEnd = pFontColor->GetEnd();
                    if(pErrorAttrib->GetStart() != m_nErrorStart || pErrorAttrib->GetEnd() != m_nErrorEnd)
                    {
                        TextAttrib* pNewError =  pErrorAttrib->GetAttr().Clone();
                        pTextEngine->RemoveAttrib( 0, *pErrorAttr );
                        SetAttrib( *pNewError, 0, m_nErrorStart, m_nErrorEnd );
                        delete pNewError;
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
    long nRet = bChange ? 1 : MultiLineEdit::PreNotify(rNEvt);
    return nRet;
}
/*-- 10.09.2003 13:38:14---------------------------------------------------

  -----------------------------------------------------------------------*/
bool SentenceEditWindow_Impl::MarkNextError()
{
    ExtTextEngine* pTextEngine = GetTextEngine();
    USHORT nTextLen = pTextEngine->GetTextLen(0);
    if(m_nErrorEnd >= nTextLen - 1)
        return false;
    //if it's not already modified the modified flag has to be reset at the and of the marking
    bool bModified = IsModified();
    bool bRet = false;
    const USHORT nOldErrorStart = m_nErrorStart;
    const USHORT nOldErrorEnd   = m_nErrorEnd;

    //create a cursor behind the end of the last error
    //- or at 0 at the start of the sentence
    TextPaM aCursor(0, m_nErrorEnd ? m_nErrorEnd + 1 : 0);
    //search for SpellErrorAttrib

    const TextCharAttrib* pNextError = 0;
    //iterate over the text and search for the next error that maybe has
    //to be replace by a ChangeAllList replacement
    while(aCursor.GetIndex() < nTextLen)
    {
        while(aCursor.GetIndex() < nTextLen &&
                0 == (pNextError = pTextEngine->FindCharAttrib( aCursor, TEXTATTR_SPELL_ERROR)))
        {
            ++aCursor.GetIndex();
        }
        // maybe the error found here is already in the ChangeAllList and has to be replaced

        Reference<XDictionary> xChangeAll( SvxGetChangeAllList(), UNO_QUERY );
        Reference<XDictionaryEntry> xEntry;

        Reference <XSpellAlternatives> xAlternatives;
        if(pNextError)
            xAlternatives = static_cast<const SpellErrorAttrib&>(pNextError->GetAttr()).GetAlternatives();
        if(xChangeAll->getCount() && xAlternatives.is() &&
                (xEntry = xChangeAll->getEntry( xAlternatives->getWord() )).is())
        {
            m_nErrorStart = pNextError->GetStart();
            m_nErrorEnd = pNextError->GetEnd();
            ChangeMarkedWord(xEntry->getReplacementText(),
                    SvxLocaleToLanguage( xAlternatives->getLocale() ));
            aCursor.GetIndex() = aCursor.GetIndex() + (USHORT)(xEntry->getReplacementText().getLength());
        }
        else
            break;
    }

    //if an attrib has been found search for the end of the error string
    if(aCursor.GetIndex() < nTextLen)
    {
        m_nErrorStart = aCursor.GetIndex();
        m_nErrorEnd = pNextError->GetEnd();
        MoveErrorMarkTo(m_nErrorStart, m_nErrorEnd);
        bRet = true;
        //add an undo action
        SpellUndoAction_Impl* pAction = new SpellUndoAction_Impl(
                SPELLUNDO_CHANGE_NEXTERROR, GetSpellDialog()->aDialogUndoLink);
        pAction->SetErrorMove(m_nErrorStart, m_nErrorEnd, nOldErrorStart, nOldErrorEnd);
        const SpellErrorAttrib* pOldAttrib = static_cast<const SpellErrorAttrib*>(
                pTextEngine->FindAttrib( TextPaM(0, nOldErrorStart), TEXTATTR_SPELL_ERROR ));
        pAction->SetErrorLanguageSelected(pOldAttrib && pOldAttrib->GetAlternatives().is() &&
                SvxLocaleToLanguage( pOldAttrib->GetAlternatives()->getLocale()) ==
                                        GetSpellDialog()->aLanguageLB.GetSelectLanguage());
        AddUndoAction(pAction);
    }
    else
        m_nErrorStart = m_nErrorEnd = nTextLen;
    if( !bModified )
        ClearModifyFlag();
    SpellDialog* pSpellDialog = GetSpellDialog();
    pSpellDialog->aIgnorePB.Enable(bRet);
    pSpellDialog->aIgnoreAllPB.Enable(bRet);
    pSpellDialog->aAutoCorrPB.Enable(bRet);
    pSpellDialog->aAddToDictMB.Enable(bRet);
    return bRet;
}

/*-- 06.11.2003 13:30:26---------------------------------------------------

  -----------------------------------------------------------------------*/
void SentenceEditWindow_Impl::MoveErrorMarkTo(USHORT nStart, USHORT nEnd)
{
    TextEngine* pTextEngine = GetTextEngine();
    pTextEngine->RemoveAttribs( 0, (USHORT)TEXTATTR_FONTCOLOR, TRUE );
    pTextEngine->RemoveAttribs( 0, (USHORT)TEXTATTR_FONTWEIGHT, TRUE );
    pTextEngine->SetAttrib( TextAttribFontWeight(WEIGHT_BOLD), 0, nStart, nEnd );
    pTextEngine->SetAttrib( TextAttribFontColor(COL_LIGHTRED), 0, nStart, nEnd );
    m_nErrorStart = nStart;
    m_nErrorEnd = nEnd;
}

/*-- 17.09.2003 10:13:08---------------------------------------------------

  -----------------------------------------------------------------------*/
void SentenceEditWindow_Impl::ChangeMarkedWord(const String& rNewWord, LanguageType eLanguage)
{
    //calculate length changes
    long nDiffLen = rNewWord.Len() - m_nErrorEnd + m_nErrorStart;
    TextSelection aSel(TextPaM(0, m_nErrorStart), TextPaM(0, m_nErrorEnd));
    //Remove spell errror attribute
    ExtTextEngine* pTextEngine = GetTextEngine();
    pTextEngine->UndoActionStart( TEXTUNDO_INSERT );
    const TextCharAttrib*  pErrorAttrib = pTextEngine->FindCharAttrib( TextPaM(0, m_nErrorStart), TEXTATTR_SPELL_ERROR );
    DBG_ASSERT(pErrorAttrib, "no error attribute found")
    Reference <XSpellAlternatives> xAlternatives;
    if(pErrorAttrib)
    {
        pTextEngine->RemoveAttrib(0, *pErrorAttrib);
        xAlternatives = static_cast<const SpellErrorAttrib&>(pErrorAttrib->GetAttr()).GetAlternatives();
    }
    const TextCharAttrib*  pBackAttrib = pTextEngine->FindCharAttrib( TextPaM(0, m_nErrorStart), TEXTATTR_SPELL_BACKGROUND );
    pTextEngine->ReplaceText( aSel, rNewWord );
    //
    if(!m_nErrorStart)
    {
        //attributes following an error at the start of the text are not moved but expanded from the
        //text engine - this is done to keep full-paragraph-attributes
        //in the current case that handling is not desired
        const TextCharAttrib*  pLangAttrib =
                pTextEngine->FindCharAttrib(
                    TextPaM(0, m_nErrorEnd), TEXTATTR_SPELL_LANGUAGE );
        USHORT nTextLen = pTextEngine->GetTextLen( 0 );
        if(pLangAttrib && !pLangAttrib->GetStart() && pLangAttrib->GetEnd() ==
            nTextLen)
        {
            SpellLanguageAttrib aNewLangAttrib( static_cast<const SpellLanguageAttrib&>(pLangAttrib->GetAttr()).GetLanguage());
            pTextEngine->RemoveAttrib(0, *pLangAttrib);
            pTextEngine->SetAttrib( aNewLangAttrib, 0, (USHORT)(m_nErrorEnd + nDiffLen) , nTextLen );
        }
    }
    // undo expanded attributes!
    if( pBackAttrib && pBackAttrib->GetStart() < m_nErrorStart && pBackAttrib->GetEnd() == m_nErrorEnd + nDiffLen)
    {
        TextAttrib* pNewBackground = pBackAttrib->GetAttr().Clone();
        USHORT nStart = pBackAttrib->GetStart();
        pTextEngine->RemoveAttrib(0, *pBackAttrib);
        pTextEngine->SetAttrib(*pNewBackground, 0, nStart, m_nErrorStart);
        delete pNewBackground;
    }
    pTextEngine->SetModified(TRUE);

    //adjust end position
    long nEndTemp = m_nErrorEnd;
    nEndTemp += nDiffLen;
    m_nErrorEnd = (USHORT)nEndTemp;

    SpellUndoAction_Impl* pAction = new SpellUndoAction_Impl(
                    SPELLUNDO_MOVE_ERROREND, GetSpellDialog()->aDialogUndoLink);
    pAction->SetOffset(nDiffLen);
    AddUndoAction(pAction);
    SetAttrib( SpellErrorAttrib(xAlternatives), 0, m_nErrorStart, m_nErrorEnd );
    SetAttrib( SpellLanguageAttrib(eLanguage), 0, m_nErrorStart, m_nErrorEnd );
    pTextEngine->UndoActionEnd( TEXTUNDO_INSERT );
}
/* -----------------08.10.2003 13:18-----------------

 --------------------------------------------------*/
String SentenceEditWindow_Impl::GetErrorText() const
{
    return GetTextEngine()->GetText(TextSelection(TextPaM(0, m_nErrorStart), TextPaM(0, m_nErrorEnd) ));
}
/*-- 10.09.2003 13:38:14---------------------------------------------------

  -----------------------------------------------------------------------*/
Reference<XSpellAlternatives> SentenceEditWindow_Impl::GetAlternatives()
{
    TextPaM aCursor(0, m_nErrorStart);
    const SpellErrorAttrib* pAttrib = static_cast<const SpellErrorAttrib*>(
            GetTextEngine()->FindAttrib( aCursor, TEXTATTR_SPELL_ERROR));
    Reference <XSpellAlternatives> xRet;
    if(pAttrib)
        xRet = pAttrib->GetAlternatives();
    return xRet;
}
/*-- 06.09.2004 10:50:32---------------------------------------------------

  -----------------------------------------------------------------------*/
void SentenceEditWindow_Impl::RestoreCurrentError()
{
    TextPaM aCursor(0, m_nErrorStart);
    const SpellErrorAttrib* pAttrib = static_cast<const SpellErrorAttrib*>(
            GetTextEngine()->FindAttrib( aCursor, TEXTATTR_SPELL_ERROR));
    Reference <XSpellAlternatives> xRet;
    Reference<XSpellAlternatives> xAlt = pAttrib ? pAttrib->GetAlternatives() : 0;
    if(xAlt.is())
    {
        String sError = xAlt->getWord();
        if( GetErrorText() != sError )
            ChangeMarkedWord(sError, SvxLocaleToLanguage( xAlt->getLocale() ));
    }
}
/*-- 28.10.2003 14:44:10---------------------------------------------------

  -----------------------------------------------------------------------*/
void SentenceEditWindow_Impl::SetAlternatives( Reference< XSpellAlternatives> xAlt )
{
    TextPaM aCursor(0, m_nErrorStart);
    DBG_ASSERT(static_cast<const SpellErrorAttrib*>(
            GetTextEngine()->FindAttrib( aCursor, TEXTATTR_SPELL_ERROR)), "no error set?")
    GetTextEngine()->SetAttrib( SpellErrorAttrib(xAlt), 0, m_nErrorStart, m_nErrorEnd );
}

/*-- 10.09.2003 14:43:02---------------------------------------------------

  -----------------------------------------------------------------------*/
void SentenceEditWindow_Impl::SetAttrib( const TextAttrib& rAttr, ULONG nPara, USHORT nStart, USHORT nEnd )
{
    GetTextEngine()->SetAttrib(rAttr, nPara, nStart, nEnd);
}
/*-- 10.09.2003 14:43:02---------------------------------------------------

  -----------------------------------------------------------------------*/
void SentenceEditWindow_Impl::SetText( const String& rStr )
{
    m_nErrorStart = m_nErrorEnd = 0;
    GetTextEngine()->SetText(rStr);
//    InitScrollBars();
}
/*-- 08.10.2003 14:35:52---------------------------------------------------

  -----------------------------------------------------------------------*/
struct LanguagePosition_Impl
{
    USHORT          nPosition;
    LanguageType    eLanguage;

    LanguagePosition_Impl(USHORT nPos, LanguageType eLang) :
        nPosition(nPos),
        eLanguage(eLang)
        {}
};
typedef std::vector<LanguagePosition_Impl> LanguagePositions_Impl;

void lcl_InsertBreakPosition_Impl(
        LanguagePositions_Impl& rBreakPositions, USHORT nInsert, LanguageType eLanguage)
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
    rBreakPositions.push_back(LanguagePosition_Impl(nInsert, eLanguage));
}
/*-- 17.09.2003 14:26:59---------------------------------------------------
    Returns the text in spell portions. Each portion contains text with an
    equal language and attribute. The spell alternatives are empty.
  -----------------------------------------------------------------------*/
svx::SpellPortions SentenceEditWindow_Impl::CreateSpellPortions() const
{
    svx::SpellPortions aRet;
    ExtTextEngine* pTextEngine = GetTextEngine();
    const USHORT nTextLen = pTextEngine->GetTextLen(0);
    if(nTextLen)
    {
        TextPaM aCursor(0, 0);
        LanguagePositions_Impl aBreakPositions;
        const TextCharAttrib* pLastLang = 0;
        const TextCharAttrib* pLastError = 0;
        LanguageType eLang = LANGUAGE_DONTKNOW;
        const TextCharAttrib* pError = 0;
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
            aCursor.GetIndex()++;
        }
        //
        if(nTextLen && aBreakPositions.empty())
        {
            //if all content has been overwritten the attributes may have been removed, too
            svx::SpellPortion aPortion1;
            aPortion1.eLanguage = GetSpellDialog()->GetSelectedLang_Impl();
            aPortion1.sText = pTextEngine->GetText(
                        TextSelection(TextPaM(0, 0), TextPaM(0, nTextLen)));

            aRet.push_back(aPortion1);

        }
        else if(!aBreakPositions.empty())
        {
            LanguagePositions_Impl::iterator aStart = aBreakPositions.begin();
            //start should always be Null
            eLang = aStart->eLanguage;
            USHORT nStart = aStart->nPosition;
            DBG_ASSERT(!nStart, "invalid start position - language attribute missing?")
            ++aStart;

            while(aStart != aBreakPositions.end())
            {
                svx::SpellPortion aPortion1;
                aPortion1.eLanguage = eLang;
                aPortion1.sText = pTextEngine->GetText(
                            TextSelection(TextPaM(0, nStart), TextPaM(0, aStart->nPosition)));

                aRet.push_back(aPortion1);
                nStart = aStart->nPosition;
                eLang = aStart->eLanguage;
                ++aStart;
            }
        }

        // quick partly fix of #i71318. Correct fix needs to patch the TextEngine itself...
        // this one will only prevent text from disappearing. It may to not have the
        // correct language and will probably not spell checked...
        ULONG nPara = pTextEngine->GetParagraphCount();
        if (nPara > 1)
        {
            String aLeftOverText;
            for (ULONG i = 1;  i < nPara;  ++i)
            {
                aLeftOverText.AppendAscii( "\x0a" );    // the manual line break...
                aLeftOverText += pTextEngine->GetText(i);
            }
            if (pError)
            {   // we need to add a new portion containing the left-over text
                svx::SpellPortion aPortion2;
                aPortion2.eLanguage = eLang;
                aPortion2.sText = aLeftOverText;
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

/*-- 06.11.2003 11:30:10---------------------------------------------------

  -----------------------------------------------------------------------*/
void SentenceEditWindow_Impl::Undo()
{
    SfxUndoManager& rUndoMgr = GetTextEngine()->GetUndoManager();
    DBG_ASSERT(GetUndoActionCount(), "no undo actions available" )
    if(!GetUndoActionCount())
        return;
    bool bSaveUndoEdit = IsUndoEditMode();
    USHORT nId;
    //if the undo edit mode is active then undo all changes until the UNDO_EDIT_MODE action has been found
    do
    {
        nId = rUndoMgr.GetUndoActionId();
        rUndoMgr.Undo();
    }while(bSaveUndoEdit && SPELLUNDO_UNDO_EDIT_MODE != nId && GetUndoActionCount());

    if(bSaveUndoEdit || SPELLUNDO_CHANGE_GROUP == nId)
        GetSpellDialog()->UpdateBoxes_Impl();
}
/*-- 06.11.2003 11:30:10---------------------------------------------------

  -----------------------------------------------------------------------*/
void SentenceEditWindow_Impl::ResetUndo()
{
    GetTextEngine()->ResetUndo();
}
/*-- 06.11.2003 12:30:41---------------------------------------------------

  -----------------------------------------------------------------------*/
void SentenceEditWindow_Impl::AddUndoAction( SfxUndoAction *pAction, BOOL bTryMerg )
{
    SfxUndoManager& rUndoMgr = GetTextEngine()->GetUndoManager();
    rUndoMgr.AddUndoAction(pAction, bTryMerg);
    GetSpellDialog()->aUndoPB.Enable();
}
/*-- 06.11.2003 12:38:44---------------------------------------------------

  -----------------------------------------------------------------------*/
USHORT SentenceEditWindow_Impl::GetUndoActionCount()
{
    return GetTextEngine()->GetUndoManager().GetUndoActionCount();
}

/*-- 12.11.2003 12:12:38---------------------------------------------------

  -----------------------------------------------------------------------*/
void SentenceEditWindow_Impl::UndoActionStart( USHORT nId )
{
    GetTextEngine()->UndoActionStart(nId);
}
/*-- 12.11.2003 12:12:38---------------------------------------------------

  -----------------------------------------------------------------------*/
void SentenceEditWindow_Impl::UndoActionEnd( USHORT nId )
{
    GetTextEngine()->UndoActionEnd(nId);
}
/*-- 12.11.2003 12:12:38---------------------------------------------------

  -----------------------------------------------------------------------*/
void SentenceEditWindow_Impl::MoveErrorEnd(long nOffset)
{
    if(nOffset > 0)
        m_nErrorEnd = m_nErrorEnd - (USHORT)nOffset;
    else
        m_nErrorEnd = m_nErrorEnd -(USHORT)- nOffset;
}
/*-- 13.11.2003 15:15:19---------------------------------------------------

  -----------------------------------------------------------------------*/
void  SentenceEditWindow_Impl::SetUndoEditMode(bool bSet)
{
    DBG_ASSERT(!bSet || m_bIsUndoEditMode != bSet, "SetUndoEditMode with equal values?")
    m_bIsUndoEditMode = bSet;
    //disable all buttons except the Change
    SpellDialog* pSpellDialog = GetSpellDialog();
    Control* aControls[] =
    {
        &pSpellDialog->aChangeAllPB,
        &pSpellDialog->aIgnoreAllPB,
        &pSpellDialog->aIgnorePB,
        &pSpellDialog->aSuggestionLB,
        &pSpellDialog->aSuggestionFT,
        &pSpellDialog->aLanguageFT,
        &pSpellDialog->aLanguageLB,
        &pSpellDialog->aAddToDictMB,
        &pSpellDialog->aAutoCorrPB,
        0
    };
    sal_Int32 nIdx = 0;
    do
    {
        aControls[nIdx]->Enable(sal_False);
    }
    while(aControls[++nIdx]);

    //remove error marks
    TextEngine* pTextEngine = GetTextEngine();
    pTextEngine->RemoveAttribs( 0, (USHORT)TEXTATTR_FONTCOLOR, TRUE );
    pTextEngine->RemoveAttribs( 0, (USHORT)TEXTATTR_FONTWEIGHT, TRUE );

    //put the appropriate action on the Undo-stack
    SpellUndoAction_Impl* pAction = new SpellUndoAction_Impl(
                        SPELLUNDO_UNDO_EDIT_MODE, GetSpellDialog()->aDialogUndoLink);
    AddUndoAction(pAction);
    pSpellDialog->aChangePB.Enable();
}


