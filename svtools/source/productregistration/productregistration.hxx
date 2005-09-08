/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: productregistration.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 16:41:20 $
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

#ifndef SVTOOLS_PRODUCTREGISTRATION_HXX
#define SVTOOLS_PRODUCTREGISTRATION_HXX

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XJOBEXECUTOR_HPP_
#include <com/sun/star/task/XJobExecutor.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XJOB_HPP_
#include <com/sun/star/task/XJob.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif

//........................................................................
namespace svt
{
//........................................................................

    //====================================================================
    //= OProductRegistration
    //====================================================================
    typedef ::cppu::WeakImplHelper3 <   ::com::sun::star::lang::XServiceInfo
                                    ,   ::com::sun::star::task::XJobExecutor
                                    ,   ::com::sun::star::task::XJob
                                    >   OProductRegistration_Base;

    class OProductRegistration : public OProductRegistration_Base
    {
    private:
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                    m_xORB;

    protected:
        OProductRegistration( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB );

    public:
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                    Create( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB );

        // XServiceInfo - static version
        static ::rtl::OUString SAL_CALL getImplementationName_Static( );
        static ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames_Static(  ) throw (::com::sun::star::uno::RuntimeException);

    protected:
        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw (::com::sun::star::uno::RuntimeException);

        // XJobExecutor
        virtual void SAL_CALL trigger( const ::rtl::OUString& sEvent ) throw (::com::sun::star::uno::RuntimeException);

        // XJob
        virtual ::com::sun::star::uno::Any SAL_CALL execute( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::NamedValue >& _rArgs ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);

    private:
        // types of events which can be handled by this component
        enum EventType
        {
            etRegistrationRequired,

            etUnknown
        };

        // classifies a event
        EventType classify( const ::rtl::OUString& _rEventDesc );

        // do the online registration
        void doOnlineRegistration( );
    };

//........................................................................
}   // namespace svt
//........................................................................

#endif // SVTOOLS_PRODUCTREGISTRATION_HXX

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.2.122.1  2005/09/05 14:54:05  rt
 *  #i54170# Change license header: remove SISSL
 *
 *  Revision 1.2  2005/04/27 11:14:23  obo
 *  INTEGRATION: CWS mhu07 (1.1.2); FILE ADDED
 *  2005/04/26 13:01:10 mhu 1.1.2.1: #i48107# Moved from svtools/source/inc/
 *
 *  Revision 1.1.2.1  2005/04/26 13:01:10  mhu
 *  #i48107# Moved from svtools/source/inc/
 *
 *  Revision 1.1  2001/12/20 13:12:41  fs
 *  initial checkin - UNO service wrapping the product registration
 *
 *
 *  Revision 1.0 19.12.01 14:21:57  fs
 ************************************************************************/

