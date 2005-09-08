/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: configflush.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:27:38 $
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
#ifndef __FILTER_CONFIG_CONFIGFLUSH_HXX_
#define __FILTER_CONFIG_CONFIGFLUSH_HXX_

//_______________________________________________
// includes

#include "cacheitem.hxx"

#ifndef _COM_SUN_STAR_UTIL_XREFRESHABLE_HPP_
#include <com/sun/star/util/XRefreshable.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

//_______________________________________________
// namespace

namespace filter{
    namespace config{

//_______________________________________________
// definitions

//_______________________________________________

/** @short      supports registration of XRefreshListener
                on the global filter configuration.

    @descr      Such refresh listener will be called in case the
                type/filter configuration will be changed at runtime.
 */
class ConfigFlush : public BaseLock
                  , public ::cppu::WeakImplHelper1< css::util::XRefreshable >
{
    //-------------------------------------------
    // member

    protected:

        /** @short  reference to an uno service manager, which can be used
                    to create own needed services. */
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

        /** @short  holds all listener, which are registered at this instance. */
        ::cppu::OMultiTypeInterfaceContainerHelper m_lListener;

    //-------------------------------------------
    // native interface

    public:

        //---------------------------------------
        // ctor/dtor

        /** @short  standard ctor.
         */
        ConfigFlush(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR);

        //---------------------------------------

        /** @short  standard dtor.
         */
        virtual ~ConfigFlush();

    //-------------------------------------------
    // uno interface

    public:

        //---------------------------------------
        // XServiceInfo

        virtual ::rtl::OUString SAL_CALL getImplementationName()
            throw (css::uno::RuntimeException);

        virtual sal_Bool SAL_CALL supportsService(const ::rtl::OUString& sServiceName)
            throw (css::uno::RuntimeException);

        virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
            throw (css::uno::RuntimeException);

        //---------------------------------------
        // XRefreshable

        virtual void SAL_CALL refresh()
            throw(css::uno::RuntimeException);

        virtual void SAL_CALL addRefreshListener(const css::uno::Reference< css::util::XRefreshListener >& xListener)
            throw(css::uno::RuntimeException);

        virtual void SAL_CALL removeRefreshListener(const css::uno::Reference< css::util::XRefreshListener >& xListener)
            throw(css::uno::RuntimeException);

        //---------------------------------------
        // interface to register/create this instance as an UNO service
        static ::rtl::OUString impl_getImplementationName();
        static css::uno::Sequence< ::rtl::OUString > impl_getSupportedServiceNames();
        static css::uno::Reference< css::uno::XInterface > impl_createInstance(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR);
};

    } // namespace config
} // namespace filter

#endif // __FILTER_CONFIG_CONFIGFLUSH_HXX_
