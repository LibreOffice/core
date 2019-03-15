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

#include <basidesh.hxx>
#include <basobj.hxx>
#include <iderdll.hxx>
#include <iderid.hxx>
#include <localizationmgr.hxx>
#include <managelang.hxx>

#include <strings.hrc>

#include <com/sun/star/i18n/Boundary.hpp>
#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/i18n/XBreakIterator.hpp>
#include <comphelper/sequence.hxx>
#include <editeng/unolingu.hxx>
#include <sfx2/bindings.hxx>
#include <svtools/langtab.hxx>
#include <svx/langbox.hxx>
#include <vcl/unohelp.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <vcl/settings.hxx>
#include <tools/debug.hxx>

namespace basctl
{

using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::resource;
using namespace ::com::sun::star::uno;

bool localesAreEqual( const Locale& rLocaleLeft, const Locale& rLocaleRight )
{
    bool bRet = ( rLocaleLeft.Language == rLocaleRight.Language &&
                  rLocaleLeft.Country == rLocaleRight.Country &&
                  rLocaleLeft.Variant == rLocaleRight.Variant );
    return bRet;
}

ManageLanguageDialog::ManageLanguageDialog(weld::Window* pParent, std::shared_ptr<LocalizationMgr> const & xLMgr)
    : GenericDialogController(pParent, "modules/BasicIDE/ui/managelanguages.ui", "ManageLanguagesDialog")
    , m_xLocalizationMgr(xLMgr)
    , m_sDefLangStr(IDEResId(RID_STR_DEF_LANG))
    , m_sCreateLangStr(IDEResId(RID_STR_CREATE_LANG))
    , m_xLanguageLB(m_xBuilder->weld_tree_view("treeview"))
    , m_xAddPB(m_xBuilder->weld_button("add"))
    , m_xDeletePB(m_xBuilder->weld_button("delete"))
    , m_xMakeDefPB(m_xBuilder->weld_button("default"))
{
    m_xLanguageLB->set_size_request(m_xLanguageLB->get_approximate_digit_width() * 42,
                                    m_xLanguageLB->get_height_rows(10));

    Init();
    FillLanguageBox();
    SelectHdl( *m_xLanguageLB );
}

ManageLanguageDialog::~ManageLanguageDialog()
{
    ClearLanguageBox();
}

void ManageLanguageDialog::Init()
{
    // get current IDE
    Shell* pShell = GetShell();
    const OUString& sLibName = pShell->GetCurLibName();
    // set dialog title with library name
    OUString sText = m_xDialog->get_title();
    sText = sText.replaceAll("$1", sLibName);
    m_xDialog->set_title(sText);
    // set handler
    m_xAddPB->connect_clicked( LINK( this, ManageLanguageDialog, AddHdl ) );
    m_xDeletePB->connect_clicked( LINK( this, ManageLanguageDialog, DeleteHdl ) );
    m_xMakeDefPB->connect_clicked( LINK( this, ManageLanguageDialog, MakeDefHdl ) );
    m_xLanguageLB->connect_changed( LINK( this, ManageLanguageDialog, SelectHdl ) );

    m_xLanguageLB->set_selection_mode(SelectionMode::Multiple);
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
            LanguageEntry* pEntry = new LanguageEntry(pLocale[i], bIsDefault);
            m_xLanguageLB->append(OUString::number(reinterpret_cast<sal_Int64>(pEntry)), sLanguage);
        }
    }
    else
        m_xLanguageLB->append_text(m_sCreateLangStr);
}

void ManageLanguageDialog::ClearLanguageBox()
{
    const sal_Int32 nCount = m_xLanguageLB->n_children();
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        LanguageEntry* pEntry = reinterpret_cast<LanguageEntry*>(m_xLanguageLB->get_id(i).toInt64());
        delete pEntry;
    }
    m_xLanguageLB->clear();
}

IMPL_LINK_NOARG(ManageLanguageDialog, AddHdl, weld::Button&, void)
{
    std::shared_ptr<SetDefaultLanguageDialog> xDlg(new SetDefaultLanguageDialog(m_xDialog.get(), m_xLocalizationMgr));
    weld::DialogController::runAsync(xDlg, [xDlg,this](sal_Int32 nResult)
        {
            if (!nResult )
                return;
            // add new locales
            Sequence< Locale > aLocaleSeq = xDlg->GetLocales();
            m_xLocalizationMgr->handleAddLocales( aLocaleSeq );
            // update listbox
            ClearLanguageBox();
            FillLanguageBox();

            if (SfxBindings* pBindings = GetBindingsPtr())
                pBindings->Invalidate( SID_BASICIDE_CURRENT_LANG );
        });
}

IMPL_LINK_NOARG(ManageLanguageDialog, DeleteHdl, weld::Button&, void)
{
    std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(m_xDialog.get(), "modules/BasicIDE/ui/deletelangdialog.ui"));
    std::unique_ptr<weld::MessageDialog> xQBox(xBuilder->weld_message_dialog("DeleteLangDialog"));
    if (xQBox->run() == RET_OK)
    {
        std::vector<int> aSelection = m_xLanguageLB->get_selected_rows();
        int nCount = aSelection.size();
        int nPos = m_xLanguageLB->get_selected_index();
        // remove locales
        Sequence< Locale > aLocaleSeq( nCount );
        for (int i = 0; i < nCount; ++i)
        {
            const sal_Int32 nSelPos = aSelection[i];
            LanguageEntry* pEntry = reinterpret_cast<LanguageEntry*>(m_xLanguageLB->get_id(nSelPos).toInt64());
            if ( pEntry )
                aLocaleSeq[i] = pEntry->m_aLocale;
        }
        m_xLocalizationMgr->handleRemoveLocales( aLocaleSeq );
        // update listbox
        ClearLanguageBox();
        FillLanguageBox();
        // reset selection
        nCount = m_xLanguageLB->n_children();
        if (nCount <= nPos)
            nPos = nCount - 1;
        m_xLanguageLB->select(nPos);
        SelectHdl( *m_xLanguageLB );
    }
}

IMPL_LINK_NOARG(ManageLanguageDialog, MakeDefHdl, weld::Button&, void)
{
    const sal_Int32 nPos = m_xLanguageLB->get_selected_index();
    LanguageEntry* pSelectEntry = reinterpret_cast<LanguageEntry*>(m_xLanguageLB->get_id(nPos).toInt64());
    if (pSelectEntry && !pSelectEntry->m_bIsDefault)
    {
        // set new default entry
        m_xLocalizationMgr->handleSetDefaultLocale( pSelectEntry->m_aLocale );
        // update Listbox
        ClearLanguageBox();
        FillLanguageBox();
        // reset selection
        m_xLanguageLB->select(nPos);
        SelectHdl( *m_xLanguageLB );
    }
}

IMPL_LINK_NOARG(ManageLanguageDialog, SelectHdl, weld::TreeView&, void)
{
    const sal_Int32 nCount = m_xLanguageLB->n_children();
    bool bEmpty = ( !nCount ||
                    m_xLanguageLB->find_text(m_sCreateLangStr) != -1 );
    bool bSelect = ( m_xLanguageLB->get_selected_index() != -1 );
    bool bEnable = !bEmpty && bSelect;

    m_xDeletePB->set_sensitive(bEnable);
    m_xMakeDefPB->set_sensitive(bEnable && nCount > 1 && m_xLanguageLB->count_selected_rows() == 1);
}

// class SetDefaultLanguageDialog -----------------------------------------------

SetDefaultLanguageDialog::SetDefaultLanguageDialog(weld::Window* pParent, std::shared_ptr<LocalizationMgr> const & xLMgr)
    : GenericDialogController(pParent, "modules/BasicIDE/ui/defaultlanguage.ui", "DefaultLanguageDialog")
    , m_xLocalizationMgr(xLMgr)
    , m_xLanguageFT(m_xBuilder->weld_label("defaultlabel"))
    , m_xLanguageLB(m_xBuilder->weld_tree_view("entries"))
    , m_xCheckLangFT(m_xBuilder->weld_label("checkedlabel"))
    , m_xCheckLangLB(m_xBuilder->weld_tree_view("checkedentries"))
    , m_xDefinedFT(m_xBuilder->weld_label("defined"))
    , m_xAddedFT(m_xBuilder->weld_label("added"))
    , m_xAltTitle(m_xBuilder->weld_label("alttitle"))
    , m_xLanguageCB(new LanguageBox(m_xBuilder->weld_combo_box("hidden")))
{
    m_xLanguageLB->set_size_request(-1, m_xLanguageLB->get_height_rows(10));
    m_xCheckLangLB->set_size_request(-1, m_xCheckLangLB->get_height_rows(10));
    std::vector<int> aWidths;
    aWidths.push_back(m_xCheckLangLB->get_checkbox_column_width());
    m_xCheckLangLB->set_column_fixed_widths(aWidths);

    if (m_xLocalizationMgr->isLibraryLocalized())
    {
        // change to "Add Interface Language" mode
        m_xLanguageLB->hide();
        m_xCheckLangLB->show();
        m_xDialog->set_title(m_xAltTitle->get_label());
        m_xLanguageFT->hide();
        m_xCheckLangFT->show();
        m_xDefinedFT->hide();
        m_xAddedFT->show();
    }

    FillLanguageBox();
}

SetDefaultLanguageDialog::~SetDefaultLanguageDialog()
{
}

void SetDefaultLanguageDialog::FillLanguageBox()
{
    // fill list with all languages
    m_xLanguageCB->SetLanguageList(SvxLanguageListFlags::ALL, false);

    if (m_xLocalizationMgr->isLibraryLocalized())
    {
        // remove the already localized languages
        Sequence< Locale > aLocaleSeq = m_xLocalizationMgr->getStringResourceManager()->getLocales();
        const Locale* pLocale = aLocaleSeq.getConstArray();
        const sal_Int32 nCountLoc = aLocaleSeq.getLength();
        for ( sal_Int32 i = 0;  i < nCountLoc;  ++i )
            m_xLanguageCB->remove_id(LanguageTag::convertToLanguageType(pLocale[i]));

        // fill checklistbox if not in default mode
        const sal_Int32 nCountLang = m_xLanguageCB->get_count();
        for (sal_Int32 j = 0;  j < nCountLang; ++j)
        {
            LanguageType eLang = m_xLanguageCB->get_id(j);
            m_xCheckLangLB->append();
            const int nRow = m_xCheckLangLB->n_children() - 1;
            m_xCheckLangLB->set_toggle(nRow, false, 0);
            m_xCheckLangLB->set_text(nRow, m_xLanguageCB->get_text(j), 1);
            m_xCheckLangLB->set_id(nRow, OUString::number(eLang.get()));
        }
        m_xLanguageCB.reset();
        m_xLanguageLB.reset();
    }
    else
    {
        const sal_Int32 nCountLang = m_xLanguageCB->get_count();
        for (sal_Int32 j = 0;  j < nCountLang; ++j)
        {
            LanguageType eLang = m_xLanguageCB->get_id(j);
            m_xLanguageLB->append(OUString::number(eLang.get()), m_xLanguageCB->get_text(j));
        }
        m_xLanguageCB.reset();

        // preselect current UI language
        m_xLanguageLB->select_id(OUString::number(Application::GetSettings().GetUILanguageTag().getLanguageType().get()));
    }
}

Sequence< Locale > SetDefaultLanguageDialog::GetLocales() const
{
    bool bNotLocalized = !m_xLocalizationMgr->isLibraryLocalized();
    if (bNotLocalized)
    {
        LanguageType eType = LanguageType(m_xLanguageLB->get_selected_id().toUInt32());
        Sequence<Locale> aLocaleSeq(1);
        aLocaleSeq[0] = LanguageTag(eType).getLocale();
        return aLocaleSeq;
    }
    std::vector<Locale> aLocaleSeq;
    const sal_Int32 nCount = m_xCheckLangLB->n_children();
    for (sal_Int32 i = 0; i < nCount; ++i)
    {
        if (m_xCheckLangLB->get_toggle(i, 0))
        {
            LanguageType eType = LanguageType(m_xCheckLangLB->get_id(i).toUInt32());
            aLocaleSeq.push_back(LanguageTag::convertToLocale(eType));
        }
    }
    return comphelper::containerToSequence(aLocaleSeq);
}

} // namespace basctl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
