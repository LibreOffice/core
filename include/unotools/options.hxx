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

#ifndef INCLUDED_UNOTOOLS_OPTIONS_HXX
#define INCLUDED_UNOTOOLS_OPTIONS_HXX

#include <sal/config.h>
#include <unotools/unotoolsdllapi.h>
#include <o3tl/typed_flags_set.hxx>
#include <vector>
#include <memory>

// bits for broadcasting hints of changes in ConfigurationListener::ConfigurationChanged, may be combined
enum class ConfigurationHints {
    NONE               = 0x0000,
    Locale             = 0x0001,
    Currency           = 0x0002,
    UiLocale           = 0x0004,
    DecSep             = 0x0008,
    DatePatterns       = 0x0010,
    IgnoreLang         = 0x0020,
    CtlSettingsChanged = 0x2000,
};
namespace o3tl {
    template<> struct typed_flags<ConfigurationHints> : is_typed_flags<ConfigurationHints, 0x203f> {};
}

/*
    The class utl::detail::Options provides a kind of multiplexer. It implements a ConfigurationListener
    that is usually registered at a ConfigItem class. At the same time it implements a ConfigurationBroadcaster
    that allows further ("external") listeners to register.
    Once the class deriving from Options is notified about
    configuration changes by the ConfigItem if its content has been changed by calling some of its methods,
    a call of the Options::NotifyListeners() method will send out notifications to all external listeners.
*/

namespace utl {

    class ConfigurationBroadcaster;

    // interface for configuration listener
    class UNOTOOLS_DLLPUBLIC ConfigurationListener
    {
    public:
        virtual ~ConfigurationListener();

        virtual void ConfigurationChanged( ConfigurationBroadcaster* p, ConfigurationHints nHint ) = 0;
    };
    typedef ::std::vector< ConfigurationListener* > IMPL_ConfigurationListenerList;

    // complete broadcasting implementation
    class UNOTOOLS_DLLPUBLIC ConfigurationBroadcaster
    {
        std::unique_ptr<IMPL_ConfigurationListenerList> mpList;
        sal_Int32               m_nBroadcastBlocked;     // broadcast only if this is 0
        ConfigurationHints      m_nBlockedHint;

    public:
        void AddListener( utl::ConfigurationListener* pListener );
        void RemoveListener( utl::ConfigurationListener const * pListener );

        // notify listeners; nHint is an implementation detail of the particular class deriving from ConfigurationBroadcaster
        void NotifyListeners( ConfigurationHints nHint );
        ConfigurationBroadcaster();
        ConfigurationBroadcaster(ConfigurationBroadcaster const & );
        virtual ~ConfigurationBroadcaster();
        virtual void BlockBroadcasts( bool bBlock );
    };

namespace detail {

// A base class for the various option classes supported by
// unotools/source/config/itemholderbase.hxx (which must be public, as it is
// shared between unotools, svl and svt)
// It also provides an implementation for a Configuration Listener and inherits a broadcaster implementation

class SAL_WARN_UNUSED UNOTOOLS_DLLPUBLIC Options
    : public utl::ConfigurationBroadcaster, public utl::ConfigurationListener
{
public:
    Options();

    virtual ~Options() override = 0;

private:
    Options(Options &) = delete;
    void operator =(Options &) = delete;

protected:
    virtual void ConfigurationChanged( ::utl::ConfigurationBroadcaster* p, ConfigurationHints nHint ) override;
};

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
