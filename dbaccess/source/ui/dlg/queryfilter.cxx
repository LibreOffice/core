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

#include "queryfilter.hxx"
#include "moduledbu.hxx"
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/sdb/XSQLQueryComposer.hpp>
#include <com/sun/star/sdbc/ColumnSearch.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdb/SQLFilterOperator.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <comphelper/string.hxx>
#include <tools/diagnose_ex.h>
#include <osl/diagnose.h>
#include <connectivity/sqliterator.hxx>
#include <connectivity/dbtools.hxx>
#include "dbustrings.hrc"
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>

using namespace dbaui;
using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;

void Replace_OS_PlaceHolder(OUString& aString)
{
    aString = aString.replaceAll( "*", "%" );
    aString = aString.replaceAll( "?", "_" );
}

void Replace_SQL_PlaceHolder(OUString& aString)
{
    aString = aString.replaceAll( "%", "*" );
    aString = aString.replaceAll( "_", "?" );
}


DlgFilterCrit::DlgFilterCrit(vcl::Window * pParent,
                             const Reference< XComponentContext >& rxContext,
                             const Reference< XConnection>& _rxConnection,
                             const Reference< XSingleSelectQueryComposer >& _rxComposer,
                             const Reference< XNameAccess>& _rxCols
                             )
    : ModalDialog(pParent, "QueryFilterDialog",
        "dbaccess/ui/queryfilterdialog.ui")

    ,m_xQueryComposer(_rxComposer)
    ,m_xColumns( _rxCols )
    ,m_xConnection( _rxConnection )
    ,m_xMetaData( _rxConnection->getMetaData() )
    ,m_aPredicateInput( rxContext, _rxConnection, getParseContext() )
{
    get(m_pLB_WHEREFIELD1, "field1");
    get(m_pLB_WHERECOMP1, "cond1");
    get(m_pET_WHEREVALUE1, "value1");

    get(m_pLB_WHERECOND2, "op2");
    get(m_pLB_WHEREFIELD2, "field2");
    get(m_pLB_WHERECOMP2, "cond2");
    get(m_pET_WHEREVALUE2, "value2");

    get(m_pLB_WHERECOND3, "op3");
    get(m_pLB_WHEREFIELD3, "field3");
    get(m_pLB_WHERECOMP3, "cond3");
    get(m_pET_WHEREVALUE3, "value3");

    //set all condition preferred width to max width
    //if all entries exist
    Size aSize(m_pLB_WHERECOMP1->get_preferred_size());
    m_pLB_WHERECOMP1->set_width_request(aSize.Width());
    m_pLB_WHERECOMP2->set_width_request(aSize.Width());
    m_pLB_WHERECOMP3->set_width_request(aSize.Width());
    const sal_Int32 nEntryCount =  m_pLB_WHERECOMP1->GetEntryCount();
    for (sal_Int32 i = 0; i < nEntryCount; ++i)
    {
        if (i > 0)
            m_aSTR_COMPARE_OPERATORS += ";";
        m_aSTR_COMPARE_OPERATORS += m_pLB_WHERECOMP1->GetEntry(i);
    }
    m_pLB_WHERECOMP1->Clear();

    // ... also write it into the remaining fields
    Sequence< OUString> aNames = m_xColumns->getElementNames();
    const OUString* pIter = aNames.getConstArray();
    const OUString* pEnd   = pIter + aNames.getLength();
    Reference<XPropertySet> xColumn;
    for(;pIter != pEnd;++pIter)
    {
        try
        {
            xColumn.set( m_xColumns->getByName( *pIter ), UNO_QUERY_THROW );

            sal_Int32 nDataType( 0 );
            OSL_VERIFY( xColumn->getPropertyValue( PROPERTY_TYPE ) >>= nDataType );
            sal_Int32 eColumnSearch = ::dbtools::getSearchColumnFlag( m_xConnection, nDataType );
            if ( eColumnSearch == ColumnSearch::NONE )
                continue;

            bool bIsSearchable( true );
            OSL_VERIFY( xColumn->getPropertyValue( PROPERTY_ISSEARCHABLE ) >>= bIsSearchable );
            if ( !bIsSearchable )
                continue;
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        m_pLB_WHEREFIELD1->InsertEntry( *pIter );
        m_pLB_WHEREFIELD2->InsertEntry( *pIter );
        m_pLB_WHEREFIELD3->InsertEntry( *pIter );
    }

    Reference<XNameAccess> xSelectColumns = Reference<XColumnsSupplier>(m_xQueryComposer,UNO_QUERY)->getColumns();
    aNames = xSelectColumns->getElementNames();
    pIter = aNames.getConstArray();
    pEnd   = pIter + aNames.getLength();
    for(;pIter != pEnd;++pIter)
    {
        // don't insert a column name twice
        if ( !m_xColumns->hasByName(*pIter) )
        {
            xColumn.set(xSelectColumns->getByName(*pIter),UNO_QUERY);
            OSL_ENSURE(xColumn.is(),"DlgFilterCrit::DlgFilterCrit: Column is null!");
            sal_Int32 nDataType(0);
            xColumn->getPropertyValue(PROPERTY_TYPE) >>= nDataType;
            sal_Int32 eColumnSearch = dbtools::getSearchColumnFlag(m_xConnection,nDataType);
            // TODO
            // !pColumn->IsFunction()
            if(eColumnSearch != ColumnSearch::NONE)
            {
                m_pLB_WHEREFIELD1->InsertEntry( *pIter );
                m_pLB_WHEREFIELD2->InsertEntry( *pIter );
                m_pLB_WHEREFIELD3->InsertEntry( *pIter );
            }
        }
    }
    // initialize the listboxes with noEntry
    m_pLB_WHEREFIELD1->SelectEntryPos(0);
    m_pLB_WHEREFIELD2->SelectEntryPos(0);
    m_pLB_WHEREFIELD3->SelectEntryPos(0);

    // insert the criteria into the dialog
    Sequence<Sequence<PropertyValue > > aValues = m_xQueryComposer->getStructuredFilter();
    fillLines(aValues);
    aValues = m_xQueryComposer->getStructuredHavingClause();
    fillLines(aValues);

    EnableLines();

    m_pLB_WHEREFIELD1->SetSelectHdl(LINK(this,DlgFilterCrit,ListSelectHdl));
    m_pLB_WHEREFIELD2->SetSelectHdl(LINK(this,DlgFilterCrit,ListSelectHdl));
    m_pLB_WHEREFIELD3->SetSelectHdl(LINK(this,DlgFilterCrit,ListSelectHdl));

    m_pLB_WHERECOMP1->SetSelectHdl(LINK(this,DlgFilterCrit,ListSelectCompHdl));
    m_pLB_WHERECOMP2->SetSelectHdl(LINK(this,DlgFilterCrit,ListSelectCompHdl));
    m_pLB_WHERECOMP3->SetSelectHdl(LINK(this,DlgFilterCrit,ListSelectCompHdl));

    m_pET_WHEREVALUE1->SetLoseFocusHdl( LINK( this, DlgFilterCrit, PredicateLoseFocus ) );
    m_pET_WHEREVALUE2->SetLoseFocusHdl( LINK( this, DlgFilterCrit, PredicateLoseFocus ) );
    m_pET_WHEREVALUE3->SetLoseFocusHdl( LINK( this, DlgFilterCrit, PredicateLoseFocus ) );

    if ( m_pET_WHEREVALUE1->IsEnabled() )
        m_pET_WHEREVALUE1->GrabFocus();
}

DlgFilterCrit::~DlgFilterCrit()
{
    disposeOnce();
}

void DlgFilterCrit::dispose()
{
    m_pLB_WHEREFIELD1.clear();
    m_pLB_WHERECOMP1.clear();
    m_pET_WHEREVALUE1.clear();
    m_pLB_WHERECOND2.clear();
    m_pLB_WHEREFIELD2.clear();
    m_pLB_WHERECOMP2.clear();
    m_pET_WHEREVALUE2.clear();
    m_pLB_WHERECOND3.clear();
    m_pLB_WHEREFIELD3.clear();
    m_pLB_WHERECOMP3.clear();
    m_pET_WHEREVALUE3.clear();
    ModalDialog::dispose();
}

#define LbText(x)       ((x).GetSelectEntry())
#define LbPos(x)        ((x).GetSelectEntryPos())

sal_Int32 DlgFilterCrit::GetOSQLPredicateType( const OUString& _rSelectedPredicate ) const
{
    sal_Int32 nPredicateIndex = -1;
    for ( sal_Int32 i=0; i < comphelper::string::getTokenCount(m_aSTR_COMPARE_OPERATORS, ';'); ++i)
        if ( m_aSTR_COMPARE_OPERATORS.getToken(i, ';') == _rSelectedPredicate )
        {
            nPredicateIndex = i;
            break;
        }

    sal_Int32 nPredicateType = SQLFilterOperator::NOT_SQLNULL;
    switch ( nPredicateIndex )
    {
    case 0:
        nPredicateType = SQLFilterOperator::EQUAL;
        break;
    case 1:
        nPredicateType = SQLFilterOperator::NOT_EQUAL;
        break;
    case 2:
        nPredicateType = SQLFilterOperator::LESS;
        break;
    case 3:
        nPredicateType = SQLFilterOperator::LESS_EQUAL;
        break;
    case 4:
        nPredicateType = SQLFilterOperator::GREATER;
        break;
    case 5:
        nPredicateType = SQLFilterOperator::GREATER_EQUAL;
        break;
    case 6:
        nPredicateType = SQLFilterOperator::LIKE;
        break;
    case 7:
        nPredicateType = SQLFilterOperator::NOT_LIKE;
        break;
    case 8:
        nPredicateType = SQLFilterOperator::SQLNULL;
        break;
    case 9:
        nPredicateType = SQLFilterOperator::NOT_SQLNULL;
        break;
    default:
        OSL_FAIL( "DlgFilterCrit::GetOSQLPredicateType: unknown predicate string!" );
        break;
    }

    return nPredicateType;
}

sal_Int32 DlgFilterCrit::GetSelectionPos(sal_Int32 eType,const ListBox& rListBox)
{
    sal_Int32 nPos;
    switch(eType)
    {
        case SQLFilterOperator::EQUAL:
            nPos = 0;
            break;
        case SQLFilterOperator::NOT_EQUAL:
            nPos = 1;
            break;
        case SQLFilterOperator::LESS:
            nPos = 2;
            break;
        case SQLFilterOperator::LESS_EQUAL:
            nPos = 3;
            break;
        case SQLFilterOperator::GREATER:
            nPos = 4;
            break;
        case SQLFilterOperator::GREATER_EQUAL:
            nPos = 5;
            break;
        case SQLFilterOperator::NOT_LIKE:
            nPos = rListBox.GetEntryCount() > 2 ? rListBox.GetEntryCount()-3 : 0;
            break;
        case SQLFilterOperator::LIKE:
            nPos = rListBox.GetEntryCount() > 2 ? rListBox.GetEntryCount()-4 : 1;
            break;
        case SQLFilterOperator::SQLNULL:
            nPos = rListBox.GetEntryCount()-2;
            break;
        case SQLFilterOperator::NOT_SQLNULL:
            nPos = rListBox.GetEntryCount()-1;
            break;
        default:
            //  TODO  What value should this be?
            nPos = 0;
            break;
    }
    return nPos;
}

bool DlgFilterCrit::getCondition(const ListBox& _rField,const ListBox& _rComp,const Edit& _rValue,PropertyValue& _rFilter) const
{
    bool bHaving = false;
    try
    {
        OUString sTableName;
        _rFilter.Name = _rField.GetSelectEntry();
        Reference< XPropertySet > xColumn = getQueryColumn(_rFilter.Name);
        if ( xColumn.is() )
        {
            bool bFunction = false;
            Reference< XPropertySetInfo > xInfo = xColumn->getPropertySetInfo();
            if ( xInfo->hasPropertyByName(PROPERTY_REALNAME) )
            {
                if ( xInfo->hasPropertyByName(PROPERTY_TABLENAME) )
                {
                    xColumn->getPropertyValue(PROPERTY_TABLENAME)   >>= sTableName;
                    if ( !sTableName.isEmpty() )
                    {
                        // properly quote all parts of the table name, so
                        // e.g. <schema>.<table> becomes "<schema>"."<table>"
                        OUString aCatlog,aSchema,aTable;
                        ::dbtools::qualifiedNameComponents( m_xMetaData, sTableName, aCatlog, aSchema, aTable, ::dbtools::eInDataManipulation );
                        sTableName = ::dbtools::composeTableName( m_xMetaData, aCatlog, aSchema, aTable, true, ::dbtools::eInDataManipulation );
                    }
                }
                xColumn->getPropertyValue(PROPERTY_REALNAME)    >>= _rFilter.Name;
                static const char sAgg[] = "AggregateFunction";
                if ( xInfo->hasPropertyByName(sAgg) )
                    xColumn->getPropertyValue(sAgg) >>= bHaving;
                static const char sFunction[] = "Function";
                if ( xInfo->hasPropertyByName(sFunction) )
                    xColumn->getPropertyValue(sFunction) >>= bFunction;
            }
            if ( !bFunction )
            {
                const OUString aQuote    = m_xMetaData.is() ? m_xMetaData->getIdentifierQuoteString() : OUString();
                _rFilter.Name = ::dbtools::quoteName(aQuote,_rFilter.Name);
                if ( !sTableName.isEmpty() )
                {
                    static const char sSep[] = ".";
                    sTableName += sSep;
                    sTableName += _rFilter.Name;
                    _rFilter.Name = sTableName;
                }
            }
        }
    }
    catch(const Exception&)
    {
    }

    _rFilter.Handle = GetOSQLPredicateType( _rComp.GetSelectEntry() );
    if ( SQLFilterOperator::SQLNULL != _rFilter.Handle && _rFilter.Handle != SQLFilterOperator::NOT_SQLNULL )
    {
        OUString sPredicateValue;
        m_aPredicateInput.getPredicateValue( _rValue.GetText(), getMatchingColumn( _rValue ) ) >>= sPredicateValue;
        if ( _rFilter.Handle == SQLFilterOperator::LIKE ||
             _rFilter.Handle == SQLFilterOperator::NOT_LIKE )
            ::Replace_OS_PlaceHolder( sPredicateValue );
        _rFilter.Value <<= OUString(sPredicateValue);
    }
    return bHaving;
}

Reference< XPropertySet > DlgFilterCrit::getColumn( const OUString& _rFieldName ) const
{
    Reference< XPropertySet > xColumn;
    try
    {
        if ( m_xColumns.is() && m_xColumns->hasByName( _rFieldName ) )
            m_xColumns->getByName( _rFieldName ) >>= xColumn;

        Reference< XNameAccess> xColumns = Reference< XColumnsSupplier >(m_xQueryComposer,UNO_QUERY)->getColumns();
        if ( xColumns.is() && !xColumn.is() )
        {
            Sequence< OUString> aSeq = xColumns->getElementNames();
            const OUString* pIter = aSeq.getConstArray();
            const OUString* pEnd   = pIter + aSeq.getLength();
            for(;pIter != pEnd;++pIter)
            {
                Reference<XPropertySet> xProp(xColumns->getByName(*pIter),UNO_QUERY);
                if ( xProp.is() && xProp->getPropertySetInfo()->hasPropertyByName(PROPERTY_REALNAME) )
                {
                    OUString sRealName;
                    xProp->getPropertyValue(PROPERTY_REALNAME)  >>= sRealName;
                    if ( sRealName == _rFieldName )
                    {
                        if ( m_xColumns.is() && m_xColumns->hasByName( *pIter ) )
                            m_xColumns->getByName( *pIter ) >>= xColumn;
                        break;
                    }
                }
            }
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    return xColumn;
}

Reference< XPropertySet > DlgFilterCrit::getQueryColumn( const OUString& _rFieldName ) const
{
    Reference< XPropertySet > xColumn;
    try
    {
        Reference< XNameAccess> xColumns = Reference< XColumnsSupplier >(m_xQueryComposer,UNO_QUERY)->getColumns();
        if ( xColumns.is() && xColumns->hasByName( _rFieldName ) )
            xColumns->getByName( _rFieldName ) >>= xColumn;
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    return xColumn;
}

Reference< XPropertySet > DlgFilterCrit::getMatchingColumn( const Edit& _rValueInput ) const
{
    // the name
    OUString sField;
    if ( &_rValueInput == m_pET_WHEREVALUE1 )
    {
        sField = m_pLB_WHEREFIELD1->GetSelectEntry();
    }
    else if ( &_rValueInput == m_pET_WHEREVALUE2 )
    {
        sField = m_pLB_WHEREFIELD2->GetSelectEntry();
    }
    else if ( &_rValueInput == m_pET_WHEREVALUE3 )
    {
        sField = m_pLB_WHEREFIELD3->GetSelectEntry();
    }
    else {
        OSL_FAIL( "DlgFilterCrit::getMatchingColumn: invalid event source!" );
    }

    // the field itself
    return getColumn( sField );
}

IMPL_LINK_TYPED( DlgFilterCrit, PredicateLoseFocus, Control&, rControl, void )
{
    Edit* _pField = static_cast<Edit*>(&rControl);
    // retrieve the field affected
    Reference< XPropertySet> xColumn( getMatchingColumn( *_pField ) );
    // and normalize it's content
    if ( xColumn.is() )
    {
        OUString sText( _pField->GetText() );
        m_aPredicateInput.normalizePredicateString( sText, xColumn );
        _pField->SetText( sText );
    }
}

void DlgFilterCrit::SetLine( sal_uInt16 nIdx,const PropertyValue& _rItem,bool _bOr  )
{
    OUString aCondition;
    _rItem.Value >>= aCondition;
    OUString aStr = aCondition;
    if ( _rItem.Handle == SQLFilterOperator::LIKE ||
         _rItem.Handle == SQLFilterOperator::NOT_LIKE )
        ::Replace_SQL_PlaceHolder(aStr);
    aStr = comphelper::string::stripEnd(aStr, ' ');

    Reference< XPropertySet > xColumn = getColumn( _rItem.Name );

    // remove the predicate from the condition
    switch(_rItem.Handle)
    {
        case SQLFilterOperator::EQUAL:
            //  aStr.Erase(0,1);
            break;
        case SQLFilterOperator::NOT_EQUAL:
            aStr = aStr.copy(2);
            break;
        case SQLFilterOperator::LESS:
            aStr = aStr.copy(1);
            break;
        case SQLFilterOperator::LESS_EQUAL:
            aStr = aStr.copy(2);
            break;
        case SQLFilterOperator::GREATER:
            aStr = aStr.copy(1);
            break;
        case SQLFilterOperator::GREATER_EQUAL:
            aStr = aStr.copy(2);
            break;
        case SQLFilterOperator::NOT_LIKE:
            aStr = aStr.copy(8);
            break;
        case SQLFilterOperator::LIKE:
            aStr = aStr.copy(4);
            break;
        case SQLFilterOperator::SQLNULL:
            aStr = aStr.copy(7);
            break;
        case SQLFilterOperator::NOT_SQLNULL:
            aStr = aStr.copy(11);
            break;
    }
    aStr = comphelper::string::stripStart(aStr, ' ');

    // to make sure that we only set first three
    ListBox* pColumnListControl =  NULL;
    ListBox* pPredicateListControl = NULL;
    Edit* pPredicateValueControl = NULL;
    switch( nIdx )
    {
        case 0:
            pColumnListControl = m_pLB_WHEREFIELD1;
            pPredicateListControl = m_pLB_WHERECOMP1;
            pPredicateValueControl = m_pET_WHEREVALUE1;
            break;
        case 1:
            m_pLB_WHERECOND2->SelectEntryPos( _bOr ? 1 : 0 );

            pColumnListControl = m_pLB_WHEREFIELD2;
            pPredicateListControl = m_pLB_WHERECOMP2;
            pPredicateValueControl = m_pET_WHEREVALUE2;
            break;
        case 2:
            m_pLB_WHERECOND3->SelectEntryPos( _bOr ? 1 : 0 );

            pColumnListControl = m_pLB_WHEREFIELD3;
            pPredicateListControl = m_pLB_WHERECOMP3;
            pPredicateValueControl = m_pET_WHEREVALUE3;
            break;
    }

    if ( pColumnListControl && pPredicateListControl && pPredicateValueControl )
    {
        OUString sName;
        if ( xColumn.is() )
            xColumn->getPropertyValue(PROPERTY_NAME) >>= sName;
        else
            sName = _rItem.Name;
        // select the appropriate field name
        SelectField( *pColumnListControl, sName );
        ListSelectHdl( pColumnListControl );

        // select the appropriate condition
        pPredicateListControl->SelectEntryPos( GetSelectionPos( (sal_Int32)_rItem.Handle, *pPredicateListControl ) );

        // initially normalize this value
        OUString aString( aStr );
        m_aPredicateInput.normalizePredicateString( aString, xColumn );
        pPredicateValueControl->SetText( aString );
    }
}

void DlgFilterCrit::SelectField( ListBox& rBox, const OUString& rField )
{
    const sal_Int32 nCnt = rBox.GetEntryCount();

    for( sal_Int32 i=0 ; i<nCnt ; i++ )
    {
        if(rBox.GetEntry(i) == rField)
        {
            rBox.SelectEntryPos(i);
            return;
        }
    }

    rBox.SelectEntryPos(0);
}

void DlgFilterCrit::EnableLines()
{
    // enabling/disabling of whole lines
    if( LbPos(*m_pLB_WHEREFIELD1) == 0 )
    {
        m_pLB_WHEREFIELD2->Disable();
        m_pLB_WHERECOND2->Disable();
        m_pLB_WHERECOMP2->Disable();
        m_pET_WHEREVALUE2->Disable();

        m_pLB_WHEREFIELD3->Disable();
        m_pLB_WHERECOND3->Disable();
        m_pLB_WHERECOMP3->Disable();
        m_pET_WHEREVALUE3->Disable();
    }
    else
    {
        m_pLB_WHEREFIELD2->Enable();
        m_pLB_WHERECOND2->Enable();
        m_pLB_WHERECOMP2->Enable();
        m_pET_WHEREVALUE2->Enable();

        m_pLB_WHEREFIELD3->Enable();
        m_pLB_WHERECOND3->Enable();
        m_pLB_WHERECOMP3->Enable();
        m_pET_WHEREVALUE3->Enable();
    }

    if( LbPos(*m_pLB_WHEREFIELD2) == 0 )
    {
        m_pLB_WHEREFIELD3->Disable();
        m_pLB_WHERECOND3->Disable();
        m_pLB_WHERECOMP3->Disable();
        m_pET_WHEREVALUE3->Disable();
    }
    else
    {
        m_pLB_WHEREFIELD3->Enable();
        m_pLB_WHERECOND3->Enable();
        m_pLB_WHERECOMP3->Enable();
        m_pET_WHEREVALUE3->Enable();
    }

    // comparison field equal to NOENTRY
    if( LbPos(*m_pLB_WHEREFIELD1) == 0 )
    {
        m_pLB_WHERECOMP1->Disable();
        m_pET_WHEREVALUE1->Disable();
    }
    else
    {
        m_pLB_WHEREFIELD1->Enable();
        m_pLB_WHERECOMP1->Enable();
        m_pET_WHEREVALUE1->Enable();
    }

    if( LbPos(*m_pLB_WHEREFIELD2) == 0 )
    {
        m_pLB_WHERECOND2->Disable();
        m_pLB_WHERECOMP2->Disable();
        m_pET_WHEREVALUE2->Disable();
    }
    else
    {
        m_pLB_WHERECOND2->Enable();
        m_pLB_WHEREFIELD2->Enable();
        m_pLB_WHERECOMP2->Enable();
        m_pET_WHEREVALUE2->Enable();
    }

    if( LbPos(*m_pLB_WHEREFIELD3) == 0 )
    {
        m_pLB_WHERECOND3->Disable();
        m_pLB_WHERECOMP3->Disable();
        m_pET_WHEREVALUE3->Disable();
    }
    else
    {
        m_pLB_WHERECOND3->Enable();
        m_pLB_WHERECOND3->Enable();
        m_pLB_WHEREFIELD3->Enable();
        m_pLB_WHERECOMP3->Enable();
        m_pET_WHEREVALUE3->Enable();
    }

    // comparison operator equal to ISNULL or ISNOTNULL
    if(m_pLB_WHERECOMP1->GetEntryCount() > 2 &&
        ((LbPos(*m_pLB_WHERECOMP1) == m_pLB_WHERECOMP1->GetEntryCount()-1) ||
         (LbPos(*m_pLB_WHERECOMP1) == m_pLB_WHERECOMP1->GetEntryCount()-2)) )
        m_pET_WHEREVALUE1->Disable();

    if(m_pLB_WHERECOMP2->GetEntryCount() > 2 &&
        ((LbPos(*m_pLB_WHERECOMP2) == m_pLB_WHERECOMP2->GetEntryCount()-1) ||
         (LbPos(*m_pLB_WHERECOMP2) == m_pLB_WHERECOMP2->GetEntryCount()-2)) )
        m_pET_WHEREVALUE2->Disable();

    if(m_pLB_WHERECOMP3->GetEntryCount() > 2 &&
        ((LbPos(*m_pLB_WHERECOMP3) == m_pLB_WHERECOMP3->GetEntryCount()-1) ||
         (LbPos(*m_pLB_WHERECOMP3) == m_pLB_WHERECOMP3->GetEntryCount()-2)) )
        m_pET_WHEREVALUE3->Disable();
}

IMPL_LINK( DlgFilterCrit, ListSelectHdl, ListBox *, pListBox )
{
    OUString aName;
    ListBox* pComp;
    if(pListBox == m_pLB_WHEREFIELD1)
    {
        aName = LbText(*m_pLB_WHEREFIELD1);
        pComp = m_pLB_WHERECOMP1;
    }
    else if(pListBox == m_pLB_WHEREFIELD2)
    {
        aName = LbText(*m_pLB_WHEREFIELD2);
        pComp = m_pLB_WHERECOMP2;
    }
    else
    {
        aName = LbText(*m_pLB_WHEREFIELD3);
        pComp = m_pLB_WHERECOMP3;
    }

    pComp->Clear();

    Reference<XPropertySet> xColumn = getColumn(aName);
    if ( xColumn.is() )
    {
        sal_Int32 nDataType = 0;
        xColumn->getPropertyValue(PROPERTY_TYPE) >>= nDataType;
        sal_Int32 eColumnSearch = dbtools::getSearchColumnFlag(m_xConnection,nDataType);

        if(eColumnSearch  == ColumnSearch::FULL)
        {
            for(sal_Int32 i=0;i < comphelper::string::getTokenCount(m_aSTR_COMPARE_OPERATORS, ';');i++)
                pComp->InsertEntry(m_aSTR_COMPARE_OPERATORS.getToken(i, ';'));
        }
        else if(eColumnSearch == ColumnSearch::CHAR)
        {
            for(sal_Int32 i=6; i<10; i++)
                pComp->InsertEntry(m_aSTR_COMPARE_OPERATORS.getToken(i, ';'));
        }
        else if(eColumnSearch == ColumnSearch::BASIC)
        {
            sal_Int32 i;
            for( i = 0; i < 6; i++ )
                pComp->InsertEntry(m_aSTR_COMPARE_OPERATORS.getToken(i, ';'));
            for(i=8; i < comphelper::string::getTokenCount(m_aSTR_COMPARE_OPERATORS, ';'); ++i)
                pComp->InsertEntry(m_aSTR_COMPARE_OPERATORS.getToken(i, ';'));
        }
        else
        {
            OSL_FAIL("DlgFilterCrit::ListSelectHdl: This column should not exist at all.");
        }
    }
    pComp->SelectEntryPos(0);

    EnableLines();
    return 0;
}

IMPL_LINK( DlgFilterCrit, ListSelectCompHdl, ListBox *, /*pListBox*/ )
{
    EnableLines();
    return 0;
}

void DlgFilterCrit::BuildWherePart()
{
    Sequence<Sequence<PropertyValue> > aFilter,aHaving;
    aFilter.realloc(1);
    aHaving.realloc(1);

    if( LbPos(*m_pLB_WHEREFIELD1) != 0 )
    {
        PropertyValue aValue;
        if ( getCondition(*m_pLB_WHEREFIELD1,*m_pLB_WHERECOMP1,*m_pET_WHEREVALUE1,aValue) )
        {
            aHaving[0].realloc(1);
            aHaving[0][0] = aValue;
        }
        else
        {
            aFilter[0].realloc(1);
            aFilter[0][0] = aValue;
        }
    }

    if( LbPos(*m_pLB_WHEREFIELD2) != 0 )
    {
        PropertyValue aValue;
        Sequence<Sequence<PropertyValue> >& _rValues = aFilter;
        if ( getCondition(*m_pLB_WHEREFIELD2,*m_pLB_WHERECOMP2,*m_pET_WHEREVALUE2,aValue) )
            _rValues = aHaving;
        PropertyValue* pPos = NULL;
        if ( m_pLB_WHERECOND2->GetSelectEntryPos() )
        {
            sal_Int32 nPos = _rValues.getLength();
            _rValues.realloc( nPos + 1);
            _rValues[nPos].realloc( 1);
            pPos = &_rValues[nPos][0];
        }
        else
        {
            sal_Int32 nPos = _rValues.getLength() - 1;
            sal_Int32 nAndPos = _rValues[nPos].getLength();
            _rValues[nPos].realloc( _rValues[nPos].getLength() + 1);
            pPos = &_rValues[nPos][nAndPos];
        }
        *pPos = aValue;
    }

    if( LbPos(*m_pLB_WHEREFIELD3) != 0 )
    {
        PropertyValue aValue;
        Sequence<Sequence<PropertyValue> >& _rValues = aFilter;
        if ( getCondition(*m_pLB_WHEREFIELD3,*m_pLB_WHERECOMP3,*m_pET_WHEREVALUE3,aValue) )
            _rValues = aHaving;
        PropertyValue* pPos = NULL;
        if ( m_pLB_WHERECOND3->GetSelectEntryPos() )
        {
            sal_Int32 nPos = _rValues.getLength();
            _rValues.realloc( nPos + 1);
            _rValues[nPos].realloc( 1);
            pPos = &_rValues[nPos][0];
        }
        else
        {
            sal_Int32 nPos = _rValues.getLength() - 1;
            sal_Int32 nAndPos = _rValues[nPos].getLength();
            _rValues[nPos].realloc( _rValues[nPos].getLength() + 1);
            pPos = &_rValues[nPos][nAndPos];
        }
        *pPos = aValue;
    }
    try
    {
        m_xQueryComposer->setStructuredFilter(aFilter);
        m_xQueryComposer->setStructuredHavingClause(aHaving);
    }
    catch(const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

void DlgFilterCrit::fillLines(const Sequence<Sequence<PropertyValue > >& _aValues)
{
    const Sequence<PropertyValue >* pOrIter = _aValues.getConstArray();
    const Sequence<PropertyValue >* pOrEnd   = pOrIter + _aValues.getLength();
    for(sal_uInt16 i=0;pOrIter != pOrEnd; ++pOrIter)
    {
        bool bOr = true;
        const PropertyValue* pAndIter   = pOrIter->getConstArray();
        const PropertyValue* pAndEnd    = pAndIter + pOrIter->getLength();
        for(;pAndIter != pAndEnd; ++pAndIter)
        {
            SetLine( i++,*pAndIter,bOr);
            bOr = false;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
