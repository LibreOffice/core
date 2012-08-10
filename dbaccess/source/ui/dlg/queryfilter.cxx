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
#include "queryfilter.hrc"
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
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::beans;

void Replace_OS_PlaceHolder(String& aString)
{
    while (aString.SearchAndReplace( '*', '%' ) != STRING_NOTFOUND) ;
    while (aString.SearchAndReplace( '?', '_' ) != STRING_NOTFOUND) ;
}

void Replace_SQL_PlaceHolder(String& aString)
{
    while (aString.SearchAndReplace( '%', '*' ) != STRING_NOTFOUND) ;
    while (aString.SearchAndReplace( '_', '?' ) != STRING_NOTFOUND) ;
}

DBG_NAME(DlgFilterCrit);

DlgFilterCrit::DlgFilterCrit(Window * pParent,
                             const Reference< XMultiServiceFactory >& _rxORB,
                             const Reference< XConnection>& _rxConnection,
                             const Reference< XSingleSelectQueryComposer >& _rxComposer,
                             const Reference< XNameAccess>& _rxCols
                             )
    :ModalDialog( pParent, ModuleRes( DLG_FILTERCRIT ) )
    ,aLB_WHEREFIELD1    ( this, ModuleRes( LB_WHEREFIELD1 ) )
    ,aLB_WHERECOMP1     ( this, ModuleRes( LB_WHERECOMP1 ) )
    ,aET_WHEREVALUE1    ( this, ModuleRes( ET_WHEREVALUE1 ) )
    ,aLB_WHERECOND2     ( this, ModuleRes( LB_WHERECOND2 ) )
    ,aLB_WHEREFIELD2    ( this, ModuleRes( LB_WHEREFIELD2 ) )
    ,aLB_WHERECOMP2     ( this, ModuleRes( LB_WHERECOMP2 ) )
    ,aET_WHEREVALUE2    ( this, ModuleRes( ET_WHEREVALUE2 ) )
    ,aLB_WHERECOND3     ( this, ModuleRes( LB_WHERECOND3 ) )
    ,aLB_WHEREFIELD3    ( this, ModuleRes( LB_WHEREFIELD3 ) )
    ,aLB_WHERECOMP3     ( this, ModuleRes( LB_WHERECOMP3 ) )
    ,aET_WHEREVALUE3    ( this, ModuleRes( ET_WHEREVALUE3 ) )
    ,aFT_WHEREFIELD     ( this, ModuleRes( FT_WHEREFIELD ) )
    ,aFT_WHERECOMP      ( this, ModuleRes( FT_WHERECOMP ) )
    ,aFT_WHEREVALUE     ( this, ModuleRes( FT_WHEREVALUE ) )
    ,aFT_WHEREOPER      ( this, ModuleRes( FT_WHEREOPER ) )
    ,aFL_FIELDS         ( this, ModuleRes( FL_FIELDS ) )
    ,aBT_OK             ( this, ModuleRes( BT_OK ) )
    ,aBT_CANCEL         ( this, ModuleRes( BT_CANCEL ) )
    ,aBT_HELP           ( this, ModuleRes( BT_HELP ) )
    ,aSTR_NOENTRY       ( ModuleRes( STR_NOENTRY ) )
    ,aSTR_COMPARE_OPERATORS( ModuleRes( STR_COMPARE_OPERATORS ) )
    ,m_xQueryComposer(_rxComposer)
    ,m_xColumns( _rxCols )
    ,m_xConnection( _rxConnection )
    ,m_xMetaData( _rxConnection->getMetaData() )
    ,m_aPredicateInput( _rxORB, _rxConnection, getParseContext() )
{
    DBG_CTOR(DlgFilterCrit,NULL);
    // Write the String for noEntry into the ListBoxes of the field names
    aLB_WHEREFIELD1.InsertEntry( aSTR_NOENTRY );
    aLB_WHEREFIELD2.InsertEntry( aSTR_NOENTRY );
    aLB_WHEREFIELD3.InsertEntry( aSTR_NOENTRY );

    try
    {
        // ... also write it into the remaining fields
        Sequence< ::rtl::OUString> aNames = m_xColumns->getElementNames();
        const ::rtl::OUString* pIter = aNames.getConstArray();
        const ::rtl::OUString* pEnd   = pIter + aNames.getLength();
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

                sal_Bool bIsSearchable( sal_True );
                OSL_VERIFY( xColumn->getPropertyValue( PROPERTY_ISSEARCHABLE ) >>= bIsSearchable );
                if ( !bIsSearchable )
                    continue;
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
            aLB_WHEREFIELD1.InsertEntry( *pIter );
            aLB_WHEREFIELD2.InsertEntry( *pIter );
            aLB_WHEREFIELD3.InsertEntry( *pIter );
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
                    aLB_WHEREFIELD1.InsertEntry( *pIter );
                    aLB_WHEREFIELD2.InsertEntry( *pIter );
                    aLB_WHEREFIELD3.InsertEntry( *pIter );
                }
            }
        }
        // initialize the listboxes with noEntry
        aLB_WHEREFIELD1.SelectEntryPos(0);
        aLB_WHEREFIELD2.SelectEntryPos(0);
        aLB_WHEREFIELD3.SelectEntryPos(0);

        // insert the criteria into the dialog
        Sequence<Sequence<PropertyValue > > aValues = m_xQueryComposer->getStructuredFilter();
        fillLines(aValues);
        aValues = m_xQueryComposer->getStructuredHavingClause();
        fillLines(aValues);

    }
    catch(const Exception&)
    {
        FreeResource();
        throw;
    }

    EnableLines();

    aLB_WHEREFIELD1.SetSelectHdl(LINK(this,DlgFilterCrit,ListSelectHdl));
    aLB_WHEREFIELD2.SetSelectHdl(LINK(this,DlgFilterCrit,ListSelectHdl));
    aLB_WHEREFIELD3.SetSelectHdl(LINK(this,DlgFilterCrit,ListSelectHdl));

    aLB_WHERECOMP1.SetSelectHdl(LINK(this,DlgFilterCrit,ListSelectCompHdl));
    aLB_WHERECOMP2.SetSelectHdl(LINK(this,DlgFilterCrit,ListSelectCompHdl));
    aLB_WHERECOMP3.SetSelectHdl(LINK(this,DlgFilterCrit,ListSelectCompHdl));

    aET_WHEREVALUE1.SetLoseFocusHdl( LINK( this, DlgFilterCrit, PredicateLoseFocus ) );
    aET_WHEREVALUE2.SetLoseFocusHdl( LINK( this, DlgFilterCrit, PredicateLoseFocus ) );
    aET_WHEREVALUE3.SetLoseFocusHdl( LINK( this, DlgFilterCrit, PredicateLoseFocus ) );

    if ( aET_WHEREVALUE1.IsEnabled() )
        aET_WHEREVALUE1.GrabFocus();

    FreeResource();
}

DlgFilterCrit::~DlgFilterCrit()
{
    DBG_DTOR(DlgFilterCrit,NULL);
}

#define LbText(x)       ((x).GetSelectEntry())
#define LbPos(x)        ((x).GetSelectEntryPos())

sal_Int32 DlgFilterCrit::GetOSQLPredicateType( const String& _rSelectedPredicate ) const
{
    sal_Int32 nPredicateIndex = -1;
    for ( xub_StrLen i=0; i < comphelper::string::getTokenCount(aSTR_COMPARE_OPERATORS, ';'); ++i)
        if ( aSTR_COMPARE_OPERATORS.GetToken(i) == _rSelectedPredicate )
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

sal_uInt16 DlgFilterCrit::GetSelectionPos(sal_Int32 eType,const ListBox& rListBox) const
{
    sal_uInt16 nPos;
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

sal_Bool DlgFilterCrit::getCondition(const ListBox& _rField,const ListBox& _rComp,const Edit& _rValue,PropertyValue& _rFilter) const
{
    sal_Bool bHaving = sal_False;
    try
    {
        ::rtl::OUString sTableName;
        sal_Bool bFunction = sal_False;
        _rFilter.Name = _rField.GetSelectEntry();
        Reference< XPropertySet > xColumn = getQueryColumn(_rFilter.Name);
        if ( xColumn.is() )
        {
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
                        ::rtl::OUString aCatlog,aSchema,aTable;
                        ::dbtools::qualifiedNameComponents( m_xMetaData, sTableName, aCatlog, aSchema, aTable, ::dbtools::eInDataManipulation );
                        sTableName = ::dbtools::composeTableName( m_xMetaData, aCatlog, aSchema, aTable, sal_True, ::dbtools::eInDataManipulation );
                    }
                }
                xColumn->getPropertyValue(PROPERTY_REALNAME)    >>= _rFilter.Name;
                static ::rtl::OUString sAgg(RTL_CONSTASCII_USTRINGPARAM("AggregateFunction"));
                if ( xInfo->hasPropertyByName(sAgg) )
                    xColumn->getPropertyValue(sAgg) >>= bHaving;
                static ::rtl::OUString sFunction(RTL_CONSTASCII_USTRINGPARAM("Function"));
                if ( xInfo->hasPropertyByName(sFunction) )
                    xColumn->getPropertyValue(sFunction) >>= bFunction;
            }
            if ( !bFunction )
            {
                const ::rtl::OUString aQuote    = m_xMetaData.is() ? m_xMetaData->getIdentifierQuoteString() : ::rtl::OUString();
                _rFilter.Name = ::dbtools::quoteName(aQuote,_rFilter.Name);
                if ( !sTableName.isEmpty() )
                {
                    static ::rtl::OUString sSep(RTL_CONSTASCII_USTRINGPARAM("."));
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
        String sPredicateValue = m_aPredicateInput.getPredicateValue( _rValue.GetText(), getMatchingColumn( _rValue ), sal_True );
        ::Replace_OS_PlaceHolder( sPredicateValue );
        _rFilter.Value <<= ::rtl::OUString(sPredicateValue);
    }
    return bHaving;
}

Reference< XPropertySet > DlgFilterCrit::getColumn( const ::rtl::OUString& _rFieldName ) const
{
    Reference< XPropertySet > xColumn;
    try
    {
        if ( m_xColumns.is() && m_xColumns->hasByName( _rFieldName ) )
            m_xColumns->getByName( _rFieldName ) >>= xColumn;

        Reference< XNameAccess> xColumns = Reference< XColumnsSupplier >(m_xQueryComposer,UNO_QUERY)->getColumns();
        if ( xColumns.is() && !xColumn.is() )
        {
            Sequence< ::rtl::OUString> aSeq = xColumns->getElementNames();
            const ::rtl::OUString* pIter = aSeq.getConstArray();
            const ::rtl::OUString* pEnd   = pIter + aSeq.getLength();
            for(;pIter != pEnd;++pIter)
            {
                Reference<XPropertySet> xProp(xColumns->getByName(*pIter),UNO_QUERY);
                if ( xProp.is() && xProp->getPropertySetInfo()->hasPropertyByName(PROPERTY_REALNAME) )
                {
                    ::rtl::OUString sRealName;
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

Reference< XPropertySet > DlgFilterCrit::getQueryColumn( const ::rtl::OUString& _rFieldName ) const
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
    ::rtl::OUString sField;
    if ( &_rValueInput == &aET_WHEREVALUE1 )
    {
        sField = aLB_WHEREFIELD1.GetSelectEntry();
    }
    else if ( &_rValueInput == &aET_WHEREVALUE2 )
    {
        sField = aLB_WHEREFIELD2.GetSelectEntry();
    }
    else if ( &_rValueInput == &aET_WHEREVALUE3 )
    {
        sField = aLB_WHEREFIELD3.GetSelectEntry();
    }
    else {
        OSL_FAIL( "DlgFilterCrit::getMatchingColumn: invalid event source!" );
    }

    // the field itself
    return getColumn( sField );
}

IMPL_LINK( DlgFilterCrit, PredicateLoseFocus, Edit*, _pField )
{
    OSL_ENSURE( _pField, "DlgFilterCrit::PredicateLoseFocus: invalid event source!" );
    if ( _pField )
    {
        // retrieve the field affected
        Reference< XPropertySet> xColumn( getMatchingColumn( *_pField ) );
        // and normalize it's content
        if ( xColumn.is() )
        {
            ::rtl::OUString sText( _pField->GetText() );
            m_aPredicateInput.normalizePredicateString( sText, xColumn );
            _pField->SetText( sText );
        }
    }

    return 0L;
}

void DlgFilterCrit::SetLine( sal_uInt16 nIdx,const PropertyValue& _rItem,sal_Bool _bOr  )
{
    DBG_CHKTHIS(DlgFilterCrit,NULL);
    ::rtl::OUString aCondition;
    _rItem.Value >>= aCondition;
    String aStr = aCondition;
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
            aStr.Erase(0,2);
            break;
        case SQLFilterOperator::LESS:
            aStr.Erase(0,1);
            break;
        case SQLFilterOperator::LESS_EQUAL:
            aStr.Erase(0,2);
            break;
        case SQLFilterOperator::GREATER:
            aStr.Erase(0,1);
            break;
        case SQLFilterOperator::GREATER_EQUAL:
            aStr.Erase(0,2);
            break;
        case SQLFilterOperator::NOT_LIKE:
            aStr.Erase(0,8);
            break;
        case SQLFilterOperator::LIKE:
            aStr.Erase(0,4);
            break;
        case SQLFilterOperator::SQLNULL:
            aStr.Erase(0,7);
            break;
        case SQLFilterOperator::NOT_SQLNULL:
            aStr.Erase(0,11);
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
            pColumnListControl = &aLB_WHEREFIELD1;
            pPredicateListControl = &aLB_WHERECOMP1;
            pPredicateValueControl = &aET_WHEREVALUE1;
            break;
        case 1:
            aLB_WHERECOND2.SelectEntryPos( _bOr ? 1 : 0 );

            pColumnListControl = &aLB_WHEREFIELD2;
            pPredicateListControl = &aLB_WHERECOMP2;
            pPredicateValueControl = &aET_WHEREVALUE2;
            break;
        case 2:
            aLB_WHERECOND3.SelectEntryPos( _bOr ? 1 : 0 );

            pColumnListControl = &aLB_WHEREFIELD3;
            pPredicateListControl = &aLB_WHERECOMP3;
            pPredicateValueControl = &aET_WHEREVALUE3;
            break;
    }

    if ( pColumnListControl && pPredicateListControl && pPredicateValueControl )
    {
        ::rtl::OUString sName;
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
        ::rtl::OUString aString( aStr );
        m_aPredicateInput.normalizePredicateString( aString, xColumn );
        pPredicateValueControl->SetText( aString );
    }
}

void DlgFilterCrit::SelectField( ListBox& rBox, const String& rField )
{
    DBG_CHKTHIS(DlgFilterCrit,NULL);
    sal_uInt16 nCnt = rBox.GetEntryCount();

    for( sal_uInt16 i=0 ; i<nCnt ; i++ )
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
    DBG_CHKTHIS(DlgFilterCrit,NULL);
    // enabling/disabling of whole lines
    if( LbPos(aLB_WHEREFIELD1) == 0 )
    {
        aLB_WHEREFIELD2.Disable();
        aLB_WHERECOND2.Disable();
        aLB_WHERECOMP2.Disable();
        aET_WHEREVALUE2.Disable();

        aLB_WHEREFIELD3.Disable();
        aLB_WHERECOND3.Disable();
        aLB_WHERECOMP3.Disable();
        aET_WHEREVALUE3.Disable();
    }
    else
    {
        aLB_WHEREFIELD2.Enable();
        aLB_WHERECOND2.Enable();
        aLB_WHERECOMP2.Enable();
        aET_WHEREVALUE2.Enable();

        aLB_WHEREFIELD3.Enable();
        aLB_WHERECOND3.Enable();
        aLB_WHERECOMP3.Enable();
        aET_WHEREVALUE3.Enable();
    }

    if( LbPos(aLB_WHEREFIELD2) == 0 )
    {
        aLB_WHEREFIELD3.Disable();
        aLB_WHERECOND3.Disable();
        aLB_WHERECOMP3.Disable();
        aET_WHEREVALUE3.Disable();
    }
    else
    {
        aLB_WHEREFIELD3.Enable();
        aLB_WHERECOND3.Enable();
        aLB_WHERECOMP3.Enable();
        aET_WHEREVALUE3.Enable();
    }

    // comparison field equal to NOENTRY
    if( LbPos(aLB_WHEREFIELD1) == 0 )
    {
        aLB_WHERECOMP1.Disable();
        aET_WHEREVALUE1.Disable();
    }
    else
    {
        aLB_WHEREFIELD1.Enable();
        aLB_WHERECOMP1.Enable();
        aET_WHEREVALUE1.Enable();
    }

    if( LbPos(aLB_WHEREFIELD2) == 0 )
    {
        aLB_WHERECOND2.Disable();
        aLB_WHERECOMP2.Disable();
        aET_WHEREVALUE2.Disable();
    }
    else
    {
        aLB_WHERECOND2.Enable();
        aLB_WHEREFIELD2.Enable();
        aLB_WHERECOMP2.Enable();
        aET_WHEREVALUE2.Enable();
    }

    if( LbPos(aLB_WHEREFIELD3) == 0 )
    {
        aLB_WHERECOND3.Disable();
        aLB_WHERECOMP3.Disable();
        aET_WHEREVALUE3.Disable();
    }
    else
    {
        aLB_WHERECOND3.Enable();
        aLB_WHERECOND3.Enable();
        aLB_WHEREFIELD3.Enable();
        aLB_WHERECOMP3.Enable();
        aET_WHEREVALUE3.Enable();
    }

    // comparison operator equal to ISNULL or ISNOTNULL
    if(aLB_WHERECOMP1.GetEntryCount() > 2 &&
        ((LbPos(aLB_WHERECOMP1) == aLB_WHERECOMP1.GetEntryCount()-1) ||
         (LbPos(aLB_WHERECOMP1) == aLB_WHERECOMP1.GetEntryCount()-2)) )
        aET_WHEREVALUE1.Disable();

    if(aLB_WHERECOMP2.GetEntryCount() > 2 &&
        ((LbPos(aLB_WHERECOMP2) == aLB_WHERECOMP2.GetEntryCount()-1) ||
         (LbPos(aLB_WHERECOMP2) == aLB_WHERECOMP2.GetEntryCount()-2)) )
        aET_WHEREVALUE2.Disable();

    if(aLB_WHERECOMP3.GetEntryCount() > 2 &&
        ((LbPos(aLB_WHERECOMP3) == aLB_WHERECOMP3.GetEntryCount()-1) ||
         (LbPos(aLB_WHERECOMP3) == aLB_WHERECOMP3.GetEntryCount()-2)) )
        aET_WHEREVALUE3.Disable();
}

IMPL_LINK( DlgFilterCrit, ListSelectHdl, ListBox *, pListBox )
{
    String aName;
    ListBox* pComp;
    if(pListBox == &aLB_WHEREFIELD1)
    {
        aName = LbText(aLB_WHEREFIELD1);
        pComp = &aLB_WHERECOMP1;
    }
    else if(pListBox == &aLB_WHEREFIELD2)
    {
        aName = LbText(aLB_WHEREFIELD2);
        pComp = &aLB_WHERECOMP2;
    }
    else
    {
        aName = LbText(aLB_WHEREFIELD3);
        pComp = &aLB_WHERECOMP3;
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
            for(xub_StrLen i=0;i < comphelper::string::getTokenCount(aSTR_COMPARE_OPERATORS, ';');i++)
                pComp->InsertEntry(aSTR_COMPARE_OPERATORS.GetToken(i));
        }
        else if(eColumnSearch == ColumnSearch::CHAR)
        {
            for(xub_StrLen i=6;i<10;i++)
                pComp->InsertEntry(aSTR_COMPARE_OPERATORS.GetToken(i));
        }
        else if(eColumnSearch == ColumnSearch::BASIC)
        {
            xub_StrLen i;
            for( i = 0; i < 6; i++ )
                pComp->InsertEntry(aSTR_COMPARE_OPERATORS.GetToken(i));
            for(i=8; i < comphelper::string::getTokenCount(aSTR_COMPARE_OPERATORS, ';'); ++i)
                pComp->InsertEntry(aSTR_COMPARE_OPERATORS.GetToken(i));
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

IMPL_LINK_INLINE_START( DlgFilterCrit, ListSelectCompHdl, ListBox *, /*pListBox*/ )
{
    EnableLines();
    return 0;
}
IMPL_LINK_INLINE_END( DlgFilterCrit, ListSelectCompHdl, ListBox *, pListBox )

void DlgFilterCrit::BuildWherePart()
{
    DBG_CHKTHIS(DlgFilterCrit,NULL);
    Sequence<Sequence<PropertyValue> > aFilter,aHaving;
    aFilter.realloc(1);
    aHaving.realloc(1);

    if( LbPos(aLB_WHEREFIELD1) != 0 )
    {
        PropertyValue aValue;
        if ( getCondition(aLB_WHEREFIELD1,aLB_WHERECOMP1,aET_WHEREVALUE1,aValue) )
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

    if( LbPos(aLB_WHEREFIELD2) != 0 )
    {
        PropertyValue aValue;
        Sequence<Sequence<PropertyValue> >& _rValues = aFilter;
        if ( getCondition(aLB_WHEREFIELD2,aLB_WHERECOMP2,aET_WHEREVALUE2,aValue) )
            _rValues = aHaving;
        PropertyValue* pPos = NULL;
        if ( aLB_WHERECOND2.GetSelectEntryPos() )
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

    if( LbPos(aLB_WHEREFIELD3) != 0 )
    {
        PropertyValue aValue;
        Sequence<Sequence<PropertyValue> >& _rValues = aFilter;
        if ( getCondition(aLB_WHEREFIELD3,aLB_WHERECOMP3,aET_WHEREVALUE3,aValue) )
            _rValues = aHaving;
        PropertyValue* pPos = NULL;
        if ( aLB_WHERECOND3.GetSelectEntryPos() )
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
    sal_Bool bOr = sal_True;
    for(sal_uInt16 i=0;pOrIter != pOrEnd; ++pOrIter)
    {
        bOr = sal_True;
        const PropertyValue* pAndIter   = pOrIter->getConstArray();
        const PropertyValue* pAndEnd    = pAndIter + pOrIter->getLength();
        for(;pAndIter != pAndEnd; ++pAndIter)
        {
            SetLine( i++,*pAndIter,bOr);
            bOr = sal_False;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
