/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: VGroup.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 02:10:24 $
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

#include <stdio.h>

#ifndef _CONNECTIVITY_SDBCX_GROUP_HXX_
#include "connectivity/sdbcx/VGroup.hxx"
#endif

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_PRIVILEGE_HPP_
#include <com/sun/star/sdbcx/Privilege.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_PRIVILEGEOBJECT_HPP_
#include <com/sun/star/sdbcx/PrivilegeObject.hpp>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif

// -------------------------------------------------------------------------
using namespace ::connectivity::sdbcx;
using namespace ::connectivity;
using namespace ::dbtools;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;

IMPLEMENT_SERVICE_INFO(OGroup,"com.sun.star.sdbcx.VGroup","com.sun.star.sdbcx.Group");
// -------------------------------------------------------------------------
OGroup::OGroup(sal_Bool _bCase) :   OGroup_BASE(m_aMutex)
                ,   ODescriptor(OGroup_BASE::rBHelper,_bCase)
                ,   m_pUsers(NULL)
{
}
// -------------------------------------------------------------------------
OGroup::OGroup(const ::rtl::OUString& _Name,sal_Bool _bCase) :  OGroup_BASE(m_aMutex)
                        ,ODescriptor(OGroup_BASE::rBHelper,_bCase)
                        ,m_pUsers(NULL)
{
    m_Name = _Name;
}
// -------------------------------------------------------------------------
OGroup::~OGroup()
{
    delete m_pUsers;
}
// -------------------------------------------------------------------------
Any SAL_CALL OGroup::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = ODescriptor::queryInterface( rType);
    return aRet.hasValue() ? aRet : OGroup_BASE::queryInterface( rType);
}
// -------------------------------------------------------------------------
Sequence< Type > SAL_CALL OGroup::getTypes(  ) throw(RuntimeException)
{
    return ::comphelper::concatSequences(ODescriptor::getTypes(),OGroup_BASE::getTypes());
}
// -------------------------------------------------------------------------
void OGroup::disposing(void)
{
    OPropertySetHelper::disposing();

    ::osl::MutexGuard aGuard(m_aMutex);

    if(m_pUsers)
        m_pUsers->disposing();
}
// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OGroup::createArrayHelper( ) const
{
    Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);
}
// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper & OGroup::getInfoHelper()
{
    return *const_cast<OGroup*>(this)->getArrayHelper();
}
// -------------------------------------------------------------------------
Reference< XNameAccess > SAL_CALL OGroup::getUsers(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OGroup_BASE::rBHelper.bDisposed);

    try
    {
        if ( !m_pUsers )
            refreshUsers();
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

    return const_cast<OGroup*>(this)->m_pUsers;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OGroup::getPrivileges( const ::rtl::OUString& /*objName*/, sal_Int32 /*objType*/ ) throw(::com::sun::star::sdbc::SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OGroup_BASE::rBHelper.bDisposed);

    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OGroup::getGrantablePrivileges( const ::rtl::OUString& /*objName*/, sal_Int32 /*objType*/ ) throw(::com::sun::star::sdbc::SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OGroup_BASE::rBHelper.bDisposed);

    return 0;
}
// -------------------------------------------------------------------------
void SAL_CALL OGroup::grantPrivileges( const ::rtl::OUString& /*objName*/, sal_Int32 /*objType*/, sal_Int32 /*objPrivileges*/ ) throw(::com::sun::star::sdbc::SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OGroup_BASE::rBHelper.bDisposed);
    throwFeatureNotImplementedException( "XAuthorizable::grantPrivileges", *this );
}
// -------------------------------------------------------------------------
void SAL_CALL OGroup::revokePrivileges( const ::rtl::OUString& /*objName*/, sal_Int32 /*objType*/, sal_Int32 /*objPrivileges*/ ) throw(::com::sun::star::sdbc::SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OGroup_BASE::rBHelper.bDisposed);
    throwFeatureNotImplementedException( "XAuthorizable::revokePrivileges", *this );
}
// -----------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL OGroup::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL OGroup::getName(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return m_Name;
}
// -----------------------------------------------------------------------------
void SAL_CALL OGroup::setName( const ::rtl::OUString& /*aName*/ ) throw(::com::sun::star::uno::RuntimeException)
{
    throwFeatureNotImplementedException( "XNamed::setName", *this );
}
// -----------------------------------------------------------------------------
// XInterface
void SAL_CALL OGroup::acquire() throw()
{
    OGroup_BASE::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OGroup::release() throw()
{
    OGroup_BASE::release();
}
// -----------------------------------------------------------------------------


