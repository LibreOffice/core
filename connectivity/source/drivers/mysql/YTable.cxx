/*************************************************************************
 *
 *  $RCSfile: YTable.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 17:08:02 $
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

#ifndef CONNECTIVITY_MYSQL_TABLE_HXX
#include "mysql/YTable.hxx"
#endif
#ifndef CONNECTIVITY_MYSQL_TABLES_HXX
#include "mysql/YTables.hxx"
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
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_PRIVILEGE_HPP_
#include <com/sun/star/sdbcx/Privilege.hpp>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include "connectivity/dbtools.hxx"
#endif
#ifndef _CONNECTIVITY_SDBCX_COLUMN_HXX_
#include "connectivity/sdbcx/VColumn.hxx"
#endif
#ifndef CONNECTIVITY_TKEYS_HXX
#include "connectivity/TKeys.hxx"
#endif
#ifndef CONNECTIVITY_INDEXESHELPER_HXX
#include "connectivity/TIndexes.hxx"
#endif
#ifndef CONNECTIVITY_COLUMNSHELPER_HXX
#include "connectivity/TColumnsHelper.hxx"
#endif
#ifndef CONNECTIVITY_MYSQL_CATALOG_HXX
#include "mysql/YCatalog.hxx"
#endif
#include "mysql/YColumns.hxx"
#ifndef CONNECTIVITY_CONNECTION_HXX
#include "TConnection.hxx"
#endif


using namespace ::comphelper;
using namespace connectivity::mysql;
using namespace connectivity::sdbcx;
using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

OMySQLTable::OMySQLTable(   sdbcx::OCollection* _pTables,
                           const Reference< XConnection >& _xConnection)
    :OTableHelper(_pTables,_xConnection,sal_True)
{
    // we create a new table here, so we should have all the rights or ;-)
    m_nPrivileges = Privilege::DROP         |
                    Privilege::REFERENCE    |
                    Privilege::ALTER        |
                    Privilege::CREATE       |
                    Privilege::READ         |
                    Privilege::DELETE       |
                    Privilege::UPDATE       |
                    Privilege::INSERT       |
                    Privilege::SELECT;
    construct();
}
// -------------------------------------------------------------------------
OMySQLTable::OMySQLTable(   sdbcx::OCollection* _pTables,
                           const Reference< XConnection >& _xConnection,
                    const ::rtl::OUString& _Name,
                    const ::rtl::OUString& _Type,
                    const ::rtl::OUString& _Description ,
                    const ::rtl::OUString& _SchemaName,
                    const ::rtl::OUString& _CatalogName,
                    sal_Int32 _nPrivileges
                ) : OTableHelper(   _pTables,
                                    _xConnection,
                                    sal_True,
                                    _Name,
                                    _Type,
                                    _Description,
                                    _SchemaName,
                                    _CatalogName)
 , m_nPrivileges(_nPrivileges)
{
    construct();
}
// -------------------------------------------------------------------------
void OMySQLTable::construct()
{
    OTableHelper::construct();
    if ( !isNew() )
        registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PRIVILEGES),  PROPERTY_ID_PRIVILEGES,PropertyAttribute::READONLY,&m_nPrivileges,  ::getCppuType(&m_nPrivileges));
}
// -----------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OMySQLTable::createArrayHelper( sal_Int32 _nId) const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    changePropertyAttributte(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}
// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper & OMySQLTable::getInfoHelper()
{
    return *static_cast<OMySQLTable_PROP*>(const_cast<OMySQLTable*>(this))->getArrayHelper(isNew() ? 1 : 0);
}
// -----------------------------------------------------------------------------
sdbcx::OCollection* OMySQLTable::createColumns(const TStringVector& _rNames)
{
    OMySQLColumns* pColumns = new OMySQLColumns(*this,sal_True,m_aMutex,_rNames);
    pColumns->setParent(this);
    return pColumns;
}
// -----------------------------------------------------------------------------
sdbcx::OCollection* OMySQLTable::createKeys(const TStringVector& _rNames)
{
    return new OKeysHelper(this,m_aMutex,_rNames);
}
// -----------------------------------------------------------------------------
sdbcx::OCollection* OMySQLTable::createIndexes(const TStringVector& _rNames)
{
    return new OIndexesHelper(this,m_aMutex,_rNames);
}
//--------------------------------------------------------------------------
Sequence< sal_Int8 > OMySQLTable::getUnoTunnelImplementationId()
{
    static ::cppu::OImplementationId * pId = 0;
    if (! pId)
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if (! pId)
        {
            static ::cppu::OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}

// com::sun::star::lang::XUnoTunnel
//------------------------------------------------------------------
sal_Int64 OMySQLTable::getSomething( const Sequence< sal_Int8 > & rId ) throw (RuntimeException)
{
    return (rId.getLength() == 16 && 0 == rtl_compareMemory(getUnoTunnelImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
                ?
            (sal_Int64)this
                :
            OTable_TYPEDEF::getSomething(rId);
}
// -------------------------------------------------------------------------
// XAlterTable
void SAL_CALL OMySQLTable::alterColumnByName( const ::rtl::OUString& colName, const Reference< XPropertySet >& descriptor ) throw(SQLException, NoSuchElementException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(
#ifdef GCC
        ::connectivity::sdbcx::OTableDescriptor_BASE::rBHelper.bDisposed
#else
        rBHelper.bDisposed
#endif
        );

    if ( m_pColumns && !m_pColumns->hasByName(colName) )
        throw NoSuchElementException(colName,*this);


    if ( !isNew() )
    {
        // first we have to check what should be altered
        Reference<XPropertySet> xProp;
        m_pColumns->getByName(colName) >>= xProp;
        // first check the types
        sal_Int32 nOldType = 0,nNewType = 0,nOldPrec = 0,nNewPrec = 0,nOldScale = 0,nNewScale = 0;

        ::dbtools::OPropertyMap& rProp = OMetaConnection::getPropMap();
        xProp->getPropertyValue(rProp.getNameByIndex(PROPERTY_ID_TYPE))         >>= nOldType;
        descriptor->getPropertyValue(rProp.getNameByIndex(PROPERTY_ID_TYPE))    >>= nNewType;
        // and precsions and scale
        xProp->getPropertyValue(rProp.getNameByIndex(PROPERTY_ID_PRECISION))    >>= nOldPrec;
        descriptor->getPropertyValue(rProp.getNameByIndex(PROPERTY_ID_PRECISION))>>= nNewPrec;
        xProp->getPropertyValue(rProp.getNameByIndex(PROPERTY_ID_SCALE))        >>= nOldScale;
        descriptor->getPropertyValue(rProp.getNameByIndex(PROPERTY_ID_SCALE))   >>= nNewScale;
        // second: check the "is nullable" value
        sal_Int32 nOldNullable = 0,nNewNullable = 0;
        xProp->getPropertyValue(rProp.getNameByIndex(PROPERTY_ID_ISNULLABLE))       >>= nOldNullable;
        descriptor->getPropertyValue(rProp.getNameByIndex(PROPERTY_ID_ISNULLABLE))  >>= nNewNullable;

        // check also the auto_increment
        sal_Bool bOldAutoIncrement = sal_False,bAutoIncrement = sal_False;
        xProp->getPropertyValue(rProp.getNameByIndex(PROPERTY_ID_ISAUTOINCREMENT))      >>= bOldAutoIncrement;
        descriptor->getPropertyValue(rProp.getNameByIndex(PROPERTY_ID_ISAUTOINCREMENT)) >>= bAutoIncrement;

        if (    nOldType != nNewType
            ||  nOldPrec != nNewPrec
            ||  nOldScale != nNewScale
            ||  nNewNullable != nOldNullable
            ||  bOldAutoIncrement != bAutoIncrement )
        {
            // special handling because they change dthe type names to distinguish
            // if a column should be an auto_incmrement one
            if ( bOldAutoIncrement != bAutoIncrement )
            {
                ::rtl::OUString sTypeName;
                descriptor->getPropertyValue(rProp.getNameByIndex(PROPERTY_ID_TYPENAME)) >>= sTypeName;

                static ::rtl::OUString s_sAutoIncrement(RTL_CONSTASCII_USTRINGPARAM("auto_increment"));
                if ( bAutoIncrement )
                {
                    if ( sTypeName.indexOf(s_sAutoIncrement) == -1 )
                    {
                        sTypeName += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" "));
                        sTypeName += s_sAutoIncrement;
                    }
                }
                else
                {
                    sal_Int32 nIndex = 0;
                    if ( sTypeName.getLength() && (nIndex = sTypeName.indexOf(s_sAutoIncrement)) != -1 )
                    {
                        sTypeName = sTypeName.copy(0,nIndex);
                        descriptor->setPropertyValue(rProp.getNameByIndex(PROPERTY_ID_TYPENAME),makeAny(sTypeName));
                    }
                }
            }
            alterColumnType(nNewType,colName,descriptor);
        }

        // third: check the default values
        ::rtl::OUString sNewDefault,sOldDefault;
        xProp->getPropertyValue(rProp.getNameByIndex(PROPERTY_ID_DEFAULTVALUE))     >>= sOldDefault;
        descriptor->getPropertyValue(rProp.getNameByIndex(PROPERTY_ID_DEFAULTVALUE)) >>= sNewDefault;

        if(sOldDefault.getLength())
        {
            dropDefaultValue(colName);
            if(sNewDefault.getLength() && sOldDefault != sNewDefault)
                alterDefaultValue(sNewDefault,colName);
        }
        else if(!sOldDefault.getLength() && sNewDefault.getLength())
            alterDefaultValue(sNewDefault,colName);

        // now we should look if the name of the column changed
        ::rtl::OUString sNewColumnName;
        descriptor->getPropertyValue(rProp.getNameByIndex(PROPERTY_ID_NAME)) >>= sNewColumnName;
        if ( !sNewColumnName.equalsIgnoreAsciiCase(colName) )
        {
            ::rtl::OUString sSql = getAlterTableColumnPart(colName);
            sSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" CHANGE "));
            const ::rtl::OUString sQuote = getMetaData()->getIdentifierQuoteString(  );
            sSql += ::dbtools::quoteName(sQuote,colName);
            sSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" "));
            sSql += ::dbtools::createStandardColumnPart(descriptor,getConnection());
            executeStatement(sSql);
        }
        m_pColumns->refresh();
    }
    else
    {
        if(m_pColumns)
        {
            m_pColumns->dropByName(colName);
            m_pColumns->appendByDescriptor(descriptor);
        }
    }

}
// -----------------------------------------------------------------------------
void OMySQLTable::alterColumnType(sal_Int32 nNewType,const ::rtl::OUString& _rColName, const Reference<XPropertySet>& _xDescriptor)
{
    ::rtl::OUString sSql = getAlterTableColumnPart(_rColName);
    sSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" CHANGE "));
    const ::rtl::OUString sQuote = getMetaData()->getIdentifierQuoteString(  );
    sSql += ::dbtools::quoteName(sQuote,_rColName);
    sSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" "));

    OColumn* pColumn = new OColumn(sal_True);
    Reference<XPropertySet> xProp = pColumn;
    ::comphelper::copyProperties(_xDescriptor,xProp);
    xProp->setPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE),makeAny(nNewType));

    sSql += ::dbtools::createStandardColumnPart(xProp,getConnection());
    executeStatement(sSql);
}
// -----------------------------------------------------------------------------
void OMySQLTable::alterDefaultValue(const ::rtl::OUString& _sNewDefault,const ::rtl::OUString& _rColName)
{
    ::rtl::OUString sSql = getAlterTableColumnPart(_rColName);
    sSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" ALTER "));

    const ::rtl::OUString sQuote = getMetaData()->getIdentifierQuoteString(  );
    sSql += ::dbtools::quoteName(sQuote,_rColName);
    sSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" SET DEFAULT '")) + _sNewDefault;
    sSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("'"));

    executeStatement(sSql);
}
// -----------------------------------------------------------------------------
void OMySQLTable::dropDefaultValue(const ::rtl::OUString& _rColName)
{
    ::rtl::OUString sSql = getAlterTableColumnPart(_rColName);
    sSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" ALTER "));

    const ::rtl::OUString sQuote = getMetaData()->getIdentifierQuoteString(  );
    sSql += ::dbtools::quoteName(sQuote,_rColName);
    sSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" DROP DEFAULT"));

    executeStatement(sSql);
}
// -----------------------------------------------------------------------------
::rtl::OUString OMySQLTable::getAlterTableColumnPart(const ::rtl::OUString& _rsColumnName )
{
    ::rtl::OUString sSql = ::rtl::OUString::createFromAscii("ALTER TABLE ");
    const ::rtl::OUString sQuote = getMetaData()->getIdentifierQuoteString(  );

    ::rtl::OUString sComposedName;
    ::dbtools::composeTableName(getMetaData(),m_CatalogName,m_SchemaName,m_Name,sComposedName,sal_True,::dbtools::eInTableDefinitions);
    sSql += sComposedName;

    return sSql;
}
// -----------------------------------------------------------------------------
void OMySQLTable::executeStatement(const ::rtl::OUString& _rStatement )
{
    ::rtl::OUString sSQL = _rStatement;
    if(sSQL.lastIndexOf(',') == (sSQL.getLength()-1))
        sSQL = sSQL.replaceAt(sSQL.getLength()-1,1,::rtl::OUString::createFromAscii(")"));

    Reference< XStatement > xStmt = getConnection()->createStatement(  );
    if ( xStmt.is() )
    {
        xStmt->execute(sSQL);
        ::comphelper::disposeComponent(xStmt);
    }
}
// -----------------------------------------------------------------------------




