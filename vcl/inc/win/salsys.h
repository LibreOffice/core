/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    virtual bool IsMultiDisplay();
    virtual unsigned int GetDefaultDisplayNumber();
    virtual Rectangle GetDisplayScreenPosSizePixel( unsigned int nScreen );
    virtual Rectangle GetDisplayWorkAreaPosSizePixel( unsigned int nScreen );
    virtual rtl::OUString GetScreenName( unsigned int nScreen );
    virtual int ShowNativeMessageBox( const String& rTitle,
                                      const String& rMessage,
                                      int nButtonCombination,
                                      int nDefaultButton);
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

