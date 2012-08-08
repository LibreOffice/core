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
#ifndef __FILTER_CONFIG_CACHEUPDATELISTENER_HXX_
#define __FILTER_CONFIG_CACHEUPDATELISTENER_HXX_

#include "filtercache.hxx"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XEventListener.hpp>
#include <com/sun/star/util/XChangesListener.hpp>
#include <salhelper/singletonref.hxx>
#include <cppuhelper/implbase1.hxx>


namespace filter{
    namespace config{


//_______________________________________________

/** @short      implements a listener, which will update the
                global filter cache, if the underlying configuration
                wa changed by other processes.
 */
class CacheUpdateListener : public BaseLock // must be the first one to guarantee right initialized mutex member!
                          , public ::cppu::WeakImplHelper1< css::util::XChangesListener >
{
    //-------------------------------------------
    // member

    private:

        /** @short  reference to an uno service manager, which can be used
                    to create own needed services. */
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

        /** @short  reference to the singleton(!) filter cache implementation,
                    which should be updated by this thread. */
        FilterCache &m_rCache;

        /** @short  holds the configuration access, where we listen alive. */
        css::uno::Reference< css::uno::XInterface > m_xConfig;

        /** @short  every instance of this update listener listen on
                    a special sub set of the filter configuration.
                    So it should know, which type of configuration entry
                    it must put into the filter cache, if the configuration notifys changes ... */
        FilterCache::EItemType m_eConfigType;

    //-------------------------------------------
    // native interface

    public:

        //---------------------------------------
        // ctor/dtor

        /** @short  initialize new instance of this class.

            @descr  Listening wont be started here. It can be done
                    by calling startListening() on this instance.

            @see    startListening()

            @param  xSMGR
                    reference to a service manager, which can be used to create
                    own needed uno services.

            @param  xConfigAccess
                    the configuration access, where this instance should listen for changes.

            @param  eConfigType
                    specify the type of configuration.
         */
        CacheUpdateListener(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR,
                            FilterCache &rFilterCache,
                            const css::uno::Reference< css::uno::XInterface >& xConfigAccess,
                            FilterCache::EItemType eConfigType);

        //---------------------------------------

        /** @short  standard dtor.
         */
        virtual ~CacheUpdateListener();

        //---------------------------------------

        /** @short  starts listening.
         */
        virtual void startListening();

        //---------------------------------------

        /** @short  stop listening.
         */
        virtual void stopListening();

    //-------------------------------------------
    // uno interface

    public:

        //---------------------------------------
        // XChangesListener

        virtual void SAL_CALL changesOccurred(const css::util::ChangesEvent& aEvent)
            throw(css::uno::RuntimeException);

        //---------------------------------------
        // lang.XEventListener
        virtual void SAL_CALL disposing(const css::lang::EventObject& aEvent)
            throw(css::uno::RuntimeException);
};

    } // namespace config
} // namespace filter

#endif // __FILTER_CONFIG_CACHEUPDATELISTENER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
