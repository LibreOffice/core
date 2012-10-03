/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include "thesdlg.hrc"
#include "thesdlg.hxx"
#include "thesdlg_impl.hxx"
#include "cuires.hrc"
#include "dialmgr.hxx"

#include <tools/shl.hxx>
#include <svl/lngmisc.hxx>
#include <svtools/filter.hxx>
#include <svtools/svlbitm.hxx>
#include <svtools/svtreebx.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <thesdlg.hxx>
#include <svx/dlgutil.hxx>
#include <svx/dialmgr.hxx>
#include <svx/svxerr.hxx>
#include <editeng/unolingu.hxx>
#include <svx/langbox.hxx>
#include <svtools/langtab.hxx>
#include <unotools/lingucfg.hxx>
#include <i18npool/mslangid.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <osl/file.hxx>

#include <stack>
#include <algorithm>

#include <com/sun/star/linguistic2/XThesaurus.hpp>
#include <com/sun/star/linguistic2/XMeaning.hpp>
#include <com/sun/star/linguistic2/XLinguServiceManager.hpp>

using namespace ::com::sun::star;
using ::rtl::OUString;

// class LookUpComboBox_Impl --------------------------------------------------

LookUpComboBox_Impl::LookUpComboBox_Impl(
    Window *pParent, const ResId &rResId, SvxThesaurusDialog &rImpl ) :
    ComboBox        (pParent, rResId),
    m_rDialogImpl( rImpl )
{
    m_aModifyTimer.SetTimeoutHdl( LINK( this, LookUpComboBox_Impl, ModifyTimer_Hdl ) );
    m_aModifyTimer.SetTimeout( 500 );

    EnableAutocomplete( sal_False );
}

LookUpComboBox_Impl::~LookUpComboBox_Impl()
{
}

void LookUpComboBox_Impl::Modify()
{
    m_aModifyTimer.Start();
}

IMPL_LINK( LookUpComboBox_Impl, ModifyTimer_Hdl, Timer *, EMPTYARG /*pTimer*/ )
{
    m_rDialogImpl.LookUp( GetText() );
    m_aModifyTimer.Stop();
    return 0;
}

// class ReplaceEdit_Impl --------------------------------------------------

ReplaceEdit_Impl::ReplaceEdit_Impl( Window *pParent, const ResId &rResId )
    : Edit(pParent, rResId)
    , m_pBtn(NULL)
{
}

ReplaceEdit_Impl::~ReplaceEdit_Impl()
{
}

void ReplaceEdit_Impl::Modify()
{
    if (m_pBtn)
        m_pBtn->Enable( GetText().Len() > 0 );
}

void ReplaceEdit_Impl::SetText( const XubString& rStr )
{
    Edit::SetText( rStr );
    Modify();
}

void ReplaceEdit_Impl::SetText( const XubString& rStr, const Selection& rNewSelection )
{
    Edit::SetText( rStr, rNewSelection );
    Modify();
}

// class ThesaurusAlternativesCtrl_Impl ----------------------------------

AlternativesString_Impl::AlternativesString_Impl(
    ThesaurusAlternativesCtrl_Impl &rControl,
    SvLBoxEntry* pEntry, sal_uInt16 nFlags, const String& rStr ) :

    SvLBoxString( pEntry, nFlags, rStr ),
    m_rControlImpl( rControl )
{
}

void AlternativesString_Impl::Paint(
    const Point& rPos,
    SvLBox& rDev, sal_uInt16,
    SvLBoxEntry* pEntry )
{
    AlternativesExtraData* pData = m_rControlImpl.GetExtraData( pEntry );
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
        SvxThesaurusDialog &rImpl ) :
    SvxCheckListBox( pParent, CUI_RES( CT_THES_ALTERNATIVES ) ),
    m_rDialogImpl( rImpl )
{
    SetStyle( GetStyle() | WB_CLIPCHILDREN | WB_HSCROLL | WB_FORCE_MAKEVISIBLE );
    SetHighlightRange();
}

ThesaurusAlternativesCtrl_Impl::~ThesaurusAlternativesCtrl_Impl()
{
    ClearExtraData();
}

void ThesaurusAlternativesCtrl_Impl::ClearExtraData()
{
    UserDataMap_t   aEmpty;
    m_aUserData.swap( aEmpty );
}

void ThesaurusAlternativesCtrl_Impl::SetExtraData(
    const SvLBoxEntry *pEntry,
    const AlternativesExtraData &rData )
{
    if (!pEntry)
        return;

    UserDataMap_t::iterator aIt( m_aUserData.find( pEntry ) );
    if (aIt != m_aUserData.end())
        aIt->second = rData;
    else
        m_aUserData[ pEntry ] = rData;
}

AlternativesExtraData * ThesaurusAlternativesCtrl_Impl::GetExtraData(
    const SvLBoxEntry *pEntry )
{
    AlternativesExtraData *pRes = NULL;
    UserDataMap_t::iterator aIt( m_aUserData.find( pEntry ) );
    if (aIt != m_aUserData.end())
        pRes = &aIt->second;
    return pRes;
}

SvLBoxEntry * ThesaurusAlternativesCtrl_Impl::AddEntry( sal_Int32 nVal, const String &rText, bool bIsHeader )
{
    SvLBoxEntry* pEntry = new SvLBoxEntry;
    String aText;
    if (bIsHeader && nVal >= 0)
    {
        aText = String::CreateFromInt32( nVal );
        aText += rtl::OUString(". ");
    }
    pEntry->AddItem( new SvLBoxString( pEntry, 0, String() ) ); // add empty column
    aText += rText;
    pEntry->AddItem( new SvLBoxContextBmp( pEntry, 0, Image(), Image(), 0 ) );  // otherwise crash
    pEntry->AddItem( new AlternativesString_Impl( *this, pEntry, 0, aText ) );

    SetExtraData( pEntry, AlternativesExtraData( rText, bIsHeader ) );
    GetModel()->Insert( pEntry );

    if (bIsHeader)
        GetViewDataEntry( pEntry )->SetSelectable( false );

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

void ThesaurusAlternativesCtrl_Impl::Paint( const Rectangle& rRect )
{
    if (!m_rDialogImpl.WordFound())
    {
        Size aTextSize( GetTextWidth( m_rDialogImpl.getErrStr() ), GetTextHeight() );
        aTextSize  = LogicToPixel( aTextSize );
        Point aPos;
        aPos.X() += GetSizePixel().Width() / 2  - aTextSize.Width() / 2;
        aPos.Y() += GetSizePixel().Height() / 2;
        aPos = PixelToLogic( aPos );
        DrawText( aPos, m_rDialogImpl.getErrStr() );

    }
    else
        SvxCheckListBox::Paint( rRect );
}

uno::Sequence< uno::Reference< linguistic2::XMeaning > > SvxThesaurusDialog::queryMeanings_Impl(
        OUString& rTerm,
        const lang::Locale& rLocale,
        const beans::PropertyValues& rProperties )
    throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    uno::Sequence< uno::Reference< linguistic2::XMeaning > > aMeanings(
            xThesaurus->queryMeanings( rTerm, rLocale, rProperties ) );

    // text with '.' at the end?
    if ( 0 == aMeanings.getLength() && !rTerm.isEmpty() &&
        rTerm.getStr()[ rTerm.getLength() - 1 ] == '.')
    {
        // try again without trailing '.' chars. It may be a word at the
        // end of a sentence and not an abbreviation...
        String aTxt(comphelper::string::stripEnd(rTerm, '.'));
        aMeanings = xThesaurus->queryMeanings( aTxt, rLocale, rProperties );
        if (aMeanings.getLength())
        {
            rTerm = aTxt;
        }
    }

    return aMeanings;
}

bool SvxThesaurusDialog::UpdateAlternativesBox_Impl()
{
    lang::Locale aLocale( SvxCreateLocale( nLookUpLanguage ) );
    uno::Sequence< uno::Reference< linguistic2::XMeaning > > aMeanings = queryMeanings_Impl(
            aLookUpText, aLocale, uno::Sequence< beans::PropertyValue >() );
    const sal_Int32 nMeanings = aMeanings.getLength();
    const uno::Reference< linguistic2::XMeaning > *pMeanings = aMeanings.getConstArray();

    m_pAlternativesCT->SetUpdateMode( sal_False );

    // clear old user data of control before creating new ones via AddEntry below
    m_pAlternativesCT->ClearExtraData();

    m_pAlternativesCT->Clear();
    for (sal_Int32 i = 0;  i < nMeanings;  ++i)
    {
        OUString rMeaningTxt = pMeanings[i]->getMeaning();
        uno::Sequence< OUString > aSynonyms( pMeanings[i]->querySynonyms() );
        const sal_Int32 nSynonyms = aSynonyms.getLength();
        const OUString *pSynonyms = aSynonyms.getConstArray();
        DBG_ASSERT( !rMeaningTxt.isEmpty(), "meaning with empty text" );
        DBG_ASSERT( nSynonyms > 0, "meaning without synonym" );

        m_pAlternativesCT->AddEntry( i + 1, rMeaningTxt, true );
        for (sal_Int32 k = 0;  k < nSynonyms;  ++k)
            m_pAlternativesCT->AddEntry( -1, pSynonyms[k], false );
    }

    m_pAlternativesCT->SetUpdateMode( sal_True );

    return nMeanings > 0;
}

void SvxThesaurusDialog::LookUp( const String &rText )
{
    if (rText != aWordCB.GetText()) // avoid moving of the cursor if the text is the same
        aWordCB.SetText( rText );
    LookUp_Impl();
}

IMPL_LINK( SvxThesaurusDialog, LeftBtnHdl_Impl, Button *, pBtn )
{
    if (pBtn && aLookUpHistory.size() >= 2)
    {
        aLookUpHistory.pop();                       // remove current look up word from stack
        aWordCB.SetText( aLookUpHistory.top() );    // retrieve previous look up word
        aLookUpHistory.pop();
        LookUp_Impl();
    }
    return 0;
}

IMPL_LINK( SvxThesaurusDialog, LanguageHdl_Impl, MenuButton *, pBtn )
{
    PopupMenu *pMenu = aLangMBtn.GetPopupMenu();
    if (pMenu && pBtn)
    {
        sal_uInt16 nItem = pBtn->GetCurItemId();
        String aLangText( pMenu->GetItemText( nItem ) );
        LanguageType nLang = SvtLanguageTable().GetType( aLangText );
        DBG_ASSERT( nLang != LANGUAGE_NONE && nLang != LANGUAGE_DONTKNOW, "failed to get language" );
        if (xThesaurus->hasLocale( SvxCreateLocale( nLang ) ))
            nLookUpLanguage = nLang;
        SetWindowTitle( nLang );
        UpdateVendorImage();
        LookUp_Impl();
    }
    return 0;
}

void SvxThesaurusDialog::LookUp_Impl()
{
    String aText( aWordCB.GetText() );

    aLookUpText = OUString( aText );
    if (!aLookUpText.isEmpty() &&
            (aLookUpHistory.empty() || aLookUpText != aLookUpHistory.top()))
        aLookUpHistory.push( aLookUpText );

    m_bWordFound = UpdateAlternativesBox_Impl();
    m_pAlternativesCT->Enable( m_bWordFound );

    if ( aWordCB.GetEntryPos( aText ) == LISTBOX_ENTRY_NOTFOUND )
        aWordCB.InsertEntry( aText );

    aReplaceEdit.SetText( String() );
    aLeftBtn.Enable( aLookUpHistory.size() > 1 );
}

IMPL_LINK( SvxThesaurusDialog, WordSelectHdl_Impl, ComboBox *, pBox )
{
    if (pBox && !aWordCB.IsTravelSelect())  // act only upon return key and not when traveling with cursor keys
    {
        sal_uInt16 nPos = pBox->GetSelectEntryPos();
        String aStr( pBox->GetEntry( nPos ) );
        aStr = linguistic::GetThesaurusReplaceText( aStr );
        aWordCB.SetText( aStr );
        LookUp_Impl();
    }

    return 0;
}

IMPL_LINK( SvxThesaurusDialog, AlternativesSelectHdl_Impl, SvxCheckListBox *, pBox )
{
    SvLBoxEntry *pEntry = pBox ? pBox->GetCurEntry() : NULL;
    if (pEntry)
    {
        AlternativesExtraData * pData = m_pAlternativesCT->GetExtraData( pEntry );
        String aStr;
        if (pData && !pData->IsHeader())
        {
            aStr = pData->GetText();
            aStr = linguistic::GetThesaurusReplaceText( aStr );
        }
        aReplaceEdit.SetText( aStr );
    }
    return 0;
}

IMPL_LINK( SvxThesaurusDialog, AlternativesDoubleClickHdl_Impl, SvxCheckListBox *, pBox )
{
    SvLBoxEntry *pEntry = pBox ? pBox->GetCurEntry() : NULL;
    if (pEntry)
    {
        AlternativesExtraData * pData = m_pAlternativesCT->GetExtraData( pEntry );
        String aStr;
        if (pData && !pData->IsHeader())
        {
            aStr = pData->GetText();
            aStr = linguistic::GetThesaurusReplaceText( aStr );
        }

        aWordCB.SetText( aStr );
        if (aStr.Len() > 0)
            LookUp_Impl();
    }

    //! workaround to set the selection since calling SelectEntryPos within
    //! the double click handler does not work
    Application::PostUserEvent( STATIC_LINK( this, SvxThesaurusDialog, SelectFirstHdl_Impl ), pBox );
    return 0;
}

IMPL_STATIC_LINK( SvxThesaurusDialog, SelectFirstHdl_Impl, SvxCheckListBox *, pBox )
{
    (void) pThis;
    if (pBox && pBox->GetEntryCount() >= 2)
        pBox->SelectEntryPos( 1 );  // pos 0 is a 'header' that is not selectable
    return 0;
}

////////////////////////////////////////////////////////////

static Image lcl_GetImageFromPngUrl( const OUString &rFileUrl )
{
    Image aRes;

    OUString aTmp;
    osl::FileBase::getSystemPathFromFileURL( rFileUrl, aTmp );

    Graphic aGraphic;
    const String aFilterName( RTL_CONSTASCII_USTRINGPARAM( IMP_PNG ) );
    if( GRFILTER_OK == GraphicFilter::LoadGraphic( aTmp, aFilterName, aGraphic, NULL, NULL ) )
    {
        aRes = Image( aGraphic.GetBitmapEx() );
    }
    return aRes;
}

static String lcl_GetThesImplName( const lang::Locale &rLocale )
{
    String aRes;

    uno::Reference< linguistic2::XLinguServiceManager >     xLngMgr;
    try
    {
        uno::Reference< lang::XMultiServiceFactory >  xMSF( ::comphelper::getProcessServiceFactory(), uno::UNO_QUERY_THROW );
        xLngMgr = uno::Reference< linguistic2::XLinguServiceManager >( xMSF->createInstance(
                OUString( RTL_CONSTASCII_USTRINGPARAM(
                    "com.sun.star.linguistic2.LinguServiceManager" ))), uno::UNO_QUERY_THROW );

        DBG_ASSERT( xLngMgr.is(), "LinguServiceManager missing" );
        if (xLngMgr.is())
        {
            uno::Sequence< OUString > aServiceNames = xLngMgr->getConfiguredServices(
                    OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.linguistic2.Thesaurus")), rLocale );
            // there should be at most one thesaurus configured for each language
            DBG_ASSERT( aServiceNames.getLength() <= 1, "more than one thesaurus found. Should not be possible" );
            if (aServiceNames.getLength() == 1)
                aRes = aServiceNames[0];
        }
    }
    catch (uno::Exception &e)
    {
        (void) e;
        DBG_ASSERT( 0, "failed to get thesaurus" );
    }

    return aRes;
}

void SvxThesaurusDialog::UpdateVendorImage()
{
    SetUpdateMode( sal_False );

    SvtLinguConfig aCfg;
    if (aCfg.HasVendorImages( "ThesaurusDialogImage" ))
    {
        Image aImage;
        String sThesImplName( lcl_GetThesImplName( SvxCreateLocale( nLookUpLanguage ) ) );
        OUString aThesDialogImageUrl( aCfg.GetThesaurusDialogImage( sThesImplName ) );
        if (sThesImplName.Len() > 0 && !aThesDialogImageUrl.isEmpty())
            aImage = Image( lcl_GetImageFromPngUrl( aThesDialogImageUrl ) );
        else
            aImage = aVendorDefaultImage;
        aVendorImageFI.SetImage( aImage );
    }

    SetUpdateMode( sal_True );
}

IMPL_STATIC_LINK( SvxThesaurusDialog, VendorImageInitHdl, SvxThesaurusDialog*, EMPTYARG )
{
    pThis->SetUpdateMode( sal_False );

    SvtLinguConfig aCfg;
    if (aCfg.HasVendorImages( "ThesaurusDialogImage" ))
    {
        Image aImage( pThis->aVendorDefaultImage );
        pThis->aVendorImageFI.SetImage( aImage );
        pThis->aVendorImageFI.Show();

        // move down visible controls according to the vendor images height
        Size aVendorSize = pThis->aVendorImageFI.GetSizePixel();
        Size aImageSize  = pThis->aVendorImageFI.GetImage().GetSizePixel();
        if (aImageSize.Height())
        {
            aVendorSize.Height() = aImageSize.Height();
            if(aVendorSize.Width() < aImageSize.Width())
                aVendorSize.Width() = aImageSize.Width();
            pThis->aVendorImageFI.SetSizePixel( aVendorSize );
        }
        const sal_Int32 nDiff = aVendorSize.Height();
        pThis->aVendorImageFI.SetSizePixel( aVendorSize );
        Control* aControls[] = {
            &pThis->aLeftBtn,
            &pThis->aWordText,
            &pThis->aWordCB,
            &pThis->m_aAlternativesText,
            pThis->m_pAlternativesCT.get(),
            &pThis->aReplaceText,
            &pThis->aReplaceEdit,
            &pThis->aFL,
            &pThis->aHelpBtn,
            &pThis->aLangMBtn,
            &pThis->aReplaceBtn,
            &pThis->aCancelBtn,
            0
        };
        sal_Int32 nControl = 0;
        while (aControls[nControl])
        {
            Point aPos = aControls[nControl]->GetPosPixel();
            aPos.Y() += nDiff;
            aControls[nControl]->SetPosPixel(aPos);
            ++nControl;
        }
        Size aDlgSize = pThis->GetSizePixel();
        aDlgSize.Height() += nDiff;
        pThis->SetSizePixel( aDlgSize );
        pThis->Invalidate();
    }

    pThis->UpdateVendorImage();
    pThis->SetUpdateMode( sal_True );

    return 0;
};

// class SvxThesaurusDialog ----------------------------------------------

SvxThesaurusDialog::SvxThesaurusDialog(
    Window* pParent,
    uno::Reference< linguistic2::XThesaurus >  xThes,
    const String &rWord,
    LanguageType nLanguage ) :

    SvxStandardDialog( pParent, CUI_RES( RID_SVXDLG_THESAURUS ) ),

    aVendorImageFI  ( this, CUI_RES( IMG_VENDOR ) ),
    aLeftBtn        ( this, CUI_RES( BTN_LEFT ) ),
    aWordText       ( this, CUI_RES( FT_WORD ) ),
    aWordCB         ( this, CUI_RES( CB_WORD ), *this ),
    m_aAlternativesText  ( this, CUI_RES( FT_THES_ALTERNATIVES ) ),
    m_pAlternativesCT    ( new ThesaurusAlternativesCtrl_Impl( this, *this ) ),
    aReplaceText    ( this, CUI_RES( FT_REPL ) ),
    aReplaceEdit    ( this, CUI_RES( ED_REPL ) ),
    aFL             ( this, CUI_RES( FL_VAR ) ),
    aHelpBtn        ( this, CUI_RES( BTN_THES_HELP ) ),
    aLangMBtn       ( this, CUI_RES( MB_LANGUAGE ) ),
    aReplaceBtn     ( this, CUI_RES( BTN_THES_OK ) ),
    aCancelBtn      ( this, CUI_RES( BTN_THES_CANCEL ) ),
    aErrStr                 ( CUI_RES( STR_ERR_TEXTNOTFOUND ) ),
    aVendorDefaultImage     ( CUI_RES( IMG_DEFAULT_VENDOR ) ),
    xThesaurus      ( NULL ),
    aLookUpText     (),
    nLookUpLanguage ( LANGUAGE_NONE ),
    m_bWordFound( false )
{
    aReplaceEdit.SetButton( &aReplaceBtn );

    aLeftBtn.SetClickHdl( LINK( this, SvxThesaurusDialog, LeftBtnHdl_Impl ) );
    aWordCB.SetSelectHdl( LINK( this, SvxThesaurusDialog, WordSelectHdl_Impl ) );
    aLangMBtn.SetSelectHdl( LINK( this, SvxThesaurusDialog, LanguageHdl_Impl ) );
    m_pAlternativesCT->SetSelectHdl( LINK( this, SvxThesaurusDialog, AlternativesSelectHdl_Impl ));
    m_pAlternativesCT->SetDoubleClickHdl( LINK( this, SvxThesaurusDialog, AlternativesDoubleClickHdl_Impl ));

    Application::PostUserEvent( STATIC_LINK( this, SvxThesaurusDialog, VendorImageInitHdl ) );

    xThesaurus = xThes;
    aLookUpText = OUString( rWord );
    nLookUpLanguage = nLanguage;
    if (rWord.Len() > 0)
        aLookUpHistory.push( rWord );

    FreeResource();

    OUString aTmp( rWord );
    linguistic::RemoveHyphens( aTmp );
    linguistic::ReplaceControlChars( aTmp );
    String aTmp2( aTmp );
    aReplaceEdit.SetText( aTmp2 );
    aWordCB.InsertEntry( aTmp2 );

    LookUp( aTmp2 );
    m_pAlternativesCT->GrabFocus();
    aLeftBtn.Enable( sal_False );

    // fill language menu button list
    SvtLanguageTable aLangTab;
    uno::Sequence< lang::Locale > aLocales;
    if (xThesaurus.is())
        aLocales = xThesaurus->getLocales();
    const sal_Int32 nLocales = aLocales.getLength();
    const lang::Locale *pLocales = aLocales.getConstArray();
    delete aLangMBtn.GetPopupMenu();
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
        pMenu->InsertItem( (sal_uInt16)i+1, aLangVec[i] );  // menu items should be enumerated from 1 and not 0
    aLangMBtn.SetPopupMenu( pMenu );

    SetWindowTitle( nLanguage );

    // disable controls if service is missing
    if (!xThesaurus.is())
        Enable( sal_False );
}

SvxThesaurusDialog::~SvxThesaurusDialog()
{
    delete aLangMBtn.GetPopupMenu();
}

void SvxThesaurusDialog::SetWindowTitle( LanguageType nLanguage )
{
    // adjust language
    String aStr( GetText() );
    aStr.Erase( aStr.Search( sal_Unicode( '(' ) ) - 1 );
    aStr.Append( rtl::OUString(" (") );
    aStr += SvtLanguageTable().GetLanguageString( nLanguage );
    aStr.Append( sal_Unicode( ')' ) );
    SetText( aStr );    // set window title
}

String SvxThesaurusDialog::GetWord()
{
    return aReplaceEdit.GetText();
}

sal_uInt16 SvxThesaurusDialog::GetLanguage() const
{
    return nLookUpLanguage;
}

void SvxThesaurusDialog::Apply()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
