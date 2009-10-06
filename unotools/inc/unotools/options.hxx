/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: options.hxx,v $
 * $Revision: 1.3 $
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

namespace utl {

    class ConfigurationBroadcaster;
    class IMPL_ConfigurationListenerList;

    class UNOTOOLS_DLLPUBLIC ConfigurationListener
    {
    public:
        virtual void ConfigurationChanged( ConfigurationBroadcaster* ) = 0;
    };

    class UNOTOOLS_DLLPUBLIC ConfigurationBroadcaster
    {
        IMPL_ConfigurationListenerList* mpList;
    public:
        void AddListener( utl::ConfigurationListener* pListener );
        void RemoveListener( utl::ConfigurationListener* pListener );
        void NotifyListeners();
        ConfigurationBroadcaster();
        ~ConfigurationBroadcaster();
    };

namespace detail {

// A base class for the various option classes supported by
// unotools/source/config/itemholderbase.hxx (which must be public, as it is
// shared between svl and svt):
class UNOTOOLS_DLLPUBLIC Options : public utl::ConfigurationBroadcaster, public utl::ConfigurationListener
{
public:
    Options();

    virtual ~Options() = 0;

private:
    UNOTOOLS_DLLPRIVATE Options(Options &); // not defined
    UNOTOOLS_DLLPRIVATE void operator =(Options &); // not defined
    virtual void ConfigurationChanged( utl::ConfigurationBroadcaster* );
};

} }

#endif
