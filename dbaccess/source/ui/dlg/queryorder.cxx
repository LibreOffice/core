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

#include <strings.hrc>
#include <strings.hxx>
#include <core_resource.hxx>
#include <queryorder.hxx>
#include <stringconstants.hxx>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#include <com/sun/star/sdbc/ColumnSearch.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <tools/debug.hxx>
#include <connectivity/sqliterator.hxx>
#include <connectivity/dbtools.hxx>
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


DlgOrderCrit::DlgOrderCrit(weld::Window * pParent,
    const Reference< XConnection>& _rxConnection,
    const Reference< XSingleSelectQueryComposer >& _rxComposer,
    const Reference< XNameAccess>& _rxCols)
    : GenericDialogController(pParent, "dbaccess/ui/sortdialog.ui", "SortDialog")
    , m_xQueryComposer(_rxComposer)
    , m_xColumns(_rxCols)
    , m_xConnection(_rxConnection)
    , m_xLB_ORDERFIELD1(m_xBuilder->weld_combo_box("field1"))
    , m_xLB_ORDERVALUE1(m_xBuilder->weld_combo_box("value1"))
    , m_xLB_ORDERFIELD2(m_xBuilder->weld_combo_box("field2"))
    , m_xLB_ORDERVALUE2(m_xBuilder->weld_combo_box("value2"))
    , m_xLB_ORDERFIELD3(m_xBuilder->weld_combo_box("field3"))
    , m_xLB_ORDERVALUE3(m_xBuilder->weld_combo_box("value3"))
{
    m_aColumnList[0] = m_xLB_ORDERFIELD1.get();
    m_aColumnList[1] = m_xLB_ORDERFIELD2.get();
    m_aColumnList[2] = m_xLB_ORDERFIELD3.get();

    m_aValueList[0] = m_xLB_ORDERVALUE1.get();
    m_aValueList[1] = m_xLB_ORDERVALUE2.get();
    m_aValueList[2] = m_xLB_ORDERVALUE3.get();

    OUString aSTR_NOENTRY(DBA_RES(STR_VALUE_NONE));
    for (auto j : m_aColumnList)
    {
        j->append_text(aSTR_NOENTRY);
    }

    for (int j=0; j < DOG_ROWS; ++j)
    {
        m_aColumnList[j]->set_active(0);
        m_aValueList[j]->set_active(0);
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
                    for (auto j : m_aColumnList)
                    {
                        j->append_text(*pIter);
                    }
                }
            }
        }

        m_sOrgOrder = m_xQueryComposer->getOrder();
        impl_initializeOrderList_nothrow();
    }
    catch(const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
    EnableLines();

    m_xLB_ORDERFIELD1->connect_changed(LINK(this,DlgOrderCrit,FieldListSelectHdl));
    m_xLB_ORDERFIELD2->connect_changed(LINK(this,DlgOrderCrit,FieldListSelectHdl));
}

DlgOrderCrit::~DlgOrderCrit()
{
}

IMPL_LINK_NOARG( DlgOrderCrit, FieldListSelectHdl, weld::ComboBox&, void )
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

            m_aColumnList[i]->set_active_text(sColumnName);
            m_aValueList[i]->set_active(bIsAscending ? 0 : 1);
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
}

void DlgOrderCrit::EnableLines()
{

    if ( m_xLB_ORDERFIELD1->get_active() == 0 )
    {
        m_xLB_ORDERFIELD2->set_sensitive(false);
        m_xLB_ORDERVALUE2->set_sensitive(false);

        m_xLB_ORDERFIELD2->set_active( 0 );
        m_xLB_ORDERVALUE2->set_active( 0 );
    }
    else
    {
        m_xLB_ORDERFIELD2->set_sensitive(true);
        m_xLB_ORDERVALUE2->set_sensitive(true);
    }

    if ( m_xLB_ORDERFIELD2->get_active() == 0 )
    {
        m_xLB_ORDERFIELD3->set_sensitive(false);
        m_xLB_ORDERVALUE3->set_sensitive(false);

        m_xLB_ORDERFIELD3->set_active( 0 );
        m_xLB_ORDERVALUE3->set_active( 0 );
    }
    else
    {
        m_xLB_ORDERFIELD3->set_sensitive(true);
        m_xLB_ORDERVALUE3->set_sensitive(true);
    }
}

OUString DlgOrderCrit::GetOrderList( ) const
{
    Reference<XDatabaseMetaData> xMetaData = m_xConnection->getMetaData();
    OUString sQuote  = xMetaData.is() ? xMetaData->getIdentifierQuoteString() : OUString();

    Reference< XNameAccess> xColumns = Reference< XColumnsSupplier >(m_xQueryComposer,UNO_QUERY)->getColumns();

    OUStringBuffer sOrder;
    for( sal_uInt16 i=0 ; i<DOG_ROWS; i++ )
    {
        if (m_aColumnList[i]->get_active() != 0)
        {
            if(!sOrder.isEmpty())
                sOrder.append(",");

            OUString sName = m_aColumnList[i]->get_active_text();
            sOrder.append(::dbtools::quoteName(sQuote,sName));
            if (m_aValueList[i]->get_active())
                sOrder.append(" DESC ");
            else
                sOrder.append(" ASC ");
        }
    }
    return sOrder.makeStringAndClear();
}

void DlgOrderCrit::BuildOrderPart()
{
    m_xQueryComposer->setOrder(GetOrderList());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
