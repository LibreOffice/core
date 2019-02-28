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

#include <swtypes.hxx>
#include "selectdbtabledialog.hxx"
#include "dbtablepreviewdialog.hxx"
#include <svtools/simptabl.hxx>
#include <vcl/treelistentry.hxx>
#include <osl/diagnose.h>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>

#include <unomid.h>

#include <dbui.hrc>
#include <strings.hrc>

using namespace ::com::sun::star;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;

SwSelectDBTableDialog::SwSelectDBTableDialog(weld::Window* pParent,
        const uno::Reference< sdbc::XConnection>& rConnection)
    : SfxDialogController(pParent, "modules/swriter/ui/selecttabledialog.ui", "SelectTableDialog")
    , m_xConnection(rConnection)
    , m_xTable(m_xBuilder->weld_tree_view("table"))
    , m_xPreviewPB(m_xBuilder->weld_button("preview"))
{
    m_xTable->set_size_request(m_xTable->get_approximate_digit_width() * 60,
                               m_xTable->get_height_rows(6));

    std::vector<int> aWidths;
    aWidths.push_back(m_xTable->get_approximate_digit_width() * 30);
    m_xTable->set_column_fixed_widths(aWidths);

    m_xPreviewPB->connect_clicked(LINK(this, SwSelectDBTableDialog, PreviewHdl));

    Reference<XTablesSupplier> xTSupplier(m_xConnection, UNO_QUERY);
    if (xTSupplier.is())
    {
        Reference<XNameAccess> xTables = xTSupplier->getTables();
        Sequence<OUString> aTables = xTables->getElementNames();
        const OUString* pTables = aTables.getConstArray();
        for (int i = 0; i < aTables.getLength(); i++)
        {
            OUString sEntry = pTables[i];
            m_xTable->append_text(sEntry);
            m_xTable->set_text(i, SwResId(ST_TABLE), 1);
        }
    }
    Reference<XQueriesSupplier> xQSupplier(m_xConnection, UNO_QUERY);
    if (xQSupplier.is())
    {
        Reference<XNameAccess> xQueries = xQSupplier->getQueries();
        Sequence<OUString> aQueries = xQueries->getElementNames();
        const OUString* pQueries = aQueries.getConstArray();
        int nPos = m_xTable->n_children();
        for (sal_Int32 i = 0; i < aQueries.getLength(); i++)
        {
            OUString sEntry = pQueries[i];
            m_xTable->append_text(sEntry);
            m_xTable->set_text(nPos, SwResId(ST_QUERY), 1);
            m_xTable->set_id(nPos, OUString::number(1));
            ++nPos;
        }
    }
}

SwSelectDBTableDialog::~SwSelectDBTableDialog()
{
}

IMPL_LINK_NOARG(SwSelectDBTableDialog, PreviewHdl, weld::Button&, void)
{
    int nEntry = m_xTable->get_selected_index();
    if (nEntry == -1)
        return;

    OUString sTableOrQuery = m_xTable->get_text(nEntry, 0);
    sal_Int32 nCommandType = m_xTable->get_id(nEntry).isEmpty() ? 0 : 1;

    OUString sDataSourceName;
    Reference<XChild> xChild(m_xConnection, UNO_QUERY);
    if(xChild.is())
    {
        Reference<XDataSource> xSource(xChild->getParent(), UNO_QUERY);
        Reference<XPropertySet> xPrSet(xSource, UNO_QUERY);
        xPrSet->getPropertyValue("Name") >>= sDataSourceName;
    }
    OSL_ENSURE(!sDataSourceName.isEmpty(), "no data source found");
    Sequence<PropertyValue> aProperties(5);
    PropertyValue* pProperties = aProperties.getArray();
    pProperties[0].Name = "DataSourceName";
    pProperties[0].Value <<= sDataSourceName;
    pProperties[1].Name = "Command";
    pProperties[1].Value <<= sTableOrQuery;
    pProperties[2].Name = "CommandType";
    pProperties[2].Value <<= nCommandType;
    pProperties[3].Name = "ShowTreeView";
    pProperties[3].Value <<= false;
    pProperties[4].Name = "ShowTreeViewButton";
    pProperties[4].Value <<= false;

    VclPtrInstance< SwDBTablePreviewDialog > pDlg(nullptr, aProperties); //TODO
    pDlg->Execute();
}

OUString SwSelectDBTableDialog::GetSelectedTable(bool& bIsTable)
{
    int nEntry = m_xTable->get_selected_index();
    if (nEntry != -1)
    {
        bIsTable = m_xTable->get_id(nEntry).isEmpty();
        return m_xTable->get_text(nEntry, 0);
    }
    bIsTable = false;
    return OUString();
}

void SwSelectDBTableDialog::SetSelectedTable(const OUString& rTable, bool bIsTable)
{
    for (int i = 0, nCount = m_xTable->n_children(); i < nCount; ++i)
    {
        if (m_xTable->get_text(i, 0) == rTable &&
            m_xTable->get_id(i).isEmpty() == bIsTable)
        {
            m_xTable->select(i);
            break;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
