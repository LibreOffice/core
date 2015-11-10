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
#include <selectdbtabledialog.hxx>
#include <dbtablepreviewdialog.hxx>
#include <svtools/simptabl.hxx>
#include <svtools/treelistentry.hxx>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>

#include <unomid.h>

#include <dbui.hrc>
#include <helpid.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;

class SwAddressTable : public SvSimpleTable
{
public:
    SwAddressTable(SvSimpleTableContainer& rParent, WinBits nBits = 0);
    void InsertHeaderItem(sal_uInt16 nColumn, const OUString& rText, HeaderBarItemBits nBits);
    virtual void Resize() override;
    void setColSizes();
};

SwAddressTable::SwAddressTable(SvSimpleTableContainer& rParent, WinBits nBits)
    : SvSimpleTable(rParent, nBits)
{
    SetSpaceBetweenEntries(3);
    SetSelectionMode(SINGLE_SELECTION);
    SetDragDropMode(DragDropMode::NONE);
    EnableAsyncDrag(false);
}

void SwAddressTable::InsertHeaderItem(sal_uInt16 nColumn, const OUString& rText, HeaderBarItemBits nBits)
{
    GetTheHeaderBar().InsertItem( nColumn, rText, 0, nBits );
}

void SwAddressTable::Resize()
{
    SvSimpleTable::Resize();
    setColSizes();
}

void SwAddressTable::setColSizes()
{
    HeaderBar &rHB = GetTheHeaderBar();
    if (rHB.GetItemCount() < 2)
        return;

    long nWidth = rHB.GetSizePixel().Width();
    nWidth /= 2;

    long nTabs_Impl[3];

    nTabs_Impl[0] = 2;
    nTabs_Impl[1] = 0;
    nTabs_Impl[2] = nWidth;

    SvSimpleTable::SetTabs(&nTabs_Impl[0], MAP_PIXEL);
}

SwSelectDBTableDialog::SwSelectDBTableDialog(vcl::Window* pParent,
        const uno::Reference< sdbc::XConnection>& rConnection)
    : SfxModalDialog(pParent, "SelectTableDialog", "modules/swriter/ui/selecttabledialog.ui")
    , m_sName(SW_RES(ST_NAME))
    , m_sType(SW_RES(ST_TYPE))
    , m_sTable(SW_RES(ST_TABLE))
    , m_sQuery(SW_RES(ST_QUERY))
    , m_xConnection(rConnection)
{
    get(m_pPreviewPB, "preview");

    SvSimpleTableContainer *pHeaderTreeContainer = get<SvSimpleTableContainer>("table");
    Size aSize = pHeaderTreeContainer->LogicToPixel(Size(238 , 50), MAP_APPFONT);
    pHeaderTreeContainer->set_width_request(aSize.Width());
    pHeaderTreeContainer->set_height_request(aSize.Height());
    m_pTable = VclPtr<SwAddressTable>::Create(*pHeaderTreeContainer);
    long aStaticTabs[]= { 2, 0, 0 };
    m_pTable->SetTabs( aStaticTabs );
    m_pTable->InsertHeaderItem(1, m_sName, HeaderBarItemBits::LEFT | HeaderBarItemBits::VCENTER);
    m_pTable->InsertHeaderItem(2, m_sType, HeaderBarItemBits::LEFT | HeaderBarItemBits::VCENTER);

    m_pPreviewPB->SetClickHdl(LINK(this, SwSelectDBTableDialog, PreviewHdl));

    Reference<XTablesSupplier> xTSupplier(m_xConnection, UNO_QUERY);
    if (xTSupplier.is())
    {
        Reference<XNameAccess> xTables = xTSupplier->getTables();
        Sequence<OUString> aTables = xTables->getElementNames();
        const OUString* pTables = aTables.getConstArray();
        for(long i = 0; i < aTables.getLength(); i++)
        {
            OUString sEntry = pTables[i];
            sEntry += "\t";
            sEntry += m_sTable;
            SvTreeListEntry* pEntry = m_pTable->InsertEntry(sEntry);
            pEntry->SetUserData(nullptr);
        }
    }
    Reference<XQueriesSupplier> xQSupplier(m_xConnection, UNO_QUERY);
    if (xQSupplier.is())
    {
        Reference<XNameAccess> xQueries = xQSupplier->getQueries();
        Sequence<OUString> aQueries = xQueries->getElementNames();
        const OUString* pQueries = aQueries.getConstArray();
        for(long i = 0; i < aQueries.getLength(); i++)
        {
            OUString sEntry = pQueries[i];
            sEntry += "\t";
            sEntry += m_sQuery;
            SvTreeListEntry* pEntry = m_pTable->InsertEntry(sEntry);
            pEntry->SetUserData(reinterpret_cast<void*>(1));
        }
    }
}

SwSelectDBTableDialog::~SwSelectDBTableDialog()
{
    disposeOnce();
}

void SwSelectDBTableDialog::dispose()
{
    m_pTable.disposeAndClear();
    m_pPreviewPB.clear();
    SfxModalDialog::dispose();
}

IMPL_LINK_TYPED(SwSelectDBTableDialog, PreviewHdl, Button*, pButton, void)
{
    SvTreeListEntry* pEntry = m_pTable->FirstSelected();
    if(pEntry)
    {
        OUString sTableOrQuery = SvTabListBox::GetEntryText(pEntry, 0);
        sal_Int32 nCommandType = nullptr == pEntry->GetUserData() ? 0 : 1;

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
        pProperties[3].Value <<= sal_False;
        pProperties[4].Name = "ShowTreeViewButton";
        pProperties[4].Value <<= sal_False;

        VclPtrInstance< SwDBTablePreviewDialog > pDlg(pButton, aProperties);
        pDlg->Execute();
    }
}

OUString    SwSelectDBTableDialog::GetSelectedTable(bool& bIsTable)
{
    SvTreeListEntry* pEntry = m_pTable->FirstSelected();
    bIsTable = pEntry->GetUserData() == nullptr;
    return SvTabListBox::GetEntryText(pEntry, 0);
}

void   SwSelectDBTableDialog::SetSelectedTable(const OUString& rTable, bool bIsTable)
{
    SvTreeListEntry*    pEntry = m_pTable->First();
    while(pEntry)
    {
        if((SvTabListBox::GetEntryText(pEntry, 0) == rTable) &&
           ((pEntry->GetUserData() == nullptr ) == bIsTable))
        {
            m_pTable->Select(pEntry);
            break;
        }
        pEntry = m_pTable->Next( pEntry );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
