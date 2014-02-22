/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include "MacabStatement.hxx"
#include "MacabConnection.hxx"
#include "MacabAddressBook.hxx"
#include "MacabDriver.hxx"
#include "MacabResultSet.hxx"
#include "MacabResultSetMetaData.hxx"
#include "macabcondition.hxx"
#include "macaborder.hxx"
#include "TConnection.hxx"
#include <connectivity/dbexception.hxx>
#include "resource/sharedresources.hxx"
#include "resource/macab_res.hrc"

#if OSL_DEBUG_LEVEL > 0
# define OUtoCStr( x ) ( OUStringToOString ( (x), RTL_TEXTENCODING_ASCII_US).getStr())
#else /* OSL_DEBUG_LEVEL */
# define OUtoCStr( x ) ("dummy")
#endif /* OSL_DEBUG_LEVEL */

using namespace connectivity::macab;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::container;
using namespace com::sun::star::io;
using namespace com::sun::star::util;

namespace connectivity
{
    namespace macab
    {
    void impl_throwError(sal_uInt16 _nErrorId)
    {
        ::connectivity::SharedResources aResources;
        const OUString sError( aResources.getResourceString(_nErrorId) );
        ::dbtools::throwGenericSQLException(sError,NULL);
    }
    }
}

IMPLEMENT_SERVICE_INFO(MacabStatement, "com.sun.star.sdbc.drivers.MacabStatement", "com.sun.star.sdbc.Statement");

MacabCommonStatement::MacabCommonStatement(MacabConnection* _pConnection )
    : MacabCommonStatement_BASE(m_aMutex),
    OPropertySetHelper(MacabCommonStatement_BASE::rBHelper),
    m_aParser(_pConnection->getDriver()->getComponentContext()),
    m_aSQLIterator(_pConnection, _pConnection->createCatalog()->getTables(), m_aParser, NULL ),
    m_pParseTree(NULL),
    m_pConnection(_pConnection),
    rBHelper(MacabCommonStatement_BASE::rBHelper)
{
    m_pConnection->acquire();
}

MacabCommonStatement::~MacabCommonStatement()
{
}

void MacabCommonStatement::disposing()
{
    MacabCommonStatement_BASE::disposing();
}

void MacabCommonStatement::resetParameters() const throw(::com::sun::star::sdbc::SQLException)
{
    impl_throwError(STR_PARA_ONLY_PREPARED);
}

void MacabCommonStatement::getNextParameter(OUString &) const throw(::com::sun::star::sdbc::SQLException)
{
    impl_throwError(STR_PARA_ONLY_PREPARED);
}

MacabCondition *MacabCommonStatement::analyseWhereClause(const OSQLParseNode *pParseNode) const throw(SQLException)
{
    if (pParseNode->count() == 3)
    {
        const OSQLParseNode *pLeft = pParseNode->getChild(0),
                            *pMiddle = pParseNode->getChild(1),
                            *pRight = pParseNode->getChild(2);

        
        if (SQL_ISPUNCTUATION(pLeft, "(") && SQL_ISPUNCTUATION(pRight, ")"))
        {
              return analyseWhereClause(pMiddle);
        }
        else if (SQL_ISRULE(pParseNode, comparison_predicate))
        {
            if (pLeft->isToken() && pRight->isToken())
            {
                switch (pMiddle->getNodeType())
                {
                    case SQL_NODE_EQUAL:
                        
                        return new MacabConditionConstant(pLeft->getTokenValue() == pRight->getTokenValue());

                    case SQL_NODE_NOTEQUAL:
                        
                        
                        return new MacabConditionConstant(pLeft->getTokenValue() != pRight->getTokenValue());

                    default:
                        break;
                }
            }
            else if (SQL_ISRULE(pLeft, column_ref))
            {
                OUString sColumnName,
                                sTableRange;

                m_aSQLIterator.getColumnRange(pLeft, sColumnName, sTableRange);

                if (pRight->isToken() || SQL_ISRULE(pRight, parameter))
                {
                    OUString sMatchString;

                    if (pRight->isToken())                      
                        sMatchString = pRight->getTokenValue();
                    else if (SQL_ISRULE(pRight, parameter))     
                        getNextParameter(sMatchString);

                    switch (pMiddle->getNodeType())
                    {
                        case SQL_NODE_EQUAL:
                            
                            return new MacabConditionEqual(m_pHeader, sColumnName, sMatchString);

                         case SQL_NODE_NOTEQUAL:
                            
                            return new MacabConditionDifferent(m_pHeader, sColumnName, sMatchString);

                        default:
                            break;
                    }
                }
            }
        }
        else if (SQL_ISRULE(pParseNode, search_condition))
        {
            if (SQL_ISTOKEN(pMiddle, OR))
            {
                
                return new MacabConditionOr(
                    analyseWhereClause(pLeft),
                    analyseWhereClause(pRight));
            }
        }
        else if (SQL_ISRULE(pParseNode, boolean_term))
        {
            if (SQL_ISTOKEN(pMiddle, AND))
            {
                
                return new MacabConditionAnd(
                    analyseWhereClause(pLeft),
                    analyseWhereClause(pRight));
            }
        }
    }
    else if (SQL_ISRULE(pParseNode, test_for_null) || SQL_ISRULE(pParseNode, like_predicate))
    {
        const OSQLParseNode *pLeft = pParseNode->getChild(0);
        const OSQLParseNode* pPart2 = pParseNode->getChild(1);
        const OSQLParseNode *pMiddleLeft = pPart2->getChild(0),
                            *pMiddleRight = pPart2->getChild(1),
                            *pRight = pPart2->getChild(2);

        if (SQL_ISRULE(pParseNode, test_for_null))
        {
            if (SQL_ISRULE(pLeft, column_ref) &&
                            SQL_ISTOKEN(pMiddleLeft, IS) &&
                            SQL_ISTOKEN(pRight, NULL))
            {
                OUString sColumnName,
                                sTableRange;

                m_aSQLIterator.getColumnRange(pLeft, sColumnName, sTableRange);

                if (SQL_ISTOKEN(pMiddleRight, NOT))
                {
                    
                    return new MacabConditionNotNull(m_pHeader, sColumnName);
                }
                else
                {
                    
                    return new MacabConditionNull(m_pHeader, sColumnName);
                }
            }
        }
        else if (SQL_ISRULE(pParseNode, like_predicate))
        {
            if (SQL_ISRULE(pLeft, column_ref))
            {
                OUString sColumnName,
                                sTableRange;

                m_aSQLIterator.getColumnRange(pLeft, sColumnName, sTableRange);

                if (pMiddleRight->isToken() || SQL_ISRULE(pMiddleRight, parameter))
                {
                    OUString sMatchString;

                    if (pMiddleRight->isToken())                    
                        sMatchString = pMiddleRight->getTokenValue();
                    else if (SQL_ISRULE(pMiddleRight, parameter))   
                        getNextParameter(sMatchString);

                    return new MacabConditionSimilar(m_pHeader, sColumnName, sMatchString);
                }
            }
        }
    }
    impl_throwError(STR_QUERY_TOO_COMPLEX);
    
    OSL_ASSERT(false);
    return 0;
}

MacabOrder *MacabCommonStatement::analyseOrderByClause(const OSQLParseNode *pParseNode) const throw(SQLException)
{
    if (SQL_ISRULE(pParseNode, ordering_spec_commalist))
    {
        MacabComplexOrder *list = new MacabComplexOrder();
        sal_uInt32 n = pParseNode->count();

        
        for (sal_uInt32 i = 0; i < n; i++)
        {
            list->addOrder
                (analyseOrderByClause(pParseNode->getChild(i)));
        }

        return list;
    }
    else if (SQL_ISRULE(pParseNode, ordering_spec))
    {
        if (pParseNode->count() == 2)
        {
            OSQLParseNode* pColumnRef = pParseNode->getChild(0);
            OSQLParseNode* pAscendingDescending = pParseNode->getChild(1);

            if (SQL_ISRULE(pColumnRef, column_ref))
            {
                if (pColumnRef->count() == 3)
                    pColumnRef = pColumnRef->getChild(2);

                if (pColumnRef->count() == 1)
                {
                    OUString sColumnName =
                        pColumnRef->getChild(0)->getTokenValue();
                    sal_Bool bAscending =
                        SQL_ISTOKEN(pAscendingDescending, DESC)?
                        sal_False:
                        sal_True;

                    return new MacabSimpleOrder(m_pHeader, sColumnName, bAscending);
                }
            }
        }
    }
    impl_throwError(STR_QUERY_TOO_COMPLEX);
    
    OSL_ASSERT(false);
    return 0;
}

OUString MacabCommonStatement::getTableName() const
{
    const OSQLTables& xTabs = m_aSQLIterator.getTables();

    if( xTabs.empty() )
        return OUString();

    
    if(xTabs.size() > 1 || m_aSQLIterator.hasErrors() )
        return OUString();

    return xTabs.begin()->first;
}

void MacabCommonStatement::setMacabFields(MacabResultSet *pResult) const throw(SQLException)
{
    ::rtl::Reference<connectivity::OSQLColumns> xColumns;   
    MacabResultSetMetaData *pMeta;              

    xColumns = m_aSQLIterator.getSelectColumns();
    if (!xColumns.is())
    {
        ::connectivity::SharedResources aResources;
        const OUString sError( aResources.getResourceString(
                STR_INVALID_COLUMN_SELECTION
             ) );
        ::dbtools::throwGenericSQLException(sError,NULL);
    }
    pMeta = static_cast<MacabResultSetMetaData *>(pResult->getMetaData().get());
    pMeta->setMacabFields(xColumns);
}

void MacabCommonStatement::selectRecords(MacabResultSet *pResult) const throw(SQLException)
{
    const OSQLParseNode *pParseNode;

    pParseNode = m_aSQLIterator.getWhereTree();
    if (pParseNode != NULL)
    {
        if (SQL_ISRULE(pParseNode, where_clause))
        {
            
            
            
            pParseNode = pParseNode->getChild(1);
            MacabCondition *pCondition = analyseWhereClause(pParseNode);
            if (pCondition->isAlwaysTrue())
                pResult->allMacabRecords();
            else if (!pCondition->isAlwaysFalse())
                pResult->someMacabRecords(pCondition);
            delete pCondition;
            return;
        }
    }

    
    pResult->allMacabRecords();
}

void MacabCommonStatement::sortRecords(MacabResultSet *pResult) const throw(SQLException)
{
    const OSQLParseNode *pParseNode;

    pParseNode = m_aSQLIterator.getOrderTree();
    if (pParseNode != NULL)
    {
        if (SQL_ISRULE(pParseNode, opt_order_by_clause))
        {
            pParseNode = pParseNode->getChild(2);
            MacabOrder *pOrder = analyseOrderByClause(pParseNode);
            pResult->sortMacabRecords(pOrder);
            delete pOrder;
        }
    }
}

Any SAL_CALL MacabCommonStatement::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = MacabCommonStatement_BASE::queryInterface(rType);
    if (!aRet.hasValue())
        aRet = OPropertySetHelper::queryInterface(rType);
    return aRet;
}

Sequence< Type > SAL_CALL MacabCommonStatement::getTypes(  ) throw(RuntimeException)
{
    ::cppu::OTypeCollection aTypes( ::getCppuType( (const Reference< XMultiPropertySet > *)0 ),
                                    ::getCppuType( (const Reference< XFastPropertySet > *)0 ),
                                    ::getCppuType( (const Reference< XPropertySet > *)0 ));

    return comphelper::concatSequences(aTypes.getTypes(),MacabCommonStatement_BASE::getTypes());
}

void SAL_CALL MacabCommonStatement::cancel(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    checkDisposed(MacabCommonStatement_BASE::rBHelper.bDisposed);
    
}

void SAL_CALL MacabCommonStatement::close(  ) throw(SQLException, RuntimeException)
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(MacabCommonStatement_BASE::rBHelper.bDisposed);

    }
    dispose();
}

sal_Bool SAL_CALL MacabCommonStatement::execute(
        const OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabCommonStatement_BASE::rBHelper.bDisposed);

    Reference< XResultSet > xRS = executeQuery(sql);

    return xRS.is();
}

Reference< XResultSet > SAL_CALL MacabCommonStatement::executeQuery(
        const OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabCommonStatement_BASE::rBHelper.bDisposed);

OSL_TRACE("Mac OS Address book - SQL Request: %s", OUtoCStr(sql));

    MacabResultSet* pResult = new MacabResultSet(this);
    Reference< XResultSet > xRS = pResult;
    OUString aErr;

    m_pParseTree = m_aParser.parseTree(aErr, sql);
    if (m_pParseTree == NULL)
        throw SQLException(aErr, *this, aErr, 0, Any());

    m_aSQLIterator.setParseTree(m_pParseTree);
    m_aSQLIterator.traverseAll();
    switch (m_aSQLIterator.getStatementType())
    {
        case SQL_STATEMENT_SELECT:
            {
            OUString sTableName = getTableName(); 
            if (sTableName.getLength() != 0) 
            {
                MacabRecords *aRecords;
                aRecords = m_pConnection->getAddressBook()->getMacabRecords(sTableName);

                
                if(aRecords == NULL)
                {
                    impl_throwError(STR_NO_TABLE);
                }
                else
                {
                    m_pHeader = aRecords->getHeader();

                    pResult->setTableName(sTableName);

                    setMacabFields(pResult);        
                    selectRecords(pResult); 
                    sortRecords(pResult);   
                }


            }
            }
            break;

        default:



            impl_throwError(STR_QUERY_TOO_COMPLEX);
    }

    m_xResultSet = Reference<XResultSet>(pResult);
    return xRS;
}

Reference< XConnection > SAL_CALL MacabCommonStatement::getConnection(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabCommonStatement_BASE::rBHelper.bDisposed);

    
    return (Reference< XConnection >) m_pConnection;
}

sal_Int32 SAL_CALL MacabCommonStatement::executeUpdate( const OUString& ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabCommonStatement_BASE::rBHelper.bDisposed);

    
    return 0;
}

Any SAL_CALL MacabCommonStatement::getWarnings(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabCommonStatement_BASE::rBHelper.bDisposed);

    return makeAny(m_aLastWarning);
}

void SAL_CALL MacabCommonStatement::clearWarnings(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(MacabCommonStatement_BASE::rBHelper.bDisposed);

    m_aLastWarning = SQLWarning();
}

::cppu::IPropertyArrayHelper* MacabCommonStatement::createArrayHelper( ) const
{
    
    
    Sequence< Property > aProps(10);
    Property* pProperties = aProps.getArray();
    sal_Int32 nPos = 0;
    DECL_PROP0(CURSORNAME,  OUString);
    DECL_BOOL_PROP0(ESCAPEPROCESSING);
    DECL_PROP0(FETCHDIRECTION,sal_Int32);
    DECL_PROP0(FETCHSIZE,   sal_Int32);
    DECL_PROP0(MAXFIELDSIZE,sal_Int32);
    DECL_PROP0(MAXROWS,     sal_Int32);
    DECL_PROP0(QUERYTIMEOUT,sal_Int32);
    DECL_PROP0(RESULTSETCONCURRENCY,sal_Int32);
    DECL_PROP0(RESULTSETTYPE,sal_Int32);
    DECL_BOOL_PROP0(USEBOOKMARKS);

    return new ::cppu::OPropertyArrayHelper(aProps);
}

::cppu::IPropertyArrayHelper & MacabCommonStatement::getInfoHelper()
{
    return *const_cast<MacabCommonStatement*>(this)->getArrayHelper();
}

sal_Bool MacabCommonStatement::convertFastPropertyValue(
        Any &,
        Any &,
        sal_Int32,
        const Any&) throw (::com::sun::star::lang::IllegalArgumentException)
{
    sal_Bool bConverted = sal_False;
    
    return bConverted;
}

void MacabCommonStatement::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any&) throw (Exception)
{
    
    switch (nHandle)
    {
        case PROPERTY_ID_QUERYTIMEOUT:
        case PROPERTY_ID_MAXFIELDSIZE:
        case PROPERTY_ID_MAXROWS:
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
        case PROPERTY_ID_FETCHDIRECTION:
        case PROPERTY_ID_FETCHSIZE:
        case PROPERTY_ID_ESCAPEPROCESSING:
        case PROPERTY_ID_USEBOOKMARKS:
        default:
            ;
    }
}

void MacabCommonStatement::getFastPropertyValue(Any&,sal_Int32 nHandle) const
{
    switch (nHandle)
    {
        case PROPERTY_ID_QUERYTIMEOUT:
        case PROPERTY_ID_MAXFIELDSIZE:
        case PROPERTY_ID_MAXROWS:
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
        case PROPERTY_ID_FETCHDIRECTION:
        case PROPERTY_ID_FETCHSIZE:
        case PROPERTY_ID_ESCAPEPROCESSING:
        case PROPERTY_ID_USEBOOKMARKS:
        default:
            ;
    }
}

void SAL_CALL MacabCommonStatement::acquire() throw()
{
    MacabCommonStatement_BASE::acquire();
}

void SAL_CALL MacabCommonStatement::release() throw()
{
    MacabCommonStatement_BASE::release();
}

Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL MacabCommonStatement::getPropertySetInfo(  ) throw(RuntimeException)
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}

MacabStatement::MacabStatement(MacabConnection* _pConnection)
    : MacabStatement_BASE(_pConnection)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
