/*************************************************************************
 *
 *  $RCSfile: sqlnode.cxx,v $
 *
 *  $Revision: 1.34 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 17:16:59 $
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
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
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


#ifndef _CONNECTIVITY_SQLNODE_HXX
#include <connectivity/sqlnode.hxx>
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
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include "connectivity/dbtools.hxx"
#endif


using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::i18n;
using namespace ::osl;
using namespace ::connectivity;
using namespace ::dbtools;
using namespace ::comphelper;


extern int SQLyyparse (void);

extern ::rtl::OUString ConvertLikeToken(const OSQLParseNode* pTokenNode, const OSQLParseNode* pEscapeNode, sal_Bool bInternational);

namespace
{
    sal_Bool lcl_convertDateFormat(const Reference< XNumberFormatter > & _xFormatter,sal_Int32 _nKey,const ::rtl::OUString& _sValue,double& _nrValue)
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
}
//------------------------------------------------------------------
OSQLParseNode::SQLParseNodeParameter::SQLParseNodeParameter(const ::rtl::OUString& _rIdentifierQuote, const ::rtl::OUString& _rCatalogSep,
                                                            const Reference< XNumberFormatter > & _xFormatter, const Reference< XPropertySet > & _xField, const ::com::sun::star::lang::Locale& _rLocale,
        const IParseContext* _pContext, sal_Bool _bIntl,  sal_Bool _bQuote, sal_Char _cDecSep,
        sal_Bool _bPredicate)
    :aIdentifierQuote(_rIdentifierQuote)
    ,aCatalogSeparator(_rCatalogSep)
    ,rLocale(_rLocale)
    ,m_pContext(_pContext ? _pContext : &OSQLParser::s_aDefaultContext)
    ,bInternational(_bIntl)
    ,bQuote(_bQuote)
    ,cDecSep(_cDecSep)
    ,xField(_xField)
    ,xFormatter(_xFormatter)
    ,bPredicate(_bPredicate)
{
}

::rtl::OUString SetQuotation(const ::rtl::OUString& rValue, const ::rtl::OUString& rQuot, const ::rtl::OUString& rQuotToReplace);
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
                                   const Reference< XDatabaseMetaData > & xMeta,
                                   const IParseContext* pContext,
                                   sal_Bool _bIntl,
                                   sal_Bool _bQuote) const
{

    parseNodeToStr(
        rString, xMeta, Reference< XNumberFormatter >(),  Reference< XPropertySet >(),
        pContext ? pContext->getPreferredLocale() : OParseContext::getDefaultLocale(),
        pContext, _bIntl, _bQuote, '.', sal_False);
}

//-----------------------------------------------------------------------------
void OSQLParseNode::parseNodeToPredicateStr(::rtl::OUString& rString,
                                              const Reference< XDatabaseMetaData > & xMeta,
                                              const Reference< XNumberFormatter > & xFormatter,
                                              const ::com::sun::star::lang::Locale& rIntl,
                                              sal_Char _cDec,
                                              const IParseContext* pContext ) const
{

    OSL_ENSURE(xFormatter.is(), "OSQLParseNode::parseNodeToPredicateStr:: no formatter!");

    if (xFormatter.is())
        parseNodeToStr(rString, xMeta, xFormatter, Reference< XPropertySet >(), rIntl, pContext, sal_True, sal_True, _cDec, sal_True);
}

//-----------------------------------------------------------------------------
void OSQLParseNode::parseNodeToPredicateStr(::rtl::OUString& rString,
                                              const Reference< XDatabaseMetaData > & xMeta,
                                              const Reference< XNumberFormatter > & xFormatter,
                                              const Reference< XPropertySet > & _xField,
                                              const ::com::sun::star::lang::Locale& rIntl,
                                              sal_Char _cDec,
                                              const IParseContext* pContext ) const
{

    OSL_ENSURE(xFormatter.is(), "OSQLParseNode::parseNodeToPredicateStr:: no formatter!");

    if (xFormatter.is())
        parseNodeToStr(rString, xMeta, xFormatter, _xField, rIntl, pContext, sal_True, sal_True, _cDec, sal_True);
}

//-----------------------------------------------------------------------------
void OSQLParseNode::parseNodeToStr(::rtl::OUString& rString,
                      const Reference< XDatabaseMetaData > & xMeta,
                      const Reference< XNumberFormatter > & xFormatter,
                      const Reference< XPropertySet > & _xField,
                      const ::com::sun::star::lang::Locale& rIntl,
                      const IParseContext* pContext,
                      sal_Bool _bIntl,
                      sal_Bool _bQuote,
                      sal_Char _cDecSep,
                      sal_Bool bPredicate) const
{

    OSL_ENSURE(xMeta.is(), "OSQLParseNode::parseNodeToStr:: no meta data!");

    if (xMeta.is())
    {
        ::rtl::OUString aIdentifierQuote(xMeta->getIdentifierQuoteString());
        ::rtl::OUString aCatalogSeparator(xMeta->getCatalogSeparator());

        OSQLParseNode::parseNodeToStr(rString,
            SQLParseNodeParameter(aIdentifierQuote, aCatalogSeparator, xFormatter, _xField, rIntl, pContext, _bIntl, _bQuote, _cDecSep, bPredicate));
    }
}
//-----------------------------------------------------------------------------
void OSQLParseNode::parseNodeToStr(::rtl::OUString& rString, const SQLParseNodeParameter& rParam) const
{


    if (!isToken())
    {
        // einmal auswerten wieviel Subtrees dieser Knoten besitzt
        sal_uInt32 nCount = count();

        // parameter erhalten sonderbehandlung
        if (SQL_ISRULE(this,parameter))
        {
            if(rString.getLength())
                rString += ::rtl::OUString::createFromAscii(" ");
            if (nCount == 1)    // ?
               m_aChilds[0]->parseNodeToStr(rString, rParam);
            else if (nCount == 2)   // :Name
            {
               m_aChilds[0]->parseNodeToStr(rString, rParam);
               rString += m_aChilds[1]->m_aNodeValue;
            }                   // [Name]
            else
            {
               m_aChilds[0]->parseNodeToStr(rString, rParam);
               rString += m_aChilds[1]->m_aNodeValue;
               rString += m_aChilds[2]->m_aNodeValue;
            }
        }

        else if(SQL_ISRULE(this,table_ref) &&
            (nCount == 4|| (nCount == 6 && SQL_ISPUNCTUATION(m_aChilds[0],"("))))
            tableRangeNodeToStr(rString, rParam);
        // je nachdem ob international angegeben wird oder nicht wird like anders behandelt
        // interanational: *, ? sind Platzhalter
        // sonst SQL92 konform: %, _
        else if (SQL_ISRULE(this,like_predicate))
            likeNodeToStr(rString, rParam);

        else if (SQL_ISRULE(this,general_set_fct)   || SQL_ISRULE(this,set_fct_spec)    ||
                SQL_ISRULE(this,position_exp)       || SQL_ISRULE(this,extract_exp)     ||
                SQL_ISRULE(this,length_exp)         || SQL_ISRULE(this,char_value_fct))
        {
            if (!addDateValue(rString, rParam))
            {
                // Funktionsname nicht quoten
                SQLParseNodeParameter aNewParam(rParam);
                aNewParam.bQuote = ( SQL_ISRULE(this,length_exp)    || SQL_ISRULE(this,char_value_fct) );

                m_aChilds[0]->parseNodeToStr(rString, aNewParam);
                aNewParam.bQuote = rParam.bQuote;
                aNewParam.bPredicate = sal_False; // disable [ ] around names
                ::rtl::OUString aStringPara;
                for (sal_uInt32 i=1; i<nCount; i++)
                {
                    const OSQLParseNode * pSubTree = m_aChilds[i];
                    if (pSubTree)
                    {
                        pSubTree->parseNodeToStr(aStringPara, aNewParam);

                        // bei den CommaListen zwischen alle Subtrees Commas setzen
                        if ((m_eNodeType == SQL_NODE_COMMALISTRULE)     && (i < (nCount - 1)))
                            aStringPara += ::rtl::OUString::createFromAscii(",");
                    }
                }
                aStringPara.trim();
                rString += aStringPara;
            }
        }
        else
        {
            for (OSQLParseNodes::const_iterator i = m_aChilds.begin();
                i != m_aChilds.end();)
            {
                const OSQLParseNode* pSubTree = *i;
                if (pSubTree)
                {
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
                            pSubTree->parseNodeToStr(rString, aNewParam);
                            i++;

                            // bei den CommaListen zwischen alle Subtrees Commas setzen
                            if ((m_eNodeType == SQL_NODE_COMMALISTRULE)     && (i != m_aChilds.end()))
                                rString += ::rtl::OUString::createFromAscii(",");
                        }
                    }
                    else
                    {
                        pSubTree->parseNodeToStr(rString, aNewParam);
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
    }
    else
    {
        parseLeaf(rString,rParam);
    }
}
//-----------------------------------------------------------------------------
void OSQLParseNode::tableRangeNodeToStr(::rtl::OUString& rString, const SQLParseNodeParameter& rParam) const
{
    sal_uInt32 nCount(count());
    rString += ::rtl::OUString::createFromAscii(" ");

    SQLParseNodeParameter aNewParam(rParam);
    //  aNewParam.bQuote = sal_False;
    if (nCount == 4)
    {
        m_aChilds[0]->parseNodeToStr(rString, rParam);
        m_aChilds[1]->parseNodeToStr(rString, rParam);
        m_aChilds[2]->parseNodeToStr(rString, aNewParam);
        m_aChilds[3]->parseNodeToStr(rString, rParam);
    }
    else if(nCount == 6 && SQL_ISPUNCTUATION(m_aChilds[0],"("))
    {
        m_aChilds[0]->parseNodeToStr(rString, rParam);
        m_aChilds[1]->parseNodeToStr(rString, rParam);
        m_aChilds[2]->parseNodeToStr(rString, rParam);
        m_aChilds[3]->parseNodeToStr(rString, rParam);
        m_aChilds[4]->parseNodeToStr(rString, aNewParam);
        m_aChilds[5]->parseNodeToStr(rString, rParam);
    }
}

//-----------------------------------------------------------------------------
void OSQLParseNode::likeNodeToStr(::rtl::OUString& rString, const SQLParseNodeParameter& rParam) const
{
    OSL_ENSURE(count() >= 4,"count != 5: Prepare for GPF");

    const OSQLParseNode* pEscNode = NULL;
    const OSQLParseNode* pParaNode = NULL;

    SQLParseNodeParameter aNewParam(rParam);
    aNewParam.bQuote = sal_True;

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
            OSL_ENSURE(0,"OSQLParseNode::likeNodeToStr Exception occured!");
        }

        const OSQLParseNode* pCol = m_aChilds[0]->getChild(m_aChilds[0]->count()-1);
        if ((SQL_ISRULE(pCol,column_val) && pCol->getChild(0)->getTokenValue().equalsIgnoreAsciiCase(aFieldName)) ||
            pCol->getTokenValue().equalsIgnoreAsciiCase(aFieldName) )
            bAddName = sal_False;
    }

    if (bAddName)
        m_aChilds[0]->parseNodeToStr(rString, aNewParam);

    m_aChilds[1]->parseNodeToStr(rString, aNewParam);
    if(count() == 5)
        m_aChilds[2]->parseNodeToStr(rString, aNewParam);

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
        pParaNode->parseNodeToStr(rString, aNewParam);

    pEscNode->parseNodeToStr(rString, aNewParam);
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
sal_Int16 OSQLParser::buildComparsionRule(OSQLParseNode*& pAppend,OSQLParseNode* pLiteral,OSQLParseNode*& pCompare)
{
    sal_Int16 nErg = 0;
    if (m_xField.is())
    {
        sal_Int32 nType = 0;
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

        if (pLiteral->isRule() && !SQL_ISRULE(pLiteral,value_exp))
        {
            switch(nType)
            {
                case DataType::CHAR:
                case DataType::VARCHAR:
                case DataType::LONGVARCHAR:
                    if(!SQL_ISRULE(pLiteral,char_value_exp) && !buildStringNodes(pLiteral))
                        break;
                default:
                    nErg = buildNode(pAppend,pLiteral,pCompare);
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
                            nErg = buildNode(pAppend,pLiteral,pCompare);
                            break;
                        case DataType::DATE:
                        case DataType::TIME:
                        case DataType::TIMESTAMP:
                            if (m_xFormatter.is())
                                nErg = buildDate(pLiteral->getTokenValue(),nType,pAppend,pLiteral,pCompare);
                            else
                                nErg = buildNode(pAppend,pLiteral,pCompare);

                            break;
                        default:
                            m_sErrorMessage = m_pContext->getErrorMessage(IParseContext::ERROR_INVALID_COMPARE);
                    }
                    break;
                case SQL_NODE_ACCESS_DATE:
                    switch(nType)
                    {
                        case DataType::DATE:
                        case DataType::TIME:
                        case DataType::TIMESTAMP:
                            nErg = -1;
                            if (m_xFormatter.is())
                            {
                                try
                                {
                                    // do we have a date
                                    if ( !m_nFormatKey )
                                    {
                                        Reference< ::com::sun::star::util::XNumberFormatsSupplier >  xFormatSup = m_xFormatter->getNumberFormatsSupplier();
                                        Reference< ::com::sun::star::util::XNumberFormatTypes >  xFormatTypes(xFormatSup->getNumberFormats(),UNO_QUERY);
                                        m_nFormatKey = ::dbtools::getDefaultNumberFormat(m_xField,xFormatTypes,*m_pLocale);
                                    }
                                    double fValue = m_xFormatter->convertStringToNumber(m_nFormatKey, pLiteral->getTokenValue().getStr());
                                    nErg = buildNode_Date(fValue, nType, pAppend,pLiteral,pCompare);
                                }
                                catch( Exception& )
                                {
                                    Reference< ::com::sun::star::util::XNumberFormatsSupplier >  xFormatSup = m_xFormatter->getNumberFormatsSupplier();
                                    Reference< ::com::sun::star::util::XNumberFormatTypes >  xFormatTypes(xFormatSup->getNumberFormats(),UNO_QUERY);
                                    if (xFormatTypes.is())
                                    {
                                        try
                                        {

                                            double fValue = m_xFormatter->convertStringToNumber(
                                                xFormatTypes->getStandardFormat(::com::sun::star::util::NumberFormat::DATE, *m_pLocale),
                                                                                pLiteral->getTokenValue().getStr());
                                            nErg = buildNode_Date(fValue, nType, pAppend,pLiteral,pCompare);

                                        }
                                        catch( Exception& )
                                        {
                                            try
                                            {
                                                double fValue = m_xFormatter->convertStringToNumber(
                                                    xFormatTypes->getFormatIndex(::com::sun::star::i18n::NumberFormatIndex::DATE_DIN_YYYYMMDD, *m_pLocale),
                                                                                    pLiteral->getTokenValue());
                                                nErg = buildNode_Date(fValue, nType, pAppend,pLiteral,pCompare);
                                            }
                                            catch( Exception& )
                                            {
                                                nErg = -1;
                                                m_sErrorMessage = m_pContext->getErrorMessage(IParseContext::ERROR_INVALID_DATE_COMPARE);
                                            }
                                        }
                                    }
                                    else
                                    {
                                        nErg = -1;
                                        m_sErrorMessage = m_pContext->getErrorMessage(IParseContext::ERROR_INVALID_DATE_COMPARE);
                                    }
                                }
                            }
                            else
                                m_sErrorMessage = m_pContext->getErrorMessage(IParseContext::ERROR_INVALID_DATE_COMPARE);
                            break;
                        default:
                            m_sErrorMessage = m_pContext->getErrorMessage(IParseContext::ERROR_INVALID_COMPARE);
                    }
                    break;
                case SQL_NODE_INTNUM:
                    switch(nType)
                    {
                        case DataType::BIT:
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
                            if (s_xLocaleData->getLocaleItem(*m_pLocale).decimalSeparator.toChar() == ',' )
                            {
                                pLiteral->m_aNodeValue = pLiteral->m_aNodeValue.replace('.', sal_Unicode());
                                // and replace decimal
                                pLiteral->m_aNodeValue = pLiteral->m_aNodeValue.replace(',', '.');
                            }
                            else
                                pLiteral->m_aNodeValue = pLiteral->m_aNodeValue.replace(',', sal_Unicode());
                            nErg = buildNode(pAppend,pLiteral,pCompare);
                            break;
                        case DataType::CHAR:
                        case DataType::VARCHAR:
                        case DataType::LONGVARCHAR:
                            nErg = buildNode_STR_NUM(pAppend,pLiteral,pCompare);
                            break;
                        default:
                            m_sErrorMessage = m_pContext->getErrorMessage(IParseContext::ERROR_INVALID_INT_COMPARE);
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
                            if (inPredicateCheck())
                            {
                                // kill thousand seperators if any
                                if (s_xLocaleData->getLocaleItem(*m_pLocale).decimalSeparator.toChar() == ',' )
                                {
                                    pLiteral->m_aNodeValue = pLiteral->m_aNodeValue.replace('.', sal_Unicode());
                                    // and replace decimal
                                    pLiteral->m_aNodeValue = pLiteral->m_aNodeValue.replace(',', '.');
                                }
                                else
                                    pLiteral->m_aNodeValue = pLiteral->m_aNodeValue.replace(',', sal_Unicode());
                            }
                            nErg = buildNode(pAppend,pLiteral,pCompare);
                            break;
                        case DataType::CHAR:
                        case DataType::VARCHAR:
                        case DataType::LONGVARCHAR:
                            nErg = buildNode_STR_NUM(pAppend,pLiteral,pCompare);
                            break;
                        case DataType::INTEGER:
                        default:
                            m_sErrorMessage = m_pContext->getErrorMessage(IParseContext::ERROR_INVALID_REAL_COMPARE);
                    }
                    break;
            }
        }
        if (!nErg)
            --nErg;
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
                    {
                        m_sErrorMessage = m_pContext->getErrorMessage(IParseContext::ERROR_VALUE_NO_LIKE);
                        m_sErrorMessage = m_sErrorMessage.replaceAt(m_sErrorMessage.indexOf(::rtl::OUString::createFromAscii("#1")),2,pLiteral->getTokenValue());
                    }
                }
            }
            break;
        default:
            m_sErrorMessage = m_pContext->getErrorMessage(IParseContext::ERROR_FIELD_NO_LIKE);
    }
    return nErg;
}
// -----------------------------------------------------------------------------
sal_Int16 OSQLParser::buildNode_STR_NUM(OSQLParseNode*& pAppend,OSQLParseNode*& pLiteral,OSQLParseNode*& pCompare)
{
    static ::rtl::OUString aEmptyString;
    OSQLParseNode* pColumnRef = new OSQLInternalNode(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::column_ref));
    pColumnRef->append(new OSQLInternalNode(m_sFieldName,SQL_NODE_NAME));
    OSQLParseNode* pComp = new OSQLInternalNode(aEmptyString, SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::comparison_predicate));
    pComp->append(pColumnRef);
    pComp->append(pCompare);

    if (m_nFormatKey)
    {
        sal_Int16 nScale = 0;
        ::rtl::OUString aDec;
        try
        {
            Any aValue = getNumberFormatProperty( m_xFormatter, m_nFormatKey, ::rtl::OUString::createFromAscii("Decimals") );
            aValue >>= nScale;
        }
        catch( Exception& )
        {
        }

        pComp->append(new OSQLInternalNode(stringToDouble(pLiteral->getTokenValue(),nScale),SQL_NODE_STRING));
    }
    else
        pComp->append(new OSQLInternalNode(pLiteral->getTokenValue(),SQL_NODE_STRING));

    pAppend->append(pComp);

    delete pLiteral;
    pLiteral = NULL;

    return 1;
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
            ParseResult aResult = m_xCharClass->parsePredefinedToken(KParseType::ANY_NUMBER,_rValue,0,*m_pLocale,0,::rtl::OUString(),KParseType::ANY_NUMBER,::rtl::OUString());
            if((aResult.TokenType & KParseType::IDENTNAME) && aResult.EndPos == _rValue.getLength())
            {
                aValue = ::rtl::OUString::valueOf(aResult.Value);
                sal_Int32 nPos = aValue.lastIndexOf(::rtl::OUString::createFromAscii("."));
                if((nPos+_nScale) < aValue.getLength())
                    aValue = aValue.replaceAt(nPos+_nScale,aValue.getLength()-nPos-_nScale,::rtl::OUString());
                aValue = aValue.replaceAt(aValue.lastIndexOf(::rtl::OUString::createFromAscii(".")),1,s_xLocaleData->getLocaleItem(*m_pLocale).decimalSeparator);
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

extern void setParser(OSQLParser*);
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
    if (!m_pLocale)
    {
        Locale aPreferredLocale( m_pContext->getPreferredLocale( ) );
            // this temporary is due to an MSVC compiler bug
        m_pLocale = new Locale( aPreferredLocale );
    }

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
            if (m_xField->getPropertySetInfo()->hasPropertyByName(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_REALNAME)))
                m_xField->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_REALNAME)) >>= aString;
            else
                m_xField->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME)) >>= aString;

            m_sFieldName = aString;

            // get the field format key
            if (m_xField->getPropertySetInfo()->hasPropertyByName(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FORMATKEY)))
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
                aValue >>= *m_pLocale;
        }
        else
            *m_pLocale = m_pContext->getPreferredLocale();

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
                        aLocale.Language = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("en_US"));
                        aLocale.Country = aLocale.Language;
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
                if (m_pLocale && s_xLocaleData->getLocaleItem(*m_pLocale).decimalSeparator.toChar() == ',')
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
           ,m_pLocale(NULL)
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
    }
    ++s_nRefCount;

    if (m_pContext == NULL)
        // take the default context
        m_pContext = &s_aDefaultContext;
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
sal_Int16 OSQLParser::buildDate(const ::rtl::OUString& _sValue,sal_Int32 _nType,OSQLParseNode*& pAppend,OSQLParseNode* pLiteral,OSQLParseNode*& pCompare)
{
    sal_Int16 nErg = 0;
    double fValue = 0.0;

    sal_Bool bBuildDate = lcl_convertDateFormat(m_xFormatter,m_nFormatKey,_sValue,fValue);
    if ( !bBuildDate )
    {
        try
        {
            Reference< ::com::sun::star::util::XNumberFormatsSupplier >  xFormatSup = m_xFormatter->getNumberFormatsSupplier();
            Reference< ::com::sun::star::util::XNumberFormatTypes >  xFormatTypes(xFormatSup->getNumberFormats(),UNO_QUERY);
            sal_Int32 nStandardFormat = 0;
            if ( xFormatTypes.is() )
            {
                nStandardFormat = xFormatTypes->getStandardFormat(::com::sun::star::util::NumberFormat::DATE, *m_pLocale);
                bBuildDate = lcl_convertDateFormat(m_xFormatter,nStandardFormat,_sValue,fValue);
            }
        }
        catch(Exception&)
        {
        }
        if ( !bBuildDate )
            bBuildDate = lcl_convertDateFormat(m_xFormatter,m_nDateFormatKey,_sValue,fValue);
    }

    if ( bBuildDate )
        nErg = buildNode_Date(fValue, _nType, pAppend,pLiteral,pCompare);
    else
    {
        nErg = -1;
        m_sErrorMessage = m_pContext->getErrorMessage(IParseContext::ERROR_INVALID_DATE_COMPARE);
    }
    return nErg;
}
// -----------------------------------------------------------------------------

