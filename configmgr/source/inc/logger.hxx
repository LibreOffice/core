/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: logger.hxx,v $
 * $Revision: 1.4 $
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

#ifndef CONFIGMGR_LOGGER_HXX
#define CONFIGMGR_LOGGER_HXX

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/logging/XLogger.hpp>
#include <com/sun/star/util/logging/LogLevel.hpp>

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace uno = com::sun::star::uno;
    namespace logging = com::sun::star::util::logging;
    namespace LogLevel = logging::LogLevel;

    /// class providing access to a log output sink in the context
    class Logger
    {
        uno::Reference< logging::XLogger > m_xLogger;
    public:
        Logger() : m_xLogger() {}

        Logger(uno::Reference< logging::XLogger > const &  xLogger)
        : m_xLogger(xLogger)
        {}

        explicit
        Logger(uno::Reference< uno::XComponentContext > const &  xContext)
        : m_xLogger( getUnoLoggerFromContext(xContext) )
        {}

        sal_Int32 level() const
        { return m_xLogger.is() ? m_xLogger->getLevel() : LogLevel::OFF; }

        bool isLogging(sal_Int32 nLogLevel) const
        { return m_xLogger.is() && m_xLogger->isLoggable(nLogLevel); }

        /// log output to the logger
        void log(sal_Int32 nLevel, const rtl::OUString & msg, const char * sourceMethod = 0, const char * sourceClass = 0) const;
        void log(sal_Int32 nLevel, const char * msg, const char * sourceMethod = 0, const char * sourceClass = 0) const;

        void error(const rtl::OUString & msg, const char * sourceMethod = 0, const char * sourceClass = 0) const
        { log( LogLevel::SEVERE, msg, sourceMethod, sourceClass); }
        void error(const char * msg, const char * sourceMethod = 0, const char * sourceClass = 0) const
        { log( LogLevel::SEVERE, msg, sourceMethod, sourceClass); }

        void warning(const rtl::OUString & msg, const char * sourceMethod = 0, const char * sourceClass = 0) const
        { log( LogLevel::WARNING, msg, sourceMethod, sourceClass); }
        void warning(const char * msg, const char * sourceMethod = 0, const char * sourceClass = 0) const
        { log( LogLevel::WARNING, msg, sourceMethod, sourceClass); }

        void info(const rtl::OUString & msg, const char * sourceMethod = 0, const char * sourceClass = 0) const
        { log( LogLevel::INFO, msg, sourceMethod, sourceClass); }
        void info(const char * msg, const char * sourceMethod = 0, const char * sourceClass = 0) const
        { log( LogLevel::INFO, msg, sourceMethod, sourceClass); }

        void config(const rtl::OUString & msg, const char * sourceMethod = 0, const char * sourceClass = 0) const
        { log( LogLevel::CONFIG, msg, sourceMethod, sourceClass); }
        void config(const char * msg, const char * sourceMethod = 0, const char * sourceClass = 0) const
        { log( LogLevel::CONFIG, msg, sourceMethod, sourceClass); }

        void fine(const rtl::OUString & msg, const char * sourceMethod = 0, const char * sourceClass = 0) const
        { log( LogLevel::FINE, msg, sourceMethod, sourceClass); }
        void fine(const char * msg, const char * sourceMethod = 0, const char * sourceClass = 0) const
        { log( LogLevel::FINE, msg, sourceMethod, sourceClass); }

        void finer(const rtl::OUString & msg, const char * sourceMethod = 0, const char * sourceClass = 0) const
        { log( LogLevel::FINER, msg, sourceMethod, sourceClass); }
        void finer(const char * msg, const char * sourceMethod = 0, const char * sourceClass = 0) const
        { log( LogLevel::FINER, msg, sourceMethod, sourceClass); }

        void finest(const rtl::OUString & msg, const char * sourceMethod = 0, const char * sourceClass = 0) const
        { log( LogLevel::FINEST, msg, sourceMethod, sourceClass); }
        void finest(const char * msg, const char * sourceMethod = 0, const char * sourceClass = 0) const
        { log( LogLevel::FINEST, msg, sourceMethod, sourceClass); }

        uno::Reference< logging::XLogger > getUnoLogger() const { return m_xLogger; }

        static
        uno::Reference< logging::XLogger >
            getUnoLoggerFromContext(uno::Reference< uno::XComponentContext > const & xContext);
    };
// -----------------------------------------------------------------------------
} // namespace configmgr

#endif // CONFIGMGR_LOGGER_HXX

