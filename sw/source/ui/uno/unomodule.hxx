/*************************************************************************
 *
 *  $RCSfile: unomodule.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2003-09-19 08:49:28 $
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

#ifndef _SW_UNOMODULE_HXX
#define _SW_UNOMODULE_HXX

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HPP_
#include <com/sun/star/uno/RuntimeException.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_DISPATCHDESCRIPTOR_HPP_
#include <com/sun/star/frame/DispatchDescriptor.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef _CPPUHELPER_IMPLBASE3_HXX_
#include <cppuhelper/implbase3.hxx>
#endif

#include <com/sun/star/lang/XServiceInfo.hpp>

namespace com
{
    namespace sun
    {
        namespace star
        {
            namespace lang
            {
                class XMultiServiceFactory;
            }
            namespace beans
            {
                struct PropertyValue;
            }
        }
    }
}

#define REFERENCE               ::com::sun::star::uno::Reference
#define SEQUENCE                ::com::sun::star::uno::Sequence
#define RUNTIMEEXCEPTION        ::com::sun::star::uno::RuntimeException
#define REFERENCE               ::com::sun::star::uno::Reference
#define SEQUENCE                ::com::sun::star::uno::Sequence
#define XDISPATCH               ::com::sun::star::frame::XDispatch
#define OUSTRING                ::rtl::OUString
#define UNOURL                  ::com::sun::star::util::URL
#define DISPATCHDESCRIPTOR      ::com::sun::star::frame::DispatchDescriptor

class SwUnoModule : public ::cppu::WeakImplHelper3< ::com::sun::star::frame::XDispatchProvider, ::com::sun::star::frame::XDispatch, ::com::sun::star::lang::XServiceInfo >
{
    REFERENCE < ::com::sun::star::lang::XMultiServiceFactory > m_xFactory;

public:
                            SwUnoModule( const REFERENCE < ::com::sun::star::lang::XMultiServiceFactory >& xFactory )
                                : m_xFactory( xFactory )
                            {}

    // XDispatch
    virtual void SAL_CALL dispatch( const ::com::sun::star::util::URL& aURL, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgs ) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL addStatusListener(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & xControl, const ::com::sun::star::util::URL& aURL) throw( ::com::sun::star::uno::RuntimeException );
    virtual void SAL_CALL removeStatusListener(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & xControl, const ::com::sun::star::util::URL& aURL) throw( ::com::sun::star::uno::RuntimeException );

    // XDispatchProvider
    virtual SEQUENCE< REFERENCE< XDISPATCH > > SAL_CALL queryDispatches( const SEQUENCE< DISPATCHDESCRIPTOR >& seqDescriptor ) throw( RUNTIMEEXCEPTION ) ;
    virtual REFERENCE< XDISPATCH > SAL_CALL queryDispatch(  const   UNOURL &            aURL            ,
                                                            const   OUSTRING &          sTargetFrameName,
                                                                    sal_Int32   eSearchFlags    ) throw( RUNTIMEEXCEPTION ) ;
    // XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);
};

#endif
