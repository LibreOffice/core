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

#include <sal/main.h>
#include <sal/log.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <tools/extendapplicationenvironment.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <comphelper/processfactory.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <framework/desktop.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace cppu;

// Forward declaration
static void Main();

SAL_IMPLEMENT_MAIN()
{
    try
    {
        tools::extendApplicationEnvironment();

        Reference< XComponentContext > xContext = defaultBootstrap_InitialComponentContext();
        Reference< XMultiServiceFactory > xServiceManager( xContext->getServiceManager(), UNO_QUERY );

        if( !xServiceManager.is() )
            Application::Abort( u"Failed to bootstrap"_ustr );

        comphelper::setProcessServiceFactory( xServiceManager );

        LanguageTag::setConfiguredSystemLanguage(MsLangId::getSystemLanguage());

        InitVCL();
        ::Main();

        framework::getDesktop(::comphelper::getProcessComponentContext())->terminate();

        DeInitVCL();
    }
    catch (const Exception&)
    {
        TOOLS_WARN_EXCEPTION("vcl.app", "Fatal");
        return 1;
    }
    catch (const std::exception &e)
    {
        fprintf(stderr, "fatal error: %s\n", e.what());
        return 1;
    }

    return 0;
}

namespace {

class MyWin : public WorkWindow
{
public:
                MyWin( vcl::Window* pParent, WinBits nWinStyle );
};

}

void Main()
{
    ScopedVclPtrInstance< MyWin > aMainWin( nullptr, WB_APP | WB_STDWORK );
    aMainWin->SetText(u"VCL - Workbench"_ustr);
    aMainWin->Show();

    Application::Execute();
}

MyWin::MyWin( vcl::Window* pParent, WinBits nWinStyle ) :
    WorkWindow( pParent, nWinStyle )
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
