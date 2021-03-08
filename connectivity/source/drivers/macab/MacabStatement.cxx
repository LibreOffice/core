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


#include "MacabStatement.hxx"
#include <sqlbison.hxx>
#include "MacabConnection.hxx"
#include "MacabAddressBook.hxx"
#include "MacabDriver.hxx"
#include "MacabResultSet.hxx"
#include "MacabResultSetMetaData.hxx"
#include "macabcondition.hxx"
#include "macaborder.hxx"
#include "macabutilities.hxx"
#include <TConnection.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <connectivity/dbexception.hxx>
#include <resource/sharedresources.hxx>
#include <rtl/ref.hxx>
#include <strings.hrc>

using namespace connectivity::macab;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::container;
using namespace com::sun::star::io;
using namespace com::sun::star::util;

namespace connectivity::macab
{
    void impl_throwError(const char* pErrorId)
    {
        ::connectivity::SharedResources aResources;
        const OUString sError( aResources.getResourceString(pErrorId) );
        ::dbtools::throwGenericSQLException(sError,nullptr);
    }
}

IMPLEMENT_SERVICE_INFO(MacabStatement, "com.sun.star.sdbc.drivers.MacabStatement", "com.sun.star.sdbc.Statement");

MacabCommonStatement::MacabCommonStatement(MacabConnection* _pConnection )
    : MacabCommonStatement_BASE(m_aMutex),
    OPropertySetHelper(rBHelper),
    m_aParser(_pConnection->getDriver()->getComponentContext()),
    m_aSQLIterator(_pConnection, _pConnection->createCatalog()->getTables(), m_aParser ),
    m_pParseTree(nullptr),
    m_pConnection(_pConnection)
{
    m_pConnection->acquire();
}

MacabCommonStatement::~MacabCommonStatement()
{
}

void MacabCommonStatement::resetParameters() const
{
}

void MacabCommonStatement::getNextParameter(OUString &) const
{
    impl_throwError(STR_PARA_ONLY_PREPARED);
}

MacabCondition *MacabCommonStatement::analyseWhereClause(const OSQLParseNode *pParseNode) const
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
                        return new MacabConditionConstant(pLeft->getTokenValue() == pRight->getTokenValue());

                    case SQLNodeType::NotEqual:
                        // WHERE 0 <> 1
                        // (might not be correct SQL... don't care, handling anyway)
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

                    if (pRight->isToken())                      // WHERE Name = 'Doe'
                        sMatchString = pRight->getTokenValue();
                    else if (SQL_ISRULE(pRight, parameter))     // WHERE Name = ?
                        getNextParameter(sMatchString);

                    switch (pMiddle->getNodeType())
                    {
                        case SQLNodeType::Equal:
                            // WHERE Name = 'Smith'
                            return new MacabConditionEqual(m_pHeader, sColumnName, sMatchString);

                         case SQLNodeType::NotEqual:
                            // WHERE Name <> 'Jones'
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
                // WHERE Name = 'Smith' OR Name = 'Jones'
                return new MacabConditionOr(
                    analyseWhereClause(pLeft),
                    analyseWhereClause(pRight));
            }
        }
        else if (SQL_ISRULE(pParseNode, boolean_term))
        {
            if (SQL_ISTOKEN(pMiddle, AND))
            {
                // WHERE Name = 'Smith' AND "Given Name" = 'Peter'
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
                    // WHERE "Mobile Phone" IS NOT NULL
                    return new MacabConditionNotNull(m_pHeader, sColumnName);
                }
                else
                {
                    // WHERE "Mobile Phone" IS NULL
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

                    if (pMiddleRight->isToken())                    // WHERE Name LIKE 'Sm%'
                        sMatchString = pMiddleRight->getTokenValue();
                    else if (SQL_ISRULE(pMiddleRight, parameter))   // WHERE Name LIKE ?
                        getNextParameter(sMatchString);

                    return new MacabConditionSimilar(m_pHeader, sColumnName, sMatchString);
                }
            }
        }
    }
    impl_throwError(STR_QUERY_TOO_COMPLEX);
    // Unreachable:
    OSL_ASSERT(false);
    return nullptr;
}

MacabOrder *MacabCommonStatement::analyseOrderByClause(const OSQLParseNode *pParseNode) const
{
    if (SQL_ISRULE(pParseNode, ordering_spec_commalist))
    {
        MacabComplexOrder *list = new MacabComplexOrder();
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

                    return new MacabSimpleOrder(m_pHeader, sColumnName, bAscending);
                }
            }
        }
    }
    impl_throwError(STR_QUERY_TOO_COMPLEX);
    // Unreachable:
    OSL_ASSERT(false);
    return nullptr;
}

OUString MacabCommonStatement::getTableName() const
{
    const OSQLTables& xTabs = m_aSQLIterator.getTables();

    if( xTabs.empty() )
        return OUString();

    // can only deal with one table at a time
    if(xTabs.size() > 1 || m_aSQLIterator.hasErrors() )
        return OUString();

    return xTabs.begin()->first;
}

void MacabCommonStatement::setMacabFields(MacabResultSet *pResult) const
{
    ::rtl::Reference<connectivity::OSQLColumns> xColumns;   // selected columns
    rtl::Reference<MacabResultSetMetaData> pMeta;           // meta information - holds the list of AddressBook fields

    xColumns = m_aSQLIterator.getSelectColumns();
    if (!xColumns.is())
    {
        ::connectivity::SharedResources aResources;
        const OUString sError( aResources.getResourceString(
                STR_INVALID_COLUMN_SELECTION
             ) );
        ::dbtools::throwGenericSQLException(sError,nullptr);
    }
    pMeta = static_cast<MacabResultSetMetaData *>(pResult->getMetaData().get());
    pMeta->setMacabFields(xColumns);
}

void MacabCommonStatement::selectRecords(MacabResultSet *pResult) const
{
    const OSQLParseNode *pParseNode;

    pParseNode = m_aSQLIterator.getWhereTree();
    if (pParseNode != nullptr)
    {
        if (SQL_ISRULE(pParseNode, where_clause))
        {
            resetParameters();
            pParseNode = pParseNode->getChild(1);
            MacabCondition *pCondition = analyseWhereClause(pParseNode);
            if (pCondition->isAlwaysTrue())
                pResult->allMacabRecords();
            else
                pResult->someMacabRecords(pCondition);
            delete pCondition;
            return;
        }
    }

    // no WHERE clause: get all rows
    pResult->allMacabRecords();
}

void MacabCommonStatement::sortRecords(MacabResultSet *pResult) const
{
    const OSQLParseNode *pParseNode;

    pParseNode = m_aSQLIterator.getOrderTree();
    if (pParseNode != nullptr)
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

Any SAL_CALL MacabCommonStatement::queryInterface( const Type & rType )
{
    Any aRet = MacabCommonStatement_BASE::queryInterface(rType);
    if (!aRet.hasValue())
        aRet = OPropertySetHelper::queryInterface(rType);
    return aRet;
}

Sequence< Type > SAL_CALL MacabCommonStatement::getTypes(  )
{
    ::cppu::OTypeCollection aTypes( cppu::UnoType<XMultiPropertySet>::get(),
                                    cppu::UnoType<XFastPropertySet>::get(),
                                    cppu::UnoType<XPropertySet>::get());

    return comphelper::concatSequences(aTypes.getTypes(),MacabCommonStatement_BASE::getTypes());
}

void SAL_CALL MacabCommonStatement::cancel(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );

    checkDisposed(rBHelper.bDisposed);
    // cancel the current sql statement
}

void SAL_CALL MacabCommonStatement::close(  )
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(rBHelper.bDisposed);

    }
    dispose();
}

sal_Bool SAL_CALL MacabCommonStatement::execute(
        const OUString& sql )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(rBHelper.bDisposed);

    Reference< XResultSet > xRS = executeQuery(sql);

    return xRS.is();
}

Reference< XResultSet > SAL_CALL MacabCommonStatement::executeQuery(
        const OUString& sql )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(rBHelper.bDisposed);

    rtl::Reference<MacabResultSet> pResult = new MacabResultSet(this);
    OUString aErr;

    m_pParseTree = m_aParser.parseTree(aErr, sql).release();
    if (m_pParseTree == nullptr)
        throw SQLException(aErr, *this, aErr, 0, Any());

    m_aSQLIterator.setParseTree(m_pParseTree);
    m_aSQLIterator.traverseAll();
    switch (m_aSQLIterator.getStatementType())
    {
        case OSQLStatementType::Select:
            {
            OUString sTableName = getTableName(); // FROM which table ?
            if (sTableName.getLength() != 0) // a match
            {
                MacabRecords *aRecords;
                aRecords = m_pConnection->getAddressBook()->getMacabRecords(sTableName);

                // In case, somehow, we don't have anything with the name m_sTableName
                if(aRecords == nullptr)
                {
                    impl_throwError(STR_NO_TABLE);
                }
                else
                {
                    m_pHeader = aRecords->getHeader();

                    pResult->setTableName(sTableName);

                    setMacabFields(pResult.get());        // SELECT which columns ?
                    selectRecords(pResult.get()); // WHERE which condition ?
                    sortRecords(pResult.get());   // ORDER BY which columns ?
                }
// To be continued: DISTINCT
//                  etc...
            }
            }
            break;

        default:
// To be continued: UPDATE
//                  DELETE
//                  etc...
            impl_throwError(STR_QUERY_TOO_COMPLEX);
    }

    m_xResultSet = Reference<XResultSet>(pResult);
    return pResult;
}

Reference< XConnection > SAL_CALL MacabCommonStatement::getConnection(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(rBHelper.bDisposed);

    // just return our connection here
    return m_pConnection;
}

sal_Int32 SAL_CALL MacabCommonStatement::executeUpdate( const OUString& )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(rBHelper.bDisposed);

    // the return values gives information about how many rows are affected by executing the sql statement
    return 0;
}

Any SAL_CALL MacabCommonStatement::getWarnings(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(rBHelper.bDisposed);

    return makeAny(m_aLastWarning);
}

void SAL_CALL MacabCommonStatement::clearWarnings(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(rBHelper.bDisposed);

    m_aLastWarning = SQLWarning();
}

::cppu::IPropertyArrayHelper* MacabCommonStatement::createArrayHelper( ) const
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

::cppu::IPropertyArrayHelper & MacabCommonStatement::getInfoHelper()
{
    return *getArrayHelper();
}

sal_Bool MacabCommonStatement::convertFastPropertyValue(
        Any &,
        Any &,
        sal_Int32,
        const Any&)
{
    // here we have to try to convert
    return false;
}

void MacabCommonStatement::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any&)
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

Reference< css::beans::XPropertySetInfo > SAL_CALL MacabCommonStatement::getPropertySetInfo(  )
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}

MacabStatement::MacabStatement(MacabConnection* _pConnection)
    : MacabStatement_BASE(_pConnection)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
