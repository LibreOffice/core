/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: BIndexes.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 14:22:04 $
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
#ifndef _CONNECTIVITY_ADABAS_INDEXES_HXX_
#include "adabas/BIndexes.hxx"
#endif
#ifndef _CONNECTIVITY_ADABAS_INDEX_HXX_
#include "adabas/BIndex.hxx"
#endif
#ifndef _CONNECTIVITY_ADABAS_TABLE_HXX_
#include "adabas/BTable.hxx"
#endif
#ifndef _CONNECTIVITY_SDBCX_INDEX_HXX_
#include "connectivity/sdbcx/VIndex.hxx"
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
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _CONNECTIVITY_ADABAS_CATALOG_HXX_
#include "adabas/BCatalog.hxx"
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include "connectivity/dbexception.hxx"
#endif


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
                    aName == ::rtl::OUString::createFromAscii("SYSPRIMARYKEYINDEX"),
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

    ::rtl::OUString aSql    = ::rtl::OUString::createFromAscii("CREATE ");
    ::rtl::OUString aQuote  = m_pTable->getMetaData()->getIdentifierQuoteString(  );
    const ::rtl::OUString& sDot = OAdabasCatalog::getDot();

    if(getBOOL(descriptor->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISUNIQUE))))
        aSql = aSql + ::rtl::OUString::createFromAscii("UNIQUE ");
    aSql = aSql + ::rtl::OUString::createFromAscii("INDEX ");


    if(_rForName.getLength())
    {
        aSql = aSql + aQuote + _rForName + aQuote
                    + ::rtl::OUString::createFromAscii(" ON ")
                    + aQuote + m_pTable->getSchema() + aQuote + sDot
                    + aQuote + m_pTable->getTableName() + aQuote
                    + ::rtl::OUString::createFromAscii(" ( ");

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
                            ::rtl::OUString::createFromAscii(" ASC")
                                        :
                            ::rtl::OUString::createFromAscii(" DESC"))
                        +   ::rtl::OUString::createFromAscii(",");
        }
        aSql = aSql.replaceAt(aSql.getLength()-1,1,::rtl::OUString::createFromAscii(")"));
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

        ::rtl::OUString aSql    = ::rtl::OUString::createFromAscii("DROP INDEX ");
        ::rtl::OUString aQuote  = m_pTable->getMetaData()->getIdentifierQuoteString(  );
        const ::rtl::OUString& sDot = OAdabasCatalog::getDot();

        if (aSchema.getLength())
            (((aSql += aQuote) += aSchema) += aQuote) += sDot;

        (((aSql += aQuote) += aName) += aQuote) += ::rtl::OUString::createFromAscii(" ON ");

        (((aSql += aQuote) += m_pTable->getSchema()) += aQuote) += sDot;
        ((aSql += aQuote) += m_pTable->getTableName()) += aQuote;

        Reference< XStatement > xStmt = m_pTable->getConnection()->createStatement(  );
        xStmt->execute(aSql);
        ::comphelper::disposeComponent(xStmt);
    }
}
// -------------------------------------------------------------------------


