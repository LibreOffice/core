/*************************************************************************
 *
 *  $RCSfile: logger.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-08-31 14:57:36 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2004 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef CONFIGMGR_LOGGER_HXX
#define CONFIGMGR_LOGGER_HXX

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_LOGGING_XLOGGER_HPP_
#include <com/sun/star/util/logging/XLogger.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_LOGGING_LOGLEVEL_HPP_
#include <com/sun/star/util/logging/LogLevel.hpp>
#endif

namespace configmgr
{
// -----------------------------------------------------------------------------
    namespace uno = com::sun::star::uno;
    namespace logging = com::sun::star::util::logging;
    namespace LogLevel = logging::LogLevel;
    using rtl::OUString;

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

        typedef sal_Int32 Level;

        Level level() const
        { return m_xLogger.is() ? m_xLogger->getLevel() : LogLevel::OFF; }

        bool isLogging(Level nLogLevel) const
        { return m_xLogger.is() && m_xLogger->isLoggable(nLogLevel); }

        /// log output to the logger
        void log(Level nLevel, const OUString & msg, const char * sourceMethod = 0, const char * sourceClass = 0) const;
        void log(Level nLevel, const char * msg, const char * sourceMethod = 0, const char * sourceClass = 0) const;

        void error(const OUString & msg, const char * sourceMethod = 0, const char * sourceClass = 0) const
        { log( LogLevel::SEVERE, msg, sourceMethod, sourceClass); }
        void error(const char * msg, const char * sourceMethod = 0, const char * sourceClass = 0) const
        { log( LogLevel::SEVERE, msg, sourceMethod, sourceClass); }

        void warning(const OUString & msg, const char * sourceMethod = 0, const char * sourceClass = 0) const
        { log( LogLevel::WARNING, msg, sourceMethod, sourceClass); }
        void warning(const char * msg, const char * sourceMethod = 0, const char * sourceClass = 0) const
        { log( LogLevel::WARNING, msg, sourceMethod, sourceClass); }

        void info(const OUString & msg, const char * sourceMethod = 0, const char * sourceClass = 0) const
        { log( LogLevel::INFO, msg, sourceMethod, sourceClass); }
        void info(const char * msg, const char * sourceMethod = 0, const char * sourceClass = 0) const
        { log( LogLevel::INFO, msg, sourceMethod, sourceClass); }

        void config(const OUString & msg, const char * sourceMethod = 0, const char * sourceClass = 0) const
        { log( LogLevel::CONFIG, msg, sourceMethod, sourceClass); }
        void config(const char * msg, const char * sourceMethod = 0, const char * sourceClass = 0) const
        { log( LogLevel::CONFIG, msg, sourceMethod, sourceClass); }

        void fine(const OUString & msg, const char * sourceMethod = 0, const char * sourceClass = 0) const
        { log( LogLevel::FINE, msg, sourceMethod, sourceClass); }
        void fine(const char * msg, const char * sourceMethod = 0, const char * sourceClass = 0) const
        { log( LogLevel::FINE, msg, sourceMethod, sourceClass); }

        void finer(const OUString & msg, const char * sourceMethod = 0, const char * sourceClass = 0) const
        { log( LogLevel::FINER, msg, sourceMethod, sourceClass); }
        void finer(const char * msg, const char * sourceMethod = 0, const char * sourceClass = 0) const
        { log( LogLevel::FINER, msg, sourceMethod, sourceClass); }

        void finest(const OUString & msg, const char * sourceMethod = 0, const char * sourceClass = 0) const
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

