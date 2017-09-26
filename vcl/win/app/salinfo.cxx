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

#include "svsys.h"
#include "rtl/ustrbuf.hxx"


#include <vcl/window.hxx>

#include "win/salsys.h"
#include "win/salframe.h"
#include "win/salinst.h"
#include "win/saldata.hxx"

#include "svdata.hxx"

#include <unordered_map>

SalSystem* WinSalInstance::CreateSalSystem()
{
    return new WinSalSystem();
}

WinSalSystem::~WinSalSystem()
{
}

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

bool WinSalSystem::handleMonitorCallback( sal_IntPtr hMonitor, sal_IntPtr, sal_IntPtr )
{
    MONITORINFOEXW aInfo;
    aInfo.cbSize = sizeof( aInfo );
    if( GetMonitorInfoW( reinterpret_cast<HMONITOR>(hMonitor), &aInfo ) )
    {
        aInfo.szDevice[CCHDEVICENAME-1] = 0;
        OUString aDeviceName( SAL_U(aInfo.szDevice) );
        std::map< OUString, unsigned int >::const_iterator it =
            m_aDeviceNameToMonitor.find( aDeviceName );
        if( it != m_aDeviceNameToMonitor.end() )
        {
            DisplayMonitor& rMon( m_aMonitors[ it->second ] );
            rMon.m_aArea = tools::Rectangle( Point( aInfo.rcMonitor.left,
                                             aInfo.rcMonitor.top ),
                                      Size( aInfo.rcMonitor.right - aInfo.rcMonitor.left,
                                            aInfo.rcMonitor.bottom - aInfo.rcMonitor.top ) );
            if( (aInfo.dwFlags & MONITORINFOF_PRIMARY) != 0 )
                m_nPrimary = it->second;
        }
    }
    return true;
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
        m_aMonitors.push_back( DisplayMonitor( OUString(),
                                               tools::Rectangle( Point(), Size( w, h ) ) ) );
        m_aDeviceNameToMonitor[ OUString() ] = 0;
        m_nPrimary = 0;
    }
    else
    {
        DISPLAY_DEVICEW aDev;
        aDev.cb = sizeof( aDev );
        DWORD nDevice = 0;
        std::unordered_map< OUString, int, OUStringHash > aDeviceStringCount;
        while( EnumDisplayDevicesW( nullptr, nDevice++, &aDev, 0 ) )
        {
            if( (aDev.StateFlags & DISPLAY_DEVICE_ACTIVE)
                && !(aDev.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) ) // sort out non/disabled monitors
            {
                aDev.DeviceName[31] = 0;
                aDev.DeviceString[127] = 0;
                OUString aDeviceName( SAL_U(aDev.DeviceName) );
                OUString aDeviceString( SAL_U(aDev.DeviceString) );
                if( aDeviceStringCount.find( aDeviceString ) == aDeviceStringCount.end() )
                    aDeviceStringCount[ aDeviceString ] = 1;
                else
                    aDeviceStringCount[ aDeviceString ]++;
                m_aDeviceNameToMonitor[ aDeviceName ] = m_aMonitors.size();
                m_aMonitors.push_back( DisplayMonitor( aDeviceString,
                                                       tools::Rectangle() ) );
            }
        }
        HDC aDesktopRC = GetDC( nullptr );
        EnumDisplayMonitors( aDesktopRC, nullptr, ImplEnumMonitorProc, reinterpret_cast<LPARAM>(this) );

        // append monitor numbers to name strings
        std::unordered_map< OUString, int, OUStringHash > aDevCount( aDeviceStringCount );
        unsigned int nMonitorCount = m_aMonitors.size();
        for( unsigned int i = 0; i < nMonitorCount; i++ )
        {
            const OUString& rDev( m_aMonitors[i].m_aName );
            if( aDeviceStringCount[ rDev ] > 1 )
            {
                int nInstance = aDeviceStringCount[ rDev ] - (-- aDevCount[ rDev ] );
                OUStringBuffer aBuf( rDev.getLength() + 8 );
                aBuf.append( rDev );
                aBuf.append( " (" );
                aBuf.append( sal_Int32( nInstance ) );
                aBuf.append( ')' );
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

tools::Rectangle WinSalSystem::GetDisplayScreenPosSizePixel( unsigned int nScreen )
{
    initMonitors();
    return (nScreen < m_aMonitors.size()) ? m_aMonitors[nScreen].m_aArea : tools::Rectangle();
}

int WinSalSystem::ShowNativeMessageBox(const OUString& rTitle, const OUString& rMessage)
{
    ImplHideSplash();
    return MessageBoxW(
        nullptr,
        SAL_W(rMessage.getStr()),
        SAL_W(rTitle.getStr()),
        MB_TASKMODAL | MB_SETFOREGROUND | MB_ICONWARNING | MB_DEFBUTTON1);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
