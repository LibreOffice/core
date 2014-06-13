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
#include <boost/scoped_ptr.hpp>

using namespace ::com::sun::star;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;

SwSelectDBTableDialog::SwSelectDBTableDialog(Window* pParent,
        const uno::Reference< sdbc::XConnection>& rConnection) :
    SfxModalDialog(pParent, "SelectTableDialog", "modules/swriter/ui/selecttabledialog.ui"),
    m_sName( SW_RES( ST_NAME )),
    m_sType( SW_RES( ST_TYPE )),
    m_sTable( SW_RES( ST_TABLE )),
    m_sQuery( SW_RES( ST_QUERY )),
    m_xConnection(rConnection)
{
    get(m_pPreviewPB, "preview");
    get(m_pContainer, "table");
    m_pTableHB = new HeaderBar(m_pContainer, WB_BUTTONSTYLE | WB_BOTTOMBORDER);
    m_pTableHB->set_height_request(40);
    m_pTableLB = new SvTabListBox(m_pContainer, WB_BORDER);

    Size aLBSize(Size(400, 100));
    m_pContainer->SetSizePixel(aLBSize);
    Size aHeadSize(m_pTableHB->CalcWindowSizePixel());
    aHeadSize.Width() = aLBSize.Width();
    m_pTableHB->SetSizePixel(aHeadSize);
    Point aLBPos(0, 0);
    m_pTableHB->SetPosPixel(aLBPos);
    aLBPos.Y() += aHeadSize.Height();
    aLBSize.Height() -= aHeadSize.Height();
    m_pTableLB->SetPosSizePixel(aLBPos, aLBSize);

    Size aSz(m_pTableHB->GetOutputSizePixel());
    m_pTableHB->InsertItem( 1, m_sName,
                            aSz.Width()/2,
                            HIB_LEFT | HIB_VCENTER /*| HIB_CLICKABLE | HIB_UPARROW */);
    m_pTableHB->InsertItem( 2, m_sType,
                            aSz.Width()/2,
                            HIB_LEFT | HIB_VCENTER /*| HIB_CLICKABLE | HIB_UPARROW */);
    m_pTableHB->SetHelpId(HID_MM_ADDRESSLIST_HB );
    m_pTableHB->Show();
    m_pTableLB->Show();

    static long nTabs[] = {3, 0, aSz.Width()/2, aSz.Width() };
    m_pTableLB->SetTabs(&nTabs[0], MAP_PIXEL);
    m_pTableLB->SetHelpId(HID_MM_SELECTDBTABLEDDIALOG_LISTBOX);
    m_pTableLB->SetStyle( m_pTableLB->GetStyle() | WB_CLIPCHILDREN );
    m_pTableLB->SetSpaceBetweenEntries(3);
    m_pTableLB->SetSelectionMode( SINGLE_SELECTION );
    m_pTableLB->SetDragDropMode( 0 );
    m_pTableLB->EnableAsyncDrag(false);

    m_pPreviewPB->SetClickHdl(LINK(this, SwSelectDBTableDialog, PreviewHdl));

    Reference<XTablesSupplier> xTSupplier(m_xConnection, UNO_QUERY);
    if(xTSupplier.is())
    {
        Reference<XNameAccess> xTbls = xTSupplier->getTables();
        Sequence<OUString> aTbls = xTbls->getElementNames();
        const OUString* pTbls = aTbls.getConstArray();
        for(long i = 0; i < aTbls.getLength(); i++)
        {
            OUString sEntry = pTbls[i];
            sEntry += "\t";
            sEntry += m_sTable;
            SvTreeListEntry* pEntry = m_pTableLB->InsertEntry(sEntry);
            pEntry->SetUserData((void*)0);
        }
    }
    Reference<XQueriesSupplier> xQSupplier(m_xConnection, UNO_QUERY);
    if(xQSupplier.is())
    {
        Reference<XNameAccess> xQueries = xQSupplier->getQueries();
        Sequence<OUString> aQueries = xQueries->getElementNames();
        const OUString* pQueries = aQueries.getConstArray();
        for(long i = 0; i < aQueries.getLength(); i++)
        {
            OUString sEntry = pQueries[i];
            sEntry += "\t";
            sEntry += m_sQuery;
            SvTreeListEntry* pEntry = m_pTableLB->InsertEntry(sEntry);
            pEntry->SetUserData((void*)1);
        }
    }
}

SwSelectDBTableDialog::~SwSelectDBTableDialog()
{
    delete m_pTableHB;
    delete m_pTableLB;
}

IMPL_LINK(SwSelectDBTableDialog, PreviewHdl, PushButton*, pButton)
{
    SvTreeListEntry* pEntry = m_pTableLB->FirstSelected();
    if(pEntry)
    {
        OUString sTableOrQuery = m_pTableLB->GetEntryText(pEntry, 0);
        sal_Int32 nCommandType = 0 == pEntry->GetUserData() ? 0 : 1;

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

        boost::scoped_ptr<SwDBTablePreviewDialog> pDlg(new SwDBTablePreviewDialog(pButton, aProperties));
        pDlg->Execute();
    }

    return 0;
}

OUString    SwSelectDBTableDialog::GetSelectedTable(bool& bIsTable)
{
    SvTreeListEntry* pEntry = m_pTableLB->FirstSelected();
    bIsTable = pEntry->GetUserData() ? false : true;
    return m_pTableLB->GetEntryText(pEntry, 0);
}

void   SwSelectDBTableDialog::SetSelectedTable(const OUString& rTable, bool bIsTable)
{
    SvTreeListEntry*    pEntry = m_pTableLB->First();
    while(pEntry)
    {
        if((m_pTableLB->GetEntryText(pEntry, 0) == rTable) &&
                 ((pEntry->GetUserData() == 0 ) == bIsTable))
        {
            m_pTableLB->Select(pEntry);
            break;
        }
        pEntry = m_pTableLB->Next( pEntry );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
