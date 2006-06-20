/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: YColumns.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:52:53 $
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

OMySQLColumns::OMySQLColumns(   ::cppu::OWeakObject& _rParent
                                ,sal_Bool _bCase
                                ,::osl::Mutex& _rMutex
                                ,const TStringVector &_rVector
                                ,sal_Bool _bUseHardRef
            ) : OColumnsHelper(_rParent,_bCase,_rMutex,_rVector,_bUseHardRef)
{
}
// -----------------------------------------------------------------------------
Reference< XPropertySet > OMySQLColumns::createEmptyObject()
{
    return new OMySQLColumn(sal_True);
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
OMySQLColumn::OMySQLColumn( sal_Bool    _bCase)
    : connectivity::sdbcx::OColumn( _bCase )
{
    construct();
}
// -------------------------------------------------------------------------
void OMySQLColumn::construct()
{
    m_sAutoIncrement = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("auto_increment"));
    registerProperty(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_AUTOINCREMENTCREATION),PROPERTY_ID_AUTOINCREMENTCREATION,0,&m_sAutoIncrement, ::getCppuType(&m_sAutoIncrement));
}
// -----------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OMySQLColumn::createArrayHelper( sal_Int32 /*_nId*/ ) const
{
    return doCreateArrayHelper();
}
// -----------------------------------------------------------------------------
::cppu::IPropertyArrayHelper & SAL_CALL OMySQLColumn::getInfoHelper()
{
    return *OMySQLColumn_PROP::getArrayHelper(isNew() ? 1 : 0);
}
// -----------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL OMySQLColumn::getSupportedServiceNames(  ) throw(RuntimeException)
{
    Sequence< ::rtl::OUString > aSupported(1);
    aSupported[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdbcx.Column");

    return aSupported;
}
// -----------------------------------------------------------------------------
