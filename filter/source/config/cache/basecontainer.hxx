/*************************************************************************
 *
 *  $RCSfile: basecontainer.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 11:11:29 $
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
#ifndef __FILTER_CONFIG_BASECONTAINER_HXX_
#define __FILTER_CONFIG_BASECONTAINER_HXX_

//_______________________________________________
// includes

#include "filtercache.hxx"

#ifndef _COM_SUN_STAR_UNO_EXCEPTION_HPP_
#include <com/sun/star/uno/Exception.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINERQUERY_HPP_
#include <com/sun/star/container/XContainerQuery.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XFLUSHABLE_HPP_
#include <com/sun/star/util/XFlushable.hpp>
#endif

#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif

#ifndef _SALHELPER_SINGLETONREF_HXX_
#include <salhelper/singletonref.hxx>
#endif

#ifndef _CPPUHELPER_IMPLBASE4_HXX_
#include <cppuhelper/implbase4.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

//_______________________________________________
// namespace

namespace filter{
    namespace config{

//_______________________________________________
// definitions

//_______________________________________________

/** @short      implements the interface css::container::XNameContainer
                on top of a FilterCache reference.

    @descr      This class can be used as base for own service implementations,
                which must provide read/write access to the filter configuration.
                Parameters regulate read/write access, which sub set of informations
                should be available etc.

    @attention  The base class BaseLock must be the first of declared ones.
                Otherwhise we cant be shure, that our own mutex member (which is
                present by this base class!) was full initialized inside our own
                ctor as first!
 */
class BaseContainer : public BaseLock
                    , public ::cppu::WeakImplHelper4< css::lang::XServiceInfo         ,
                                                      css::container::XNameContainer  , // => XNameReplace => XNameAccess => XElementAccess
                                                      css::container::XContainerQuery ,
                                                      css::util::XFlushable           >
{
    //-------------------------------------------
    // member

    protected:

        /** @short  reference to an uno service manager, which can be used
                    to create own needed services. */
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

        /** @short  the implementation name of our derived class, which we provide
                    at the interface XServiceInfo of our class ... */
        ::rtl::OUString m_sImplementationName;

        /** @short  the list of supported uno service names of our derived class, which we provide
                    at the interface XServiceInfo of our class ... */
        css::uno::Sequence< ::rtl::OUString > m_lServiceNames;

        /** @short  reference(!) to a singleton filter cache implementation,
                    which is used to work with the underlying configuration. */
        ::salhelper::SingletonRef< FilterCache > m_rCache;

        /** @short  local filter cache, which is used to collect changes on the
                    filter configuration first and flush it later.

            @descr  Normaly this member isnt used nor initialized. Thats true,
                    if this container is used for reading only. The first write access
                    (e.g. by calling insertByName()) creates a copy of the current
                    global cache m_rCache to initialize the m_pFlushCache member.

                    Afterwards only the flush cache copy is used. Inside flush() this
                    copy will be removed and m_rCache can be used again.

                    m_pFlushCache and m_rCache must not be synchronized manually here.
                    m_rCache listen on the global configuration, where m_pFlushCache
                    write its data. m_rCache update itself automaticly.
         */
        FilterCache* m_pFlushCache;

        /** @short  specify, which sub container of the used filter cache
                    must be wrapped by this container interface. */
        FilterCache::EItemType m_eType;

        /** @short  holds all listener, which are registered at this instance. */
        ::cppu::OMultiTypeInterfaceContainerHelper m_lListener;

        /** @short  hold at least on filter cache instance alive and
                    prevent he office from unloading this cache if no filter
                    is currently used.*/
        static ::salhelper::SingletonRef< FilterCache >* m_pPerformanceOptimizer;

    //-------------------------------------------
    // native interface

    public:

        //---------------------------------------
        // ctor/dtor

        /** @short  standard ctor.

            @descr  Because mostly this class is used as base class for own service
                    implementations in combination with a ImplInheritanceHelper2 template ...
                    there is no way to provide some initializing data through the ctor :-(
                    This base class will be created inside its default ctor and must be
                    initialized with its needed parameters explicitly by calling: "init()".

            @see    init()
         */
        BaseContainer();

        //---------------------------------------

        /** @short  standard dtor.
         */
        virtual ~BaseContainer();

        //---------------------------------------

        /** @short  initialize this generic intsnace with some specialized values
                    from our derived object.

            @descr  Because an outside class must use ImplInheritanceHelper2 template to
                    use us a base class ... and there is no way to pass such initializing
                    parameters trough a required default ctor ... we must be initialized
                    by this special method. Of course this method must be called first before
                    any other interface method is used.

            @param  xSMGR
                    reference to the uno service manager, which created this service instance.

            @param  sImplementationName
                    the implementation name of our derived class, which we provide
                    at the interface XServiceInfo of our class ...

            @param  lServiceNames
                    the list of supported uno service names of our derived class, which we provide
                    at the interface XServiceInfo of our class ...

            @param  eType
                    specify, which sub container of the used filter cache
                    must be wrapped by this container interface.
         */
        virtual void init(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR              ,
                          const ::rtl::OUString&                                        sImplementationName,
                          const css::uno::Sequence< ::rtl::OUString >&                  lServiceNames      ,
                                FilterCache::EItemType                                  eType              );

    //-------------------------------------------
    // helper

    protected:

        //---------------------------------------

        /** @short  check if the underlying configuration data was already loaded
                    and do it if neccessary automaticly.
         */
        void impl_loadOnDemand();

        //---------------------------------------

        /** @short  it creates the global instance m_pFilterCache, which is a copy
                    of the global instance m_rCache, and will be used to change the
                    configuration.

            @descr  If no exception occures, its guaranteed, that the member m_rFlushCache
                    was initialized right and can be used further.
         */
        void impl_initFlushMode()
            throw (css::uno::RuntimeException);

        //---------------------------------------

        /** @short  returns a pointer to the current used cache member.

            @descr  Its a point to the FilterCache instance behind m_pFlushCache
                    or m_rCache.

            @note   The lifetime of this pointer is restricted to the time, where
                    the mutex of this BaseContainer instance is locked.
                    Otherwhise may be the interface method flush() will destroy
                    m_pFlushCache and the here returned pointer will be invalid!

                    Use:

                        Guard aLock(m_aLock);
                            FilterCache* p = impl_getWorkingCache();
                            p->doSomething();
                        aLock.clear();
                        // after this point p cant b e guaranteed any longer!
         */
        FilterCache* impl_getWorkingCache() const;

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
        // XNameContainer

        virtual void SAL_CALL insertByName(const ::rtl::OUString& sItem ,
                                           const css::uno::Any&   aValue)
            throw (css::lang::IllegalArgumentException  ,
                   css::container::ElementExistException,
                   css::lang::WrappedTargetException    ,
                   css::uno::RuntimeException           );

        virtual void SAL_CALL removeByName(const ::rtl::OUString& sItem)
            throw (css::container::NoSuchElementException,
                   css::lang::WrappedTargetException     ,
                   css::uno::RuntimeException            );

        //---------------------------------------
        // XNameReplace

        virtual void SAL_CALL replaceByName(const ::rtl::OUString& sItem ,
                                            const css::uno::Any&   aValue)
            throw (css::lang::IllegalArgumentException   ,
                   css::container::NoSuchElementException,
                   css::lang::WrappedTargetException     ,
                   css::uno::RuntimeException            );

        //---------------------------------------
        // XElementAccess

        virtual css::uno::Any SAL_CALL getByName(const ::rtl::OUString& sItem)
            throw (css::container::NoSuchElementException,
                   css::lang::WrappedTargetException     ,
                   css::uno::RuntimeException            );

        virtual css::uno::Sequence< ::rtl::OUString > SAL_CALL getElementNames()
            throw (css::uno::RuntimeException);

        virtual sal_Bool SAL_CALL hasByName(const ::rtl::OUString& sItem)
            throw (css::uno::RuntimeException);

        virtual css::uno::Type SAL_CALL getElementType()
            throw (css::uno::RuntimeException);

        virtual sal_Bool SAL_CALL hasElements()
            throw (css::uno::RuntimeException);

        //---------------------------------------
        // XContainerQuery

        // must be implemented realy by derived class ...
        // We implement return of an empty result here only!
        // But we show an assertion :-)
        virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createSubSetEnumerationByQuery(const ::rtl::OUString& sQuery)
            throw (css::uno::RuntimeException);

        virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createSubSetEnumerationByProperties(const css::uno::Sequence< css::beans::NamedValue >& lProperties)
            throw (css::uno::RuntimeException);

        //---------------------------------------
        // XFlushable

        virtual void SAL_CALL flush()
            throw (css::uno::RuntimeException);

        virtual void SAL_CALL addFlushListener(const css::uno::Reference< css::util::XFlushListener >& xListener)
            throw (css::uno::RuntimeException);

        virtual void SAL_CALL removeFlushListener(const css::uno::Reference< css::util::XFlushListener >& xListener)
            throw (css::uno::RuntimeException);
};

    } // namespace config
} // namespace filter

#endif // __FILTER_CONFIG_BASECONTAINER_HXX_
