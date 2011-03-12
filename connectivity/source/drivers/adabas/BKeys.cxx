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
#include "adabas/BKeys.hxx"
#include "adabas/BTable.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include <com/sun/star/sdbc/KeyRule.hpp>
#include <comphelper/types.hxx>
#include "adabas/BCatalog.hxx"
#include <comphelper/property.hxx>
#include <connectivity/TKeys.hxx>
#include <connectivity/dbtools.hxx>


using namespace ::comphelper;
using namespace connectivity;
using namespace connectivity::adabas;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
typedef OKeysHelper OCollection_TYPE;

// -------------------------------------------------------------------------
// XAppend
sdbcx::ObjectType OKeys::appendObject( const ::rtl::OUString& _rForName, const Reference< XPropertySet >& descriptor )
{
    if ( getTable()->isNew() )
    {
        Reference< XPropertySet > xNewDescriptor( cloneDescriptor( descriptor ) );
        OKeysHelper::cloneDescriptorColumns( descriptor, xNewDescriptor );
        return xNewDescriptor;
    }

    sal_Int32 nKeyType      = getINT32(descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE)));

    ::rtl::OUString aSql( RTL_CONSTASCII_USTRINGPARAM( "ALTER TABLE " ));
    const ::rtl::OUString aQuote    = getTable()->getConnection()->getMetaData()->getIdentifierQuoteString(  );
    const ::rtl::OUString& sDot = OAdabasCatalog::getDot();

    aSql += composeTableName( getTable()->getConnection()->getMetaData(), getTable(), ::dbtools::eInTableDefinitions, false, false, true );

    if(nKeyType == KeyType::PRIMARY)
    {
        aSql = aSql + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" ALTER PRIMARY KEY ("));
    }
    else if(nKeyType == KeyType::FOREIGN)
    {
        aSql = aSql + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" FOREIGN KEY ("));
    }
    else
        throw SQLException();

    Reference<XColumnsSupplier> xColumnSup(descriptor,UNO_QUERY);
    Reference<XIndexAccess> xColumns(xColumnSup->getColumns(),UNO_QUERY);

    for(sal_Int32 i=0;i<xColumns->getCount();++i)
    {
        Reference< XPropertySet > xColProp;
        xColumns->getByIndex(i) >>= xColProp;
        aSql = aSql + aQuote + getString(xColProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME))) + aQuote
                    +   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(","));
    }
    aSql = aSql.replaceAt(aSql.getLength()-1,1,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(")")));

    sal_Int32 nUpdateRule = 0, nDeleteRule = 0;
    ::rtl::OUString sReferencedName;

    if(nKeyType == KeyType::FOREIGN)
    {
        nDeleteRule = getINT32(descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DELETERULE)));

        ::rtl::OUString aName,aSchema;
        sReferencedName = getString(descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_REFERENCEDTABLE)));
        sal_Int32 nLen = sReferencedName.indexOf('.');
        aSchema = sReferencedName.copy(0,nLen);
        aName   = sReferencedName.copy(nLen+1);
        aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" REFERENCES "))
                    + aQuote + aSchema + aQuote + sDot + aQuote + aName + aQuote;
        aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" ("));

        for(sal_Int32 i=0;i<xColumns->getCount();++i)
        {
            Reference< XPropertySet > xColProp;
            xColumns->getByIndex(i) >>= xColProp;
            aSql = aSql + aQuote + getString(xColProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RELATEDCOLUMN))) + aQuote
                        +   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(","));
        }
        aSql = aSql.replaceAt(aSql.getLength()-1,1,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(")")));

        switch(nDeleteRule)
        {
            case KeyRule::CASCADE:
                aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" ON DELETE CASCADE "));
                break;
            case KeyRule::RESTRICT:
                aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" ON DELETE RESTRICT "));
                break;
            case KeyRule::SET_NULL:
                aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" ON DELETE SET NULL "));
                break;
            case KeyRule::SET_DEFAULT:
                aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" ON DELETE SET DEFAULT "));
                break;
            default:
                ;
        }
    }

    Reference< XStatement > xStmt = getTable()->getConnection()->createStatement(  );
    xStmt->execute(aSql);
    ::comphelper::disposeComponent(xStmt);
    // find the name which the database gave the new key
    ::rtl::OUString sNewName( _rForName );
    if(nKeyType == KeyType::FOREIGN)
    {
        const ::dbtools::OPropertyMap& rPropMap = OMetaConnection::getPropMap();
        ::rtl::OUString aSchema,aTable;
        getTable()->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_SCHEMANAME))   >>= aSchema;
        getTable()->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_NAME))     >>= aTable;
        Reference< XResultSet > xResult = getTable()->getMetaData()->getImportedKeys( getTable()->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_CATALOGNAME))
                                                                                    ,aSchema
                                                                                    ,aTable);
        if(xResult.is())
        {
            Reference< XRow > xRow(xResult,UNO_QUERY);
            while(xResult->next())
            {
                ::rtl::OUString sName = xRow->getString(12);
                if ( !m_pElements->exists(sName) ) // this name wasn't inserted yet so it must be te new one
                {
                    descriptor->setPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME),makeAny(sName));
                    sNewName = sName;
                    break;
                }
            }
            ::comphelper::disposeComponent(xResult);
        }
    }

    getTable()->addKey(sNewName,sdbcx::TKeyProperties(new sdbcx::KeyProperties(sReferencedName,nKeyType,nUpdateRule,nDeleteRule)));
    return createObject( sNewName );
}
// -------------------------------------------------------------------------
::rtl::OUString OKeys::getDropForeignKey() const
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" DROP FOREIGN KEY "));
}

// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
