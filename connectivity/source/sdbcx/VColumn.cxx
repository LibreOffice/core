/*************************************************************************
 *
 *  $RCSfile: VColumn.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: oj $ $Date: 2000-10-17 08:36:20 $
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
#ifndef _CONNECTIVITY_SDBCX_COLUMN_HXX_
#include "connectivity/sdbcx/VColumn.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _CONNECTIVITY_PROPERTYIDS_HXX_
#include "propertyids.hxx"
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif

// -------------------------------------------------------------------------
using namespace connectivity::sdbcx;
using namespace cppu;
using namespace connectivity;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
//  using namespace ::com::sun::star::sdbc;


IMPLEMENT_SERVICE_INFO(OColumn,"com.sun.star.sdbcx.VColumn","com.sun.star.sdbcx.Column");
// -------------------------------------------------------------------------
OColumn::OColumn(sal_Bool _bCase)  :    OColumn_BASE(m_aMutex)
                    ,   ODescriptor(OColumn_BASE::rBHelper,_bCase,sal_True)
                    ,   m_Precision(0)
                    ,   m_Type(0)
                    ,   m_Scale(0)
                    ,   m_IsNullable(sal_True)
                    ,   m_IsAutoIncrement(sal_False)
                    ,   m_IsRowVersion(sal_False)
                    ,   m_IsCurrency(sal_False)
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
            :   OColumn_BASE(m_aMutex)
            ,   ODescriptor(OColumn_BASE::rBHelper,_bCase)
            ,   m_TypeName(_TypeName)
            ,   m_DefaultValue(_DefaultValue)
            ,   m_Precision(_Precision)
            ,   m_Type(_Type)
            ,   m_Scale(_Scale)
            ,   m_IsNullable(_IsNullable)
            ,   m_IsAutoIncrement(_IsAutoIncrement)
            ,   m_IsRowVersion(_IsRowVersion)
            ,   m_IsCurrency(_IsCurrency)
{
    m_Name = _Name;

    construct();
}
// -------------------------------------------------------------------------
OColumn::~OColumn()
{
}
// -------------------------------------------------------------------------
Any SAL_CALL OColumn::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = ODescriptor::queryInterface( rType);
    if(aRet.hasValue())
        return aRet;
    return OColumn_BASE::queryInterface( rType);
}
// -------------------------------------------------------------------------
Sequence< Type > SAL_CALL OColumn::getTypes(  ) throw(RuntimeException)
{
    return ::comphelper::concatSequences(ODescriptor::getTypes(),OColumn_BASE::getTypes());
}
// -------------------------------------------------------------------------
void OColumn::construct()
{
    ODescriptor::construct();

    sal_Int32 nAttrib = isNew() ? 0 : PropertyAttribute::READONLY;

    registerProperty(connectivity::PROPERTY_TYPENAME,           PROPERTY_ID_TYPENAME,           nAttrib,&m_TypeName,        ::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));
    registerProperty(connectivity::PROPERTY_DESCRIPTION,        PROPERTY_ID_DESCRIPTION,        nAttrib,&m_Description,     ::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));
    registerProperty(connectivity::PROPERTY_DEFAULTVALUE,       PROPERTY_ID_DEFAULTVALUE,       nAttrib,&m_DefaultValue,    ::getCppuType(reinterpret_cast< ::rtl::OUString*>(NULL)));
    registerProperty(connectivity::PROPERTY_PRECISION,          PROPERTY_ID_PRECISION,          nAttrib,&m_Precision,       ::getCppuType(reinterpret_cast<sal_Int32*>(NULL)));
    registerProperty(connectivity::PROPERTY_TYPE,               PROPERTY_ID_TYPE,               nAttrib,&m_Type,            ::getCppuType(reinterpret_cast<sal_Int32*>(NULL)));
    registerProperty(connectivity::PROPERTY_SCALE,              PROPERTY_ID_SCALE,              nAttrib,&m_Scale,           ::getCppuType(reinterpret_cast<sal_Int32*>(NULL)));
    registerProperty(connectivity::PROPERTY_ISNULLABLE,         PROPERTY_ID_ISNULLABLE,         nAttrib,&m_IsNullable,      ::getCppuType(reinterpret_cast<sal_Int32*>(NULL)));
    registerProperty(connectivity::PROPERTY_ISAUTOINCREMENT,    PROPERTY_ID_ISAUTOINCREMENT,    nAttrib,&m_IsAutoIncrement, ::getBooleanCppuType());
    registerProperty(connectivity::PROPERTY_ISROWVERSION,       PROPERTY_ID_ISROWVERSION,       nAttrib,&m_IsRowVersion,    ::getBooleanCppuType());
    registerProperty(connectivity::PROPERTY_ISCURRENCY,         PROPERTY_ID_ISCURRENCY,         nAttrib,&m_IsCurrency,      ::getBooleanCppuType());
}
// -------------------------------------------------------------------------
void OColumn::disposing(void)
{
    OPropertySetHelper::disposing();

    ::osl::MutexGuard aGuard(m_aMutex);
    if (OColumn_BASE::rBHelper.bDisposed)
        throw DisposedException();
}
// -------------------------------------------------------------------------
IPropertyArrayHelper* OColumn::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new OPropertyArrayHelper(aProps);
}
// -------------------------------------------------------------------------
IPropertyArrayHelper & OColumn::getInfoHelper()
{
    return *const_cast<OColumn*>(this)->getArrayHelper();
}
// -------------------------------------------------------------------------
Reference< XPropertySet > SAL_CALL OColumn::createDataDescriptor(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    if (OColumn_BASE::rBHelper.bDisposed)
        throw DisposedException();
    return this;
}

