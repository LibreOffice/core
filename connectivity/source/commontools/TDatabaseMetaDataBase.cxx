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

#include "TDatabaseMetaDataBase.hxx"
#include "RowFunctionParser.hxx"

#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/evtlistenerhlp.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include "resource/sharedresources.hxx"
#include "strings.hrc"
#include <connectivity/dbexception.hxx>
#include <sal/macros.h>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::beans;
using namespace comphelper;
using namespace connectivity;


ODatabaseMetaDataBase::ODatabaseMetaDataBase(const Reference< XConnection >& _rxConnection,const Sequence< PropertyValue >& _rInfo)
    : m_aConnectionInfo(_rInfo)
    ,m_isCatalogAtStart(false,false)
    ,m_sCatalogSeparator(false,OUString())
    ,m_sIdentifierQuoteString(false,OUString())
    ,m_supportsCatalogsInTableDefinitions(false,false)
    ,m_supportsSchemasInTableDefinitions(false,false)
    ,m_supportsCatalogsInDataManipulation(false,false)
    ,m_supportsSchemasInDataManipulation(false,false)
    ,m_supportsMixedCaseQuotedIdentifiers(false,false)
    ,m_supportsAlterTableWithAddColumn(false,false)
    ,m_supportsAlterTableWithDropColumn(false,false)
    ,m_MaxStatements(false,0)
    ,m_MaxTablesInSelect(false,0)
    ,m_storesMixedCaseQuotedIdentifiers(false,false)
    , m_xConnection(_rxConnection)
{
    osl_atomic_increment( &m_refCount );
    {
        m_xListenerHelper = new OEventListenerHelper(this);
        Reference<XComponent> xCom(m_xConnection,UNO_QUERY);
        if(xCom.is())
            xCom->addEventListener(m_xListenerHelper);
    }
    osl_atomic_decrement( &m_refCount );
}

ODatabaseMetaDataBase::~ODatabaseMetaDataBase()
{
}


Sequence< PropertyValue > SAL_CALL ODatabaseMetaDataBase::getConnectionInfo(  )
{
    return m_aConnectionInfo;
}


void SAL_CALL ODatabaseMetaDataBase::disposing( const EventObject& /*Source*/ )
{
    // cut off all references to the connection
m_xConnection.clear();
m_xListenerHelper.clear();
}

Reference< XResultSet > SAL_CALL ODatabaseMetaDataBase::getTypeInfo(  )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_aTypeInfoRows.empty() )
    {
        Reference< XResultSet > xRet = impl_getTypeInfo_throw();
        Reference< XRow > xRow(xRet,UNO_QUERY);
        ::comphelper::SequenceAsHashMap aMap(m_aConnectionInfo);
        Sequence< Any > aTypeInfoSettings;
        aTypeInfoSettings = aMap.getUnpackedValueOrDefault("TypeInfoSettings",aTypeInfoSettings);

        if ( xRow.is() )
        {
            static const sal_Int32 pTypes[] = {
                                        DataType::VARCHAR
                                        ,DataType::INTEGER
                                        ,DataType::INTEGER
                                        ,DataType::VARCHAR
                                        ,DataType::VARCHAR
                                        ,DataType::VARCHAR
                                        ,DataType::INTEGER
                                        ,DataType::BOOLEAN
                                        ,DataType::INTEGER
                                        ,DataType::BOOLEAN
                                        ,DataType::BOOLEAN
                                        ,DataType::BOOLEAN
                                        ,DataType::VARCHAR
                                        ,DataType::INTEGER
                                        ,DataType::INTEGER
                                        ,DataType::INTEGER
                                        ,DataType::INTEGER
                                        ,DataType::INTEGER
                                    };
            std::vector<std::shared_ptr<ExpressionNode>> aConditions;
            if ( aTypeInfoSettings.getLength() > 1 && ((aTypeInfoSettings.getLength() % 2) == 0) )
            {
                const Any* pIter = aTypeInfoSettings.getConstArray();
                const Any* pEnd  = pIter + aTypeInfoSettings.getLength();
                try
                {
                    for(;pIter != pEnd;++pIter)
                        aConditions.push_back(FunctionParser::parseFunction(::comphelper::getString(*pIter)));
                }
                catch(ParseError&)
                {
                    ::connectivity::SharedResources aResources;
                    const OUString sError( aResources.getResourceString(STR_FORMULA_WRONG));
                    ::dbtools::throwGenericSQLException(sError,*this);
                }
            }

            ::connectivity::ODatabaseMetaDataResultSet::ORows aTypeInfoRows;
            while( xRet->next() )
            {
                ::connectivity::ODatabaseMetaDataResultSet::ORow aRow;
                aRow.push_back(ODatabaseMetaDataResultSet::getEmptyValue());
                const sal_Int32* pType = pTypes;
                for (sal_Int32 i = 1; i <= sal_Int32(SAL_N_ELEMENTS(pTypes)); ++i,++pType)
                {
                    ORowSetValue aValue;
                    aValue.fill(i,*pType,xRow);
                    aRow.push_back(new ORowSetValueDecorator(aValue));
                }

                std::vector<std::shared_ptr<ExpressionNode>>::iterator aIter = aConditions.begin();
                std::vector<std::shared_ptr<ExpressionNode>>::const_iterator aEnd = aConditions.end();
                for (; aIter != aEnd; ++aIter)
                {
                    if ( (*aIter)->evaluate(aRow)->getValue().getBool() )
                    {
                        ++aIter;
                        (*aIter)->fill(aRow);
                    }
                    else
                        ++aIter;
                }
                aTypeInfoRows.push_back(aRow);
            }
            m_aTypeInfoRows = aTypeInfoRows;
        }
    }
    ::connectivity::ODatabaseMetaDataResultSet* pResult = new ::connectivity::ODatabaseMetaDataResultSet(::connectivity::ODatabaseMetaDataResultSet::eTypeInfo);
    Reference< XResultSet > xRet = pResult;
    pResult->setRows(m_aTypeInfoRows);
    return xRet;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaDataBase::getExportedKeys(
        const Any& /*catalog*/, const OUString& /*schema*/, const OUString& /*table*/ )
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eExportedKeys );
}

Reference< XResultSet > SAL_CALL ODatabaseMetaDataBase::getImportedKeys(
        const Any& /*catalog*/, const OUString& /*schema*/, const OUString& /*table*/ )
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eImportedKeys );
}

Reference< XResultSet > SAL_CALL ODatabaseMetaDataBase::getPrimaryKeys(
        const Any& /*catalog*/, const OUString& /*schema*/, const OUString& /*table*/ )
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::ePrimaryKeys );
}

Reference< XResultSet > SAL_CALL ODatabaseMetaDataBase::getIndexInfo(
        const Any& /*catalog*/, const OUString& /*schema*/, const OUString& /*table*/,
        sal_Bool /*unique*/, sal_Bool /*approximate*/ )
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eIndexInfo );
}

Reference< XResultSet > SAL_CALL ODatabaseMetaDataBase::getBestRowIdentifier(
        const Any& /*catalog*/, const OUString& /*schema*/, const OUString& /*table*/, sal_Int32 /*scope*/,
        sal_Bool /*nullable*/ )
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eBestRowIdentifier );
}

Reference< XResultSet > SAL_CALL ODatabaseMetaDataBase::getCrossReference(
        const Any& /*primaryCatalog*/, const OUString& /*primarySchema*/,
        const OUString& /*primaryTable*/, const Any& /*foreignCatalog*/,
        const OUString& /*foreignSchema*/, const OUString& /*foreignTable*/ )
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eCrossReference );
}

Reference< XConnection > SAL_CALL ODatabaseMetaDataBase::getConnection(  )
{
    return m_xConnection;
}

Reference< XResultSet > SAL_CALL ODatabaseMetaDataBase::getProcedureColumns(
        const Any& /*catalog*/, const OUString& /*schemaPattern*/,
        const OUString& /*procedureNamePattern*/, const OUString& /*columnNamePattern*/ )
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eProcedureColumns );
}

Reference< XResultSet > SAL_CALL ODatabaseMetaDataBase::getProcedures(
        const Any& /*catalog*/, const OUString& /*schemaPattern*/,
        const OUString& /*procedureNamePattern*/ )
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eProcedures );
}

Reference< XResultSet > SAL_CALL ODatabaseMetaDataBase::getVersionColumns(
        const Any& /*catalog*/, const OUString& /*schema*/, const OUString& /*table*/ )
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eVersionColumns );
}

Reference< XResultSet > SAL_CALL ODatabaseMetaDataBase::getSchemas(  )
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eSchemas );
}

Reference< XResultSet > SAL_CALL ODatabaseMetaDataBase::getColumnPrivileges(
        const Any& /*catalog*/, const OUString& /*schema*/, const OUString& /*table*/,
        const OUString& /*columnNamePattern*/ )
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eColumnPrivileges );
}

Reference< XResultSet > SAL_CALL ODatabaseMetaDataBase::getTablePrivileges(
        const Any& /*catalog*/, const OUString& /*schema*/, const OUString& /*table*/)
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eTablePrivileges );
}

Reference< XResultSet > SAL_CALL ODatabaseMetaDataBase::getCatalogs(  )
{
    return new ODatabaseMetaDataResultSet( ODatabaseMetaDataResultSet::eCatalogs );
}

OUString SAL_CALL ODatabaseMetaDataBase::getIdentifierQuoteString(  )
{
    return callImplMethod(m_sIdentifierQuoteString,std::mem_fun_t< OUString ,ODatabaseMetaDataBase>(&ODatabaseMetaDataBase::impl_getIdentifierQuoteString_throw));
}

sal_Bool SAL_CALL ODatabaseMetaDataBase::isCatalogAtStart(  )
{
    return callImplMethod(m_isCatalogAtStart,std::mem_fun_t< bool,ODatabaseMetaDataBase>(&ODatabaseMetaDataBase::impl_isCatalogAtStart_throw));
}

OUString SAL_CALL ODatabaseMetaDataBase::getCatalogSeparator(  )
{
    return callImplMethod(m_sCatalogSeparator,std::mem_fun_t< OUString,ODatabaseMetaDataBase>(&ODatabaseMetaDataBase::impl_getCatalogSeparator_throw));
}

sal_Bool SAL_CALL ODatabaseMetaDataBase::supportsCatalogsInTableDefinitions(  )
{
    return callImplMethod(m_supportsCatalogsInTableDefinitions,std::mem_fun_t< bool,ODatabaseMetaDataBase>(&ODatabaseMetaDataBase::impl_supportsCatalogsInTableDefinitions_throw));
}

sal_Bool SAL_CALL ODatabaseMetaDataBase::supportsSchemasInTableDefinitions(  )
{
    return callImplMethod(m_supportsSchemasInTableDefinitions,std::mem_fun_t< bool,ODatabaseMetaDataBase>(&ODatabaseMetaDataBase::impl_supportsSchemasInTableDefinitions_throw));
}

sal_Bool SAL_CALL ODatabaseMetaDataBase::supportsCatalogsInDataManipulation(  )
{
    return callImplMethod(m_supportsCatalogsInDataManipulation,std::mem_fun_t< bool,ODatabaseMetaDataBase>(&ODatabaseMetaDataBase::impl_supportsCatalogsInDataManipulation_throw));
}

sal_Bool SAL_CALL ODatabaseMetaDataBase::supportsSchemasInDataManipulation(  )
{
    return callImplMethod(m_supportsSchemasInDataManipulation,std::mem_fun_t< bool,ODatabaseMetaDataBase>(&ODatabaseMetaDataBase::impl_supportsSchemasInDataManipulation_throw));
}

sal_Bool SAL_CALL ODatabaseMetaDataBase::supportsMixedCaseQuotedIdentifiers(  )
{
    return callImplMethod(m_supportsMixedCaseQuotedIdentifiers,std::mem_fun_t< bool,ODatabaseMetaDataBase>(&ODatabaseMetaDataBase::impl_supportsMixedCaseQuotedIdentifiers_throw));
}

sal_Bool SAL_CALL ODatabaseMetaDataBase::supportsAlterTableWithAddColumn(  )
{
    return callImplMethod(m_supportsAlterTableWithAddColumn,std::mem_fun_t< bool,ODatabaseMetaDataBase>(&ODatabaseMetaDataBase::impl_supportsAlterTableWithAddColumn_throw));
}

sal_Bool SAL_CALL ODatabaseMetaDataBase::supportsAlterTableWithDropColumn(  )
{
    return callImplMethod(m_supportsAlterTableWithDropColumn,std::mem_fun_t< bool,ODatabaseMetaDataBase>(&ODatabaseMetaDataBase::impl_supportsAlterTableWithDropColumn_throw));
}

sal_Int32 SAL_CALL ODatabaseMetaDataBase::getMaxStatements(  )
{
    return callImplMethod(m_MaxStatements,std::mem_fun_t< sal_Int32,ODatabaseMetaDataBase>(&ODatabaseMetaDataBase::impl_getMaxStatements_throw));
}

sal_Int32 SAL_CALL ODatabaseMetaDataBase::getMaxTablesInSelect(  )
{
    return callImplMethod(m_MaxTablesInSelect,std::mem_fun_t< sal_Int32,ODatabaseMetaDataBase>(&ODatabaseMetaDataBase::impl_getMaxTablesInSelect_throw));
}

sal_Bool SAL_CALL ODatabaseMetaDataBase::storesMixedCaseQuotedIdentifiers(  )
{
    return callImplMethod(m_storesMixedCaseQuotedIdentifiers,std::mem_fun_t< bool,ODatabaseMetaDataBase>(&ODatabaseMetaDataBase::impl_storesMixedCaseQuotedIdentifiers_throw));
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
