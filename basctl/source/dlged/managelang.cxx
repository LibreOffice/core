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

#include "basidesh.hxx"
#include "basobj.hxx"
#include "iderdll.hxx"
#include "iderid.hxx"
#include "localizationmgr.hxx"
#include "managelang.hxx"

#include "dlgresid.hrc"
#include "helpid.hrc"
#include "managelang.hrc"

#include <com/sun/star/i18n/Boundary.hpp>
#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <editeng/unolingu.hxx>
#include <sfx2/bindings.hxx>
#include <svtools/langtab.hxx>
#include <svx/langbox.hxx>
#include <vcl/unohelp.hxx>
#include <vcl/svapp.hxx>
#include <vcl/msgbox.hxx>

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
    long getLongestWordWidth( const ::rtl::OUString& rText, const Window& rWin )
    {
        long nWidth = 0;
        Reference< XBreakIterator > xBreakIter( vcl::unohelper::CreateBreakIterator() );
        sal_Int32 nStartPos = 0;
        const Locale aLocale = Application::GetSettings().GetUILocale();
        Boundary aBoundary = xBreakIter->getWordBoundary(
            rText, nStartPos, aLocale, WordType::ANYWORD_IGNOREWHITESPACES, true );

        while ( aBoundary.startPos != aBoundary.endPos )
        {
            nStartPos = aBoundary.endPos;
            ::rtl::OUString sWord(rText.copy(aBoundary.startPos, aBoundary.endPos - aBoundary.startPos));
            long nTemp = rWin.GetCtrlTextWidth( sWord );
            if ( nTemp > nWidth )
                nWidth = nTemp;
            aBoundary = xBreakIter->nextWord(
                rText, nStartPos, aLocale, WordType::ANYWORD_IGNOREWHITESPACES );
        }

        return nWidth;
    }
}

ManageLanguageDialog::ManageLanguageDialog( Window* pParent, boost::shared_ptr<LocalizationMgr> _pLMgr ) :
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
    m_sDefLangStr       (IDE_RESSTR(STR_DEF_LANG)),
    m_sDeleteStr        (IDE_RESSTR(STR_DELETE)),
    m_sCreateLangStr    (IDE_RESSTR(STR_CREATE_LANG))
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
    BasicIDEShell* pIDEShell = BasicIDEGlobals::GetShell();
    ::rtl::OUString sLibName = pIDEShell->GetCurLibName();
    // set dialog title with library name
    ::rtl::OUString sText = GetText();
    sText = sText.replaceAll("$1", sLibName);
    SetText( sText );
    // set handler
    m_aAddPB.SetClickHdl( LINK( this, ManageLanguageDialog, AddHdl ) );
    m_aDeletePB.SetClickHdl( LINK( this, ManageLanguageDialog, DeleteHdl ) );
    m_aMakeDefPB.SetClickHdl( LINK( this, ManageLanguageDialog, MakeDefHdl ) );
    m_aLanguageLB.SetSelectHdl( LINK( this, ManageLanguageDialog, SelectHdl ) );

    m_aLanguageLB.EnableMultiSelection( true );
    CalcInfoSize();
}

void ManageLanguageDialog::CalcInfoSize()
{
    ::rtl::OUString sInfoStr = m_aInfoFT.GetText();
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
            ::rtl::OUStringBuffer sLanguageBuf(aLangTable.GetString( eLangType ));
            if ( bIsDefault )
            {
                sLanguageBuf.append(' ');
                sLanguageBuf.append(m_sDefLangStr);
            }
            ::rtl::OUString sLanguage(sLanguageBuf.makeStringAndClear());
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
        delete pEntry;
    }
    m_aLanguageLB.Clear();
}

IMPL_LINK_NOARG(ManageLanguageDialog, AddHdl)
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

IMPL_LINK_NOARG(ManageLanguageDialog, DeleteHdl)
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

IMPL_LINK_NOARG(ManageLanguageDialog, MakeDefHdl)
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

IMPL_LINK_NOARG(ManageLanguageDialog, SelectHdl)
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

SetDefaultLanguageDialog::SetDefaultLanguageDialog( Window* pParent, boost::shared_ptr<LocalizationMgr> _pLMgr ) :

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
        SetText( IDE_RESSTR(STR_ADDLANG_TITLE) );
        m_aLanguageFT.SetText( IDE_RESSTR(STR_ADDLANG_LABEL) );
        m_aInfoFT.SetText( IDE_RESSTR(STR_ADDLANG_INFO) );
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
    m_pLanguageLB->SetLanguageList( LANG_LIST_ALL, false );
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
    ::rtl::OUString sInfoStr = m_aInfoFT.GetText();
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
