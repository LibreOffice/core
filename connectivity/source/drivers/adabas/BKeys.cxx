/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: BKeys.cxx,v $
 * $Revision: 1.24 $
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
#ifndef _CONNECTIVITY_ADABAS_INDEX_HXX_
#include "adabas/BKey.hxx"
#endif
#include "adabas/BTable.hxx"
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbcx/KeyType.hpp>
#include <com/sun/star/sdbc/KeyRule.hpp>
#include <comphelper/types.hxx>
#include "adabas/BCatalog.hxx"
#include <comphelper/property.hxx>
#include <connectivity/TKeys.hxx>


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

// -------------------------------------------------------------------------
sdbcx::ObjectType OKeys::createObject(const ::rtl::OUString& _rName)
{
    sdbcx::ObjectType xRet = NULL;

    if(_rName.getLength())
    {
        Reference< XResultSet > xResult = m_pTable->getMetaData()->getImportedKeys(Any(),
            m_pTable->getSchema(),m_pTable->getTableName());

        if(xResult.is())
        {
            Reference< XRow > xRow(xResult,UNO_QUERY);
            ::rtl::OUString sName;
            const ::rtl::OUString& sDot = OAdabasCatalog::getDot();
            while(xResult->next())
            {
                sName = xRow->getString(2);
                if(sName.getLength())
                    sName += sDot;
                sName += xRow->getString(3);
                sal_Int32 nUpdateRule = xRow->getInt(10);
                if(xRow->wasNull())
                    nUpdateRule = KeyRule::NO_ACTION;

                sal_Int32 nDeleteRule = xRow->getInt(11);
                if(xRow->wasNull())
                    nDeleteRule = KeyRule::NO_ACTION;
                if(xRow->getString(12) == _rName)
                {
                    OAdabasKey* pRet = new OAdabasKey(m_pTable,_rName,sName,KeyType::FOREIGN,nUpdateRule,nDeleteRule);
                    xRet = pRet;
                    break;
                }
            }
            ::comphelper::disposeComponent(xResult);
        }
    }
    else
        xRet = new OAdabasKey(m_pTable,_rName,::rtl::OUString(),KeyType::PRIMARY,KeyRule::NO_ACTION,KeyRule::NO_ACTION);

    return xRet;
}
// -------------------------------------------------------------------------
void OKeys::impl_refresh() throw(RuntimeException)
{
    m_pTable->refreshKeys();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OKeys::createDescriptor()
{
    return new OAdabasKey(m_pTable);
}
// -------------------------------------------------------------------------
// XAppend
sdbcx::ObjectType OKeys::appendObject( const ::rtl::OUString& _rForName, const Reference< XPropertySet >& descriptor )
{
    if ( m_pTable->isNew() )
    {
        Reference< XPropertySet > xNewDescriptor( cloneDescriptor( descriptor ) );
        OKeysHelper::cloneDescriptorColumns( descriptor, xNewDescriptor );
        return xNewDescriptor;
    }

    sal_Int32 nKeyType      = getINT32(descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE)));

    ::rtl::OUString aSql    = ::rtl::OUString::createFromAscii("ALTER TABLE ");
    ::rtl::OUString aQuote  = m_pTable->getMetaData()->getIdentifierQuoteString(  );
    const ::rtl::OUString& sDot = OAdabasCatalog::getDot();

    aSql = aSql + aQuote + m_pTable->getSchema() + aQuote + sDot + aQuote + m_pTable->getTableName() + aQuote;
    if(nKeyType == KeyType::PRIMARY)
    {
        aSql = aSql + ::rtl::OUString::createFromAscii(" ALTER PRIMARY KEY (");
    }
    else if(nKeyType == KeyType::FOREIGN)
    {
        aSql = aSql + ::rtl::OUString::createFromAscii(" FOREIGN KEY (");
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
                    +   ::rtl::OUString::createFromAscii(",");
    }
    aSql = aSql.replaceAt(aSql.getLength()-1,1,::rtl::OUString::createFromAscii(")"));

    if(nKeyType == KeyType::FOREIGN)
    {
        sal_Int32 nDeleteRule   = getINT32(descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DELETERULE)));

        ::rtl::OUString aName,aSchema,aRefTable = getString(descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_REFERENCEDTABLE)));
        sal_Int32 nLen = aRefTable.indexOf('.');
        aSchema = aRefTable.copy(0,nLen);
        aName   = aRefTable.copy(nLen+1);
        aSql += ::rtl::OUString::createFromAscii(" REFERENCES ")
                    + aQuote + aSchema + aQuote + sDot + aQuote + aName + aQuote;
        aSql += ::rtl::OUString::createFromAscii(" (");

        for(sal_Int32 i=0;i<xColumns->getCount();++i)
        {
            Reference< XPropertySet > xColProp;
            xColumns->getByIndex(i) >>= xColProp;
            aSql = aSql + aQuote + getString(xColProp->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RELATEDCOLUMN))) + aQuote
                        +   ::rtl::OUString::createFromAscii(",");
        }
        aSql = aSql.replaceAt(aSql.getLength()-1,1,::rtl::OUString::createFromAscii(")"));

        switch(nDeleteRule)
        {
            case KeyRule::CASCADE:
                aSql += ::rtl::OUString::createFromAscii(" ON DELETE CASCADE ");
                break;
            case KeyRule::RESTRICT:
                aSql += ::rtl::OUString::createFromAscii(" ON DELETE RESTRICT ");
                break;
            case KeyRule::SET_NULL:
                aSql += ::rtl::OUString::createFromAscii(" ON DELETE SET NULL ");
                break;
            case KeyRule::SET_DEFAULT:
                aSql += ::rtl::OUString::createFromAscii(" ON DELETE SET DEFAULT ");
                break;
            default:
                ;
        }
    }

    Reference< XStatement > xStmt = m_pTable->getConnection()->createStatement(  );
    xStmt->execute(aSql);
    ::comphelper::disposeComponent(xStmt);
    // find the name which the database gave the new key
    ::rtl::OUString sNewName( _rForName );
    if(nKeyType == KeyType::FOREIGN)
    {
        Reference< XResultSet > xResult = m_pTable->getMetaData()->getImportedKeys(Any(),m_pTable->getSchema(),m_pTable->getTableName());
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

    return createObject( sNewName );
}
// -------------------------------------------------------------------------
// XDrop
void OKeys::dropObject(sal_Int32 _nPos,const ::rtl::OUString _sElementName)
{
    if(!m_pTable->isNew())
    {
        ::rtl::OUString aSql    = ::rtl::OUString::createFromAscii("ALTER TABLE ");
        ::rtl::OUString aQuote  = m_pTable->getMetaData()->getIdentifierQuoteString(  );
        const ::rtl::OUString& sDot = OAdabasCatalog::getDot();

        Reference<XPropertySet> xKey(getObject(_nPos),UNO_QUERY);
        if ( xKey.is() )
        {
            sal_Int32 nKeyType      = getINT32(xKey->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE)));

            aSql += aQuote + m_pTable->getSchema() + aQuote + sDot + aQuote + m_pTable->getTableName() + aQuote;
            if ( nKeyType == KeyType::PRIMARY )
                aSql += ::rtl::OUString::createFromAscii(" DROP PRIMARY KEY");
            else
            {
                aSql += ::rtl::OUString::createFromAscii(" DROP FOREIGN KEY ");
                aSql += aQuote + _sElementName + aQuote;
            }

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

