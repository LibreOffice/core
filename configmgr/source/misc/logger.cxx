/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: logger.cxx,v $
 * $Revision: 1.5 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"

#include "logger.hxx"

#define CONFIG_LOGGER_SINGLETON "/singletons/com.sun.star.configuration.theLogger"
#define OUSTR( lit ) rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( lit ) )
#define OU2A( ustr ) rtl::OUStringToOString( ustr, RTL_TEXTENCODING_UTF8 ).getStr()
#define A2OU( astr ) rtl::OUString::createFromAscii( astr )

static const sal_Char k_unspecifiedClass[] = "configmgr";
static const sal_Char k_unspecifiedMethod[] = "log-message";

namespace configmgr
{

//--------------------------------------------------------------------------
void Logger::log(sal_Int32 nLevel, const char * msg, const char * sourceMethod, const char * sourceClass) const
{
    OSL_ASSERT(msg);
    if (!msg) msg = "";

    this->log(nLevel,A2OU(msg),sourceMethod,sourceClass);
}

//--------------------------------------------------------------------------
void Logger::log(sal_Int32 nLevel, const rtl::OUString & msg, const char * sourceMethod, const char * sourceClass) const
{
    if (!sourceClass) sourceClass = k_unspecifiedClass;
    if (!sourceMethod) sourceMethod = k_unspecifiedMethod;

    // this place can be used to further enrich or instrument log output
    if (m_xLogger.is())
        try
        {
            m_xLogger->logp(nLevel,A2OU(sourceClass),A2OU(sourceMethod),msg);
        }
        catch (uno::Exception & e)
        {
            OSL_TRACE("Configuration Log failure: %s\n"
                      "Log message was [Level=%04d] %s::%s : %s\n",
                      OU2A(e.Message),int(nLevel),sourceClass,sourceMethod,OU2A(msg));
        }
    else if (nLevel >= (LogLevel::SEVERE - OSL_DEBUG_LEVEL*(LogLevel::SEVERE-LogLevel::WARNING)))
        OSL_TRACE("Configuration Log [%04d] %s::%s : %s\n", int(nLevel),sourceClass,sourceMethod,OU2A(msg));

}

//--------------------------------------------------------------------------
uno::Reference< logging::XLogger >
    Logger::getUnoLoggerFromContext(uno::Reference< uno::XComponentContext > const & xContext)
{
    uno::Reference< logging::XLogger > xLogger;

    if (xContext.is())
        try { xContext->getValueByName( OUSTR(CONFIG_LOGGER_SINGLETON) ) >>= xLogger; }
        catch (uno::Exception & ) {}

    return xLogger;
}

//--------------------------------------------------------------------------

}   // namespace configmgr


