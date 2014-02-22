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


#include "KStatement.hxx"
#include "KConnection.hxx"
#include "KDriver.hxx"
#include "KResultSet.hxx"
#include "KResultSetMetaData.hxx"
#include "kcondition.hxx"
#include "korder.hxx"
#include "TConnection.hxx"
#include <connectivity/dbexception.hxx>
#include "resource/kab_res.hrc"
#include "resource/sharedresources.hxx"


#if OSL_DEBUG_LEVEL > 0
# define OUtoCStr( x ) ( OUStringToOString ( (x), RTL_TEXTENCODING_ASCII_US).getStr())
#else /* OSL_DEBUG_LEVEL */
# define OUtoCStr( x ) ("dummy")
#endif /* OSL_DEBUG_LEVEL */

using namespace connectivity::kab;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::container;
using namespace com::sun::star::io;
using namespace com::sun::star::util;

namespace
{
    void lcl_throwError(sal_uInt16 _nErrorId)
    {
        ::connectivity::SharedResources aResources;
        const OUString sError( aResources.getResourceString(_nErrorId) );
        ::dbtools::throwGenericSQLException(sError,NULL);
    }
}

IMPLEMENT_SERVICE_INFO(KabStatement, "com.sun.star.sdbc.drivers.KabStatement", "com.sun.star.sdbc.Statement");

KabCommonStatement::KabCommonStatement(KabConnection* _pConnection )
    : KabCommonStatement_BASE(m_aMutex),
    OPropertySetHelper(KabCommonStatement_BASE::rBHelper),
    m_aParser(_pConnection->getDriver()->getComponentContext()),
    m_aSQLIterator(_pConnection, _pConnection->createCatalog()->getTables(), m_aParser, NULL ),
    m_pParseTree(NULL),
    m_pConnection(_pConnection),
    rBHelper(KabCommonStatement_BASE::rBHelper)
{
    m_pConnection->acquire();
}

KabCommonStatement::~KabCommonStatement()
{
}

void KabCommonStatement::disposing()
{
    KabCommonStatement_BASE::disposing();
}

void KabCommonStatement::resetParameters() const throw(::com::sun::star::sdbc::SQLException)
{
    lcl_throwError(STR_PARA_ONLY_PREPARED);
}

void KabCommonStatement::getNextParameter(OUString &) const throw(::com::sun::star::sdbc::SQLException)
{
    lcl_throwError(STR_PARA_ONLY_PREPARED);
}

KabCondition *KabCommonStatement::analyseWhereClause(const OSQLParseNode *pParseNode) const throw(SQLException)
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
                        
                        return new KabConditionConstant(pLeft->getTokenValue() == pRight->getTokenValue());

                    case SQL_NODE_NOTEQUAL:
                        
                        
                        return new KabConditionConstant(pLeft->getTokenValue() != pRight->getTokenValue());

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
                            
                            return new KabConditionEqual(sColumnName, sMatchString);

                         case SQL_NODE_NOTEQUAL:
                            
                            return new KabConditionDifferent(sColumnName, sMatchString);

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
                
                return new KabConditionOr(
                    analyseWhereClause(pLeft),
                    analyseWhereClause(pRight));
            }
        }
        else if (SQL_ISRULE(pParseNode, boolean_term))
        {
            if (SQL_ISTOKEN(pMiddle, AND))
            {
                
                return new KabConditionAnd(
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
                    
                    return new KabConditionNotNull(sColumnName);
                }
                else
                {
                    
                    return new KabConditionNull(sColumnName);
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

                    return new KabConditionSimilar(sColumnName, sMatchString);
                }
            }
        }
    }

    lcl_throwError(STR_QUERY_TOO_COMPLEX);

    
    OSL_ASSERT(false);
    return 0;
}

KabOrder *KabCommonStatement::analyseOrderByClause(const OSQLParseNode *pParseNode) const throw(SQLException)
{
    if (SQL_ISRULE(pParseNode, ordering_spec_commalist))
    {
        KabComplexOrder *list = new KabComplexOrder();
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

                    return new KabSimpleOrder(sColumnName, bAscending);
                }
            }
        }
    }
    lcl_throwError(STR_QUERY_TOO_COMPLEX);
    
    OSL_ASSERT(false);
    return 0;
}

sal_Bool KabCommonStatement::isTableKnown(KabResultSet *pResult) const
{
    
    
    if (m_aSQLIterator.getTables().size() != 1)
        return sal_False;

    if (m_aSQLIterator.getTables().begin()->first != pResult->getMetaData()->getTableName(0))
        return sal_False;

    return sal_True;
}

void KabCommonStatement::setKabFields(KabResultSet *pResult) const throw(SQLException)
{
    ::rtl::Reference<connectivity::OSQLColumns> xColumns;   

    xColumns = m_aSQLIterator.getSelectColumns();
    if (!xColumns.is())
    {
        lcl_throwError(STR_INVALID_COLUMN_SELECTION);
    }
    pResult->getKabMetaData()->setKabFields(xColumns);
}

void KabCommonStatement::selectAddressees(KabResultSet *pResult) const throw(SQLException)
{
    const OSQLParseNode *pParseNode;

    pParseNode = m_aSQLIterator.getWhereTree();
    if (pParseNode != NULL)
    {
        if (SQL_ISRULE(pParseNode, where_clause))
        {
            resetParameters();
            pParseNode = pParseNode->getChild(1);
            KabCondition *pCondition = analyseWhereClause(pParseNode);
            if (pCondition->isAlwaysTrue())
                pResult->allKabAddressees();
            else if (!pCondition->isAlwaysFalse())
                pResult->someKabAddressees(pCondition);
            delete pCondition;
            return;
        }
    }

    
    pResult->allKabAddressees();
}

void KabCommonStatement::sortAddressees(KabResultSet *pResult) const throw(SQLException)
{
    const OSQLParseNode *pParseNode;

    pParseNode = m_aSQLIterator.getOrderTree();
    if (pParseNode != NULL)
    {
        if (SQL_ISRULE(pParseNode, opt_order_by_clause))
        {
            pParseNode = pParseNode->getChild(2);
            KabOrder *pOrder = analyseOrderByClause(pParseNode);
            pResult->sortKabAddressees(pOrder);
            delete pOrder;
        }
    }
}

Any SAL_CALL KabCommonStatement::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = KabCommonStatement_BASE::queryInterface(rType);
    if (!aRet.hasValue())
        aRet = OPropertySetHelper::queryInterface(rType);
    return aRet;
}

Sequence< Type > SAL_CALL KabCommonStatement::getTypes(  ) throw(RuntimeException)
{
    ::cppu::OTypeCollection aTypes( ::getCppuType( (const Reference< XMultiPropertySet > *)0 ),
                                    ::getCppuType( (const Reference< XFastPropertySet > *)0 ),
                                    ::getCppuType( (const Reference< XPropertySet > *)0 ));

    return comphelper::concatSequences(aTypes.getTypes(),KabCommonStatement_BASE::getTypes());
}

void SAL_CALL KabCommonStatement::cancel(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);
    
}

void SAL_CALL KabCommonStatement::close(  ) throw(SQLException, RuntimeException)
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);

    }
    dispose();
}

sal_Bool SAL_CALL KabCommonStatement::execute(
        const OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);

    Reference< XResultSet > xRS = executeQuery(sql);

    return xRS.is();
}

Reference< XResultSet > SAL_CALL KabCommonStatement::executeQuery(
        const OUString& sql ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);

OSL_TRACE("KDE Address book - SQL Request: %s", OUtoCStr(sql));

    KabResultSet* pResult = new KabResultSet(this);
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
            if (isTableKnown(pResult))      
            {
                setKabFields(pResult);      
                selectAddressees(pResult);  
                sortAddressees(pResult);    


            }
            break;

        default:



            lcl_throwError(STR_QUERY_TOO_COMPLEX);
    }

    return xRS;
}

Reference< XConnection > SAL_CALL KabCommonStatement::getConnection(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);

    
    return (Reference< XConnection >) m_pConnection;
}

sal_Int32 SAL_CALL KabCommonStatement::executeUpdate( const OUString& ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);

    
    return 0;
}

Any SAL_CALL KabCommonStatement::getWarnings(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);

    return makeAny(m_aLastWarning);
}

void SAL_CALL KabCommonStatement::clearWarnings(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);

    m_aLastWarning = SQLWarning();
}

::cppu::IPropertyArrayHelper* KabCommonStatement::createArrayHelper( ) const
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

::cppu::IPropertyArrayHelper & KabCommonStatement::getInfoHelper()
{
    return *const_cast<KabCommonStatement*>(this)->getArrayHelper();
}

sal_Bool KabCommonStatement::convertFastPropertyValue(
        Any &,
        Any &,
        sal_Int32,
        const Any&) throw (::com::sun::star::lang::IllegalArgumentException)
{
    sal_Bool bConverted = sal_False;
    
    return bConverted;
}

void KabCommonStatement::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any&) throw (Exception)
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

void KabCommonStatement::getFastPropertyValue(Any&,sal_Int32 nHandle) const
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

void SAL_CALL KabCommonStatement::acquire() throw()
{
    KabCommonStatement_BASE::acquire();
}

void SAL_CALL KabCommonStatement::release() throw()
{
    KabCommonStatement_BASE::release();
}

Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL KabCommonStatement::getPropertySetInfo(  ) throw(RuntimeException)
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}

KabStatement::KabStatement(KabConnection* _pConnection)
    : KabStatement_BASE(_pConnection)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
