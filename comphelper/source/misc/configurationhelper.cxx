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

#include <comphelper/configurationhelper.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>


namespace comphelper{

namespace css = ::com::sun::star;


//-----------------------------------------------
css::uno::Reference< css::uno::XInterface > ConfigurationHelper::openConfig(const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                                                                            const ::rtl::OUString&                                    sPackage,
                                                                                  sal_Int32                                           eMode   )
{
    css::uno::Reference< css::lang::XMultiServiceFactory > xConfigProvider(
        css::configuration::theDefaultProvider::get( rxContext ) );

    ::comphelper::SequenceAsVector< css::uno::Any > lParams;
    css::beans::PropertyValue                       aParam ;

    // set root path
    aParam.Name    = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("nodepath"));
    aParam.Value <<= sPackage;
    lParams.push_back(css::uno::makeAny(aParam));

    // enable all locales mode
    if ((eMode & ConfigurationHelper::E_ALL_LOCALES)==ConfigurationHelper::E_ALL_LOCALES)
    {
        aParam.Name    = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("locale"));
        aParam.Value <<= ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("*"));
        lParams.push_back(css::uno::makeAny(aParam));
    }

    // enable lazy writing
    sal_Bool bLazy = ((eMode & ConfigurationHelper::E_LAZY_WRITE)==ConfigurationHelper::E_LAZY_WRITE);
    aParam.Name    = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("lazywrite"));
    aParam.Value   = css::uno::makeAny(bLazy);
    lParams.push_back(css::uno::makeAny(aParam));

    // open it
    css::uno::Reference< css::uno::XInterface > xCFG;

    sal_Bool bReadOnly = ((eMode & ConfigurationHelper::E_READONLY)==ConfigurationHelper::E_READONLY);
    if (bReadOnly)
        xCFG = xConfigProvider->createInstanceWithArguments(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.ConfigurationAccess")),
                lParams.getAsConstList());
    else
        xCFG = xConfigProvider->createInstanceWithArguments(
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.configuration.ConfigurationUpdateAccess")),
                lParams.getAsConstList());

    return xCFG;
}

//-----------------------------------------------
css::uno::Any ConfigurationHelper::readRelativeKey(const css::uno::Reference< css::uno::XInterface > xCFG    ,
                                                   const ::rtl::OUString&                            sRelPath,
                                                   const ::rtl::OUString&                            sKey    )
{
    css::uno::Reference< css::container::XHierarchicalNameAccess > xAccess(xCFG, css::uno::UNO_QUERY_THROW);

    css::uno::Reference< css::beans::XPropertySet > xProps;
    xAccess->getByHierarchicalName(sRelPath) >>= xProps;
    if (!xProps.is())
    {
        ::rtl::OUStringBuffer sMsg(256);
        sMsg.appendAscii("The requested path \"");
        sMsg.append     (sRelPath               );
        sMsg.appendAscii("\" does not exists."  );

        throw css::container::NoSuchElementException(
                    sMsg.makeStringAndClear(),
                    css::uno::Reference< css::uno::XInterface >());
    }
    return xProps->getPropertyValue(sKey);
}

//-----------------------------------------------
void ConfigurationHelper::writeRelativeKey(const css::uno::Reference< css::uno::XInterface > xCFG    ,
                                           const ::rtl::OUString&                            sRelPath,
                                           const ::rtl::OUString&                            sKey    ,
                                           const css::uno::Any&                              aValue  )
{
    css::uno::Reference< css::container::XHierarchicalNameAccess > xAccess(xCFG, css::uno::UNO_QUERY_THROW);

    css::uno::Reference< css::beans::XPropertySet > xProps;
    xAccess->getByHierarchicalName(sRelPath) >>= xProps;
    if (!xProps.is())
    {
        ::rtl::OUStringBuffer sMsg(256);
        sMsg.appendAscii("The requested path \"");
        sMsg.append     (sRelPath               );
        sMsg.appendAscii("\" does not exists."  );

        throw css::container::NoSuchElementException(
                    sMsg.makeStringAndClear(),
                    css::uno::Reference< css::uno::XInterface >());
    }
    xProps->setPropertyValue(sKey, aValue);
}

//-----------------------------------------------
css::uno::Reference< css::uno::XInterface > ConfigurationHelper::makeSureSetNodeExists(const css::uno::Reference< css::uno::XInterface > xCFG         ,
                                                                                       const ::rtl::OUString&                            sRelPathToSet,
                                                                                       const ::rtl::OUString&                            sSetNode     )
{
    css::uno::Reference< css::container::XHierarchicalNameAccess > xAccess(xCFG, css::uno::UNO_QUERY_THROW);
    css::uno::Reference< css::container::XNameAccess > xSet;
    xAccess->getByHierarchicalName(sRelPathToSet) >>= xSet;
    if (!xSet.is())
    {
        ::rtl::OUStringBuffer sMsg(256);
        sMsg.appendAscii("The requested path \"");
        sMsg.append     (sRelPathToSet          );
        sMsg.appendAscii("\" does not exists."  );

        throw css::container::NoSuchElementException(
                    sMsg.makeStringAndClear(),
                    css::uno::Reference< css::uno::XInterface >());
    }

    css::uno::Reference< css::uno::XInterface > xNode;
    if (xSet->hasByName(sSetNode))
        xSet->getByName(sSetNode) >>= xNode;
    else
    {
        css::uno::Reference< css::lang::XSingleServiceFactory > xNodeFactory(xSet, css::uno::UNO_QUERY_THROW);
        xNode = xNodeFactory->createInstance();
        css::uno::Reference< css::container::XNameContainer > xSetReplace(xSet, css::uno::UNO_QUERY_THROW);
        xSetReplace->insertByName(sSetNode, css::uno::makeAny(xNode));
    }

    return xNode;
}

//-----------------------------------------------
css::uno::Any ConfigurationHelper::readDirectKey(const css::uno::Reference< css::uno::XComponentContext >&    rxContext,
                                                 const ::rtl::OUString&                                       sPackage,
                                                 const ::rtl::OUString&                                       sRelPath,
                                                 const ::rtl::OUString&                                       sKey    ,
                                                       sal_Int32                                              eMode   )
{
    css::uno::Reference< css::uno::XInterface > xCFG = ConfigurationHelper::openConfig(rxContext, sPackage, eMode);
    return ConfigurationHelper::readRelativeKey(xCFG, sRelPath, sKey);
}

//-----------------------------------------------
void ConfigurationHelper::writeDirectKey(const css::uno::Reference< css::uno::XComponentContext >&    rxContext,
                                         const ::rtl::OUString&                                       sPackage,
                                         const ::rtl::OUString&                                       sRelPath,
                                         const ::rtl::OUString&                                       sKey    ,
                                         const css::uno::Any&                                         aValue  ,
                                               sal_Int32                                              eMode   )
{
    css::uno::Reference< css::uno::XInterface > xCFG = ConfigurationHelper::openConfig(rxContext, sPackage, eMode);
    ConfigurationHelper::writeRelativeKey(xCFG, sRelPath, sKey, aValue);
    ConfigurationHelper::flush(xCFG);
}

//-----------------------------------------------
void ConfigurationHelper::flush(const css::uno::Reference< css::uno::XInterface >& xCFG)
{
    css::uno::Reference< css::util::XChangesBatch > xBatch(xCFG, css::uno::UNO_QUERY_THROW);
    xBatch->commitChanges();
}

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
