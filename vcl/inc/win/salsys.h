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

#ifndef _SV_SALSYS_H
#define _SV_SALSYS_H

#include <salsys.hxx>

#include <vector>
#include <map>

class WinSalSystem : public SalSystem
{
    public:
    struct DisplayMonitor
    {
        rtl::OUString        m_aName;
        rtl::OUString        m_aDeviceName;
        Rectangle            m_aArea;
        Rectangle            m_aWorkArea;
        sal_Int32            m_nStateFlags;

        DisplayMonitor() : m_nStateFlags( 0 ) {}
        DisplayMonitor( const rtl::OUString& rName,
                        const rtl::OUString& rDevName,
                        const Rectangle& rArea,
                        const Rectangle& rWorkArea,
                        DWORD nStateFlags )
                        : m_aName( rName ),
                          m_aDeviceName( rDevName ),
                          m_aArea( rArea ),
                          m_aWorkArea( rWorkArea ),
                          m_nStateFlags( nStateFlags )
        {
        }
        ~DisplayMonitor() {}
    };
    private:
    std::vector<DisplayMonitor>             m_aMonitors;
    std::map<rtl::OUString, unsigned int>   m_aDeviceNameToMonitor;
    unsigned int                            m_nPrimary;
public:
    WinSalSystem() : m_nPrimary( 0 ) {}
    virtual ~WinSalSystem();

    virtual unsigned int GetDisplayScreenCount();
    virtual unsigned int GetDisplayBuiltInScreen();
    virtual Rectangle GetDisplayScreenPosSizePixel( unsigned int nScreen );
    virtual Rectangle GetDisplayScreenWorkAreaPosSizePixel( unsigned int nScreen );
    virtual rtl::OUString GetDisplayScreenName( unsigned int nScreen );
    virtual int ShowNativeMessageBox( const rtl::OUString& rTitle,
                                      const rtl::OUString& rMessage,
                                      int nButtonCombination,
                                      int nDefaultButton, bool bUseResources);
    bool initMonitors();
    // discards monitorinfo; used by WM_DISPLAYCHANGED handler
    void clearMonitors();
    const std::vector<DisplayMonitor>& getMonitors()
    { initMonitors(); return m_aMonitors;}

    sal_Bool handleMonitorCallback( sal_IntPtr /*HMONITOR*/,
                                sal_IntPtr /*HDC*/,
                                sal_IntPtr /*LPRECT*/ );
};

#endif // _SV_SALSYS_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
