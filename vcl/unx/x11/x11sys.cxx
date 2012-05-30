/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


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

// -----------------------------------------------------------------------

X11SalSystem::~X11SalSystem()
{
}

// for the moment only handle xinerama case
unsigned int X11SalSystem::GetDisplayScreenCount()
{
    SalDisplay* pSalDisp = GetGenericData()->GetSalDisplay();
    return pSalDisp->IsXinerama() ? pSalDisp->GetXineramaScreens().size() :
           pSalDisp->GetXScreenCount();
}

bool X11SalSystem::IsUnifiedDisplay()
{
    SalDisplay* pSalDisp = GetGenericData()->GetSalDisplay();
    unsigned int nScreenCount = pSalDisp->GetXScreenCount();
    return pSalDisp->IsXinerama() ? true : (nScreenCount == 1);
}

unsigned int X11SalSystem::GetDisplayBuiltInScreen()
{
    SalDisplay* pSalDisp = GetGenericData()->GetSalDisplay();
    return pSalDisp->GetDefaultXScreen().getXScreen();
}

Rectangle X11SalSystem::GetDisplayScreenPosSizePixel( unsigned int nScreen )
{
    Rectangle aRet;
    SalDisplay* pSalDisp = GetGenericData()->GetSalDisplay();
    if( pSalDisp->IsXinerama() )
    {
        const std::vector< Rectangle >& rScreens = pSalDisp->GetXineramaScreens();
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

Rectangle X11SalSystem::GetDisplayScreenWorkAreaPosSizePixel( unsigned int nScreen )
{
    // FIXME: workareas
    return GetDisplayScreenPosSizePixel( nScreen );
}

rtl::OUString X11SalSystem::GetDisplayScreenName( unsigned int nScreen )
{
    rtl::OUString aScreenName;
    SalDisplay* pSalDisp = GetGenericData()->GetSalDisplay();
    if( pSalDisp->IsXinerama() )
    {
        const std::vector< Rectangle >& rScreens = pSalDisp->GetXineramaScreens();
        if( nScreen >= rScreens.size() )
            nScreen = 0;
        rtl::OUStringBuffer aBuf( 256 );
        aBuf.append( rtl::OStringToOUString( rtl::OString( DisplayString( pSalDisp->GetDisplay() ) ), osl_getThreadTextEncoding() ) );
        aBuf.appendAscii( " [" );
        aBuf.append( static_cast<sal_Int32>(nScreen) );
        aBuf.append( sal_Unicode(']') );
        aScreenName = aBuf.makeStringAndClear();
    }
    else
    {
        if( nScreen >= static_cast<unsigned int>(pSalDisp->GetXScreenCount()) )
            nScreen = 0;
        rtl::OUStringBuffer aBuf( 256 );
        aBuf.append( rtl::OStringToOUString( rtl::OString( DisplayString( pSalDisp->GetDisplay() ) ), osl_getThreadTextEncoding() ) );
        // search backwards for ':'
        int nPos = aBuf.getLength();
        if( nPos > 0 )
            nPos--;
        while( nPos > 0 && aBuf[nPos] != ':' )
            nPos--;
        // search forward to '.'
        while( nPos < aBuf.getLength() && aBuf[nPos] != '.' )
            nPos++;
        if( nPos < aBuf.getLength() )
            aBuf.setLength( nPos+1 );
        else
            aBuf.append( sal_Unicode('.') );
        aBuf.append( static_cast<sal_Int32>(nScreen) );
        aScreenName = aBuf.makeStringAndClear();
    }
    return aScreenName;
}

int X11SalSystem::ShowNativeDialog( const rtl::OUString& rTitle, const rtl::OUString& rMessage, const std::list< rtl::OUString >& rButtons, int nDefButton )
{
    int nRet = -1;

    ImplSVData* pSVData = ImplGetSVData();
    if( pSVData->mpIntroWindow )
        pSVData->mpIntroWindow->Hide();

    WarningBox aWarn( NULL, WB_STDWORK, rMessage );
    aWarn.SetText( rTitle );
    aWarn.Clear();

    sal_uInt16 nButton = 0;
    for( std::list< rtl::OUString >::const_iterator it = rButtons.begin(); it != rButtons.end(); ++it )
    {
            aWarn.AddButton( *it, nButton+1, nButton == (sal_uInt16)nDefButton ? BUTTONDIALOG_DEFBUTTON : 0 );
            nButton++;
    }
    aWarn.SetFocusButton( (sal_uInt16)nDefButton+1 );

    nRet = ((int)aWarn.Execute()) - 1;

    // normalize behaviour, actually this should never happen
    if( nRet < -1 || nRet >= int(rButtons.size()) )
        nRet = -1;

    return nRet;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
