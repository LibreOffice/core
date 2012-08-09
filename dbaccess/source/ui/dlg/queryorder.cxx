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

#include "queryorder.hrc"
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

DBG_NAME(DlgOrderCrit)
//------------------------------------------------------------------------------
DlgOrderCrit::DlgOrderCrit( Window * pParent,
                            const Reference< XConnection>& _rxConnection,
                            const Reference< XSingleSelectQueryComposer >& _rxComposer,
                            const Reference< XNameAccess>& _rxCols)
             :ModalDialog( pParent, ModuleRes(DLG_ORDERCRIT) )
            ,aLB_ORDERFIELD1(   this, ModuleRes( LB_ORDERFIELD1 ) )
            ,aLB_ORDERVALUE1(   this, ModuleRes( LB_ORDERVALUE1 ) )
            ,aLB_ORDERFIELD2(   this, ModuleRes( LB_ORDERFIELD2 ) )
            ,aLB_ORDERVALUE2(   this, ModuleRes( LB_ORDERVALUE2 ) )
            ,aLB_ORDERFIELD3(   this, ModuleRes( LB_ORDERFIELD3 ) )
            ,aLB_ORDERVALUE3(   this, ModuleRes( LB_ORDERVALUE3 ) )
            ,aFT_ORDERFIELD(    this, ModuleRes( FT_ORDERFIELD ) )
            ,aFT_ORDERAFTER1(   this, ModuleRes( FT_ORDERAFTER1 ) )
            ,aFT_ORDERAFTER2(   this, ModuleRes( FT_ORDERAFTER2 ) )
            ,aFT_ORDEROPER(     this, ModuleRes( FT_ORDEROPER ) )
            ,aFT_ORDERDIR(      this, ModuleRes( FT_ORDERDIR ) )
            ,aBT_OK(            this, ModuleRes( BT_OK ) )
            ,aBT_CANCEL(        this, ModuleRes( BT_CANCEL ) )
            ,aBT_HELP(          this, ModuleRes( BT_HELP ) )
            ,aFL_ORDER(         this, ModuleRes( FL_ORDER ) )
            ,aSTR_NOENTRY(      ModuleRes( STR_NOENTRY ) )
            ,m_xQueryComposer( _rxComposer )
            ,m_xColumns(_rxCols)
            ,m_xConnection(_rxConnection)
{
    DBG_CTOR(DlgOrderCrit,NULL);

    AllSettings aSettings( GetSettings() );
    StyleSettings aStyle( aSettings.GetStyleSettings() );
    aStyle.SetAutoMnemonic( sal_False );
    aSettings.SetStyleSettings( aStyle );
    SetSettings( aSettings );

    m_aColumnList[0] = &aLB_ORDERFIELD1;
    m_aColumnList[1] = &aLB_ORDERFIELD2;
    m_aColumnList[2] = &aLB_ORDERFIELD3;

    m_aValueList[0] = &aLB_ORDERVALUE1;
    m_aValueList[1] = &aLB_ORDERVALUE2;
    m_aValueList[2] = &aLB_ORDERVALUE3;

    xub_StrLen j;
    for(j=0 ; j < DOG_ROWS ; j++ )
    {
        m_aColumnList[j]->InsertEntry( aSTR_NOENTRY );
    }

    for( j=0 ; j < DOG_ROWS ; j++ )
    {
        m_aColumnList[j]->SelectEntryPos(0);
        m_aValueList[j]->SelectEntryPos(0);
    }
    try
    {
        // ... also the remaining fields
        Sequence< ::rtl::OUString> aNames = m_xColumns->getElementNames();
        const ::rtl::OUString* pIter = aNames.getConstArray();
        const ::rtl::OUString* pEnd   = pIter + aNames.getLength();
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
                    for( j=0 ; j < DOG_ROWS ; j++ )
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

    aLB_ORDERFIELD1.SetSelectHdl(LINK(this,DlgOrderCrit,FieldListSelectHdl));
    aLB_ORDERFIELD2.SetSelectHdl(LINK(this,DlgOrderCrit,FieldListSelectHdl));

    FreeResource();

}

//------------------------------------------------------------------------------
DlgOrderCrit::~DlgOrderCrit()
{
    DBG_DTOR(DlgOrderCrit,NULL);
}

//------------------------------------------------------------------------------
IMPL_LINK_INLINE_START( DlgOrderCrit, FieldListSelectHdl, ListBox *, /*pListBox*/ )
{
    DBG_CHKTHIS(DlgOrderCrit,NULL);
    EnableLines();
    return 0;
}
IMPL_LINK_INLINE_END( DlgOrderCrit, FieldListSelectHdl, ListBox *, pListBox )

//------------------------------------------------------------------------------
void DlgOrderCrit::impl_initializeOrderList_nothrow()
{
    try
    {
        const ::rtl::OUString sNameProperty = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Name" ) );
        const ::rtl::OUString sAscendingProperty = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "IsAscending" ) );

        Reference< XIndexAccess > xOrderColumns( m_xQueryComposer->getOrderColumns(), UNO_QUERY_THROW );
        sal_Int32 nColumns = xOrderColumns->getCount();
        if ( nColumns > DOG_ROWS )
            nColumns = DOG_ROWS;

        for ( sal_Int32 i = 0; i < nColumns; ++i )
        {
            Reference< XPropertySet > xColumn( xOrderColumns->getByIndex( i ), UNO_QUERY_THROW );

            ::rtl::OUString sColumnName;
            sal_Bool        bIsAscending( sal_True );

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

//------------------------------------------------------------------------------
void DlgOrderCrit::EnableLines()
{
    DBG_CHKTHIS(DlgOrderCrit,NULL);

    if ( aLB_ORDERFIELD1.GetSelectEntryPos() == 0 )
    {
        aLB_ORDERFIELD2.Disable();
        aLB_ORDERVALUE2.Disable();

        aLB_ORDERFIELD2.SelectEntryPos( 0 );
        aLB_ORDERVALUE2.SelectEntryPos( 0 );
    }
    else
    {
        aLB_ORDERFIELD2.Enable();
        aLB_ORDERVALUE2.Enable();
    }

    if ( aLB_ORDERFIELD2.GetSelectEntryPos() == 0 )
    {
        aLB_ORDERFIELD3.Disable();
        aLB_ORDERVALUE3.Disable();

        aLB_ORDERFIELD3.SelectEntryPos( 0 );
        aLB_ORDERVALUE3.SelectEntryPos( 0 );
    }
    else
    {
        aLB_ORDERFIELD3.Enable();
        aLB_ORDERVALUE3.Enable();
    }
}

//------------------------------------------------------------------------------
::rtl::OUString DlgOrderCrit::GetOrderList( ) const
{
    DBG_CHKTHIS(DlgOrderCrit,NULL);
    Reference<XDatabaseMetaData> xMetaData = m_xConnection->getMetaData();
    ::rtl::OUString sQuote  = xMetaData.is() ? xMetaData->getIdentifierQuoteString() : ::rtl::OUString();
    static const ::rtl::OUString sDESC(RTL_CONSTASCII_USTRINGPARAM(" DESC "));
    static const ::rtl::OUString sASC(RTL_CONSTASCII_USTRINGPARAM(" ASC "));

    Reference< XNameAccess> xColumns = Reference< XColumnsSupplier >(m_xQueryComposer,UNO_QUERY)->getColumns();

    ::rtl::OUString sOrder;
    for( sal_uInt16 i=0 ; i<DOG_ROWS; i++ )
    {
        if(m_aColumnList[i]->GetSelectEntryPos() != 0)
        {
            if(!sOrder.isEmpty())
                sOrder += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(","));

            String sName = m_aColumnList[i]->GetSelectEntry();
            try
            {
                sal_Bool bFunction = sal_False;
                Reference< XPropertySet > xColumn;
                if ( xColumns.is() && xColumns->hasByName( sName ) && (xColumns->getByName( sName ) >>= xColumn) && xColumn.is() )
                {
                    if ( xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_REALNAME) )
                    {
                        ::rtl::OUString sRealName;
                        xColumn->getPropertyValue(PROPERTY_REALNAME)    >>= sRealName;
                        sName = sRealName;
                        static ::rtl::OUString sFunction(RTL_CONSTASCII_USTRINGPARAM("Function"));
                        if ( xColumn->getPropertySetInfo()->hasPropertyByName(sFunction) )
                            xColumn->getPropertyValue(sFunction) >>= bFunction;
                    }
                }
                if ( bFunction )
                    sOrder += sName;
                else
                    sOrder += ::dbtools::quoteName(sQuote,sName);
            }
            catch(const Exception&)
            {
            }
            if(m_aValueList[i]->GetSelectEntryPos())
                sOrder += sDESC;
            else
                sOrder += sASC;
        }
    }
    return sOrder;
}

//------------------------------------------------------------------------------
void DlgOrderCrit::BuildOrderPart()
{
    DBG_CHKTHIS(DlgOrderCrit,NULL);
    m_xQueryComposer->setOrder(GetOrderList());
}
// -----------------------------------------------------------------------------





/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
