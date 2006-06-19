/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: acceleratorconfiguration.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 11:21:05 $
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

#ifndef __FRAMEWORK_ACCELERATORS_ACCELERATORCONFIGURATION_HXX_
#define __FRAMEWORK_ACCELERATORS_ACCELERATORCONFIGURATION_HXX_

//__________________________________________
// own includes

#ifndef __FRAMEWORK_ACCELERATORS_ISTORAGELISTENER_HXX_
#include <accelerators/istoragelistener.hxx>
#endif

#ifndef __FRAMEWORK_ACCELERATORS_PRESETHANDLER_HXX_
#include <accelerators/presethandler.hxx>
#endif

#ifndef __FRAMEWORK_ACCELERATORS_ACCELERATORCACHE_HXX_
#include <accelerators/acceleratorcache.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XINTERFACE_HXX_
#include <macros/xinterface.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XTYPEPROVIDER_HXX_
#include <macros/xtypeprovider.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#include <threadhelp/threadhelpbase.hxx>
#endif

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

#ifndef __FRAMEWORK_STDTYPES_H_
#include <stdtypes.h>
#endif

//__________________________________________
// interface includes

#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_UI_XACCELERATORCONFIGURATION_HPP_
#include <com/sun/star/ui/XAcceleratorConfiguration.hpp>
#endif

#ifndef _COM_SUN_STAR_UI_XUICONFIGURATION_HPP_
#include <com/sun/star/ui/XUIConfiguration.hpp>
#endif

#ifndef _COM_SUN_STAR_UI_XUICONFIGURATIONPERSISTENCE_HPP_
#include <com/sun/star/ui/XUIConfigurationPersistence.hpp>
#endif

#ifndef _COM_SUN_STAR_UI_XUICONFIGURATIONSTORAGE_HDL_
#include <com/sun/star/ui/XUIConfigurationStorage.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XSTREAM_HPP_
#include <com/sun/star/io/XStream.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif

// TODO use XPresetHandler interface instead if available
#ifndef _COM_SUN_STAR_FORM_XRESET_HPP_
#include <com/sun/star/form/XReset.hpp>
#endif

//__________________________________________
// other includes

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#ifndef _COMPHELPER_LOCALE_HXX_
#include <comphelper/locale.hxx>
#endif

//__________________________________________
// definition

namespace framework
{

/** "global" type to make accelerator presets unique, so they can be used
    in combination with the salhelper::SingletonRef mechanism! */
typedef PresetHandler AcceleratorPresets;

//__________________________________________
/**
    implements a read/write access to the accelerator configuration.
 */
class AcceleratorConfiguration : protected ThreadHelpBase                       // attention! Must be the first base class to guarentee right initialize lock ...
                               , public    IStorageListener
                               , public    ::cppu::OWeakObject
                               , public    css::lang::XTypeProvider
                               , public    css::form::XReset                    // TODO use XPresetHandler instead if available
                               , public    css::ui::XAcceleratorConfiguration  // => css::ui::XUIConfigurationPersistence
                                                                                //    css::ui::XUIConfigurationStorage
                                                                                //    css::ui::XUIConfiguration
{
    //______________________________________
    // member

    protected:

        //---------------------------------------
        /** the global uno service manager.
            Must be used to create own needed services. */
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xSMGR;

        //---------------------------------------
        /** used to:
            i  ) copy configuration files from the share to the user layer
            ii ) provide access to these config files
            iii) cache all sub storages on the path from the top to the bottom(!)
            iv ) provide commit for changes. */
        PresetHandler m_aPresetHandler;

        //---------------------------------------
        /** contains the cached configuration data */
        AcceleratorCache m_aReadCache;

        //---------------------------------------
        /** used to implement the copy on write pattern! */
        AcceleratorCache* m_pWriteCache;

    //______________________________________
    // native interface!

    public:

                 AcceleratorConfiguration( const css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR );
        virtual ~AcceleratorConfiguration(                                                                    );

    //______________________________________
    // uno interface!

    public:

        // XInterface, XTypeProvider
        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER

        // XAcceleratorConfiguration
        virtual css::uno::Sequence< css::awt::KeyEvent > SAL_CALL getAllKeyEvents()
            throw(css::uno::RuntimeException);

        virtual ::rtl::OUString SAL_CALL getCommandByKeyEvent(const css::awt::KeyEvent& aKeyEvent)
            throw(css::container::NoSuchElementException,
                  css::uno::RuntimeException            );

        virtual void SAL_CALL setKeyEvent(const css::awt::KeyEvent& aKeyEvent,
                                          const ::rtl::OUString&    sCommand )
            throw(css::lang::IllegalArgumentException,
                  css::uno::RuntimeException         );

        virtual void SAL_CALL removeKeyEvent(const css::awt::KeyEvent& aKeyEvent)
            throw(css::container::NoSuchElementException,
                  css::uno::RuntimeException            );

        virtual css::uno::Sequence< css::awt::KeyEvent > SAL_CALL getKeyEventsByCommand(const ::rtl::OUString& sCommand)
            throw(css::lang::IllegalArgumentException   ,
                  css::container::NoSuchElementException,
                  css::uno::RuntimeException            );

        virtual css::uno::Sequence< css::uno::Any > SAL_CALL getPreferredKeyEventsForCommandList(const css::uno::Sequence< ::rtl::OUString >& lCommandList)
            throw(css::lang::IllegalArgumentException   ,
                  css::uno::RuntimeException            );

        virtual void SAL_CALL removeCommandFromAllKeyEvents(const ::rtl::OUString& sCommand)
            throw(css::lang::IllegalArgumentException   ,
                  css::container::NoSuchElementException,
                  css::uno::RuntimeException            );

        // XUIConfigurationPersistence
        virtual void SAL_CALL reload()
            throw(css::uno::Exception       ,
                  css::uno::RuntimeException);

        virtual void SAL_CALL store()
            throw(css::uno::Exception       ,
                  css::uno::RuntimeException);

        virtual void SAL_CALL storeToStorage(const css::uno::Reference< css::embed::XStorage >& xStorage)
            throw(css::uno::Exception       ,
                  css::uno::RuntimeException);

        virtual ::sal_Bool SAL_CALL isModified()
            throw(css::uno::RuntimeException);

        virtual ::sal_Bool SAL_CALL isReadOnly()
            throw(css::uno::RuntimeException);

        // XUIConfigurationStorage
        virtual void SAL_CALL setStorage(const css::uno::Reference< css::embed::XStorage >& xStorage)
            throw(css::uno::RuntimeException);

        virtual ::sal_Bool SAL_CALL hasStorage()
            throw(css::uno::RuntimeException);

        // XUIConfiguration
        virtual void SAL_CALL addConfigurationListener(const css::uno::Reference< css::ui::XUIConfigurationListener >& xListener)
            throw(css::uno::RuntimeException);

        virtual void SAL_CALL removeConfigurationListener(const css::uno::Reference< css::ui::XUIConfigurationListener >& xListener)
            throw(css::uno::RuntimeException);

        // XReset
        // TODO use XPresetHandler instead if available
        virtual void SAL_CALL reset()
            throw(css::uno::RuntimeException);

        virtual void SAL_CALL addResetListener(const css::uno::Reference< css::form::XResetListener >& xListener)
            throw(css::uno::RuntimeException);

        virtual void SAL_CALL removeResetListener(const css::uno::Reference< css::form::XResetListener >& xListener)
            throw(css::uno::RuntimeException);

        // IStorageListener
        virtual void changesOccured(const ::rtl::OUString& sPath);

    //______________________________________
    // helper for derived classes

    protected:

        //---------------------------------------
        /** @short  return the current office locale.

            @descr  We does not cache this value, because we are not listen
                    for changes on the configuration layer ...

            @return ::comphelper::Locale
                    The current office locale.
         */
        ::comphelper::Locale impl_ts_getLocale() const;

    //______________________________________
    // helper

    private:

        //---------------------------------------
        /** @short  load a configuration set, using the given stream.

            @param  xStream
                    provides the XML structure as stream.
         */
        void impl_ts_load(const css::uno::Reference< css::io::XInputStream >& xStream);

        //---------------------------------------
        /** @short  save a configuration set, using the given stream.

            @param  xStream
                    the XML structure can be written there.
         */
        void impl_ts_save(const css::uno::Reference< css::io::XOutputStream >& xStream);

        //---------------------------------------
        /** @short  try to locate and open a sub storage.

            @descr  It search at the root storage for the specified
                    sub storage. If it exists - it will be opened.
                    By default this method tries to open the storage
                    for reading. But the calli can request a writeable
                    storage.

            @param  xRooStorage
                    used to locate the sub storage.

            @param  sSubStorage
                    relativ path of the sub storage.

            @param  bOutStream
                    force open of the sub storage in
                    write mode - instead of read mode, which
                    is the default.

            @return [XInterface]
                    will be a css::io::XInpoutStream or a css::io::XOutputStream.
                    Depends from the parameter bWriteable!
         */
        css::uno::Reference< css::uno::XInterface > impl_ts_openSubStorage(const css::uno::Reference< css::embed::XStorage >& xRootStorage,
                                                                           const ::rtl::OUString&                             sSubStorage ,
                                                                                 sal_Bool                                     bOutStream  );

        //---------------------------------------
        /** @short  returns a reference to one of our internal cache members.

            @descr  We implement the copy-on-write pattern. Doing so
                    we know two caches internaly. The second one is used
                    only, if the container was changed.

                    This method here returns access to one of these
                    caches - depending on the change state of this
                    configuration service.

            @param  bWriteAccessRequested
                    if the outside code whish to change the container
                    it must call this method with "TRUE". So the internal
                    cache can be prepared for that (means copy-on-write ...).

            @return [AcceleratorCache]
                    c++ reference(!) to one of our internal caches.
         */
        AcceleratorCache& impl_getCFG(sal_Bool bWriteAccessRequested = sal_False);
};

} // namespace framework

#endif // __FRAMEWORK_ACCELERATORS_ACCELERATORCONFIGURATION_HXX_
