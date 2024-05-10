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

#include <comphelper/processfactory.hxx>

#include <vcl/printer/Options.hxx>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

namespace vcl::printer
{
void Options::ReadFromConfig(bool i_bFile)
{
    bool bSuccess = false;
    // save old state in case something goes wrong
    Options aOldValues(*this);

    // get the configuration service
    css::uno::Reference<css::lang::XMultiServiceFactory> xConfigProvider;
    css::uno::Reference<css::container::XNameAccess> xConfigAccess;
    try
    {
        // get service provider
        css::uno::Reference<css::uno::XComponentContext> xContext(
            comphelper::getProcessComponentContext());
        // create configuration hierarchical access name
        try
        {
            xConfigProvider = css::configuration::theDefaultProvider::get(xContext);

            css::beans::PropertyValue aVal;
            aVal.Name = "nodepath";
            if (i_bFile)
                aVal.Value <<= u"/org.openoffice.Office.Common/Print/Option/File"_ustr;
            else
                aVal.Value <<= u"/org.openoffice.Office.Common/Print/Option/Printer"_ustr;
            xConfigAccess.set(xConfigProvider->createInstanceWithArguments(
                                  u"com.sun.star.configuration.ConfigurationAccess"_ustr,
                                  { css::uno::Any(aVal) }),
                              css::uno::UNO_QUERY);
            if (xConfigAccess.is())
            {
                css::uno::Reference<css::beans::XPropertySet> xSet(xConfigAccess,
                                                                   css::uno::UNO_QUERY);
                if (xSet.is())
                {
                    sal_Int32 nValue = 0;
                    bool bValue = false;
                    if (xSet->getPropertyValue(u"ReduceTransparency"_ustr) >>= bValue)
                        SetReduceTransparency(bValue);
                    if (xSet->getPropertyValue(u"ReducedTransparencyMode"_ustr) >>= nValue)
                        SetReducedTransparencyMode(static_cast<TransparencyMode>(nValue));
                    if (xSet->getPropertyValue(u"ReduceGradients"_ustr) >>= bValue)
                        SetReduceGradients(bValue);
                    if (xSet->getPropertyValue(u"ReducedGradientMode"_ustr) >>= nValue)
                        SetReducedGradientMode(static_cast<GradientMode>(nValue));
                    if (xSet->getPropertyValue(u"ReducedGradientStepCount"_ustr) >>= nValue)
                        SetReducedGradientStepCount(static_cast<sal_uInt16>(nValue));
                    if (xSet->getPropertyValue(u"ReduceBitmaps"_ustr) >>= bValue)
                        SetReduceBitmaps(bValue);
                    if (xSet->getPropertyValue(u"ReducedBitmapMode"_ustr) >>= nValue)
                        SetReducedBitmapMode(static_cast<BitmapMode>(nValue));
                    if (xSet->getPropertyValue(u"ReducedBitmapResolution"_ustr) >>= nValue)
                        SetReducedBitmapResolution(static_cast<sal_uInt16>(nValue));
                    if (xSet->getPropertyValue(u"ReducedBitmapIncludesTransparency"_ustr)
                        >>= bValue)
                        SetReducedBitmapIncludesTransparency(bValue);
                    if (xSet->getPropertyValue(u"ConvertToGreyscales"_ustr) >>= bValue)
                        SetConvertToGreyscales(bValue);

                    bSuccess = true;
                }
            }
        }
        catch (const css::uno::Exception&)
        {
        }
    }
    catch (const css::lang::WrappedTargetException&)
    {
    }

    if (!bSuccess)
        *this = aOldValues;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
