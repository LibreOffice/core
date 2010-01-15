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

#include <com/sun/star/linguistic2/XThesaurus.hpp>
#include <com/sun/star/linguistic2/XMeaning.hpp>

#include <tools/shl.hxx>
#include <svl/lngmisc.hxx>
#include <svtools/svlbitm.hxx>
#include <svtools/svtreebx.hxx>
#include <svtools/langtab.hxx>
#include <vcl/edit.hxx>
#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <vcl/menubtn.hxx>
#include <i18npool/mslangid.hxx>


#define _SVX_THESDLG_CXX

#include <svx/thesdlg.hxx>
#include <svx/dialmgr.hxx>
#include "dlgutil.hxx"
#include "svxerr.hxx"
#include "thesdlg.hrc"

#include <unolingu.hxx>
#include <svx/dialogs.hrc>
#include <svx/langbox.hxx>
#include <svx/checklbx.hxx>

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

// class ThesaurusAlternativesCtrl_Impl ----------------------------------

class AlternativesUserData_Impl
{
    String  sText;
    bool    bHeader;

public:
    AlternativesUserData_Impl( const String &rText, bool bIsHeader ) :
        sText(rText),
        bHeader(bIsHeader)
        {
        }

    bool  IsHeader() const          { return bHeader; }
    const String& GetText() const   { return sText; }
};


class AlternativesString_Impl : public SvLBoxString
{
public:

    AlternativesString_Impl( SvLBoxEntry* pEntry, USHORT nFlags, const String& rStr )
        : SvLBoxString( pEntry, nFlags, rStr ) {}

    virtual void Paint( const Point& rPos, SvLBox& rDev, USHORT nFlags, SvLBoxEntry* pEntry);
};


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


class ThesaurusAlternativesCtrl_Impl :
    public SvxCheckListBox
{
    // disable copy c-tor and assignment operator
    ThesaurusAlternativesCtrl_Impl( const ThesaurusAlternativesCtrl_Impl & );
    ThesaurusAlternativesCtrl_Impl & operator = ( const ThesaurusAlternativesCtrl_Impl & );

public:
    ThesaurusAlternativesCtrl_Impl( Window* pParent );
    virtual ~ThesaurusAlternativesCtrl_Impl();



    SvLBoxEntry *   AddEntry( sal_Int32 nVal, const String &rText, bool bIsHeader );
    void            ClearUserData();

    virtual void KeyInput( const KeyEvent& rKEvt );
};


ThesaurusAlternativesCtrl_Impl::ThesaurusAlternativesCtrl_Impl(
        Window* pParent ) :
    SvxCheckListBox( pParent, SVX_RES( CT_THES_ALTERNATIVES ) )
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
    else if ( GetEntryCount() )
        SvxCheckListBox::KeyInput( rKEvt );
}


// struct SvxThesaurusDialog_Impl ----------------------------------------

struct SvxThesaurusDialog_Impl
{
    FixedText       aWordText;
    ListBox         aWordLB;
    FixedText       aReplaceText;
    Edit            aReplaceEdit;
    FixedText       m_aAlternativesText;
    boost::shared_ptr< ThesaurusAlternativesCtrl_Impl > m_pAlternativesCT;
    FixedLine       aFL;

    OKButton        aOkBtn;
    CancelButton    aCancelBtn;
    PushButton      aLookUpBtn;
    MenuButton      aLangMBtn;
    HelpButton      aHelpBtn;

    String          aErrStr;

    uno::Reference< linguistic2::XThesaurus >   xThesaurus;
    OUString        aLookUpText;
    LanguageType    nLookUpLanguage;

    SfxErrorContext*    pErrContext;    // ErrorContext, w"ahrend der Dialog oben ist


    SvxThesaurusDialog_Impl( Window* pParent );
    ~SvxThesaurusDialog_Impl();
};


SvxThesaurusDialog_Impl::SvxThesaurusDialog_Impl( Window* pParent ) :
    aWordText       ( pParent, SVX_RES( FT_WORD ) ),
    aWordLB         ( pParent, SVX_RES( LB_WORD ) ),
    aReplaceText    ( pParent, SVX_RES( FT_REPL ) ),
    aReplaceEdit    ( pParent, SVX_RES( ED_REPL ) ),
    m_aAlternativesText  ( pParent, SVX_RES( FT_THES_ALTERNATIVES ) ),
    m_pAlternativesCT    ( new ThesaurusAlternativesCtrl_Impl( pParent ) ),
    aFL             ( pParent, SVX_RES( FL_VAR ) ),
    aOkBtn          ( pParent, SVX_RES( BTN_THES_OK ) ),
    aCancelBtn      ( pParent, SVX_RES( BTN_THES_CANCEL ) ),
    aLookUpBtn      ( pParent, SVX_RES( BTN_LOOKUP ) ),
    aLangMBtn       ( pParent, SVX_RES( MB_LANGUAGE ) ),
    aHelpBtn        ( pParent, SVX_RES( BTN_THES_HELP ) ),
    aErrStr         (          SVX_RES( STR_ERR_WORDNOTFOUND ) ),
    xThesaurus      ( NULL ),
    aLookUpText     (),
    nLookUpLanguage ( LANGUAGE_NONE ),
    pErrContext     ( NULL )
{
    // note: FreeResource must only be called in the c-tor of SvxThesaurusDialog
}


SvxThesaurusDialog_Impl::~SvxThesaurusDialog_Impl()
{
    delete aLangMBtn.GetPopupMenu();
    delete pErrContext;
}

// class SvxThesaurusDialog ----------------------------------------------

SvxThesaurusDialog::SvxThesaurusDialog(
    Window* pParent,
    uno::Reference< linguistic2::XThesaurus >  xThes,
    const String &rWord,
    sal_Int16 nLanguage ) :

    SvxStandardDialog( pParent, SVX_RES( RID_SVXDLG_THESAURUS ) )
{
    m_pImpl = boost::shared_ptr< SvxThesaurusDialog_Impl >(new SvxThesaurusDialog_Impl( this ));

    m_pImpl->xThesaurus = xThes;
    m_pImpl->aLookUpText = OUString( rWord );
    m_pImpl->nLookUpLanguage = nLanguage;
    m_pImpl->pErrContext = new SfxErrorContext( ERRCTX_SVX_LINGU_THESAURUS, String(), this,
                             RID_SVXERRCTX, &DIALOG_MGR() );

    FreeResource();

    m_pImpl->aLangMBtn.SetSelectHdl( LINK( this, SvxThesaurusDialog, LanguageHdl_Impl ) );
    m_pImpl->aLookUpBtn.SetClickHdl( LINK( this, SvxThesaurusDialog, LookUpHdl_Impl ) );
    m_pImpl->aWordLB.SetSelectHdl( LINK( this, SvxThesaurusDialog, WordSelectHdl_Impl ) );
    m_pImpl->m_pAlternativesCT->SetSelectHdl( LINK( this, SvxThesaurusDialog, AlternativesSelectHdl_Impl ));
    m_pImpl->m_pAlternativesCT->SetDoubleClickHdl( LINK( this, SvxThesaurusDialog, AlternativesDoubleClickHdl_Impl ));

    OUString aTmp( rWord );
    linguistic::RemoveHyphens( aTmp );
    linguistic::ReplaceControlChars( aTmp );
    m_pImpl->aReplaceEdit.SetText( aTmp );
    m_pImpl->aWordLB.InsertEntry( aTmp );
    m_pImpl->aWordLB.SelectEntry( aTmp );

    SetWindowTitle( nLanguage );
    UpdateAlternativesBox_Impl();
    m_pImpl->m_pAlternativesCT->GrabFocus();

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


// -----------------------------------------------------------------------

SvxThesaurusDialog::~SvxThesaurusDialog()
{
}

// -----------------------------------------------------------------------

uno::Sequence< uno::Reference< linguistic2::XMeaning > > SAL_CALL SvxThesaurusDialog::queryMeanings_Impl(
        OUString& rTerm,
        const lang::Locale& rLocale,
        const beans::PropertyValues& rProperties )
    throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    uno::Sequence< uno::Reference< linguistic2::XMeaning > > aMeanings(
            m_pImpl->xThesaurus->queryMeanings( rTerm, rLocale, rProperties ) );

    // text with '.' at the end?
    if (0 == aMeanings.getLength() && rTerm.getLength() &&
        rTerm.getStr()[ rTerm.getLength() - 1 ] == '.')
    {
        // try again without trailing '.' chars. It may be a word at the
        // end of a sentence and not an abbreviation...
        String aTxt( rTerm );
        aTxt.EraseTrailingChars( '.' );
        aMeanings = m_pImpl->xThesaurus->queryMeanings( aTxt, rLocale, rProperties );
        if (aMeanings.getLength())
        {
            rTerm = aTxt;
        }
    }

    return aMeanings;
}

// -----------------------------------------------------------------------

String SvxThesaurusDialog::GetWord()
{ 
    return m_pImpl->aReplaceEdit.GetText();
}

// -----------------------------------------------------------------------

sal_uInt16 SvxThesaurusDialog::GetLanguage() const
{
    return m_pImpl->nLookUpLanguage;
}

// -----------------------------------------------------------------------

bool SvxThesaurusDialog::UpdateAlternativesBox_Impl()
{
    // clear old user data of control before creating new ones via AddEntry below
    m_pImpl->m_pAlternativesCT->ClearUserData();

    m_pImpl->m_pAlternativesCT->Clear();
    m_pImpl->m_pAlternativesCT->SetUpdateMode( FALSE );

    lang::Locale aLocale( SvxCreateLocale( m_pImpl->nLookUpLanguage ) );
    uno::Sequence< uno::Reference< linguistic2::XMeaning > > aMeanings = queryMeanings_Impl(
            m_pImpl->aLookUpText, aLocale, uno::Sequence< beans::PropertyValue >() );
    const sal_Int32 nMeanings = aMeanings.getLength();
    const uno::Reference< linguistic2::XMeaning > *pMeanings = aMeanings.getConstArray();
    for (sal_Int32 i = 0;  i < nMeanings;  ++i)
    {
        OUString rMeaningTxt = pMeanings[i]->getMeaning();
        uno::Sequence< OUString > aSynonyms( pMeanings[i]->querySynonyms() );
        const sal_Int32 nSynonyms = aSynonyms.getLength();
        const OUString *pSynonyms = aSynonyms.getConstArray();
        DBG_ASSERT( rMeaningTxt.getLength() > 0, "meaning with empty text" );
        DBG_ASSERT( nSynonyms > 0, "meaning without synonym" );

        m_pImpl->m_pAlternativesCT->AddEntry( i + 1, rMeaningTxt, true );
        for (sal_Int32 k = 0;  k < nSynonyms;  ++k)
            m_pImpl->m_pAlternativesCT->AddEntry( -1, pSynonyms[k], false );
    }

    m_pImpl->m_pAlternativesCT->SetUpdateMode( TRUE );

    return nMeanings > 0;
}

// -----------------------------------------------------------------------

void SvxThesaurusDialog::Apply()
{
}

// -----------------------------------------------------------------------
void SvxThesaurusDialog::SetWindowTitle(sal_Int16 nLanguage)
{
    // Sprache anpassen
    String aStr( GetText() );
    aStr.Erase( aStr.Search( sal_Unicode( '(' ) ) - 1 );
    aStr.Append( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( " (" ) ) );
    aStr += GetLanguageString( (LanguageType) nLanguage );
    aStr.Append( sal_Unicode( ')' ) );
    SetText( aStr );    // set window title
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxThesaurusDialog, LanguageHdl_Impl, MenuButton *, pBtn )
{
    PopupMenu *pMenu = m_pImpl->aLangMBtn.GetPopupMenu();
    if (pMenu && pBtn)
    {
        USHORT nItem = pBtn->GetCurItemId();
        String aLangText( pMenu->GetItemText( nItem ) );
        LanguageType nLang = SvtLanguageTable().GetType( aLangText );
        DBG_ASSERT( nLang != LANGUAGE_NONE && nLang != LANGUAGE_DONTKNOW, "failed to get language" );
        if (m_pImpl->xThesaurus->hasLocale( SvxCreateLocale( nLang ) ))
            m_pImpl->nLookUpLanguage = nLang;
        SetWindowTitle( nLang );
        UpdateAlternativesBox_Impl();
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxThesaurusDialog, LookUpHdl_Impl, Button *, pBtn )
{
    String aText( m_pImpl->aReplaceEdit.GetText() );

    OUString aOldLookUpText = m_pImpl->aLookUpText;
    m_pImpl->aLookUpText = OUString( aText );

    bool bWordFound = UpdateAlternativesBox_Impl();
    if (!bWordFound)
        InfoBox( this, m_pImpl->aErrStr ).Execute();

    if ( m_pImpl->aWordLB.GetEntryPos( aText ) == LISTBOX_ENTRY_NOTFOUND )
        m_pImpl->aWordLB.InsertEntry( aText );

    m_pImpl->aWordLB.SelectEntry( aText );
    m_pImpl->aReplaceEdit.SetText( String() );

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxThesaurusDialog, WordSelectHdl_Impl, ListBox *, pBox )
{
    String aStr( pBox->GetSelectEntry() );
    GetReplaceEditString( aStr );
    m_pImpl->aReplaceEdit.SetText( aStr );

    LookUpHdl_Impl( &m_pImpl->aCancelBtn );

    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxThesaurusDialog, AlternativesSelectHdl_Impl, SvxCheckListBox *, pBox )
{
    SvLBoxEntry *pEntry = pBox->GetCurEntry();
    if (pEntry)
    {
        AlternativesUserData_Impl * pData = (AlternativesUserData_Impl *) pEntry->GetUserData();
        String aStr( pData->GetText() );
        GetReplaceEditString( aStr );
        m_pImpl->aReplaceEdit.SetText( aStr );
    }
    return 0;
}

// -----------------------------------------------------------------------

IMPL_LINK( SvxThesaurusDialog, AlternativesDoubleClickHdl_Impl, SvxCheckListBox *, pBox )
{
    SvLBoxEntry *pEntry = pBox->GetCurEntry();
    if (pEntry)
    {
        AlternativesUserData_Impl * pData = (AlternativesUserData_Impl *) pEntry->GetUserData();
        String aStr( pData->GetText() );
        GetReplaceEditString( aStr );
        m_pImpl->aReplaceEdit.SetText( aStr );
        LookUpHdl_Impl( &m_pImpl->aCancelBtn );
    }
    return 0;
}


