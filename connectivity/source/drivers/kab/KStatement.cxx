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
#include "sqlbison.hxx"


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
    m_aParser(_pConnection->getComponentContext()),
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

void KabCommonStatement::resetParameters() const throw(css::sdbc::SQLException)
{
    lcl_throwError(STR_PARA_ONLY_PREPARED);
}

void KabCommonStatement::getNextParameter(OUString &) const throw(css::sdbc::SQLException)
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

        // WHERE ( ... ) ?
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
                    case SQLNodeType::Equal:
                        // WHERE 0 = 1
                        return new KabConditionConstant(pLeft->getTokenValue() == pRight->getTokenValue());

                    case SQLNodeType::NotEqual:
                        // WHERE 0 <> 1
                        // (might not be correct SQL... don't care, handling anyway)
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

                    if (pRight->isToken())                      // WHERE Name = 'Doe'
                        sMatchString = pRight->getTokenValue();
                    else if (SQL_ISRULE(pRight, parameter))     // WHERE Name = ?
                        getNextParameter(sMatchString);

                    switch (pMiddle->getNodeType())
                    {
                        case SQLNodeType::Equal:
                            // WHERE Name = 'Smith'
                            return new KabConditionEqual(sColumnName, sMatchString);

                         case SQLNodeType::NotEqual:
                            // WHERE Name <> 'Jones'
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
                // WHERE Name = 'Smith' OR Name = 'Jones'
                return new KabConditionOr(
                    analyseWhereClause(pLeft),
                    analyseWhereClause(pRight));
            }
        }
        else if (SQL_ISRULE(pParseNode, boolean_term))
        {
            if (SQL_ISTOKEN(pMiddle, AND))
            {
                // WHERE Name = 'Smith' AND "Given Name" = 'Peter'
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
                    // WHERE "Mobile Phone" IS NOT NULL
                    return new KabConditionNotNull(sColumnName);
                }
                else
                {
                    // WHERE "Mobile Phone" IS NULL
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

                    if (pMiddleRight->isToken())                    // WHERE Name LIKE 'Sm%'
                        sMatchString = pMiddleRight->getTokenValue();
                    else if (SQL_ISRULE(pMiddleRight, parameter))   // WHERE Name LIKE ?
                        getNextParameter(sMatchString);

                    return new KabConditionSimilar(sColumnName, sMatchString);
                }
            }
        }
    }

    lcl_throwError(STR_QUERY_TOO_COMPLEX);

    // Unreachable:
    OSL_ASSERT(false);
    return 0;
}

KabOrder *KabCommonStatement::analyseOrderByClause(const OSQLParseNode *pParseNode) const throw(SQLException)
{
    if (SQL_ISRULE(pParseNode, ordering_spec_commalist))
    {
        KabComplexOrder *list = new KabComplexOrder();
        sal_uInt32 n = pParseNode->count();

        // Iterate through the ordering columns
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
                    bool bAscending =
                        !SQL_ISTOKEN(pAscendingDescending, DESC);

                    return new KabSimpleOrder(sColumnName, bAscending);
                }
            }
        }
    }
    lcl_throwError(STR_QUERY_TOO_COMPLEX);
    // Unreachable:
    OSL_ASSERT(false);
    return 0;
}

bool KabCommonStatement::isTableKnown(KabResultSet *pResult) const
{
    // can handle requests like        SELECT * FROM addresses addresses
    // but cannot handle requests like SELECT * FROM addresses persons
    if (m_aSQLIterator.getTables().size() != 1)
        return false;

    if (m_aSQLIterator.getTables().begin()->first != pResult->getMetaData()->getTableName(0))
        return false;

    return true;
}

void KabCommonStatement::setKabFields(KabResultSet *pResult) const throw(SQLException)
{
    ::rtl::Reference<connectivity::OSQLColumns> xColumns;   // selected columns

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

    // no WHERE clause: get all rows
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

Any SAL_CALL KabCommonStatement::queryInterface( const Type & rType ) throw(RuntimeException, std::exception)
{
    Any aRet = KabCommonStatement_BASE::queryInterface(rType);
    if (!aRet.hasValue())
        aRet = OPropertySetHelper::queryInterface(rType);
    return aRet;
}

Sequence< Type > SAL_CALL KabCommonStatement::getTypes(  ) throw(RuntimeException, std::exception)
{
    ::cppu::OTypeCollection aTypes( cppu::UnoType<XMultiPropertySet>::get(),
                                    cppu::UnoType<XFastPropertySet>::get(),
                                    cppu::UnoType<XPropertySet>::get());

    return comphelper::concatSequences(aTypes.getTypes(),KabCommonStatement_BASE::getTypes());
}

void SAL_CALL KabCommonStatement::cancel(  ) throw(RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);
    // cancel the current sql statement
}

void SAL_CALL KabCommonStatement::close(  ) throw(SQLException, RuntimeException, std::exception)
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);

    }
    dispose();
}

sal_Bool SAL_CALL KabCommonStatement::execute(
        const OUString& sql ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);

    Reference< XResultSet > xRS = executeQuery(sql);

    return xRS.is();
}

Reference< XResultSet > SAL_CALL KabCommonStatement::executeQuery(
        const OUString& sql ) throw(SQLException, RuntimeException, std::exception)
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
        case OSQLStatementType::Select:
            if (isTableKnown(pResult))      // FROM which table ?
            {
                setKabFields(pResult);      // SELECT which columns ?
                selectAddressees(pResult);  // WHERE which condition ?
                sortAddressees(pResult);    // ORDER BY which columns ?
// To be continued: DISTINCT
//                  etc...
            }
            break;

        default:
// To be continued: UPDATE
//                  DELETE
//                  etc...
            lcl_throwError(STR_QUERY_TOO_COMPLEX);
    }

    return xRS;
}

Reference< XConnection > SAL_CALL KabCommonStatement::getConnection(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);

    // just return our connection here
    return m_pConnection;
}

sal_Int32 SAL_CALL KabCommonStatement::executeUpdate( const OUString& ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);

    // the return values gives information about how many rows are affected by executing the sql statement
    return 0;
}

Any SAL_CALL KabCommonStatement::getWarnings(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);

    return makeAny(m_aLastWarning);
}

void SAL_CALL KabCommonStatement::clearWarnings(  ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(KabCommonStatement_BASE::rBHelper.bDisposed);

    m_aLastWarning = SQLWarning();
}

::cppu::IPropertyArrayHelper* KabCommonStatement::createArrayHelper( ) const
{
    // this properties are defined by the service statement
    // they must be in alphabetic order
    Sequence< Property > aProps(10);
    Property* pProperties = aProps.getArray();
    sal_Int32 nPos = 0;
    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_CURSORNAME),
        PROPERTY_ID_CURSORNAME, cppu::UnoType<OUString>::get(), 0);
    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ESCAPEPROCESSING),
        PROPERTY_ID_ESCAPEPROCESSING, cppu::UnoType<bool>::get(), 0);
    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FETCHDIRECTION),
        PROPERTY_ID_FETCHDIRECTION, cppu::UnoType<sal_Int32>::get(), 0);
    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FETCHSIZE),
        PROPERTY_ID_FETCHSIZE, cppu::UnoType<sal_Int32>::get(), 0);
    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_MAXFIELDSIZE),
        PROPERTY_ID_MAXFIELDSIZE, cppu::UnoType<sal_Int32>::get(), 0);
    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_MAXROWS),
        PROPERTY_ID_MAXROWS, cppu::UnoType<sal_Int32>::get(), 0);
    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_QUERYTIMEOUT),
        PROPERTY_ID_QUERYTIMEOUT, cppu::UnoType<sal_Int32>::get(), 0);
    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RESULTSETCONCURRENCY),
        PROPERTY_ID_RESULTSETCONCURRENCY, cppu::UnoType<sal_Int32>::get(), 0);
    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RESULTSETTYPE),
        PROPERTY_ID_RESULTSETTYPE, cppu::UnoType<sal_Int32>::get(), 0);
    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_USEBOOKMARKS),
        PROPERTY_ID_USEBOOKMARKS, cppu::UnoType<bool>::get(), 0);

    return new ::cppu::OPropertyArrayHelper(aProps);
}

::cppu::IPropertyArrayHelper & KabCommonStatement::getInfoHelper()
{
    return *getArrayHelper();
}

sal_Bool KabCommonStatement::convertFastPropertyValue(
        Any &,
        Any &,
        sal_Int32,
        const Any&) throw (css::lang::IllegalArgumentException)
{
    bool bConverted = false;
    // here we have to try to convert
    return bConverted;
}

void KabCommonStatement::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any&) throw (Exception, std::exception)
{
    // set the value to whatever is necessary
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

Reference< css::beans::XPropertySetInfo > SAL_CALL KabCommonStatement::getPropertySetInfo(  ) throw(RuntimeException, std::exception)
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}

KabStatement::KabStatement(KabConnection* _pConnection)
    : KabStatement_BASE(_pConnection)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
