/*************************************************************************
 *
 *  $RCSfile: TIndexes.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 16:52:00 $
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

Reference< XNamed > OIndexesHelper::createObject(const ::rtl::OUString& _rName)
{
    Reference< XNamed > xRet = NULL;
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


    Reference< XResultSet > xResult = m_pTable->getMetaData()->getIndexInfo(m_pTable->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_CATALOGNAME)),aSchema,aTable,sal_False,sal_False);

    if ( xResult.is() )
    {
        Reference< XRow > xRow(xResult,UNO_QUERY);
        while( xResult->next() )
        {
            sal_Bool bUnique = !xRow->getBoolean(4);
            if((!aQualifier.getLength() || xRow->getString(5) == aQualifier ) && xRow->getString(6) == aName)
            {
                OIndexHelper* pRet = new OIndexHelper(m_pTable,aName,aQualifier,bUnique,
                    sal_False,
                    xRow->getShort(7) == IndexType::CLUSTERED);
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
Reference< XPropertySet > OIndexesHelper::createEmptyObject()
{
    return new OIndexHelper(m_pTable);
}
// -------------------------------------------------------------------------
// XAppend
void OIndexesHelper::appendObject( const Reference< XPropertySet >& descriptor )
{
    ::dbtools::OPropertyMap& rPropMap = OMetaConnection::getPropMap();
    ::rtl::OUString aName = comphelper::getString(descriptor->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_NAME)));
    if ( m_pElements->exists(aName) )
        throw ElementExistException(aName,*this);

    if(!m_pTable->isNew())
    {
        ::rtl::OUString aSql    = ::rtl::OUString::createFromAscii("CREATE ");
        ::rtl::OUString aQuote  = m_pTable->getMetaData()->getIdentifierQuoteString(  );
        ::rtl::OUString aDot    = ::rtl::OUString::createFromAscii(".");

        if(comphelper::getINT32(descriptor->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_ISUNIQUE))))
            aSql = aSql + ::rtl::OUString::createFromAscii("UNIQUE ");
        aSql = aSql + ::rtl::OUString::createFromAscii("INDEX ");


        ::rtl::OUString aCatalog,aSchema,aTable;
        dbtools::qualifiedNameComponents(m_pTable->getMetaData(),m_pTable->getName(),aCatalog,aSchema,aTable,::dbtools::eInDataManipulation);
        ::rtl::OUString aComposedName;

        dbtools::composeTableName(m_pTable->getMetaData(),aCatalog,aSchema,aTable,aComposedName,sal_True,::dbtools::eInIndexDefinitions);
        if ( aName.getLength() )
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
                aSql = aSql + ::dbtools::quoteName( aQuote,comphelper::getString(xColProp->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_NAME))));

                if ( ::dbtools::isDataSourcePropertyEnabled(m_pTable->getConnection(),::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AddIndexAppendix")),sal_True) )
                {

                    aSql += (any2bool(xColProp->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_ISASCENDING)))
                                                ?
                                    ::rtl::OUString::createFromAscii(" ASC")
                                                :
                                    ::rtl::OUString::createFromAscii(" DESC"));
                }
                aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(","));
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

            aSql = aSql + aDot + ::dbtools::quoteName( aQuote,comphelper::getString(xColProp->getPropertyValue(rPropMap.getNameByIndex(PROPERTY_ID_NAME))));
        }

        Reference< XStatement > xStmt = m_pTable->getConnection()->createStatement(  );
        if ( xStmt.is() )
        {
            xStmt->execute(aSql);
            ::comphelper::disposeComponent(xStmt);
        }
    }
}
// -------------------------------------------------------------------------
// XDrop
void OIndexesHelper::dropObject(sal_Int32 _nPos,const ::rtl::OUString _sElementName)
{
    if(!m_pTable->isNew())
    {
        ::rtl::OUString aName,aSchema;
        sal_Int32 nLen = _sElementName.indexOf('.');
        if(nLen != -1)
            aSchema = _sElementName.copy(0,nLen);
        aName   = _sElementName.copy(nLen+1);

        ::rtl::OUString aSql    = ::rtl::OUString::createFromAscii("DROP INDEX ");

        ::rtl::OUString aComposedName = dbtools::composeTableName(m_pTable->getMetaData(),m_pTable,sal_True,::dbtools::eInIndexDefinitions);
        ::rtl::OUString sIndexName,sTemp;
        dbtools::composeTableName(m_pTable->getMetaData(),sTemp,aSchema,aName,sIndexName,sal_True,::dbtools::eInIndexDefinitions,sal_True,sal_True);

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
Reference< XNamed > OIndexesHelper::cloneObject(const Reference< XPropertySet >& _xDescriptor)
{
    Reference< XNamed > xName(_xDescriptor,UNO_QUERY);
    OSL_ENSURE(xName.is(),"Must be a XName interface here !");
    return xName.is() ? createObject(xName->getName()) : Reference< XNamed >();
}
// -----------------------------------------------------------------------------



