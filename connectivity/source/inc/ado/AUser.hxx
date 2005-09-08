/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AUser.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 06:54:35 $
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

#ifndef _CONNECTIVITY_ADO_USER_HXX_
#define _CONNECTIVITY_ADO_USER_HXX_

#ifndef _CONNECTIVITY_SDBCX_USER_HXX_
#include "connectivity/sdbcx/VUser.hxx"
#endif
#ifndef _CONNECTIVITY_ADO_AWRAPADOX_HXX_
#include "ado/Awrapadox.hxx"
#endif

namespace connectivity
{
    namespace ado
    {
        class OCatalog;
        typedef connectivity::sdbcx::OUser OUser_TYPEDEF;

        class OAdoUser : public OUser_TYPEDEF
        {
        protected:
            WpADOUser   m_aUser;
            OCatalog*   m_pCatalog;

            virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                            sal_Int32 nHandle,
                                    const ::com::sun::star::uno::Any& rValue
                         )
                         throw (::com::sun::star::uno::Exception);
            virtual void SAL_CALL getFastPropertyValue(
                                    ::com::sun::star::uno::Any& rValue,
                                    sal_Int32 nHandle
                                         ) const;
        public:
            virtual void refreshGroups();
        public:
            OAdoUser(OCatalog* _pParent,sal_Bool _bCase,    ADOUser* _pUser=NULL);
            OAdoUser(OCatalog* _pParent,sal_Bool _bCase,  const ::rtl::OUString& _Name);

            // com::sun::star::lang::XUnoTunnel
            virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);
            static ::com::sun::star::uno::Sequence< sal_Int8 > getUnoTunnelImplementationId();
            virtual void SAL_CALL acquire() throw();
            virtual void SAL_CALL release() throw();

            // XUser
            virtual void SAL_CALL changePassword( const ::rtl::OUString& objPassword, const ::rtl::OUString& newPassword ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            // XAuthorizable
            virtual sal_Int32 SAL_CALL getPrivileges( const ::rtl::OUString& objName, sal_Int32 objType ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual sal_Int32 SAL_CALL getGrantablePrivileges( const ::rtl::OUString& objName, sal_Int32 objType ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL grantPrivileges( const ::rtl::OUString& objName, sal_Int32 objType, sal_Int32 objPrivileges ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);
            virtual void SAL_CALL revokePrivileges( const ::rtl::OUString& objName, sal_Int32 objType, sal_Int32 objPrivileges ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException);

            WpADOUser getImpl() const { return m_aUser;}
        };

        class OUserExtend;
        typedef ::comphelper::OPropertyArrayUsageHelper<OUserExtend> OUserExtend_PROP;

        class OUserExtend : public OAdoUser,
                            public OUserExtend_PROP
        {
        protected:
            ::rtl::OUString m_Password;
            // OPropertyArrayUsageHelper
            virtual ::cppu::IPropertyArrayHelper* createArrayHelper() const;
            // OPropertySetHelper
            virtual ::cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper();
        public:
            OUserExtend(OCatalog* _pParent,sal_Bool _bCase,ADOUser* _pUser=NULL);
            OUserExtend(OCatalog* _pParent,sal_Bool _bCase,const ::rtl::OUString& _Name);

            virtual void construct();
            ::rtl::OUString getPassword() const { return m_Password;}
        };
    }
}
#endif // _CONNECTIVITY_ADO_USER_HXX_

