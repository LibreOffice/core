/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salinfo.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 14:02:34 $
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
#include <windows.h>
#include <tools/postsys.h>

#include <tools/string.hxx>
#include <salsys.hxx>
#include <salframe.h>
#include <salinst.h>
#include <tools/debug.hxx>
#include <svdata.hxx>
#include <window.hxx>

class WinSalSystem : public SalSystem
{
public:
    WinSalSystem() {}
    virtual ~WinSalSystem();

    virtual bool GetSalSystemDisplayInfo( DisplayInfo& rInfo );
    virtual int ShowNativeMessageBox( const String& rTitle,
                                      const String& rMessage,
                                      int nButtonCombination,
                                      int nDefaultButton);
};

SalSystem* WinSalInstance::CreateSalSystem()
{
    return new WinSalSystem();
}

WinSalSystem::~WinSalSystem()
{
}

// -----------------------------------------------------------------------

bool WinSalSystem::GetSalSystemDisplayInfo( DisplayInfo& rInfo )
{
    RECT aRect;
    ImplSalGetWorkArea( NULL, &aRect, NULL );

    HDC hDC;
    if( hDC = GetDC( NULL ) )
    {
        rInfo.nWidth    = aRect.right - aRect.left;
        rInfo.nHeight   = aRect.bottom - aRect.top;
        rInfo.nDepth    = GetDeviceCaps( hDC, BITSPIXEL );
        ReleaseDC( NULL, hDC );
        return true;
    }
    else
        return false;
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
