/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: HTerminateListener.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 06:05:02 $
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
#ifndef CONNECTIVITY_HSQLDB_TERMINATELISTENER_HXX
#define CONNECTIVITY_HSQLDB_TERMINATELISTENER_HXX

#ifndef _CPPUHELPER_COMPBASE2_HXX_
#include <cppuhelper/compbase2.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XTERMINATELISTENER_HPP_
#include <com/sun/star/frame/XTerminateListener.hpp>
#endif

//........................................................................
namespace connectivity
{
//........................................................................

    namespace hsqldb
    {
        class ODriverDelegator;
        class OConnectionController : public ::cppu::WeakImplHelper2<
                                                    ::com::sun::star::lang::XServiceInfo,
                                                    ::com::sun::star::frame::XTerminateListener >
        {
            ODriverDelegator* m_pDriver;
            protected:
                virtual ~OConnectionController() {m_pDriver = NULL;}
            public:
                OConnectionController(ODriverDelegator* _pDriver) : m_pDriver(_pDriver){}

                // XServiceInfo
                virtual ::rtl::OUString SAL_CALL getImplementationName()
                    throw ( ::com::sun::star::uno::RuntimeException );
                virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
                    throw ( ::com::sun::star::uno::RuntimeException );
                virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
                    throw ( ::com::sun::star::uno::RuntimeException );

                // XEventListener
                virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source )
                    throw( ::com::sun::star::uno::RuntimeException );

                // XTerminateListener
                virtual void SAL_CALL queryTermination( const ::com::sun::star::lang::EventObject& aEvent )
                    throw( ::com::sun::star::frame::TerminationVetoException, ::com::sun::star::uno::RuntimeException );
                virtual void SAL_CALL notifyTermination( const ::com::sun::star::lang::EventObject& aEvent )
                    throw( ::com::sun::star::uno::RuntimeException );
        };
    }
//........................................................................
}   // namespace connectivity
//........................................................................
#endif // CONNECTIVITY_HSQLDB_TERMINATELISTENER_HXX
