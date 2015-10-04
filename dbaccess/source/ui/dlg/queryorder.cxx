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

#include "dbu_tbl.hrc"
#include "queryorder.hxx"
#include "dbustrings.hrc"
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#include <com/sun/star/sdbc/ColumnSearch.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <tools/debug.hxx>
#include "moduledbu.hxx"
#include <connectivity/sqliterator.hxx>
#include <connectivity/dbtools.hxx>
#include <comphelper/extract.hxx>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <tools/diagnose_ex.h>
#include <vcl/settings.hxx>
#include <algorithm>

using namespace dbaui;
using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;


DlgOrderCrit::DlgOrderCrit(vcl::Window * pParent,
    const Reference< XConnection>& _rxConnection,
    const Reference< XSingleSelectQueryComposer >& _rxComposer,
    const Reference< XNameAccess>& _rxCols)
    : ModalDialog(pParent, "SortDialog", "dbaccess/ui/sortdialog.ui")
    , aSTR_NOENTRY(ModuleRes(STR_VALUE_NONE))
    , m_xQueryComposer(_rxComposer)
    , m_xColumns(_rxCols)
    , m_xConnection(_rxConnection)
{

    get(m_pLB_ORDERFIELD1, "field1");
    get(m_pLB_ORDERVALUE1, "value1");
    get(m_pLB_ORDERFIELD2, "field2");
    get(m_pLB_ORDERVALUE2, "value2");
    get(m_pLB_ORDERFIELD3, "field3");
    get(m_pLB_ORDERVALUE3, "value3");

    AllSettings aSettings( GetSettings() );
    StyleSettings aStyle( aSettings.GetStyleSettings() );
    aStyle.SetAutoMnemonic( false );
    aSettings.SetStyleSettings( aStyle );
    SetSettings( aSettings );

    m_aColumnList[0] = m_pLB_ORDERFIELD1;
    m_aColumnList[1] = m_pLB_ORDERFIELD2;
    m_aColumnList[2] = m_pLB_ORDERFIELD3;

    m_aValueList[0] = m_pLB_ORDERVALUE1;
    m_aValueList[1] = m_pLB_ORDERVALUE2;
    m_aValueList[2] = m_pLB_ORDERVALUE3;

    for (int j=0; j < DOG_ROWS; ++j)
    {
        m_aColumnList[j]->InsertEntry( aSTR_NOENTRY );
    }

    for (int j=0; j < DOG_ROWS; ++j)
    {
        m_aColumnList[j]->SelectEntryPos(0);
        m_aValueList[j]->SelectEntryPos(0);
    }
    try
    {
        // ... also the remaining fields
        Sequence< OUString> aNames = m_xColumns->getElementNames();
        const OUString* pIter = aNames.getConstArray();
        const OUString* pEnd   = pIter + aNames.getLength();
        Reference<XPropertySet> xColumn;
        for(;pIter != pEnd;++pIter)
        {
            xColumn.set(m_xColumns->getByName(*pIter),UNO_QUERY);
            OSL_ENSURE(xColumn.is(),"Column is null!");
            if ( xColumn.is() )
            {
                sal_Int32 nDataType = 0;
                xColumn->getPropertyValue(PROPERTY_TYPE) >>= nDataType;
                sal_Int32 eColumnSearch = dbtools::getSearchColumnFlag(m_xConnection,nDataType);
                if(eColumnSearch != ColumnSearch::NONE)
                {
                    for (int j=0; j < DOG_ROWS; ++j)
                    {
                        m_aColumnList[j]->InsertEntry(*pIter);
                    }
                }
            }
        }

        m_sOrgOrder = m_xQueryComposer->getOrder();
        impl_initializeOrderList_nothrow();
    }
    catch(const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    EnableLines();

    m_pLB_ORDERFIELD1->SetSelectHdl(LINK(this,DlgOrderCrit,FieldListSelectHdl));
    m_pLB_ORDERFIELD2->SetSelectHdl(LINK(this,DlgOrderCrit,FieldListSelectHdl));
}

DlgOrderCrit::~DlgOrderCrit()
{
    disposeOnce();
}

void DlgOrderCrit::dispose()
{
    m_pLB_ORDERFIELD1.clear();
    m_pLB_ORDERVALUE1.clear();
    m_pLB_ORDERFIELD2.clear();
    m_pLB_ORDERVALUE2.clear();
    m_pLB_ORDERFIELD3.clear();
    m_pLB_ORDERVALUE3.clear();
    for (auto a : m_aColumnList) a.clear();
    for (auto a : m_aValueList) a.clear();
    ModalDialog::dispose();
}

IMPL_LINK_NOARG_TYPED( DlgOrderCrit, FieldListSelectHdl, ListBox&, void )
{
    EnableLines();
}

void DlgOrderCrit::impl_initializeOrderList_nothrow()
{
    try
    {
        const OUString sNameProperty = "Name";
        const OUString sAscendingProperty = "IsAscending";

        Reference< XIndexAccess > xOrderColumns( m_xQueryComposer->getOrderColumns(), UNO_QUERY_THROW );
        sal_Int32 nColumns = xOrderColumns->getCount();
        if ( nColumns > DOG_ROWS )
            nColumns = DOG_ROWS;

        for ( sal_Int32 i = 0; i < nColumns; ++i )
        {
            Reference< XPropertySet > xColumn( xOrderColumns->getByIndex( i ), UNO_QUERY_THROW );

            OUString sColumnName;
            bool        bIsAscending( true );

            xColumn->getPropertyValue( sNameProperty ) >>= sColumnName;
            xColumn->getPropertyValue( sAscendingProperty ) >>= bIsAscending;

            m_aColumnList[i]->SelectEntry( sColumnName );
            m_aValueList[i]->SelectEntryPos( bIsAscending ? 0 : 1 );
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

void DlgOrderCrit::EnableLines()
{

    if ( m_pLB_ORDERFIELD1->GetSelectEntryPos() == 0 )
    {
        m_pLB_ORDERFIELD2->Disable();
        m_pLB_ORDERVALUE2->Disable();

        m_pLB_ORDERFIELD2->SelectEntryPos( 0 );
        m_pLB_ORDERVALUE2->SelectEntryPos( 0 );
    }
    else
    {
        m_pLB_ORDERFIELD2->Enable();
        m_pLB_ORDERVALUE2->Enable();
    }

    if ( m_pLB_ORDERFIELD2->GetSelectEntryPos() == 0 )
    {
        m_pLB_ORDERFIELD3->Disable();
        m_pLB_ORDERVALUE3->Disable();

        m_pLB_ORDERFIELD3->SelectEntryPos( 0 );
        m_pLB_ORDERVALUE3->SelectEntryPos( 0 );
    }
    else
    {
        m_pLB_ORDERFIELD3->Enable();
        m_pLB_ORDERVALUE3->Enable();
    }
}

OUString DlgOrderCrit::GetOrderList( ) const
{
    Reference<XDatabaseMetaData> xMetaData = m_xConnection->getMetaData();
    OUString sQuote  = xMetaData.is() ? xMetaData->getIdentifierQuoteString() : OUString();
    static const char sDESC[] = " DESC ";
    static const char sASC[] = " ASC ";

    Reference< XNameAccess> xColumns = Reference< XColumnsSupplier >(m_xQueryComposer,UNO_QUERY)->getColumns();

    OUString sOrder;
    for( sal_uInt16 i=0 ; i<DOG_ROWS; i++ )
    {
        if(m_aColumnList[i]->GetSelectEntryPos() != 0)
        {
            if(!sOrder.isEmpty())
                sOrder += ",";

            OUString sName = m_aColumnList[i]->GetSelectEntry();
            sOrder += ::dbtools::quoteName(sQuote,sName);
            if(m_aValueList[i]->GetSelectEntryPos())
                sOrder += sDESC;
            else
                sOrder += sASC;
        }
    }
    return sOrder;
}

void DlgOrderCrit::BuildOrderPart()
{
    m_xQueryComposer->setOrder(GetOrderList());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
