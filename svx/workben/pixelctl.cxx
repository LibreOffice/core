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

#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/logging.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <tools/extendapplicationenvironment.hxx>

#include <svx/dlgctrl.hxx>

#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>

#include <math.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace cppu;

// Forward declaration
void Main();

SAL_IMPLEMENT_MAIN()
{
    try
    {
        tools::extendApplicationEnvironment();

        // create the global service-manager
        Reference< XComponentContext > xContext = defaultBootstrap_InitialComponentContext();
        Reference< XMultiServiceFactory > xServiceManager( xContext->getServiceManager(), UNO_QUERY );

        if( !xServiceManager.is() )
            Application::Abort( "Failed to bootstrap" );

        comphelper::setProcessServiceFactory( xServiceManager );

        InitVCL();
        ::Main();
        DeInitVCL();
    }
    catch (const Exception& e)
    {
        SAL_WARN("vcl.app", "Fatal: " << e);
        return 1;
    }
    catch (const std::exception& e)
    {
        SAL_WARN("vcl.app", "Fatal: " << e.what());
        return 1;
    }

    return 0;
}

class MyWin : public WorkWindow
{
    VclPtr<SvxPixelCtl> maPixelCtl;

public:
    MyWin(vcl::Window* pParent, WinBits nWinStyle);
    virtual ~MyWin() override
    {
        disposeOnce();
    }
    virtual void dispose() override;
    bool Close() override;
};

void Main()
{
    ScopedVclPtrInstance< MyWin > aMainWin( nullptr, WB_STDWORK );
    aMainWin->SetText( "SvxPixelCtl" );
    aMainWin->Show();

    Application::Execute();
}

MyWin::MyWin( vcl::Window* pParent, WinBits nWinStyle ) :
    WorkWindow( pParent, nWinStyle ),
    maPixelCtl( VclPtr<SvxPixelCtl>::Create(this) )
{
    maPixelCtl->SetPosSizePixel( Point( 10, 10 ), Size( 200, 200 ) );
    maPixelCtl->Show();

}

void MyWin::dispose()
{
    maPixelCtl.disposeAndClear();
    WorkWindow::dispose();
}

bool MyWin::Close()
{
    bool bRet = WorkWindow::Close();
    if( bRet )
        Application::Quit();
    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
