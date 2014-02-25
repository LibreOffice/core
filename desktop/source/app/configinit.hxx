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

#ifndef INCLUDED_DESKTOP_SOURCE_APP_CONFIGINIT_HXX
#define INCLUDED_DESKTOP_SOURCE_APP_CONFIGINIT_HXX

#include "sal/config.h"

#include <com/sun/star/task/XInteractionHandler.hpp>

/**
    sets an InteractionHandler for configuration errors in the current context.

  */
    class ConfigurationErrorHandler
    {
    public:
        typedef com::sun::star::uno::Reference< com::sun::star::task::XInteractionHandler > InteractionHandler;

        /// Constructor: Uses the default interaction handler
        ConfigurationErrorHandler()
        : m_pContext(0), m_xHandler()
        {}

        /// Constructor: Uses an externally provided interaction handler
        ConfigurationErrorHandler(const InteractionHandler & xHandler)
        : m_pContext(0), m_xHandler( xHandler )
        {}

        ~ConfigurationErrorHandler();

        static InteractionHandler getDefaultInteractionHandler();

        /// installs the handler into the current context
        void activate();
        /// deinstalls the handler from the current context, restoring the previous context
        void deactivate();
    private:
        class Context;
        Context * m_pContext;
        InteractionHandler m_xHandler;
    private:
        // not implemented - suppress copy
        ConfigurationErrorHandler(const ConfigurationErrorHandler&);
        void operator=(const ConfigurationErrorHandler&);
    };


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
