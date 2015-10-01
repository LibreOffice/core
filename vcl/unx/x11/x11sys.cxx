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

#include <unx/salunx.h>
#include <unx/saldata.hxx>
#include <unx/salinst.h>
#include <unx/saldisp.hxx>
#include <unx/x11/x11sys.hxx>

#include <vcl/msgbox.hxx>
#include <vcl/button.hxx>

#include <svdata.hxx>

#include <rtl/ustrbuf.hxx>
#include <osl/thread.h>

SalSystem* X11SalInstance::CreateSalSystem()
{
    return new X11SalSystem();
}

X11SalSystem::~X11SalSystem()
{
}

// for the moment only handle xinerama case
unsigned int X11SalSystem::GetDisplayScreenCount()
{
    SalDisplay* pSalDisp = vcl_sal::getSalDisplay(GetGenericData());
    return pSalDisp->IsXinerama() ? pSalDisp->GetXineramaScreens().size() :
           pSalDisp->GetXScreenCount();
}

bool X11SalSystem::IsUnifiedDisplay()
{
    SalDisplay* pSalDisp = vcl_sal::getSalDisplay(GetGenericData());
    unsigned int nScreenCount = pSalDisp->GetXScreenCount();
    return pSalDisp->IsXinerama() || (nScreenCount == 1);
}

unsigned int X11SalSystem::GetDisplayBuiltInScreen()
{
    SalDisplay* pSalDisp = vcl_sal::getSalDisplay(GetGenericData());
    return pSalDisp->GetDefaultXScreen().getXScreen();
}

Rectangle X11SalSystem::GetDisplayScreenPosSizePixel( unsigned int nScreen )
{
    Rectangle aRet;
    SalDisplay* pSalDisp = vcl_sal::getSalDisplay(GetGenericData());
    if( pSalDisp->IsXinerama() )
    {
        const std::vector< Rectangle >& rScreens = pSalDisp->GetXineramaScreens();

        // we shouldn't be able to pick a screen > number of screens available
        assert(nScreen < rScreens.size() );

        if( nScreen < rScreens.size() )
            aRet = rScreens[nScreen];
    }
    else
    {
        const SalDisplay::ScreenData& rScreen =
            pSalDisp->getDataForScreen( SalX11Screen( nScreen ) );
        aRet = Rectangle( Point( 0, 0 ), rScreen.m_aSize );
    }

    return aRet;
}

int X11SalSystem::ShowNativeDialog( const OUString& rTitle, const OUString& rMessage, const std::list< OUString >& rButtons, int nDefButton )
{
    int nRet = -1;

    ImplSVData* pSVData = ImplGetSVData();
    if( pSVData->mpIntroWindow )
        pSVData->mpIntroWindow->Hide();

    ScopedVclPtrInstance<WarningBox> aWarn(nullptr, WB_STDWORK, rMessage);
    aWarn->SetText( rTitle );
    aWarn->Clear();

    sal_uInt16 nButton = 0;
    for( std::list< OUString >::const_iterator it = rButtons.begin(); it != rButtons.end(); ++it )
    {
            aWarn->AddButton( *it, nButton+1, nButton == (sal_uInt16)nDefButton ? ButtonDialogFlags::Default : ButtonDialogFlags::NONE );
            nButton++;
    }
    aWarn->SetFocusButton( (sal_uInt16)nDefButton+1 );

    nRet = ((int)aWarn->Execute()) - 1;

    // normalize behaviour, actually this should never happen
    if( nRet < -1 || nRet >= int(rButtons.size()) )
        nRet = -1;

    return nRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
