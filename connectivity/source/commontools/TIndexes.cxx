/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"
#include "connectivity/TIndexes.hxx"
#include "connectivity/TIndex.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/IndexType.hpp>
#include <connectivity/dbtools.hxx>
#include "connectivity/TTableHelper.hxx"
#include "TConnection.hxx"
#include <comphelper/extract.hxx>
#include <rtl/ustrbuf.hxx>
using namespace connectivity;
using namespace connectivity::sdbcx;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace cppu;

typedef connectivity::sdbcx::OCollection OCollection_TYPE;
// -----------------------------------------------------------------------------
OIndexesHelper::OIndexesHelper(OTableHelper* _pTable,
                 ::osl::Mutex& _rMutex,
             const ::std::vector< ::rtl::OUString> &_rVector
             )
    : OCollection(*_pTable,sal_True,_rMutex,_rVector)
    ,m_pTable(_pTable)
{
}
// -----------------------------------------------------------------------------

sdbcx::ObjectType OIndexesHelper::createObject(const ::rtl::OUString& _rName)
{
    Reference< XConnection> xConnection = m_pTable->getConnection();
    if ( !xConnection.is() )
        return NULL;

    sdbcx::ObjectType xRet;
    ::rtl::OUString aName,aQualifier;
    sal_Int32 nLen = _rName.indexOf('.');
    if ( nLen != -1 )
    {
        aQualifier  = _rName.copy(0,nLen);
        aName       = _rName.copy(nLen+1);
    }
    else
        aName       = _rName;

    ::dbtools::OPropertyMap& rPropMap = OMetaConnection::getPropMap();
    ::rtl::OUString aSchema,aTable;
    m_pTable->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_SCHEMANAME)) >>= aSchema;
    m_pTable->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_NAME))       >>= aTable;

    Any aCatalog = m_pTable->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_CATALOGNAME));
    Reference< XResultSet > xResult = m_pTable->getMetaData()->getIndexInfo(aCatalog,aSchema,aTable,sal_False,sal_False);

    if ( xResult.is() )
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        while( xResult->next() )
        {
            sal_Bool bUnique = !xRow->getBoolean(4);
            if((!aQualifier.getLength() || xRow->getString(5) == aQualifier ) && xRow->getString(6) == aName)
            {
                sal_Int32 nClustered = xRow->getShort(7);
                sal_Bool bPrimarKeyIndex = sal_False;
                xRow.clear();
                xResult.clear();
                try
                {
                    xResult = m_pTable->getMetaData()->getPrimaryKeys(aCatalog,aSchema,aTable);
                    xRow.set(xResult,UNO_QUERY);

                    if ( xRow.is() && xResult->next() ) // there can be only one primary key
                    {
                        bPrimarKeyIndex = xRow->getString(6) == aName;
                    }
                }
                catch(Exception)
                {
                }
                OIndexHelper* pRet = new OIndexHelper(m_pTable,aName,aQualifier,bUnique,
                    bPrimarKeyIndex,
                    nClustered == IndexType::CLUSTERED);
                xRet = pRet;
                break;
            }
        }
    }

    return xRet;
}
// -------------------------------------------------------------------------
void OIndexesHelper::impl_refresh() throw(RuntimeException)
{
    m_pTable->refreshIndexes();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OIndexesHelper::createDescriptor()
{
    return new OIndexHelper(m_pTable);
}
// -------------------------------------------------------------------------
// XAppend
sdbcx::ObjectType OIndexesHelper::appendObject( const ::rtl::OUString& _rForName, const Reference< XPropertySet >& descriptor )
{
    Reference< XConnection> xConnection = m_pTable->getConnection();
    if ( !xConnection.is() )
        return NULL;
    if ( m_pTable->isNew() )
        return cloneDescriptor( descriptor );

    if ( m_pTable->getIndexService().is() )
    {
        m_pTable->getIndexService()->addIndex(m_pTable,descriptor);
    }
    else
    {
        ::dbtools::OPropertyMap& rPropMap = OMetaConnection::getPropMap();
        ::rtl::OUStringBuffer aSql( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CREATE ")));
        ::rtl::OUString aQuote  = m_pTable->getMetaData()->getIdentifierQuoteString(  );
        ::rtl::OUString aDot( RTL_CONSTASCII_USTRINGPARAM( "." ));

        if(comphelper::getBOOL(descriptor->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_ISUNIQUE))))
            aSql.appendAscii("UNIQUE ");
        aSql.appendAscii("INDEX ");


        ::rtl::OUString aCatalog,aSchema,aTable;
        dbtools::qualifiedNameComponents(m_pTable->getMetaData(),m_pTable->getName(),aCatalog,aSchema,aTable,::dbtools::eInDataManipulation);
        ::rtl::OUString aComposedName;

        aComposedName = dbtools::composeTableName(m_pTable->getMetaData(),aCatalog,aSchema,aTable,sal_True,::dbtools::eInIndexDefinitions);
        if ( _rForName.getLength() )
        {
            aSql.append( ::dbtools::quoteName( aQuote, _rForName ) );
            aSql.appendAscii(" ON ");
            aSql.append(aComposedName);
            aSql.appendAscii(" ( ");

            Reference<XColumnsSupplier> xColumnSup(descriptor,UNO_QUERY);
            Reference<XIndexAccess> xColumns(xColumnSup->getColumns(),UNO_QUERY);
            Reference< XPropertySet > xColProp;
            sal_Bool bAddIndexAppendix = ::dbtools::getBooleanDataSourceSetting( m_pTable->getConnection(), "AddIndexAppendix" );
            sal_Int32 nCount = xColumns->getCount();
            for(sal_Int32 i = 0 ; i < nCount; ++i)
            {
                xColProp.set(xColumns->getByIndex(i),UNO_QUERY);
                aSql.append(::dbtools::quoteName( aQuote,comphelper::getString(xColProp->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_NAME)))));

                if ( bAddIndexAppendix )
                {

                    aSql.appendAscii(any2bool(xColProp->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_ISASCENDING)))
                                                ?
                                    " ASC"
                                                :
                                    " DESC");
                }
                aSql.appendAscii(",");
            }
            aSql.setCharAt(aSql.getLength()-1,')');
        }
        else
        {
            aSql.append(aComposedName);

            Reference<XColumnsSupplier> xColumnSup(descriptor,UNO_QUERY);
            Reference<XIndexAccess> xColumns(xColumnSup->getColumns(),UNO_QUERY);
            Reference< XPropertySet > xColProp;
            if(xColumns->getCount() != 1)
                throw SQLException();

            xColumns->getByIndex(0) >>= xColProp;

            aSql.append(aDot);
            aSql.append(::dbtools::quoteName( aQuote,comphelper::getString(xColProp->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_NAME)))));
        }

        Reference< XStatement > xStmt = m_pTable->getConnection()->createStatement(  );
        if ( xStmt.is() )
        {
            ::rtl::OUString sSql = aSql.makeStringAndClear();
            xStmt->execute(sSql);
            ::comphelper::disposeComponent(xStmt);
        }
    }

    return createObject( _rForName );
}
// -------------------------------------------------------------------------
// XDrop
void OIndexesHelper::dropObject(sal_Int32 /*_nPos*/,const ::rtl::OUString _sElementName)
{
    Reference< XConnection> xConnection = m_pTable->getConnection();
    if( xConnection.is() && !m_pTable->isNew())
    {
        if ( m_pTable->getIndexService().is() )
        {
            m_pTable->getIndexService()->dropIndex(m_pTable,_sElementName);
        }
        else
        {
            ::rtl::OUString aName,aSchema;
            sal_Int32 nLen = _sElementName.indexOf('.');
            if(nLen != -1)
                aSchema = _sElementName.copy(0,nLen);
            aName   = _sElementName.copy(nLen+1);

            ::rtl::OUString aSql( RTL_CONSTASCII_USTRINGPARAM( "DROP INDEX " ));

            ::rtl::OUString aComposedName = dbtools::composeTableName( m_pTable->getMetaData(), m_pTable, ::dbtools::eInIndexDefinitions, false, false, true );
            ::rtl::OUString sIndexName,sTemp;
            sIndexName = dbtools::composeTableName( m_pTable->getMetaData(), sTemp, aSchema, aName, sal_True, ::dbtools::eInIndexDefinitions );

            aSql += sIndexName
                    + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" ON "))
                        + aComposedName;

            Reference< XStatement > xStmt = m_pTable->getConnection()->createStatement(  );
            if ( xStmt.is() )
            {
                xStmt->execute(aSql);
                ::comphelper::disposeComponent(xStmt);
            }
        }
    }
}
// -----------------------------------------------------------------------------



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
