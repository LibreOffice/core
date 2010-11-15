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
#include "adabas/BIndexes.hxx"
#include "adabas/BIndex.hxx"
#include "adabas/BTable.hxx"
#include "connectivity/sdbcx/VIndex.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/IndexType.hpp>
#include <comphelper/types.hxx>
#include <comphelper/types.hxx>
#include "adabas/BCatalog.hxx"
#include "connectivity/dbexception.hxx"


using namespace ::comphelper;
using namespace connectivity;
using namespace connectivity::adabas;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
typedef connectivity::sdbcx::OCollection OCollection_TYPE;

sdbcx::ObjectType OIndexes::createObject(const ::rtl::OUString& _rName)
{
    ::rtl::OUString aName,aQualifier;
    sal_Int32 nLen = _rName.indexOf('.');
    if(nLen != -1)
    {
        aQualifier  = _rName.copy(0,nLen);
        aName       = _rName.copy(nLen+1);
    }
    else
        aName       = _rName;


    Reference< XResultSet > xResult = m_pTable->getMetaData()->getIndexInfo(Any(),
        m_pTable->getSchema(),m_pTable->getTableName(),sal_False,sal_False);

    sdbcx::ObjectType xRet = NULL;
    if(xResult.is())
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        while(xResult->next())
        {
            if(xRow->getString(6) == aName && (!aQualifier.getLength() || xRow->getString(5) == aQualifier ))
            {
                OAdabasIndex* pRet = new OAdabasIndex(m_pTable,aName,aQualifier,!xRow->getBoolean(4),
                    aName == ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SYSPRIMARYKEYINDEX")),
                    xRow->getShort(7) == IndexType::CLUSTERED);
                xRet = pRet;
                break;
            }
        }
        ::comphelper::disposeComponent(xResult);
    }

    return xRet;
}
// -------------------------------------------------------------------------
void OIndexes::impl_refresh() throw(RuntimeException)
{
    m_pTable->refreshIndexes();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OIndexes::createDescriptor()
{
    return new OAdabasIndex(m_pTable);
}
// -------------------------------------------------------------------------
// XAppend
sdbcx::ObjectType OIndexes::appendObject( const ::rtl::OUString& _rForName, const Reference< XPropertySet >& descriptor )
{
    if ( m_pTable->isNew() )
        ::dbtools::throwFunctionSequenceException(static_cast<XTypeProvider*>(this));

    ::rtl::OUString aSql( RTL_CONSTASCII_USTRINGPARAM( "CREATE " ));
    ::rtl::OUString aQuote  = m_pTable->getMetaData()->getIdentifierQuoteString(  );
    const ::rtl::OUString& sDot = OAdabasCatalog::getDot();

    if(getBOOL(descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISUNIQUE))))
        aSql = aSql + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("UNIQUE "));
    aSql = aSql + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("INDEX "));


    if(_rForName.getLength())
    {
        aSql = aSql + aQuote + _rForName + aQuote
                    + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" ON "))
                    + aQuote + m_pTable->getSchema() + aQuote + sDot
                    + aQuote + m_pTable->getTableName() + aQuote
                    + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" ( "));

        Reference<XColumnsSupplier> xColumnSup(descriptor,UNO_QUERY);
        Reference<XIndexAccess> xColumns(xColumnSup->getColumns(),UNO_QUERY);
        Reference< XPropertySet > xColProp;
        sal_Int32 nCount = xColumns->getCount();
        for(sal_Int32 i=0;i<nCount;++i)
        {
            xColumns->getByIndex(i) >>= xColProp;
            aSql = aSql + aQuote + getString(xColProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME))) + aQuote;
            aSql = aSql +   (getBOOL(xColProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISASCENDING)))
                                        ?
                            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" ASC"))
                                        :
                            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" DESC")))
                        +   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(","));
        }
        aSql = aSql.replaceAt(aSql.getLength()-1,1,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(")")));
    }
    else
    {
        aSql = aSql + aQuote + m_pTable->getSchema() + aQuote + sDot + aQuote + m_pTable->getTableName() + aQuote;

        Reference<XColumnsSupplier> xColumnSup(descriptor,UNO_QUERY);
        Reference<XIndexAccess> xColumns(xColumnSup->getColumns(),UNO_QUERY);
        Reference< XPropertySet > xColProp;
        if(xColumns->getCount() != 1)
            throw SQLException();

        xColumns->getByIndex(0) >>= xColProp;

        aSql = aSql + sDot + aQuote + getString(xColProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME))) + aQuote;
    }

    Reference< XStatement > xStmt = m_pTable->getConnection()->createStatement(  );
    xStmt->execute(aSql);
    ::comphelper::disposeComponent(xStmt);

    return createObject( _rForName );
}
// -------------------------------------------------------------------------
// XDrop
void OIndexes::dropObject(sal_Int32 /*_nPos*/,const ::rtl::OUString _sElementName)
{
    if(!m_pTable->isNew())
    {
        ::rtl::OUString aName,aSchema;
        sal_Int32 nLen = _sElementName.indexOf('.');
        aSchema = _sElementName.copy(0,nLen);
        aName   = _sElementName.copy(nLen+1);

        ::rtl::OUString aSql( RTL_CONSTASCII_USTRINGPARAM( "DROP INDEX " ));
        ::rtl::OUString aQuote  = m_pTable->getMetaData()->getIdentifierQuoteString(  );
        const ::rtl::OUString& sDot = OAdabasCatalog::getDot();

        if (aSchema.getLength())
            (((aSql += aQuote) += aSchema) += aQuote) += sDot;

        (((aSql += aQuote) += aName) += aQuote) += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" ON "));

        (((aSql += aQuote) += m_pTable->getSchema()) += aQuote) += sDot;
        ((aSql += aQuote) += m_pTable->getTableName()) += aQuote;

        Reference< XStatement > xStmt = m_pTable->getConnection()->createStatement(  );
        xStmt->execute(aSql);
        ::comphelper::disposeComponent(xStmt);
    }
}
// -------------------------------------------------------------------------


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
