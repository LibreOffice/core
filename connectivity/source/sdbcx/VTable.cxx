/*************************************************************************
 *
 *  $RCSfile: VTable.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 17:17:36 $
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

#ifndef _CONNECTIVITY_SDBCX_TABLE_HXX_
#include "connectivity/sdbcx/VTable.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _CONNECTIVITY_SDBCX_INDEX_HXX_
#include "connectivity/sdbcx/VIndex.hxx"
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _CONNECTIVITY_SDBCX_COLLECTION_HXX_
#include "connectivity/sdbcx/VCollection.hxx"
#endif
#ifndef CONNECTIVITY_CONNECTION_HXX
#include "TConnection.hxx"
#endif
#ifndef _CONNECTIVITY_SDBCX_COLUMN_HXX_
#include "connectivity/sdbcx/VColumn.hxx"
#endif
#ifndef _CONNECTIVITY_SDBCX_KEY_HXX_
#include "connectivity/sdbcx/VKey.hxx"
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include "connectivity/dbtools.hxx"
#endif


// -------------------------------------------------------------------------
using namespace connectivity;
using namespace connectivity::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL OTable::getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException)
{
    if(isNew())
        return ::rtl::OUString::createFromAscii("com.sun.star.sdbcx.VTableDescriptor");
    return ::rtl::OUString::createFromAscii("com.sun.star.sdbcx.Table");
}

// -----------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL OTable::getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Sequence< ::rtl::OUString > aSupported(1);
    if(isNew())
        aSupported[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdbcx.TableDescriptor");
    else
        aSupported[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdbcx.Table");

    return aSupported;
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OTable::supportsService( const ::rtl::OUString& _rServiceName ) throw(::com::sun::star::uno::RuntimeException)
{
    Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());
    const ::rtl::OUString* pSupported = aSupported.getConstArray();
    const ::rtl::OUString* pEnd = pSupported + aSupported.getLength();
    for (;pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported)
        ;

    return pSupported != pEnd;
}
// -------------------------------------------------------------------------
OTable::OTable(OCollection* _pTables,
               sal_Bool _bCase)
               : OTableDescriptor_BASE(m_aMutex)
                ,ODescriptor(OTableDescriptor_BASE::rBHelper,_bCase,sal_True)
                ,m_pKeys(NULL)
                ,m_pColumns(NULL)
                ,m_pIndexes(NULL)
                ,m_pTables(_pTables)
{
}
// -----------------------------------------------------------------------------
OTable::OTable( OCollection*    _pTables,
                sal_Bool _bCase,
                const ::rtl::OUString& _Name,       const ::rtl::OUString& _Type,
                const ::rtl::OUString& _Description,const ::rtl::OUString& _SchemaName,
                const ::rtl::OUString& _CatalogName) :  OTableDescriptor_BASE(m_aMutex)
                ,ODescriptor(OTableDescriptor_BASE::rBHelper,_bCase)
                ,m_pKeys(NULL)
                ,m_pColumns(NULL)
                ,m_pIndexes(NULL)
                ,m_CatalogName(_CatalogName)
                ,m_SchemaName(_SchemaName)
                ,m_Description(_Description)
                ,m_Type(_Type)
                ,m_pTables(_pTables)
{
    m_Name = _Name;
}
// -------------------------------------------------------------------------
OTable::~OTable()
{
    delete m_pKeys;
    delete m_pColumns;
    delete m_pIndexes;
}
// -------------------------------------------------------------------------
void OTable::construct()
{
    ODescriptor::construct();

    sal_Int32 nAttrib = isNew() ? 0 : PropertyAttribute::READONLY;

    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_CATALOGNAME),     PROPERTY_ID_CATALOGNAME,nAttrib,&m_CatalogName, ::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCHEMANAME),      PROPERTY_ID_SCHEMANAME, nAttrib,&m_SchemaName,  ::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DESCRIPTION),     PROPERTY_ID_DESCRIPTION,nAttrib,&m_Description, ::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE),            PROPERTY_ID_TYPE,       nAttrib,&m_Type,        ::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));
}
// -----------------------------------------------------------------------------
void SAL_CALL OTable::acquire() throw()
{
    OTableDescriptor_BASE::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OTable::release() throw()
{
    OTableDescriptor_BASE::release();
}

// -------------------------------------------------------------------------
Any SAL_CALL OTable::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = ODescriptor::queryInterface( rType);
    if(!aRet.hasValue())
    {
        if(!isNew())
            aRet = OTable_BASE::queryInterface( rType);
        if(isNew() && (rType == getCppuType( (Reference<XIndexesSupplier>*)0)))
            return Any();
        if(!aRet.hasValue())
            aRet = OTableDescriptor_BASE::queryInterface( rType);
    }
    return aRet;
}
// -------------------------------------------------------------------------
Sequence< Type > SAL_CALL OTable::getTypes(  ) throw(RuntimeException)
{
    if(isNew())
        return ::comphelper::concatSequences(ODescriptor::getTypes(),OTableDescriptor_BASE::getTypes());
    return ::comphelper::concatSequences(ODescriptor::getTypes(),OTableDescriptor_BASE::getTypes(),OTable_BASE::getTypes());
}
// -------------------------------------------------------------------------
void SAL_CALL OTable::disposing(void)
{
    ODescriptor::disposing();

    ::osl::MutexGuard aGuard(m_aMutex);

    if(m_pKeys)
        m_pKeys->disposing();
    if(m_pColumns)
        m_pColumns->disposing();
    if(m_pIndexes)
        m_pIndexes->disposing();

    m_pTables = NULL;
}
// -----------------------------------------------------------------------------
// XColumnsSupplier
Reference< XNameAccess > SAL_CALL OTable::getColumns(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OTableDescriptor_BASE::rBHelper.bDisposed);

    try
    {
        if ( !m_pColumns )
            refreshColumns();
    }
    catch( const RuntimeException& )
    {
        // allowed to leave this method
        throw;
    }
    catch( const Exception& )
    {
        // allowed
    }

    return m_pColumns;
}

// -------------------------------------------------------------------------
// XKeysSupplier
Reference< XIndexAccess > SAL_CALL OTable::getKeys(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OTableDescriptor_BASE::rBHelper.bDisposed);

    try
    {
        if(!m_pKeys)
            refreshKeys();
    }
    catch( const RuntimeException& )
    {
        // allowed to leave this method
        throw;
    }
    catch( const Exception& )
    {
        // allowed
    }

    return m_pKeys;
}
// -----------------------------------------------------------------------------
cppu::IPropertyArrayHelper* OTable::createArrayHelper( sal_Int32 _nId) const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    changePropertyAttributte(aProps);
    return new cppu::OPropertyArrayHelper(aProps);
}
// -------------------------------------------------------------------------
cppu::IPropertyArrayHelper & OTable::getInfoHelper()
{
    return *const_cast<OTable*>(this)->getArrayHelper(isNew() ? 1 : 0);
}
// -------------------------------------------------------------------------
Reference< XPropertySet > SAL_CALL OTable::createDataDescriptor(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OTableDescriptor_BASE::rBHelper.bDisposed);

    OTable* pTable = new OTable(m_pTables,isCaseSensitive(),m_Name,m_Type,m_Description,m_SchemaName,m_CatalogName);
    pTable->setNew(sal_True);
    return pTable;
}
// -------------------------------------------------------------------------
// XIndexesSupplier
Reference< XNameAccess > SAL_CALL OTable::getIndexes(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OTableDescriptor_BASE::rBHelper.bDisposed);

    try
    {
        if ( !m_pIndexes )
            refreshIndexes();
    }
    catch( const RuntimeException& )
    {
        // allowed to leave this method
        throw;
    }
    catch( const Exception& )
    {
        // allowed
    }

    return m_pIndexes;
}
// -------------------------------------------------------------------------
// XRename
void SAL_CALL OTable::rename( const ::rtl::OUString& newName ) throw(SQLException, ElementExistException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OTableDescriptor_BASE::rBHelper.bDisposed);

    ::rtl::OUString sOldComposedName = getName();
    Reference< XDatabaseMetaData> xMetaData = getMetaData();
    if ( xMetaData.is() )
        ::dbtools::qualifiedNameComponents(xMetaData,newName,m_CatalogName,m_SchemaName,m_Name,::dbtools::eInDataManipulation);
    else
        m_Name = newName;

    m_pTables->renameObject(sOldComposedName,newName);
}
// -----------------------------------------------------------------------------
Reference< XDatabaseMetaData> OTable::getMetaData() const
{
    return NULL;
}
// -------------------------------------------------------------------------
// XAlterTable
void SAL_CALL OTable::alterColumnByName( const ::rtl::OUString& colName, const Reference< XPropertySet >& descriptor ) throw(SQLException, NoSuchElementException, RuntimeException)
{
}
// -------------------------------------------------------------------------
void SAL_CALL OTable::alterColumnByIndex( sal_Int32 index, const Reference< XPropertySet >& descriptor ) throw(SQLException, ::com::sun::star::lang::IndexOutOfBoundsException, RuntimeException)
{
}
// -------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL OTable::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL OTable::getName() throw(::com::sun::star::uno::RuntimeException)
{
    // this is only correct for tables who haven't a schema or catalog name
    OSL_ENSURE(!m_CatalogName.getLength(),"getName(): forgot to overload getName()!");
    OSL_ENSURE(!m_SchemaName.getLength(),"getName(): forgot to overload getName()!");
    return m_Name;
}
// -----------------------------------------------------------------------------
void SAL_CALL OTable::setName( const ::rtl::OUString& aName ) throw(::com::sun::star::uno::RuntimeException)
{
}
// -----------------------------------------------------------------------------
void OTable::refreshColumns()
{
}
// -----------------------------------------------------------------------------
void OTable::refreshKeys()
{
}
// -----------------------------------------------------------------------------
void OTable::refreshIndexes()
{
}
// -----------------------------------------------------------------------------
