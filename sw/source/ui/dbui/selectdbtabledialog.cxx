/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <swtypes.hxx>
#include <selectdbtabledialog.hxx>
#include <dbtablepreviewdialog.hxx>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/sdbc/XDataSource.hpp>

#include <unomid.h>

#include <selectdbtabledialog.hrc>
#include <dbui.hrc>
#include <helpid.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;

SwSelectDBTableDialog::SwSelectDBTableDialog(Window* pParent,
        const uno::Reference< sdbc::XConnection>& rConnection) :
    SfxModalDialog(pParent, SW_RES(DLG_MM_SELECTDBTABLEDDIALOG)),
#ifdef MSC
#pragma warning (disable : 4355)
#endif
    m_aSelectFI( this, SW_RES(       FI_SELECT     )),
    m_aTableHB( this, WB_BUTTONSTYLE | WB_BOTTOMBORDER),
    m_aTableLB( this, SW_RES(        LB_TABLE      )),
    m_aPreviewPB( this, SW_RES(      PB_PREVIEW    )),
    m_aSeparatorFL(this, SW_RES(    FL_SEPARATOR      )),
    m_aOK( this, SW_RES(             PB_OK         )),
    m_aCancel( this, SW_RES(         PB_CANCEL     )),
    m_aHelp( this, SW_RES(           PB_HELP       )),
#ifdef MSC
#pragma warning (default : 4355)
#endif
    m_sName( SW_RES( ST_NAME )),
    m_sType( SW_RES( ST_TYPE )),
    m_sTable( SW_RES( ST_TABLE )),
    m_sQuery( SW_RES( ST_QUERY )),
    m_xConnection(rConnection)
{
    FreeResource();

    Size aLBSize(m_aTableLB.GetSizePixel());
    m_aTableHB.SetSizePixel(aLBSize);
    Size aHeadSize(m_aTableHB.CalcWindowSizePixel());
    aHeadSize.Width() = aLBSize.Width();
    m_aTableHB.SetSizePixel(aHeadSize);
    Point aLBPos(m_aTableLB.GetPosPixel());
    m_aTableHB.SetPosPixel(aLBPos);
    aLBPos.Y() += aHeadSize.Height();
    aLBSize.Height() -= aHeadSize.Height();
    m_aTableLB.SetPosSizePixel(aLBPos, aLBSize);

    Size aSz(m_aTableHB.GetOutputSizePixel());
    m_aTableHB.InsertItem( 1, m_sName,
                            aSz.Width()/2,
                            HIB_LEFT | HIB_VCENTER /*| HIB_CLICKABLE | HIB_UPARROW */);
    m_aTableHB.InsertItem( 2, m_sType,
                            aSz.Width()/2,
                            HIB_LEFT | HIB_VCENTER /*| HIB_CLICKABLE | HIB_UPARROW */);
    m_aTableHB.SetHelpId(HID_MM_ADDRESSLIST_HB );
    m_aTableHB.Show();

    static long nTabs[] = {3, 0, aSz.Width()/2, aSz.Width() };
    m_aTableLB.SetTabs(&nTabs[0], MAP_PIXEL);
    m_aTableLB.SetHelpId(HID_MM_SELECTDBTABLEDDIALOG_LISTBOX);
    m_aTableLB.SetStyle( m_aTableLB.GetStyle() | WB_CLIPCHILDREN );
    m_aTableLB.SetSpaceBetweenEntries(3);
    m_aTableLB.SetSelectionMode( SINGLE_SELECTION );
    m_aTableLB.SetDragDropMode(   0 );
    m_aTableLB.EnableAsyncDrag(sal_False);

    m_aPreviewPB.SetClickHdl(LINK(this, SwSelectDBTableDialog, PreviewHdl));

    Reference<XTablesSupplier> xTSupplier(m_xConnection, UNO_QUERY);
    if(xTSupplier.is())
    {
        Reference<XNameAccess> xTbls = xTSupplier->getTables();
        Sequence<rtl::OUString> aTbls = xTbls->getElementNames();
        const rtl::OUString* pTbls = aTbls.getConstArray();
        for(long i = 0; i < aTbls.getLength(); i++)
        {
            String sEntry = pTbls[i];
            sEntry += '\t';
            sEntry += m_sTable;
            SvLBoxEntry* pEntry = m_aTableLB.InsertEntry(sEntry);
            pEntry->SetUserData((void*)0);
        }
    }
    Reference<XQueriesSupplier> xQSupplier(m_xConnection, UNO_QUERY);
    if(xQSupplier.is())
    {
        Reference<XNameAccess> xQueries = xQSupplier->getQueries();
        Sequence<rtl::OUString> aQueries = xQueries->getElementNames();
        const rtl::OUString* pQueries = aQueries.getConstArray();
        for(long i = 0; i < aQueries.getLength(); i++)
        {
            String sEntry = pQueries[i];
            sEntry += '\t';
            sEntry += m_sQuery;
            SvLBoxEntry* pEntry = m_aTableLB.InsertEntry(sEntry);
            pEntry->SetUserData((void*)1);
        }
    }
}

SwSelectDBTableDialog::~SwSelectDBTableDialog()
{
}

IMPL_LINK(SwSelectDBTableDialog, PreviewHdl, PushButton*, pButton)
{
    SvLBoxEntry* pEntry = m_aTableLB.FirstSelected();
    if(pEntry)
    {
        ::rtl::OUString sTableOrQuery = m_aTableLB.GetEntryText(pEntry, 0);
        sal_Int32 nCommandType = 0 == pEntry->GetUserData() ? 0 : 1;

        ::rtl::OUString sDataSourceName;
        Reference<XChild> xChild(m_xConnection, UNO_QUERY);
        if(xChild.is())
        {
            Reference<XDataSource> xSource(xChild->getParent(), UNO_QUERY);
            Reference<XPropertySet> xPrSet(xSource, UNO_QUERY);
            xPrSet->getPropertyValue(C2U("Name")) >>= sDataSourceName;
        }
        OSL_ENSURE(!sDataSourceName.isEmpty(), "no data source found");
        Sequence<PropertyValue> aProperties(5);
        PropertyValue* pProperties = aProperties.getArray();
        pProperties[0].Name = C2U("DataSourceName");
        pProperties[0].Value <<= sDataSourceName;
        pProperties[1].Name = C2U("Command");
        pProperties[1].Value <<= sTableOrQuery;
        pProperties[2].Name = C2U("CommandType");
        pProperties[2].Value <<= nCommandType;
        pProperties[3].Name = C2U("ShowTreeView");
        sal_Bool bFalse = sal_False;
        pProperties[3].Value <<= bFalse;
        pProperties[4].Name = C2U("ShowTreeViewButton");
        pProperties[4].Value <<= bFalse;

        SwDBTablePreviewDialog* pDlg = new SwDBTablePreviewDialog(pButton, aProperties);
        pDlg->Execute();
        delete pDlg;
    }

    return 0;
}

String      SwSelectDBTableDialog::GetSelectedTable(bool& bIsTable)
{
    SvLBoxEntry* pEntry = m_aTableLB.FirstSelected();
    bIsTable = pEntry->GetUserData() ? false : true;
    return pEntry ? m_aTableLB.GetEntryText(pEntry, 0) : String();
}

void   SwSelectDBTableDialog::SetSelectedTable(const String& rTable, bool bIsTable)
{
    SvLBoxEntry*    pEntry = m_aTableLB.First();
    while(pEntry)
    {
        if((m_aTableLB.GetEntryText(pEntry, 0) == rTable) &&
                 ((pEntry->GetUserData() == 0 ) == bIsTable))
        {
            m_aTableLB.Select(pEntry);
            break;
        }
        pEntry = m_aTableLB.Next( pEntry );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
