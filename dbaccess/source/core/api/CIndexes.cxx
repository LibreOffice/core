/*************************************************************************
 *
 *  $RCSfile: CIndexes.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: fs $ $Date: 2001-03-15 08:29:16 $
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
#ifndef DBACCESS_INDEXES_HXX_
#include "CIndexes.hxx"
#endif
#ifndef DBACCESS_INDEX_HXX_
#include "CIndex.hxx"
#endif
#ifndef _DBA_CORE_TABLE_HXX_
#include "table.hxx"
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
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif

using namespace connectivity::sdbcx;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace dbaccess;
using namespace cppu;

typedef connectivity::sdbcx::OCollection OCollection_TYPE;

Reference< XNamed > OIndexes::createObject(const ::rtl::OUString& _rName)
{
    Reference< XNamed > xRet = NULL;
    if(m_xIndexes.is() && m_xIndexes->hasByName(_rName))
    {
        ::cppu::extractInterface(xRet,m_xIndexes->getByName(_rName));
    }
    else
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

        ::rtl::OUString aCatalog,aSchema,aTable;
        dbtools::qualifiedNameComponents(m_pTable->getMetaData(),m_pTable->getName(),aCatalog,aSchema,aTable);

        Reference< XResultSet > xResult = m_pTable->getMetaData()->getIndexInfo(makeAny(aCatalog),aSchema,aTable,sal_False,sal_False);

        Reference< XNamed > xRet = NULL;
        if(xResult.is())
        {
            Reference< XRow > xRow(xResult,UNO_QUERY);
            while(xResult->next())
            {
                sal_Bool bUnique = !xRow->getBoolean(4);
                if((!aQualifier.getLength() || xRow->getString(5) == aQualifier ) && xRow->getString(6) == aName)
                {
                    ODBIndex* pRet = new ODBIndex(m_pTable,aName,aQualifier,bUnique,
                        sal_False,
                        xRow->getShort(7) == IndexType::CLUSTERED);
                    xRet = pRet;
                }
            }
        }
    }

    return xRet;
}
// -------------------------------------------------------------------------
void OIndexes::impl_refresh() throw(RuntimeException)
{
    m_pTable->refreshIndexes();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > OIndexes::createEmptyObject()
{
    if(m_xIndexes.is())
    {
        Reference<XDataDescriptorFactory> xData( m_xIndexes,UNO_QUERY);
        return xData->createDataDescriptor();
    }
    else
    {
        connectivity::sdbcx::OIndex* pNew = new connectivity::sdbcx::OIndex(sal_True);
        return pNew;
    }
}
// -------------------------------------------------------------------------
// XAppend
void SAL_CALL OIndexes::appendByDescriptor( const Reference< XPropertySet >& descriptor ) throw(SQLException, ElementExistException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);

    if(m_xIndexes.is())
    {
        Reference<XAppend> xData( m_xIndexes,UNO_QUERY);
        xData->appendByDescriptor(descriptor);
    }
    else
    {
        ::rtl::OUString aName = comphelper::getString(descriptor->getPropertyValue(PROPERTY_NAME));
        ObjectMap::iterator aIter = m_aNameMap.find(aName);
        if( aIter != m_aNameMap.end())
            throw ElementExistException(aName,*this);

        if(!m_pTable->isNew())
        {
            ::rtl::OUString aSql    = ::rtl::OUString::createFromAscii("CREATE ");
            ::rtl::OUString aQuote  = m_pTable->getMetaData()->getIdentifierQuoteString(  );
            ::rtl::OUString aDot    = ::rtl::OUString::createFromAscii(".");

            if(comphelper::getINT32(descriptor->getPropertyValue(PROPERTY_ISUNIQUE)))
                aSql = aSql + ::rtl::OUString::createFromAscii("UNIQUE ");
            aSql = aSql + ::rtl::OUString::createFromAscii("INDEX ");


            ::rtl::OUString aCatalog,aSchema,aTable;
            dbtools::qualifiedNameComponents(m_pTable->getMetaData(),m_pTable->getName(),aCatalog,aSchema,aTable);
            ::rtl::OUString aComposedName;
            if(!m_pTable->getMetaData()->supportsCatalogsInIndexDefinitions())
                aCatalog = ::rtl::OUString();
            if(!m_pTable->getMetaData()->supportsSchemasInIndexDefinitions())
                aSchema = ::rtl::OUString();

            dbtools::composeTableName(m_pTable->getMetaData(),aCatalog,aSchema,aTable,aComposedName,sal_True);
            if(aName.getLength())
            {
                aSql = aSql + ::dbtools::quoteName( aQuote,aName )
                            + ::rtl::OUString::createFromAscii(" ON ")
                            + aComposedName
                            + ::rtl::OUString::createFromAscii(" ( ");

                Reference<XColumnsSupplier> xColumnSup(descriptor,UNO_QUERY);
                Reference<XIndexAccess> xColumns(xColumnSup->getColumns(),UNO_QUERY);
                Reference< XPropertySet > xColProp;
                for(sal_Int32 i=0;i<xColumns->getCount();++i)
                {
                    ::cppu::extractInterface(xColProp,xColumns->getByIndex(i));
                    aSql = aSql + ::dbtools::quoteName( aQuote,comphelper::getString(xColProp->getPropertyValue(PROPERTY_NAME)));
                    aSql = aSql +   (any2bool(xColProp->getPropertyValue(PROPERTY_ISASCENDING))
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
                aSql = aSql + aComposedName;

                Reference<XColumnsSupplier> xColumnSup(descriptor,UNO_QUERY);
                Reference<XIndexAccess> xColumns(xColumnSup->getColumns(),UNO_QUERY);
                Reference< XPropertySet > xColProp;
                if(xColumns->getCount() != 1)
                    throw SQLException();

                xColumns->getByIndex(0) >>= xColProp;

                aSql = aSql + aDot + ::dbtools::quoteName( aQuote,comphelper::getString(xColProp->getPropertyValue(PROPERTY_NAME)));
            }

            Reference< XStatement > xStmt = m_pTable->getConnection()->createStatement(  );
            xStmt->execute(aSql);
        }
    }
    OCollection_TYPE::appendByDescriptor(descriptor);
}
// -------------------------------------------------------------------------
// XDrop
void SAL_CALL OIndexes::dropByName( const ::rtl::OUString& elementName ) throw(SQLException, NoSuchElementException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);
    if(m_xIndexes.is())
    {
        Reference<XDrop> xData( m_xIndexes,UNO_QUERY);
        xData->dropByName(elementName);
    }
    else
    {
        ObjectMap::iterator aIter = m_aNameMap.find(elementName);
        if( aIter == m_aNameMap.end())
            throw NoSuchElementException(elementName,*this);

        if(!m_pTable->isNew())
        {
            ::rtl::OUString aName,aSchema;
            sal_Int32 nLen = elementName.indexOf('.');
            if(nLen != -1)
                aSchema = elementName.copy(0,nLen);
            aName   = elementName.copy(nLen+1);

            ::rtl::OUString aSql    = ::rtl::OUString::createFromAscii("DROP INDEX ");
            ::rtl::OUString aQuote  = m_pTable->getMetaData()->getIdentifierQuoteString(  );
            ::rtl::OUString aDot    = ::rtl::OUString::createFromAscii(".");

            ::rtl::OUString aCatalog,aSchema2,aTable;
            dbtools::qualifiedNameComponents(m_pTable->getMetaData(),m_pTable->getName(),aCatalog,aSchema2,aTable);
            ::rtl::OUString aComposedName;
            if(!m_pTable->getMetaData()->supportsCatalogsInIndexDefinitions())
                aCatalog = ::rtl::OUString();
            if(!m_pTable->getMetaData()->supportsSchemasInIndexDefinitions())
                aSchema2 = ::rtl::OUString();
            dbtools::composeTableName(m_pTable->getMetaData(),aCatalog,aSchema2,aTable,aComposedName,sal_True);

            aSql = aSql + ::dbtools::quoteName( aQuote,aSchema)
                        + aDot   + ::dbtools::quoteName( aQuote,aName)
                        + ::rtl::OUString::createFromAscii(" ON ")
                        + aComposedName;

            Reference< XStatement > xStmt = m_pTable->getConnection()->createStatement(  );
            xStmt->execute(aSql);
        }
    }
    OCollection_TYPE::dropByName(elementName);
}
// -------------------------------------------------------------------------
void SAL_CALL OIndexes::dropByIndex( sal_Int32 index ) throw(SQLException, IndexOutOfBoundsException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_rMutex);
    if(m_xIndexes.is())
    {
        Reference<XDrop> xData( m_xIndexes,UNO_QUERY);
        xData->dropByIndex(index);
        OCollection_TYPE::dropByIndex(index);
    }
    else
    {
        if (index < 0 || index >= getCount())
            throw IndexOutOfBoundsException();
        dropByName(m_aElements[index]->first);
    }
}
// -------------------------------------------------------------------------
void SAL_CALL OIndexes::disposing(void)
{
    if(m_xIndexes.is())
        clear_NoDispose();
    else
        OCollection_TYPE::disposing();
}
// -----------------------------------------------------------------------------


