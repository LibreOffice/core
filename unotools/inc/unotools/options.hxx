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

#ifndef INCLUDED_unotools_OPTIONS_HXX
#define INCLUDED_unotools_OPTIONS_HXX

#include "sal/config.h"
#include "unotools/unotoolsdllapi.h"
#include <vector>

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
        virtual void ConfigurationChanged( ConfigurationBroadcaster* p, sal_uInt32 nHint=0 ) = 0;
    };
    typedef ::std::vector< ConfigurationListener* > IMPL_ConfigurationListenerList;

    // complete broadcasting implementation
    class UNOTOOLS_DLLPUBLIC ConfigurationBroadcaster
    {
        IMPL_ConfigurationListenerList* mpList;
        sal_Int32               m_nBroadcastBlocked;     // broadcast only if this is 0
        sal_uInt32              m_nBlockedHint;

    public:
        void AddListener( utl::ConfigurationListener* pListener );
        void RemoveListener( utl::ConfigurationListener* pListener );

        // notify listeners; nHint is an implementation detail of the particular class deriving from ConfigurationBroadcaster
        void NotifyListeners( sal_uInt32 nHint );
        ConfigurationBroadcaster();
        virtual ~ConfigurationBroadcaster();
        virtual void BlockBroadcasts( bool bBlock );
    };

namespace detail {

// A base class for the various option classes supported by
// unotools/source/config/itemholderbase.hxx (which must be public, as it is
// shared between unotools, svl and svt)
// It also provides an implementation for a Configuration Listener and inherits a broadcaster implementation

class UNOTOOLS_DLLPUBLIC Options : public utl::ConfigurationBroadcaster, public utl::ConfigurationListener
{
public:
    Options();

    virtual ~Options() = 0;

private:
    UNOTOOLS_DLLPRIVATE Options(Options &); // not defined
    UNOTOOLS_DLLPRIVATE void operator =(Options &); // not defined

protected:
    virtual void ConfigurationChanged( ::utl::ConfigurationBroadcaster* p, sal_uInt32 nHint=0 );
};

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
