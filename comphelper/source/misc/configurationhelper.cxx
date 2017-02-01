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
#include <comphelper/sequence.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>


namespace comphelper{


css::uno::Reference< css::uno::XInterface > ConfigurationHelper::openConfig(const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                                                                            const OUString&                                           sPackage,
                                                                                  EConfigurationModes                                 eMode   )
{
    css::uno::Reference< css::lang::XMultiServiceFactory > xConfigProvider(
        css::configuration::theDefaultProvider::get( rxContext ) );

    ::std::vector< css::uno::Any > lParams;
    css::beans::PropertyValue      aParam ;

    // set root path
    aParam.Name    = "nodepath";
    aParam.Value <<= sPackage;
    lParams.push_back(css::uno::makeAny(aParam));

    // enable all locales mode
    if (eMode & EConfigurationModes::AllLocales)
    {
        aParam.Name    = "locale";
        aParam.Value <<= OUString("*");
        lParams.push_back(css::uno::makeAny(aParam));
    }

    // enable lazy writing
    bool bLazy(eMode & EConfigurationModes::LazyWrite);
    aParam.Name    = "lazywrite";
    aParam.Value   = css::uno::makeAny(bLazy);
    lParams.push_back(css::uno::makeAny(aParam));

    // open it
    css::uno::Reference< css::uno::XInterface > xCFG;

    bool bReadOnly(eMode & EConfigurationModes::ReadOnly);
    if (bReadOnly)
        xCFG = xConfigProvider->createInstanceWithArguments(
                "com.sun.star.configuration.ConfigurationAccess",
                comphelper::containerToSequence(lParams));
    else
        xCFG = xConfigProvider->createInstanceWithArguments(
                "com.sun.star.configuration.ConfigurationUpdateAccess",
                comphelper::containerToSequence(lParams));

    return xCFG;
}


css::uno::Any ConfigurationHelper::readRelativeKey(const css::uno::Reference< css::uno::XInterface >& xCFG    ,
                                                   const OUString&                            sRelPath,
                                                   const OUString&                            sKey    )
{
    css::uno::Reference< css::container::XHierarchicalNameAccess > xAccess(xCFG, css::uno::UNO_QUERY_THROW);

    css::uno::Reference< css::beans::XPropertySet > xProps;
    xAccess->getByHierarchicalName(sRelPath) >>= xProps;
    if (!xProps.is())
    {
        throw css::container::NoSuchElementException(
            "The requested path \"" + sRelPath + "\" does not exist.");
    }
    return xProps->getPropertyValue(sKey);
}


void ConfigurationHelper::writeRelativeKey(const css::uno::Reference< css::uno::XInterface >& xCFG    ,
                                           const OUString&                            sRelPath,
                                           const OUString&                            sKey    ,
                                           const css::uno::Any&                              aValue  )
{
    css::uno::Reference< css::container::XHierarchicalNameAccess > xAccess(xCFG, css::uno::UNO_QUERY_THROW);

    css::uno::Reference< css::beans::XPropertySet > xProps;
    xAccess->getByHierarchicalName(sRelPath) >>= xProps;
    if (!xProps.is())
    {
        throw css::container::NoSuchElementException(
            "The requested path \"" + sRelPath + "\" does not exist.");
    }
    xProps->setPropertyValue(sKey, aValue);
}


css::uno::Reference< css::uno::XInterface > ConfigurationHelper::makeSureSetNodeExists(const css::uno::Reference< css::uno::XInterface >& xCFG         ,
                                                                                       const OUString&                            sRelPathToSet,
                                                                                       const OUString&                            sSetNode     )
{
    css::uno::Reference< css::container::XHierarchicalNameAccess > xAccess(xCFG, css::uno::UNO_QUERY_THROW);
    css::uno::Reference< css::container::XNameAccess > xSet;
    xAccess->getByHierarchicalName(sRelPathToSet) >>= xSet;
    if (!xSet.is())
    {
        throw css::container::NoSuchElementException(
            "The requested path \"" + sRelPathToSet + "\" does not exist." );
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


css::uno::Any ConfigurationHelper::readDirectKey(const css::uno::Reference< css::uno::XComponentContext >&    rxContext,
                                                 const OUString&                                       sPackage,
                                                 const OUString&                                       sRelPath,
                                                 const OUString&                                       sKey    ,
                                                       EConfigurationModes                             eMode   )
{
    css::uno::Reference< css::uno::XInterface > xCFG = ConfigurationHelper::openConfig(rxContext, sPackage, eMode);
    return ConfigurationHelper::readRelativeKey(xCFG, sRelPath, sKey);
}


void ConfigurationHelper::writeDirectKey(const css::uno::Reference< css::uno::XComponentContext >&    rxContext,
                                         const OUString&                                       sPackage,
                                         const OUString&                                       sRelPath,
                                         const OUString&                                       sKey    ,
                                         const css::uno::Any&                                  aValue  ,
                                               EConfigurationModes                             eMode   )
{
    css::uno::Reference< css::uno::XInterface > xCFG = ConfigurationHelper::openConfig(rxContext, sPackage, eMode);
    ConfigurationHelper::writeRelativeKey(xCFG, sRelPath, sKey, aValue);
    ConfigurationHelper::flush(xCFG);
}


void ConfigurationHelper::flush(const css::uno::Reference< css::uno::XInterface >& xCFG)
{
    css::uno::Reference< css::util::XChangesBatch > xBatch(xCFG, css::uno::UNO_QUERY_THROW);
    xBatch->commitChanges();
}

} // namespace comphelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
