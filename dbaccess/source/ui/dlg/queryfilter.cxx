/*************************************************************************
 *
 *  $RCSfile: queryfilter.cxx,v $
 *
 *  $Revision: 1.23 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:50:45 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef DBAUI_QUERYFILTER_HXX
#include "queryfilter.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATE_HPP_
#include <com/sun/star/util/Date.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_TIME_HPP_
#include <com/sun/star/util/Time.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XSQLQUERYCOMPOSER_HPP_
#include <com/sun/star/sdb/XSQLQueryComposer.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNSEARCH_HPP_
#include <com/sun/star/sdbc/ColumnSearch.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XCOLUMNSSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_SQLFILTEROPERATOR_HPP_
#include <com/sun/star/sdb/SQLFilterOperator.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _CONNECTIVITY_PARSE_SQLITERATOR_HXX_
#include <connectivity/sqliterator.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef DBAUI_QUERYFILTER_HRC
#include "queryfilter.hrc"
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
//#ifndef _COMPHELPER_EXTRACT_HXX_
//#include <comphelper/extract.hxx>
//#endif
#ifndef _COM_SUN_STAR_SDB_XSINGLESELECTQUERYANALYZER_HPP_
#include <com/sun/star/sdb/XSingleSelectQueryAnalyzer.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XSINGLESELECTQUERYCOMPOSER_HPP_
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_SQLFILTEROPERATOR_HPP_
#include <com/sun/star/sdb/SQLFilterOperator.hpp>
#endif

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

//------------------------------------------------------------------------------
void Replace_OS_PlaceHolder(String& aString)
{
    while (aString.SearchAndReplace( '*', '%' ) != STRING_NOTFOUND);
    while (aString.SearchAndReplace( '?', '_' ) != STRING_NOTFOUND);
}

//------------------------------------------------------------------------------
void Replace_SQL_PlaceHolder(String& aString)
{
    while (aString.SearchAndReplace( '%', '*' ) != STRING_NOTFOUND);
    while (aString.SearchAndReplace( '_', '?' ) != STRING_NOTFOUND);
}

DBG_NAME(DlgFilterCrit);
//------------------------------------------------------------------------------
DlgFilterCrit::DlgFilterCrit(Window * pParent,
                             const Reference< XMultiServiceFactory >& _rxORB,
                             const Reference< XConnection>& _rxConnection,
                             const Reference< XSQLQueryComposer>& _rxQueryComposer,
                             const Reference< XNameAccess>& _rxCols
                             )
    :ModalDialog( pParent, ModuleRes( DLG_FILTERCRIT ) )
    ,aLB_WHEREFIELD1    ( this, ResId( LB_WHEREFIELD1 ) )
    ,aLB_WHERECOMP1     ( this, ResId( LB_WHERECOMP1 ) )
    ,aET_WHEREVALUE1    ( this, ResId( ET_WHEREVALUE1 ) )
    ,aLB_WHERECOND2     ( this, ResId( LB_WHERECOND2 ) )
    ,aLB_WHEREFIELD2    ( this, ResId( LB_WHEREFIELD2 ) )
    ,aLB_WHERECOMP2     ( this, ResId( LB_WHERECOMP2 ) )
    ,aET_WHEREVALUE2    ( this, ResId( ET_WHEREVALUE2 ) )
    ,aLB_WHERECOND3     ( this, ResId( LB_WHERECOND3 ) )
    ,aLB_WHEREFIELD3    ( this, ResId( LB_WHEREFIELD3 ) )
    ,aLB_WHERECOMP3     ( this, ResId( LB_WHERECOMP3 ) )
    ,aET_WHEREVALUE3    ( this, ResId( ET_WHEREVALUE3 ) )
    ,aFT_WHEREFIELD     ( this, ResId( FT_WHEREFIELD ) )
    ,aFT_WHERECOMP      ( this, ResId( FT_WHERECOMP ) )
    ,aFT_WHEREVALUE     ( this, ResId( FT_WHEREVALUE ) )
    ,aFT_WHEREOPER      ( this, ResId( FT_WHEREOPER ) )
    ,aFL_FIELDS         ( this, ResId( FL_FIELDS ) )
    ,aBT_OK             ( this, ResId( BT_OK ) )
    ,aBT_CANCEL         ( this, ResId( BT_CANCEL ) )
    ,aBT_HELP           ( this, ResId( BT_HELP ) )
    ,aSTR_NOENTRY       ( ResId( STR_NOENTRY ) )
    ,aSTR_COMPARE_OPERATORS( ResId( STR_COMPARE_OPERATORS ) )
    ,m_xQueryComposer( _rxQueryComposer )
    ,m_xColumns( _rxCols )
    ,m_xConnection( _rxConnection )
    ,m_xMetaData( _rxConnection->getMetaData() )
    ,m_aPredicateInput( _rxORB, _rxConnection, getParseContext() )
{
    DBG_CTOR(DlgFilterCrit,NULL);
    // Den String fuer noEntry in die ListBoxen der Feldnamen schreiben
    aLB_WHEREFIELD1.InsertEntry( aSTR_NOENTRY );
    aLB_WHEREFIELD2.InsertEntry( aSTR_NOENTRY );
    aLB_WHEREFIELD3.InsertEntry( aSTR_NOENTRY );

    try
    {
        // ... sowie auch die restlichen Felder
        Sequence< ::rtl::OUString> aNames = m_xColumns->getElementNames();
        const ::rtl::OUString* pBegin = aNames.getConstArray();
        const ::rtl::OUString* pEnd   = pBegin + aNames.getLength();
        Reference<XPropertySet> xColumn;
        for(;pBegin != pEnd;++pBegin)
        {
            if (m_xColumns->hasByName(*pBegin))
            {
                xColumn.set(m_xColumns->getByName(*pBegin),UNO_QUERY);
                OSL_ENSURE(xColumn.is(),"DlgFilterCrit::DlgFilterCrit: Column is null!");
            }
            else
                OSL_ENSURE(sal_False, "DlgFilterCrit::DlgFilterCrit: invalid column name!");
            sal_Int32 nDataType(0);
            xColumn->getPropertyValue(PROPERTY_TYPE) >>= nDataType;
            sal_Int32 eColumnSearch = dbtools::getSearchColumnFlag(m_xConnection,nDataType);
            // TODO
            // !pColumn->IsFunction()
            if(eColumnSearch != ColumnSearch::NONE)
            {
                aLB_WHEREFIELD1.InsertEntry( *pBegin );
                aLB_WHEREFIELD2.InsertEntry( *pBegin );
                aLB_WHEREFIELD3.InsertEntry( *pBegin );
            }
        }
        // initialize the listboxes with noEntry
        aLB_WHEREFIELD1.SelectEntryPos(0);
        aLB_WHEREFIELD2.SelectEntryPos(0);
        aLB_WHEREFIELD3.SelectEntryPos(0);

        // insert the criteria into the dialog
        Sequence<Sequence<PropertyValue > > aValues = m_xQueryComposer->getStructuredFilter();
        const Sequence<PropertyValue >* pOrBegin = aValues.getConstArray();
        const Sequence<PropertyValue >* pOrEnd   = pOrBegin + aValues.getLength();
        sal_Bool bOr = sal_True;
        for(sal_uInt16 i=0;pOrBegin != pOrEnd; ++pOrBegin)
        {
            bOr = sal_True;
            const PropertyValue* pAndBegin  = pOrBegin->getConstArray();
            const PropertyValue* pAndEnd    = pAndBegin + pOrBegin->getLength();
            for(;pAndBegin != pAndEnd; ++pAndBegin)
            {
                SetLine( i++,*pAndBegin,bOr);
                bOr = sal_False;
            }
        }
    }
    catch(Exception&)
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
//------------------------------------------------------------------------------
DlgFilterCrit::~DlgFilterCrit()
{
    DBG_DTOR(DlgFilterCrit,NULL);
}

#define LbText(x)       ((x).GetSelectEntry())
#define LbPos(x)        ((x).GetSelectEntryPos())

//------------------------------------------------------------------------------
sal_Int32 DlgFilterCrit::GetOSQLPredicateType(sal_uInt16 nPos,sal_uInt16 nCount) const
{
    sal_Int32 ePreType;

    if(nCount == 10)
    {
        switch(nPos)
        {
        case 0:
            ePreType = SQLFilterOperator::EQUAL;
            break;
        case 1:
            ePreType = SQLFilterOperator::NOT_EQUAL;
            break;
        case 2:
            ePreType = SQLFilterOperator::LESS;
            break;
        case 3:
            ePreType = SQLFilterOperator::LESS_EQUAL;
            break;
        case 4:
            ePreType = SQLFilterOperator::GREATER;
            break;
        case 5:
            ePreType = SQLFilterOperator::GREATER_EQUAL;
            break;
        case 6:
            ePreType = SQLFilterOperator::LIKE;
            break;
        case 7:
            ePreType = SQLFilterOperator::NOT_LIKE;
            break;
        case 8:
            ePreType = SQLFilterOperator::SQLNULL;
            break;
        case 9:
            ePreType = SQLFilterOperator::NOT_SQLNULL;
            break;
        }
    }
    else if(nCount == 8)
    {
        switch(nPos)
        {
        case 0:
            ePreType = SQLFilterOperator::EQUAL;
            break;
        case 1:
            ePreType = SQLFilterOperator::NOT_EQUAL;
            break;
        case 2:
            ePreType = SQLFilterOperator::LESS;
            break;
        case 3:
            ePreType = SQLFilterOperator::LESS_EQUAL;
            break;
        case 4:
            ePreType = SQLFilterOperator::GREATER;
            break;
        case 5:
            ePreType = SQLFilterOperator::GREATER_EQUAL;
            break;
        case 6:
            ePreType = SQLFilterOperator::SQLNULL;
            break;
        case 7:
            ePreType = SQLFilterOperator::NOT_SQLNULL;
            break;
        }
    }
    else
    {
        switch(nPos)
        {
        case 0:
            ePreType = SQLFilterOperator::LIKE;
            break;
        case 1:
            ePreType = SQLFilterOperator::NOT_LIKE;
            break;
        }
    }
    return ePreType;
}
//------------------------------------------------------------------------------
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
    }
    return nPos;
}
// -----------------------------------------------------------------------------
void DlgFilterCrit::getCondition(const ListBox& _rField,const ListBox& _rComp,const Edit& _rValue,PropertyValue& _rFilter) const
{
    try
    {
        _rFilter.Name = _rField.GetSelectEntry();
        Reference< XPropertySet > xColumn = getQueryColumn(_rFilter.Name);
        if ( xColumn.is() && xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_REALNAME) )
        {
            xColumn->getPropertyValue(PROPERTY_REALNAME)    >>= _rFilter.Name;
        }
    }
    catch(Exception)
    {
    }

    _rFilter.Handle = GetOSQLPredicateType(_rComp.GetSelectEntryPos(),_rComp.GetEntryCount());
    if ( SQLFilterOperator::SQLNULL != _rFilter.Handle && _rFilter.Handle != SQLFilterOperator::NOT_SQLNULL )
    {
        String sPredicateValue = m_aPredicateInput.getPredicateValue( _rValue.GetText(), getMatchingColumn( _rValue ), sal_True );
        ::Replace_OS_PlaceHolder( sPredicateValue );
        _rFilter.Value <<= ::rtl::OUString(sPredicateValue);
    }
/*
    ::rtl::OUString aFilter(_rField.GetSelectEntry());

    Reference< XPropertySet > xColumn = getQueryColumn(aFilter);
    ::rtl::OUString aQuote  = m_xMetaData.is() ? m_xMetaData->getIdentifierQuoteString() : ::rtl::OUString();
    if ( xColumn.is() )
    {
        ::rtl::OUString sRealName,sTableName;
        xColumn->getPropertyValue(PROPERTY_REALNAME)    >>= sRealName;
        xColumn->getPropertyValue(PROPERTY_TABLENAME)   >>= sTableName;
        if(sTableName.indexOf('.',0) != -1)
        {
            ::rtl::OUString aCatlog,aSchema,aTable;
            ::dbtools::qualifiedNameComponents(m_xMetaData,sTableName,aCatlog,aSchema,aTable,::dbtools::eInDataManipulation);
            ::dbtools::composeTableName(m_xMetaData,aCatlog,aSchema,aTable,sTableName,sal_True,::dbtools::eInDataManipulation);
        }
        else
            sTableName = ::dbtools::quoteName(aQuote,sTableName);

        aFilter =  sTableName;
        aFilter += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("."));
        aFilter += ::dbtools::quoteName(aQuote,sRealName);
    }
    else
        aFilter = ::dbtools::quoteName(aQuote,aFilter);

    aFilter += ::rtl::OUString::createFromAscii(" ");
    sal_Bool bNeedText = sal_True;
    switch(GetOSQLPredicateType(_rComp.GetSelectEntryPos(),_rComp.GetEntryCount()))
    {
        case SQLFilterOperator::EQUAL:
            aFilter += ::rtl::OUString::createFromAscii("=");
            break;
        case SQLFilterOperator::NOT_EQUAL:
            aFilter += ::rtl::OUString::createFromAscii("<>");
            break;
        case SQLFilterOperator::LESS:
            aFilter += ::rtl::OUString::createFromAscii("<");
            break;
        case SQLFilterOperator::LESS_EQUAL:
            aFilter += ::rtl::OUString::createFromAscii("<=");
            break;
        case SQLFilterOperator::GREATER:
            aFilter += ::rtl::OUString::createFromAscii(">");
            break;
        case SQLFilterOperator::GREATER_EQUAL:
            aFilter += ::rtl::OUString::createFromAscii(">=");
            break;
        case SQLFilterOperator::NOT_LIKE:
            aFilter += ::rtl::OUString::createFromAscii("NOT LIKE");
            break;
        case SQLFilterOperator::LIKE:
            aFilter += ::rtl::OUString::createFromAscii("LIKE");
            break;
        case SQLFilterOperator::SQLNULL:
            aFilter += ::rtl::OUString::createFromAscii("IS NULL");
            bNeedText = sal_False;
            break;
        case SQLFilterOperator::NOT_SQLNULL:
            aFilter += ::rtl::OUString::createFromAscii("IS NOT NULL");
            bNeedText = sal_False;
            break;
    }
    if(bNeedText)
    {
        aFilter += ::rtl::OUString::createFromAscii(" ");
        String sPredicateValue = m_aPredicateInput.getPredicateValue( _rValue.GetText(), getMatchingColumn( _rValue ), sal_True );
        ::Replace_OS_PlaceHolder( sPredicateValue );
        aFilter += sPredicateValue;
    }
    return aFilter;
    */
}

//------------------------------------------------------------------------------
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
    catch( const Exception& e )
    {
        e;  // make compiler happy
        DBG_ERROR( "DlgFilterCrit::getMatchingColumn: caught an exception!" );
    }

    return xColumn;
}
//------------------------------------------------------------------------------
Reference< XPropertySet > DlgFilterCrit::getQueryColumn( const ::rtl::OUString& _rFieldName ) const
{
    Reference< XPropertySet > xColumn;
    try
    {
        Reference< XNameAccess> xColumns = Reference< XColumnsSupplier >(m_xQueryComposer,UNO_QUERY)->getColumns();
        if ( xColumns.is() && xColumns->hasByName( _rFieldName ) )
            xColumns->getByName( _rFieldName ) >>= xColumn;
    }
    catch( const Exception& e )
    {
        e;  // make compiler happy
        DBG_ERROR( "DlgFilterCrit::getMatchingColumn: caught an exception!" );
    }

    return xColumn;
}

//------------------------------------------------------------------------------
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
    else
        DBG_ERROR( "DlgFilterCrit::getMatchingColumn: invalid event source!" );

    // the field itself
    return getColumn( sField );
}

//------------------------------------------------------------------------------
IMPL_LINK( DlgFilterCrit, PredicateLoseFocus, Edit*, _pField )
{
    DBG_ASSERT( _pField, "DlgFilterCrit::PredicateLoseFocus: invalid event source!" );
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

//------------------------------------------------------------------------------
void DlgFilterCrit::GetFilterList() const
{
    Sequence<Sequence<PropertyValue> > aFilter;
    aFilter.realloc(1);
    //  ::rtl::OUString aFilter;

    if( LbPos(aLB_WHEREFIELD1) != 0 )
    {
        aFilter[0].realloc(1);
        getCondition(aLB_WHEREFIELD1,aLB_WHERECOMP1,aET_WHEREVALUE1,aFilter[0][0]);
    }

    if( LbPos(aLB_WHEREFIELD2) != 0 )
    {

        PropertyValue* pPos;
        if ( aLB_WHERECOND2.GetSelectEntryPos() )
        {
            sal_Int32 nPos = aFilter.getLength();
            aFilter.realloc( nPos + 1);
            aFilter[nPos].realloc( 1);
            pPos = &aFilter[nPos][0];
        }
        else
        {
            sal_Int32 nPos = aFilter.getLength() - 1;
            sal_Int32 nAndPos = aFilter[nPos].getLength();
            aFilter[nPos].realloc( aFilter[nPos].getLength() + 1);
            pPos = &aFilter[nPos][nAndPos];
        }
        getCondition(aLB_WHEREFIELD2,aLB_WHERECOMP2,aET_WHEREVALUE2,*pPos);
    }

    if( LbPos(aLB_WHEREFIELD3) != 0 )
    {
        PropertyValue* pPos;
        if ( aLB_WHERECOND3.GetSelectEntryPos() )
        {
            sal_Int32 nPos = aFilter.getLength();
            aFilter.realloc( nPos + 1);
            aFilter[nPos].realloc( 1);
            pPos = &aFilter[nPos][0];
        }
        else
        {
            sal_Int32 nPos = aFilter.getLength() - 1;
            sal_Int32 nAndPos = aFilter[nPos].getLength();
            aFilter[nPos].realloc( aFilter[nPos].getLength() + 1);
            pPos = &aFilter[nPos][nAndPos];
        }
        getCondition(aLB_WHEREFIELD3,aLB_WHERECOMP3,aET_WHEREVALUE3,*pPos);
    }
    try
    {
        Reference<XMultiServiceFactory> xFac(m_xConnection,UNO_QUERY);
        if ( xFac.is() )
        {
            Reference< XSingleSelectQueryAnalyzer> xAnalyzer( xFac->createInstance( SERVICE_NAME_SINGLESELECTQUERYCOMPOSER ),UNO_QUERY);
            Reference< XSingleSelectQueryComposer> xComposer(xAnalyzer,UNO_QUERY);
            if ( xComposer.is() )
            {
                xAnalyzer->setQuery(m_xQueryComposer->getQuery());
                xComposer->setStructuredFilter(aFilter);
                m_xQueryComposer->setFilter(xAnalyzer->getFilter());
            }
        }
    }
    catch(Exception)
    {
        OSL_ENSURE(0,"Could create filter!");
    }
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void DlgFilterCrit::SetLine( sal_uInt16 nIdx,const PropertyValue& _rItem,sal_Bool _bOr  )
{
    DBG_CHKTHIS(DlgFilterCrit,NULL);
    ::rtl::OUString aCondition;
    _rItem.Value >>= aCondition;
    String aStr = aCondition.getStr();
    ::Replace_SQL_PlaceHolder(aStr);
    aStr.EraseTrailingChars();

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
    aStr.EraseLeadingChars();

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

//------------------------------------------------------------------------------
void DlgFilterCrit::SelectField( ListBox& rBox, const String& rField )
{
    DBG_CHKTHIS(DlgFilterCrit,NULL);
    sal_uInt16 nCnt = rBox.GetEntryCount();
    //  sal_Bool bCase = m_rIterator.TablesAreSensitive();

    for( sal_uInt16 i=0 ; i<nCnt ; i++ )
    {
        //  if(bCase ? rBox.GetEntry(i) == rField : rBox.GetEntry(i).EqualsIgnoreCaseAscii(rField))
        if(rBox.GetEntry(i) == rField)
        {
            rBox.SelectEntryPos(i);
            return;
        }
    }

    rBox.SelectEntryPos(0);
}

//------------------------------------------------------------------------------
void DlgFilterCrit::EnableLines()
{
    DBG_CHKTHIS(DlgFilterCrit,NULL);
    // Enablen/Disablen ganzer Zeilen
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

    // Vergleichsfeld gleich NOENTRY
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

    // Vergleichsoperator gleich ISNULL oder ISNOTNULL
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

//------------------------------------------------------------------------------
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

    Reference<XPropertySet> xColumn;
    if (m_xColumns->hasByName(aName))
        xColumn.set(m_xColumns->getByName(aName),UNO_QUERY);
    if(xColumn.is())
    {
        sal_Int32 nDataType;
        xColumn->getPropertyValue(PROPERTY_TYPE) >>= nDataType;
        sal_Int32 eColumnSearch = dbtools::getSearchColumnFlag(m_xConnection,nDataType);

        if(eColumnSearch  == ColumnSearch::FULL)
        {
            for(xub_StrLen i=0;i<aSTR_COMPARE_OPERATORS.GetTokenCount();i++)
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
            for(i=8;i<aSTR_COMPARE_OPERATORS.GetTokenCount();i++)
                pComp->InsertEntry(aSTR_COMPARE_OPERATORS.GetToken(i));
        }
        else
        {
            DBG_ASSERT(0,"DlgFilterCrit::ListSelectHdl: Diese Column dürfte garnicht vorhanden sein!");
        }
    }
    pComp->SelectEntryPos(0);

    EnableLines();
    return 0;
}



//------------------------------------------------------------------------------
IMPL_LINK_INLINE_START( DlgFilterCrit, ListSelectCompHdl, ListBox *, pListBox )
{
    EnableLines();
    return 0;
}
IMPL_LINK_INLINE_END( DlgFilterCrit, ListSelectCompHdl, ListBox *, pListBox )
//------------------------------------------------------------------------------
String DlgFilterCrit::BuildWherePart()
{
    DBG_CHKTHIS(DlgFilterCrit,NULL);
    GetFilterList();

    return m_xQueryComposer->getFilter();
}
// -----------------------------------------------------------------------------
void DlgFilterCrit::addQuoting(const ::rtl::OUString& _rColumnName,String& _rCondition) const
{
    Reference<XPropertySet> xColumn;
    if ( m_xColumns->hasByName(_rColumnName) )
        xColumn.set(m_xColumns->getByName(_rColumnName),UNO_QUERY);

    if(!m_xMetaData.is() || !xColumn.is())
        return;
    sal_Int32 nType = 0;
    xColumn->getPropertyValue(PROPERTY_TYPE) >>= nType;
    Reference< XResultSet> xRs = m_xMetaData->getTypeInfo();
    Reference< XRow> xRow(xRs,UNO_QUERY);
    // Information for a single SQL type
    String aCondition = _rCondition;
    // Loop on the result set
    if ( xRs.is() )
        while (xRs->next())
        {
            if(nType == xRow->getShort(2))
            {
                ::rtl::OUString sPrefix = xRow->getString(4);
                if ( !xRow->wasNull() )
                {
                    aCondition = String(sPrefix);
                    aCondition += _rCondition;
                }

                ::rtl::OUString sSuffix = xRow->getString(5);
                if ( !xRow->wasNull() )
                    aCondition += String(sSuffix);
                break;
            }
        }
    _rCondition = aCondition;
}
// -----------------------------------------------------------------------------



