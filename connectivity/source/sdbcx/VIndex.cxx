/*************************************************************************
 *
 *  $RCSfile: VIndex.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: oj $ $Date: 2001-03-30 14:01:49 $
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

#ifndef _CONNECTIVITY_SDBCX_INDEX_HXX_
#include "connectivity/sdbcx/VIndex.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#define CONNECTIVITY_PROPERTY_NAME_SPACE dbtools
#ifndef _CONNECTIVITY_PROPERTYIDS_HXX_
#include "propertyids.hxx"
#endif
#ifndef _CONNECTIVITY_SDBCX_COLUMN_HXX_
#include "connectivity/sdbcx/VColumn.hxx"
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _CONNECTIVITY_SDBCX_COLLECTION_HXX_
#include "connectivity/sdbcx/VCollection.hxx"
#endif
// -------------------------------------------------------------------------
using namespace connectivity::dbtools;
using namespace connectivity;
using namespace connectivity::sdbcx;
using namespace cppu;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL OIndex::getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException)
{
    if(isNew())
        return ::rtl::OUString::createFromAscii("com.sun.star.sdbcx.VIndexDescriptor");
    return ::rtl::OUString::createFromAscii("com.sun.star.sdbcx.VIndex");
}
// -----------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL OIndex::getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Sequence< ::rtl::OUString > aSupported(1);
    if(isNew())
        aSupported[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdbcx.IndexDescriptor");
    else
        aSupported[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdbcx.Index");

    return aSupported;
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OIndex::supportsService( const ::rtl::OUString& _rServiceName ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());
    const ::rtl::OUString* pSupported = aSupported.getConstArray();
    for (sal_Int32 i=0; i<aSupported.getLength(); ++i, ++pSupported)
        if (pSupported->equals(_rServiceName))
            return sal_True;

    return sal_False;
}
// -------------------------------------------------------------------------
OIndex::OIndex(sal_Bool _bCase) :   ODescriptor_BASE(m_aMutex)
                ,   ODescriptor(ODescriptor_BASE::rBHelper,_bCase,sal_True)
                ,   m_pColumns(NULL)
                ,m_IsUnique(sal_False)
                ,m_IsClustered(sal_False)
                ,m_IsPrimaryKeyIndex(sal_False)
{
}
// -------------------------------------------------------------------------
OIndex::OIndex( const ::rtl::OUString& _Name,
                const ::rtl::OUString& _Catalog,
                sal_Bool _isUnique,
                sal_Bool _isPrimaryKeyIndex,
                sal_Bool _isClustered,
                sal_Bool _bCase) :  ODescriptor_BASE(m_aMutex)
                        ,ODescriptor(ODescriptor_BASE::rBHelper,_bCase)
                        ,m_pColumns(NULL)
                        ,m_Catalog(_Catalog)
                        ,m_IsUnique(_isUnique)
                        ,m_IsPrimaryKeyIndex(_isPrimaryKeyIndex)
                        ,m_IsClustered(_isClustered)
{
    m_Name = _Name;
}
// -------------------------------------------------------------------------
OIndex::~OIndex( )
{
    delete m_pColumns;
}
// -----------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OIndex::createArrayHelper( sal_Int32 _nId) const
{
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::Property > aProps;
    describeProperties(aProps);
    changePropertyAttributte(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}
// -----------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& SAL_CALL OIndex::getInfoHelper()
{
    return *OIndex_PROP::getArrayHelper(isNew() ? 1 : 0);
}
// -------------------------------------------------------------------------
Any SAL_CALL OIndex::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = ODescriptor::queryInterface( rType);
    if(!aRet.hasValue())
    {
        if(!isNew())
            aRet = OIndex_BASE::queryInterface(rType);
        if(!aRet.hasValue())
            aRet = ODescriptor_BASE::queryInterface( rType);
    }
    return aRet;
}
// -------------------------------------------------------------------------
Sequence< Type > SAL_CALL OIndex::getTypes(  ) throw(RuntimeException)
{
    if(isNew())
        return ::comphelper::concatSequences(ODescriptor::getTypes(),ODescriptor_BASE::getTypes());
    return ::comphelper::concatSequences(ODescriptor::getTypes(),ODescriptor_BASE::getTypes(),OIndex_BASE::getTypes());
}
// -------------------------------------------------------------------------
void OIndex::construct()
{
    ODescriptor::construct();

    sal_Int32 nAttrib = isNew() ? 0 : PropertyAttribute::READONLY;

    registerProperty(PROPERTY_CATALOG,          PROPERTY_ID_CATALOG,            nAttrib,&m_Catalog,         ::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));
    registerProperty(PROPERTY_ISUNIQUE,         PROPERTY_ID_ISUNIQUE,           nAttrib,&m_IsUnique,            ::getBooleanCppuType());
    registerProperty(PROPERTY_ISPRIMARYKEYINDEX,PROPERTY_ID_ISPRIMARYKEYINDEX,  nAttrib,&m_IsPrimaryKeyIndex,   ::getBooleanCppuType());
    registerProperty(PROPERTY_ISCLUSTERED,      PROPERTY_ID_ISCLUSTERED,        nAttrib,&m_IsClustered,     ::getBooleanCppuType());
}
// -------------------------------------------------------------------------
void OIndex::disposing(void)
{
    OPropertySetHelper::disposing();

    ::osl::MutexGuard aGuard(m_aMutex);

    if(m_pColumns)
        m_pColumns->disposing();
}
// -------------------------------------------------------------------------
Reference< ::com::sun::star::container::XNameAccess > SAL_CALL OIndex::getColumns(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if (ODescriptor_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if(!m_pColumns)
        refreshColumns();

    return const_cast<OIndex*>(this)->m_pColumns;
}
// -------------------------------------------------------------------------
Reference< XPropertySet > SAL_CALL OIndex::createDataDescriptor(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if (ODescriptor_BASE::rBHelper.bDisposed)
        throw DisposedException();

    return this;
}
// -----------------------------------------------------------------------------
