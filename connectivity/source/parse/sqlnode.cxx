/*************************************************************************
 *
 *  $RCSfile: sqlnode.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: oj $ $Date: 2001-05-23 09:10:28 $
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
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATABASEMETADATA_HPP_
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
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

using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::osl;
using namespace connectivity;
using namespace dbtools;

extern Any getNumberFormatProperty(const Reference< XNumberFormatsSupplier > & xFormatter,
                                                          sal_Int32 nKey,
                                                          const rtl::OUString& );
extern ::rtl::OUString ConvertLikeToken(const OSQLParseNode* pTokenNode, const OSQLParseNode* pEscapeNode, sal_Bool bInternational);
//------------------------------------------------------------------
OSQLParseNode::SQLParseNodeParameter::SQLParseNodeParameter(const ::rtl::OUString& _rIdentifierQuote, const ::rtl::OUString& _rCatalogSep,
                                                            const Reference< XNumberFormatter > & _xFormatter, const Reference< XPropertySet > & _xField, const ::com::sun::star::lang::Locale& _rLocale,
        const OParseContext* _pContext, sal_Bool _bIntl,  sal_Bool _bQuote, sal_Char _cDecSep,
        sal_Bool _bPredicate)
    :aIdentifierQuote(_rIdentifierQuote)
    ,aCatalogSeparator(_rCatalogSep)
    ,rLocale(_rLocale)
    ,rContext(_pContext ? *_pContext : OSQLParser::s_aDefaultContext)
    ,bInternational(_bIntl)
    ,bQuote(_bQuote)
    ,cDecSep(_cDecSep)
    ,xField(_xField)
    ,xFormatter(_xFormatter)
    ,bPredicate(_bPredicate)
{
}

::rtl::OUString SetQuotation(const ::rtl::OUString& rValue, const ::rtl::OUString& rQuot, const ::rtl::OUString& rQuotToReplace);
sal_Int32 getToken(const ::rtl::OUString& _rValue,const ::rtl::OUString& _rToken,sal_uInt32 _nPos);
//-----------------------------------------------------------------------------
::rtl::OUString OSQLParseNode::convertDateString(const SQLParseNodeParameter& rParam, const ::rtl::OUString& rString) const
{
    // get the token out of a string
    static ::rtl::OUString sDateSep = ::rtl::OUString::createFromAscii("-");

    sal_Int32 nYear     = getToken(rString,sDateSep,0);
    sal_Int32 nMonth    = getToken(rString,sDateSep,1);
    sal_Int32 nDay      = getToken(rString,sDateSep,2);

    Date aDate(nDay,nMonth,nYear);
    Reference< XNumberFormatsSupplier > xSupplier(rParam.xFormatter->getNumberFormatsSupplier());
    Reference< XNumberFormatTypes >     xTypes(xSupplier->getNumberFormats(), UNO_QUERY);

    double fDate = DBTypeConversion::toDouble(aDate,DBTypeConversion::getNULLDate(xSupplier));
    sal_Int32 nKey = xTypes->getStandardIndex(rParam.rLocale) + 36; // XXX hack
    return rParam.xFormatter->convertNumberToString(nKey, fDate);
}

//-----------------------------------------------------------------------------
::rtl::OUString OSQLParseNode::convertDateTimeString(const SQLParseNodeParameter& rParam, const ::rtl::OUString& rString) const
{
    static ::rtl::OUString sDateSep = ::rtl::OUString::createFromAscii("-");
    static ::rtl::OUString sTimeSep = ::rtl::OUString::createFromAscii(":");

    sal_Int32 nYear     = getToken(rString,sDateSep,0);
    sal_Int32 nMonth    = getToken(rString,sDateSep,1);
    sal_Int32 nDay      = getToken(rString,sDateSep,2);
    sal_Int32 nHour     = getToken(rString,sTimeSep,0);
    sal_Int32 nMinute   = getToken(rString,sTimeSep,1);
    sal_Int32 nSecond   = getToken(rString,sTimeSep,2);

    DateTime aDate(0,nSecond,nMinute,nHour,nDay,nMonth,nYear);
    Reference< XNumberFormatsSupplier >  xSupplier(rParam.xFormatter->getNumberFormatsSupplier());
    Reference< XNumberFormatTypes >  xTypes(xSupplier->getNumberFormats(), UNO_QUERY);

    double fDateTime = DBTypeConversion::toDouble(aDate,DBTypeConversion::getNULLDate(xSupplier));
    sal_Int32 nKey = xTypes->getStandardIndex(rParam.rLocale) + 51; // XXX hack
    return rParam.xFormatter->convertNumberToString(nKey, fDateTime);
}

//-----------------------------------------------------------------------------
::rtl::OUString OSQLParseNode::convertTimeString(const SQLParseNodeParameter& rParam, const ::rtl::OUString& rString) const
{
    static ::rtl::OUString sTimeSep = ::rtl::OUString::createFromAscii(":");

    sal_Int32 nHour     = getToken(rString,sTimeSep,0);
    sal_Int32 nMinute   = getToken(rString,sTimeSep,1);
    sal_Int32 nSecond   = getToken(rString,sTimeSep,2);

    Time aTime(0,nHour,nMinute,nSecond);
    Reference< XNumberFormatsSupplier >  xSupplier(rParam.xFormatter->getNumberFormatsSupplier());

    Reference< XNumberFormatTypes >  xTypes(xSupplier->getNumberFormats(), UNO_QUERY);

    double fTime = DBTypeConversion::toDouble(aTime);
    sal_Int32 nKey = xTypes->getStandardIndex(rParam.rLocale) + 41; // XXX hack
    return rParam.xFormatter->convertNumberToString(nKey, fTime);
}

//-----------------------------------------------------------------------------
void OSQLParseNode::parseNodeToStr(::rtl::OUString& rString,
                                   const Reference< XDatabaseMetaData > & xMeta,
                                   const OParseContext* pContext,
                                   sal_Bool _bIntl,
                                   sal_Bool _bQuote) const
{

    parseNodeToStr(rString, xMeta, Reference< XNumberFormatter >(),
        Reference< XPropertySet >(), OParseContext::getDefaultLocale(), pContext, _bIntl, _bQuote, '.', sal_False);
}

//-----------------------------------------------------------------------------
void OSQLParseNode::parseNodeToPredicateStr(::rtl::OUString& rString,
                                              const Reference< XDatabaseMetaData > & xMeta,
                                              const Reference< XNumberFormatter > & xFormatter,
                                              const ::com::sun::star::lang::Locale& rIntl,
                                              sal_Char _cDec,
                                              const OParseContext* pContext ) const
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
                                              const OParseContext* pContext ) const
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
                      const OParseContext* pContext,
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
                aNewParam.bQuote = sal_False;

                m_aChilds[0]->parseNodeToStr(rString, aNewParam);
                ::rtl::OUString aStringPara;
                for (sal_uInt32 i=1; i<nCount; i++)
                {
                    const OSQLParseNode * pSubTree = m_aChilds[i];
                    if (pSubTree)
                    {
                        pSubTree->parseNodeToStr(aStringPara, rParam);

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

                    // if there is a field given we don't display the fieldname, if there are any
                    if (rParam.xField.is() && SQL_ISRULE(pSubTree,column_ref))
                    {
                        sal_Bool bFilter = sal_False;
                        // retrieve the fields name
                        ::rtl::OUString aFieldName;
                        try
                        {
                            // retrieve the fields name
                            rParam.xField->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME)) >>= aFieldName;
                        }
                        catch ( Exception& )
                        {
                        }

                        const OSQLParseNode* pCol = pSubTree->m_aChilds[pSubTree->count()-1];
                        if ((SQL_ISRULE(pCol,column_val) && pCol->getChild(0)->getTokenValue().equalsIgnoreAsciiCase(aFieldName)) ||
                            pCol->getTokenValue().equalsIgnoreAsciiCase(aFieldName))
                            bFilter = sal_True;

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

    if (pParaNode->isToken() && rParam.bInternational)
    {
        ::rtl::OUString aStr = ConvertLikeToken(pParaNode, pEscNode, sal_True);
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




