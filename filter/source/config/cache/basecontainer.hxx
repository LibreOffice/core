/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef __FILTER_CONFIG_BASECONTAINER_HXX_
#define __FILTER_CONFIG_BASECONTAINER_HXX_

#include "filtercache.hxx"
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/util/XRefreshable.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/container/XContainerQuery.hpp>
#include <com/sun/star/util/XFlushable.hpp>
#include <cppuhelper/interfacecontainer.h>
#include <salhelper/singletonref.hxx>
#include <cppuhelper/implbase4.hxx>
#include <cppuhelper/weakref.hxx>
#include <rtl/ustring.hxx>

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
                Otherwise we cant be shure, that our own mutex member (which is
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

        // TODO
        css::uno::WeakReference< css::util::XRefreshable > m_xRefreshBroadcaster;

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
                    Otherwise may be the interface method flush() will destroy
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
