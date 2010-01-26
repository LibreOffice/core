/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: thesdlg.cxx,v $
 * $Revision: 1.21 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <svx/thesdlg.hxx>
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>
#include <svx/langbox.hxx>
#include <svx/checklbx.hxx>
#include <dlgutil.hxx>
#include <svxerr.hxx>
#include <unolingu.hxx>
#include "thesdlg_impl.hxx"
#include "thesdlg.hrc"

#include <com/sun/star/linguistic2/XThesaurus.hpp>
#include <com/sun/star/linguistic2/XMeaning.hpp>

#include <tools/shl.hxx>
#include <svl/lngmisc.hxx>
#include <svtools/svlbitm.hxx>
#include <svtools/svtreebx.hxx>
#include <svtools/langtab.hxx>
#include <i18npool/mslangid.hxx>


#include <stack>
#include <algorithm>

using namespace ::com::sun::star;
using ::rtl::OUString;


#define A2S(x)          String::CreateFromAscii( x )



// GetReplaceEditString -------------------------------

static void GetReplaceEditString( String &rText )
{
    // The strings returned by the thesaurus saometimes have some
    // explanation text put in between '(' and ')' or a trailing '*'.
    // These parts should not be put in the ReplaceEdit Text that may get
    // inserted into the document. Thus we strip them from the text.

    xub_StrLen nPos = rText.Search( sal_Unicode('(') );
    while (STRING_NOTFOUND != nPos)
    {
        xub_StrLen nEnd = rText.Search( sal_Unicode(')'), nPos );
        if (STRING_NOTFOUND != nEnd)
            rText.Erase( nPos, nEnd-nPos+1 );
        else
            break;
        nPos = rText.Search( sal_Unicode('(') );
    }

    nPos = rText.Search( sal_Unicode('*') );
    if (STRING_NOTFOUND != nPos)
        rText.Erase( nPos );

    // remove any possible remaining ' ' that may confuse the thesaurus
    // when it gets called with the text
    rText.EraseLeadingAndTrailingChars( sal_Unicode(' ') );
}

// class LookUpComboBox --------------------------------------------------

LookUpComboBox::LookUpComboBox(
    Window *pParent, const ResId &rResId ) :
    ComboBox        (pParent, rResId)
{
}


LookUpComboBox::~LookUpComboBox()
{
}


void LookUpComboBox::Modify()
{
    if (m_pBtn)
        m_pBtn->Enable( GetText().Len() > 0 );
}

void LookUpComboBox::SetText( const XubString& rStr )
{
    ComboBox::SetText( rStr );
    Modify();
}


void LookUpComboBox::SetText( const XubString& rStr, const Selection& rNewSelection )
{
    ComboBox::SetText( rStr, rNewSelection );
    Modify();
}

// class ThesaurusAlternativesCtrl_Impl ----------------------------------

void AlternativesString_Impl::Paint(
    const Point& rPos,
    SvLBox& rDev, USHORT,
    SvLBoxEntry* pEntry )
{
    AlternativesUserData_Impl* pData = (AlternativesUserData_Impl*)pEntry->GetUserData();
    Point aPos( rPos );
    Font aOldFont( rDev.GetFont());
    if (pData && pData->IsHeader())
    {
        Font aFont( aOldFont );
        aFont.SetWeight( WEIGHT_BOLD );
        rDev.SetFont( aFont );
        aPos.X() = 0;
    }
    else
        aPos.X() += 5;
    rDev.DrawText( aPos, GetText() );
    rDev.SetFont( aOldFont );
}


ThesaurusAlternativesCtrl_Impl::ThesaurusAlternativesCtrl_Impl(
        Window* pParent,
        SvxThesaurusDialog_Impl &rImpl ) :
    SvxCheckListBox( pParent, SVX_RES( CT_THES_ALTERNATIVES ) ),
    m_rDialogImpl( rImpl )
{
    SetWindowBits( WB_CLIPCHILDREN | WB_HSCROLL | WB_FORCE_MAKEVISIBLE );
    SetHighlightRange();
}


ThesaurusAlternativesCtrl_Impl::~ThesaurusAlternativesCtrl_Impl()
{
    ClearUserData();
}


void ThesaurusAlternativesCtrl_Impl::ClearUserData()
{
    for (USHORT i = 0; i < GetEntryCount(); ++i)
        delete (AlternativesUserData_Impl*)GetEntry(i)->GetUserData();
}


SvLBoxEntry * ThesaurusAlternativesCtrl_Impl::AddEntry( sal_Int32 nVal, const String &rText, bool bIsHeader )
{
    SvLBoxEntry* pEntry = new SvLBoxEntry;
    String aText;
    if (bIsHeader)
    {
        aText = String::CreateFromInt32( nVal );
        aText += A2S( ". " );
    }
    pEntry->AddItem( new SvLBoxString( pEntry, 0, String() ) ); // add empty column
    aText += rText;
    pEntry->AddItem( new SvLBoxContextBmp( pEntry, 0, Image(), Image(), 0 ) );  // otherwise crash
    pEntry->AddItem( new AlternativesString_Impl( pEntry, 0, aText ) );

    AlternativesUserData_Impl* pUserData = new AlternativesUserData_Impl( rText, bIsHeader );
    pEntry->SetUserData( pUserData );
    GetModel()->Insert( pEntry );

    return pEntry;
}


void ThesaurusAlternativesCtrl_Impl::KeyInput( const KeyEvent& rKEvt )
{
    const KeyCode& rKey = rKEvt.GetKeyCode();

    if (rKey.GetCode() == KEY_RETURN || rKey.GetCode() == KEY_ESCAPE)
        GetParent()->KeyInput( rKEvt ); // parent will close dialog...
    else if (rKey.GetCode() == KEY_SPACE)
        m_rDialogImpl.AlternativesDoubleClickHdl_Impl( this ); // look up current selected entry
    else if (GetEntryCount())
        SvxCheckListBox::KeyInput( rKEvt );
}


// struct SvxThesaurusDialog_Impl ----------------------------------------

SvxThesaurusDialog_Impl::SvxThesaurusDialog_Impl( Window* pParent ) :
    m_pParent       ( pParent ),
    aLeftBtn        ( pParent, SVX_RES( BTN_LEFT ) ),
    aWordText       ( pParent, SVX_RES( FT_WORD ) ),
    aWordCB         ( pParent, SVX_RES( CB_WORD ) ),
    aLookUpBtn      ( pParent, SVX_RES( BTN_LOOKUP ) ),
    m_aAlternativesText  ( pParent, SVX_RES( FT_THES_ALTERNATIVES ) ),
    m_pAlternativesCT    ( new ThesaurusAlternativesCtrl_Impl( pParent, *this ) ),
    aReplaceText    ( pParent, SVX_RES( FT_REPL ) ),
    aReplaceEdit    ( pParent, SVX_RES( ED_REPL ) ),
    aFL             ( pParent, SVX_RES( FL_VAR ) ),
    aHelpBtn        ( pParent, SVX_RES( BTN_THES_HELP ) ),
    aLangMBtn       ( pParent, SVX_RES( MB_LANGUAGE ) ),
    aOkBtn          ( pParent, SVX_RES( BTN_THES_OK ) ),
    aCancelBtn      ( pParent, SVX_RES( BTN_THES_CANCEL ) ),
    aErrStr         (          SVX_RES( STR_ERR_WORDNOTFOUND ) ),
    xThesaurus      ( NULL ),
    aLookUpText     (),
    nLookUpLanguage ( LANGUAGE_NONE ),
    pErrContext     ( NULL )
{
    // note: FreeResource must only be called in the c-tor of SvxThesaurusDialog

    aWordCB.SetButton( &aLookUpBtn );

    aLeftBtn.SetClickHdl( LINK( this, SvxThesaurusDialog_Impl, LeftBtnHdl_Impl ) );
    aWordCB.SetSelectHdl( LINK( this, SvxThesaurusDialog_Impl, WordSelectHdl_Impl ) );
    aLangMBtn.SetSelectHdl( LINK( this, SvxThesaurusDialog_Impl, LanguageHdl_Impl ) );
    aLookUpBtn.SetClickHdl( LINK( this, SvxThesaurusDialog_Impl, LookUpHdl_Impl ) );
    m_pAlternativesCT->SetSelectHdl( LINK( this, SvxThesaurusDialog_Impl, AlternativesSelectHdl_Impl ));
    m_pAlternativesCT->SetDoubleClickHdl( LINK( this, SvxThesaurusDialog_Impl, AlternativesDoubleClickHdl_Impl ));
}


SvxThesaurusDialog_Impl::~SvxThesaurusDialog_Impl()
{
    delete aLangMBtn.GetPopupMenu();
    delete pErrContext;
}


uno::Sequence< uno::Reference< linguistic2::XMeaning > > SAL_CALL SvxThesaurusDialog_Impl::queryMeanings_Impl(
        OUString& rTerm,
        const lang::Locale& rLocale,
        const beans::PropertyValues& rProperties )
    throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    uno::Sequence< uno::Reference< linguistic2::XMeaning > > aMeanings(
            xThesaurus->queryMeanings( rTerm, rLocale, rProperties ) );

    // text with '.' at the end?
    if (0 == aMeanings.getLength() && rTerm.getLength() &&
        rTerm.getStr()[ rTerm.getLength() - 1 ] == '.')
    {
        // try again without trailing '.' chars. It may be a word at the
        // end of a sentence and not an abbreviation...
        String aTxt( rTerm );
        aTxt.EraseTrailingChars( '.' );
        aMeanings = xThesaurus->queryMeanings( aTxt, rLocale, rProperties );
        if (aMeanings.getLength())
        {
            rTerm = aTxt;
        }
    }

    return aMeanings;
}


bool SvxThesaurusDialog_Impl::UpdateAlternativesBox_Impl()
{
    lang::Locale aLocale( SvxCreateLocale( nLookUpLanguage ) );
    uno::Sequence< uno::Reference< linguistic2::XMeaning > > aMeanings = queryMeanings_Impl(
            aLookUpText, aLocale, uno::Sequence< beans::PropertyValue >() );
    const sal_Int32 nMeanings = aMeanings.getLength();
    const uno::Reference< linguistic2::XMeaning > *pMeanings = aMeanings.getConstArray();

    // clear old user data of control before creating new ones via AddEntry below
    m_pAlternativesCT->ClearUserData();

    m_pAlternativesCT->Clear();
    m_pAlternativesCT->SetUpdateMode( FALSE );

    for (sal_Int32 i = 0;  i < nMeanings;  ++i)
    {
        OUString rMeaningTxt = pMeanings[i]->getMeaning();
        uno::Sequence< OUString > aSynonyms( pMeanings[i]->querySynonyms() );
        const sal_Int32 nSynonyms = aSynonyms.getLength();
        const OUString *pSynonyms = aSynonyms.getConstArray();
        DBG_ASSERT( rMeaningTxt.getLength() > 0, "meaning with empty text" );
        DBG_ASSERT( nSynonyms > 0, "meaning without synonym" );

        m_pAlternativesCT->AddEntry( i + 1, rMeaningTxt, true );
        for (sal_Int32 k = 0;  k < nSynonyms;  ++k)
            m_pAlternativesCT->AddEntry( -1, pSynonyms[k], false );
    }

    m_pAlternativesCT->SetUpdateMode( TRUE );

    const bool bWordFound = nMeanings > 0;
    if (!bWordFound)
        InfoBox( m_pParent, aErrStr ).Execute();
    return bWordFound;
}


void SvxThesaurusDialog_Impl::SetWindowTitle( LanguageType nLanguage )
{
    // Sprache anpassen
    String aStr( m_pParent->GetText() );
    aStr.Erase( aStr.Search( sal_Unicode( '(' ) ) - 1 );
    aStr.Append( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( " (" ) ) );
    aStr += GetLanguageString( (LanguageType) nLanguage );
    aStr.Append( sal_Unicode( ')' ) );
    m_pParent->SetText( aStr );    // set window title
}


IMPL_LINK( SvxThesaurusDialog_Impl, LeftBtnHdl_Impl, Button *, pBtn )
{
    if (pBtn)
    {
        aLookUpHistory.pop();                       // remove current look up word from stack
        aWordCB.SetText( aLookUpHistory.top() );    // retrieve previous look up word
        aLookUpHistory.pop();
        LookUpHdl_Impl( NULL );
    }
    return 0;
}


IMPL_LINK( SvxThesaurusDialog_Impl, LanguageHdl_Impl, MenuButton *, pBtn )
{
    PopupMenu *pMenu = aLangMBtn.GetPopupMenu();
    if (pMenu && pBtn)
    {
        USHORT nItem = pBtn->GetCurItemId();
        String aLangText( pMenu->GetItemText( nItem ) );
        LanguageType nLang = SvtLanguageTable().GetType( aLangText );
        DBG_ASSERT( nLang != LANGUAGE_NONE && nLang != LANGUAGE_DONTKNOW, "failed to get language" );
        if (xThesaurus->hasLocale( SvxCreateLocale( nLang ) ))
            nLookUpLanguage = nLang;
        SetWindowTitle( nLang );
        UpdateAlternativesBox_Impl();
    }
    return 0;
}


IMPL_LINK( SvxThesaurusDialog_Impl, LookUpHdl_Impl, Button *, EMPTYARG /*pBtn*/ )
{
    String aText( aWordCB.GetText() );

    aLookUpText = OUString( aText );
    if (aLookUpText.getLength() > 0)
        aLookUpHistory.push( aLookUpText );

    UpdateAlternativesBox_Impl();

    if ( aWordCB.GetEntryPos( aText ) == LISTBOX_ENTRY_NOTFOUND )
        aWordCB.InsertEntry( aText );

    aWordCB.SelectEntryPos( aWordCB.GetEntryPos( aText ) );
    aReplaceEdit.SetText( String() );
    aOkBtn.Enable( FALSE );
    m_pAlternativesCT->GrabFocus();

    aLeftBtn.Enable( aLookUpHistory.size() > 1 );

    return 0;
}


IMPL_LINK( SvxThesaurusDialog_Impl, WordSelectHdl_Impl, ComboBox *, pBox )
{
    if (pBox && !aWordCB.IsTravelSelect())  // act only upon return key and not when traveling with cursor keys
    {
        USHORT nPos = pBox->GetSelectEntryPos();
        String aStr( pBox->GetEntry( nPos ) );
        GetReplaceEditString( aStr );
        aWordCB.SetText( aStr );
        aOkBtn.Enable( aStr.Len() > 0 );

        LookUpHdl_Impl( NULL );
    }

    return 0;
}


IMPL_LINK( SvxThesaurusDialog_Impl, AlternativesSelectHdl_Impl, SvxCheckListBox *, pBox )
{
    SvLBoxEntry *pEntry = pBox ? pBox->GetCurEntry() : NULL;
    if (pEntry)
    {
        AlternativesUserData_Impl * pData = (AlternativesUserData_Impl *) pEntry->GetUserData();
        String aStr;
        if (!pData->IsHeader())
        {
            aStr = pData->GetText();
            GetReplaceEditString( aStr );
        }
        aReplaceEdit.SetText( aStr );
        aOkBtn.Enable( aStr.Len() > 0 );
    }
    return 0;
}


IMPL_LINK( SvxThesaurusDialog_Impl, AlternativesDoubleClickHdl_Impl, SvxCheckListBox *, pBox )
{
    SvLBoxEntry *pEntry = pBox ? pBox->GetCurEntry() : NULL;
    if (pEntry)
    {
        AlternativesUserData_Impl * pData = (AlternativesUserData_Impl *) pEntry->GetUserData();
        String aStr;
        if (!pData->IsHeader())
        {
            aStr = pData->GetText();
            GetReplaceEditString( aStr );
        }
        aWordCB.SetText( aStr );
        aOkBtn.Enable( aStr.Len() > 0 );

        if (aStr.Len() > 0)
            LookUpHdl_Impl( NULL );
    }

    //! workaround to set the selection since calling SelectEntryPos within
    //! the double click handler does not work
    Application::PostUserEvent( STATIC_LINK( this, SvxThesaurusDialog_Impl, SelectFirstHdl_Impl ), pBox );
    return 0;
}


IMPL_STATIC_LINK( SvxThesaurusDialog_Impl, SelectFirstHdl_Impl, SvxCheckListBox *, pBox )
{
    if (pBox && pBox->GetEntryCount() > 0)
        pBox->SelectEntryPos( 0 );
    return 0;
}


// class SvxThesaurusDialog ----------------------------------------------

SvxThesaurusDialog::SvxThesaurusDialog(
    Window* pParent,
    uno::Reference< linguistic2::XThesaurus >  xThes,
    const String &rWord,
    LanguageType nLanguage ) :

    SvxStandardDialog( pParent, SVX_RES( RID_SVXDLG_THESAURUS ) )
{
    m_pImpl = boost::shared_ptr< SvxThesaurusDialog_Impl >(new SvxThesaurusDialog_Impl( this ));

    m_pImpl->xThesaurus = xThes;
    m_pImpl->aLookUpText = OUString( rWord );
    m_pImpl->nLookUpLanguage = nLanguage;
    if (rWord.Len() > 0)
        m_pImpl->aLookUpHistory.push( rWord );
    m_pImpl->pErrContext = new SfxErrorContext( ERRCTX_SVX_LINGU_THESAURUS, String(), this,
                             RID_SVXERRCTX, &DIALOG_MGR() );

    FreeResource();

    OUString aTmp( rWord );
    linguistic::RemoveHyphens( aTmp );
    linguistic::ReplaceControlChars( aTmp );
    String aTmp2( aTmp );
    m_pImpl->aReplaceEdit.SetText( aTmp2 );
    m_pImpl->aWordCB.InsertEntry( aTmp2 );
    m_pImpl->aWordCB.SelectEntryPos( m_pImpl->aWordCB.GetEntryPos( aTmp2 ) );

    m_pImpl->SetWindowTitle( nLanguage );
    m_pImpl->UpdateAlternativesBox_Impl();
    m_pImpl->m_pAlternativesCT->GrabFocus();
    m_pImpl->aLeftBtn.Enable( sal_False );

    // fill language menu button list
    SvtLanguageTable aLangTab;
    uno::Sequence< lang::Locale > aLocales;
    if (m_pImpl->xThesaurus.is())
        aLocales = m_pImpl->xThesaurus->getLocales();
    const sal_Int32 nLocales = aLocales.getLength();
    const lang::Locale *pLocales = aLocales.getConstArray();
    delete m_pImpl->aLangMBtn.GetPopupMenu();
    PopupMenu* pMenu = new PopupMenu;
    pMenu->SetMenuFlags( MENU_FLAG_NOAUTOMNEMONICS );
    std::vector< OUString > aLangVec;
    for (sal_Int32 i = 0;  i < nLocales;  ++i )
    {
        const LanguageType nLang = SvxLocaleToLanguage( pLocales[i] );
        DBG_ASSERT( nLang != LANGUAGE_NONE && nLang != LANGUAGE_DONTKNOW, "failed to get language" );
        aLangVec.push_back( aLangTab.GetString( nLang ) );
    }
    std::sort( aLangVec.begin(), aLangVec.end() );
    for (size_t i = 0;  i < aLangVec.size();  ++i)
        pMenu->InsertItem( (USHORT)i+1, aLangVec[i] );  // menu items should be enumerated from 1 and not 0
    m_pImpl->aLangMBtn.SetPopupMenu( pMenu );

    // disable controls if service is missing
    if (!m_pImpl->xThesaurus.is())
        Enable( sal_False );
}


SvxThesaurusDialog::~SvxThesaurusDialog()
{
}


String SvxThesaurusDialog::GetWord()
{ 
    return m_pImpl->aReplaceEdit.GetText();
}


sal_uInt16 SvxThesaurusDialog::GetLanguage() const
{
    return m_pImpl->nLookUpLanguage;
}


void SvxThesaurusDialog::Apply()
{
}

