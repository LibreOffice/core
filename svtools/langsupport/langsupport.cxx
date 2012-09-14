/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Caolán McNamara <caolanm@redhat.com>
 * Portions created by the Initial Developer are Copyright (C) 2011 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Caolán McNamara <caolanm@redhat.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <i18npool/mslangid.hxx>
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

    InitVCL(xSFactory);

    {
        SvtLanguageTable aTable;

        sal_uInt32 nCount = aTable.GetEntryCount();
        for (sal_uInt32 i = 0; i < nCount; ++i)
        {
            LanguageType eLang = aTable.GetTypeAtIndex(i);

            if (eLang >= LANGUAGE_USER1 && eLang <= LANGUAGE_USER9)
                continue;

            if (eLang == LANGUAGE_DONTKNOW ||
                eLang == LANGUAGE_NONE ||
                eLang == LANGUAGE_HID_HUMAN_INTERFACE_DEVICE ||
                eLang == LANGUAGE_SYSTEM)
            {
                continue;
            }

            lang::Locale aLocale;

            MsLangId::convertLanguageToLocale(eLang, aLocale);

            rtl::OUStringBuffer aBuf(aLocale.Language);
            if (!aLocale.Country.isEmpty())
                aBuf.append('-').append(aLocale.Country);
            rtl::OUString sTag = aBuf.makeStringAndClear();

            std::cout << rtl::OUStringToOString(sTag, osl_getThreadTextEncoding()).getStr()
                << std::endl;
        }
    }

    DeInitVCL();

    uno::Reference< lang::XComponent >(xContext, uno::UNO_QUERY_THROW)->dispose();

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
