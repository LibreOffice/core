/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: VUser.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 02:11:34 $
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

#ifndef _CONNECTIVITY_SDBCX_USER_HXX_
#include "connectivity/sdbcx/VUser.hxx"
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
#ifndef CONNECTIVITY_CONNECTION_HXX
#include "TConnection.hxx"
#endif
#ifndef _CONNECTIVITY_SDBCX_COLLECTION_HXX_
#include "connectivity/sdbcx/VCollection.hxx"
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include <connectivity/dbexception.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif

// -------------------------------------------------------------------------
using namespace connectivity;
using namespace connectivity::sdbcx;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;

IMPLEMENT_SERVICE_INFO(OUser,"com.sun.star.sdbcx.VUser","com.sun.star.sdbcx.User");
// -------------------------------------------------------------------------
OUser::OUser(sal_Bool _bCase)  : OUser_BASE(m_aMutex)
                , ODescriptor(OUser_BASE::rBHelper,_bCase,sal_True)
                , m_pGroups(NULL)
{
}
// -------------------------------------------------------------------------
OUser::OUser(const ::rtl::OUString& _Name,sal_Bool _bCase) :    OUser_BASE(m_aMutex)
                        ,ODescriptor(OUser_BASE::rBHelper,_bCase)
                        ,m_pGroups(NULL)
{
    m_Name = _Name;
}
// -------------------------------------------------------------------------
OUser::~OUser( )
{
    delete m_pGroups;
}
// -------------------------------------------------------------------------
void OUser::disposing(void)
{
    OPropertySetHelper::disposing();
    ::osl::MutexGuard aGuard(m_aMutex);
    if(m_pGroups)
        m_pGroups->disposing();
}
// -------------------------------------------------------------------------
Any SAL_CALL OUser::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = ODescriptor::queryInterface( rType);
    return aRet.hasValue() ? aRet : OUser_BASE::queryInterface( rType);
}
// -------------------------------------------------------------------------
Sequence< Type > SAL_CALL OUser::getTypes(  ) throw(RuntimeException)
{
    return ::comphelper::concatSequences(ODescriptor::getTypes(),OUser_BASE::getTypes());
}
// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* OUser::createArrayHelper( ) const
{
        Sequence< Property > aProps;
    describeProperties(aProps);
    return new ::cppu::OPropertyArrayHelper(aProps);

}
// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper & OUser::getInfoHelper()
{
    return *const_cast<OUser*>(this)->getArrayHelper();
}
// -------------------------------------------------------------------------
// XUser
void SAL_CALL OUser::changePassword( const ::rtl::OUString& /*objPassword*/, const ::rtl::OUString& /*newPassword*/ ) throw(::com::sun::star::sdbc::SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OUser_BASE::rBHelper.bDisposed);
    ::dbtools::throwFeatureNotImplementedException( "XUser::changePassword", *this );
}
// -------------------------------------------------------------------------
// XGroupsSupplier
Reference< XNameAccess > SAL_CALL OUser::getGroups(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OUser_BASE::rBHelper.bDisposed);

    try
    {
        if ( !m_pGroups )
            refreshGroups();
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

    return const_cast<OUser*>(this)->m_pGroups;
}
// -------------------------------------------------------------------------
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OUser::getPrivileges( const ::rtl::OUString& /*objName*/, sal_Int32 /*objType*/ ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OUser_BASE::rBHelper.bDisposed);
    ::dbtools::throwFeatureNotImplementedException( "XAuthorizable::changePassword", *this );
    return 0;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OUser::getGrantablePrivileges( const ::rtl::OUString& /*objName*/, sal_Int32 /*objType*/ ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OUser_BASE::rBHelper.bDisposed);
    ::dbtools::throwFeatureNotImplementedException( "XAuthorizable::getGrantablePrivileges", *this );
    return 0;
}
// -------------------------------------------------------------------------
void SAL_CALL OUser::grantPrivileges( const ::rtl::OUString& /*objName*/, sal_Int32 /*objType*/, sal_Int32 /*objPrivileges*/ ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OUser_BASE::rBHelper.bDisposed);
    ::dbtools::throwFeatureNotImplementedException( "XAuthorizable::grantPrivileges", *this );
}
// -------------------------------------------------------------------------
void SAL_CALL OUser::revokePrivileges( const ::rtl::OUString& /*objName*/, sal_Int32 /*objType*/, sal_Int32 /*objPrivileges*/ ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    checkDisposed(OUser_BASE::rBHelper.bDisposed);
    ::dbtools::throwFeatureNotImplementedException( "XAuthorizable::revokePrivileges", *this );
}
// -----------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL OUser::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL OUser::getName(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return m_Name;
}
// -----------------------------------------------------------------------------
void SAL_CALL OUser::setName( const ::rtl::OUString& /*aName*/ ) throw(::com::sun::star::uno::RuntimeException)
{
    OSL_ENSURE( false, "OUser::setName: not implemented!" );
        // not allowed to throw an SQLException here ...
}
// -----------------------------------------------------------------------------
// XInterface
void SAL_CALL OUser::acquire() throw()
{
    OUser_BASE::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OUser::release() throw()
{
    OUser_BASE::release();
}
// -----------------------------------------------------------------------------


