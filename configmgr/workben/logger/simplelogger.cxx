/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: simplelogger.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 04:50:55 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include "simplelogger.hxx"

#ifndef _COM_SUN_STAR_UTIL_LOGGING_LOGLEVEL_HPP_
#include <com/sun/star/util/logging/LogLevel.hpp>
#endif

#include <osl/thread.h>
#include <stdlib.h> // for getenv

namespace logger
{

    namespace LogLevel = com::sun::star::util::logging::LogLevel;

//==============================================================================
static rtl::OString level2str(sal_Int32 nLevel)
{
   return rtl::OString::valueOf(nLevel);
}

sal_Int32 str2level(rtl::OString const & str)
{
    return str.toInt32();
}

//------------------------------------------------------------------------------
SimpleLogger::SimpleLogger( const uno::Reference<uno::XComponentContext>& xContext, char const * name)
: mContext(xContext)
, mName( OUString::createFromAscii(name) )
, mOutput(stderr)
, mLevel(LogLevel::INFO)
{
    if (char const * fname = getenv("CFG_LOGFILE"))
    {
        mOutput = fopen(fname,"a+");
        OSL_ENSURE(mOutput,"ERROR: could not open logfile\n");
    }

    if (char const * level = getenv("CFG_LOGLEVEL"))
    {
        mLevel = str2level(level);
    }
}
//------------------------------------------------------------------------------
SimpleLogger::~SimpleLogger()
{
}
//------------------------------------------------------------------------------
uno::Reference< logging::XLogger > SAL_CALL
    SimpleLogger::getLogger( const OUString& name )
        throw (uno::RuntimeException)
{
    if (name == mName) return this;

    // try whatever
    uno::Reference< logging::XLogger > xNamedLogger;
    if (mContext.is())
    {
        OUString const singleton(RTL_CONSTASCII_USTRINGPARAM("/singletons/"));
        mContext->getValueByName(singleton.concat(name)) >>= xNamedLogger;
    }
    return xNamedLogger;
}

//------------------------------------------------------------------------------
sal_Int32 SAL_CALL SimpleLogger::getLevel(  ) throw (uno::RuntimeException)
{
    return mLevel;
}

//------------------------------------------------------------------------------
OUString SAL_CALL SimpleLogger::getName(  ) throw (uno::RuntimeException)
{
    return mName;
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL SimpleLogger::isLoggable( sal_Int32 level ) throw (uno::RuntimeException)
{
    return mOutput && level >= mLevel;
}

//------------------------------------------------------------------------------
#define OU2OUT( ustr ) ( rtl::OUStringToOString( ustr, enc ).getStr() )

void SAL_CALL SimpleLogger::logp( sal_Int32 level, const OUString& sourceClass, const OUString& sourceMethod, const OUString& msg )
        throw (uno::RuntimeException)
{
    rtl_TextEncoding enc = osl_getThreadTextEncoding();
    if (mOutput && level > mLevel)
    {
        fprintf( mOutput, "%s {%s.%s}: [%s] %s\n", OU2OUT(mName),
                            OU2OUT(sourceClass) , OU2OUT(sourceMethod),
                            level2str(level).getStr(), OU2OUT(msg) );
    }
}

//------------------------------------------------------------------------------

OUString SAL_CALL SimpleLogger::getImplementationName_static()
{
    static const char kImplementationName[] = "com.sun.star.comp.configmgr.logging.SimpleLogger";

    return OUString(RTL_CONSTASCII_USTRINGPARAM(kImplementationName)) ;
}
//------------------------------------------------------------------------------

rtl::OUString SAL_CALL SimpleLogger::getImplementationName()
    throw (uno::RuntimeException)
{
    return getImplementationName_static() ;
}
//------------------------------------------------------------------------------

uno::Sequence<rtl::OUString> SAL_CALL SimpleLogger::getSupportedServiceNames_static()
{
    uno::Sequence<rtl::OUString> aServices(2) ;
    aServices[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.logging.SimpleLogger")) ;
    aServices[1] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.util.logging.Logger")) ;

    return aServices ;
}
//------------------------------------------------------------------------------

sal_Bool SAL_CALL SimpleLogger::supportsService(
    const rtl::OUString& aServiceName)
    throw (uno::RuntimeException)
{
    uno::Sequence< rtl::OUString > const svc = getSupportedServiceNames_static();

    for(sal_Int32 i = 0; i < svc.getLength(); ++i )
        if(svc[i] == aServiceName)
            return true;
    return false;
}
//------------------------------------------------------------------------------

uno::Sequence<rtl::OUString>
SAL_CALL SimpleLogger::getSupportedServiceNames()
    throw (uno::RuntimeException)
{
    return getSupportedServiceNames_static() ;
}
//------------------------------------------------------------------------------


} // namespace logger
