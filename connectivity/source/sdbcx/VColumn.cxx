/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: VColumn.cxx,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 02:10:00 $
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
#ifndef _CONNECTIVITY_SDBCX_COLUMN_HXX_
#include "connectivity/sdbcx/VColumn.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef CONNECTIVITY_CONNECTION_HXX
#include "TConnection.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
// -------------------------------------------------------------------------
using namespace connectivity;
using namespace connectivity::sdbcx;
using namespace cppu;
using namespace connectivity;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;

// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL OColumn::getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException)
{
    if(isNew())
        return ::rtl::OUString::createFromAscii("com.sun.star.sdbcx.VColumnDescription");
    return ::rtl::OUString::createFromAscii("com.sun.star.sdbcx.VColumn");
}
// -----------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL OColumn::getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Sequence< ::rtl::OUString > aSupported(1);
    if(isNew())
        aSupported[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdbcx.ColumnDescription");
    else
        aSupported[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdbcx.Column");

    return aSupported;
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OColumn::supportsService( const ::rtl::OUString& _rServiceName ) throw(::com::sun::star::uno::RuntimeException)
{
    Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());
    const ::rtl::OUString* pSupported = aSupported.getConstArray();
    const ::rtl::OUString* pEnd = pSupported + aSupported.getLength();
    for (;pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported)
        ;

    return pSupported != pEnd;
}
// -------------------------------------------------------------------------
OColumn::OColumn(sal_Bool _bCase)
    :OColumnDescriptor_BASE(m_aMutex)
    ,ODescriptor(OColumnDescriptor_BASE::rBHelper,_bCase,sal_True)
    ,m_IsNullable(ColumnValue::NULLABLE)
    ,m_Precision(0)
    ,m_Scale(0)
    ,m_Type(0)
    ,m_IsAutoIncrement(sal_False)
    ,m_IsRowVersion(sal_False)
    ,m_IsCurrency(sal_False)
{
    construct();
}
// -------------------------------------------------------------------------
OColumn::OColumn(   const ::rtl::OUString& _Name,
                    const ::rtl::OUString& _TypeName,
                    const ::rtl::OUString& _DefaultValue,
                    sal_Int32       _IsNullable,
                    sal_Int32       _Precision,
                    sal_Int32       _Scale,
                    sal_Int32       _Type,
                    sal_Bool        _IsAutoIncrement,
                    sal_Bool        _IsRowVersion,
                    sal_Bool        _IsCurrency,
                    sal_Bool        _bCase)
    :OColumnDescriptor_BASE(m_aMutex)
    ,ODescriptor(OColumnDescriptor_BASE::rBHelper,_bCase)
    ,m_TypeName(_TypeName)
    ,m_DefaultValue(_DefaultValue)
    ,m_IsNullable(_IsNullable)
    ,m_Precision(_Precision)
    ,m_Scale(_Scale)
    ,m_Type(_Type)
    ,m_IsAutoIncrement(_IsAutoIncrement)
    ,m_IsRowVersion(_IsRowVersion)
    ,m_IsCurrency(_IsCurrency)
{
    m_Name = _Name;

    construct();
}
// -------------------------------------------------------------------------
OColumn::~OColumn()
{
}
// -----------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OColumn::createArrayHelper( sal_Int32 /*_nId*/ ) const
{
    return doCreateArrayHelper();
}
// -----------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& SAL_CALL OColumn::getInfoHelper()
{
    return *OColumn_PROP::getArrayHelper(isNew() ? 1 : 0);
}
// -----------------------------------------------------------------------------
void SAL_CALL OColumn::acquire() throw()
{
    OColumnDescriptor_BASE::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OColumn::release() throw()
{
    OColumnDescriptor_BASE::release();
}
// -----------------------------------------------------------------------------
Any SAL_CALL OColumn::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = ODescriptor::queryInterface( rType);
    if(!aRet.hasValue())
    {
        if(!isNew())
            aRet = OColumn_BASE::queryInterface(rType);
        if(!aRet.hasValue())
            aRet = OColumnDescriptor_BASE::queryInterface( rType);
    }
    return aRet;
}
// -------------------------------------------------------------------------
Sequence< Type > SAL_CALL OColumn::getTypes(  ) throw(RuntimeException)
{
    if(isNew())
        return ::comphelper::concatSequences(ODescriptor::getTypes(),OColumnDescriptor_BASE::getTypes());

    return ::comphelper::concatSequences(ODescriptor::getTypes(),OColumn_BASE::getTypes(),OColumnDescriptor_BASE::getTypes());
}
// -------------------------------------------------------------------------
void OColumn::construct()
{
    ODescriptor::construct();

    sal_Int32 nAttrib = isNew() ? 0 : PropertyAttribute::READONLY;

    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPENAME),            PROPERTY_ID_TYPENAME,           nAttrib,&m_TypeName,        ::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DESCRIPTION),     PROPERTY_ID_DESCRIPTION,        nAttrib,&m_Description,     ::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_DEFAULTVALUE),        PROPERTY_ID_DEFAULTVALUE,       nAttrib,&m_DefaultValue,    ::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_PRECISION),       PROPERTY_ID_PRECISION,          nAttrib,&m_Precision,       ::getCppuType(reinterpret_cast<sal_Int32*>(NULL)));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_TYPE),                PROPERTY_ID_TYPE,               nAttrib,&m_Type,            ::getCppuType(reinterpret_cast<sal_Int32*>(NULL)));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_SCALE),           PROPERTY_ID_SCALE,              nAttrib,&m_Scale,           ::getCppuType(reinterpret_cast<sal_Int32*>(NULL)));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISNULLABLE),      PROPERTY_ID_ISNULLABLE,         nAttrib,&m_IsNullable,      ::getCppuType(reinterpret_cast<sal_Int32*>(NULL)));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISAUTOINCREMENT), PROPERTY_ID_ISAUTOINCREMENT,    nAttrib,&m_IsAutoIncrement, ::getBooleanCppuType());
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISROWVERSION),        PROPERTY_ID_ISROWVERSION,       nAttrib,&m_IsRowVersion,    ::getBooleanCppuType());
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISCURRENCY),      PROPERTY_ID_ISCURRENCY,         nAttrib,&m_IsCurrency,      ::getBooleanCppuType());
}
// -------------------------------------------------------------------------
void OColumn::disposing(void)
{
    OPropertySetHelper::disposing();

    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OColumnDescriptor_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------
Reference< XPropertySet > SAL_CALL OColumn::createDataDescriptor(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OColumnDescriptor_BASE::rBHelper.bDisposed);


    OColumn* pNewColumn = new OColumn(  m_Name,
                                        m_TypeName,
                                        m_DefaultValue,
                                        m_IsNullable,
                                        m_Precision,
                                        m_Scale,
                                        m_Type,
                                        m_IsAutoIncrement,
                                        m_IsRowVersion,
                                        m_IsCurrency,
                                        isCaseSensitive());
    pNewColumn->m_Description = m_Description;
    pNewColumn->setNew(sal_True);
    return pNewColumn;
}
// -----------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL OColumn::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}
// -----------------------------------------------------------------------------
// XNamed
::rtl::OUString SAL_CALL OColumn::getName(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return m_Name;
}
// -----------------------------------------------------------------------------
void SAL_CALL OColumn::setName( const ::rtl::OUString& aName ) throw(::com::sun::star::uno::RuntimeException)
{
    m_Name = aName;
}
// -----------------------------------------------------------------------------

