/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salinfo.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-26 09:20:53 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#define VCL_NEED_BASETSD
#include <tools/presys.h>
#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#include <tools/postsys.h>

#include <tools/string.hxx>
#include <salsys.hxx>
#include <salframe.h>
#include <salinst.h>
#include <tools/debug.hxx>
#include <svdata.hxx>
#include <window.hxx>

#include <multimon.h>

#include <vector>

class WinSalSystem : public SalSystem
{
    std::vector< Rectangle > m_aMonitors;
public:
    WinSalSystem() {}
    virtual ~WinSalSystem();

    virtual unsigned int GetDisplayScreenCount();
    virtual Rectangle GetDisplayScreenPosSizePixel( unsigned int nScreen );
    virtual int ShowNativeMessageBox( const String& rTitle,
                                      const String& rMessage,
                                      int nButtonCombination,
                                      int nDefaultButton);

    bool initMonitors();
    void addMonitor( const Rectangle& rRect) { m_aMonitors.push_back( rRect ); }
};

SalSystem* WinSalInstance::CreateSalSystem()
{
    return new WinSalSystem();
}

WinSalSystem::~WinSalSystem()
{
}

// -----------------------------------------------------------------------

static BOOL CALLBACK ImplEnumMonitorProc( HMONITOR hMonitor,
                                          HDC hdcMonitor,
                                          LPRECT lprcMonitor,
                                          LPARAM dwData )
{
    WinSalSystem* pSys = reinterpret_cast<WinSalSystem*>(dwData);
    pSys->addMonitor( Rectangle( Point( lprcMonitor->left,
                                        lprcMonitor->top ),
                                 Size( lprcMonitor->right - lprcMonitor->left,
                                       lprcMonitor->bottom - lprcMonitor->top ) ) );
    return TRUE;
}

bool WinSalSystem::initMonitors()
{
    if( m_aMonitors.size() > 0 )
        return true;

    bool winVerOk = true;

    // multi monitor calls not available on Win95/NT
    OSVERSIONINFO aVerInfo;
    aVerInfo.dwOSVersionInfoSize = sizeof( aVerInfo );
    if ( GetVersionEx( &aVerInfo ) )
    {
        if ( aVerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT )
        {
            if ( aVerInfo.dwMajorVersion <= 4 )
                winVerOk = false;   // NT
        }
        else if( aVerInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS )
        {
            if ( aVerInfo.dwMajorVersion == 4 && aVerInfo.dwMinorVersion == 0 )
                winVerOk = false;   // Win95
        }
    }
    if( winVerOk )
    {
        int nMonitors = GetSystemMetrics( SM_CMONITORS );
        if( nMonitors == 1 )
        {
            int w = GetSystemMetrics( SM_CXSCREEN );
            int h = GetSystemMetrics( SM_CYSCREEN );
            m_aMonitors.push_back( Rectangle( Point(), Size( w, h ) ) );
        }
        else
        {
            HDC aDesktopRC = GetDC( NULL );
            EnumDisplayMonitors( aDesktopRC, NULL, ImplEnumMonitorProc, reinterpret_cast<LPARAM>(this) );
        }
    }
    else
    {
        int w = GetSystemMetrics( SM_CXSCREEN );
        int h = GetSystemMetrics( SM_CYSCREEN );
        m_aMonitors.push_back( Rectangle( Point(), Size( w, h ) ) );
    }

    return m_aMonitors.size() > 0;
}

unsigned int WinSalSystem::GetDisplayScreenCount()
{
    initMonitors();
    return m_aMonitors.size();
}

Rectangle WinSalSystem::GetDisplayScreenPosSizePixel( unsigned int nScreen )
{
    initMonitors();
    return (nScreen < m_aMonitors.size()) ? m_aMonitors[nScreen] : Rectangle();
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

int WinSalSystem::ShowNativeMessageBox(const String& rTitle, const String& rMessage, int nButtonCombination, int nDefaultButton)
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

    //#107209 hide the splash screen if active
    ImplSVData* pSVData = ImplGetSVData();
    if (pSVData->mpIntroWindow)
        pSVData->mpIntroWindow->Hide();

    return MessageBoxW(
        0,
        rMessage.GetBuffer(),
        rTitle.GetBuffer(),
        nFlags);
}
