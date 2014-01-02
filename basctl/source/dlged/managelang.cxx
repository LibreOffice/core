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
#include <vcl/settings.hxx>

namespace basctl
{

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

ManageLanguageDialog::ManageLanguageDialog(Window* pParent, boost::shared_ptr<LocalizationMgr> xLMgr)
    : ModalDialog(pParent, "ManageLanguagesDialog", "modules/BasicIDE/ui/managelanguages.ui")
    , m_xLocalizationMgr(xLMgr)
    , m_sDefLangStr(IDE_RESSTR(RID_STR_DEF_LANG))
    , m_sDeleteStr(IDE_RESSTR(RID_STR_DELETE))
    , m_sCreateLangStr(IDE_RESSTR(RID_STR_CREATE_LANG))
{
    get(m_pLanguageLB, "treeview");
    m_pLanguageLB->set_height_request(m_pLanguageLB->GetTextHeight() * 10);
    m_pLanguageLB->set_width_request(m_pLanguageLB->approximate_char_width() * 50);
    get(m_pAddPB, "add");
    get(m_pDeletePB, "delete");
    get(m_pMakeDefPB, "default");

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
    Shell* pShell = GetShell();
    OUString sLibName = pShell->GetCurLibName();
    // set dialog title with library name
    OUString sText = GetText();
    sText = sText.replaceAll("$1", sLibName);
    SetText( sText );
    // set handler
    m_pAddPB->SetClickHdl( LINK( this, ManageLanguageDialog, AddHdl ) );
    m_pDeletePB->SetClickHdl( LINK( this, ManageLanguageDialog, DeleteHdl ) );
    m_pMakeDefPB->SetClickHdl( LINK( this, ManageLanguageDialog, MakeDefHdl ) );
    m_pLanguageLB->SetSelectHdl( LINK( this, ManageLanguageDialog, SelectHdl ) );

    m_pLanguageLB->EnableMultiSelection( true );
}

void ManageLanguageDialog::FillLanguageBox()
{
    DBG_ASSERT( m_xLocalizationMgr, "ManageLanguageDialog::FillLanguageBox(): no localization manager" );

    if ( m_xLocalizationMgr->isLibraryLocalized() )
    {
        SvtLanguageTable aLangTable;
        Locale aDefaultLocale = m_xLocalizationMgr->getStringResourceManager()->getDefaultLocale();
        Sequence< Locale > aLocaleSeq = m_xLocalizationMgr->getStringResourceManager()->getLocales();
        const Locale* pLocale = aLocaleSeq.getConstArray();
        sal_Int32 i, nCount = aLocaleSeq.getLength();
        for ( i = 0;  i < nCount;  ++i )
        {
            bool bIsDefault = localesAreEqual( aDefaultLocale, pLocale[i] );
            LanguageType eLangType = LanguageTag::convertToLanguageType( pLocale[i] );
            OUString sLanguage = aLangTable.GetString( eLangType );
            if ( bIsDefault )
            {
                sLanguage += " " + m_sDefLangStr;
            }
            sal_uInt16 nPos = m_pLanguageLB->InsertEntry( sLanguage );
            m_pLanguageLB->SetEntryData( nPos, new LanguageEntry( sLanguage, pLocale[i], bIsDefault ) );
        }
    }
    else
        m_pLanguageLB->InsertEntry( m_sCreateLangStr );
}

void ManageLanguageDialog::ClearLanguageBox()
{
    sal_uInt16 i, nCount = m_pLanguageLB->GetEntryCount();
    for ( i = 0; i < nCount; ++i )
    {
        LanguageEntry* pEntry = (LanguageEntry*)( m_pLanguageLB->GetEntryData(i) );
        delete pEntry;
    }
    m_pLanguageLB->Clear();
}

IMPL_LINK_NOARG(ManageLanguageDialog, AddHdl)
{
    SetDefaultLanguageDialog aDlg( this, m_xLocalizationMgr );
    if ( RET_OK == aDlg.Execute() )
    {
        // add new locales
        Sequence< Locale > aLocaleSeq = aDlg.GetLocales();
        m_xLocalizationMgr->handleAddLocales( aLocaleSeq );
        // update listbox
        ClearLanguageBox();
        FillLanguageBox();

        if (SfxBindings* pBindings = GetBindingsPtr())
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
        sal_uInt16 i, nCount = m_pLanguageLB->GetSelectEntryCount();
        sal_uInt16 nPos = m_pLanguageLB->GetSelectEntryPos();
        // remove locales
        Sequence< Locale > aLocaleSeq( nCount );
        for ( i = 0; i < nCount; ++i )
        {
            sal_uInt16 nSelPos = m_pLanguageLB->GetSelectEntryPos(i);
            LanguageEntry* pEntry = (LanguageEntry*)( m_pLanguageLB->GetEntryData( nSelPos ) );
            if ( pEntry )
                aLocaleSeq[i] = pEntry->m_aLocale;
        }
        m_xLocalizationMgr->handleRemoveLocales( aLocaleSeq );
        // update listbox
        ClearLanguageBox();
        FillLanguageBox();
        // reset selection
        nCount = m_pLanguageLB->GetEntryCount();
        if ( nCount <= nPos )
            nPos = nCount - 1;
        m_pLanguageLB->SelectEntryPos( nPos );
        SelectHdl( NULL );
    }
    return 1;
}

IMPL_LINK_NOARG(ManageLanguageDialog, MakeDefHdl)
{
    sal_uInt16 nPos = m_pLanguageLB->GetSelectEntryPos();
    LanguageEntry* pSelectEntry = (LanguageEntry*)( m_pLanguageLB->GetEntryData( nPos ) );
    if ( pSelectEntry && !pSelectEntry->m_bIsDefault )
    {
        // set new default entry
        m_xLocalizationMgr->handleSetDefaultLocale( pSelectEntry->m_aLocale );
        // update Listbox
        ClearLanguageBox();
        FillLanguageBox();
        // reset selection
        m_pLanguageLB->SelectEntryPos( nPos );
        SelectHdl( NULL );
    }

    return 1;
}

IMPL_LINK_NOARG(ManageLanguageDialog, SelectHdl)
{
    sal_uInt16 nCount = m_pLanguageLB->GetEntryCount();
    bool bEmpty = ( !nCount ||
                    m_pLanguageLB->GetEntryPos( m_sCreateLangStr ) != LISTBOX_ENTRY_NOTFOUND );
    bool bSelect = ( m_pLanguageLB->GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND );
    bool bEnable = ( !bEmpty && bSelect != false );

    m_pDeletePB->Enable( bEnable != false );
    m_pMakeDefPB->Enable( bEnable != false && nCount > 1 && m_pLanguageLB->GetSelectEntryCount() == 1 );

    return 1;
}

// class SetDefaultLanguageDialog -----------------------------------------------

SetDefaultLanguageDialog::SetDefaultLanguageDialog(Window* pParent, boost::shared_ptr<LocalizationMgr> xLMgr)
    : ModalDialog(pParent, "DefaultLanguageDialog", "modules/BasicIDE/ui/defaultlanguage.ui")
    , m_pCheckLangLB(NULL)
    , m_xLocalizationMgr(xLMgr)
{
    get(m_pLanguageLB, "entries");
    get(m_pCheckLangLB, "checkedentries");
    get(m_pDefinedFT, "defined");
    get(m_pAddedFT, "added");
    get(m_pLanguageFT, "defaultlabel");
    get(m_pCheckLangFT, "checkedlabel");

    m_pLanguageLB->set_height_request(m_pLanguageLB->GetTextHeight() * 10);
    m_pCheckLangLB->set_height_request(m_pCheckLangLB->GetTextHeight() * 10);

    if (m_xLocalizationMgr->isLibraryLocalized())
    {
        // change to "Add Interface Language" mode
        m_pLanguageLB->Hide();
        m_pCheckLangLB->Show();
        SetText(get<FixedText>("alttitle")->GetText());
        m_pLanguageFT->Hide();
        m_pCheckLangFT->Show();
        m_pDefinedFT->Hide();
        m_pAddedFT->Show();
    }

    FillLanguageBox();
}

void SetDefaultLanguageDialog::FillLanguageBox()
{
    // fill list with all languages
    m_pLanguageLB->SetLanguageList( LANG_LIST_ALL, false );
    // remove the already localized languages
    Sequence< Locale > aLocaleSeq = m_xLocalizationMgr->getStringResourceManager()->getLocales();
    const Locale* pLocale = aLocaleSeq.getConstArray();
    sal_Int32 i, nCount = aLocaleSeq.getLength();
    for ( i = 0;  i < nCount;  ++i )
        m_pLanguageLB->RemoveLanguage( LanguageTag::convertToLanguageType( pLocale[i] ) );

    // fill checklistbox if not in default mode
    if ( m_xLocalizationMgr->isLibraryLocalized() )
    {
        sal_uInt16 j, nCount_ = m_pLanguageLB->GetEntryCount();
        for ( j = 0;  j < nCount_;  ++j )
        {
            m_pCheckLangLB->InsertEntry(
                m_pLanguageLB->GetEntry(j), LISTBOX_APPEND, m_pLanguageLB->GetEntryData(j) );
        }
        m_pLanguageLB = NULL;
    }
    else
        // preselect current UI language
        m_pLanguageLB->SelectLanguage( Application::GetSettings().GetUILanguageTag().getLanguageType() );
}

Sequence< Locale > SetDefaultLanguageDialog::GetLocales() const
{
    bool bNotLocalized = !m_xLocalizationMgr->isLibraryLocalized();
    sal_Int32 nSize = bNotLocalized ? 1 : m_pCheckLangLB->GetCheckedEntryCount();
    Sequence< Locale > aLocaleSeq( nSize );
    if ( bNotLocalized )
    {
        aLocaleSeq[0] = LanguageTag( m_pLanguageLB->GetSelectLanguage() ).getLocale();
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
                aLocaleSeq[j++] = LanguageTag::convertToLocale( eType );
            }
        }
        DBG_ASSERT( nSize == j, "SetDefaultLanguageDialog::GetLocales(): invalid indexes" );
    }
    return aLocaleSeq;
}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
