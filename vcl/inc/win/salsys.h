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
        OUString        m_aName;
        OUString        m_aDeviceName;
        Rectangle            m_aArea;
        sal_Int32            m_nStateFlags;

        DisplayMonitor() : m_nStateFlags( 0 ) {}
        DisplayMonitor( const OUString& rName,
                        const OUString& rDevName,
                        const Rectangle& rArea,
                        DWORD nStateFlags )
                        : m_aName( rName ),
                          m_aDeviceName( rDevName ),
                          m_aArea( rArea ),
                          m_nStateFlags( nStateFlags )
        {
        }
        ~DisplayMonitor() {}
    };
    private:
    std::vector<DisplayMonitor>             m_aMonitors;
    std::map<OUString, unsigned int>   m_aDeviceNameToMonitor;
    unsigned int                            m_nPrimary;
public:
    WinSalSystem() : m_nPrimary( 0 ) {}
    virtual ~WinSalSystem();

    virtual unsigned int GetDisplayScreenCount();
    virtual unsigned int GetDisplayBuiltInScreen();
    virtual Rectangle GetDisplayScreenPosSizePixel( unsigned int nScreen );
    virtual OUString GetDisplayScreenName( unsigned int nScreen );
    virtual int ShowNativeMessageBox( const OUString& rTitle,
                                      const OUString& rMessage,
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
