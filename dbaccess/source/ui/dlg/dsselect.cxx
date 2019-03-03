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

#include "dsselect.hxx"
#include <dbu_dlg.hxx>

#include <com/sun/star/sdbcx/XCreateCatalog.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <stringconstants.hxx>
#include <dsitems.hxx>
#include <svl/stritem.hxx>
#include <svl/intitem.hxx>
#include <svl/eitem.hxx>
#include <svl/itemset.hxx>
#include <sal/log.hxx>

namespace dbaui
{
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::comphelper;

ODatasourceSelectDialog::ODatasourceSelectDialog(weld::Window* _pParent, const std::set<OUString>& _rDatasources)
    : GenericDialogController(_pParent, "dbaccess/ui/choosedatasourcedialog.ui", "ChooseDataSourceDialog")
    , m_xDatasource(m_xBuilder->weld_tree_view("treeview"))
    , m_xOk(m_xBuilder->weld_button("ok"))
    , m_xCancel(m_xBuilder->weld_button("cancel"))
    , m_xManageDatasources(m_xBuilder->weld_button("organize"))
{
    m_xDatasource->set_size_request(-1, m_xDatasource->get_height_rows(6));

    fillListBox(_rDatasources);
#ifdef HAVE_ODBC_ADMINISTRATION
    // allow ODBC datasource management
    m_xManageDatasources->show();
    m_xManageDatasources->set_sensitive(true);
    m_xManageDatasources->connect_clicked(LINK(this,ODatasourceSelectDialog,ManageClickHdl));
#endif
    m_xDatasource->connect_row_activated(LINK(this,ODatasourceSelectDialog,ListDblClickHdl));
}

ODatasourceSelectDialog::~ODatasourceSelectDialog()
{
}

IMPL_LINK(ODatasourceSelectDialog, ListDblClickHdl, weld::TreeView&, rListBox, void)
{
    if (rListBox.n_children())
        m_xDialog->response(RET_OK);
}

short ODatasourceSelectDialog::run()
{
    short nRet = GenericDialogController::run();
#ifdef HAVE_ODBC_ADMINISTRATION
    if (m_xODBCManagement.get())
        m_xODBCManagement->disableCallback();
#endif
    return nRet;
}

#ifdef HAVE_ODBC_ADMINISTRATION
IMPL_LINK_NOARG(ODatasourceSelectDialog, ManageClickHdl, weld::Button&, void)
{
    if ( !m_xODBCManagement.get() )
        m_xODBCManagement.reset( new OOdbcManagement( LINK( this, ODatasourceSelectDialog, ManageProcessFinished ) ) );

    if ( !m_xODBCManagement->manageDataSources_async() )
    {
        // TODO: error message
        m_xDatasource->grab_focus();
        m_xManageDatasources->set_sensitive(false);
        return;
    }

    m_xDatasource->set_sensitive(false);
    m_xOk->set_sensitive(false);
    m_xCancel->set_sensitive(false);
    m_xManageDatasources->set_sensitive(false);

    SAL_WARN_IF( !m_xODBCManagement->isRunning(), "dbaccess.ui", "ODatasourceSelectDialog::ManageClickHdl: success, but not running - you were *fast*!" );
}

IMPL_LINK_NOARG( ODatasourceSelectDialog, ManageProcessFinished, void*, void )
{
    m_xODBCManagement->receivedCallback();

    std::set<OUString> aOdbcDatasources;
    OOdbcEnumeration aEnumeration;
    aEnumeration.getDatasourceNames( aOdbcDatasources );
    fillListBox( aOdbcDatasources );

    m_xDatasource->set_sensitive(true);
    m_xOk->set_sensitive(true);
    m_xCancel->set_sensitive(true);
    m_xManageDatasources->set_sensitive(true);
}

#endif
void ODatasourceSelectDialog::fillListBox(const std::set<OUString>& _rDatasources)
{
    OUString sSelected;
    if (m_xDatasource->n_children())
         sSelected = m_xDatasource->get_selected_text();
    m_xDatasource->clear();
    // fill the list
    for (auto const& datasource : _rDatasources)
    {
        m_xDatasource->append_text(datasource);
    }

    if (m_xDatasource->n_children())
    {
        if (!sSelected.isEmpty())
            m_xDatasource->select_text(sSelected);
        else        // select the first entry
            m_xDatasource->select(0);
    }
}

}   // namespace dbaui

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
