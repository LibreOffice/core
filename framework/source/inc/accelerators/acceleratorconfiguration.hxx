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

#ifndef INCLUDED_FRAMEWORK_SOURCE_INC_ACCELERATORS_ACCELERATORCONFIGURATION_HXX
#define INCLUDED_FRAMEWORK_SOURCE_INC_ACCELERATORS_ACCELERATORCONFIGURATION_HXX

#include <accelerators/presethandler.hxx>
#include <accelerators/acceleratorcache.hxx>

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/ui/XAcceleratorConfiguration.hpp>

#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/util/XChangesListener.hpp>

// TODO use XPresetHandler interface instead if available
#include <com/sun/star/form/XReset.hpp>

#include <cppuhelper/implbase.hxx>

// definition

namespace framework
{

const OUStringLiteral CFG_ENTRY_PRIMARY = u"PrimaryKeys";
const OUStringLiteral CFG_ENTRY_GLOBAL = u"Global";
const OUStringLiteral CFG_ENTRY_MODULES = u"Modules";

/**
    implements a read/write access to the accelerator configuration.
 */
class XMLBasedAcceleratorConfiguration : public    ::cppu::WeakImplHelper<
                                                       css::form::XReset,                    // TODO use XPresetHandler instead if available
                                                       css::ui::XAcceleratorConfiguration >  // => css::ui::XUIConfigurationPersistence
                                                                                             //    css::ui::XUIConfigurationStorage
                                                                                             //    css::ui::XUIConfiguration
{

    // member

    protected:

        /** the global uno service manager.
        Must be used to create own needed services. */
        css::uno::Reference< css::uno::XComponentContext > m_xContext;

        /** used to:
        i  ) copy configuration files from the share to the user layer
        ii ) provide access to these config files
        iii) cache all sub storages on the path from the top to the bottom(!)
        iv ) provide commit for changes. */
        PresetHandler m_aPresetHandler;

        /** contains the cached configuration data */
        AcceleratorCache m_aReadCache;

        /** used to implement the copy on write pattern! */
        std::unique_ptr<AcceleratorCache> m_pWriteCache;

        // native interface!

    public:

        XMLBasedAcceleratorConfiguration( const css::uno::Reference< css::uno::XComponentContext >& xContext);
        virtual ~XMLBasedAcceleratorConfiguration(                                                                    ) override;

        // uno interface!

    public:

        // XAcceleratorConfiguration
        virtual css::uno::Sequence< css::awt::KeyEvent > SAL_CALL getAllKeyEvents() override;

        virtual OUString SAL_CALL getCommandByKeyEvent(const css::awt::KeyEvent& aKeyEvent) override;

        virtual void SAL_CALL setKeyEvent(const css::awt::KeyEvent& aKeyEvent,
            const OUString&    sCommand ) override;

        virtual void SAL_CALL removeKeyEvent(const css::awt::KeyEvent& aKeyEvent) override;

        virtual css::uno::Sequence< css::awt::KeyEvent > SAL_CALL getKeyEventsByCommand(const OUString& sCommand) override;

        virtual css::uno::Sequence< css::uno::Any > SAL_CALL getPreferredKeyEventsForCommandList(const css::uno::Sequence< OUString >& lCommandList) override;

        virtual void SAL_CALL removeCommandFromAllKeyEvents(const OUString& sCommand) override;

        // XUIConfigurationPersistence
        virtual void SAL_CALL reload() override;

        virtual void SAL_CALL store() override;

        virtual void SAL_CALL storeToStorage(const css::uno::Reference< css::embed::XStorage >& xStorage) override;

        virtual sal_Bool SAL_CALL isModified() override;

        virtual sal_Bool SAL_CALL isReadOnly() override;

        // XUIConfigurationStorage
        virtual void SAL_CALL setStorage(const css::uno::Reference< css::embed::XStorage >& xStorage) override;

        virtual sal_Bool SAL_CALL hasStorage() override;

        // XUIConfiguration
        virtual void SAL_CALL addConfigurationListener(const css::uno::Reference< css::ui::XUIConfigurationListener >& xListener) override;

        virtual void SAL_CALL removeConfigurationListener(const css::uno::Reference< css::ui::XUIConfigurationListener >& xListener) override;

        // XReset
        // TODO use XPresetHandler instead if available
        virtual void SAL_CALL reset() override;

        virtual void SAL_CALL addResetListener(const css::uno::Reference< css::form::XResetListener >& xListener) override;

        virtual void SAL_CALL removeResetListener(const css::uno::Reference< css::form::XResetListener >& xListener) override;

        // called when changes occurred in the storage
        void changesOccurred();

        // helper for derived classes

    protected:

        /** @short  return the current office locale.

        @descr  We do not cache this value, because we are not listen
        for changes on the configuration layer ...

        @return OUString
        The current office locale as BCP47 string.
        */
        OUString impl_ts_getLocale() const;

        // helper

    private:

        /** @short  load a configuration set, using the given stream.

        @param  xStream
        provides the XML structure as stream.
        */
        void impl_ts_load(const css::uno::Reference< css::io::XInputStream >& xStream);

        /** @short  save a configuration set, using the given stream.

        @param  xStream
        the XML structure can be written there.
        */
        void impl_ts_save(const css::uno::Reference< css::io::XOutputStream >& xStream);

        /** @short  returns a reference to one of our internal cache members.

        @descr  We implement the copy-on-write pattern. Doing so
        we know two caches internally. The second one is used
        only, if the container was changed.

        This method here returns access to one of these
        caches - depending on the change state of this
        configuration service.

        @param  bWriteAccessRequested
        if the outside code wish to change the container
        it must call this method with "sal_True". So the internal
        cache can be prepared for that (means copy-on-write ...).

        @return [AcceleratorCache]
        c++ reference(!) to one of our internal caches.
        */
        AcceleratorCache& impl_getCFG(bool bWriteAccessRequested = false);

};

class XCUBasedAcceleratorConfiguration : public  ::cppu::WeakImplHelper<
                                                     css::util::XChangesListener,
                                                     css::form::XReset,                    // TODO use XPresetHandler instead if available
                                                     css::ui::XAcceleratorConfiguration >  // => css::ui::XUIConfigurationPersistence
                                                                                           //    css::ui::XUIConfigurationStorage
                                                                                           //    css::ui::XUIConfiguration
{

    // member

    protected:

        /** the global uno service manager.
        Must be used to create own needed services. */
        css::uno::Reference< css::uno::XComponentContext > m_xContext;

        css::uno::Reference< css::container::XNameAccess > m_xCfg;
        AcceleratorCache m_aPrimaryReadCache;
        AcceleratorCache m_aSecondaryReadCache;
        std::unique_ptr<AcceleratorCache> m_pPrimaryWriteCache;
        std::unique_ptr<AcceleratorCache> m_pSecondaryWriteCache;

        OUString m_sGlobalOrModules;
        OUString m_sModuleCFG;

        // native interface!

    public:

        XCUBasedAcceleratorConfiguration( const css::uno::Reference< css::uno::XComponentContext >& xContext );
        virtual ~XCUBasedAcceleratorConfiguration(                                                           ) override;

        // uno interface!

    public:

        // XAcceleratorConfiguration
        virtual css::uno::Sequence< css::awt::KeyEvent > SAL_CALL getAllKeyEvents() override;

        virtual OUString SAL_CALL getCommandByKeyEvent(const css::awt::KeyEvent& aKeyEvent) override;

        virtual void SAL_CALL setKeyEvent(const css::awt::KeyEvent& aKeyEvent,
            const OUString&    sCommand ) override;

        virtual void SAL_CALL removeKeyEvent(const css::awt::KeyEvent& aKeyEvent) override;

        virtual css::uno::Sequence< css::awt::KeyEvent > SAL_CALL getKeyEventsByCommand(const OUString& sCommand) override;

        virtual css::uno::Sequence< css::uno::Any > SAL_CALL getPreferredKeyEventsForCommandList(const css::uno::Sequence< OUString >& lCommandList) override;

        virtual void SAL_CALL removeCommandFromAllKeyEvents(const OUString& sCommand) override;

        // XUIConfigurationPersistence
        virtual void SAL_CALL reload() override;

        virtual void SAL_CALL store() override;

        virtual void SAL_CALL storeToStorage(const css::uno::Reference< css::embed::XStorage >& xStorage) override;

        virtual sal_Bool SAL_CALL isModified() override;

        virtual sal_Bool SAL_CALL isReadOnly() override;

        // XUIConfigurationStorage
        virtual void SAL_CALL setStorage(const css::uno::Reference< css::embed::XStorage >& xStorage) override;

        virtual sal_Bool SAL_CALL hasStorage() override;

        // XUIConfiguration
        virtual void SAL_CALL addConfigurationListener(const css::uno::Reference< css::ui::XUIConfigurationListener >& xListener) override;

        virtual void SAL_CALL removeConfigurationListener(const css::uno::Reference< css::ui::XUIConfigurationListener >& xListener) override;

        // XReset
        // TODO use XPresetHandler instead if available
        virtual void SAL_CALL reset() override;

        virtual void SAL_CALL addResetListener(const css::uno::Reference< css::form::XResetListener >& xListener) override;

        virtual void SAL_CALL removeResetListener(const css::uno::Reference< css::form::XResetListener >& xListener) override;

        // css.util.XChangesListener
        virtual void SAL_CALL changesOccurred(const css::util::ChangesEvent& aEvent) override;

        // css.lang.XEventListener
        virtual void SAL_CALL disposing(const css::lang::EventObject& aEvent) override;

        // helper for derived classes

    protected:

        /** @short  return the current office locale.

        @descr  We do not cache this value, because we are not listen
        for changes on the configuration layer ...

        @return OUString
        The current office locale as BCP47 string.
        */
        OUString impl_ts_getLocale() const;

        // helper

    private:

        void impl_ts_load(bool bPreferred, const css::uno::Reference< css::container::XNameAccess >& xCfg);
        void impl_ts_save(bool bPreferred);

        void insertKeyToConfiguration(const css::awt::KeyEvent& aKeyEvent, const OUString& sCommand, const bool bPreferred);
        void removeKeyFromConfiguration(const css::awt::KeyEvent& aKeyEvent, const bool bPreferred);

        void reloadChanged(const OUString& sPrimarySecondary, const OUString& sGlobalModules, const OUString& sModule, const OUString& sKey);
        AcceleratorCache& impl_getCFG(bool bPreferred, bool bWriteAccessRequested = false);

};

} // namespace framework

#endif // INCLUDED_FRAMEWORK_SOURCE_INC_ACCELERATORS_ACCELERATORCONFIGURATION_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
