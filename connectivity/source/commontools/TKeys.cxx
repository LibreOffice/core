/*************************************************************************
 *
 *  $RCSfile: TKeys.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:52:13 $
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

#ifndef CONNECTIVITY_TKEYS_HXX
#include "connectivity/TKeys.hxx"
#endif

#ifndef CONNECTIVITY_TKEY_HXX
#include "connectivity/TKey.hxx"
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
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include "connectivity/dbtools.hxx"
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef CONNECTIVITY_CONNECTION_HXX
#include "TConnection.hxx"
#endif

namespace connectivity
{
using namespace comphelper;
using namespace connectivity::sdbcx;
using namespace dbtools;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;



OKeysHelper::OKeysHelper(   OTableHelper* _pTable,
        ::osl::Mutex& _rMutex,
        const TStringVector& _rVector
        ) : OKeys_BASE(*_pTable,sal_True,_rMutex,_rVector,sal_True)
    ,m_pTable(_pTable)
{
}
// -------------------------------------------------------------------------
Reference< XNamed > OKeysHelper::createObject(const ::rtl::OUString& _rName)
{
    Reference< XNamed > xRet = NULL;

    if(_rName.getLength())
    {
        ::rtl::OUString aSchema,aTable;
        ::dbtools::OPropertyMap& rPropMap = OMetaConnection::getPropMap();
        m_pTable->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_SCHEMANAME)) >>= aSchema;
        m_pTable->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_NAME))       >>= aTable;

        Reference< XResultSet > xResult = m_pTable->getMetaData()->getImportedKeys(m_pTable->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_CATALOGNAME)),
            aSchema,aTable);

        if(xResult.is())
        {
            Reference< XRow > xRow(xResult,UNO_QUERY);
            ::rtl::OUString aName,aCatalog;
            while( xResult->next() )
            {
                // this must be outsid the "if" because we have to call in a right order
                aCatalog    = xRow->getString(1);
                if ( xRow->wasNull() )
                    aCatalog = ::rtl::OUString();
                aSchema     = xRow->getString(2);
                aName       = xRow->getString(3);

                sal_Int32 nUpdateRule = xRow->getInt(10);
                sal_Int32 nDeleteRule = xRow->getInt(11);
                if ( xRow->getString(12) == _rName )
                {
                    ::rtl::OUString aComposedName;
                    ::dbtools::composeTableName(m_pTable->getMetaData(),aCatalog,aSchema,aName,aComposedName,sal_False,::dbtools::eInDataManipulation);
                    OTableKeyHelper* pRet = new OTableKeyHelper(m_pTable,_rName,aComposedName,KeyType::FOREIGN,nUpdateRule,nDeleteRule);
                    xRet = pRet;
                    break;
                }
            }
        }
    }

    if(!xRet.is()) // we have a primary key with a system name
    {
        OTableKeyHelper* pRet = new OTableKeyHelper(m_pTable,_rName,::rtl::OUString(),KeyType::PRIMARY,KeyRule::NO_ACTION,KeyRule::NO_ACTION);
        xRet = pRet;
    }

    return xRet;
}
// -------------------------------------------------------------------------
void OKeysHelper::impl_refresh() throw(RuntimeException)
{
    m_pTable->refreshKeys();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OKeysHelper::createEmptyObject()
{
    return new OTableKeyHelper(m_pTable);
}
// -----------------------------------------------------------------------------
/** returns the keyrule string for the primary key
*/
::rtl::OUString getKeyRuleString(sal_Bool _bUpdate,sal_Int32 _nKeyRule)
{
    const char* pKeyRule = NULL;
    switch ( _nKeyRule )
    {
        case KeyRule::CASCADE:
            pKeyRule = _bUpdate ? " ON UPDATE CASCADE " : " ON DELETE CASCADE ";
            break;
        case KeyRule::RESTRICT:
            pKeyRule = _bUpdate ? " ON UPDATE RESTRICT " : " ON DELETE RESTRICT ";
            break;
        case KeyRule::SET_NULL:
            pKeyRule = _bUpdate ? " ON UPDATE SET NULL " : " ON DELETE SET NULL ";
            break;
        case KeyRule::SET_DEFAULT:
            pKeyRule = _bUpdate ? " ON UPDATE SET DEFAULT " : " ON DELETE SET DEFAULT ";
            break;
        default:
            ;
    }
    ::rtl::OUString sRet;
    if ( pKeyRule )
        sRet = ::rtl::OUString::createFromAscii(pKeyRule);
    return sRet;
}
// -------------------------------------------------------------------------
// XAppend
void OKeysHelper::appendObject( const Reference< XPropertySet >& descriptor )
{
    ::dbtools::OPropertyMap& rPropMap = OMetaConnection::getPropMap();
    ::rtl::OUString aName = getString(descriptor->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_NAME)));
    if ( !m_pTable->isNew() )
    {
        sal_Int32 nKeyType      = getINT32(descriptor->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_TYPE)));

        ::rtl::OUString aSql    = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ALTER TABLE "));
        ::rtl::OUString aQuote  = m_pTable->getConnection()->getMetaData()->getIdentifierQuoteString(  );
        ::rtl::OUString aDot    = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("."));

        aSql += composeTableName(m_pTable->getConnection()->getMetaData(),m_pTable,sal_True,::dbtools::eInTableDefinitions) + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" ADD "));

        if ( nKeyType == KeyType::PRIMARY )
        {
            aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" PRIMARY KEY ("));
        }
        else if ( nKeyType == KeyType::FOREIGN )
        {
            aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" FOREIGN KEY ("));
        }
        else
            throw SQLException();

        Reference<XColumnsSupplier> xColumnSup(descriptor,UNO_QUERY);
        Reference<XIndexAccess> xColumns(xColumnSup->getColumns(),UNO_QUERY);
        Reference< XPropertySet > xColProp;
        for(sal_Int32 i=0;i<xColumns->getCount();++i)
        {
            ::cppu::extractInterface(xColProp,xColumns->getByIndex(i));
            aSql += ::dbtools::quoteName( aQuote,getString(xColProp->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_NAME))))
                            +   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(","));
        }
        aSql = aSql.replaceAt(aSql.getLength()-1,1,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(")")));

        if ( nKeyType == KeyType::FOREIGN )
        {
            ::rtl::OUString aRefTable;

            descriptor->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_REFERENCEDTABLE)) >>= aRefTable;

            aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" REFERENCES "))
                 +  ::dbtools::quoteTableName(m_pTable->getConnection()->getMetaData(),aRefTable,::dbtools::eInTableDefinitions);
            aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" ("));

            for(sal_Int32 i=0;i<xColumns->getCount();++i)
            {
                ::cppu::extractInterface(xColProp,xColumns->getByIndex(i));
                aSql += ::dbtools::quoteName( aQuote,getString(xColProp->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_RELATEDCOLUMN))))
                                +   ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(","));
            }
            aSql = aSql.replaceAt(aSql.getLength()-1,1,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(")")));

            aSql += getKeyRuleString(sal_True   ,getINT32(descriptor->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_UPDATERULE))));
            aSql += getKeyRuleString(sal_False  ,getINT32(descriptor->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_DELETERULE))));
        }

        Reference< XStatement > xStmt = m_pTable->getConnection()->createStatement(  );
        xStmt->execute(aSql);
        // we need a name for the insertion
        try
        {
            ::rtl::OUString aSchema,aTable;
            ::dbtools::OPropertyMap& rPropMap = OMetaConnection::getPropMap();
            m_pTable->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_SCHEMANAME)) >>= aSchema;
            m_pTable->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_NAME))       >>= aTable;
            Reference< XResultSet > xResult;
            sal_Int32 nColumn = 12;
            if ( nKeyType == KeyType::FOREIGN )
                xResult = m_pTable->getMetaData()->getImportedKeys( m_pTable->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_CATALOGNAME))
                                                                                        ,aSchema
                                                                                        ,aTable);
            else
            {
                xResult = m_pTable->getMetaData()->getPrimaryKeys( m_pTable->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_CATALOGNAME))
                                                                                        ,aSchema
                                                                                        ,aTable);
                nColumn = 6;
            }
            if ( xResult.is() )
            {
                Reference< XRow > xRow(xResult,UNO_QUERY);
                while( xResult->next() )
                {
                    ::rtl::OUString sName = xRow->getString(nColumn);
                    if ( !m_pElements->exists(sName) ) // this name wasn't inserted yet so it must be te new one
                    {
                        descriptor->setPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_NAME),makeAny(sName));
                        break;
                    }
                }
            }
        }
        catch(const SQLException&)
        {
        }
    }
}
// -------------------------------------------------------------------------
// XDrop
void OKeysHelper::dropObject(sal_Int32 _nPos,const ::rtl::OUString _sElementName)
{
    if ( !m_pTable->isNew() )
    {
        ::rtl::OUString aSql    = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ALTER TABLE "));

        aSql += composeTableName(m_pTable->getConnection()->getMetaData(),m_pTable,sal_True,::dbtools::eInTableDefinitions);

        Reference<XPropertySet> xKey(getObject(_nPos),UNO_QUERY);

        sal_Int32 nKeyType = KeyType::PRIMARY;
        if ( xKey.is() )
        {
            ::dbtools::OPropertyMap& rPropMap = OMetaConnection::getPropMap();
            xKey->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_TYPE)) >>= nKeyType;
        }
        if ( KeyType::PRIMARY == nKeyType )
        {
            aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" DROP PRIMARY KEY"));
        }
        else
        {
            aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" DROP CONSTRAINT "));
            ::rtl::OUString aQuote  = m_pTable->getConnection()->getMetaData()->getIdentifierQuoteString();
            aSql += ::dbtools::quoteName( aQuote,_sElementName);
        }

        Reference< XStatement > xStmt = m_pTable->getConnection()->createStatement(  );
        if ( xStmt.is() )
        {
            xStmt->execute(aSql);
            ::comphelper::disposeComponent(xStmt);
        }
    }
}
// -----------------------------------------------------------------------------
Reference< XNamed > OKeysHelper::cloneObject(const Reference< XPropertySet >& _xDescriptor)
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
        Reference<XPropertySet> xProp = createEmptyObject();
        ::comphelper::copyProperties(_xDescriptor,xProp);
        Reference<XColumnsSupplier> xSup(_xDescriptor,UNO_QUERY);
        Reference<XIndexAccess> xIndex(xSup->getColumns(),UNO_QUERY);
        Reference<XColumnsSupplier> xDestSup(xProp,UNO_QUERY);
        Reference<XAppend> xAppend(xDestSup->getColumns(),UNO_QUERY);
        sal_Int32 nCount = xIndex->getCount();
        for(sal_Int32 i=0;i< nCount;++i)
        {
            Reference<XPropertySet> xColProp;
            xIndex->getByIndex(i) >>= xColProp;
            xAppend->appendByDescriptor(xColProp);
        }
        xName = Reference< XNamed >(xProp,UNO_QUERY);
    }
    return xName;
}
// -----------------------------------------------------------------------------
} // namespace connectivity
// -----------------------------------------------------------------------------
