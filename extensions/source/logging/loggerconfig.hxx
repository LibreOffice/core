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

#ifndef INCLUDED_EXTENSIONS_SOURCE_LOGGING_LOGGERCONFIG_HXX
#define INCLUDED_EXTENSIONS_SOURCE_LOGGING_LOGGERCONFIG_HXX

#include <com/sun/star/logging/XLogger.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>


namespace logging
{


    /** initializes the given logger from the configuration

        The configuration node /org.openoffice.Office.Logging/Settings/<logger_name>
        is examined for this. If it does not yet exist, it will be created.

        The function creates a default handler and a default formatter, as specified in the
        configuration.

        This function is currently external to the logger instance. Perhaps it can, on the long
        run, be moved to the logger implementation - not sure if it's the best place.
    */
    void    initializeLoggerFromConfiguration(
                const css::uno::Reference<css::uno::XComponentContext>& _rContext,
                const css::uno::Reference< css::logging::XLogger >& _rxLogger
            );


} // namespace logging


#endif // INCLUDED_EXTENSIONS_SOURCE_LOGGING_LOGGERCONFIG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
