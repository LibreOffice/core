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

#include <o3tl/safeint.hxx>
#include <comphelper/propertyvalue.hxx>
#include <swtypes.hxx>
#include "selectdbtabledialog.hxx"
#include "dbtablepreviewdialog.hxx"
#include <osl/diagnose.h>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>

#include <strings.hrc>
#include <utility>

using namespace ::com::sun::star;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;

SwSelectDBTableDialog::SwSelectDBTableDialog(weld::Window* pParent,
                                             uno::Reference<sdbc::XConnection> xConnection)
    : SfxDialogController(pParent, u"modules/swriter/ui/selecttabledialog.ui"_ustr,
                          u"SelectTableDialog"_ustr)
    , m_xConnection(std::move(xConnection))
    , m_xTable(m_xBuilder->weld_tree_view(u"table"_ustr))
    , m_xPreviewPB(m_xBuilder->weld_button(u"preview"_ustr))
{
    m_xTable->set_size_request(m_xTable->get_approximate_digit_width() * 60,
                               m_xTable->get_height_rows(6));

    std::vector<int> aWidths{ o3tl::narrowing<int>(m_xTable->get_approximate_digit_width() * 30) };
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
    if (!xQSupplier.is())
        return;

    Reference<XNameAccess> xQueries = xQSupplier->getQueries();
    const Sequence<OUString> aQueries = xQueries->getElementNames();
    int nPos = m_xTable->n_children();
    for (const OUString& rQuery : aQueries)
    {
        m_xTable->append_text(rQuery);
        m_xTable->set_text(nPos, SwResId(ST_QUERY), 1);
        m_xTable->set_id(nPos, OUString::number(1));
        ++nPos;
    }
}

SwSelectDBTableDialog::~SwSelectDBTableDialog() {}

IMPL_LINK_NOARG(SwSelectDBTableDialog, PreviewHdl, weld::Button&, void)
{
    int nEntry = m_xTable->get_selected_index();
    if (nEntry == -1)
        return;

    OUString sTableOrQuery = m_xTable->get_text(nEntry, 0);
    sal_Int32 nCommandType = m_xTable->get_id(nEntry).isEmpty() ? 0 : 1;

    OUString sDataSourceName;
    Reference<XChild> xChild(m_xConnection, UNO_QUERY);
    if (xChild.is())
    {
        Reference<XDataSource> xSource(xChild->getParent(), UNO_QUERY);
        Reference<XPropertySet> xPrSet(xSource, UNO_QUERY);
        xPrSet->getPropertyValue(u"Name"_ustr) >>= sDataSourceName;
    }
    OSL_ENSURE(!sDataSourceName.isEmpty(), "no data source found");
    Sequence<PropertyValue> aProperties{
        comphelper::makePropertyValue(u"DataSourceName"_ustr, sDataSourceName),
        comphelper::makePropertyValue(u"Command"_ustr, sTableOrQuery),
        comphelper::makePropertyValue(u"CommandType"_ustr, nCommandType),
        comphelper::makePropertyValue(u"ShowTreeView"_ustr, false),
        comphelper::makePropertyValue(u"ShowTreeViewButton"_ustr, false)
    };

    SwDBTablePreviewDialog aDlg(m_xDialog.get(), aProperties);
    aDlg.run();
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

void SwSelectDBTableDialog::SetSelectedTable(std::u16string_view rTable, bool bIsTable)
{
    for (int i = 0, nCount = m_xTable->n_children(); i < nCount; ++i)
    {
        if (m_xTable->get_text(i, 0) == rTable && m_xTable->get_id(i).isEmpty() == bIsTable)
        {
            m_xTable->select(i);
            break;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
