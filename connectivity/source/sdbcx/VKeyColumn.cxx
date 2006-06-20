/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: VKeyColumn.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 02:11:10 $
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

#ifndef _CONNECTIVITY_SDBCX_KEYCOLUMN_HXX_
#include "connectivity/sdbcx/VKeyColumn.hxx"
#endif
#ifndef CONNECTIVITY_CONNECTION_HXX
#include "TConnection.hxx"
#endif

using namespace connectivity;
using namespace connectivity::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace cppu;
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL OKeyColumn::getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException)
{
    if(isNew())
        return ::rtl::OUString::createFromAscii("com.sun.star.sdbcx.VKeyColumnDescription");
    return ::rtl::OUString::createFromAscii("com.sun.star.sdbcx.VKeyColumn");
}
// -----------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL OKeyColumn::getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::com::sun::star::uno::Sequence< ::rtl::OUString > aSupported(1);
    if(isNew())
        aSupported[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdbcx.KeyColumnDescription");
    else
        aSupported[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdbcx.KeyColumn");

    return aSupported;
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OKeyColumn::supportsService( const ::rtl::OUString& _rServiceName ) throw(::com::sun::star::uno::RuntimeException)
{
    Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());
    const ::rtl::OUString* pSupported = aSupported.getConstArray();
    const ::rtl::OUString* pEnd = pSupported + aSupported.getLength();
    for (;pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported)
        ;

    return pSupported != pEnd;
}
// -------------------------------------------------------------------------
OKeyColumn::OKeyColumn(sal_Bool _bCase) : OColumn(_bCase)
{
    construct();
}
// -------------------------------------------------------------------------
OKeyColumn::OKeyColumn( const ::rtl::OUString&  _ReferencedColumn,
                        const ::rtl::OUString&  _Name,
                        const ::rtl::OUString&  _TypeName,
                        const ::rtl::OUString&  _DefaultValue,
                        sal_Int32               _IsNullable,
                        sal_Int32               _Precision,
                        sal_Int32               _Scale,
                        sal_Int32               _Type,
                        sal_Bool                _IsAutoIncrement,
                        sal_Bool                _IsRowVersion,
                        sal_Bool                _IsCurrency,
                        sal_Bool                _bCase
                        ) : OColumn(_Name,
                            _TypeName,
                            _DefaultValue,
                            _IsNullable,
                            _Precision,
                            _Scale,
                            _Type,
                            _IsAutoIncrement,
                            _IsRowVersion,
                            _IsCurrency,
                            _bCase)
                        ,   m_ReferencedColumn(_ReferencedColumn)
{
    construct();
}
// -------------------------------------------------------------------------
OKeyColumn::~OKeyColumn()
{
}
// -----------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OKeyColumn::createArrayHelper( sal_Int32 /*_nId*/ ) const
{
    return doCreateArrayHelper();
}
// -----------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& SAL_CALL OKeyColumn::getInfoHelper()
{
    return *OKeyColumn_PROP::getArrayHelper(isNew() ? 1 : 0);
}
// -------------------------------------------------------------------------
void OKeyColumn::construct()
{
    sal_Int32 nAttrib = isNew() ? 0 : PropertyAttribute::READONLY;
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RELATEDCOLUMN),   PROPERTY_ID_RELATEDCOLUMN,  nAttrib,&m_ReferencedColumn,    ::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));
}
// -----------------------------------------------------------------------------




