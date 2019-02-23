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

#include <queryfilter.hxx>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/sdb/XSQLQueryComposer.hpp>
#include <com/sun/star/sdbc/ColumnSearch.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdb/SQLFilterOperator.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <comphelper/string.hxx>
#include <tools/diagnose_ex.h>
#include <osl/diagnose.h>
#include <connectivity/sqliterator.hxx>
#include <connectivity/dbtools.hxx>
#include <stringconstants.hxx>
#include <strings.hxx>
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

static void Replace_OS_PlaceHolder(OUString& aString)
{
    aString = aString.replaceAll( "*", "%" );
    aString = aString.replaceAll( "?", "_" );
}

static void Replace_SQL_PlaceHolder(OUString& aString)
{
    aString = aString.replaceAll( "%", "*" );
    aString = aString.replaceAll( "_", "?" );
}

DlgFilterCrit::DlgFilterCrit(weld::Window * pParent,
                             const Reference< XComponentContext >& rxContext,
                             const Reference< XConnection>& _rxConnection,
                             const Reference< XSingleSelectQueryComposer >& _rxComposer,
                             const Reference< XNameAccess>& _rxCols)
    : GenericDialogController(pParent, "dbaccess/ui/queryfilterdialog.ui", "QueryFilterDialog")
    , m_xQueryComposer(_rxComposer)
    , m_xColumns( _rxCols )
    , m_xConnection( _rxConnection )
    , m_xMetaData( _rxConnection->getMetaData() )
    , m_aPredicateInput( rxContext, _rxConnection, getParseContext() )
    , m_xLB_WHEREFIELD1(m_xBuilder->weld_combo_box("field1"))
    , m_xLB_WHERECOMP1(m_xBuilder->weld_combo_box("cond1"))
    , m_xET_WHEREVALUE1(m_xBuilder->weld_entry("value1"))
    , m_xLB_WHERECOND2(m_xBuilder->weld_combo_box("op2"))
    , m_xLB_WHEREFIELD2(m_xBuilder->weld_combo_box("field2"))
    , m_xLB_WHERECOMP2(m_xBuilder->weld_combo_box("cond2"))
    , m_xET_WHEREVALUE2(m_xBuilder->weld_entry("value2"))
    , m_xLB_WHERECOND3(m_xBuilder->weld_combo_box("op3"))
    , m_xLB_WHEREFIELD3(m_xBuilder->weld_combo_box("field3"))
    , m_xLB_WHERECOMP3(m_xBuilder->weld_combo_box("cond3"))
    , m_xET_WHEREVALUE3(m_xBuilder->weld_entry("value3"))
{
    //set all condition preferred width to max width
    //if all entries exist
    Size aSize(m_xLB_WHERECOMP1->get_preferred_size());
    m_xLB_WHERECOMP1->set_size_request(aSize.Width(), -1);
    m_xLB_WHERECOMP2->set_size_request(aSize.Width(), -1);
    m_xLB_WHERECOMP3->set_size_request(aSize.Width(), -1);
    const sal_Int32 nEntryCount =  m_xLB_WHERECOMP1->get_count();
    m_aSTR_COMPARE_OPERATORS.resize(nEntryCount);
    for (sal_Int32 i = 0; i < nEntryCount; ++i)
    {
        m_aSTR_COMPARE_OPERATORS[i] = m_xLB_WHERECOMP1->get_text(i);
    }
    m_xLB_WHERECOMP1->clear();

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
            DBG_UNHANDLED_EXCEPTION("dbaccess");
        }
        m_xLB_WHEREFIELD1->append_text( *pIter );
        m_xLB_WHEREFIELD2->append_text( *pIter );
        m_xLB_WHEREFIELD3->append_text( *pIter );
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
                m_xLB_WHEREFIELD1->append_text( *pIter );
                m_xLB_WHEREFIELD2->append_text( *pIter );
                m_xLB_WHEREFIELD3->append_text( *pIter );
            }
        }
    }
    // initialize the listboxes with noEntry
    m_xLB_WHEREFIELD1->set_active(0);
    m_xLB_WHEREFIELD2->set_active(0);
    m_xLB_WHEREFIELD3->set_active(0);

    // insert the criteria into the dialog
    Sequence<Sequence<PropertyValue > > aValues = m_xQueryComposer->getStructuredFilter();
    int i(0);
    fillLines(i, aValues);
    aValues = m_xQueryComposer->getStructuredHavingClause();
    fillLines(i, aValues);

    EnableLines();

    m_xLB_WHEREFIELD1->connect_changed(LINK(this,DlgFilterCrit,ListSelectHdl));
    m_xLB_WHEREFIELD2->connect_changed(LINK(this,DlgFilterCrit,ListSelectHdl));
    m_xLB_WHEREFIELD3->connect_changed(LINK(this,DlgFilterCrit,ListSelectHdl));

    m_xLB_WHERECOMP1->connect_changed(LINK(this,DlgFilterCrit,ListSelectCompHdl));
    m_xLB_WHERECOMP2->connect_changed(LINK(this,DlgFilterCrit,ListSelectCompHdl));
    m_xLB_WHERECOMP3->connect_changed(LINK(this,DlgFilterCrit,ListSelectCompHdl));

    m_xET_WHEREVALUE1->connect_focus_out( LINK( this, DlgFilterCrit, PredicateLoseFocus ) );
    m_xET_WHEREVALUE2->connect_focus_out( LINK( this, DlgFilterCrit, PredicateLoseFocus ) );
    m_xET_WHEREVALUE3->connect_focus_out( LINK( this, DlgFilterCrit, PredicateLoseFocus ) );

    if (m_xET_WHEREVALUE1->get_sensitive())
        m_xET_WHEREVALUE1->grab_focus();
}

DlgFilterCrit::~DlgFilterCrit()
{
}

#define LbText(x)       ((x).get_active_text())
#define LbPos(x)        ((x).get_active())

sal_Int32 DlgFilterCrit::GetOSQLPredicateType( const OUString& _rSelectedPredicate ) const
{
    sal_Int32 nPredicateIndex = -1;
    for ( size_t i=0; i < m_aSTR_COMPARE_OPERATORS.size(); ++i)
        if ( m_aSTR_COMPARE_OPERATORS[i] == _rSelectedPredicate )
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

sal_Int32 DlgFilterCrit::GetSelectionPos(sal_Int32 eType, const weld::ComboBox& rListBox)
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
            nPos = rListBox.get_count() > 2 ? rListBox.get_count()-3 : 0;
            break;
        case SQLFilterOperator::LIKE:
            nPos = rListBox.get_count() > 2 ? rListBox.get_count()-4 : 1;
            break;
        case SQLFilterOperator::SQLNULL:
            nPos = rListBox.get_count()-2;
            break;
        case SQLFilterOperator::NOT_SQLNULL:
            nPos = rListBox.get_count()-1;
            break;
        default:
            //  TODO  What value should this be?
            nPos = 0;
            break;
    }
    return nPos;
}

bool DlgFilterCrit::getCondition(const weld::ComboBox& _rField,const weld::ComboBox& _rComp,const weld::Entry& _rValue,PropertyValue& _rFilter) const
{
    bool bHaving = false;
    try
    {
        OUString sTableName;
        _rFilter.Name = _rField.get_active_text();
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
                        ::dbtools::qualifiedNameComponents( m_xMetaData, sTableName, aCatlog, aSchema, aTable, ::dbtools::EComposeRule::InDataManipulation );
                        sTableName = ::dbtools::composeTableName( m_xMetaData, aCatlog, aSchema, aTable, true, ::dbtools::EComposeRule::InDataManipulation );
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
                    sTableName += ".";
                    sTableName += _rFilter.Name;
                    _rFilter.Name = sTableName;
                }
            }
        }
    }
    catch(const Exception&)
    {
    }

    _rFilter.Handle = GetOSQLPredicateType( _rComp.get_active_text() );
    if ( SQLFilterOperator::SQLNULL != _rFilter.Handle && _rFilter.Handle != SQLFilterOperator::NOT_SQLNULL )
    {
        OUString sPredicateValue;
        m_aPredicateInput.getPredicateValue( _rValue.get_text(), getMatchingColumn( _rValue ) ) >>= sPredicateValue;
        if ( _rFilter.Handle == SQLFilterOperator::LIKE ||
             _rFilter.Handle == SQLFilterOperator::NOT_LIKE )
            ::Replace_OS_PlaceHolder( sPredicateValue );
        _rFilter.Value <<= sPredicateValue;
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
        DBG_UNHANDLED_EXCEPTION("dbaccess");
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
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }

    return xColumn;
}

Reference< XPropertySet > DlgFilterCrit::getMatchingColumn( const weld::Entry& _rValueInput ) const
{
    // the name
    OUString sField;
    if ( &_rValueInput == m_xET_WHEREVALUE1.get() )
    {
        sField = m_xLB_WHEREFIELD1->get_active_text();
    }
    else if ( &_rValueInput == m_xET_WHEREVALUE2.get() )
    {
        sField = m_xLB_WHEREFIELD2->get_active_text();
    }
    else if ( &_rValueInput == m_xET_WHEREVALUE3.get() )
    {
        sField = m_xLB_WHEREFIELD3->get_active_text();
    }
    else {
        OSL_FAIL( "DlgFilterCrit::getMatchingColumn: invalid event source!" );
    }

    // the field itself
    return getColumn( sField );
}

IMPL_LINK( DlgFilterCrit, PredicateLoseFocus, weld::Widget&, rControl, void )
{
    weld::Entry& rField = dynamic_cast<weld::Entry&>(rControl);
    // retrieve the field affected
    Reference< XPropertySet> xColumn(getMatchingColumn(rField));
    // and normalize its content
    if ( xColumn.is() )
    {
        OUString sText(rField.get_text());
        m_aPredicateInput.normalizePredicateString(sText, xColumn);
        rField.set_text(sText);
    }
}

void DlgFilterCrit::SetLine( int nIdx, const PropertyValue& _rItem, bool _bOr )
{
    OUString aStr;
    _rItem.Value >>= aStr;
    if ( _rItem.Handle == SQLFilterOperator::LIKE ||
         _rItem.Handle == SQLFilterOperator::NOT_LIKE )
        ::Replace_SQL_PlaceHolder(aStr);
    aStr = comphelper::string::stripEnd(aStr, ' ');

    Reference< XPropertySet > xColumn = getColumn( _rItem.Name );

    // to make sure that we only set first three
    weld::ComboBox* pColumnListControl =  nullptr;
    weld::ComboBox* pPredicateListControl = nullptr;
    weld::Entry* pPredicateValueControl = nullptr;
    switch( nIdx )
    {
        case 0:
            pColumnListControl = m_xLB_WHEREFIELD1.get();
            pPredicateListControl = m_xLB_WHERECOMP1.get();
            pPredicateValueControl = m_xET_WHEREVALUE1.get();
            break;
        case 1:
            m_xLB_WHERECOND2->set_active( _bOr ? 1 : 0 );

            pColumnListControl = m_xLB_WHEREFIELD2.get();
            pPredicateListControl = m_xLB_WHERECOMP2.get();
            pPredicateValueControl = m_xET_WHEREVALUE2.get();
            break;
        case 2:
            m_xLB_WHERECOND3->set_active( _bOr ? 1 : 0 );

            pColumnListControl = m_xLB_WHEREFIELD3.get();
            pPredicateListControl = m_xLB_WHERECOMP3.get();
            pPredicateValueControl = m_xET_WHEREVALUE3.get();
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
        ListSelectHdl( *pColumnListControl );

        // select the appropriate condition
        pPredicateListControl->set_active( GetSelectionPos( static_cast<sal_Int32>(_rItem.Handle), *pPredicateListControl ) );

        // initially normalize this value
        OUString aString( aStr );
        m_aPredicateInput.normalizePredicateString( aString, xColumn );
        pPredicateValueControl->set_text( aString );
    }
}

void DlgFilterCrit::SelectField(weld::ComboBox& rBox, const OUString& rField)
{
    const sal_Int32 nCnt = rBox.get_count();

    for( sal_Int32 i=0 ; i<nCnt ; i++ )
    {
        if (rBox.get_text(i) == rField)
        {
            rBox.set_active(i);
            return;
        }
    }

    rBox.set_active(0);
}

void DlgFilterCrit::EnableLines()
{
    // enabling/disabling of whole lines
    if( LbPos(*m_xLB_WHEREFIELD1) == 0 )
    {
        m_xLB_WHEREFIELD2->set_sensitive(false);
        m_xLB_WHERECOND2->set_sensitive(false);
        m_xLB_WHERECOMP2->set_sensitive(false);
        m_xET_WHEREVALUE2->set_sensitive(false);

        m_xLB_WHEREFIELD3->set_sensitive(false);
        m_xLB_WHERECOND3->set_sensitive(false);
        m_xLB_WHERECOMP3->set_sensitive(false);
        m_xET_WHEREVALUE3->set_sensitive(false);
    }
    else
    {
        m_xLB_WHEREFIELD2->set_sensitive(true);
        m_xLB_WHERECOND2->set_sensitive(true);
        m_xLB_WHERECOMP2->set_sensitive(true);
        m_xET_WHEREVALUE2->set_sensitive(true);

        m_xLB_WHEREFIELD3->set_sensitive(true);
        m_xLB_WHERECOND3->set_sensitive(true);
        m_xLB_WHERECOMP3->set_sensitive(true);
        m_xET_WHEREVALUE3->set_sensitive(true);
    }

    if( LbPos(*m_xLB_WHEREFIELD2) == 0 )
    {
        m_xLB_WHEREFIELD3->set_sensitive(false);
        m_xLB_WHERECOND3->set_sensitive(false);
        m_xLB_WHERECOMP3->set_sensitive(false);
        m_xET_WHEREVALUE3->set_sensitive(false);
    }
    else
    {
        m_xLB_WHEREFIELD3->set_sensitive(true);
        m_xLB_WHERECOND3->set_sensitive(true);
        m_xLB_WHERECOMP3->set_sensitive(true);
        m_xET_WHEREVALUE3->set_sensitive(true);
    }

    // comparison field equal to NOENTRY
    if( LbPos(*m_xLB_WHEREFIELD1) == 0 )
    {
        m_xLB_WHERECOMP1->set_sensitive(false);
        m_xET_WHEREVALUE1->set_sensitive(false);
    }
    else
    {
        m_xLB_WHEREFIELD1->set_sensitive(true);
        m_xLB_WHERECOMP1->set_sensitive(true);
        m_xET_WHEREVALUE1->set_sensitive(true);
    }

    if( LbPos(*m_xLB_WHEREFIELD2) == 0 )
    {
        m_xLB_WHERECOND2->set_sensitive(false);
        m_xLB_WHERECOMP2->set_sensitive(false);
        m_xET_WHEREVALUE2->set_sensitive(false);
    }
    else
    {
        m_xLB_WHERECOND2->set_sensitive(true);
        m_xLB_WHEREFIELD2->set_sensitive(true);
        m_xLB_WHERECOMP2->set_sensitive(true);
        m_xET_WHEREVALUE2->set_sensitive(true);
    }

    if( LbPos(*m_xLB_WHEREFIELD3) == 0 )
    {
        m_xLB_WHERECOND3->set_sensitive(false);
        m_xLB_WHERECOMP3->set_sensitive(false);
        m_xET_WHEREVALUE3->set_sensitive(false);
    }
    else
    {
        m_xLB_WHERECOND3->set_sensitive(true);
        m_xLB_WHERECOND3->set_sensitive(true);
        m_xLB_WHEREFIELD3->set_sensitive(true);
        m_xLB_WHERECOMP3->set_sensitive(true);
        m_xET_WHEREVALUE3->set_sensitive(true);
    }

    // comparison operator equal to ISNULL or ISNOTNULL
    if(m_xLB_WHERECOMP1->get_count() > 2 &&
        ((LbPos(*m_xLB_WHERECOMP1) == m_xLB_WHERECOMP1->get_count()-1) ||
         (LbPos(*m_xLB_WHERECOMP1) == m_xLB_WHERECOMP1->get_count()-2)) )
        m_xET_WHEREVALUE1->set_sensitive(false);

    if(m_xLB_WHERECOMP2->get_count() > 2 &&
        ((LbPos(*m_xLB_WHERECOMP2) == m_xLB_WHERECOMP2->get_count()-1) ||
         (LbPos(*m_xLB_WHERECOMP2) == m_xLB_WHERECOMP2->get_count()-2)) )
        m_xET_WHEREVALUE2->set_sensitive(false);

    if(m_xLB_WHERECOMP3->get_count() > 2 &&
        ((LbPos(*m_xLB_WHERECOMP3) == m_xLB_WHERECOMP3->get_count()-1) ||
         (LbPos(*m_xLB_WHERECOMP3) == m_xLB_WHERECOMP3->get_count()-2)) )
        m_xET_WHEREVALUE3->set_sensitive(false);
}

IMPL_LINK( DlgFilterCrit, ListSelectHdl, weld::ComboBox&, rListBox, void )
{
    OUString aName;
    weld::ComboBox* pComp;
    if(&rListBox == m_xLB_WHEREFIELD1.get())
    {
        aName = LbText(*m_xLB_WHEREFIELD1);
        pComp = m_xLB_WHERECOMP1.get();
    }
    else if(&rListBox == m_xLB_WHEREFIELD2.get())
    {
        aName = LbText(*m_xLB_WHEREFIELD2);
        pComp = m_xLB_WHERECOMP2.get();
    }
    else
    {
        aName = LbText(*m_xLB_WHEREFIELD3);
        pComp = m_xLB_WHERECOMP3.get();
    }

    pComp->clear();

    Reference<XPropertySet> xColumn = getColumn(aName);
    if ( xColumn.is() )
    {
        sal_Int32 nDataType = 0;
        xColumn->getPropertyValue(PROPERTY_TYPE) >>= nDataType;
        sal_Int32 eColumnSearch = dbtools::getSearchColumnFlag(m_xConnection,nDataType);

        if(eColumnSearch  == ColumnSearch::FULL)
        {
            for(size_t i=0;i < m_aSTR_COMPARE_OPERATORS.size(); i++)
                pComp->append_text(m_aSTR_COMPARE_OPERATORS[i]);
        }
        else if(eColumnSearch == ColumnSearch::CHAR)
        {
            for(sal_Int32 i=6; i<10; i++)
                pComp->append_text(m_aSTR_COMPARE_OPERATORS[i]);
        }
        else if(eColumnSearch == ColumnSearch::BASIC)
        {
            size_t i;
            for( i = 0; i < 6; i++ )
                pComp->append_text(m_aSTR_COMPARE_OPERATORS[i]);
            for(i=8; i < m_aSTR_COMPARE_OPERATORS.size(); ++i)
                pComp->append_text(m_aSTR_COMPARE_OPERATORS[i]);
        }
        else
        {
            OSL_FAIL("DlgFilterCrit::ListSelectHdl: This column should not exist at all.");
        }
    }
    pComp->set_active(0);

    EnableLines();
}

IMPL_LINK_NOARG(DlgFilterCrit, ListSelectCompHdl, weld::ComboBox&, void)
{
    EnableLines();
}

void DlgFilterCrit::BuildWherePart()
{
    Sequence<Sequence<PropertyValue> > aFilter,aHaving;
    aFilter.realloc(1);
    aHaving.realloc(1);

    if( LbPos(*m_xLB_WHEREFIELD1) != 0 )
    {
        PropertyValue aValue;
        if ( getCondition(*m_xLB_WHEREFIELD1,*m_xLB_WHERECOMP1,*m_xET_WHEREVALUE1,aValue) )
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

    if( LbPos(*m_xLB_WHEREFIELD2) != 0 )
    {
        PropertyValue aValue;
        Sequence<Sequence<PropertyValue> >& _rValues = aFilter;
        if ( getCondition(*m_xLB_WHEREFIELD2,*m_xLB_WHERECOMP2,*m_xET_WHEREVALUE2,aValue) )
            _rValues = aHaving;
        PropertyValue* pPos = nullptr;
        if ( m_xLB_WHERECOND2->get_active() )
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

    if( LbPos(*m_xLB_WHEREFIELD3) != 0 )
    {
        PropertyValue aValue;
        Sequence<Sequence<PropertyValue> >& _rValues = aFilter;
        if ( getCondition(*m_xLB_WHEREFIELD3,*m_xLB_WHERECOMP3,*m_xET_WHEREVALUE3,aValue) )
            _rValues = aHaving;
        PropertyValue* pPos = nullptr;
        if (m_xLB_WHERECOND3->get_active())
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
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
}

void DlgFilterCrit::fillLines(int &i, const Sequence< Sequence< PropertyValue > >& _aValues)
{
    const Sequence<PropertyValue >* pOrIter = _aValues.getConstArray();
    const Sequence<PropertyValue >* pOrEnd   = pOrIter + _aValues.getLength();
    bool bOr(i != 0); // WHERE clause and HAVING clause are always ANDed, nor ORed
    for(; pOrIter != pOrEnd; ++pOrIter)
    {
        const PropertyValue* pAndIter   = pOrIter->getConstArray();
        const PropertyValue* pAndEnd    = pAndIter + pOrIter->getLength();
        for(;pAndIter != pAndEnd; ++pAndIter)
        {
            SetLine( i++,*pAndIter,bOr);
            bOr = false;
        }
        bOr=true;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
