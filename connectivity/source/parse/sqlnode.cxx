/*************************************************************************
 *
 *  $RCSfile: sqlnode.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: oj $ $Date: 2001-02-14 10:29:29 $
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
#ifndef __SGI_STL_ROPE
#include <rope>
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

//------------------------------------------------------------------
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
// -------------------------------------------------------------------------
sal_Int32 getToken(const ::rtl::OUString& _rValue,const ::rtl::OUString& _rToken,sal_uInt32 _nPos)
{
    sal_Int32 nLen          = _rValue.getLength();
    sal_Int32 nFirstIndex   = 0;
    sal_Int32 nSecondIndex  = _rValue.indexOf(::rtl::OUString::createFromAscii("-"), nFirstIndex);
    while(_nPos--)
    {
        nFirstIndex  = nSecondIndex;
        nSecondIndex = _rValue.indexOf(::rtl::OUString::createFromAscii("-"), nFirstIndex);
    }

    return _rValue.copy(nFirstIndex,nSecondIndex-1).toInt32();
}
//-----------------------------------------------------------------------------
::rtl::OUString OSQLParseNode::convertDateString(const SQLParseNodeParameter& rParam, const ::rtl::OUString& rString) const
{
    // get the token out of a string
    sal_Int32 nYear     = getToken(rString,::rtl::OUString::createFromAscii("-"),0);
    sal_Int32 nMonth    = getToken(rString,::rtl::OUString::createFromAscii("-"),1);
    sal_Int32 nDay      = getToken(rString,::rtl::OUString::createFromAscii("-"),2);

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
    sal_Int32 nYear     = getToken(rString,::rtl::OUString::createFromAscii("-"),0);
    sal_Int32 nMonth    = getToken(rString,::rtl::OUString::createFromAscii("-"),1);
    sal_Int32 nDay      = getToken(rString,::rtl::OUString::createFromAscii("-"),2);
    sal_Int32 nHour     = getToken(rString,::rtl::OUString::createFromAscii(":"),0);
    sal_Int32 nMinute   = getToken(rString,::rtl::OUString::createFromAscii(":"),1);
    sal_Int32 nSecond   = getToken(rString,::rtl::OUString::createFromAscii(":"),2);

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
    sal_Int32 nHour     = getToken(rString,::rtl::OUString::createFromAscii(":"),0);
    sal_Int32 nMinute   = getToken(rString,::rtl::OUString::createFromAscii(":"),1);
    sal_Int32 nSecond   = getToken(rString,::rtl::OUString::createFromAscii(":"),2);

    Time aTime(0,nHour,nMinute,nSecond);
    Reference< XNumberFormatsSupplier >  xSupplier(rParam.xFormatter->getNumberFormatsSupplier());

    Reference< XNumberFormatTypes >  xTypes(xSupplier->getNumberFormats(), UNO_QUERY);

    double fTime = DBTypeConversion::toDouble(aTime);
    sal_Int32 nKey = xTypes->getStandardIndex(rParam.rLocale) + 41; // XXX hack
    return rParam.xFormatter->convertNumberToString(nKey, fTime);
}

DBG_NAME(OSQLParseNode);
//-----------------------------------------------------------------------------
OSQLParseNode::OSQLParseNode(const sal_Char * pNewValue,
                             SQLNodeType eNewNodeType,
                             sal_uInt32 nNewNodeID)
        : m_aNodeValue(::rtl::OUString::createFromAscii(pNewValue))
        , m_eNodeType(eNewNodeType)
        , m_nNodeID(nNewNodeID)
        , m_pParent(NULL)
{

    OSL_ENSHURE(m_eNodeType >= SQL_NODE_RULE && m_eNodeType <= SQL_NODE_ACCESS_DATE,"OSQLParseNode: mit unzulaessigem NodeType konstruiert");
}
//-----------------------------------------------------------------------------
OSQLParseNode::OSQLParseNode(const ::rtl::OString &_rNewValue,
                             SQLNodeType eNewNodeType,
                             sal_uInt32 nNewNodeID)
        : m_aNodeValue(::rtl::OUString::createFromAscii(_rNewValue.getStr()))
        , m_eNodeType(eNewNodeType)
        , m_nNodeID(nNewNodeID)
        , m_pParent(NULL)
{

    OSL_ENSHURE(m_eNodeType >= SQL_NODE_RULE && m_eNodeType <= SQL_NODE_ACCESS_DATE,"OSQLParseNode: mit unzulaessigem NodeType konstruiert");
}
//-----------------------------------------------------------------------------
OSQLParseNode::OSQLParseNode(const sal_Unicode * pNewValue,
                                 SQLNodeType eNewNodeType,
                                 sal_uInt32 nNewNodeID)
        : m_aNodeValue(pNewValue)
        , m_eNodeType(eNewNodeType)
        , m_nNodeID(nNewNodeID)
        , m_pParent(NULL)
{

    OSL_ENSHURE(m_eNodeType >= SQL_NODE_RULE && m_eNodeType <= SQL_NODE_ACCESS_DATE,"OSQLParseNode: mit unzulaessigem NodeType konstruiert");
}
//-----------------------------------------------------------------------------
OSQLParseNode::OSQLParseNode(const ::rtl::OUString &_rNewValue,
                                 SQLNodeType eNewNodeType,
                                 sal_uInt32 nNewNodeID)
        : m_aNodeValue(_rNewValue)
        , m_eNodeType(eNewNodeType)
        , m_nNodeID(nNewNodeID)
        , m_pParent(NULL)
{

    OSL_ENSHURE(m_eNodeType >= SQL_NODE_RULE && m_eNodeType <= SQL_NODE_ACCESS_DATE,"OSQLParseNode: mit unzulaessigem NodeType konstruiert");
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
    for (::std::vector<OSQLParseNode*>::const_iterator i = rParseNode.m_aChilds.begin();
         i != rParseNode.m_aChilds.end(); i++)
        append(new OSQLParseNode(**i));
}

//-----------------------------------------------------------------------------
OSQLParseNode& OSQLParseNode::operator=(const OSQLParseNode& rParseNode)
{


    if (this != &rParseNode)
    {
        // kopiere die member - pParent bleibt der alte
        m_aNodeValue = rParseNode.m_aNodeValue;
        m_eNodeType  = rParseNode.m_eNodeType;
        m_nNodeID    = rParseNode.m_nNodeID;

        for (::std::vector<OSQLParseNode*>::const_iterator i = m_aChilds.begin();
            i != m_aChilds.end(); i++)
            delete *i;

        m_aChilds.clear();

        for (::std::vector<OSQLParseNode*>::const_iterator j = rParseNode.m_aChilds.begin();
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
    for (::std::vector<OSQLParseNode*>::const_iterator i = m_aChilds.begin();
         i != m_aChilds.end(); i++)
        delete *i;
}

//-----------------------------------------------------------------------------
void OSQLParseNode::append(OSQLParseNode* pNewNode)
{

    OSL_ENSHURE(pNewNode != NULL, "OSQLParseNode: ungueltiger NewSubTree");
    OSL_ENSHURE(pNewNode->getParent() == NULL, "OSQLParseNode: Knoten ist kein Waise");
    OSL_ENSHURE(::std::find(m_aChilds.begin(), m_aChilds.end(), pNewNode) == m_aChilds.end(),
            "OSQLParseNode::append() Node already element of parent");

    // stelle Verbindung zum getParent her:
    pNewNode->setParent( this );
    // und haenge den SubTree hinten an
    m_aChilds.push_back(pNewNode);
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

    OSL_ENSHURE(xFormatter.is(), "OSQLParseNode::parseNodeToPredicateStr:: no formatter!");

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

    OSL_ENSHURE(xFormatter.is(), "OSQLParseNode::parseNodeToPredicateStr:: no formatter!");

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

    OSL_ENSHURE(xMeta.is(), "OSQLParseNode::parseNodeToStr:: no meta data!");

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
            for (::std::vector<OSQLParseNode*>::const_iterator i = m_aChilds.begin();
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
                            rtl::OUString aString;
                            rParam.xField->getPropertyValue(::rtl::OUString::createFromAscii("Name")) >>= aString;
                            aFieldName = aString.getStr();
                        }
                        catch ( ... )
                        {
                        }

                        const OSQLParseNode* pCol = pSubTree->m_aChilds[pSubTree->count()-1];
                        if ((SQL_ISRULE(pCol,column_val) && pCol->getChild(0)->getTokenValue().equalsIgnoreCase(aFieldName)) ||
                            pCol->getTokenValue().equalsIgnoreCase(aFieldName))
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
        // ein Blatt ist gefunden
        // Inhalt dem Ausgabestring anfuegen
        switch (m_eNodeType)
        {
            case SQL_NODE_KEYWORD:
            {
                if (rString.getLength())
                    rString += ::rtl::OUString::createFromAscii(" ");

                rString += ::rtl::OUString::createFromAscii(
                            OSQLParser::TokenIDToStr(m_nNodeID, &rParam.rContext).getStr());
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
                            if (!rParam.aCatalogSeparator.getLength() || rString.getStr()[rString.getLength()-1] != rParam.aCatalogSeparator.toChar())
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
                        rString += SetQuotation(m_aNodeValue, rParam.aIdentifierQuote, rParam.aIdentifierQuote);
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
                        aTmp.replace('.', rParam.cDecSep);

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
                            if (!rParam.aCatalogSeparator.getLength() || rString.getStr()[rString.getLength()-1] != rParam.aCatalogSeparator.toChar())
                                rString += ::rtl::OUString::createFromAscii(" "); break;
                    }
                }
                rString += m_aNodeValue;
        }
    }
}

//-----------------------------------------------------------------------------
sal_Bool OSQLParseNode::addDateValue(::rtl::OUString& rString, const SQLParseNodeParameter& rParam) const
{
    // special display for date/time values
    if (rParam.bPredicate && SQL_ISRULE(this,set_fct_spec) && SQL_ISPUNCTUATION(m_aChilds[0],"{"))
    {
        const OSQLParseNode* pODBCNode = m_aChilds[1];
        const OSQLParseNode* pODBCNodeChild = pODBCNode->m_aChilds[0];

        if (pODBCNodeChild->getNodeType() == SQL_NODE_KEYWORD && (
            SQL_ISTOKEN(pODBCNodeChild, D) ||
            SQL_ISTOKEN(pODBCNodeChild, T) ||
            SQL_ISTOKEN(pODBCNodeChild, TS) ))
        {
            if (rString.getLength())
                rString += ::rtl::OUString::createFromAscii(" ");
            rString += ::rtl::OUString::createFromAscii("#");
            if (SQL_ISTOKEN(pODBCNodeChild, D))
                rString += convertDateString(rParam, pODBCNode->m_aChilds[1]->getTokenValue());
            else if (SQL_ISTOKEN(pODBCNodeChild, T))
                rString += convertTimeString(rParam, pODBCNode->m_aChilds[1]->getTokenValue());
            else
                rString += convertDateTimeString(rParam, pODBCNode->m_aChilds[1]->getTokenValue());

            rString += ::rtl::OUString::createFromAscii("#");
            return sal_True;
        }
    }
    return sal_False;
}

//-----------------------------------------------------------------------------
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
    OSL_ENSHURE(count() >= 4,"count != 5: Prepare for GPF");

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
            rParam.xField->getPropertyValue(::rtl::OUString::createFromAscii("Name")) >>= aString;
            aFieldName = aString.getStr();
        }
        catch ( Exception& )
        {
            OSL_ENSHURE(0,"OSQLParseNode::likeNodeToStr Exception occured!");
        }

        const OSQLParseNode* pCol = m_aChilds[0]->getChild(m_aChilds[0]->count()-1);
        if ((SQL_ISRULE(pCol,column_val) && pCol->getChild(0)->getTokenValue().equalsIgnoreCase(aFieldName)) ||
            pCol->getTokenValue().equalsIgnoreCase(aFieldName) )
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

//-----------------------------------------------------------------------------
OSQLParseNode* OSQLParseNode::getByRule(OSQLParseNode::Rule eRule) const
{
    OSQLParseNode* pRetNode = 0;
    if (isRule() && OSQLParser::RuleID(eRule) == getRuleID())
        pRetNode = (OSQLParseNode*)this;
    else
    {
        for (::std::vector<OSQLParseNode*>::const_iterator i = m_aChilds.begin();
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
        // '(' search_condition ')'
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
            pSearchCondition->getParent()->replace(pSearchCondition, pNewNode);
            delete pSearchCondition;

            disjunctiveNormalForm(pNewNode);
        }
        else if(pRight->count() == 3 && SQL_ISRULE(pRight,boolean_primary) && SQL_ISRULE(pRight->getChild(1),search_condition))
        {
            // and-or tree  on right side
            OSQLParseNode* pOr = pRight->getChild(1);
            OSQLParseNode* pNewLeft = NULL;
            OSQLParseNode* pNewRight = NULL;

            // cut left from parent
            pSearchCondition->removeAt((sal_uInt32)0);

            pNewRight   = MakeANDNode(pLeft,pOr->removeAt(2));
            pNewLeft    = MakeANDNode(new OSQLParseNode(*pLeft),pOr->removeAt((sal_uInt32)0));
            pNewNode    = MakeORNode(pNewLeft,pNewRight);

            // and append new Node
            pSearchCondition->getParent()->replace(pSearchCondition, pNewNode);
            delete pSearchCondition;
            disjunctiveNormalForm(pNewNode);
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
    OSQLParseNode* pNode = NULL;
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
            pSearchCondition->getParent()->replace(pSearchCondition, pNewNode);
            delete pSearchCondition;
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
            pSearchCondition->getParent()->replace(pSearchCondition, pNewNode);
            delete pSearchCondition;
            pLeft   = pNewNode->getChild(0);
            pRight  = pNewNode->getChild(2);
        }

        negateSearchCondition(pLeft,bNegate);
        negateSearchCondition(pRight,bNegate);
    }
    // SQL_TOKEN_NOT boolean_test
    else if (SQL_ISRULE(pSearchCondition,boolean_factor))
    {
        OSQLParseNode *pNot = pSearchCondition->removeAt((sal_uInt32)0);
        delete pNot;
        OSQLParseNode *pBooleanTest = pSearchCondition->removeAt((sal_uInt32)0);
        pBooleanTest->setParent(NULL);
        pSearchCondition->getParent()->replace(pSearchCondition, pBooleanTest);
        delete pSearchCondition;
        if (!bNegate)
            negateSearchCondition(pBooleanTest,sal_True);   //  negate all deeper values
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
                pNewComparison = new OSQLParseNode(::rtl::OUString::createFromAscii("<>"),SQL_NODE_NOTEQUAL,NOTEQUAL);
                break;
            case SQL_NODE_LESS:
                pNewComparison = new OSQLParseNode(::rtl::OUString::createFromAscii(">="),SQL_NODE_GREATEQ,GREATEQ);
                break;
            case SQL_NODE_GREAT:
                pNewComparison = new OSQLParseNode(::rtl::OUString::createFromAscii("<="),SQL_NODE_LESSEQ,LESSEQ);
                break;
            case SQL_NODE_LESSEQ:
                pNewComparison = new OSQLParseNode(::rtl::OUString::createFromAscii(">"),SQL_NODE_GREAT,GREAT);
                break;
            case SQL_NODE_GREATEQ:
                pNewComparison = new OSQLParseNode(::rtl::OUString::createFromAscii("<"),SQL_NODE_LESS,LESS);
                break;
            case SQL_NODE_NOTEQUAL:
                pNewComparison = new OSQLParseNode(::rtl::OUString::createFromAscii("="),SQL_NODE_EQUAL,EQUAL);
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
            pNotNot = new OSQLParseNode(::rtl::OUString(),SQL_NODE_RULE,OSQLParser::RuleID(OSQLParseNode::not));
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
            pSearchCondition->getParent()->replace(pSearchCondition, pNode);
            delete pSearchCondition;
            pSearchCondition = pNode;
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

    // a and a || a or a
    OSQLParseNode* pNewNode = NULL;
    if(( SQL_ISRULE(pSearchCondition,boolean_term) || SQL_ISRULE(pSearchCondition,search_condition))
        && *pSearchCondition->getChild(0) == *pSearchCondition->getChild(2))
    {
        pNewNode = pSearchCondition->removeAt((sal_uInt32)0);
        pSearchCondition->getParent()->replace(pSearchCondition, pNewNode);
        delete pSearchCondition;
        pSearchCondition = pNewNode;
    }
    // (a or b) and a
    else if(SQL_ISRULE(pSearchCondition,boolean_term) && SQL_ISRULE(pSearchCondition->getChild(0),boolean_primary) &&
            SQL_ISRULE(pSearchCondition->getChild(0)->getChild(1),search_condition) &&
            *pSearchCondition->getChild(0)->getChild(1)->getChild(0) == *pSearchCondition->getChild(2))
    {
        pSearchCondition->getParent()->replace(pSearchCondition, pNewNode = pSearchCondition->removeAt(2));
        delete pSearchCondition;
        pSearchCondition = pNewNode;
    }
    // a and ( a or b)
    else if(SQL_ISRULE(pSearchCondition,boolean_term) && SQL_ISRULE(pSearchCondition->getChild(2),boolean_primary) &&
            SQL_ISRULE(pSearchCondition->getChild(2)->getChild(1),search_condition))

    {
        if(*pSearchCondition->getChild(2)->getChild(1)->getChild(0) == *pSearchCondition->getChild(0))
        {
            pSearchCondition->getParent()->replace(pSearchCondition, pNewNode = pSearchCondition->removeAt((sal_uInt32)0));
            delete pSearchCondition;
            pSearchCondition = pNewNode;
        }
        else if(*pSearchCondition->getChild(2)->getChild(1)->getChild(2) == *pSearchCondition->getChild(0))
        {
            pSearchCondition->getParent()->replace(pSearchCondition, pNewNode = pSearchCondition->removeAt((sal_uInt32)2));
            delete pSearchCondition;
            pSearchCondition = pNewNode;
        }
    }
    // a or a and b || a or b and a
    else if(SQL_ISRULE(pSearchCondition,search_condition) && SQL_ISRULE(pSearchCondition->getChild(2),boolean_term))
    {
        if(*pSearchCondition->getChild(2)->getChild(0) == *pSearchCondition->getChild(0))
        {
            pSearchCondition->getParent()->replace(pSearchCondition, pNewNode = pSearchCondition->removeAt((sal_uInt32)0));
            delete pSearchCondition;
            pSearchCondition = pNewNode;
        }
        else if(*pSearchCondition->getChild(2)->getChild(2) == *pSearchCondition->getChild(0))
        {
            pSearchCondition->getParent()->replace(pSearchCondition, pNewNode = pSearchCondition->removeAt((sal_uInt32)0));
            delete pSearchCondition;
            pSearchCondition = pNewNode;
        }
    }
    // a and b or a || b and a or a
    else if(SQL_ISRULE(pSearchCondition,search_condition) && SQL_ISRULE(pSearchCondition->getChild(0),boolean_term))
    {
        if(*pSearchCondition->getChild(0)->getChild(0) == *pSearchCondition->getChild(2))
        {
            pSearchCondition->getParent()->replace(pSearchCondition, pNewNode = pSearchCondition->removeAt((sal_uInt32)2));
            delete pSearchCondition;
            pSearchCondition = pNewNode;
        }
        else if(*pSearchCondition->getChild(0)->getChild(2) == *pSearchCondition->getChild(2))
        {
            pSearchCondition->getParent()->replace(pSearchCondition, pNewNode = pSearchCondition->removeAt((sal_uInt32)2));
            delete pSearchCondition;
            pSearchCondition = pNewNode;
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
            pSearchCondition->getParent()->replace(pSearchCondition, pNode);
            delete pSearchCondition;
            pSearchCondition = pNode;
        }
    }

    OSQLParseNode* pNewNode = NULL;
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
            pSearchCondition->getParent()->replace(pSearchCondition, pNode);
            delete pSearchCondition;
            pSearchCondition = pNode;
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
            pSearchCondition->getParent()->replace(pSearchCondition, pNode);
            delete pSearchCondition;
            pSearchCondition = pNode;
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
            pSearchCondition->getParent()->replace(pSearchCondition, pNode);
            delete pSearchCondition;
            pSearchCondition = pNode;
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
            pSearchCondition->getParent()->replace(pSearchCondition, pNode);
            delete pSearchCondition;
            pSearchCondition = pNode;
        }
    }
}
//-----------------------------------------------------------------------------
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

        // einmal auswerten wieviel Subtrees dieser Knoten besitzt
        sal_uInt32 nStop = count();
        // hol dir den ersten Subtree
        for (::std::vector<OSQLParseNode*>::const_iterator i = m_aChilds.begin();
            i != m_aChilds.end(); i++)
            (*i)->showParseTree(rString, nLevel+1);
    }
    else {
        // ein Token gefunden
        // tabs fuer das Einruecken entsprechend nLevel
        for (sal_uInt32 j=0; j<nLevel; j++) {rString+= ::rtl::OUString::createFromAscii("\t");};

        switch (m_eNodeType) {

        case SQL_NODE_KEYWORD:
            {rString+= ::rtl::OUString::createFromAscii("SQL_KEYWORD:\t");
             rString += ::rtl::OUString::createFromAscii(OSQLParser::TokenIDToStr(getTokenID()).getStr());
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
    };
}

// Insert-Methoden
//-----------------------------------------------------------------------------
void OSQLParseNode::insert(sal_uInt32 nPos, OSQLParseNode* pNewSubTree)
{
    OSL_ENSHURE(pNewSubTree != NULL, "OSQLParseNode: ungueltiger NewSubTree");
    OSL_ENSHURE(pNewSubTree->getParent() == NULL, "OSQLParseNode: Knoten ist kein Waise");

    // stelle Verbindung zum getParent her:
    pNewSubTree->setParent( this );
    m_aChilds.insert(m_aChilds.begin() + nPos);
}

// removeAt-Methoden
//-----------------------------------------------------------------------------
OSQLParseNode* OSQLParseNode::removeAt(sal_uInt32 nPos)
{
    ::std::vector<OSQLParseNode*>::iterator aPos(m_aChilds.begin() + nPos);
    OSQLParseNode* pNode = *aPos;

    // setze den getParent des removeten auf NULL
    pNode->setParent( NULL );

    m_aChilds.erase(aPos);
    return pNode;
}
//-----------------------------------------------------------------------------
OSQLParseNode* OSQLParseNode::remove(OSQLParseNode* pSubTree)
{
    OSL_ENSHURE(pSubTree != NULL, "OSQLParseNode: ungueltiger SubTree");
    ::std::vector<OSQLParseNode*>::iterator aPos = ::std::find(m_aChilds.begin(), m_aChilds.end(), pSubTree);
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
    OSL_ENSHURE(pNewSubNode != NULL, "OSQLParseNode: invalid nodes");
    OSL_ENSHURE(pNewSubNode->getParent() == NULL, "OSQLParseNode: node already has getParent");
    OSL_ENSHURE(nPos < m_aChilds.size(), "OSQLParseNode: invalid position");
    OSL_ENSHURE(::std::find(m_aChilds.begin(), m_aChilds.end(), pNewSubNode) == m_aChilds.end(),
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
    OSL_ENSHURE(pOldSubNode != NULL && pNewSubNode != NULL, "OSQLParseNode: invalid nodes");
    OSL_ENSHURE(pNewSubNode->getParent() == NULL, "OSQLParseNode: node already has getParent");
    OSL_ENSHURE(::std::find(m_aChilds.begin(), m_aChilds.end(), pOldSubNode) != m_aChilds.end(),
            "OSQLParseNode::Replace() Node not element of parent");
    OSL_ENSHURE(::std::find(m_aChilds.begin(), m_aChilds.end(), pNewSubNode) == m_aChilds.end(),
            "OSQLParseNode::Replace() Node already element of parent");

    pOldSubNode->setParent( NULL );
    pNewSubNode->setParent( this );
    ::std::replace(m_aChilds.begin(), m_aChilds.end(), pOldSubNode, pNewSubNode);
    return pOldSubNode;
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




