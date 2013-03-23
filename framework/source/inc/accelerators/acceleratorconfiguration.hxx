/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#pragma once
#if 1

#include <accelerators/istoragelistener.hxx>
#include <accelerators/presethandler.hxx>
#include <accelerators/acceleratorcache.hxx>
#include <accelerators/keymapping.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <threadhelp/threadhelpbase.hxx>
#include <general.h>
#include <stdtypes.h>

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ui/XAcceleratorConfiguration.hpp>
#include <com/sun/star/ui/XUIConfiguration.hpp>
#include <com/sun/star/ui/XUIConfigurationPersistence.hpp>

#include <com/sun/star/ui/XUIConfigurationStorage.hpp>
#include <com/sun/star/io/XStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/util/XChangesListener.hpp>

// TODO use XPresetHandler interface instead if available
#include <com/sun/star/form/XReset.hpp>

#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/weak.hxx>
#include <comphelper/locale.hxx>
#include <salhelper/singletonref.hxx>

//__________________________________________
// definition

namespace framework
{

const char CFG_ENTRY_PRIMARY[] = "PrimaryKeys";
const char CFG_ENTRY_GLOBAL[] = "Global";
const char CFG_ENTRY_MODULES[] = "Modules";

/** "global" type to make accelerator presets unique, so they can be used
    in combination with the salhelper::SingletonRef mechanism! */
typedef PresetHandler AcceleratorPresets;

//__________________________________________
/**
    implements a read/write access to the accelerator configuration.
 */
class XMLBasedAcceleratorConfiguration : protected ThreadHelpBase                       // attention! Must be the first base class to guarentee right initialize lock ...
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

        XMLBasedAcceleratorConfiguration( const css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR );
        virtual ~XMLBasedAcceleratorConfiguration(                                                                    );

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
        virtual void changesOccurred(const ::rtl::OUString& sPath);

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
        it must call this method with "sal_True". So the internal
        cache can be prepared for that (means copy-on-write ...).

        @return [AcceleratorCache]
        c++ reference(!) to one of our internal caches.
        */
        AcceleratorCache& impl_getCFG(sal_Bool bWriteAccessRequested = sal_False);

};

class XCUBasedAcceleratorConfiguration : protected ThreadHelpBase                       // attention! Must be the first base class to guarentee right initialize lock ...
                                       , public    ::cppu::OWeakObject
                                       , public    css::lang::XTypeProvider
                                       , public    css::util::XChangesListener
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

        css::uno::Reference< css::container::XNameAccess > m_xCfg;
        AcceleratorCache m_aPrimaryReadCache;
        AcceleratorCache m_aSecondaryReadCache;
        AcceleratorCache* m_pPrimaryWriteCache;
        AcceleratorCache* m_pSecondaryWriteCache;

        ::rtl::OUString m_sGlobalOrModules;
        ::rtl::OUString m_sModuleCFG;

        ::salhelper::SingletonRef< KeyMapping > m_rKeyMapping;

        //______________________________________
        // native interface!

    public:

        XCUBasedAcceleratorConfiguration( const css::uno::Reference< css::lang::XMultiServiceFactory > xSMGR );
        virtual ~XCUBasedAcceleratorConfiguration(                                                           );

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

        // css.util.XChangesListener
        virtual void SAL_CALL changesOccurred(const css::util::ChangesEvent& aEvent)
            throw(css::uno::RuntimeException);

        // css.lang.XEventListener
        virtual void SAL_CALL disposing(const css::lang::EventObject& aEvent)
            throw(css::uno::RuntimeException);

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

        void impl_ts_load(sal_Bool bPreferred, const css::uno::Reference< css::container::XNameAccess >& xCfg);
        void impl_ts_save(sal_Bool bPreferred, const css::uno::Reference< css::container::XNameAccess >& xCfg);

        void insertKeyToConfiguration(const css::awt::KeyEvent& aKeyEvent, const ::rtl::OUString& sCommand, const sal_Bool bPreferred);
        void removeKeyFromConfiguration(const css::awt::KeyEvent& aKeyEvent, const sal_Bool bPreferred);

        void reloadChanged(const ::rtl::OUString& sPrimarySecondary, const ::rtl::OUString& sGlobalModules, const ::rtl::OUString& sModule, const ::rtl::OUString& sKey);
        AcceleratorCache& impl_getCFG(sal_Bool bPreferred, sal_Bool bWriteAccessRequested = sal_False);

};

} // namespace framework

#endif // __FRAMEWORK_ACCELERATORS_ACCELERATORCONFIGURATION_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
