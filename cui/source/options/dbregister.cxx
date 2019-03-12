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

#include <dbregister.hxx>
#include "dbregistersettings.hxx"
#include "connpooloptions.hxx"
#include <svl/filenotation.hxx>
#include <helpids.h>
#include <svtools/editbrowsebox.hxx>
#include <vcl/treelistentry.hxx>
#include <strings.hrc>
#include <bitmaps.hlst>
#include <vcl/field.hxx>
#include <vcl/weld.hxx>
#include <svl/eitem.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <vcl/svtabbx.hxx>
#include <svl/itemset.hxx>
#include "doclinkdialog.hxx"
#include <unotools/localfilehelper.hxx>
#include "optHeaderTabListbox.hxx"
#include <sfx2/docfilt.hxx>
#include <dialmgr.hxx>
#include "dbregisterednamesconfig.hxx"
#include <svx/dialogs.hrc>

#define TAB_WIDTH1      80

#define COL_TYPE       0

namespace svx
{


using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::uno;
using namespace ::svt;

// class RegistrationItemSetHolder  -------------------------------------------------

RegistrationItemSetHolder::RegistrationItemSetHolder( const SfxItemSet& _rMasterSet )
    :m_aRegistrationItems( _rMasterSet )
{
    DbRegisteredNamesConfig::GetOptions( m_aRegistrationItems );
}

RegistrationItemSetHolder::~RegistrationItemSetHolder()
{
}

// class DatabaseRegistrationDialog  ------------------------------------------------

DatabaseRegistrationDialog::DatabaseRegistrationDialog(weld::Window* pParent, const SfxItemSet& rInAttrs)
    : RegistrationItemSetHolder(rInAttrs)
    , SfxSingleTabDialogController(pParent, getRegistrationItems())
{
    TabPageParent aParent(get_content_area(), this);
    SetTabPage(DbRegistrationOptionsPage::Create(aParent, &getRegistrationItems()));
    m_xDialog->set_title(CuiResId(RID_SVXSTR_REGISTERED_DATABASES));
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

DbRegistrationOptionsPage::DbRegistrationOptionsPage(TabPageParent pParent, const SfxItemSet& rSet)
    : SfxTabPage(pParent, "cui/ui/dbregisterpage.ui", "DbRegisterPage", &rSet)
    , m_nOldCount(0)
    , m_bModified(false)
    , m_xNew(m_xBuilder->weld_button("new"))
    , m_xEdit(m_xBuilder->weld_button("edit"))
    , m_xDelete(m_xBuilder->weld_button("delete"))
    , m_xPathBox(m_xBuilder->weld_tree_view("pathctrl"))
    , m_xIter(m_xPathBox->make_iterator())
{
    Size aControlSize(248, 147);
    aControlSize = LogicToPixel(aControlSize, MapMode(MapUnit::MapAppFont));
    m_xPathBox->set_size_request(aControlSize.Width(), aControlSize.Height());

    std::vector<int> aWidths;
    aWidths.push_back(LogicToPixel(Size(TAB_WIDTH1, 0), MapMode(MapUnit::MapAppFont)).Width());
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
    disposeOnce();
}

void DbRegistrationOptionsPage::dispose()
{
    for (int i = 0, nCount = m_xPathBox->n_children(); i < nCount; ++i )
        delete reinterpret_cast<DatabaseRegistration*>(m_xPathBox->get_id(i).toInt64());
    SfxTabPage::dispose();
}

VclPtr<SfxTabPage> DbRegistrationOptionsPage::Create( TabPageParent pParent,
                                    const SfxItemSet* rAttrSet )
{
    return VclPtr<DbRegistrationOptionsPage>::Create( pParent, *rAttrSet );
}

bool DbRegistrationOptionsPage::FillItemSet( SfxItemSet* rCoreSet )
{
    // the settings for the single drivers
    bool bModified = false;
    DatabaseRegistrations aRegistrations;
    int nCount = m_xPathBox->n_children();
    for (int i = 0; i < nCount; ++i)
    {
        DatabaseRegistration* pRegistration = reinterpret_cast<DatabaseRegistration*>(m_xPathBox->get_id(i).toInt64());
        if ( pRegistration && !pRegistration->sLocation.isEmpty() )
        {
            OUString sName(m_xPathBox->get_text(i, 0));
            OFileNotation aTransformer( pRegistration->sLocation );
            aRegistrations[ sName ] = DatabaseRegistration( aTransformer.get( OFileNotation::N_URL ), pRegistration->bReadOnly );
        }
    }
    if ( m_nOldCount != aRegistrations.size() || m_bModified )
    {
        rCoreSet->Put(DatabaseMapItem( SID_SB_DB_REGISTER, aRegistrations ));
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
    if ( !aUserData.isEmpty() )
    {
        sal_Int32 nIdx {0};
        // restore column width
        std::vector<int> aWidths;
        aWidths.push_back(aUserData.getToken(0, ';', nIdx).toInt32());
        m_xPathBox->set_column_fixed_widths(aWidths);
        // restore sort direction
        bool bUp = aUserData.getToken(0, ';', nIdx).toInt32() != 0;
        m_xPathBox->set_sort_order(bUp);
        m_xPathBox->set_sort_indicator(bUp ? TRISTATE_TRUE : TRISTATE_FALSE, COL_TYPE);
    }
}

void DbRegistrationOptionsPage::FillUserData()
{
    OUString aUserData = OUString::number( m_xPathBox->get_column_width(COL_TYPE) ) + ";";
    bool bUp = m_xPathBox->get_sort_order();
    aUserData += (bUp ? OUString("1") : OUString("0"));
    SetUserData( aUserData );
}

IMPL_LINK_NOARG(DbRegistrationOptionsPage, DeleteHdl, weld::Button&, void)
{
    int nEntry = m_xPathBox->get_selected_index();
    if (nEntry != -1)
    {
        std::unique_ptr<weld::MessageDialog> xQuery(Application::CreateMessageDialog(GetDialogFrameWeld(),
                                                    VclMessageType::Question, VclButtonsType::YesNo, CuiResId(RID_SVXSTR_QUERY_DELETE_CONFIRM)));
        if (xQuery->run() == RET_YES)
            m_xPathBox->remove(nEntry);
    }
}

IMPL_LINK_NOARG(DbRegistrationOptionsPage, NewHdl, weld::Button&, void)
{
    openLinkDialog(OUString(),OUString());
}

IMPL_LINK_NOARG(DbRegistrationOptionsPage, PathBoxDoubleClickHdl, weld::TreeView&, void)
{
    EditHdl(*m_xEdit);
}

IMPL_LINK_NOARG(DbRegistrationOptionsPage, EditHdl, weld::Button&, void)
{
    int nEntry = m_xPathBox->get_selected_index();
    if (nEntry == -1)
        return;

    DatabaseRegistration* pOldRegistration = reinterpret_cast<DatabaseRegistration*>(m_xPathBox->get_id(nEntry).toInt64());
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
    DatabaseRegistration* pRegistration = reinterpret_cast<DatabaseRegistration*>(m_xPathBox->get_selected_id().toInt64());

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
    OUString sId(OUString::number(reinterpret_cast<sal_Int64>(new DatabaseRegistration(_sLocation, _bReadOnly))));
    m_xPathBox->insert(nullptr, -1, &_sName, &sId, nullptr, nullptr, nullptr, false, m_xIter.get());

    if (_bReadOnly)
        m_xPathBox->set_image(*m_xIter, RID_SVXBMP_LOCK);

    m_xPathBox->set_text(*m_xIter, _sLocation, 1);
}

void DbRegistrationOptionsPage::openLinkDialog(const OUString& sOldName, const OUString& sOldLocation, int nEntry)
{
    ODocumentLinkDialog aDlg(GetDialogFrameWeld(), nEntry == -1);

    aDlg.setLink(sOldName, sOldLocation);
    aDlg.setNameValidator(LINK( this, DbRegistrationOptionsPage, NameValidator ) );

    if (aDlg.run() == RET_OK)
    {
        OUString sNewName,sNewLocation;
        aDlg.getLink(sNewName,sNewLocation);
        if ( nEntry == -1 || sNewName != sOldName || sNewLocation != sOldLocation )
        {
            if (nEntry != -1)
            {
                delete reinterpret_cast<DatabaseRegistration*>(m_xPathBox->get_id(nEntry).toInt64());
                m_xPathBox->remove(nEntry);
            }
            insertNewEntry( sNewName, sNewLocation, false );
            m_bModified = true;
        }
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
