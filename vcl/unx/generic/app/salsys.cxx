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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <unx/salunx.h>
#include <unx/dtint.hxx>
#include <unx/saldata.hxx>
#include <unx/salinst.h>
#include <unx/saldisp.hxx>
#include <unx/salsys.h>

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
    SalDisplay* pSalDisp = GetX11SalData()->GetDisplay();
    return pSalDisp->IsXinerama() ? pSalDisp->GetXineramaScreens().size() : pSalDisp->GetScreenCount();
}

bool X11SalSystem::IsMultiDisplay()
{
    SalDisplay* pSalDisp = GetX11SalData()->GetDisplay();
    unsigned int nScreenCount = pSalDisp->GetScreenCount();
    return pSalDisp->IsXinerama() ? false : (nScreenCount > 1);
}

unsigned int X11SalSystem::GetDefaultDisplayNumber()
{
    SalDisplay* pSalDisp = GetX11SalData()->GetDisplay();
    return pSalDisp->IsXinerama() ? pSalDisp->GetDefaultMonitorNumber() : pSalDisp->GetDefaultScreenNumber();
}

Rectangle X11SalSystem::GetDisplayScreenPosSizePixel( unsigned int nScreen )
{
    Rectangle aRet;
    SalDisplay* pSalDisp = GetX11SalData()->GetDisplay();
    if( pSalDisp->IsXinerama() )
    {
        const std::vector< Rectangle >& rScreens = pSalDisp->GetXineramaScreens();
        if( nScreen < rScreens.size() )
            aRet = rScreens[nScreen];
    }
    else
    {
        const SalDisplay::ScreenData& rScreen = pSalDisp->getDataForScreen( nScreen );
        aRet = Rectangle( Point( 0, 0 ), rScreen.m_aSize );
    }

    return aRet;
}

Rectangle X11SalSystem::GetDisplayWorkAreaPosSizePixel( unsigned int nScreen )
{
    // FIXME: workareas
    return GetDisplayScreenPosSizePixel( nScreen );
}

rtl::OUString X11SalSystem::GetScreenName( unsigned int nScreen )
{
    rtl::OUString aScreenName;
    SalDisplay* pSalDisp = GetX11SalData()->GetDisplay();
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
        if( nScreen >= static_cast<unsigned int>(pSalDisp->GetScreenCount()) )
            nScreen = 0;
        rtl::OUStringBuffer aBuf( 256 );
        aBuf.append( rtl::OStringToOUString( rtl::OString( DisplayString( pSalDisp->GetDisplay() ) ), osl_getThreadTextEncoding() ) );
        // search backwards for ':'
        int nPos = aBuf.getLength();
        if( nPos > 0 )
            nPos--;
        while( nPos > 0 && aBuf.charAt( nPos ) != ':' )
            nPos--;
        // search forward to '.'
        while( nPos < aBuf.getLength() && aBuf.charAt( nPos ) != '.' )
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

int X11SalSystem::ShowNativeDialog( const String& rTitle, const String& rMessage, const std::list< String >& rButtons, int nDefButton )
{
    int nRet = -1;

    ImplSVData* pSVData = ImplGetSVData();
    if( pSVData->mpIntroWindow )
        pSVData->mpIntroWindow->Hide();

    WarningBox aWarn( NULL, WB_STDWORK, rMessage );
    aWarn.SetText( rTitle );
    aWarn.Clear();

    sal_uInt16 nButton = 0;
    for( std::list< String >::const_iterator it = rButtons.begin(); it != rButtons.end(); ++it )
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

int X11SalSystem::ShowNativeMessageBox(const String& rTitle, const String& rMessage, int nButtonCombination, int nDefaultButton)
{
    int nDefButton = 0;
    std::list< String > aButtons;
    int nButtonIds[5], nBut = 0;

    if( nButtonCombination == SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK ||
        nButtonCombination == SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK_CANCEL )
    {
        aButtons.push_back( Button::GetStandardText( BUTTON_OK ) );
        nButtonIds[nBut++] = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_OK;
    }
    if( nButtonCombination == SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_YES_NO_CANCEL ||
        nButtonCombination == SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_YES_NO )
    {
        aButtons.push_back( Button::GetStandardText( BUTTON_YES ) );
        nButtonIds[nBut++] = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_YES;
        aButtons.push_back( Button::GetStandardText( BUTTON_NO ) );
        nButtonIds[nBut++] = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_NO;
        if( nDefaultButton == SALSYSTEM_SHOWNATIVEMSGBOX_BTN_NO )
            nDefButton = 1;
    }
    if( nButtonCombination == SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK_CANCEL ||
        nButtonCombination == SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_YES_NO_CANCEL ||
        nButtonCombination == SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_RETRY_CANCEL )
    {
        if( nButtonCombination == SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_RETRY_CANCEL )
        {
            aButtons.push_back( Button::GetStandardText( BUTTON_RETRY ) );
            nButtonIds[nBut++] = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_RETRY;
        }
        aButtons.push_back( Button::GetStandardText( BUTTON_CANCEL ) );
        nButtonIds[nBut++] = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL;
        if( nDefaultButton == SALSYSTEM_SHOWNATIVEMSGBOX_BTN_CANCEL )
            nDefButton = aButtons.size()-1;
    }
    if( nButtonCombination == SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_ABORT_RETRY_IGNORE )
    {
        aButtons.push_back( Button::GetStandardText( BUTTON_ABORT ) );
        nButtonIds[nBut++] = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_ABORT;
        aButtons.push_back( Button::GetStandardText( BUTTON_RETRY ) );
        nButtonIds[nBut++] = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_RETRY;
        aButtons.push_back( Button::GetStandardText( BUTTON_IGNORE ) );
        nButtonIds[nBut++] = SALSYSTEM_SHOWNATIVEMSGBOX_BTN_IGNORE;
        switch( nDefaultButton )
        {
            case SALSYSTEM_SHOWNATIVEMSGBOX_BTN_RETRY: nDefButton = 1;break;
            case SALSYSTEM_SHOWNATIVEMSGBOX_BTN_IGNORE: nDefButton = 2;break;
        }
    }
    int nResult = ShowNativeDialog( rTitle, rMessage, aButtons, nDefButton );

    return nResult != -1 ? nButtonIds[ nResult ] : 0;
}
