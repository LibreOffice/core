/*************************************************************************
 *
 *  $RCSfile: BKeys.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:56:04 $
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
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _CONNECTIVITY_ADABAS_CATALOG_HXX_
#include "adabas/BCatalog.hxx"
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif


using namespace ::comphelper;

using namespace connectivity::adabas;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
typedef connectivity::sdbcx::OCollection OCollection_TYPE;

// -------------------------------------------------------------------------
Reference< XNamed > OKeys::createObject(const ::rtl::OUString& _rName)
{
    Reference< XNamed > xRet = NULL;

    if(_rName.getLength())
    {
        Reference< XResultSet > xResult = m_pTable->getConnection()->getMetaData()->getImportedKeys(Any(),
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
    return new OAdabasKey(m_pTable);
}
// -------------------------------------------------------------------------
// XAppend
void OKeys::appendObject( const Reference< XPropertySet >& descriptor )
{
    if(!m_pTable->isNew())
    {
        sal_Int32 nKeyType      = getINT32(descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE)));

        ::rtl::OUString aSql    = ::rtl::OUString::createFromAscii("ALTER TABLE ");
        ::rtl::OUString aQuote  = m_pTable->getConnection()->getMetaData()->getIdentifierQuoteString(  );
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
        ::comphelper::disposeComponent(xStmt);
        // we need a name for the insertion
        if(nKeyType == KeyType::FOREIGN)
        {
            Reference< XResultSet > xResult = m_pTable->getConnection()->getMetaData()->getImportedKeys(Any(),m_pTable->getSchema(),m_pTable->getTableName());
            if(xResult.is())
            {
                Reference< XRow > xRow(xResult,UNO_QUERY);
                while(xResult->next())
                {
                    ::rtl::OUString sName = xRow->getString(12);
                    if ( !m_pElements->exists(sName) ) // this name wasn't inserted yet so it must be te new one
                    {
                        descriptor->setPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME),makeAny(sName));
                        break;
                    }
                }
                ::comphelper::disposeComponent(xResult);
            }
        }
    }
}
// -------------------------------------------------------------------------
// XDrop
void OKeys::dropObject(sal_Int32 _nPos,const ::rtl::OUString _sElementName)
{
    if(!m_pTable->isNew())
    {
        ::rtl::OUString aSql    = ::rtl::OUString::createFromAscii("ALTER TABLE ");
        ::rtl::OUString aQuote  = m_pTable->getConnection()->getMetaData()->getIdentifierQuoteString(  );
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
Reference< XNamed > OKeys::cloneObject(const Reference< XPropertySet >& _xDescriptor)
{
    Reference< XNamed > xName;
    if(!m_pTable->isNew())
    {
        xName = Reference< XNamed >(_xDescriptor,UNO_QUERY);
        OSL_ENSURE(xName.is(),"Must be a XName interface here !");
        xName = xName.is() ? createObject(xName->getName()) : Reference< XNamed >();
    }
    else
    {
        OAdabasKey* pKey = new OAdabasKey(m_pTable);
        xName = pKey;
        Reference<XPropertySet> xProp = pKey;
        ::comphelper::copyProperties(_xDescriptor,xProp);
        Reference<XColumnsSupplier> xSup(_xDescriptor,UNO_QUERY);
        Reference<XIndexAccess> xIndex(xSup->getColumns(),UNO_QUERY);
        Reference<XAppend> xAppend(pKey->getColumns(),UNO_QUERY);
        sal_Int32 nCount = xIndex->getCount();
        for(sal_Int32 i=0;i< nCount;++i)
        {
            Reference<XPropertySet> xProp;
            xIndex->getByIndex(i) >>= xProp;
            xAppend->appendByDescriptor(xProp);
        }
    }
    return xName;
}
// -----------------------------------------------------------------------------

