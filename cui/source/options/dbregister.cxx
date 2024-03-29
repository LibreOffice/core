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

#include <sal/config.h>

#include <string_view>

#include <dbregister.hxx>
#include "dbregistersettings.hxx"
#include <o3tl/safeint.hxx>
#include <svl/filenotation.hxx>
#include <helpids.h>
#include <tools/debug.hxx>
#include <strings.hrc>
#include <bitmaps.hlst>
#include <utility>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <svl/itemset.hxx>
#include "doclinkdialog.hxx"
#include <dialmgr.hxx>
#include "dbregisterednamesconfig.hxx"
#include <svx/databaseregistrationui.hxx>
#include <o3tl/string_view.hxx>

#define COL_TYPE       0

namespace svx
{


using namespace ::com::sun::star::ui::dialogs;
using namespace ::svt;

// class RegistrationItemSetHolder  -------------------------------------------------

RegistrationItemSetHolder::RegistrationItemSetHolder( SfxItemSet _aMasterSet )
    :m_aRegistrationItems(std::move( _aMasterSet ))
{
    DbRegisteredNamesConfig::GetOptions( m_aRegistrationItems );
}

RegistrationItemSetHolder::~RegistrationItemSetHolder()
{
}

// class DatabaseRegistrationDialog  ------------------------------------------------

DatabaseRegistrationDialog::DatabaseRegistrationDialog(weld::Window* pParent, const SfxItemSet& rInAttrs)
    : RegistrationItemSetHolder(rInAttrs)
    , SfxSingleTabDialogController(pParent, &getRegistrationItems())
{
    SetTabPage(DbRegistrationOptionsPage::Create(get_content_area(), this, &getRegistrationItems()));
    m_xDialog->set_title(CuiResId(RID_CUISTR_REGISTERED_DATABASES));
}

short DatabaseRegistrationDialog::run()
{
    short result = SfxSingleTabDialogController::run();
    if (result == RET_OK)
    {
        DBG_ASSERT( GetOutputItemSet(), "DatabaseRegistrationDialog::Execute: no output items!" );
        if ( GetOutputItemSet() )
            DbRegisteredNamesConfig::SetOptions( *GetOutputItemSet() );
    }
    return result;
}

// class DbRegistrationOptionsPage --------------------------------------------------

DbRegistrationOptionsPage::DbRegistrationOptionsPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet)
    : SfxTabPage(pPage, pController, "cui/ui/dbregisterpage.ui", "DbRegisterPage", &rSet)
    , m_nOldCount(0)
    , m_bModified(false)
    , m_xNew(m_xBuilder->weld_button("new"))
    , m_xEdit(m_xBuilder->weld_button("edit"))
    , m_xDelete(m_xBuilder->weld_button("delete"))
    , m_xPathBox(m_xBuilder->weld_tree_view("pathctrl"))
    , m_xIter(m_xPathBox->make_iterator())
{
    Size aControlSize(m_xPathBox->get_approximate_digit_width() * 60,
                      m_xPathBox->get_height_rows(12));
    m_xPathBox->set_size_request(aControlSize.Width(), aControlSize.Height());

    std::vector<int> aWidths
    {
        o3tl::narrowing<int>(m_xPathBox->get_approximate_digit_width() * 20)
    };
    m_xPathBox->set_column_fixed_widths(aWidths);

    m_xNew->connect_clicked( LINK( this, DbRegistrationOptionsPage, NewHdl ) );
    m_xEdit->connect_clicked( LINK( this, DbRegistrationOptionsPage, EditHdl ) );
    m_xDelete->connect_clicked( LINK( this, DbRegistrationOptionsPage, DeleteHdl ) );

    m_xPathBox->connect_column_clicked(LINK(this, DbRegistrationOptionsPage, HeaderSelect_Impl));

    m_xPathBox->make_sorted();
    m_xPathBox->connect_row_activated( LINK( this, DbRegistrationOptionsPage, PathBoxDoubleClickHdl ) );
    m_xPathBox->connect_changed( LINK( this, DbRegistrationOptionsPage, PathSelect_Impl ) );

    m_xPathBox->set_help_id(HID_DBPATH_CTL_PATH);
}

DbRegistrationOptionsPage::~DbRegistrationOptionsPage()
{
    for (int i = 0, nCount = m_xPathBox->n_children(); i < nCount; ++i )
        delete weld::fromId<DatabaseRegistration*>(m_xPathBox->get_id(i));
}

std::unique_ptr<SfxTabPage> DbRegistrationOptionsPage::Create( weld::Container* pPage, weld::DialogController* pController,
                                    const SfxItemSet* rAttrSet )
{
    return std::make_unique<DbRegistrationOptionsPage>(pPage, pController, *rAttrSet);
}

OUString DbRegistrationOptionsPage::GetAllStrings()
{
    OUString sAllStrings;

    if (const auto& pString = m_xBuilder->weld_label("label1"))
        sAllStrings += pString->get_label() + " ";

    return sAllStrings.replaceAll("_", "");
}

bool DbRegistrationOptionsPage::FillItemSet( SfxItemSet* rCoreSet )
{
    // the settings for the single drivers
    bool bModified = false;
    DatabaseRegistrations aRegistrations;
    int nCount = m_xPathBox->n_children();
    for (int i = 0; i < nCount; ++i)
    {
        DatabaseRegistration* pRegistration = weld::fromId<DatabaseRegistration*>(m_xPathBox->get_id(i));
        if ( pRegistration && !pRegistration->sLocation.isEmpty() )
        {
            OUString sName(m_xPathBox->get_text(i, 0));
            OFileNotation aTransformer( pRegistration->sLocation );
            aRegistrations[ sName ] = DatabaseRegistration( aTransformer.get( OFileNotation::N_URL ), pRegistration->bReadOnly );
        }
    }
    if ( m_nOldCount != aRegistrations.size() || m_bModified )
    {
        rCoreSet->Put(DatabaseMapItem( SID_SB_DB_REGISTER, std::move(aRegistrations) ));
        bModified = true;
    }

    return bModified;
}

void DbRegistrationOptionsPage::Reset( const SfxItemSet* rSet )
{
    // the settings for the single drivers
    const DatabaseMapItem* pRegistrations = rSet->GetItem<DatabaseMapItem>(SID_SB_DB_REGISTER);
    if ( !pRegistrations )
        return;

    m_xPathBox->clear();

    const DatabaseRegistrations& rRegistrations = pRegistrations->getRegistrations();
    m_nOldCount = rRegistrations.size();
    for (auto const& elem : rRegistrations)
    {
        OFileNotation aTransformer( elem.second.sLocation );
        insertNewEntry( elem.first, aTransformer.get( OFileNotation::N_SYSTEM ), elem.second.bReadOnly );
    }

    OUString aUserData = GetUserData();
    if ( aUserData.isEmpty() )
        return;

    sal_Int32 nIdx {0};
    // restore column width
    std::vector<int> aWidths
    {
        o3tl::toInt32(o3tl::getToken(aUserData, 0, ';', nIdx))
    };
    m_xPathBox->set_column_fixed_widths(aWidths);
    // restore sort direction
    bool bUp = o3tl::toInt32(o3tl::getToken(aUserData, 0, ';', nIdx)) != 0;
    m_xPathBox->set_sort_order(bUp);
    m_xPathBox->set_sort_indicator(bUp ? TRISTATE_TRUE : TRISTATE_FALSE, COL_TYPE);
}

void DbRegistrationOptionsPage::FillUserData()
{
    OUString aUserData = OUString::number( m_xPathBox->get_column_width(COL_TYPE) ) + ";";
    bool bUp = m_xPathBox->get_sort_order();
    aUserData += (bUp ? std::u16string_view(u"1") : std::u16string_view(u"0"));
    SetUserData( aUserData );
}

IMPL_LINK_NOARG(DbRegistrationOptionsPage, DeleteHdl, weld::Button&, void)
{
    int nEntry = m_xPathBox->get_selected_index();
    if (nEntry != -1)
    {
        std::unique_ptr<weld::MessageDialog> xQuery(Application::CreateMessageDialog(GetFrameWeld(),
                                                    VclMessageType::Question, VclButtonsType::YesNo, CuiResId(RID_CUISTR_QUERY_DELETE_CONFIRM)));
        if (xQuery->run() == RET_YES)
            m_xPathBox->remove(nEntry);
    }
}

IMPL_LINK_NOARG(DbRegistrationOptionsPage, NewHdl, weld::Button&, void)
{
    openLinkDialog(OUString(),OUString());
}

IMPL_LINK_NOARG(DbRegistrationOptionsPage, PathBoxDoubleClickHdl, weld::TreeView&, bool)
{
    EditHdl(*m_xEdit);
    return true;
}

IMPL_LINK_NOARG(DbRegistrationOptionsPage, EditHdl, weld::Button&, void)
{
    int nEntry = m_xPathBox->get_selected_index();
    if (nEntry == -1)
        return;

    DatabaseRegistration* pOldRegistration = weld::fromId<DatabaseRegistration*>(m_xPathBox->get_id(nEntry));
    if (!pOldRegistration || pOldRegistration->bReadOnly)
        return;

    OUString sOldName = m_xPathBox->get_text(nEntry, 0);
    openLinkDialog(sOldName, pOldRegistration->sLocation, nEntry);
}

IMPL_LINK( DbRegistrationOptionsPage, HeaderSelect_Impl, int, nCol, void )
{
    if (nCol != COL_TYPE)
        return;

    bool bSortMode = m_xPathBox->get_sort_order();

    //set new arrow positions in headerbar
    bSortMode = !bSortMode;
    m_xPathBox->set_sort_order(bSortMode);

    //sort lists
    m_xPathBox->set_sort_indicator(bSortMode ? TRISTATE_TRUE : TRISTATE_FALSE, nCol);
}

IMPL_LINK_NOARG(DbRegistrationOptionsPage, PathSelect_Impl, weld::TreeView&, void)
{
    DatabaseRegistration* pRegistration = weld::fromId<DatabaseRegistration*>(m_xPathBox->get_selected_id());

    bool bReadOnly = true;
    if (pRegistration)
    {
        bReadOnly = pRegistration->bReadOnly;
    }

    m_xEdit->set_sensitive( !bReadOnly );
    m_xDelete->set_sensitive( !bReadOnly );
}

void DbRegistrationOptionsPage::insertNewEntry(const OUString& _sName,const OUString& _sLocation, const bool _bReadOnly)
{
    OUString sId(weld::toId(new DatabaseRegistration(_sLocation, _bReadOnly)));
    m_xPathBox->insert(nullptr, -1, &_sName, &sId, nullptr, nullptr, false, m_xIter.get());

    if (_bReadOnly)
        m_xPathBox->set_image(*m_xIter, RID_SVXBMP_LOCK);

    m_xPathBox->set_text(*m_xIter, _sLocation, 1);
}

void DbRegistrationOptionsPage::openLinkDialog(const OUString& sOldName, const OUString& sOldLocation, int nEntry)
{
    ODocumentLinkDialog aDlg(GetFrameWeld(), nEntry == -1);

    aDlg.setLink(sOldName, sOldLocation);

    // tdf#149195: control the name (ie check duplicate) only if new entry case
    if (nEntry == -1)
        aDlg.setNameValidator(LINK( this, DbRegistrationOptionsPage, NameValidator ) );

    if (aDlg.run() != RET_OK)
        return;

    OUString sNewName,sNewLocation;
    aDlg.getLink(sNewName,sNewLocation);
    if ( nEntry == -1 || sNewName != sOldName || sNewLocation != sOldLocation )
    {
        if (nEntry != -1)
        {
            delete weld::fromId<DatabaseRegistration*>(m_xPathBox->get_id(nEntry));
            m_xPathBox->remove(nEntry);
        }
        insertNewEntry( sNewName, sNewLocation, false );
        m_bModified = true;
    }
}

IMPL_LINK( DbRegistrationOptionsPage, NameValidator, const OUString&, _rName, bool )
{
    int nCount = m_xPathBox->n_children();
    for (int i = 0; i < nCount; ++i)
    {
        if (m_xPathBox->get_text(i, 0) == _rName)
            return false;
    }
    return true;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
