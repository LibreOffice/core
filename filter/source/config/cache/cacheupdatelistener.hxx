/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cacheupdatelistener.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:27:07 $
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
#ifndef __FILTER_CONFIG_CACHEUPDATELISTENER_HXX_
#define __FILTER_CONFIG_CACHEUPDATELISTENER_HXX_

//_______________________________________________
// includes

#include "filtercache.hxx"

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XEVENTLISTENER_HPP_
#include <com/sun/star/lang/XEventListener.hpp>
#endif

#ifndef _COM_SUN_STAR_UTIL_XCHANGESLISTENER_HPP_
#include <com/sun/star/util/XChangesListener.hpp>
#endif

#ifndef _SALHELPER_SINGLETONREF_HXX_
#include <salhelper/singletonref.hxx>
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
        ::salhelper::SingletonRef< FilterCache > m_rCache;

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
        CacheUpdateListener(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR        ,
                            const css::uno::Reference< css::uno::XInterface >&            xConfigAccess,
                                  FilterCache::EItemType                                  eConfigType  );

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
