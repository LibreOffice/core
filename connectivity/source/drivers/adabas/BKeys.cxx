/*************************************************************************
 *
 *  $RCSfile: BKeys.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:14:20 $
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

#ifndef _CONNECTIVITY_ADABAS_KEYS_HXX_
#include "adabas/BKeys.hxx"
#endif
#ifndef _CONNECTIVITY_ADABAS_INDEX_HXX_
#include "adabas/BKey.hxx"
#endif
#ifndef _CONNECTIVITY_ADABAS_TABLE_HXX_
#include "adabas/BTable.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_KEYTYPE_HPP_
#include <com/sun/star/sdbcx/KeyType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_KEYRULE_HPP_
#include <com/sun/star/sdbc/KeyRule.hpp>
#endif
#ifndef _CONNECTIVITY_PROPERTYIDS_HXX_
#include "propertyids.hxx"
#endif
using namespace connectivity::adabas;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
typedef connectivity::sdbcx::OCollection OCollection_TYPE;

Any SAL_CALL OKeys::queryInterface( const Type & rType ) throw(RuntimeException)
{
    if(rType == ::getCppuType(static_cast< Reference<XNameAccess> *>(NULL)))
        return Any();

    return OCollection_TYPE::queryInterface(rType);
}
// -------------------------------------------------------------------------
Sequence< Type > SAL_CALL OKeys::getTypes(  ) throw(RuntimeException)
{
    Sequence< Type > aTypes(OCollection_TYPE::getTypes());
    Type* pBegin    = aTypes.getArray();
    Type* pEnd      = pBegin + aTypes.getLength();

    Sequence< Type > aRetType(aTypes.getLength()-1);
    sal_Int32 i=0;
    for(;pBegin != pEnd; ++pBegin)
    {
        if(*pBegin != ::getCppuType(static_cast< Reference<XNameAccess> *>(NULL)))
        {
            aRetType.getArray()[i++] = *pBegin;
        }
    }

    return aRetType;
}
// -------------------------------------------------------------------------
Reference< XNamed > OKeys::createObject(const ::rtl::OUString& _rName)
{
    Reference< XNamed > xRet = NULL;

    if(_rName.getLength())
    {
        Reference< XResultSet > xResult = m_pTable->getConnection()->getMetaData()->getExportedKeys(Any(),
            m_pTable->getSchema(),m_pTable->getName());

        if(xResult.is())
        {
            Reference< XRow > xRow(xResult,UNO_QUERY);
            ::rtl::OUString aName,aDot = ::rtl::OUString::createFromAscii(".");
            while(xResult->next())
            {
                if(xRow->getString(13) == _rName)
                {
                    aName = xRow->getString(6);
                    if(aName.getLength())
                        aName += aDot;
                    aName += xRow->getString(7);

                    OAdabasKey* pRet = new OAdabasKey(m_pTable,_rName,aName,KeyType::FOREIGN,xRow->getInt(10),xRow->getInt(11));
                    xRet = pRet;
                    break;
                }
            }
        }
    }
    else
    {
        OAdabasKey* pRet = new OAdabasKey(m_pTable,_rName,::rtl::OUString(),KeyType::PRIMARY,KeyRule::NO_ACTION,KeyRule::NO_ACTION);
        xRet = pRet;
    }

    return xRet;
}
// -------------------------------------------------------------------------
void OKeys::impl_refresh() throw(RuntimeException)
{
    m_pTable->refreshKeys();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OKeys::createEmptyObject()
{
    OAdabasKey* pNew = new OAdabasKey(m_pTable);
    return pNew;
}
// -------------------------------------------------------------------------
// XAppend
void SAL_CALL OKeys::appendByDescriptor( const Reference< XPropertySet >& descriptor ) throw(SQLException, ElementExistException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);
    ::rtl::OUString aName = getString(descriptor->getPropertyValue(PROPERTY_NAME));
    ObjectMap::iterator aIter = m_aNameMap.find(aName);
    if( aIter != m_aNameMap.end())
        throw ElementExistException(aName,*this);
    if(!m_pTable->isNew())
    {
        sal_Int32 nKeyType      = getINT32(descriptor->getPropertyValue(PROPERTY_TYPE));

        ::rtl::OUString aSql    = ::rtl::OUString::createFromAscii("ALTER TABLE ");
        ::rtl::OUString aQuote  = m_pTable->getConnection()->getMetaData()->getIdentifierQuoteString(  );
        ::rtl::OUString aDot    = ::rtl::OUString::createFromAscii(".");

        aSql = aSql + aQuote + m_pTable->getSchema() + aQuote + aDot + aQuote + m_pTable->getName() + aQuote;
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
        Reference< XPropertySet > xColProp;
        for(sal_Int32 i=0;i<xColumns->getCount();++i)
        {
            xColumns->getByIndex(i) >>= xColProp;
            aSql = aSql + aQuote + getString(xColProp->getPropertyValue(PROPERTY_NAME)) + aQuote
                        +   ::rtl::OUString::createFromAscii(",");
        }
        aSql = aSql.replaceAt(aSql.getLength()-1,1,::rtl::OUString::createFromAscii(")"));

        if(nKeyType == KeyType::FOREIGN)
        {
            sal_Int32 nDeleteRule   = getINT32(xColProp->getPropertyValue(PROPERTY_DELETERULE));

            ::rtl::OUString aName,aSchema,aRefTable = getString(xColProp->getPropertyValue(PROPERTY_REFERENCEDTABLE));
            sal_Int32 nLen = aRefTable.indexOf('.');
            aSchema = aRefTable.copy(0,nLen);
            aName   = aRefTable.copy(nLen+1);
            aSql = aSql + ::rtl::OUString::createFromAscii(" REFERENCES ")
                        + aQuote + aSchema + aQuote + aDot + aQuote + aName + aQuote;

            for(sal_Int32 i=0;i<xColumns->getCount();++i)
            {
                xColumns->getByIndex(i) >>= xColProp;
                aSql = aSql + aQuote + getString(xColProp->getPropertyValue(PROPERTY_REFERENCEDCOLUMN)) + aQuote
                            +   ::rtl::OUString::createFromAscii(",");
            }
            aSql = aSql.replaceAt(aSql.getLength()-1,1,::rtl::OUString::createFromAscii(")"));

            switch(nDeleteRule)
            {
                case KeyRule::CASCADE:
                    aSql = aSql + ::rtl::OUString::createFromAscii(" ON DELETE CASCADE ");
                    break;
                case KeyRule::RESTRICT:
                    aSql = aSql + ::rtl::OUString::createFromAscii(" ON DELETE RESTRICT ");
                    break;
                case KeyRule::SET_NULL:
                    aSql = aSql + ::rtl::OUString::createFromAscii(" ON DELETE SET NULL ");
                    break;
                case KeyRule::SET_DEFAULT:
                    aSql = aSql + ::rtl::OUString::createFromAscii(" ON DELETE SET DEFAULT ");
                    break;
                default:
                    ;
            }
        }

        Reference< XStatement > xStmt = m_pTable->getConnection()->createStatement(  );
        xStmt->execute(aSql);
    }
    OCollection_TYPE::appendByDescriptor(descriptor);
}
// -------------------------------------------------------------------------
// XDrop
void SAL_CALL OKeys::dropByName( const ::rtl::OUString& elementName ) throw(SQLException, NoSuchElementException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);
    ObjectMap::iterator aIter = m_aNameMap.find(elementName);
    if( aIter == m_aNameMap.end())
        throw NoSuchElementException(elementName,*this);

    if(!m_pTable->isNew())
    {
        ::rtl::OUString aSql    = ::rtl::OUString::createFromAscii("ALTER TABLE ");
        ::rtl::OUString aQuote  = m_pTable->getConnection()->getMetaData()->getIdentifierQuoteString(  );
        ::rtl::OUString aDot    = ::rtl::OUString::createFromAscii(".");

        aSql = aSql + aQuote + m_pTable->getSchema() + aQuote + m_pTable->getName() + aQuote
                    + ::rtl::OUString::createFromAscii(" DROP PRIMARY KEY");

        Reference< XStatement > xStmt = m_pTable->getConnection()->createStatement(  );
        xStmt->execute(aSql);
    }
    OCollection_TYPE::dropByName(elementName);
}
// -------------------------------------------------------------------------
void SAL_CALL OKeys::dropByIndex( sal_Int32 index ) throw(SQLException, IndexOutOfBoundsException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);
    if (index < 0 || index >= getCount())
        throw IndexOutOfBoundsException();

    dropByName(m_aElements[index]->first);
}

