/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TIndexes.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 14:19:09 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef CONNECTIVITY_INDEXESHELPER_HXX
#include "connectivity/TIndexes.hxx"
#endif
#ifndef CONNECTIVITY_INDEXHELPER_HXX_
#include "connectivity/TIndex.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_XROW_HPP_
#include <com/sun/star/sdbc/XRow.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSET_HPP_
#include <com/sun/star/sdbc/XResultSet.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_INDEXTYPE_HPP_
#include <com/sun/star/sdbc/IndexType.hpp>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef CONNECTIVITY_TABLEHELPER_HXX
#include "connectivity/TTableHelper.hxx"
#endif
#ifndef CONNECTIVITY_CONNECTION_HXX
#include "TConnection.hxx"
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
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
                xRow = NULL;
                xResult = NULL;
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
    if ( m_pTable->isNew() )
        return cloneDescriptor( descriptor );

    ::dbtools::OPropertyMap& rPropMap = OMetaConnection::getPropMap();
    ::rtl::OUStringBuffer aSql( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("CREATE ")));
    ::rtl::OUString aQuote  = m_pTable->getMetaData()->getIdentifierQuoteString(  );
    ::rtl::OUString aDot    = ::rtl::OUString::createFromAscii(".");

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
        sal_Bool bAddIndexAppendix = ::dbtools::isDataSourcePropertyEnabled(m_pTable->getConnection(),::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AddIndexAppendix")),sal_True);
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

    return createObject( _rForName );
}
// -------------------------------------------------------------------------
// XDrop
void OIndexesHelper::dropObject(sal_Int32 /*_nPos*/,const ::rtl::OUString _sElementName)
{
    if(!m_pTable->isNew())
    {
        ::rtl::OUString aName,aSchema;
        sal_Int32 nLen = _sElementName.indexOf('.');
        if(nLen != -1)
            aSchema = _sElementName.copy(0,nLen);
        aName   = _sElementName.copy(nLen+1);

        ::rtl::OUString aSql    = ::rtl::OUString::createFromAscii("DROP INDEX ");

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
// -----------------------------------------------------------------------------



