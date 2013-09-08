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

#ifndef __FRAMEWORK_CONFIG_CONFIGACCESS_HXX_
#define __FRAMEWORK_CONFIG_CONFIGACCESS_HXX_

#include <threadhelp/threadhelpbase.hxx>
#include <general.h>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <rtl/ustring.hxx>
#include <fwidllapi.h>

namespace framework{


//_______________________________________
/**
    @short  implements a simple configuration access
    @descr  Sometimes it's better to have direct config access
            instead of using soecialize config items of the svtools
            project. This class can wrapp such configuration access.
 */
class FWI_DLLPUBLIC ConfigAccess : public ThreadHelpBase
{


    public:

        /** represent the possible modes of the internal wrapped configuration access */
        enum EOpenMode
        {
            /// config isn't used yet
            E_CLOSED,
            /// config access is open for reading only
            E_READONLY,
            /// config access is open for reading/writing data
            E_READWRITE
        };

    //___________________________________
    // member

    private:

        /**
            reference to the uno service manager
            It's neccessary to instanciate own needed services.
         */
        css::uno::Reference< css::uno::XComponentContext > m_xContext;

        /** hold an opened configuration alive */
        css::uno::Reference< css::uno::XInterface > m_xConfig;

        /** knows the root of the opened config access point */
        OUString m_sRoot;

        /** represent the current open mode */
        EOpenMode m_eMode;

    //___________________________________
    // native interface methods

    public:

                 ConfigAccess( const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                               const OUString&                                    sRoot );
        virtual ~ConfigAccess();

        virtual void      open   ( EOpenMode eMode );
        virtual void      close  (                 );
        virtual EOpenMode getMode(                 ) const;

        virtual const css::uno::Reference< css::uno::XInterface >& cfg();
};

} // namespace framework

#endif // __FRAMEWORK_CONFIG_CONFIGACCESS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
