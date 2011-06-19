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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_basctl.hxx"

#include "managelang.hxx"
#include "iderid.hxx"
#include "localizationmgr.hxx"
#include "iderdll.hxx"
#include "basidesh.hxx"
#include "basobj.hxx"

#include "managelang.hrc"
#include "dlgresid.hrc"
#include "helpid.hrc"

#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/resource/XStringResourceManager.hpp>
#include <comphelper/processfactory.hxx>
#include <vcl/unohelp.hxx>
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>
#include <unotools/localedatawrapper.hxx>
#include <editeng/unolingu.hxx>

#include <svtools/langtab.hxx>

#include <sfx2/bindings.hxx>

using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::resource;
using namespace ::com::sun::star::uno;

bool localesAreEqual( const Locale& rLocaleLeft, const Locale& rLocaleRight )
{
    bool bRet = ( rLocaleLeft.Language.equals( rLocaleRight.Language ) &&
                  rLocaleLeft.Country.equals( rLocaleRight.Country ) &&
                  rLocaleLeft.Variant.equals( rLocaleRight.Variant ) );
    return bRet;
}

namespace {
    long getLongestWordWidth( const String& rText, const Window& rWin )
    {
        long nWidth = 0;
        Reference< XBreakIterator > xBreakIter( vcl::unohelper::CreateBreakIterator() );
        sal_Int32 nStartPos = 0;
        const Locale aLocale = Application::GetSettings().GetUILocale();
        Boundary aBoundary = xBreakIter->getWordBoundary(
            rText, nStartPos, aLocale, WordType::ANYWORD_IGNOREWHITESPACES, sal_True );

        while ( aBoundary.startPos != aBoundary.endPos )
        {
            nStartPos = aBoundary.endPos;
            String sWord( rText.Copy(
                (sal_uInt16)aBoundary.startPos,
                (sal_uInt16)aBoundary.endPos - (sal_uInt16)aBoundary.startPos ) );
            long nTemp = rWin.GetCtrlTextWidth( sWord );
            if ( nTemp > nWidth )
                nWidth = nTemp;
            aBoundary = xBreakIter->nextWord(
                rText, nStartPos, aLocale, WordType::ANYWORD_IGNOREWHITESPACES );
        }

        return nWidth;
    }
}

ManageLanguageDialog::ManageLanguageDialog( Window* pParent, LocalizationMgr* _pLMgr ) :

    ModalDialog( pParent, IDEResId( RID_DLG_MANAGE_LANGUAGE ) ),

    m_aLanguageFT       ( this, IDEResId( FT_LANGUAGE ) ),
    m_aLanguageLB       ( this, IDEResId( LB_LANGUAGE ) ),
    m_aAddPB            ( this, IDEResId( PB_ADD_LANG ) ),
    m_aDeletePB         ( this, IDEResId( PB_DEL_LANG ) ),
    m_aMakeDefPB        ( this, IDEResId( PB_MAKE_DEFAULT ) ),
    m_aInfoFT           ( this, IDEResId( FT_INFO ) ),
    m_aBtnLine          ( this, IDEResId( FL_BUTTONS ) ),
    m_aHelpBtn          ( this, IDEResId( PB_HELP ) ),
    m_aCloseBtn         ( this, IDEResId( PB_CLOSE ) ),
    m_pLocalizationMgr  ( _pLMgr ),
    m_sDefLangStr       (       IDEResId( STR_DEF_LANG ) ),
    m_sDeleteStr        (       IDEResId( STR_DELETE ) ),
    m_sCreateLangStr    (       IDEResId( STR_CREATE_LANG ) )

{
    FreeResource();

    Init();
    FillLanguageBox();
    SelectHdl( NULL );
}

ManageLanguageDialog::~ManageLanguageDialog()
{
    ClearLanguageBox();
}

void ManageLanguageDialog::Init()
{
    // get current IDE
    BasicIDEShell* pIDEShell = IDE_DLL()->GetShell();
    String sLibName = pIDEShell->GetCurLibName();
    // set dialog title with library name
    String sText = GetText();
    sText.SearchAndReplace( String::CreateFromAscii("$1"), sLibName );
    SetText( sText );
    // set handler
    m_aAddPB.SetClickHdl( LINK( this, ManageLanguageDialog, AddHdl ) );
    m_aDeletePB.SetClickHdl( LINK( this, ManageLanguageDialog, DeleteHdl ) );
    m_aMakeDefPB.SetClickHdl( LINK( this, ManageLanguageDialog, MakeDefHdl ) );
    m_aLanguageLB.SetSelectHdl( LINK( this, ManageLanguageDialog, SelectHdl ) );

    m_aLanguageLB.EnableMultiSelection( sal_True );
    CalcInfoSize();
}

void ManageLanguageDialog::CalcInfoSize()
{
    String sInfoStr = m_aInfoFT.GetText();
    long nInfoWidth = m_aInfoFT.GetSizePixel().Width();
    long nLongWord = getLongestWordWidth( sInfoStr, m_aInfoFT );
    long nTxtWidth = m_aInfoFT.GetCtrlTextWidth( sInfoStr ) + nLongWord;
    long nLines = ( nTxtWidth / nInfoWidth ) + 1;
    if ( nLines > INFO_LINES_COUNT )
    {
        Size aFTSize = m_aLanguageFT.GetSizePixel();
        Size aSize = m_aInfoFT.GetSizePixel();
        long nNewHeight = aFTSize.Height() * nLines;
        long nDelta = nNewHeight - aSize.Height();
        aSize.Height() = nNewHeight;
        m_aInfoFT.SetSizePixel( aSize );

        aSize = m_aLanguageLB.GetSizePixel();
        aSize.Height() -= nDelta;
        m_aLanguageLB.SetSizePixel( aSize );

        Point aNewPos = m_aInfoFT.GetPosPixel();
        aNewPos.Y() -= nDelta;
        m_aInfoFT.SetPosPixel( aNewPos );
        aNewPos = m_aMakeDefPB.GetPosPixel();
        aNewPos.Y() -= nDelta;
        m_aMakeDefPB.SetPosPixel( aNewPos );
    }
}

void ManageLanguageDialog::FillLanguageBox()
{
    DBG_ASSERT( m_pLocalizationMgr, "ManageLanguageDialog::FillLanguageBox(): no localization manager" );

    if ( m_pLocalizationMgr->isLibraryLocalized() )
    {
        SvtLanguageTable aLangTable;
        Locale aDefaultLocale = m_pLocalizationMgr->getStringResourceManager()->getDefaultLocale();
        Sequence< Locale > aLocaleSeq = m_pLocalizationMgr->getStringResourceManager()->getLocales();
        const Locale* pLocale = aLocaleSeq.getConstArray();
        sal_Int32 i, nCount = aLocaleSeq.getLength();
        for ( i = 0;  i < nCount;  ++i )
        {
            bool bIsDefault = localesAreEqual( aDefaultLocale, pLocale[i] );
            LanguageType eLangType = SvxLocaleToLanguage( pLocale[i] );
            String sLanguage = aLangTable.GetString( eLangType );
            if ( bIsDefault )
            {
                sLanguage += ' ';
                sLanguage += m_sDefLangStr;
            }
            sal_uInt16 nPos = m_aLanguageLB.InsertEntry( sLanguage );
            m_aLanguageLB.SetEntryData( nPos, new LanguageEntry( sLanguage, pLocale[i], bIsDefault ) );
        }
    }
    else
        m_aLanguageLB.InsertEntry( m_sCreateLangStr );
}

void ManageLanguageDialog::ClearLanguageBox()
{
    sal_uInt16 i, nCount = m_aLanguageLB.GetEntryCount();
    for ( i = 0; i < nCount; ++i )
    {
        LanguageEntry* pEntry = (LanguageEntry*)( m_aLanguageLB.GetEntryData(i) );
        if ( pEntry )
            delete pEntry;
    }
    m_aLanguageLB.Clear();
}

IMPL_LINK( ManageLanguageDialog, AddHdl, Button *, EMPTYARG )
{
    SetDefaultLanguageDialog aDlg( this, m_pLocalizationMgr );
    if ( RET_OK == aDlg.Execute() )
    {
        // add new locales
        Sequence< Locale > aLocaleSeq = aDlg.GetLocales();
        m_pLocalizationMgr->handleAddLocales( aLocaleSeq );
        // update listbox
        ClearLanguageBox();
        FillLanguageBox();

        SfxBindings* pBindings = BasicIDE::GetBindingsPtr();
        if ( pBindings )
            pBindings->Invalidate( SID_BASICIDE_CURRENT_LANG );
    }
    return 1;
}

IMPL_LINK( ManageLanguageDialog, DeleteHdl, Button *, EMPTYARG )
{
    QueryBox aQBox( this, IDEResId( RID_QRYBOX_LANGUAGE ) );
    aQBox.SetButtonText( RET_OK, m_sDeleteStr );
    if ( aQBox.Execute() == RET_OK )
    {
        sal_uInt16 i, nCount = m_aLanguageLB.GetSelectEntryCount();
        sal_uInt16 nPos = m_aLanguageLB.GetSelectEntryPos();
        // remove locales
        Sequence< Locale > aLocaleSeq( nCount );
        for ( i = 0; i < nCount; ++i )
        {
            sal_uInt16 nSelPos = m_aLanguageLB.GetSelectEntryPos(i);
            LanguageEntry* pEntry = (LanguageEntry*)( m_aLanguageLB.GetEntryData( nSelPos ) );
            if ( pEntry )
                aLocaleSeq[i] = pEntry->m_aLocale;
        }
        m_pLocalizationMgr->handleRemoveLocales( aLocaleSeq );
        // update listbox
        ClearLanguageBox();
        FillLanguageBox();
        // reset selection
        nCount = m_aLanguageLB.GetEntryCount();
        if ( nCount <= nPos )
            nPos = nCount - 1;
        m_aLanguageLB.SelectEntryPos( nPos );
        SelectHdl( NULL );
    }
    return 1;
}

IMPL_LINK( ManageLanguageDialog, MakeDefHdl, Button *, EMPTYARG )
{
    sal_uInt16 nPos = m_aLanguageLB.GetSelectEntryPos();
    LanguageEntry* pSelectEntry = (LanguageEntry*)( m_aLanguageLB.GetEntryData( nPos ) );
    if ( pSelectEntry && !pSelectEntry->m_bIsDefault )
    {
        // set new default entry
        m_pLocalizationMgr->handleSetDefaultLocale( pSelectEntry->m_aLocale );
        // update Listbox
        ClearLanguageBox();
        FillLanguageBox();
        // reset selection
        m_aLanguageLB.SelectEntryPos( nPos );
        SelectHdl( NULL );
    }

    return 1;
}

IMPL_LINK( ManageLanguageDialog, SelectHdl, ListBox *, EMPTYARG )
{
    sal_uInt16 nCount = m_aLanguageLB.GetEntryCount();
    bool bEmpty = ( !nCount ||
                    m_aLanguageLB.GetEntryPos( m_sCreateLangStr ) != LISTBOX_ENTRY_NOTFOUND );
    bool bSelect = ( m_aLanguageLB.GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND );
    bool bEnable = ( !bEmpty && bSelect != false );

    m_aDeletePB.Enable( bEnable != false );
    m_aMakeDefPB.Enable( bEnable != false && nCount > 1 && m_aLanguageLB.GetSelectEntryCount() == 1 );

    return 1;
}

// class SetDefaultLanguageDialog -----------------------------------------------

SetDefaultLanguageDialog::SetDefaultLanguageDialog( Window* pParent, LocalizationMgr* _pLMgr ) :

    ModalDialog( pParent, IDEResId( RID_DLG_SETDEF_LANGUAGE ) ),

    m_aLanguageFT   ( this, IDEResId( FT_DEF_LANGUAGE ) ),
    m_pLanguageLB   ( new SvxLanguageBox( this, IDEResId( LB_DEF_LANGUAGE ) ) ),
    m_pCheckLangLB  ( NULL ),
    m_aInfoFT       ( this, IDEResId( FT_DEF_INFO ) ),
    m_aBtnLine      ( this, IDEResId( FL_DEF_BUTTONS ) ),
    m_aOKBtn        ( this, IDEResId( PB_DEF_OK ) ),
    m_aCancelBtn    ( this, IDEResId( PB_DEF_CANCEL ) ),
    m_aHelpBtn      ( this, IDEResId( PB_DEF_HELP ) ),

    m_pLocalizationMgr( _pLMgr )

{
    if ( m_pLocalizationMgr->isLibraryLocalized() )
    {
        // change to "Add Interface Language" mode
        SetHelpId( HID_BASICIDE_ADDNEW_LANGUAGE );
        m_pCheckLangLB = new SvxCheckListBox( this, IDEResId( LB_ADD_LANGUAGE ) );
        SetText( String( IDEResId( STR_ADDLANG_TITLE ) ) );
        m_aLanguageFT.SetText( String( IDEResId( STR_ADDLANG_LABEL ) ) );
        m_aInfoFT.SetText( String( IDEResId( STR_ADDLANG_INFO ) ) );
    }

    FreeResource();

    FillLanguageBox();
    CalcInfoSize();
}

SetDefaultLanguageDialog::~SetDefaultLanguageDialog()
{
    delete m_pLanguageLB;
    delete m_pCheckLangLB;
}

void SetDefaultLanguageDialog::FillLanguageBox()
{
    // fill list with all languages
    m_pLanguageLB->SetLanguageList( LANG_LIST_ALL, sal_False );
    // remove the already localized languages
    Sequence< Locale > aLocaleSeq = m_pLocalizationMgr->getStringResourceManager()->getLocales();
    const Locale* pLocale = aLocaleSeq.getConstArray();
    sal_Int32 i, nCount = aLocaleSeq.getLength();
    for ( i = 0;  i < nCount;  ++i )
        m_pLanguageLB->RemoveLanguage( SvxLocaleToLanguage( pLocale[i] ) );

    // fill checklistbox if not in default mode
    if ( m_pLocalizationMgr->isLibraryLocalized() )
    {
        sal_uInt16 j, nCount_ = m_pLanguageLB->GetEntryCount();
        for ( j = 0;  j < nCount_;  ++j )
        {
            m_pCheckLangLB->InsertEntry(
                m_pLanguageLB->GetEntry(j), LISTBOX_APPEND, m_pLanguageLB->GetEntryData(j) );
        }
        delete m_pLanguageLB;
        m_pLanguageLB = NULL;
    }
    else
        // preselect current UI language
        m_pLanguageLB->SelectLanguage( Application::GetSettings().GetUILanguage() );
}

void SetDefaultLanguageDialog::CalcInfoSize()
{
    String sInfoStr = m_aInfoFT.GetText();
    long nInfoWidth = m_aInfoFT.GetSizePixel().Width();
    long nLongWord = getLongestWordWidth( sInfoStr, m_aInfoFT );
    long nTxtWidth = m_aInfoFT.GetCtrlTextWidth( sInfoStr ) + nLongWord;
    long nLines = ( nTxtWidth / nInfoWidth ) + 1;
    if ( nLines > INFO_LINES_COUNT )
    {
        Size aFTSize = m_aLanguageFT.GetSizePixel();
        Size aSize = m_aInfoFT.GetSizePixel();
        long nNewHeight = aFTSize.Height() * nLines;
        long nDelta = nNewHeight - aSize.Height();
        aSize.Height() = nNewHeight;
        m_aInfoFT.SetSizePixel( aSize );

        Window* pWin = ( m_pLanguageLB != NULL ) ? dynamic_cast< Window* >( m_pLanguageLB )
                                                 : dynamic_cast< Window* >( m_pCheckLangLB );
        aSize = pWin->GetSizePixel();
        aSize.Height() -= nDelta;
        pWin->SetSizePixel( aSize );

        Point aNewPos = m_aInfoFT.GetPosPixel();
        aNewPos.Y() -= nDelta;
        m_aInfoFT.SetPosPixel( aNewPos );
    }
}

Sequence< Locale > SetDefaultLanguageDialog::GetLocales() const
{
    bool bNotLocalized = !m_pLocalizationMgr->isLibraryLocalized();
    sal_Int32 nSize = bNotLocalized ? 1 : m_pCheckLangLB->GetCheckedEntryCount();
    Sequence< Locale > aLocaleSeq( nSize );
    if ( bNotLocalized )
    {
        Locale aLocale;
        SvxLanguageToLocale( aLocale, m_pLanguageLB->GetSelectLanguage() );
        aLocaleSeq[0] = aLocale;
    }
    else
    {
        sal_uInt16 i, nCount = static_cast< sal_uInt16 >( m_pCheckLangLB->GetEntryCount() );
        sal_Int32 j = 0;
        for ( i = 0; i < nCount; ++i )
        {
            if ( m_pCheckLangLB->IsChecked(i) )
            {
                LanguageType eType = LanguageType( (sal_uLong)m_pCheckLangLB->GetEntryData(i) );
                Locale aLocale;
                SvxLanguageToLocale( aLocale, eType );
                aLocaleSeq[j++] = aLocale;
            }
        }
        DBG_ASSERT( nSize == j, "SetDefaultLanguageDialog::GetLocales(): invalid indexes" );
    }
    return aLocaleSeq;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
