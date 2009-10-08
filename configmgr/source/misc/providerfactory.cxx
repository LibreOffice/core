/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: providerfactory.cxx,v $
 * $Revision: 1.24 $
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
#include <stdio.h>

#include "providerfactory.hxx"

#ifndef CONFIGMGR_API_FACTORY_HXX_
#include "confapifactory.hxx"
#endif
#include "bootstrap.hxx"
#include "providerwrapper.hxx"
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/configuration/CannotLoadConfigurationException.hpp>
#include <cppuhelper/exc_hlp.hxx>
#include <rtl/ustrbuf.hxx>
#include <osl/diagnose.h>
#include <rtl/logfile.hxx>

//---------------------------------------------------------------------------------------
namespace configmgr
{
    //---------------------------------------------------------------------------------------
    //= OProviderFactory
    //---------------------------------------------------------------------------------------

    //---------------------------------------------------------------------------------------

    ProviderFactory::ProviderFactory(rtl::OUString const & aImplementationName, bool bAdmin)
    : m_aImplementationName(aImplementationName)
    , m_bAdmin(bAdmin)
    {
    }
    //---------------------------------------------------------------------------------------

    ProviderFactory::~ProviderFactory()
    {
    }
    //---------------------------------------------------------------------------------------

    uno::Reference< uno::XInterface > ProviderFactory::getProviderAlways(uno::Reference< uno::XComponentContext > const & xContext)
    {
        RTL_LOGFILE_CONTEXT_AUTHOR(aLog, "configmgr::ProviderFactory", "jb99855", "configmgr::ProviderFactory::getProviderAlways()");
        uno::Reference< uno::XInterface > xResult = getDefaultConfigProviderSingleton(xContext);

        // check for success
        OSL_ENSURE(xResult.is(), "Context could not create provider, but returned NULL instead of throwing an exception");
        if (!xResult.is())
        {
            static sal_Char const sCannotCreate[] = "Cannot create ConfigurationProvider. Unknown backend or factory error.";

            throw com::sun::star::configuration::CannotLoadConfigurationException( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(sCannotCreate)), *this );
        }

        return xResult;
    }
    //---------------------------------------------------------------------------------------
    uno::Reference< uno::XInterface > ProviderFactory::getProviderFromContext(uno::Reference< uno::XComponentContext > const & xContext)
    {
        OSL_ENSURE(ContextReader::testAdminService(xContext, this->m_bAdmin),
                    "Creation context admin flag does not match service being created");

        try
        {
            return getProviderAlways(xContext);
        }
        catch(uno::Exception& e)
        {
            ContextReader aContext(xContext);

            uno::Any aError = aContext.getBootstrapError();
            if (aError.hasValue())
            {
                OSL_ASSERT(aError.getValueTypeClass() == uno::TypeClass_EXCEPTION);
                cppu::throwException(aError);
            }

            OSL_ASSERT(aContext.isBootstrapValid());

            static const sal_Char sErrContext[] = "Cannot open Configuration: ";
            rtl::OUString const sContext(RTL_CONSTASCII_USTRINGPARAM(sErrContext));
            e.Message = sContext.concat(e.Message);
            throw;
        }
    }
    //---------------------------------------------------------------------------------------
    uno::Reference< uno::XInterface > ProviderFactory::createProviderWithArguments(uno::Reference< uno::XComponentContext > const & xContext, uno::Sequence < uno::Any > const & _aArguments)
    {
        RTL_LOGFILE_CONTEXT_AUTHOR(aLog, "configmgr::ProviderFactory", "jb99855", "configmgr::ProviderFactory::createProviderWithArguments()");

        ContextReader aContext(xContext);
        ArgumentHelper aParser(aContext.getBootstrapContext());

        uno::Sequence < beans::NamedValue > aValues(_aArguments.getLength() + 2);
        sal_Int32 nCount = parseArguments(aParser,aValues,_aArguments);

        bool bNeedNewBackend = aParser.hasBackendArguments();

        if (!aContext.testAdminService(aContext.getBaseContext(),m_bAdmin))
        {
            bNeedNewBackend = true;
            OSL_ASSERT( nCount+2  <= aValues.getLength());
            aValues[nCount++] = ArgumentHelper::makeAdminServiceOverride(m_bAdmin);
            aValues[nCount++] = BootstrapContext::makePassthroughMarker(sal_False);
        }

        OSL_ASSERT(nCount <= aValues.getLength());
        aValues.realloc(nCount);

        if (bNeedNewBackend)
        {
            uno::Reference< uno::XComponentContext > xMergedContext = BootstrapContext::createWrapper(xContext,aValues);
            uno::Reference< uno::XInterface > xResult = getProviderFromContext(xMergedContext);

            return xResult;
        }
        else
        {
            uno::Reference< uno::XInterface > xBaseProvider = getProviderFromContext(xContext);
            uno::Reference< uno::XInterface > xResult = ProviderWrapper::create(xBaseProvider,aValues);

            return xResult;
        }
    }
    //---------------------------------------------------------------------------------------
    uno::Reference< uno::XInterface > ProviderFactory::createProvider(uno::Reference< uno::XComponentContext > const & xContext, bool bAdmin)
    {
        RTL_LOGFILE_CONTEXT_AUTHOR(aLog, "configmgr::ProviderFactory", "jb99855", "configmgr::ProviderFactory::createProvider(bAdmin)");

        uno::Sequence < beans::NamedValue > aValues(2);
        aValues[0] = ArgumentHelper::makeAdminServiceOverride(bAdmin);
        aValues[1] = BootstrapContext::makePassthroughMarker(sal_False);

        uno::Reference< uno::XComponentContext > xMergedContext = BootstrapContext::createWrapper(xContext,aValues);
        uno::Reference< uno::XInterface > xResult = getProviderFromContext(xMergedContext);

        return xResult;
    }
    //---------------------------------------------------------------------------------------
    uno::Reference< uno::XInterface > ProviderFactory::createProvider(uno::Reference< uno::XComponentContext > const & xContext)
    {
        RTL_LOGFILE_CONTEXT_AUTHOR(aLog, "configmgr::ProviderFactory", "jb99855", "configmgr::ProviderFactory::createProvider()");

        if (BootstrapContext::isPassthrough(xContext))
        {
            // make sure this uses a new BootstrapContext !
            uno::Reference< uno::XComponentContext > xPatchedContext = BootstrapContext::createWrapper(xContext,uno::Sequence < beans::NamedValue >());
            return getProviderFromContext(xPatchedContext);
        }
        else
            return getProviderFromContext(xContext);
    }
    //---------------------------------------------------------------------------------------

    sal_Int32 ProviderFactory::parseArguments(ArgumentHelper & aParser, uno::Sequence < beans::NamedValue > & rValues, uno::Sequence < uno::Any > const & _aArguments)
    {
        OSL_ASSERT(rValues.getLength() >= _aArguments.getLength());

        sal_Int32 nCount = 0;
        for (sal_Int32 i = 0; i < _aArguments.getLength(); ++i)
        {
            if (!aParser.extractArgument(rValues[nCount],_aArguments[i]))
            {
                rtl::OUStringBuffer sMsg;
                sMsg.appendAscii("ProviderFactory: Unexpected Argument Type. ");
                sMsg.appendAscii("Expected NamedValue or PropertyValue,  ");
                sMsg.appendAscii("found ").append(_aArguments[i].getValueTypeName()).appendAscii(". ");
                throw lang::IllegalArgumentException(sMsg.makeStringAndClear(),*this,static_cast<sal_Int16>(i));
            }

            if (aParser.filterAndAdjustArgument(rValues[nCount]))
            {
                aParser.checkBackendArgument(rValues[nCount]);
                ++nCount;
            }
        }
        return nCount;
    }
    //---------------------------------------------------------------------------------------

    uno::Reference< uno::XInterface >
        SAL_CALL ProviderFactory::createInstanceWithContext( const uno::Reference< uno::XComponentContext >& xContext )
            throw (uno::Exception, ::com::sun::star::uno::RuntimeException)
    {
        // default provider ?
        if (ContextReader::testAdminService(xContext,m_bAdmin))
            return createProvider( xContext );

        else
            return createProvider(xContext,m_bAdmin);
    }
    //---------------------------------------------------------------------------------------

    uno::Reference< uno::XInterface > SAL_CALL
        ProviderFactory::createInstanceWithArgumentsAndContext( const uno::Sequence< uno::Any >& aArguments, const uno::Reference< uno::XComponentContext >& xContext )
            throw (uno::Exception, uno::RuntimeException)
    {
        // default request
        return createProviderWithArguments(xContext, aArguments);
    }

    //---------------------------------------------------------------------------------------
    //---------------------------------------------------------------------------------------

    uno::Reference< lang::XSingleComponentFactory > SAL_CALL createProviderFactory(
            rtl::OUString const & aImplementationName,
            bool bAdmin
        )
    {
        return new ProviderFactory(aImplementationName, bAdmin);
    }
    //---------------------------------------------------------------------------------------
}   // namespace configmgr
//........................................................................

