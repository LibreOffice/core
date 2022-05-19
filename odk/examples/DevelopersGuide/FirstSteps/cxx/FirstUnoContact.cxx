/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4;
 * fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <cppuhelper/bootstrap.hxx>
#include <iostream>
#include <sal/main.h>

SAL_IMPLEMENT_MAIN()
{
    try
    {
        css::uno::Reference<css::uno::XComponentContext> xContext(cppu::bootstrap());
        std::cout << "Connected to a running office ..." << std::endl;
        css::uno::Reference<css::lang::XMultiComponentFactory> xMCF = xContext->getServiceManager();
        std::string available = xMCF != nullptr ? "available" : "not available";
        std::cout << "remote ServiceManager is " + available << std::endl;
    }
    catch (css::uno::Exception& e)
    {
        std::cout << e.Message << std::endl;
        return 1;
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s
 * cinkeys+=0=break: */
