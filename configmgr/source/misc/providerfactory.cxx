/*************************************************************************
 *
 *  $RCSfile: providerfactory.cxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:19:23 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include <stdio.h>

#include "providerfactory.hxx"

#ifndef CONFIGMGR_API_FACTORY_HXX_
#include "confapifactory.hxx"
#endif
#ifndef CONFIGMGR_BOOTSTRAP_HXX_
#include "bootstrap.hxx"
#endif
#ifndef CONFIGMGR_API_PROVIDERWRAPPER_HXX_
#include "providerwrapper.hxx"
#endif

#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif
#ifndef _COM_SUN_STAR_CONFIGURATION_CANNOTLOADCONFIGURATIONEXCEPTION_HPP_
#include <com/sun/star/configuration/CannotLoadConfigurationException.hpp>
#endif

#ifndef _CPPUHELPER_EXC_HLP_HXX_
#include <cppuhelper/exc_hlp.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif

//---------------------------------------------------------------------------------------
namespace configmgr
{
    //---------------------------------------------------------------------------------------
    //= OProviderFactory
    //---------------------------------------------------------------------------------------

    //---------------------------------------------------------------------------------------

    ProviderFactory::ProviderFactory(OUString const & aImplementationName, bool bAdmin)
    : m_aImplementationName(aImplementationName)
    , m_bAdmin(bAdmin)
    {
    }
    //---------------------------------------------------------------------------------------

    ProviderFactory::~ProviderFactory()
    {
    }
    //---------------------------------------------------------------------------------------

    uno::Reference< uno::XInterface > ProviderFactory::getProviderAlways(Context const & xContext)
    {
        RTL_LOGFILE_CONTEXT_AUTHOR(aLog, "configmgr::ProviderFactory", "jb99855", "configmgr::ProviderFactory::getProviderAlways()");
        uno::Reference< uno::XInterface > xResult = getDefaultConfigProviderSingleton(xContext);

        // check for success
        OSL_ENSURE(xResult.is(), "Context could not create provider, but returned NULL instead of throwing an exception");
        if (!xResult.is())
        {
            using ::com::sun::star::configuration::CannotLoadConfigurationException;

            static sal_Char const sCannotCreate[] = "Cannot create ConfigurationProvider. Unknown backend or factory error.";

            throw CannotLoadConfigurationException( OUString(RTL_CONSTASCII_USTRINGPARAM(sCannotCreate)), *this );
        }

        return xResult;
    }
    //---------------------------------------------------------------------------------------
    uno::Reference< uno::XInterface > ProviderFactory::getProviderFromContext(Context const & xContext)
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
            OUString const sContext(RTL_CONSTASCII_USTRINGPARAM(sErrContext));
            e.Message = sContext.concat(e.Message);
            throw;
        }
    }
    //---------------------------------------------------------------------------------------
    uno::Reference< uno::XInterface > ProviderFactory::createProviderWithArguments(Context const & xContext, Arguments const & _aArguments)
    {
        RTL_LOGFILE_CONTEXT_AUTHOR(aLog, "configmgr::ProviderFactory", "jb99855", "configmgr::ProviderFactory::createProviderWithArguments()");

        ContextReader aContext(xContext);
        ArgumentHelper aParser(aContext.getBootstrapContext());

        NamedValues aValues(_aArguments.getLength() + 2);
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
            Context xMergedContext = BootstrapContext::createWrapper(xContext,aValues);
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
    uno::Reference< uno::XInterface > ProviderFactory::createProvider(Context const & xContext, bool bAdmin)
    {
        RTL_LOGFILE_CONTEXT_AUTHOR(aLog, "configmgr::ProviderFactory", "jb99855", "configmgr::ProviderFactory::createProvider(bAdmin)");

        NamedValues aValues(2);
        aValues[0] = ArgumentHelper::makeAdminServiceOverride(bAdmin);
        aValues[1] = BootstrapContext::makePassthroughMarker(sal_False);

        Context xMergedContext = BootstrapContext::createWrapper(xContext,aValues);
        uno::Reference< uno::XInterface > xResult = getProviderFromContext(xMergedContext);

        return xResult;
    }
    //---------------------------------------------------------------------------------------
    uno::Reference< uno::XInterface > ProviderFactory::createProvider(Context const & xContext)
    {
        RTL_LOGFILE_CONTEXT_AUTHOR(aLog, "configmgr::ProviderFactory", "jb99855", "configmgr::ProviderFactory::createProvider()");

        if (BootstrapContext::isPassthrough(xContext))
        {
            // make sure this uses a new BootstrapContext !
            Context xPatchedContext = BootstrapContext::createWrapper(xContext,NamedValues());
            return getProviderFromContext(xPatchedContext);
        }
        else
            return getProviderFromContext(xContext);
    }
    //---------------------------------------------------------------------------------------

    sal_Int32 ProviderFactory::parseArguments(ArgumentHelper & aParser, NamedValues & rValues, Arguments const & _aArguments)
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
            OUString const & aImplementationName,
            bool bAdmin
        )
    {
        return new ProviderFactory(aImplementationName, bAdmin);
    }
    //---------------------------------------------------------------------------------------
}   // namespace configmgr
//........................................................................

