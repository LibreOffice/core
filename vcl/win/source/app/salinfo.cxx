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


#include "svsys.h"
#include "rtl/ustrbuf.hxx"

#include "tools/debug.hxx"

#include "vcl/window.hxx"

#include "win/salsys.h"
#include "win/salframe.h"
#include "win/salinst.h"
#include "win/saldata.hxx"

#include "svdata.hxx"

#include <boost/unordered_map.hpp>

SalSystem* WinSalInstance::CreateSalSystem()
{
    return new WinSalSystem();
}

WinSalSystem::~WinSalSystem()
{
}

// -----------------------------------------------------------------------

static BOOL CALLBACK ImplEnumMonitorProc( HMONITOR hMonitor,
                                          HDC hDC,
                                          LPRECT lpRect,
                                          LPARAM dwData )
{
    WinSalSystem* pSys = reinterpret_cast<WinSalSystem*>(dwData);
    return pSys->handleMonitorCallback( reinterpret_cast<sal_IntPtr>(hMonitor),
                                        reinterpret_cast<sal_IntPtr>(hDC),
                                        reinterpret_cast<sal_IntPtr>(lpRect) );
}

sal_Bool WinSalSystem::handleMonitorCallback( sal_IntPtr hMonitor, sal_IntPtr, sal_IntPtr )
{
    MONITORINFOEXW aInfo;
    aInfo.cbSize = sizeof( aInfo );
    if( GetMonitorInfoW( reinterpret_cast<HMONITOR>(hMonitor), &aInfo ) )
    {
        aInfo.szDevice[CCHDEVICENAME-1] = 0;
        rtl::OUString aDeviceName( reinterpret_cast<const sal_Unicode *>(aInfo.szDevice) );
        std::map< rtl::OUString, unsigned int >::const_iterator it =
            m_aDeviceNameToMonitor.find( aDeviceName );
        if( it != m_aDeviceNameToMonitor.end() )
        {
            DisplayMonitor& rMon( m_aMonitors[ it->second ] );
            rMon.m_aArea = Rectangle( Point( aInfo.rcMonitor.left,
                                             aInfo.rcMonitor.top ),
                                      Size( aInfo.rcMonitor.right - aInfo.rcMonitor.left,
                                            aInfo.rcMonitor.bottom - aInfo.rcMonitor.top ) );
            rMon.m_aWorkArea = Rectangle( Point( aInfo.rcWork.left,
                                                 aInfo.rcWork.top ),
                                          Size( aInfo.rcWork.right - aInfo.rcWork.left,
                                                aInfo.rcWork.bottom - aInfo.rcWork.top ) );
            if( (aInfo.dwFlags & MONITORINFOF_PRIMARY) != 0 )
                m_nPrimary = it->second;
        }
    }
    return sal_True;
}

void WinSalSystem::clearMonitors()
{
    m_aMonitors.clear();
    m_nPrimary = 0;
}

bool WinSalSystem::initMonitors()
{
    if( m_aMonitors.size() > 0 )
        return true;

    int nMonitors = GetSystemMetrics( SM_CMONITORS );
    if( nMonitors == 1 )
    {
        int w = GetSystemMetrics( SM_CXSCREEN );
        int h = GetSystemMetrics( SM_CYSCREEN );
        m_aMonitors.push_back( DisplayMonitor( rtl::OUString(),
                                               rtl::OUString(),
                                               Rectangle( Point(), Size( w, h ) ),
                                               Rectangle( Point(), Size( w, h ) ),
                                               0 ) );
        m_aDeviceNameToMonitor[ rtl::OUString() ] = 0;
        m_nPrimary = 0;
        RECT aWorkRect;
        if( SystemParametersInfo( SPI_GETWORKAREA, 0, &aWorkRect, 0 ) )
            m_aMonitors.back().m_aWorkArea =  Rectangle( aWorkRect.left, aWorkRect.top,
                                                         aWorkRect.right, aWorkRect.bottom );
    }
    else
    {
        DISPLAY_DEVICEW aDev;
        aDev.cb = sizeof( aDev );
        DWORD nDevice = 0;
        boost::unordered_map< rtl::OUString, int, rtl::OUStringHash > aDeviceStringCount;
        while( EnumDisplayDevicesW( NULL, nDevice++, &aDev, 0 ) )
        {
            if( (aDev.StateFlags & DISPLAY_DEVICE_ACTIVE)
                && !(aDev.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) ) // sort out non/disabled monitors
            {
                aDev.DeviceName[31] = 0;
                aDev.DeviceString[127] = 0;
                rtl::OUString aDeviceName( reinterpret_cast<const sal_Unicode *>(aDev.DeviceName) );
                rtl::OUString aDeviceString( reinterpret_cast<const sal_Unicode *>(aDev.DeviceString) );
                if( aDeviceStringCount.find( aDeviceString ) == aDeviceStringCount.end() )
                    aDeviceStringCount[ aDeviceString ] = 1;
                else
                    aDeviceStringCount[ aDeviceString ]++;
                m_aDeviceNameToMonitor[ aDeviceName ] = m_aMonitors.size();
                m_aMonitors.push_back( DisplayMonitor( aDeviceString,
                                                       aDeviceName,
                                                       Rectangle(),
                                                       Rectangle(),
                                                       aDev.StateFlags ) );
            }
        }
        HDC aDesktopRC = GetDC( NULL );
        EnumDisplayMonitors( aDesktopRC, NULL, ImplEnumMonitorProc, reinterpret_cast<LPARAM>(this) );

        // append monitor numbers to name strings
        boost::unordered_map< rtl::OUString, int, rtl::OUStringHash > aDevCount( aDeviceStringCount );
        unsigned int nMonitorCount = m_aMonitors.size();
        for( unsigned int i = 0; i < nMonitorCount; i++ )
        {
            const rtl::OUString& rDev( m_aMonitors[i].m_aName );
            if( aDeviceStringCount[ rDev ] > 1 )
            {
                int nInstance = aDeviceStringCount[ rDev ] - (-- aDevCount[ rDev ] );
                rtl::OUStringBuffer aBuf( rDev.getLength() + 8 );
                aBuf.append( rDev );
                aBuf.appendAscii( " (" );
                aBuf.append( sal_Int32( nInstance ) );
                aBuf.append( sal_Unicode(')') );
                m_aMonitors[ i ].m_aName = aBuf.makeStringAndClear();
            }
        }
    }

    return m_aMonitors.size() > 0;
}

unsigned int WinSalSystem::GetDisplayScreenCount()
{
    initMonitors();
    return m_aMonitors.size();
}

unsigned int WinSalSystem::GetDisplayBuiltInScreen()
{
    initMonitors();
    return m_nPrimary;
}

Rectangle WinSalSystem::GetDisplayScreenPosSizePixel( unsigned int nScreen )
{
    initMonitors();
    return (nScreen < m_aMonitors.size()) ? m_aMonitors[nScreen].m_aArea : Rectangle();
}

Rectangle WinSalSystem::GetDisplayScreenWorkAreaPosSizePixel( unsigned int nScreen )
{
    initMonitors();
    return (nScreen < m_aMonitors.size()) ? m_aMonitors[nScreen].m_aWorkArea : Rectangle();
}

rtl::OUString WinSalSystem::GetDisplayScreenName( unsigned int nScreen )
{
    initMonitors();
    return (nScreen < m_aMonitors.size()) ? m_aMonitors[nScreen].m_aName : rtl::OUString();
}

// -----------------------------------------------------------------------
/* We have to map the button identifier to the identifier used by the Win32
   Platform SDK to specify the default button for the MessageBox API.
   The first dimension is the button combination, the second dimension
   is the button identifier.
*/
static int DEFAULT_BTN_MAPPING_TABLE[][8] =
{
    //  Undefined        OK             CANCEL         ABORT          RETRY          IGNORE         YES             NO
    { MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON1 }, //OK
    { MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON2, MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON1 }, //OK_CANCEL
    { MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON2, MB_DEFBUTTON3, MB_DEFBUTTON1, MB_DEFBUTTON1 }, //ABORT_RETRY_IGNO
    { MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON3, MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON2 }, //YES_NO_CANCEL
    { MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON2 }, //YES_NO
    { MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON2, MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON1, MB_DEFBUTTON1 }  //RETRY_CANCEL
};

int WinSalSystem::ShowNativeMessageBox(const rtl::OUString& rTitle, const rtl::OUString& rMessage, int nButtonCombination, int nDefaultButton, SAL_UNUSED_PARAMETER bool)
{
    DBG_ASSERT( nButtonCombination >= SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK &&
                nButtonCombination <= SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_RETRY_CANCEL &&
                nDefaultButton >= SALSYSTEM_SHOWNATIVEMSGBOX_BTN_OK &&
                nDefaultButton <= SALSYSTEM_SHOWNATIVEMSGBOX_BTN_NO, "Invalid arguments!" );

    int nFlags = MB_TASKMODAL | MB_SETFOREGROUND | MB_ICONWARNING | nButtonCombination;

    if (nButtonCombination >= SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK &&
        nButtonCombination <= SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_RETRY_CANCEL &&
        nDefaultButton >= SALSYSTEM_SHOWNATIVEMSGBOX_BTN_OK &&
        nDefaultButton <= SALSYSTEM_SHOWNATIVEMSGBOX_BTN_NO)
        nFlags |= DEFAULT_BTN_MAPPING_TABLE[nButtonCombination][nDefaultButton];

    ImplHideSplash();
    return MessageBoxW(
        0,
        reinterpret_cast<LPCWSTR>(rMessage.getStr()),
        reinterpret_cast<LPCWSTR>(rTitle.getStr()),
        nFlags);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
