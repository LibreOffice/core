/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: collatorwrapper.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 01:24:39 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_unotools.hxx"

#ifndef _UNOTOOLS_COLLATORWRAPPER_HXX
#include <unotools/collatorwrapper.hxx>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _COMPHELPER_COMPONENTFACTORY_HXX_
#include <comphelper/componentfactory.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_XINTERFACE_HPP_
#include <com/sun/star/uno/XInterface.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

using namespace ::com::sun::star;

CollatorWrapper::CollatorWrapper (
        const uno::Reference< lang::XMultiServiceFactory > &xServiceFactory)
    : mxServiceFactory (xServiceFactory)
{
    ::rtl::OUString aService (RTL_CONSTASCII_USTRINGPARAM("com.sun.star.i18n.Collator"));

    if (mxServiceFactory.is())
    {
        try
        {
            mxInternationalCollator = uno::Reference< i18n::XCollator > (
                mxServiceFactory->createInstance(aService), uno::UNO_QUERY);
        }
        catch (uno::Exception& rException)
        {
            (void)rException;
            DBG_ERRORFILE ("CollatorWrapper: failed to create instance");
        }
    }
    else
    {
        ::rtl::OUString aLibrary (RTL_CONSTASCII_USTRINGPARAM(LLCF_LIBNAME("i18n")));

        try
        {
            uno::Reference< uno::XInterface > xInstance =
                ::comphelper::getComponentInstance (aLibrary, aService);

            if (xInstance.is())
            {
                uno::Any xInterface = xInstance->queryInterface (
                    ::getCppuType((const uno::Reference< i18n::XCollator >*)0) );
                xInterface >>= mxInternationalCollator;
            }
        }
        catch (uno::Exception& rException)
        {
            (void)rException;
            DBG_ERRORFILE ("CollatorWrapper: failed to get component instance!");
        }
    }

    DBG_ASSERT (mxInternationalCollator.is(), "CollatorWrapper: no i18n collator");
}

CollatorWrapper::~CollatorWrapper()
{
}

sal_Int32
CollatorWrapper::compareSubstring (
        const ::rtl::OUString& s1, sal_Int32 off1, sal_Int32 len1,
        const ::rtl::OUString& s2, sal_Int32 off2, sal_Int32 len2) const
{
    try
    {
        if (mxInternationalCollator.is())
            return mxInternationalCollator->compareSubstring (
                                                        s1, off1, len1, s2, off2, len2);
    }
    catch (uno::RuntimeException& rRuntimeException)
    {
        (void)rRuntimeException;
        DBG_ERRORFILE ("CollatorWrapper: compareSubstring failed");
    }

    return 0;
}

sal_Int32
CollatorWrapper::compareString (const ::rtl::OUString& s1, const ::rtl::OUString& s2) const
{
    try
    {
        if (mxInternationalCollator.is())
            return mxInternationalCollator->compareString (s1, s2);
    }
    catch (uno::RuntimeException& rRuntimeException)
    {
        (void)rRuntimeException;
        DBG_ERRORFILE ("CollatorWrapper: compareString failed");
    }

    return 0;
}

uno::Sequence< ::rtl::OUString >
CollatorWrapper::listCollatorAlgorithms (const lang::Locale& rLocale) const
{
    try
    {
        if (mxInternationalCollator.is())
            return mxInternationalCollator->listCollatorAlgorithms (rLocale);
    }
    catch (uno::RuntimeException& rRuntimeException)
    {
        (void)rRuntimeException;
        DBG_ERRORFILE ("CollatorWrapper: listCollatorAlgorithms failed");
    }

    return uno::Sequence< ::rtl::OUString > ();
}

uno::Sequence< sal_Int32 >
CollatorWrapper::listCollatorOptions (const ::rtl::OUString& rAlgorithm) const
{
    try
    {
        if (mxInternationalCollator.is())
            return mxInternationalCollator->listCollatorOptions (rAlgorithm);
    }
    catch (uno::RuntimeException& rRuntimeException)
    {
        (void)rRuntimeException;
        DBG_ERRORFILE ("CollatorWrapper: listCollatorOptions failed");
    }

    return uno::Sequence< sal_Int32 > ();
}

sal_Int32
CollatorWrapper::loadDefaultCollator (const lang::Locale& rLocale, sal_Int32 nOptions)
{
    try
    {
        if (mxInternationalCollator.is())
            return mxInternationalCollator->loadDefaultCollator (rLocale, nOptions);
    }
    catch (uno::RuntimeException& rRuntimeException)
    {
        (void)rRuntimeException;
        DBG_ERRORFILE ("CollatorWrapper: loadDefaultCollator failed");
    }

    return 0;
}

sal_Int32
CollatorWrapper::loadCollatorAlgorithm (const ::rtl::OUString& rAlgorithm,
        const lang::Locale& rLocale, sal_Int32 nOptions)
{
    try
    {
        if (mxInternationalCollator.is())
            return mxInternationalCollator->loadCollatorAlgorithm (
                                                        rAlgorithm, rLocale, nOptions);
    }
    catch (uno::RuntimeException& rRuntimeException)
    {
        (void)rRuntimeException;
        DBG_ERRORFILE ("CollatorWrapper: loadCollatorAlgorithm failed");
    }

    return 0;

}

void
CollatorWrapper::loadCollatorAlgorithmWithEndUserOption (
        const ::rtl::OUString& rAlgorithm,
        const lang::Locale& rLocale, const uno::Sequence< sal_Int32 >& rOption)
{
    try
    {
        if (mxInternationalCollator.is())
            mxInternationalCollator->loadCollatorAlgorithmWithEndUserOption (
                                                        rAlgorithm, rLocale, rOption);
    }
    catch (uno::RuntimeException& rRuntimeException)
    {
        (void)rRuntimeException;
        DBG_ERRORFILE ("CollatorWrapper: loadCollatorAlgorithmWithEndUserOption failed");
    }
}
