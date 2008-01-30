/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sqlnode.cxx,v $
 *
 *  $Revision: 1.52 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 08:04:40 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"


#ifndef _CONNECTIVITY_SQLNODE_HXX
#include <connectivity/sqlnode.hxx>
#endif
#ifndef CONNECTIVITY_SQLERROR_HXX
#include <connectivity/sqlerror.hxx>
#endif
#ifndef _CONNECTIVITY_SQLINTERNALNODE_HXX
#include <internalnode.hxx>
#endif
#define YYBISON   1
#ifndef BISON_INCLUDED
#define BISON_INCLUDED
#include <sqlbison.hxx>
#endif
#ifndef _CONNECTIVITY_SQLPARSE_HXX
#include <connectivity/sqlparse.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTER_HPP_
#include <com/sun/star/util/XNumberFormatter.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTYPES_HPP_
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_NUMBERFORMATINDEX_HPP_
#include <com/sun/star/i18n/NumberFormatIndex.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATABASEMETADATA_HPP_
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XQUERIESSUPPLIER_HPP_
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_ERRORCONDITION_HPP_
#include <com/sun/star/sdb/ErrorCondition.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTER_HPP_
#include <com/sun/star/util/XNumberFormatter.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATSSUPPLIER_HPP_
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATS_HPP_
#include <com/sun/star/util/XNumberFormats.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_NUMBERFORMAT_HPP_
#include <com/sun/star/util/NumberFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTYPES_HPP_
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_KPARSETYPE_HPP_
#include <com/sun/star/i18n/KParseType.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_KPARSETOKENS_HPP_
#include <com/sun/star/i18n/KParseTokens.hpp>
#endif

#ifndef _DBHELPER_DBCONVERSION_HXX_
#include "connectivity/dbconversion.hxx"
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_TIME_HPP_
#include <com/sun/star/util/Time.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATE_HPP_
#include <com/sun/star/util/Date.hpp>
#endif
#ifndef CONNECTIVITY_CONNECTION_HXX
#include "TConnection.hxx"
#endif
#ifndef _CONNECTIVITY_SQLSCAN_HXX
#include "sqlscan.hxx"
#endif
#ifndef _COMPHELPER_NUMBERS_HXX_
#include <comphelper/numbers.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include "connectivity/dbtools.hxx"
#endif
#ifndef CONNECTIVITY_INC_CONNECTIVITY_DBMETADATA_HXX
#include "connectivity/dbmetadata.hxx"
#endif
#ifndef CONNECTIVITY_SQLERROR_HXX
#include "connectivity/sqlerror.hxx"
#endif
#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
#endif


using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star;
using namespace ::osl;
using namespace ::dbtools;
using namespace ::comphelper;


extern int SQLyyparse (void);
extern ::rtl::OUString ConvertLikeToken(const ::connectivity::OSQLParseNode* pTokenNode, const ::connectivity::OSQLParseNode* pEscapeNode, sal_Bool bInternational);
extern void setParser( ::connectivity::OSQLParser* );

namespace
{
    // -----------------------------------------------------------------------------
    sal_Bool lcl_saveConvertToNumber(const Reference< XNumberFormatter > & _xFormatter,sal_Int32 _nKey,const ::rtl::OUString& _sValue,double& _nrValue)
    {
        sal_Bool bRet = sal_False;
        try
        {
            _nrValue = _xFormatter->convertStringToNumber(_nKey, _sValue);
            bRet = sal_True;
        }
        catch(Exception&)
        {
        }
        return bRet;
    }
    // -----------------------------------------------------------------------------
    void replaceAndReset(connectivity::OSQLParseNode*& _pResetNode,connectivity::OSQLParseNode* _pNewNode)
    {
        _pResetNode->getParent()->replace(_pResetNode, _pNewNode);
        delete _pResetNode;
        _pResetNode = _pNewNode;
    }
    // -----------------------------------------------------------------------------
    /** quotes a string and search for quotes inside the string and replace them with the new quote
        @param  rValue
            The value to be quoted.
        @param  rQuot
            The quote
        @param  rQuotToReplace
            The quote to replace with
        @return
            The quoted string.
    */
    ::rtl::OUString SetQuotation(const ::rtl::OUString& rValue, const ::rtl::OUString& rQuot, const ::rtl::OUString& rQuotToReplace)
    {
        ::rtl::OUString rNewValue = rQuot;
        rNewValue += rValue;
        sal_Int32 nIndex = (sal_Int32)-1;   // Quotes durch zweifache Quotes ersetzen, sonst kriegt der Parser Probleme

        if (rQuot.getLength())
        {
            do
            {
                nIndex += 2;
                nIndex = rNewValue.indexOf(rQuot,nIndex);
                if(nIndex != -1)
                    rNewValue = rNewValue.replaceAt(nIndex,rQuot.getLength(),rQuotToReplace);
            } while (nIndex != -1);
        }

        rNewValue += rQuot;
        return rNewValue;
    }
}

namespace connectivity
{

//=============================================================================
struct OSQLParser_Data
{
    ::com::sun::star::lang::Locale  aLocale;
    ::connectivity::SQLError        aErrors;

    OSQLParser_Data( const Reference< XMultiServiceFactory >& _xServiceFactory )
        :aErrors( _xServiceFactory )
    {
    }
};

//=============================================================================
//= SQLParseNodeParameter
//=============================================================================
//-----------------------------------------------------------------------------
SQLParseNodeParameter::SQLParseNodeParameter( const Reference< XConnection >& _rxConnection,
        const Reference< XNumberFormatter >& _xFormatter, const Reference< XPropertySet >& _xField,
        const Locale& _rLocale, const IParseContext* _pContext,
        bool _bIntl, bool _bQuote, sal_Char _cDecSep, bool _bPredicate, bool _bParseToSDBC )
    :rLocale(_rLocale)
    ,aMetaData( _rxConnection )
    ,pParser( NULL )
    ,pSubQueryHistory( new QueryNameSet )
    ,xFormatter(_xFormatter)
    ,xField(_xField)
    ,m_rContext( _pContext ? (const IParseContext&)(*_pContext) : (const IParseContext&)OSQLParser::s_aDefaultContext )
    ,cDecSep(_cDecSep)
    ,bQuote(_bQuote)
    ,bInternational(_bIntl)
    ,bPredicate(_bPredicate)
    ,bParseToSDBCLevel( _bParseToSDBC )
{
}

//-----------------------------------------------------------------------------
SQLParseNodeParameter::~SQLParseNodeParameter()
{
}

//=============================================================================
//= OSQLParseNode
//=============================================================================
//-----------------------------------------------------------------------------
::rtl::OUString OSQLParseNode::convertDateString(const SQLParseNodeParameter& rParam, const ::rtl::OUString& rString) const
{
    Date aDate = DBTypeConversion::toDate(rString);
    Reference< XNumberFormatsSupplier > xSupplier(rParam.xFormatter->getNumberFormatsSupplier());
    Reference< XNumberFormatTypes >     xTypes(xSupplier->getNumberFormats(), UNO_QUERY);

    double fDate = DBTypeConversion::toDouble(aDate,DBTypeConversion::getNULLDate(xSupplier));
    sal_Int32 nKey = xTypes->getStandardIndex(rParam.rLocale) + 36; // XXX hack
    return rParam.xFormatter->convertNumberToString(nKey, fDate);
}

//-----------------------------------------------------------------------------
::rtl::OUString OSQLParseNode::convertDateTimeString(const SQLParseNodeParameter& rParam, const ::rtl::OUString& rString) const
{
    DateTime aDate = DBTypeConversion::toDateTime(rString);
    Reference< XNumberFormatsSupplier >  xSupplier(rParam.xFormatter->getNumberFormatsSupplier());
    Reference< XNumberFormatTypes >  xTypes(xSupplier->getNumberFormats(), UNO_QUERY);

    double fDateTime = DBTypeConversion::toDouble(aDate,DBTypeConversion::getNULLDate(xSupplier));
    sal_Int32 nKey = xTypes->getStandardIndex(rParam.rLocale) + 51; // XXX hack
    return rParam.xFormatter->convertNumberToString(nKey, fDateTime);
}

//-----------------------------------------------------------------------------
::rtl::OUString OSQLParseNode::convertTimeString(const SQLParseNodeParameter& rParam, const ::rtl::OUString& rString) const
{
    Time aTime = DBTypeConversion::toTime(rString);
    Reference< XNumberFormatsSupplier >  xSupplier(rParam.xFormatter->getNumberFormatsSupplier());

    Reference< XNumberFormatTypes >  xTypes(xSupplier->getNumberFormats(), UNO_QUERY);

    double fTime = DBTypeConversion::toDouble(aTime);
    sal_Int32 nKey = xTypes->getStandardIndex(rParam.rLocale) + 41; // XXX hack
    return rParam.xFormatter->convertNumberToString(nKey, fTime);
}

//-----------------------------------------------------------------------------
void OSQLParseNode::parseNodeToStr(::rtl::OUString& rString,
                                   const Reference< XConnection >& _rxConnection,
                                   const IParseContext* pContext,
                                   sal_Bool _bIntl,
                                   sal_Bool _bQuote) const
{

    parseNodeToStr(
        rString, _rxConnection, NULL, NULL,
        pContext ? pContext->getPreferredLocale() : OParseContext::getDefaultLocale(),
        pContext, _bIntl, _bQuote, '.', false, false );
}

//-----------------------------------------------------------------------------
void OSQLParseNode::parseNodeToPredicateStr(::rtl::OUString& rString,
                                              const Reference< XConnection >& _rxConnection,
                                              const Reference< XNumberFormatter > & xFormatter,
                                              const ::com::sun::star::lang::Locale& rIntl,
                                              sal_Char _cDec,
                                              const IParseContext* pContext ) const
{

    OSL_ENSURE(xFormatter.is(), "OSQLParseNode::parseNodeToPredicateStr:: no formatter!");

    if (xFormatter.is())
        parseNodeToStr(rString, _rxConnection, xFormatter, NULL, rIntl, pContext, sal_True, sal_True, _cDec, true, false);
}

//-----------------------------------------------------------------------------
void OSQLParseNode::parseNodeToPredicateStr(::rtl::OUString& rString,
                                              const Reference< XConnection > & _rxConnection,
                                              const Reference< XNumberFormatter > & xFormatter,
                                              const Reference< XPropertySet > & _xField,
                                              const ::com::sun::star::lang::Locale& rIntl,
                                              sal_Char _cDec,
                                              const IParseContext* pContext ) const
{

    OSL_ENSURE(xFormatter.is(), "OSQLParseNode::parseNodeToPredicateStr:: no formatter!");

    if (xFormatter.is())
        parseNodeToStr( rString, _rxConnection, xFormatter, _xField, rIntl, pContext, true, true, _cDec, true, false );
}

//-----------------------------------------------------------------------------
void OSQLParseNode::parseNodeToStr(::rtl::OUString& rString,
                      const Reference< XConnection > & _rxConnection,
                      const Reference< XNumberFormatter > & xFormatter,
                      const Reference< XPropertySet > & _xField,
                      const ::com::sun::star::lang::Locale& rIntl,
                      const IParseContext* pContext,
                      bool _bIntl,
                      bool _bQuote,
                      sal_Char _cDecSep,
                      bool _bPredicate,
                      bool _bSubstitute) const
{

    OSL_ENSURE( _rxConnection.is(), "OSQLParseNode::parseNodeToStr: invalid connection!" );

    if ( _rxConnection.is() )
    {
        try
        {
            OSQLParseNode::impl_parseNodeToString_throw( rString,
                SQLParseNodeParameter(
                    _rxConnection, xFormatter, _xField, rIntl, pContext,
                    _bIntl, _bQuote, _cDecSep, _bPredicate, _bSubstitute
                ) );
        }
        catch( const SQLException& )
        {
            OSL_ENSURE( false, "OSQLParseNode::parseNodeToStr: this should not throw!" );
            // our callers don't expect this method to throw anything. The only known situation
            // where impl_parseNodeToString_throw can throw is when there is a cyclic reference
            // in the sub queries, but this cannot be the case here, as we do not parse to
            // SDBC level.
        }
    }
}
//-----------------------------------------------------------------------------
bool OSQLParseNode::parseNodeToExecutableStatement( ::rtl::OUString& _out_rString, const Reference< XConnection >& _rxConnection,
    OSQLParser& _rParser, ::com::sun::star::sdbc::SQLException* _pErrorHolder ) const
{
    OSL_PRECOND( _rxConnection.is(), "OSQLParseNode::parseNodeToExecutableStatement: invalid connection!" );
    SQLParseNodeParameter aParseParam( _rxConnection,
        NULL, NULL, OParseContext::getDefaultLocale(), NULL, false, true, '.', false, true );

    if ( aParseParam.aMetaData.supportsSubqueriesInFrom() )
    {
        Reference< XQueriesSupplier > xSuppQueries( _rxConnection, UNO_QUERY );
        OSL_ENSURE( xSuppQueries.is(), "OSQLParseNode::parseNodeToExecutableStatement: cannot substitute everything without a QueriesSupplier!" );
        if ( xSuppQueries.is() )
            aParseParam.xQueries = xSuppQueries->getQueries();
    }

    aParseParam.pParser = &_rParser;

    _out_rString = ::rtl::OUString();
    bool bSuccess = false;
    try
    {
        impl_parseNodeToString_throw( _out_rString, aParseParam );
        bSuccess = true;
    }
    catch( const SQLException& e )
    {
        if ( _pErrorHolder )
            *_pErrorHolder = e;
    }
    return bSuccess;
}

//-----------------------------------------------------------------------------
namespace
{
    bool lcl_isAliasNamePresent( const OSQLParseNode& _rTableNameNode )
    {
        OSL_ENSURE( _rTableNameNode.getKnownRuleID() == OSQLParseNode::table_name, "lcl_isAliasNamePresent: Must be a 'table_name' rule!" );
        OSL_ENSURE( _rTableNameNode.getParent()->getKnownRuleID() == OSQLParseNode::table_ref, "lcl_isAliasNamePresent: parent must be a 'table_ref'!" );

        const OSQLParseNode* pTableRef = _rTableNameNode.getParent();
        if ( pTableRef->count() == 4 )
        {
            // table_ref := table_node as range_variable op_column_commalist
            // table_node := table_name | ...
            if ( pTableRef->getChild(1)->getKnownRuleID() == OSQLParseNode::as )
                return true;

            // table_ref := '{' SQL_TOKEN_OJ joined_table '}'
            OSL_ENSURE( SQL_ISPUNCTUATION( pTableRef->getChild(0), "(" ), "lcl_isAliasNamePresent: grammar changed(1)!" );
            return false;
        }

        if ( pTableRef->count() == 6 )
        {   // '(' joined_table ')' as range_variable op_column_commalist
            OSL_ENSURE( SQL_ISPUNCTUATION( pTableRef->getChild(2), "(" )
                    &&  SQL_ISRULE( pTableRef->getChild(3), as ),
                    "lcl_isAliasNamePresent: grammar changed(3)!" );
            return true;
        }

        if ( pTableRef->count() == 3 )
            // subquery as range_variable
            return true;

        if ( pTableRef->count() == 1 )
            return false;

#if OSL_DEBUG_LEVEL > 0
        for ( size_t i=0; i<pTableRef->count(); ++i )
        {
            const OSQLParseNode* pChildNode = pTableRef->getChild( i );
            OSQLParseNode::Rule eRuleID = pChildNode->getKnownRuleID();
            OSL_UNUSED( eRuleID );
        }
#endif
        OSL_ENSURE( false, "lcl_isAliasNamePresent: unreachable code - except you extended the production rules for table_ref!" );
        return false;
    }
}

//-----------------------------------------------------------------------------
void OSQLParseNode::impl_parseNodeToString_throw(::rtl::OUString& rString, const SQLParseNodeParameter& rParam) const
{
    if ( isToken() )
    {
        parseLeaf(rString,rParam);
        return;
    }

    // einmal auswerten wieviel Subtrees dieser Knoten besitzt
    sal_uInt32 nCount = count();

    bool bHandled = false;
    switch ( getKnownRuleID() )
    {
    // special handling for parameters
    case parameter:
    {
        if(rString.getLength())
            rString += ::rtl::OUString::createFromAscii(" ");
        if (nCount == 1)    // ?
            m_aChilds[0]->impl_parseNodeToString_throw( rString, rParam );
        else if (nCount == 2)   // :Name
        {
            m_aChilds[0]->impl_parseNodeToString_throw( rString, rParam );
            rString += m_aChilds[1]->m_aNodeValue;
        }                   // [Name]
        else
        {
            m_aChilds[0]->impl_parseNodeToString_throw( rString, rParam );
            rString += m_aChilds[1]->m_aNodeValue;
            rString += m_aChilds[2]->m_aNodeValue;
        }
        bHandled = true;
    }
    break;

    // table refs
    case table_ref:
        if  (   ( nCount == 4 )
            ||  ( ( nCount == 6 ) &&  SQL_ISPUNCTUATION( m_aChilds[0], "(" ) )
            )
        {
            impl_parseTableRangeNodeToString_throw( rString, rParam );
            bHandled = true;
        }
        break;

    // table name - might be a query name
    case table_name:
        bHandled = impl_parseTableNameNodeToString_throw( rString, rParam );
        break;

    case as:
        if ( rParam.aMetaData.generateASBeforeCorrelationName() )
            rString += ::rtl::OUString::createFromAscii( " AS" );
        bHandled = true;
        break;

    case like_predicate:
        // je nachdem ob international angegeben wird oder nicht wird like anders behandelt
        // interanational: *, ? sind Platzhalter
        // sonst SQL92 konform: %, _
        impl_parseLikeNodeToString_throw( rString, rParam );
        bHandled = true;
        break;

    case general_set_fct:
    case set_fct_spec:
    case position_exp:
    case extract_exp:
    case length_exp:
    case char_value_fct:
    {
        if (!addDateValue(rString, rParam))
        {
            // Funktionsname nicht quoten
            SQLParseNodeParameter aNewParam(rParam);
            aNewParam.bQuote = ( SQL_ISRULE(this,length_exp)    || SQL_ISRULE(this,char_value_fct) );

            m_aChilds[0]->impl_parseNodeToString_throw( rString, aNewParam );
            aNewParam.bQuote = rParam.bQuote;
            //aNewParam.bPredicate = sal_False; // disable [ ] around names // look at i73215
            ::rtl::OUString aStringPara;
            for (sal_uInt32 i=1; i<nCount; i++)
            {
                const OSQLParseNode * pSubTree = m_aChilds[i];
                if (pSubTree)
                {
                    pSubTree->impl_parseNodeToString_throw( aStringPara, aNewParam );

                    // bei den CommaListen zwischen alle Subtrees Commas setzen
                    if ((m_eNodeType == SQL_NODE_COMMALISTRULE)     && (i < (nCount - 1)))
                        aStringPara += ::rtl::OUString::createFromAscii(",");
                }
                else
                    i++;
            }
            aStringPara.trim();
            rString += aStringPara;
        }
        bHandled = true;
    }
    break;
    default:
        break;
    }   // switch ( getKnownRuleID() )

    if ( !bHandled )
    {
        for (OSQLParseNodes::const_iterator i = m_aChilds.begin();
            i != m_aChilds.end();)
        {
            const OSQLParseNode* pSubTree = *i;
            if ( !pSubTree )
            {
                ++i;
                continue;
            }

            SQLParseNodeParameter aNewParam(rParam);

            // don't replace the field for subqueries
            if (rParam.xField.is() && SQL_ISRULE(pSubTree,subquery))
                aNewParam.xField = NULL;

            // if there is a field given we don't display the fieldname, if there is any
            if (rParam.xField.is() && SQL_ISRULE(pSubTree,column_ref))
            {
                sal_Bool bFilter = sal_False;
                // retrieve the fields name
                ::rtl::OUString aFieldName;
                try
                {
                    sal_Int32 nNamePropertyId = PROPERTY_ID_NAME;
                    if ( rParam.xField->getPropertySetInfo()->hasPropertyByName( OMetaConnection::getPropMap().getNameByIndex( PROPERTY_ID_REALNAME ) ) )
                        nNamePropertyId = PROPERTY_ID_REALNAME;
                    rParam.xField->getPropertyValue( OMetaConnection::getPropMap().getNameByIndex( nNamePropertyId ) ) >>= aFieldName;
                }
                catch ( Exception& )
                {
                }

                if(pSubTree->count())
                {
                    const OSQLParseNode* pCol = pSubTree->m_aChilds[pSubTree->count()-1];
                    if  (   (   SQL_ISRULE(pCol,column_val)
                            &&  pCol->getChild(0)->getTokenValue().equalsIgnoreAsciiCase(aFieldName)
                            )
                        ||  pCol->getTokenValue().equalsIgnoreAsciiCase(aFieldName)
                        )
                        bFilter = sal_True;
                }

                // ok we found the field, if the following node is the
                // comparision operator '=' we filter it as well
                if (bFilter)
                {
                    if (SQL_ISRULE(this, comparison_predicate))
                    {
                        ++i;
                        if(i != m_aChilds.end())
                        {
                            pSubTree = *i;
                            if (pSubTree && pSubTree->getNodeType() == SQL_NODE_EQUAL)
                                i++;
                        }
                    }
                    else
                        i++;
                }
                else
                {
                    pSubTree->impl_parseNodeToString_throw( rString, aNewParam );
                    i++;

                    // bei den CommaListen zwischen alle Subtrees Commas setzen
                    if ((m_eNodeType == SQL_NODE_COMMALISTRULE)     && (i != m_aChilds.end()))
                        rString += ::rtl::OUString::createFromAscii(",");
                }
            }
            else
            {
                pSubTree->impl_parseNodeToString_throw( rString, aNewParam );
                i++;

                // bei den CommaListen zwischen alle Subtrees Commas setzen
                if ((m_eNodeType == SQL_NODE_COMMALISTRULE)     && (i != m_aChilds.end()))
                {
                    if (SQL_ISRULE(this,value_exp_commalist) && rParam.bPredicate)
                        rString += ::rtl::OUString::createFromAscii(";");
                    else
                        rString += ::rtl::OUString::createFromAscii(",");
                }
            }
        }
    }
}

//-----------------------------------------------------------------------------
bool OSQLParseNode::impl_parseTableNameNodeToString_throw( ::rtl::OUString& rString, const SQLParseNodeParameter& rParam ) const
{
    // is the table_name part of a table_ref?
    OSL_ENSURE( getParent(), "OSQLParseNode::impl_parseTableNameNodeToString_throw: table_name without parent?" );
    if ( !getParent() || ( getParent()->getKnownRuleID() != table_ref ) )
        return false;

    // if it's a query, maybe we need to substitute the SQL statement ...
    if ( !rParam.bParseToSDBCLevel )
        return false;

    if ( !rParam.xQueries.is() )
        // connection does not support queries in queries, or was no query supplier
        return false;

    try
    {
        ::rtl::OUString sTableOrQueryName( getChild(0)->getTokenValue() );
        bool bIsQuery = rParam.xQueries->hasByName( sTableOrQueryName );
        if ( !bIsQuery )
            return false;

        // avoid recursion (e.g. "foo" defined as "SELECT * FROM bar" and "bar" defined as "SELECT * FROM foo".
        if ( rParam.pSubQueryHistory->find( sTableOrQueryName ) != rParam.pSubQueryHistory->end() )
        {
            ::rtl::OUString sMessage( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "cyclic sub queries" ) ) );
            OSL_ENSURE( rParam.pParser, "OSQLParseNode::impl_parseTableNameNodeToString_throw: no parser?" );
            if ( rParam.pParser )
            {
                const SQLError& rErrors( rParam.pParser->getErrorHelper() );
                rErrors.raiseException( sdb::ErrorCondition::PARSER_CYCLIC_SUB_QUERIES );
            }
            else
            {
                SQLError aErrors( ::comphelper::getProcessServiceFactory() );
                aErrors.raiseException( sdb::ErrorCondition::PARSER_CYCLIC_SUB_QUERIES );
            }
        }
        rParam.pSubQueryHistory->insert( sTableOrQueryName );

        Reference< XPropertySet > xQuery( rParam.xQueries->getByName( sTableOrQueryName ), UNO_QUERY_THROW );

        // substitute the query name with the constituting command
        ::rtl::OUString sCommand;
        OSL_VERIFY( xQuery->getPropertyValue( OMetaConnection::getPropMap().getNameByIndex( PROPERTY_ID_COMMAND ) ) >>= sCommand );

        sal_Bool bEscapeProcessing = sal_False;
        OSL_VERIFY( xQuery->getPropertyValue( OMetaConnection::getPropMap().getNameByIndex( PROPERTY_ID_ESCAPEPROCESSING ) ) >>= bEscapeProcessing );

        // the query we found here might itself be based on another query, so parse it recursively
        OSL_ENSURE( rParam.pParser, "OSQLParseNode::impl_parseTableNameNodeToString_throw: cannot analyze sub queries without a parser!" );
        if ( bEscapeProcessing && rParam.pParser )
        {
            ::rtl::OUString sError;
            ::std::auto_ptr< OSQLParseNode > pSubQueryNode( rParam.pParser->parseTree( sError, sCommand, sal_False ) );
            if ( pSubQueryNode.get() )
            {
                // parse the sub-select to SDBC level, too
                ::rtl::OUString sSubSelect;
                pSubQueryNode->impl_parseNodeToString_throw( sSubSelect, rParam );
                if ( sSubSelect.getLength() )
                    sCommand = sSubSelect;
            }
        }

        rString += ::rtl::OUString::createFromAscii( " ( " );
        rString += sCommand;
        rString += ::rtl::OUString::createFromAscii( " )" );

        // append the query name as table alias, since it might be referenced in other
        // parts of the statement - but only if there's no other alias name present
        if ( !lcl_isAliasNamePresent( *this ) )
        {
            rString += ::rtl::OUString::createFromAscii( " AS " );
            if ( rParam.bQuote )
                rString += SetQuotation( sTableOrQueryName,
                    rParam.aMetaData.getIdentifierQuoteString(), rParam.aMetaData.getIdentifierQuoteString() );
        }

        // don't forget to remove the query name from the history, else multiple inclusions
        // won't work
        // #i69227# / 2006-10-10 / frank.schoenheit@sun.com
        rParam.pSubQueryHistory->erase( sTableOrQueryName );

        return true;
    }
    catch( const SQLException& )
    {
        throw;
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
    return false;
}

//-----------------------------------------------------------------------------
void OSQLParseNode::impl_parseTableRangeNodeToString_throw(::rtl::OUString& rString, const SQLParseNodeParameter& rParam) const
{
    sal_uInt32 nCount(count());
    rString += ::rtl::OUString::createFromAscii(" ");

    if (nCount == 4)
    {
        m_aChilds[0]->impl_parseNodeToString_throw( rString, rParam );
        m_aChilds[1]->impl_parseNodeToString_throw( rString, rParam );
        m_aChilds[2]->impl_parseNodeToString_throw( rString, rParam );
        m_aChilds[3]->impl_parseNodeToString_throw( rString, rParam );
    }
    else if(nCount == 6 && SQL_ISPUNCTUATION(m_aChilds[0],"("))
    {
        m_aChilds[0]->impl_parseNodeToString_throw( rString, rParam );
        m_aChilds[1]->impl_parseNodeToString_throw( rString, rParam );
        m_aChilds[2]->impl_parseNodeToString_throw( rString, rParam );
        m_aChilds[3]->impl_parseNodeToString_throw( rString, rParam );
        m_aChilds[4]->impl_parseNodeToString_throw( rString, rParam );
        m_aChilds[5]->impl_parseNodeToString_throw( rString, rParam );
    }
}

//-----------------------------------------------------------------------------
void OSQLParseNode::impl_parseLikeNodeToString_throw( ::rtl::OUString& rString, const SQLParseNodeParameter& rParam ) const
{
    OSL_ENSURE(count() >= 4,"count != 5: Prepare for GPF");

    const OSQLParseNode* pEscNode = NULL;
    const OSQLParseNode* pParaNode = NULL;

    SQLParseNodeParameter aNewParam(rParam);
    //aNewParam.bQuote = sal_True; // why setting this to true? @see http://www.openoffice.org/issues/show_bug.cgi?id=75557

    // if there is a field given we don't display the fieldname, if there are any
    sal_Bool bAddName = sal_True;
    if (rParam.xField.is())
    {
        // retrieve the fields name
        ::rtl::OUString aFieldName;
        try
        {
            // retrieve the fields name
            rtl::OUString aString;
            rParam.xField->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME)) >>= aString;
            aFieldName = aString.getStr();
        }
        catch ( Exception& )
        {
            OSL_ENSURE( false, "OSQLParseNode::impl_parseLikeNodeToString_throw Exception occured!" );
        }
        if ( !m_aChilds[0]->isLeaf() )
        {
            const OSQLParseNode* pCol = m_aChilds[0]->getChild(m_aChilds[0]->count()-1);
            if ((SQL_ISRULE(pCol,column_val) && pCol->getChild(0)->getTokenValue().equalsIgnoreAsciiCase(aFieldName)) ||
                pCol->getTokenValue().equalsIgnoreAsciiCase(aFieldName) )
                bAddName = sal_False;
        }
    }

    if (bAddName)
        m_aChilds[0]->impl_parseNodeToString_throw( rString, aNewParam );

    m_aChilds[1]->impl_parseNodeToString_throw( rString, aNewParam );
    if(count() == 5)
        m_aChilds[2]->impl_parseNodeToString_throw( rString, aNewParam );

    sal_Int32 nCurentPos = m_aChilds.size()-2;
    pParaNode = m_aChilds[nCurentPos];
    pEscNode = m_aChilds[nCurentPos+1];

    if (pParaNode->isToken())
    {
        ::rtl::OUString aStr = ConvertLikeToken(pParaNode, pEscNode, rParam.bInternational);
        rString += ::rtl::OUString::createFromAscii(" ");
        rString += SetQuotation(aStr,::rtl::OUString::createFromAscii("\'"),::rtl::OUString::createFromAscii("\'\'"));
    }
    else
        pParaNode->impl_parseNodeToString_throw( rString, aNewParam );

    pEscNode->impl_parseNodeToString_throw( rString, aNewParam );
}


// -----------------------------------------------------------------------------
sal_Bool OSQLParseNode::getTableComponents(const OSQLParseNode* _pTableNode,
                                            ::com::sun::star::uno::Any &_rCatalog,
                                            ::rtl::OUString &_rSchema,
                                            ::rtl::OUString &_rTable)
{
    OSL_ENSURE(_pTableNode,"Wrong use of getTableComponents! _pTableNode is not allowed to be null!");
    if(_pTableNode)
    {
        const OSQLParseNode* pTableNode = _pTableNode;
        // clear the parameter given
        _rCatalog = Any();
        _rSchema = _rTable = ::rtl::OUString();
        // see rule catalog_name: in sqlbison.y
        if (SQL_ISRULE(pTableNode,catalog_name))
        {
            OSL_ENSURE(pTableNode->getChild(0) && pTableNode->getChild(0)->isToken(),"Invalid parsenode!");
            _rCatalog <<= pTableNode->getChild(0)->getTokenValue();
            pTableNode = pTableNode->getChild(2);
        }
        // check if we have schema_name rule
        if(SQL_ISRULE(pTableNode,schema_name))
        {
            _rSchema = pTableNode->getChild(0)->getTokenValue();
            pTableNode = pTableNode->getChild(2);
        }
        // check if we have table_name rule
        if(SQL_ISRULE(pTableNode,table_name))
        {
            _rTable = pTableNode->getChild(0)->getTokenValue();
        }
        else
        {
            OSL_ENSURE(0,"Error in parse tree!");
        }
    }
    return _rTable.getLength() != 0;
}
// -----------------------------------------------------------------------------
void OSQLParser::killThousandSeparator(OSQLParseNode* pLiteral)
{
    if ( pLiteral )
    {
        if ( s_xLocaleData->getLocaleItem( m_pData->aLocale ).decimalSeparator.toChar() == ',' )
        {
            pLiteral->m_aNodeValue = pLiteral->m_aNodeValue.replace('.', sal_Unicode());
            // and replace decimal
            pLiteral->m_aNodeValue = pLiteral->m_aNodeValue.replace(',', '.');
        }
        else
            pLiteral->m_aNodeValue = pLiteral->m_aNodeValue.replace(',', sal_Unicode());
        }
}
// -----------------------------------------------------------------------------
OSQLParseNode* OSQLParser::convertNode(sal_Int32 nType,OSQLParseNode*& pLiteral)
{
    if ( !pLiteral )
        return NULL;

    OSQLParseNode* pReturn = pLiteral;

    if ( ( pLiteral->isRule() && !SQL_ISRULE(pLiteral,value_exp) ) || SQL_ISTOKEN(pLiteral,FALSE) || SQL_ISTOKEN(pLiteral,TRUE) )
    {
        switch(nType)
        {
            case DataType::CHAR:
            case DataType::VARCHAR:
            case DataType::LONGVARCHAR:
            if ( !SQL_ISRULE(pReturn,char_value_exp) && !buildStringNodes(pReturn) )
                pReturn = NULL;
            default:
                break;
        }
    }
    else
    {
        switch(pLiteral->getNodeType())
        {
        case SQL_NODE_STRING:
            switch(nType)
            {
                case DataType::CHAR:
                case DataType::VARCHAR:
                case DataType::LONGVARCHAR:
                    break;
                case DataType::DATE:
                case DataType::TIME:
                case DataType::TIMESTAMP:
                    if (m_xFormatter.is())
                    pReturn = buildDate( nType, pReturn);
                    break;
                default:
                    m_sErrorMessage = m_pContext->getErrorMessage(IParseContext::ERROR_INVALID_COMPARE);
                    break;
            }
            break;
        case SQL_NODE_ACCESS_DATE:
            switch(nType)
            {
                case DataType::DATE:
                case DataType::TIME:
                case DataType::TIMESTAMP:
                if ( m_xFormatter.is() )
                    pReturn = buildDate( nType, pReturn);
                    else
                        m_sErrorMessage = m_pContext->getErrorMessage(IParseContext::ERROR_INVALID_DATE_COMPARE);
                    break;
                default:
                    m_sErrorMessage = m_pContext->getErrorMessage(IParseContext::ERROR_INVALID_COMPARE);
                    break;
            }
            break;
        case SQL_NODE_INTNUM:
            switch(nType)
            {
                case DataType::BIT:
                case DataType::BOOLEAN:
                case DataType::DECIMAL:
                case DataType::NUMERIC:
                case DataType::TINYINT:
                case DataType::SMALLINT:
                case DataType::INTEGER:
                case DataType::BIGINT:
                case DataType::FLOAT:
                case DataType::REAL:
                case DataType::DOUBLE:
                    // kill thousand seperators if any
                killThousandSeparator(pReturn);
                    break;
                case DataType::CHAR:
                case DataType::VARCHAR:
                case DataType::LONGVARCHAR:
                pReturn = buildNode_STR_NUM(pReturn);
                    break;
                default:
                    m_sErrorMessage = m_pContext->getErrorMessage(IParseContext::ERROR_INVALID_INT_COMPARE);
                    break;
            }
            break;
        case SQL_NODE_APPROXNUM:
            switch(nType)
            {
                case DataType::DECIMAL:
                case DataType::NUMERIC:
                case DataType::FLOAT:
                case DataType::REAL:
                case DataType::DOUBLE:
                        // kill thousand seperators if any
                killThousandSeparator(pReturn);
                    break;
                case DataType::CHAR:
                case DataType::VARCHAR:
                case DataType::LONGVARCHAR:
                pReturn = buildNode_STR_NUM(pReturn);
                    break;
                case DataType::INTEGER:
                default:
                    m_sErrorMessage = m_pContext->getErrorMessage(IParseContext::ERROR_INVALID_REAL_COMPARE);
                    break;
            }
            break;
        default:
            OSL_ENSURE(0,"Not handled!");
        }
    }
    return pReturn;
}
// -----------------------------------------------------------------------------
sal_Int16 OSQLParser::buildPredicateRule(OSQLParseNode*& pAppend,OSQLParseNode* pLiteral,OSQLParseNode*& pCompare,OSQLParseNode* pLiteral2)
{
    OSL_ENSURE(inPredicateCheck(),"Only in predicate check allowed!");
    sal_Int16 nErg = 0;
    if ( m_xField.is() )
    {
        sal_Int32 nType = 0;
        try
        {
            m_xField->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE)) >>= nType;
        }
        catch( Exception& )
        {
            return nErg;
        }

        OSQLParseNode* pNode1 = convertNode(nType,pLiteral);
        if ( pNode1 )
        {
            OSQLParseNode* pNode2 = convertNode(nType,pLiteral2);
            if ( !m_sErrorMessage.getLength() )
                nErg = buildNode(pAppend,pCompare,pNode1,pNode2);
        }
    }
    if (!pCompare->getParent()) // I have no parent so I was not used and I must die :-)
        delete pCompare;
    return nErg;
}
// -----------------------------------------------------------------------------
sal_Int16 OSQLParser::buildLikeRule(OSQLParseNode*& pAppend, OSQLParseNode*& pLiteral, const OSQLParseNode* pEscape)
{
    sal_Int16 nErg = 0;
    sal_Int32 nType = 0;

    if (!m_xField.is())
        return nErg;
    try
    {
        Any aValue;
        {
            aValue = m_xField->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE));
            aValue >>= nType;
        }
    }
    catch( Exception& )
    {
        return nErg;
    }

    switch (nType)
    {
        case DataType::CHAR:
        case DataType::VARCHAR:
        case DataType::LONGVARCHAR:
            if(pLiteral->isRule())
            {
                pAppend->append(pLiteral);
                nErg = 1;
            }
            else
            {
                switch(pLiteral->getNodeType())
                {
                    case SQL_NODE_STRING:
                        pLiteral->m_aNodeValue = ConvertLikeToken(pLiteral, pEscape, sal_False);
                        pAppend->append(pLiteral);
                        nErg = 1;
                        break;
                    case SQL_NODE_APPROXNUM:
                        if (m_xFormatter.is() && m_nFormatKey)
                        {
                            sal_Int16 nScale = 0;
                            try
                            {
                                Any aValue = getNumberFormatProperty( m_xFormatter, m_nFormatKey, ::rtl::OUString::createFromAscii("Decimals") );
                                aValue >>= nScale;
                            }
                            catch( Exception& )
                            {
                            }

                            pAppend->append(new OSQLInternalNode(stringToDouble(pLiteral->getTokenValue(),nScale),SQL_NODE_STRING));
                        }
                        else
                            pAppend->append(new OSQLInternalNode(pLiteral->getTokenValue(),SQL_NODE_STRING));

                        delete pLiteral;
                        nErg = 1;
                        break;
                    default:
                        m_sErrorMessage = m_pContext->getErrorMessage(IParseContext::ERROR_VALUE_NO_LIKE);
                        m_sErrorMessage = m_sErrorMessage.replaceAt(m_sErrorMessage.indexOf(::rtl::OUString::createFromAscii("#1")),2,pLiteral->getTokenValue());
                        break;
                }
            }
            break;
        default:
            m_sErrorMessage = m_pContext->getErrorMessage(IParseContext::ERROR_FIELD_NO_LIKE);
            break;
    }
    return nErg;
}
//-----------------------------------------------------------------------------
OSQLParseNode* OSQLParser::buildNode_Date(const double& fValue, sal_Int32 nType)
{
    ::rtl::OUString aEmptyString;
    OSQLParseNode* pNewNode = new OSQLInternalNode(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::set_fct_spec));
    pNewNode->append(new OSQLInternalNode(::rtl::OUString::createFromAscii("{"), SQL_NODE_PUNCTUATION));
    OSQLParseNode* pDateNode = new OSQLInternalNode(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::odbc_fct_spec));
    pNewNode->append(pDateNode);
    pNewNode->append(new OSQLInternalNode(::rtl::OUString::createFromAscii("}"), SQL_NODE_PUNCTUATION));

    switch (nType)
    {
        case DataType::DATE:
        {
            Date aDate = DBTypeConversion::toDate(fValue,DBTypeConversion::getNULLDate(m_xFormatter->getNumberFormatsSupplier()));
            ::rtl::OUString aString = DBTypeConversion::toDateString(aDate);
            pDateNode->append(new OSQLInternalNode(aEmptyString, SQL_NODE_KEYWORD, SQL_TOKEN_D));
            pDateNode->append(new OSQLInternalNode(aString, SQL_NODE_STRING));
            break;
        }
        case DataType::TIME:
        {
            Time aTime = DBTypeConversion::toTime(fValue);
            ::rtl::OUString aString = DBTypeConversion::toTimeString(aTime);
            pDateNode->append(new OSQLInternalNode(aEmptyString, SQL_NODE_KEYWORD, SQL_TOKEN_T));
            pDateNode->append(new OSQLInternalNode(aString, SQL_NODE_STRING));
            break;
        }
        case DataType::TIMESTAMP:
        {
            DateTime aDateTime = DBTypeConversion::toDateTime(fValue,DBTypeConversion::getNULLDate(m_xFormatter->getNumberFormatsSupplier()));
            if (aDateTime.Seconds || aDateTime.Minutes || aDateTime.Hours)
            {
                ::rtl::OUString aString = DBTypeConversion::toDateTimeString(aDateTime);
                pDateNode->append(new OSQLInternalNode(aEmptyString, SQL_NODE_KEYWORD, SQL_TOKEN_TS));
                pDateNode->append(new OSQLInternalNode(aString, SQL_NODE_STRING));
            }
            else
            {
                Date aDate(aDateTime.Day,aDateTime.Month,aDateTime.Year);
                pDateNode->append(new OSQLInternalNode(aEmptyString, SQL_NODE_KEYWORD, SQL_TOKEN_D));
                pDateNode->append(new OSQLInternalNode(DBTypeConversion::toDateString(aDate), SQL_NODE_STRING));
            }
            break;
        }
    }

    return pNewNode;
}
// -----------------------------------------------------------------------------
OSQLParseNode* OSQLParser::buildNode_STR_NUM(OSQLParseNode*& _pLiteral)
{
    OSQLParseNode* pReturn = NULL;
    if ( _pLiteral )
    {
        if (m_nFormatKey)
        {
            sal_Int16 nScale = 0;
            ::rtl::OUString aDec;
            try
            {
                Any aValue = getNumberFormatProperty( m_xFormatter, m_nFormatKey, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Decimals")) );
                aValue >>= nScale;
            }
            catch( Exception& )
            {
            }

            pReturn = new OSQLInternalNode(stringToDouble(_pLiteral->getTokenValue(),nScale),SQL_NODE_STRING);
        }
        else
            pReturn = new OSQLInternalNode(_pLiteral->getTokenValue(),SQL_NODE_STRING);

        delete _pLiteral;
        _pLiteral = NULL;
    }
    return pReturn;
}
// -----------------------------------------------------------------------------
::rtl::OUString OSQLParser::stringToDouble(const ::rtl::OUString& _rValue,sal_Int16 _nScale)
{
    ::rtl::OUString aValue;
    if(!m_xCharClass.is())
        m_xCharClass  = Reference<XCharacterClassification>(m_xServiceFactory->createInstance(::rtl::OUString::createFromAscii("com.sun.star.i18n.CharacterClassification")),UNO_QUERY);
    if(m_xCharClass.is() && s_xLocaleData.is())
    {
        try
        {
            ParseResult aResult = m_xCharClass->parsePredefinedToken(KParseType::ANY_NUMBER,_rValue,0,m_pData->aLocale,0,::rtl::OUString(),KParseType::ANY_NUMBER,::rtl::OUString());
            if((aResult.TokenType & KParseType::IDENTNAME) && aResult.EndPos == _rValue.getLength())
            {
                aValue = ::rtl::OUString::valueOf(aResult.Value);
                sal_Int32 nPos = aValue.lastIndexOf(::rtl::OUString::createFromAscii("."));
                if((nPos+_nScale) < aValue.getLength())
                    aValue = aValue.replaceAt(nPos+_nScale,aValue.getLength()-nPos-_nScale,::rtl::OUString());
                aValue = aValue.replaceAt(aValue.lastIndexOf(::rtl::OUString::createFromAscii(".")),1,s_xLocaleData->getLocaleItem(m_pData->aLocale).decimalSeparator);
                return aValue;
            }
        }
        catch(Exception&)
        {
        }
    }
    return aValue;
}
// -----------------------------------------------------------------------------

::osl::Mutex& OSQLParser::getMutex()
{
    static ::osl::Mutex aMutex;
    return aMutex;
}

//-----------------------------------------------------------------------------
OSQLParseNode* OSQLParser::predicateTree(::rtl::OUString& rErrorMessage, const ::rtl::OUString& rStatement,
                                          const Reference< ::com::sun::star::util::XNumberFormatter > & xFormatter,
                                         const Reference< XPropertySet > & xField)
{


    // mutex for parsing
    static ::osl::Mutex aMutex;

    // Guard the parsing
    ::osl::MutexGuard aGuard(getMutex());
    // must be reset
    setParser(this);


    // reset the parser
    m_xField        = xField;
    m_xFormatter    = xFormatter;

    if (m_xField.is())
    {
        sal_Int32 nType=0;
        try
        {
            // get the field name
            rtl::OUString aString;

            // retrieve the fields name
            // #75243# use the RealName of the column if there is any otherwise the name which could be the alias
            // of the field
            Reference< XPropertySetInfo> xInfo = m_xField->getPropertySetInfo();
            if ( xInfo->hasPropertyByName(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_REALNAME)))
                m_xField->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_REALNAME)) >>= aString;
            else
                m_xField->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME)) >>= aString;

            m_sFieldName = aString;

            // get the field format key
            if ( xInfo->hasPropertyByName(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FORMATKEY)))
                m_xField->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FORMATKEY)) >>= m_nFormatKey;
            else
                m_nFormatKey = 0;

            // get the field type
            m_xField->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE)) >>= nType;
        }
        catch ( Exception& )
        {
            OSL_ASSERT(0);
        }

        if (m_nFormatKey && m_xFormatter.is())
        {
            Any aValue = getNumberFormatProperty( m_xFormatter, m_nFormatKey, OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_LOCALE) );
            OSL_ENSURE(aValue.getValueType() == ::getCppuType((const ::com::sun::star::lang::Locale*)0), "OSQLParser::PredicateTree : invalid language property !");

            if (aValue.getValueType() == ::getCppuType((const ::com::sun::star::lang::Locale*)0))
                aValue >>= m_pData->aLocale;
        }
        else
            m_pData->aLocale = m_pContext->getPreferredLocale();

        if ( m_xFormatter.is() )
        {
            try
            {
                Reference< ::com::sun::star::util::XNumberFormatsSupplier >  xFormatSup = m_xFormatter->getNumberFormatsSupplier();
                if ( xFormatSup.is() )
                {
                    Reference< ::com::sun::star::util::XNumberFormats >  xFormats = xFormatSup->getNumberFormats();
                    if ( xFormats.is() )
                    {
                        ::com::sun::star::lang::Locale aLocale;
                        aLocale.Language = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("en"));
                        aLocale.Country = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("US"));
                        ::rtl::OUString sFormat(RTL_CONSTASCII_USTRINGPARAM("YYYY-MM-DD"));
                        m_nDateFormatKey = xFormats->queryKey(sFormat,aLocale,sal_False);
                        if ( m_nDateFormatKey == sal_Int32(-1) )
                            m_nDateFormatKey = xFormats->addNew(sFormat, aLocale);
                    }
                }
            }
            catch ( Exception& )
            {
                OSL_ENSURE(0,"DateFormatKey");
            }
        }

        switch (nType)
        {
            case DataType::DATE:
            case DataType::TIME:
            case DataType::TIMESTAMP:
                s_pScanner->SetRule(s_pScanner->GetDATERule());
                break;
            case DataType::CHAR:
            case DataType::VARCHAR:
            case DataType::LONGVARCHAR:
                s_pScanner->SetRule(s_pScanner->GetSTRINGRule());
                break;
            default:
                if ( s_xLocaleData->getLocaleItem( m_pData->aLocale ).decimalSeparator.toChar() == ',' )
                    s_pScanner->SetRule(s_pScanner->GetGERRule());
                else
                    s_pScanner->SetRule(s_pScanner->GetENGRule());
        }

    }
    else
        s_pScanner->SetRule(s_pScanner->GetSQLRule());

    s_pScanner->prepareScan(rStatement, m_pContext, sal_True);

    SQLyylval.pParseNode = NULL;
    //  SQLyypvt = NULL;
    m_pParseTree = NULL;
    m_sErrorMessage= ::rtl::OUString();

    // ... und den Parser anwerfen ...
    if (SQLyyparse() != 0)
    {
        m_sFieldName= ::rtl::OUString();
        m_xField = NULL;
        m_xFormatter = NULL;
        m_nFormatKey = 0;
        m_nDateFormatKey = 0;

        if (!m_sErrorMessage.getLength())
            m_sErrorMessage = s_pScanner->getErrorMessage();
        if (!m_sErrorMessage.getLength())
            m_sErrorMessage = m_pContext->getErrorMessage(IParseContext::ERROR_GENERAL);

        rErrorMessage = m_sErrorMessage;

        // clear the garbage collector
        while (!s_pGarbageCollector->empty())
        {
            OSQLParseNode* pNode = *s_pGarbageCollector->begin();
            while (pNode->getParent())
                pNode = pNode->getParent();
            delete pNode;
        }
        return NULL;
    }
    else
    {
        s_pGarbageCollector->clear();

        m_sFieldName= ::rtl::OUString();
        m_xField = NULL;
        m_xFormatter = NULL;
        m_nFormatKey = 0;
        m_nDateFormatKey = 0;

        // Das Ergebnis liefern (den Root Parse Node):

        // Stattdessen setzt die Parse-Routine jetzt den Member pParseTree
        // - einfach diesen zurueckliefern:
        OSL_ENSURE(m_pParseTree != NULL,"OSQLParser: Parser hat keinen ParseTree geliefert");
        return m_pParseTree;
    }
}

//=============================================================================
//-----------------------------------------------------------------------------
OSQLParser::OSQLParser(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _xServiceFactory,const IParseContext* _pContext)
    :m_pContext(_pContext)
    ,m_pParseTree(NULL)
    ,m_pData( new OSQLParser_Data( _xServiceFactory ) )
    ,m_nFormatKey(0)
    ,m_nDateFormatKey(0)
    ,m_xServiceFactory(_xServiceFactory)
{


    setParser(this);

#ifdef SQLYYDEBUG
#ifdef SQLYYDEBUG_ON
    SQLyydebug = 1;
#endif
#endif

    ::osl::MutexGuard aGuard(getMutex());
    // do we have to initialize the data
    if (s_nRefCount == 0)
    {
        s_pScanner = new OSQLScanner();
        s_pScanner->setScanner();
        s_pGarbageCollector = new OSQLParseNodes();

        if(!s_xLocaleData.is())
            s_xLocaleData = Reference<XLocaleData>(m_xServiceFactory->createInstance(::rtl::OUString::createFromAscii("com.sun.star.i18n.LocaleData")),UNO_QUERY);

        // auf 0 zuruecksetzen
        memset(OSQLParser::s_nRuleIDs,0,sizeof(OSQLParser::s_nRuleIDs[0]) * (OSQLParseNode::rule_count+1));

        struct
        {
            OSQLParseNode::Rule eRule;      // the parse node's ID for the rule
            ::rtl::OString      sRuleName;  // the name of the rule ("select_statement")
        }   aRuleDescriptions[] =
        {
            { OSQLParseNode::select_statement, "select_statement" },
            { OSQLParseNode::table_exp, "table_exp" },
            { OSQLParseNode::table_ref_commalist, "table_ref_commalist" },
            { OSQLParseNode::table_ref, "table_ref" },
            { OSQLParseNode::catalog_name, "catalog_name" },
            { OSQLParseNode::schema_name, "schema_name" },
            { OSQLParseNode::table_name, "table_name" },
            { OSQLParseNode::opt_column_commalist, "opt_column_commalist" },
            { OSQLParseNode::column_commalist, "column_commalist" },
            { OSQLParseNode::column_ref_commalist, "column_ref_commalist" },
            { OSQLParseNode::column_ref, "column_ref" },
            { OSQLParseNode::opt_order_by_clause, "opt_order_by_clause" },
            { OSQLParseNode::ordering_spec_commalist, "ordering_spec_commalist" },
            { OSQLParseNode::ordering_spec, "ordering_spec" },
            { OSQLParseNode::opt_asc_desc, "opt_asc_desc" },
            { OSQLParseNode::where_clause, "where_clause" },
            { OSQLParseNode::opt_where_clause, "opt_where_clause" },
            { OSQLParseNode::search_condition, "search_condition" },
            { OSQLParseNode::comparison_predicate, "comparison_predicate" },
            { OSQLParseNode::between_predicate, "between_predicate" },
            { OSQLParseNode::like_predicate, "like_predicate" },
            { OSQLParseNode::opt_escape, "opt_escape" },
            { OSQLParseNode::test_for_null, "test_for_null" },
            { OSQLParseNode::scalar_exp_commalist, "scalar_exp_commalist" },
            { OSQLParseNode::scalar_exp, "scalar_exp" },
            { OSQLParseNode::parameter_ref, "parameter_ref" },
            { OSQLParseNode::parameter, "parameter" },
            { OSQLParseNode::general_set_fct, "general_set_fct" },
            { OSQLParseNode::range_variable, "range_variable" },
            { OSQLParseNode::column, "column" },
            { OSQLParseNode::delete_statement_positioned, "delete_statement_positioned" },
            { OSQLParseNode::delete_statement_searched, "delete_statement_searched" },
            { OSQLParseNode::update_statement_positioned, "update_statement_positioned" },
            { OSQLParseNode::update_statement_searched, "update_statement_searched" },
            { OSQLParseNode::assignment_commalist, "assignment_commalist" },
            { OSQLParseNode::assignment, "assignment" },
            { OSQLParseNode::values_or_query_spec, "values_or_query_spec" },
            { OSQLParseNode::insert_statement, "insert_statement" },
            { OSQLParseNode::insert_atom_commalist, "insert_atom_commalist" },
            { OSQLParseNode::insert_atom, "insert_atom" },
            { OSQLParseNode::predicate_check, "predicate_check" },
            { OSQLParseNode::from_clause, "from_clause" },
            { OSQLParseNode::qualified_join, "qualified_join" },
            { OSQLParseNode::cross_union, "cross_union" },
            { OSQLParseNode::select_sublist, "select_sublist" },
            { OSQLParseNode::derived_column, "derived_column" },
            { OSQLParseNode::column_val, "column_val" },
            { OSQLParseNode::set_fct_spec, "set_fct_spec" },
            { OSQLParseNode::boolean_term, "boolean_term" },
            { OSQLParseNode::boolean_primary, "boolean_primary" },
            { OSQLParseNode::num_value_exp, "num_value_exp" },
            { OSQLParseNode::join_type, "join_type" },
            { OSQLParseNode::position_exp, "position_exp" },
            { OSQLParseNode::extract_exp, "extract_exp" },
            { OSQLParseNode::length_exp, "length_exp" },
            { OSQLParseNode::char_value_fct, "char_value_fct" },
            { OSQLParseNode::odbc_call_spec, "odbc_call_spec" },
            { OSQLParseNode::in_predicate, "in_predicate" },
            { OSQLParseNode::existence_test, "existence_test" },
            { OSQLParseNode::unique_test, "unique_test" },
            { OSQLParseNode::all_or_any_predicate, "all_or_any_predicate" },
            { OSQLParseNode::named_columns_join, "named_columns_join" },
            { OSQLParseNode::join_condition, "join_condition" },
            { OSQLParseNode::joined_table, "joined_table" },
            { OSQLParseNode::boolean_factor, "boolean_factor" },
            { OSQLParseNode::sql_not, "sql_not" },
            { OSQLParseNode::boolean_test, "boolean_test" },
            { OSQLParseNode::manipulative_statement, "manipulative_statement" },
            { OSQLParseNode::subquery, "subquery" },
            { OSQLParseNode::value_exp_commalist, "value_exp_commalist" },
            { OSQLParseNode::odbc_fct_spec, "odbc_fct_spec" },
            { OSQLParseNode::union_statement, "union_statement" },
            { OSQLParseNode::outer_join_type, "outer_join_type" },
            { OSQLParseNode::char_value_exp, "char_value_exp" },
            { OSQLParseNode::term, "term" },
            { OSQLParseNode::value_exp_primary, "value_exp_primary" },
            { OSQLParseNode::value_exp, "value_exp" },
            { OSQLParseNode::selection, "selection" },
            { OSQLParseNode::fold, "fold" },
            { OSQLParseNode::char_substring_fct, "char_substring_fct" },
            { OSQLParseNode::factor, "factor" },
            { OSQLParseNode::base_table_def, "base_table_def" },
            { OSQLParseNode::base_table_element_commalist, "base_table_element_commalist" },
            { OSQLParseNode::data_type, "data_type" },
            { OSQLParseNode::column_def, "column_def" },
            { OSQLParseNode::table_node, "table_node" },
            { OSQLParseNode::as, "as" },
            { OSQLParseNode::op_column_commalist, "op_column_commalist" }
        };
        size_t nRuleMapCount = sizeof( aRuleDescriptions ) / sizeof( aRuleDescriptions[0] );
        OSL_ENSURE( nRuleMapCount == size_t( OSQLParseNode::rule_count ), "OSQLParser::OSQLParser: added a new rule? Adjust this map!" );

        for ( size_t mapEntry = 0; mapEntry < nRuleMapCount; ++mapEntry )
        {
            // look up the rule description in the our identifier map
            sal_uInt32 nParserRuleID = StrToRuleID( aRuleDescriptions[ mapEntry ].sRuleName );
            // map the parser's rule ID to the OSQLParseNode::Rule
            s_aReverseRuleIDLookup[ nParserRuleID ] = aRuleDescriptions[ mapEntry ].eRule;
            // and map the OSQLParseNode::Rule to the parser's rule ID
            s_nRuleIDs[ aRuleDescriptions[ mapEntry ].eRule ] = nParserRuleID;
        }
    }
    ++s_nRefCount;

    if (m_pContext == NULL)
        // take the default context
        m_pContext = &s_aDefaultContext;

    m_pData->aLocale = m_pContext->getPreferredLocale();
}

//-----------------------------------------------------------------------------
OSQLParser::~OSQLParser()
{
    {
        ::osl::MutexGuard aGuard(getMutex());
        OSL_ENSURE(s_nRefCount > 0, "OSQLParser::~OSQLParser() : suspicious call : have a refcount of 0 !");
        if (!--s_nRefCount)
        {
            s_pScanner->setScanner(sal_True);
            delete s_pScanner;
            s_pScanner = NULL;

            delete s_pGarbageCollector;
            s_pGarbageCollector = NULL;
            // is only set the first time so we should delete it only when there no more instances
            s_xLocaleData = NULL;

            RuleIDMap aEmpty;
            s_aReverseRuleIDLookup.swap( aEmpty );
        }
        m_pParseTree = NULL;
    }
}
// -----------------------------------------------------------------------------
void OSQLParseNode::substituteParameterNames(OSQLParseNode* _pNode)
{
    sal_Int32 nCount = _pNode->count();
    for(sal_Int32 i=0;i < nCount;++i)
    {
        OSQLParseNode* pChildNode = _pNode->getChild(i);
        if(SQL_ISRULE(pChildNode,parameter) && pChildNode->count() > 1)
        {
            OSQLParseNode* pNewNode = new OSQLParseNode(::rtl::OUString::createFromAscii("?") ,SQL_NODE_PUNCTUATION,0);
            delete pChildNode->replace(pChildNode->getChild(0),pNewNode);
            sal_Int32 nChildCount = pChildNode->count();
            for(sal_Int32 j=1;j < nChildCount;++j)
                delete pChildNode->removeAt(1);
        }
        else
            substituteParameterNames(pChildNode);

    }
}
// -----------------------------------------------------------------------------
bool OSQLParser::extractDate(OSQLParseNode* pLiteral,double& _rfValue)
{
    Reference< XNumberFormatsSupplier > xFormatSup = m_xFormatter->getNumberFormatsSupplier();
    Reference< XNumberFormatTypes > xFormatTypes;
    if ( xFormatSup.is() )
        xFormatTypes = xFormatTypes.query( xFormatSup->getNumberFormats() );

    // if there is no format key, yet, make sure we have a feasible one for our locale
    try
    {
        if ( !m_nFormatKey && xFormatTypes.is() )
            m_nFormatKey = ::dbtools::getDefaultNumberFormat( m_xField, xFormatTypes, m_pData->aLocale );
    }
    catch( Exception& ) { }
    ::rtl::OUString sValue = pLiteral->getTokenValue();
    sal_Int32 nTryFormat = m_nFormatKey;
    bool bSuccess = lcl_saveConvertToNumber( m_xFormatter, nTryFormat, sValue, _rfValue );

    // If our format key didn't do, try the default date format for our locale.
    if ( !bSuccess && xFormatTypes.is() )
    {
        try
        {
            nTryFormat = xFormatTypes->getStandardFormat( NumberFormat::DATE, m_pData->aLocale );
        }
        catch( Exception& ) { }
        bSuccess = lcl_saveConvertToNumber( m_xFormatter, nTryFormat, sValue, _rfValue );
    }

    // if this also didn't do, try ISO format
    if ( !bSuccess && xFormatTypes.is() )
    {
        try
        {
            nTryFormat = xFormatTypes->getFormatIndex( NumberFormatIndex::DATE_DIN_YYYYMMDD, m_pData->aLocale );
        }
        catch( Exception& ) { }
        bSuccess = lcl_saveConvertToNumber( m_xFormatter, nTryFormat, sValue, _rfValue );
    }

    // if this also didn't do, try fallback date format (en-US)
    if ( !bSuccess )
    {
        nTryFormat = m_nDateFormatKey;
        bSuccess = lcl_saveConvertToNumber( m_xFormatter, nTryFormat, sValue, _rfValue );
    }
    return bSuccess;
}
// -----------------------------------------------------------------------------
OSQLParseNode* OSQLParser::buildDate(sal_Int32 _nType,OSQLParseNode*& pLiteral)
{
    // try converting the string into a date, according to our format key
    double fValue = 0.0;
    OSQLParseNode* pFCTNode = NULL;

    if ( extractDate(pLiteral,fValue) )
        pFCTNode = buildNode_Date( fValue, _nType);

    delete pLiteral;
    pLiteral = NULL;

    if ( !pFCTNode )
        m_sErrorMessage = m_pContext->getErrorMessage(IParseContext::ERROR_INVALID_DATE_COMPARE);

    return pFCTNode;
}
// -----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
OSQLParseNode::OSQLParseNode(const sal_Char * pNewValue,
                             SQLNodeType eNewNodeType,
                             sal_uInt32 nNewNodeID)
        :m_pParent(NULL)
        ,m_aNodeValue(pNewValue,strlen(pNewValue),RTL_TEXTENCODING_UTF8)
        ,m_eNodeType(eNewNodeType)
        ,m_nNodeID(nNewNodeID)
{

    OSL_ENSURE(m_eNodeType >= SQL_NODE_RULE && m_eNodeType <= SQL_NODE_CONCAT,"OSQLParseNode: mit unzulaessigem NodeType konstruiert");
}
//-----------------------------------------------------------------------------
OSQLParseNode::OSQLParseNode(const ::rtl::OString &_rNewValue,
                             SQLNodeType eNewNodeType,
                             sal_uInt32 nNewNodeID)
        :m_pParent(NULL)
        ,m_aNodeValue(_rNewValue,_rNewValue.getLength(),RTL_TEXTENCODING_UTF8)
        ,m_eNodeType(eNewNodeType)
        ,m_nNodeID(nNewNodeID)
{

    OSL_ENSURE(m_eNodeType >= SQL_NODE_RULE && m_eNodeType <= SQL_NODE_CONCAT,"OSQLParseNode: mit unzulaessigem NodeType konstruiert");
}
//-----------------------------------------------------------------------------
OSQLParseNode::OSQLParseNode(const sal_Unicode * pNewValue,
                                 SQLNodeType eNewNodeType,
                                 sal_uInt32 nNewNodeID)
        :m_pParent(NULL)
        ,m_aNodeValue(pNewValue)
        ,m_eNodeType(eNewNodeType)
        ,m_nNodeID(nNewNodeID)
{

    OSL_ENSURE(m_eNodeType >= SQL_NODE_RULE && m_eNodeType <= SQL_NODE_CONCAT,"OSQLParseNode: mit unzulaessigem NodeType konstruiert");
}
//-----------------------------------------------------------------------------
OSQLParseNode::OSQLParseNode(const ::rtl::OUString &_rNewValue,
                                 SQLNodeType eNewNodeType,
                                 sal_uInt32 nNewNodeID)
        :m_pParent(NULL)
        ,m_aNodeValue(_rNewValue)
        ,m_eNodeType(eNewNodeType)
        ,m_nNodeID(nNewNodeID)
{

    OSL_ENSURE(m_eNodeType >= SQL_NODE_RULE && m_eNodeType <= SQL_NODE_CONCAT,"OSQLParseNode: mit unzulaessigem NodeType konstruiert");
}
//-----------------------------------------------------------------------------
OSQLParseNode::OSQLParseNode(const OSQLParseNode& rParseNode)
{

    // klemm den getParent auf NULL
    m_pParent = NULL;

    // kopiere die member
    m_aNodeValue = rParseNode.m_aNodeValue;
    m_eNodeType  = rParseNode.m_eNodeType;
    m_nNodeID    = rParseNode.m_nNodeID;


    // denk dran, dass von Container abgeleitet wurde, laut SV-Help erzeugt
    // copy-Constructor des Containers einen neuen Container mit den gleichen
    // Zeigern als Inhalt -> d.h. nach dem Kopieren des Container wird fuer
    // alle Zeiger ungleich NULL eine Kopie hergestellt und anstelle des alten
    // Zeigers wieder eingehangen.

    // wenn kein Blatt, dann SubTrees bearbeiten
    for (OSQLParseNodes::const_iterator i = rParseNode.m_aChilds.begin();
         i != rParseNode.m_aChilds.end(); i++)
        append(new OSQLParseNode(**i));
}
// -----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
OSQLParseNode& OSQLParseNode::operator=(const OSQLParseNode& rParseNode)
{
    if (this != &rParseNode)
    {
        // kopiere die member - pParent bleibt der alte
        m_aNodeValue = rParseNode.m_aNodeValue;
        m_eNodeType  = rParseNode.m_eNodeType;
        m_nNodeID    = rParseNode.m_nNodeID;

        for (OSQLParseNodes::const_iterator i = m_aChilds.begin();
            i != m_aChilds.end(); i++)
            delete *i;

        m_aChilds.clear();

        for (OSQLParseNodes::const_iterator j = rParseNode.m_aChilds.begin();
             j != rParseNode.m_aChilds.end(); j++)
            append(new OSQLParseNode(**j));
    }
    return *this;
}

//-----------------------------------------------------------------------------
sal_Bool OSQLParseNode::operator==(OSQLParseNode& rParseNode) const
{
    // die member muessen gleich sein
    sal_Bool bResult = (m_nNodeID  == rParseNode.m_nNodeID) &&
                   (m_eNodeType == rParseNode.m_eNodeType) &&
                   (m_aNodeValue == rParseNode.m_aNodeValue) &&
                    count() == rParseNode.count();

    // Parameters are not equal!
    bResult = bResult && !SQL_ISRULE(this, parameter);

    // compare childs
    for (sal_uInt32 i=0; bResult && i < count(); i++)
        bResult = *getChild(i) == *rParseNode.getChild(i);

    return bResult;
}

//-----------------------------------------------------------------------------
OSQLParseNode::~OSQLParseNode()
{
    for (OSQLParseNodes::const_iterator i = m_aChilds.begin();
         i != m_aChilds.end(); i++)
        delete *i;
    m_aChilds.clear();
}

//-----------------------------------------------------------------------------
void OSQLParseNode::append(OSQLParseNode* pNewNode)
{

    OSL_ENSURE(pNewNode != NULL, "OSQLParseNode: ungueltiger NewSubTree");
    OSL_ENSURE(pNewNode->getParent() == NULL, "OSQLParseNode: Knoten ist kein Waise");
    OSL_ENSURE(::std::find(m_aChilds.begin(), m_aChilds.end(), pNewNode) == m_aChilds.end(),
            "OSQLParseNode::append() Node already element of parent");

    // stelle Verbindung zum getParent her:
    pNewNode->setParent( this );
    // und haenge den SubTree hinten an
    m_aChilds.push_back(pNewNode);
}
// -----------------------------------------------------------------------------
sal_Bool OSQLParseNode::addDateValue(::rtl::OUString& rString, const SQLParseNodeParameter& rParam) const
{
    // special display for date/time values
    if (SQL_ISRULE(this,set_fct_spec) && SQL_ISPUNCTUATION(m_aChilds[0],"{"))
    {
        const OSQLParseNode* pODBCNode = m_aChilds[1];
        const OSQLParseNode* pODBCNodeChild = pODBCNode->m_aChilds[0];

        if (pODBCNodeChild->getNodeType() == SQL_NODE_KEYWORD && (
            SQL_ISTOKEN(pODBCNodeChild, D) ||
            SQL_ISTOKEN(pODBCNodeChild, T) ||
            SQL_ISTOKEN(pODBCNodeChild, TS) ))
        {
            ::rtl::OUString suQuote(::rtl::OUString::createFromAscii("'"));
            if (rParam.bPredicate)
            {
                 if (rParam.aMetaData.shouldEscapeDateTime())
                 {
                     suQuote = ::rtl::OUString::createFromAscii("#");
                 }
                 else
                 {
                     suQuote = ::rtl::OUString::createFromAscii("'");
                 }
            }
            else
            {
                 if (rParam.aMetaData.shouldEscapeDateTime())
                 {
                     // suQuote = ::rtl::OUString::createFromAscii("'");
                     return sal_False;
                 }
                 else
                 {
                     suQuote = ::rtl::OUString::createFromAscii("'");
                 }
            }

            if (rString.getLength())
                rString += ::rtl::OUString::createFromAscii(" ");
            rString += suQuote;
            const ::rtl::OUString sTokenValue = pODBCNode->m_aChilds[1]->getTokenValue();
            if (SQL_ISTOKEN(pODBCNodeChild, D))
            {
                rString += rParam.bPredicate ? convertDateString(rParam, sTokenValue) : sTokenValue;
            }
            else if (SQL_ISTOKEN(pODBCNodeChild, T))
            {
                rString += rParam.bPredicate ? convertTimeString(rParam, sTokenValue) : sTokenValue;
            }
            else
            {
                rString += rParam.bPredicate ? convertDateTimeString(rParam, sTokenValue) : sTokenValue;
            }
            rString += suQuote;
            return sal_True;
        }
    }
    return sal_False;
}
// -----------------------------------------------------------------------------
void OSQLParseNode::replaceNodeValue(const ::rtl::OUString& rTableAlias,const ::rtl::OUString& rColumnName)
{
    for (sal_uInt32 i=0;i<count();++i)
    {
        if (SQL_ISRULE(this,column_ref) && count() == 1 && getChild(0)->getTokenValue() == rColumnName)
        {
            OSQLParseNode * pCol = removeAt((sal_uInt32)0);
            append(new OSQLParseNode(rTableAlias,SQL_NODE_NAME));
            append(new OSQLParseNode(::rtl::OUString::createFromAscii("."),SQL_NODE_PUNCTUATION));
            append(pCol);
        }
        else
            getChild(i)->replaceNodeValue(rTableAlias,rColumnName);
    }
}
//-----------------------------------------------------------------------------
OSQLParseNode* OSQLParseNode::getByRule(OSQLParseNode::Rule eRule) const
{
    OSQLParseNode* pRetNode = 0;
    if (isRule() && OSQLParser::RuleID(eRule) == getRuleID())
        pRetNode = (OSQLParseNode*)this;
    else
    {
        for (OSQLParseNodes::const_iterator i = m_aChilds.begin();
            !pRetNode && i != m_aChilds.end(); i++)
            pRetNode = (*i)->getByRule(eRule);
    }
    return pRetNode;
}
//-----------------------------------------------------------------------------
OSQLParseNode* MakeANDNode(OSQLParseNode *pLeftLeaf,OSQLParseNode *pRightLeaf)
{
    OSQLParseNode* pNewNode = new OSQLParseNode(::rtl::OUString(),SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::boolean_term));
    pNewNode->append(pLeftLeaf);
    pNewNode->append(new OSQLParseNode(::rtl::OUString::createFromAscii("AND"),SQL_NODE_KEYWORD,SQL_TOKEN_AND));
    pNewNode->append(pRightLeaf);
    return pNewNode;
}
//-----------------------------------------------------------------------------
OSQLParseNode* MakeORNode(OSQLParseNode *pLeftLeaf,OSQLParseNode *pRightLeaf)
{
    OSQLParseNode* pNewNode = new OSQLParseNode(::rtl::OUString(),SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::search_condition));
    pNewNode->append(pLeftLeaf);
    pNewNode->append(new OSQLParseNode(::rtl::OUString::createFromAscii("OR"),SQL_NODE_KEYWORD,SQL_TOKEN_OR));
    pNewNode->append(pRightLeaf);
    return pNewNode;
}
//-----------------------------------------------------------------------------
void OSQLParseNode::disjunctiveNormalForm(OSQLParseNode*& pSearchCondition)
{
    if(!pSearchCondition) // no where condition at entry point
        return;

    OSQLParseNode::absorptions(pSearchCondition);
    // '(' search_condition ')'
    if (SQL_ISRULE(pSearchCondition,boolean_primary))
    {
        OSQLParseNode* pLeft    = pSearchCondition->getChild(1);
        disjunctiveNormalForm(pLeft);
    }
    // search_condition SQL_TOKEN_OR boolean_term
    else if (SQL_ISRULE(pSearchCondition,search_condition))
    {
        OSQLParseNode* pLeft    = pSearchCondition->getChild(0);
        disjunctiveNormalForm(pLeft);

        OSQLParseNode* pRight = pSearchCondition->getChild(2);
        disjunctiveNormalForm(pRight);
    }
    // boolean_term SQL_TOKEN_AND boolean_factor
    else if (SQL_ISRULE(pSearchCondition,boolean_term))
    {
        OSQLParseNode* pLeft    = pSearchCondition->getChild(0);
        disjunctiveNormalForm(pLeft);

        OSQLParseNode* pRight = pSearchCondition->getChild(2);
        disjunctiveNormalForm(pRight);

        OSQLParseNode* pNewNode = NULL;
        // '(' search_condition ')' on left side
        if(pLeft->count() == 3 && SQL_ISRULE(pLeft,boolean_primary) && SQL_ISRULE(pLeft->getChild(1),search_condition))
        {
            // and-or tree  on left side
            OSQLParseNode* pOr = pLeft->getChild(1);
            OSQLParseNode* pNewLeft = NULL;
            OSQLParseNode* pNewRight = NULL;

            // cut right from parent
            pSearchCondition->removeAt(2);

            pNewRight   = MakeANDNode(pOr->removeAt(2)      ,pRight);
            pNewLeft    = MakeANDNode(pOr->removeAt((sal_uInt32)0)  ,new OSQLParseNode(*pRight));
            pNewNode    = MakeORNode(pNewLeft,pNewRight);
            // and append new Node
            replaceAndReset(pSearchCondition,pNewNode);

            disjunctiveNormalForm(pSearchCondition);
        }
        else if(pRight->count() == 3 && SQL_ISRULE(pRight,boolean_primary) && SQL_ISRULE(pRight->getChild(1),search_condition))
        {   // '(' search_condition ')' on right side
            // and-or tree  on right side
            // a and (b or c)
            OSQLParseNode* pOr = pRight->getChild(1);
            OSQLParseNode* pNewLeft = NULL;
            OSQLParseNode* pNewRight = NULL;

            // cut left from parent
            pSearchCondition->removeAt((sal_uInt32)0);

            pNewRight   = MakeANDNode(pLeft,pOr->removeAt(2));
            pNewLeft    = MakeANDNode(new OSQLParseNode(*pLeft),pOr->removeAt((sal_uInt32)0));
            pNewNode    = MakeORNode(pNewLeft,pNewRight);

            // and append new Node
            replaceAndReset(pSearchCondition,pNewNode);
            disjunctiveNormalForm(pSearchCondition);
        }
        else if(SQL_ISRULE(pLeft,boolean_primary) && (!SQL_ISRULE(pLeft->getChild(1),search_condition) || !SQL_ISRULE(pLeft->getChild(1),boolean_term)))
            pSearchCondition->replace(pLeft, pLeft->removeAt(1));
        else if(SQL_ISRULE(pRight,boolean_primary) && (!SQL_ISRULE(pRight->getChild(1),search_condition) || !SQL_ISRULE(pRight->getChild(1),boolean_term)))
            pSearchCondition->replace(pRight, pRight->removeAt(1));
    }
}
//-----------------------------------------------------------------------------
void OSQLParseNode::negateSearchCondition(OSQLParseNode*& pSearchCondition,sal_Bool bNegate)
{
    if(!pSearchCondition) // no where condition at entry point
        return;
    // '(' search_condition ')'
    if (pSearchCondition->count() == 3 && SQL_ISRULE(pSearchCondition,boolean_primary))
    {
        OSQLParseNode* pRight = pSearchCondition->getChild(1);
        negateSearchCondition(pRight,bNegate);
    }
    // search_condition SQL_TOKEN_OR boolean_term
    else if (SQL_ISRULE(pSearchCondition,search_condition))
    {
        OSQLParseNode* pLeft    = pSearchCondition->getChild(0);
        OSQLParseNode* pRight = pSearchCondition->getChild(2);
        if(bNegate)
        {
            OSQLParseNode* pNewNode = new OSQLParseNode(::rtl::OUString(),SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::boolean_term));
            pNewNode->append(pSearchCondition->removeAt((sal_uInt32)0));
            pNewNode->append(new OSQLParseNode(::rtl::OUString::createFromAscii("AND"),SQL_NODE_KEYWORD,SQL_TOKEN_AND));
            pNewNode->append(pSearchCondition->removeAt((sal_uInt32)1));
            replaceAndReset(pSearchCondition,pNewNode);

            pLeft   = pNewNode->getChild(0);
            pRight  = pNewNode->getChild(2);
        }

        negateSearchCondition(pLeft,bNegate);
        negateSearchCondition(pRight,bNegate);
    }
    // boolean_term SQL_TOKEN_AND boolean_factor
    else if (SQL_ISRULE(pSearchCondition,boolean_term))
    {
        OSQLParseNode* pLeft    = pSearchCondition->getChild(0);
        OSQLParseNode* pRight = pSearchCondition->getChild(2);
        if(bNegate)
        {
            OSQLParseNode* pNewNode = new OSQLParseNode(::rtl::OUString(),SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::search_condition));
            pNewNode->append(pSearchCondition->removeAt((sal_uInt32)0));
            pNewNode->append(new OSQLParseNode(::rtl::OUString::createFromAscii("OR"),SQL_NODE_KEYWORD,SQL_TOKEN_OR));
            pNewNode->append(pSearchCondition->removeAt((sal_uInt32)1));
            replaceAndReset(pSearchCondition,pNewNode);

            pLeft   = pNewNode->getChild(0);
            pRight  = pNewNode->getChild(2);
        }

        negateSearchCondition(pLeft,bNegate);
        negateSearchCondition(pRight,bNegate);
    }
    // SQL_TOKEN_NOT ( boolean_test )
    else if (SQL_ISRULE(pSearchCondition,boolean_factor))
    {
        OSQLParseNode *pNot = pSearchCondition->removeAt((sal_uInt32)0);
        delete pNot;
        OSQLParseNode *pBooleanTest = pSearchCondition->removeAt((sal_uInt32)0);
        pBooleanTest->setParent(NULL);
        replaceAndReset(pSearchCondition,pBooleanTest);

        if (!bNegate)
            negateSearchCondition(pSearchCondition,sal_True);   //  negate all deeper values
    }
    // row_value_constructor comparison row_value_constructor
    // row_value_constructor comparison any_all_some subquery
    else if(bNegate  && SQL_ISRULE(pSearchCondition,comparison_predicate) || SQL_ISRULE(pSearchCondition,all_or_any_predicate))
    {
        OSQLParseNode* pComparison = pSearchCondition->getChild(1);
        OSQLParseNode* pNewComparison = NULL;
        switch(pComparison->getNodeType())
        {
            case SQL_NODE_EQUAL:
                pNewComparison = new OSQLParseNode(::rtl::OUString::createFromAscii("<>"),SQL_NODE_NOTEQUAL,SQL_NOTEQUAL);
                break;
            case SQL_NODE_LESS:
                pNewComparison = new OSQLParseNode(::rtl::OUString::createFromAscii(">="),SQL_NODE_GREATEQ,SQL_GREATEQ);
                break;
            case SQL_NODE_GREAT:
                pNewComparison = new OSQLParseNode(::rtl::OUString::createFromAscii("<="),SQL_NODE_LESSEQ,SQL_LESSEQ);
                break;
            case SQL_NODE_LESSEQ:
                pNewComparison = new OSQLParseNode(::rtl::OUString::createFromAscii(">"),SQL_NODE_GREAT,SQL_GREAT);
                break;
            case SQL_NODE_GREATEQ:
                pNewComparison = new OSQLParseNode(::rtl::OUString::createFromAscii("<"),SQL_NODE_LESS,SQL_LESS);
                break;
            case SQL_NODE_NOTEQUAL:
                pNewComparison = new OSQLParseNode(::rtl::OUString::createFromAscii("="),SQL_NODE_EQUAL,SQL_EQUAL);
                break;
            default:
                OSL_ENSURE( false, "OSQLParseNode::negateSearchCondition: unexpected node type!" );
                break;
        }
        pSearchCondition->replace(pComparison, pNewComparison);
        delete pComparison;
    }

    else if(bNegate && (SQL_ISRULE(pSearchCondition,test_for_null) || SQL_ISRULE(pSearchCondition,in_predicate) ||
                        SQL_ISRULE(pSearchCondition,like_predicate) || SQL_ISRULE(pSearchCondition,between_predicate) ||
                        SQL_ISRULE(pSearchCondition,boolean_test) ))
    {
        sal_uInt32 nNotPos = 0;
        // row_value_constructor not SQL_TOKEN_IN in_predicate_value
        // row_value_constructor not SQL_TOKEN_LIKE num_value_exp opt_escape
        // row_value_constructor not SQL_TOKEN_BETWEEN row_value_constructor SQL_TOKEN_AND row_value_constructor
        if(SQL_ISRULE(pSearchCondition,in_predicate) || SQL_ISRULE(pSearchCondition,like_predicate) ||
           SQL_ISRULE(pSearchCondition,between_predicate))
            nNotPos = 1;
        // row_value_constructor SQL_TOKEN_IS not SQL_TOKEN_NULL
        // boolean_primary SQL_TOKEN_IS not truth_value
        else if(SQL_ISRULE(pSearchCondition,test_for_null) || SQL_ISRULE(pSearchCondition,boolean_test))
            nNotPos = 2;

        OSQLParseNode* pNot = pSearchCondition->getChild(nNotPos);
        OSQLParseNode* pNotNot = NULL;
        if(pNot->isRule())
            pNotNot = new OSQLParseNode(::rtl::OUString::createFromAscii("NOT"),SQL_NODE_KEYWORD,SQL_TOKEN_NOT);
        else
            pNotNot = new OSQLParseNode(::rtl::OUString(),SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::sql_not));
        pSearchCondition->replace(pNot, pNotNot);
        delete pNot;
    }
}
//-----------------------------------------------------------------------------
void OSQLParseNode::eraseBraces(OSQLParseNode*& pSearchCondition)
{
    if (pSearchCondition && (SQL_ISRULE(pSearchCondition,boolean_primary) || (pSearchCondition->count() == 3 && SQL_ISPUNCTUATION(pSearchCondition->getChild(0),"(") &&
         SQL_ISPUNCTUATION(pSearchCondition->getChild(2),")"))))
    {
        OSQLParseNode* pRight = pSearchCondition->getChild(1);
        absorptions(pRight);
        // if child is not a or or and tree then delete () around child
        if(!(SQL_ISRULE(pSearchCondition->getChild(1),boolean_term) || SQL_ISRULE(pSearchCondition->getChild(1),search_condition)) ||
            SQL_ISRULE(pSearchCondition->getChild(1),boolean_term) || // and can always stand without ()
            (SQL_ISRULE(pSearchCondition->getChild(1),search_condition) && SQL_ISRULE(pSearchCondition->getParent(),search_condition)))
        {
            OSQLParseNode* pNode = pSearchCondition->removeAt(1);
            replaceAndReset(pSearchCondition,pNode);
        }
    }
}
//-----------------------------------------------------------------------------
void OSQLParseNode::absorptions(OSQLParseNode*& pSearchCondition)
{
    if(!pSearchCondition) // no where condition at entry point
        return;

    eraseBraces(pSearchCondition);

    if(SQL_ISRULE(pSearchCondition,boolean_term) || SQL_ISRULE(pSearchCondition,search_condition))
    {
        OSQLParseNode* pLeft = pSearchCondition->getChild(0);
        absorptions(pLeft);
        OSQLParseNode* pRight = pSearchCondition->getChild(2);
        absorptions(pRight);
    }

    sal_uInt32 nPos = 0;
    // a and a || a or a
    OSQLParseNode* pNewNode = NULL;
    if(( SQL_ISRULE(pSearchCondition,boolean_term) || SQL_ISRULE(pSearchCondition,search_condition))
        && *pSearchCondition->getChild(0) == *pSearchCondition->getChild(2))
    {
        pNewNode = pSearchCondition->removeAt((sal_uInt32)0);
        replaceAndReset(pSearchCondition,pNewNode);
    }
    // (a or b) and a || ( b or c ) and a
    // a and ( a or b) || a and ( b or c )
    else if (   SQL_ISRULE(pSearchCondition,boolean_term)
            &&  (
                    (       SQL_ISRULE(pSearchCondition->getChild(nPos = 0),boolean_primary)
                        ||  SQL_ISRULE(pSearchCondition->getChild(nPos),search_condition)
                    )
                ||  (       SQL_ISRULE(pSearchCondition->getChild(nPos = 2),boolean_primary)
                        ||  SQL_ISRULE(pSearchCondition->getChild(nPos),search_condition)
                    )
                )
            )
    {
        OSQLParseNode* p2ndSearch = pSearchCondition->getChild(nPos);
        if ( SQL_ISRULE(p2ndSearch,boolean_primary) )
            p2ndSearch = p2ndSearch->getChild(1);

        if ( *p2ndSearch->getChild(0) == *pSearchCondition->getChild(2-nPos) )
        {
            pNewNode = pSearchCondition->removeAt((sal_uInt32)0);
            replaceAndReset(pSearchCondition,pNewNode);

        }
        else if ( *p2ndSearch->getChild(2) == *pSearchCondition->getChild(2-nPos) )
        {
            pNewNode = pSearchCondition->removeAt((sal_uInt32)2);
            replaceAndReset(pSearchCondition,pNewNode);
        }
        else if ( p2ndSearch->getByRule(OSQLParseNode::boolean_term) )
        {
            // a and ( b or c ) -> ( a and b ) or ( a and c )
            // ( b or c ) and a -> ( a and b ) or ( a and c )
            OSQLParseNode* pC = p2ndSearch->removeAt((sal_uInt32)2);
            OSQLParseNode* pB = p2ndSearch->removeAt((sal_uInt32)0);
            OSQLParseNode* pA = pSearchCondition->removeAt((sal_uInt32)2-nPos);

            OSQLParseNode* p1stAnd = MakeANDNode(pA,pB);
            OSQLParseNode* p2ndAnd = MakeANDNode(new OSQLParseNode(*pA),pC);
            pNewNode = MakeORNode(p1stAnd,p2ndAnd);
            replaceAndReset(pSearchCondition,pNewNode);
        }
    }
    // a or a and b || a or b and a
    else if(SQL_ISRULE(pSearchCondition,search_condition) && SQL_ISRULE(pSearchCondition->getChild(2),boolean_term))
    {
        if(*pSearchCondition->getChild(2)->getChild(0) == *pSearchCondition->getChild(0))
        {
            pNewNode = pSearchCondition->removeAt((sal_uInt32)0);
            replaceAndReset(pSearchCondition,pNewNode);
        }
        else if(*pSearchCondition->getChild(2)->getChild(2) == *pSearchCondition->getChild(0))
        {
            pNewNode = pSearchCondition->removeAt((sal_uInt32)0);
            replaceAndReset(pSearchCondition,pNewNode);
        }
    }
    // a and b or a || b and a or a
    else if(SQL_ISRULE(pSearchCondition,search_condition) && SQL_ISRULE(pSearchCondition->getChild(0),boolean_term))
    {
        if(*pSearchCondition->getChild(0)->getChild(0) == *pSearchCondition->getChild(2))
        {
            pNewNode = pSearchCondition->removeAt((sal_uInt32)2);
            replaceAndReset(pSearchCondition,pNewNode);
        }
        else if(*pSearchCondition->getChild(0)->getChild(2) == *pSearchCondition->getChild(2))
        {
            pNewNode = pSearchCondition->removeAt((sal_uInt32)2);
            replaceAndReset(pSearchCondition,pNewNode);
        }
    }
    eraseBraces(pSearchCondition);
}
//-----------------------------------------------------------------------------
void OSQLParseNode::compress(OSQLParseNode *&pSearchCondition)
{
    if(!pSearchCondition) // no where condition at entry point
        return;

    OSQLParseNode::eraseBraces(pSearchCondition);

    if(SQL_ISRULE(pSearchCondition,boolean_term) || SQL_ISRULE(pSearchCondition,search_condition))
    {
        OSQLParseNode* pLeft = pSearchCondition->getChild(0);
        compress(pLeft);

        OSQLParseNode* pRight = pSearchCondition->getChild(2);
        compress(pRight);
    }
    else if( SQL_ISRULE(pSearchCondition,boolean_primary) || (pSearchCondition->count() == 3 && SQL_ISPUNCTUATION(pSearchCondition->getChild(0),"(") &&
             SQL_ISPUNCTUATION(pSearchCondition->getChild(2),")")))
    {
        OSQLParseNode* pRight = pSearchCondition->getChild(1);
        compress(pRight);
        // if child is not a or or and tree then delete () around child
        if(!(SQL_ISRULE(pSearchCondition->getChild(1),boolean_term) || SQL_ISRULE(pSearchCondition->getChild(1),search_condition)) ||
            (SQL_ISRULE(pSearchCondition->getChild(1),boolean_term) && SQL_ISRULE(pSearchCondition->getParent(),boolean_term)) ||
            (SQL_ISRULE(pSearchCondition->getChild(1),search_condition) && SQL_ISRULE(pSearchCondition->getParent(),search_condition)))
        {
            OSQLParseNode* pNode = pSearchCondition->removeAt(1);
            replaceAndReset(pSearchCondition,pNode);
        }
    }

    // or with two and trees where one element of the and trees are equal
    if(SQL_ISRULE(pSearchCondition,search_condition) && SQL_ISRULE(pSearchCondition->getChild(0),boolean_term) && SQL_ISRULE(pSearchCondition->getChild(2),boolean_term))
    {
        if(*pSearchCondition->getChild(0)->getChild(0) == *pSearchCondition->getChild(2)->getChild(0))
        {
            OSQLParseNode* pLeft    = pSearchCondition->getChild(0)->removeAt(2);
            OSQLParseNode* pRight = pSearchCondition->getChild(2)->removeAt(2);
            OSQLParseNode* pNode    = MakeORNode(pLeft,pRight);

            OSQLParseNode* pNewRule = new OSQLParseNode(::rtl::OUString(),SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::boolean_primary));
            pNewRule->append(new OSQLParseNode(::rtl::OUString::createFromAscii("("),SQL_NODE_PUNCTUATION));
            pNewRule->append(pNode);
            pNewRule->append(new OSQLParseNode(::rtl::OUString::createFromAscii(")"),SQL_NODE_PUNCTUATION));

            OSQLParseNode::eraseBraces(pLeft);
            OSQLParseNode::eraseBraces(pRight);

            pNode = MakeANDNode(pSearchCondition->getChild(0)->removeAt((sal_uInt32)0),pNewRule);
            replaceAndReset(pSearchCondition,pNode);
        }
        else if(*pSearchCondition->getChild(0)->getChild(2) == *pSearchCondition->getChild(2)->getChild(0))
        {
            OSQLParseNode* pLeft = pSearchCondition->getChild(0)->removeAt((sal_uInt32)0);
            OSQLParseNode* pRight = pSearchCondition->getChild(2)->removeAt(2);
            OSQLParseNode* pNode = MakeORNode(pLeft,pRight);

            OSQLParseNode* pNewRule = new OSQLParseNode(::rtl::OUString(),SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::boolean_primary));
            pNewRule->append(new OSQLParseNode(::rtl::OUString::createFromAscii("("),SQL_NODE_PUNCTUATION));
            pNewRule->append(pNode);
            pNewRule->append(new OSQLParseNode(::rtl::OUString::createFromAscii(")"),SQL_NODE_PUNCTUATION));

            OSQLParseNode::eraseBraces(pLeft);
            OSQLParseNode::eraseBraces(pRight);

            pNode = MakeANDNode(pSearchCondition->getChild(0)->removeAt(1),pNewRule);
            replaceAndReset(pSearchCondition,pNode);
        }
        else if(*pSearchCondition->getChild(0)->getChild(0) == *pSearchCondition->getChild(2)->getChild(2))
        {
            OSQLParseNode* pLeft    = pSearchCondition->getChild(0)->removeAt(2);
            OSQLParseNode* pRight = pSearchCondition->getChild(2)->removeAt((sal_uInt32)0);
            OSQLParseNode* pNode    = MakeORNode(pLeft,pRight);

            OSQLParseNode* pNewRule = new OSQLParseNode(::rtl::OUString(),SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::boolean_primary));
            pNewRule->append(new OSQLParseNode(::rtl::OUString::createFromAscii("("),SQL_NODE_PUNCTUATION));
            pNewRule->append(pNode);
            pNewRule->append(new OSQLParseNode(::rtl::OUString::createFromAscii(")"),SQL_NODE_PUNCTUATION));

            OSQLParseNode::eraseBraces(pLeft);
            OSQLParseNode::eraseBraces(pRight);

            pNode = MakeANDNode(pSearchCondition->getChild(0)->removeAt((sal_uInt32)0),pNewRule);
            replaceAndReset(pSearchCondition,pNode);
        }
        else if(*pSearchCondition->getChild(0)->getChild(2) == *pSearchCondition->getChild(2)->getChild(2))
        {
            OSQLParseNode* pLeft    = pSearchCondition->getChild(0)->removeAt((sal_uInt32)0);
            OSQLParseNode* pRight = pSearchCondition->getChild(2)->removeAt((sal_uInt32)0);
            OSQLParseNode* pNode    = MakeORNode(pLeft,pRight);

            OSQLParseNode* pNewRule = new OSQLParseNode(::rtl::OUString(),SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::boolean_primary));
            pNewRule->append(new OSQLParseNode(::rtl::OUString::createFromAscii("("),SQL_NODE_PUNCTUATION));
            pNewRule->append(pNode);
            pNewRule->append(new OSQLParseNode(::rtl::OUString::createFromAscii(")"),SQL_NODE_PUNCTUATION));

            OSQLParseNode::eraseBraces(pLeft);
            OSQLParseNode::eraseBraces(pRight);

            pNode = MakeANDNode(pSearchCondition->getChild(0)->removeAt(1),pNewRule);
            replaceAndReset(pSearchCondition,pNode);
        }
    }
}
#if OSL_DEBUG_LEVEL > 1
// -----------------------------------------------------------------------------
void OSQLParseNode::showParseTree(::rtl::OUString& rString, sal_uInt32 nLevel)
{

    if (!isToken())
    {
        for (sal_uInt32 j=0; j<nLevel; j++) {rString+= ::rtl::OUString::createFromAscii("\t");};
        // Regelnamen als rule: ...
        rString+= ::rtl::OUString::createFromAscii("RULE_ID:\t ");
        rString += ::rtl::OUString::valueOf( (sal_Int32)getRuleID());
        rString+= ::rtl::OUString::createFromAscii("(");
        rString += OSQLParser::RuleIDToStr(getRuleID());
        rString+= ::rtl::OUString::createFromAscii(")");
        rString+= ::rtl::OUString::createFromAscii("\n");

        // hol dir den ersten Subtree
        for (OSQLParseNodes::const_iterator i = m_aChilds.begin();
            i != m_aChilds.end(); i++)
            (*i)->showParseTree(rString, nLevel+1);
    }
    else
    {
        // ein Token gefunden
        // tabs fuer das Einruecken entsprechend nLevel
        for (sal_uInt32 j=0; j<nLevel; j++) {rString+= ::rtl::OUString::createFromAscii("\t");};

        switch (m_eNodeType) {

        case SQL_NODE_KEYWORD:
            {rString+= ::rtl::OUString::createFromAscii("SQL_KEYWORD:\t");
            ::rtl::OString sT = OSQLParser::TokenIDToStr(getTokenID());
            rString += ::rtl::OUString(sT,sT.getLength(),RTL_TEXTENCODING_UTF8);
            rString+= ::rtl::OUString::createFromAscii("\n");
            break;}

        case SQL_NODE_COMPARISON:
            {rString+= ::rtl::OUString::createFromAscii("SQL_COMPARISON:\t");
            rString += m_aNodeValue;    // haenge Nodevalue an
            rString+= ::rtl::OUString::createFromAscii("\n");       // und beginne neu Zeile
            break;}

        case SQL_NODE_NAME:
            {rString+= ::rtl::OUString::createFromAscii("SQL_NAME:\t");
             rString+= ::rtl::OUString::createFromAscii("\"");
             rString += m_aNodeValue;
             rString+= ::rtl::OUString::createFromAscii("\"");
             rString+= ::rtl::OUString::createFromAscii("\n");
             break;}

        case SQL_NODE_STRING:
            {rString += ::rtl::OUString::createFromAscii("SQL_STRING:\t'");
             rString += m_aNodeValue;
             rString += ::rtl::OUString::createFromAscii("'\n");
             break;}

        case SQL_NODE_INTNUM:
            {rString += ::rtl::OUString::createFromAscii("SQL_INTNUM:\t");
             rString += m_aNodeValue;
             rString += ::rtl::OUString::createFromAscii("\n");
             break;}

        case SQL_NODE_APPROXNUM:
            {rString += ::rtl::OUString::createFromAscii("SQL_APPROXNUM:\t");
             rString += m_aNodeValue;
             rString += ::rtl::OUString::createFromAscii("\n");
             break;}

        case SQL_NODE_PUNCTUATION:
            {rString += ::rtl::OUString::createFromAscii("SQL_PUNCTUATION:\t");
            rString += m_aNodeValue;    // haenge Nodevalue an
            rString += ::rtl::OUString::createFromAscii("\n");      // und beginne neu Zeile
            break;}

        case SQL_NODE_AMMSC:
            {rString += ::rtl::OUString::createFromAscii("SQL_AMMSC:\t");
            rString += m_aNodeValue;    // haenge Nodevalue an
            rString += ::rtl::OUString::createFromAscii("\n");      // und beginne neu Zeile
            break;}

        default:
            OSL_ASSERT("OSQLParser::ShowParseTree: unzulaessiger NodeType");
        }
    }
}
#endif // OSL_DEBUG_LEVEL > 0
// -----------------------------------------------------------------------------
// Insert-Methoden
//-----------------------------------------------------------------------------
void OSQLParseNode::insert(sal_uInt32 nPos, OSQLParseNode* pNewSubTree)
{
    OSL_ENSURE(pNewSubTree != NULL, "OSQLParseNode: ungueltiger NewSubTree");
    OSL_ENSURE(pNewSubTree->getParent() == NULL, "OSQLParseNode: Knoten ist kein Waise");

    // stelle Verbindung zum getParent her:
    pNewSubTree->setParent( this );
    m_aChilds.insert(m_aChilds.begin() + nPos, pNewSubTree);
}

// removeAt-Methoden
//-----------------------------------------------------------------------------
OSQLParseNode* OSQLParseNode::removeAt(sal_uInt32 nPos)
{
    OSL_ENSURE(nPos < m_aChilds.size(),"Illegal position for removeAt");
    OSQLParseNodes::iterator aPos(m_aChilds.begin() + nPos);
    OSQLParseNode* pNode = *aPos;

    // setze den getParent des removeten auf NULL
    pNode->setParent( NULL );

    m_aChilds.erase(aPos);
    return pNode;
}
//-----------------------------------------------------------------------------
OSQLParseNode* OSQLParseNode::remove(OSQLParseNode* pSubTree)
{
    OSL_ENSURE(pSubTree != NULL, "OSQLParseNode: ungueltiger SubTree");
    OSQLParseNodes::iterator aPos = ::std::find(m_aChilds.begin(), m_aChilds.end(), pSubTree);
    if (aPos != m_aChilds.end())
    {
        // setze den getParent des removeten auf NULL
        pSubTree->setParent( NULL );
        m_aChilds.erase(aPos);
        return pSubTree;
    }
    else
        return NULL;
}

// Replace-Methoden
//-----------------------------------------------------------------------------
OSQLParseNode* OSQLParseNode::replaceAt(sal_uInt32 nPos, OSQLParseNode* pNewSubNode)
{
    OSL_ENSURE(pNewSubNode != NULL, "OSQLParseNode: invalid nodes");
    OSL_ENSURE(pNewSubNode->getParent() == NULL, "OSQLParseNode: node already has getParent");
    OSL_ENSURE(nPos < m_aChilds.size(), "OSQLParseNode: invalid position");
    OSL_ENSURE(::std::find(m_aChilds.begin(), m_aChilds.end(), pNewSubNode) == m_aChilds.end(),
            "OSQLParseNode::Replace() Node already element of parent");

    OSQLParseNode* pOldSubNode = m_aChilds[nPos];

    // stelle Verbindung zum getParent her:
    pNewSubNode->setParent( this );
    pOldSubNode->setParent( NULL );

    m_aChilds[nPos] = pNewSubNode;
    return pOldSubNode;
}

//-----------------------------------------------------------------------------
OSQLParseNode* OSQLParseNode::replace (OSQLParseNode* pOldSubNode, OSQLParseNode* pNewSubNode )
{
    OSL_ENSURE(pOldSubNode != NULL && pNewSubNode != NULL, "OSQLParseNode: invalid nodes");
    OSL_ENSURE(pNewSubNode->getParent() == NULL, "OSQLParseNode: node already has getParent");
    OSL_ENSURE(::std::find(m_aChilds.begin(), m_aChilds.end(), pOldSubNode) != m_aChilds.end(),
            "OSQLParseNode::Replace() Node not element of parent");
    OSL_ENSURE(::std::find(m_aChilds.begin(), m_aChilds.end(), pNewSubNode) == m_aChilds.end(),
            "OSQLParseNode::Replace() Node already element of parent");

    pOldSubNode->setParent( NULL );
    pNewSubNode->setParent( this );
    ::std::replace(m_aChilds.begin(), m_aChilds.end(), pOldSubNode, pNewSubNode);
    return pOldSubNode;
}
// -----------------------------------------------------------------------------
void OSQLParseNode::parseLeaf(::rtl::OUString & rString, const SQLParseNodeParameter& rParam) const
{
    // ein Blatt ist gefunden
    // Inhalt dem Ausgabestring anfuegen
    switch (m_eNodeType)
    {
        case SQL_NODE_KEYWORD:
        {
            if (rString.getLength())
                rString += ::rtl::OUString::createFromAscii(" ");

            ::rtl::OString sT = OSQLParser::TokenIDToStr(m_nNodeID, &rParam.m_rContext);
            rString += ::rtl::OUString(sT,sT.getLength(),RTL_TEXTENCODING_UTF8);
        }   break;
        case SQL_NODE_STRING:
            if (rString.getLength())
                rString += ::rtl::OUString::createFromAscii(" ");
            rString += SetQuotation(m_aNodeValue,::rtl::OUString::createFromAscii("\'"),::rtl::OUString::createFromAscii("\'\'"));
            break;
        case SQL_NODE_NAME:
            if (rString.getLength())
            {
                switch(rString.getStr()[rString.getLength()-1] )
                {
                    case ' ' :
                    case '.' : break;
                    default  :
                        if  (   !rParam.aMetaData.getCatalogSeparator().getLength()
                            ||  rString.getStr()[ rString.getLength()-1 ] != rParam.aMetaData.getCatalogSeparator().toChar()
                            )
                            rString += ::rtl::OUString::createFromAscii(" "); break;
                }
            }
            if (rParam.bQuote)
            {
                if (rParam.bPredicate)
                {
                    rString+= ::rtl::OUString::createFromAscii("[");
                    rString += m_aNodeValue;
                    rString+= ::rtl::OUString::createFromAscii("]");
                }
                else
                    rString += SetQuotation(m_aNodeValue,
                        rParam.aMetaData.getIdentifierQuoteString(), rParam.aMetaData.getIdentifierQuoteString() );
            }
            else
                rString += m_aNodeValue;
            break;
        case SQL_NODE_ACCESS_DATE:
            if (rString.getLength())
                rString += ::rtl::OUString::createFromAscii(" ");
            rString += ::rtl::OUString::createFromAscii("#");
            rString += m_aNodeValue;
            rString += ::rtl::OUString::createFromAscii("#");
            break;
        case SQL_NODE_INTNUM:
        case SQL_NODE_APPROXNUM:
            {
                ::rtl::OUString aTmp = m_aNodeValue;
                if (rParam.bInternational && rParam.bPredicate && rParam.cDecSep != '.')
                    aTmp = aTmp.replace('.', rParam.cDecSep);

                if (rString.getLength())
                    rString += ::rtl::OUString::createFromAscii(" ");
                rString += aTmp;

            }   break;
            // fall through
        default:
            if (rString.getLength() && m_aNodeValue.toChar() != '.' && m_aNodeValue.toChar() != ':' )
            {
                switch( rString.getStr()[rString.getLength()-1] )
                {
                    case ' ' :
                    case '.' : break;
                    default  :
                        if  (   !rParam.aMetaData.getCatalogSeparator().getLength()
                            ||  rString.getStr()[ rString.getLength()-1 ] != rParam.aMetaData.getCatalogSeparator().toChar()
                            )
                            rString += ::rtl::OUString::createFromAscii(" "); break;
                }
            }
            rString += m_aNodeValue;
    }
}

// -----------------------------------------------------------------------------
sal_Int32 OSQLParser::getFunctionReturnType(const ::rtl::OUString& _sFunctionName, const IParseContext* pContext)
{
    sal_Int32 nType = DataType::VARCHAR;
    ::rtl::OString sFunctionName(_sFunctionName,_sFunctionName.getLength(),RTL_TEXTENCODING_UTF8);

    if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_ASCII,pContext)))                     nType = DataType::INTEGER;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_BIT_LENGTH,pContext)))           nType = DataType::INTEGER;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_CHAR,pContext)))                 nType = DataType::VARCHAR;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_CHAR_LENGTH,pContext)))          nType = DataType::INTEGER;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_CHARACTER_LENGTH,pContext)))     nType = DataType::INTEGER;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_CONCAT,pContext)))               nType = DataType::VARCHAR;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_DIFFERENCE,pContext)))           nType = DataType::VARCHAR;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_INSERT,pContext)))               nType = DataType::VARCHAR;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_LCASE,pContext)))                nType = DataType::VARCHAR;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_LEFT,pContext)))                 nType = DataType::VARCHAR;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_LENGTH,pContext)))               nType = DataType::INTEGER;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_LOCATE,pContext)))               nType = DataType::VARCHAR;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_LOCATE_2,pContext)))             nType = DataType::VARCHAR;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_LTRIM,pContext)))                nType = DataType::VARCHAR;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_OCTET_LENGTH,pContext)))         nType = DataType::INTEGER;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_POSITION,pContext)))             nType = DataType::INTEGER;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_REPEAT,pContext)))               nType = DataType::VARCHAR;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_REPLACE,pContext)))              nType = DataType::VARCHAR;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_RIGHT,pContext)))                nType = DataType::VARCHAR;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_RTRIM,pContext)))                nType = DataType::VARCHAR;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_SOUNDEX,pContext)))              nType = DataType::VARCHAR;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_SPACE,pContext)))                nType = DataType::VARCHAR;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_SUBSTRING,pContext)))            nType = DataType::VARCHAR;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_UCASE,pContext)))                nType = DataType::VARCHAR;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_CURRENT_DATE,pContext)))         nType = DataType::DATE;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_CURRENT_TIME,pContext)))         nType = DataType::TIME;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_CURRENT_TIMESTAMP,pContext)))    nType = DataType::TIMESTAMP;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_CURDATE,pContext)))              nType = DataType::DATE;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_DATEDIFF,pContext)))             nType = DataType::INTEGER;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_DATEVALUE,pContext)))            nType = DataType::DATE;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_CURTIME,pContext)))              nType = DataType::TIME;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_DAYNAME,pContext)))              nType = DataType::VARCHAR;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_DAYOFMONTH,pContext)))           nType = DataType::INTEGER;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_DAYOFWEEK,pContext)))            nType = DataType::INTEGER;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_DAYOFYEAR,pContext)))            nType = DataType::INTEGER;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_EXTRACT,pContext)))              nType = DataType::VARCHAR;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_HOUR,pContext)))                 nType = DataType::INTEGER;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_MINUTE,pContext)))               nType = DataType::INTEGER;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_MONTH,pContext)))                nType = DataType::INTEGER;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_MONTHNAME,pContext)))            nType = DataType::VARCHAR;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_NOW,pContext)))                  nType = DataType::TIMESTAMP;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_QUARTER,pContext)))              nType = DataType::INTEGER;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_SECOND,pContext)))               nType = DataType::INTEGER;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_TIMESTAMPADD,pContext)))         nType = DataType::TIMESTAMP;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_TIMESTAMPDIFF,pContext)))        nType = DataType::TIMESTAMP;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_TIMEVALUE,pContext)))            nType = DataType::TIMESTAMP;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_WEEK,pContext)))                 nType = DataType::INTEGER;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_YEAR,pContext)))                 nType = DataType::INTEGER;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_ABS,pContext)))                  nType = DataType::DOUBLE;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_ACOS,pContext)))                 nType = DataType::DOUBLE;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_ASIN,pContext)))                 nType = DataType::DOUBLE;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_ATAN,pContext)))                 nType = DataType::DOUBLE;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_ATAN2,pContext)))                nType = DataType::DOUBLE;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_CEILING,pContext)))              nType = DataType::DOUBLE;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_COS,pContext)))                  nType = DataType::DOUBLE;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_COT,pContext)))                  nType = DataType::DOUBLE;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_DEGREES,pContext)))              nType = DataType::DOUBLE;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_EXP,pContext)))                  nType = DataType::DOUBLE;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_FLOOR,pContext)))                nType = DataType::DOUBLE;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_LOGF,pContext)))                 nType = DataType::DOUBLE;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_LOG,pContext)))                  nType = DataType::DOUBLE;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_LOG10,pContext)))                nType = DataType::DOUBLE;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_LN,pContext)))                   nType = DataType::DOUBLE;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_MOD,pContext)))                  nType = DataType::DOUBLE;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_PI,pContext)))                   nType = DataType::DOUBLE;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_POWER,pContext)))                nType = DataType::DOUBLE;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_RADIANS,pContext)))              nType = DataType::DOUBLE;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_RAND,pContext)))                 nType = DataType::DOUBLE;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_ROUND,pContext)))                nType = DataType::DOUBLE;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_ROUNDMAGIC,pContext)))           nType = DataType::DOUBLE;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_SIGN,pContext)))                 nType = DataType::DOUBLE;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_SIN,pContext)))                  nType = DataType::DOUBLE;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_SQRT,pContext)))                 nType = DataType::DOUBLE;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_TAN,pContext)))                  nType = DataType::DOUBLE;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_TRUNCATE,pContext)))             nType = DataType::DOUBLE;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_COUNT,pContext)))                nType = DataType::INTEGER;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_MAX,pContext)))                  nType = DataType::DOUBLE;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_MIN,pContext)))                  nType = DataType::DOUBLE;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_AVG,pContext)))                  nType = DataType::DOUBLE;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_SUM,pContext)))                  nType = DataType::DOUBLE;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_LOWER,pContext)))                nType = DataType::VARCHAR;
    else if(sFunctionName.equalsIgnoreAsciiCase(TokenIDToStr(SQL_TOKEN_UPPER,pContext)))                nType = DataType::VARCHAR;

    return nType;
}

// -----------------------------------------------------------------------------
const SQLError& OSQLParser::getErrorHelper() const
{
    return m_pData->aErrors;
}

// -----------------------------------------------------------------------------
OSQLParseNode::Rule OSQLParseNode::getKnownRuleID() const
{
    if ( !isRule() )
        return UNKNOWN_RULE;
    return OSQLParser::RuleIDToRule( getRuleID() );
}

}   // namespace connectivity
