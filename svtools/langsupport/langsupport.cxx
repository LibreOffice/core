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
 */

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <sal/main.h>
#include <svtools/langtab.hxx>
#include <tools/extendapplicationenvironment.hxx>
#include <vcl/svapp.hxx>
#include <iostream>

using namespace com::sun::star;

SAL_IMPLEMENT_MAIN()
{
    tools::extendApplicationEnvironment();

    uno::Reference<uno::XComponentContext> xContext =
        cppu::defaultBootstrap_InitialComponentContext();

    uno::Reference<lang::XMultiComponentFactory> xFactory =
        xContext->getServiceManager();

    uno::Reference<lang::XMultiServiceFactory> xSFactory(xFactory,
        uno::UNO_QUERY_THROW);

    comphelper::setProcessServiceFactory(xSFactory);

    InitVCL();

    {
        SvtLanguageTable aTable;

        sal_uInt32 nCount = aTable.GetEntryCount();
        for (sal_uInt32 i = 0; i < nCount; ++i)
        {
            LanguageType eLang = aTable.GetTypeAtIndex(i);

            if (eLang == LANGUAGE_DONTKNOW ||
                eLang == LANGUAGE_NONE ||
                eLang == LANGUAGE_HID_HUMAN_INTERFACE_DEVICE ||
                eLang == LANGUAGE_SYSTEM)
            {
                continue;
            }

            OUString sTag( LanguageTag::convertToBcp47( eLang));

            std::cout << OUStringToOString(sTag, osl_getThreadTextEncoding()).getStr()
                << std::endl;
        }
    }

    DeInitVCL();

    uno::Reference< lang::XComponent >(xContext, uno::UNO_QUERY_THROW)->dispose();

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
