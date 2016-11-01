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
#include <vcl/layout.hxx>
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

ManageLanguageDialog::ManageLanguageDialog(vcl::Window* pParent, std::shared_ptr<LocalizationMgr> xLMgr)
    : ModalDialog(pParent, "ManageLanguagesDialog", "modules/BasicIDE/ui/managelanguages.ui")
    , m_xLocalizationMgr(xLMgr)
    , m_sDefLangStr(IDE_RESSTR(RID_STR_DEF_LANG))
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
    SelectHdl( *m_pLanguageLB );
}

ManageLanguageDialog::~ManageLanguageDialog()
{
    disposeOnce();
}

void ManageLanguageDialog::dispose()
{
    ClearLanguageBox();
    m_pLanguageLB.clear();
    m_pAddPB.clear();
    m_pDeletePB.clear();
    m_pMakeDefPB.clear();
    ModalDialog::dispose();
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
        Locale aDefaultLocale = m_xLocalizationMgr->getStringResourceManager()->getDefaultLocale();
        Sequence< Locale > aLocaleSeq = m_xLocalizationMgr->getStringResourceManager()->getLocales();
        const Locale* pLocale = aLocaleSeq.getConstArray();
        sal_Int32 i, nCount = aLocaleSeq.getLength();
        for ( i = 0;  i < nCount;  ++i )
        {
            bool bIsDefault = localesAreEqual( aDefaultLocale, pLocale[i] );
            LanguageType eLangType = LanguageTag::convertToLanguageType( pLocale[i] );
            OUString sLanguage = SvtLanguageTable::GetLanguageString( eLangType );
            if ( bIsDefault )
            {
                sLanguage += " " + m_sDefLangStr;
            }
            const sal_Int32 nPos = m_pLanguageLB->InsertEntry( sLanguage );
            m_pLanguageLB->SetEntryData( nPos, new LanguageEntry( pLocale[i], bIsDefault ) );
        }
    }
    else
        m_pLanguageLB->InsertEntry( m_sCreateLangStr );
}

void ManageLanguageDialog::ClearLanguageBox()
{
    const sal_Int32 nCount = m_pLanguageLB->GetEntryCount();
    for ( sal_Int32 i = 0; i < nCount; ++i )
    {
        LanguageEntry* pEntry = static_cast<LanguageEntry*>(m_pLanguageLB->GetEntryData(i));
        delete pEntry;
    }
    m_pLanguageLB->Clear();
}

IMPL_LINK_NOARG_TYPED(ManageLanguageDialog, AddHdl, Button*, void)
{
    ScopedVclPtrInstance< SetDefaultLanguageDialog > aDlg( this, m_xLocalizationMgr );
    if ( RET_OK == aDlg->Execute() )
    {
        // add new locales
        Sequence< Locale > aLocaleSeq = aDlg->GetLocales();
        m_xLocalizationMgr->handleAddLocales( aLocaleSeq );
        // update listbox
        ClearLanguageBox();
        FillLanguageBox();

        if (SfxBindings* pBindings = GetBindingsPtr())
            pBindings->Invalidate( SID_BASICIDE_CURRENT_LANG );
    }
}

IMPL_LINK_NOARG_TYPED(ManageLanguageDialog, DeleteHdl, Button*, void)
{
    ScopedVclPtrInstance< MessageDialog > aQBox(this, "DeleteLangDialog", "modules/BasicIDE/ui/deletelangdialog.ui");
    if ( aQBox->Execute() == RET_OK )
    {
        sal_Int32 nCount = m_pLanguageLB->GetSelectEntryCount();
        sal_Int32 nPos = m_pLanguageLB->GetSelectEntryPos();
        // remove locales
        Sequence< Locale > aLocaleSeq( nCount );
        for ( sal_Int32 i = 0; i < nCount; ++i )
        {
            const sal_Int32 nSelPos = m_pLanguageLB->GetSelectEntryPos(i);
            LanguageEntry* pEntry = static_cast<LanguageEntry*>(m_pLanguageLB->GetEntryData( nSelPos ));
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
        SelectHdl( *m_pLanguageLB );
    }
}

IMPL_LINK_NOARG_TYPED(ManageLanguageDialog, MakeDefHdl, Button*, void)
{
    const sal_Int32 nPos = m_pLanguageLB->GetSelectEntryPos();
    LanguageEntry* pSelectEntry = static_cast<LanguageEntry*>(m_pLanguageLB->GetEntryData( nPos ));
    if ( pSelectEntry && !pSelectEntry->m_bIsDefault )
    {
        // set new default entry
        m_xLocalizationMgr->handleSetDefaultLocale( pSelectEntry->m_aLocale );
        // update Listbox
        ClearLanguageBox();
        FillLanguageBox();
        // reset selection
        m_pLanguageLB->SelectEntryPos( nPos );
        SelectHdl( *m_pLanguageLB );
    }
}

IMPL_LINK_NOARG_TYPED(ManageLanguageDialog, SelectHdl, ListBox&, void)
{
    const sal_Int32 nCount = m_pLanguageLB->GetEntryCount();
    bool bEmpty = ( !nCount ||
                    m_pLanguageLB->GetEntryPos( m_sCreateLangStr ) != LISTBOX_ENTRY_NOTFOUND );
    bool bSelect = ( m_pLanguageLB->GetSelectEntryPos() != LISTBOX_ENTRY_NOTFOUND );
    bool bEnable = !bEmpty && bSelect;

    m_pDeletePB->Enable(bEnable);
    m_pMakeDefPB->Enable(bEnable && nCount > 1 && m_pLanguageLB->GetSelectEntryCount() == 1);
}

// class SetDefaultLanguageDialog -----------------------------------------------

SetDefaultLanguageDialog::SetDefaultLanguageDialog(vcl::Window* pParent, std::shared_ptr<LocalizationMgr> xLMgr)
    : ModalDialog(pParent, "DefaultLanguageDialog", "modules/BasicIDE/ui/defaultlanguage.ui")
    , m_pCheckLangLB(nullptr)
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

SetDefaultLanguageDialog::~SetDefaultLanguageDialog()
{
    disposeOnce();
}

void SetDefaultLanguageDialog::dispose()
{
    m_pLanguageFT.clear();
    m_pLanguageLB.clear();
    m_pCheckLangFT.clear();
    m_pCheckLangLB.clear();
    m_pDefinedFT.clear();
    m_pAddedFT.clear();
    ModalDialog::dispose();
}

void SetDefaultLanguageDialog::FillLanguageBox()
{
    // fill list with all languages
    m_pLanguageLB->SetLanguageList( SvxLanguageListFlags::ALL, false );

    if ( m_xLocalizationMgr->isLibraryLocalized() )
    {
        // remove the already localized languages
        Sequence< Locale > aLocaleSeq = m_xLocalizationMgr->getStringResourceManager()->getLocales();
        const Locale* pLocale = aLocaleSeq.getConstArray();
        const sal_Int32 nCountLoc = aLocaleSeq.getLength();
        for ( sal_Int32 i = 0;  i < nCountLoc;  ++i )
            m_pLanguageLB->RemoveLanguage( LanguageTag::convertToLanguageType( pLocale[i] ) );

        // fill checklistbox if not in default mode
        const sal_Int32 nCountLang = m_pLanguageLB->GetEntryCount();
        for ( sal_Int32 j = 0;  j < nCountLang;  ++j )
        {
            m_pCheckLangLB->InsertEntry(
                m_pLanguageLB->GetEntry(j), LISTBOX_APPEND, m_pLanguageLB->GetEntryData(j) );
        }
        m_pLanguageLB = nullptr;
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
        const sal_Int32 nCount = m_pCheckLangLB->GetEntryCount();
        sal_Int32 j = 0;
        for ( sal_Int32 i = 0; i < nCount; ++i )
        {
            if ( m_pCheckLangLB->IsChecked(i) )
            {
                LanguageType eType = LanguageType( reinterpret_cast<sal_uLong>(m_pCheckLangLB->GetEntryData(i)) );
                aLocaleSeq[j++] = LanguageTag::convertToLocale( eType );
            }
        }
        DBG_ASSERT( nSize == j, "SetDefaultLanguageDialog::GetLocales(): invalid indexes" );
    }
    return aLocaleSeq;
}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
