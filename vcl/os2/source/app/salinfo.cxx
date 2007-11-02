/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: salinfo.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 12:48:22 $
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

#define INCL_PM
#define INCL_DOS
#define INCL_GPI
#include <svpm.h>

#include <tools/string.hxx>
#include <salsys.h>
#include <salframe.h>
#include <salinst.h>
#include <tools/debug.hxx>
#include <vcl/svdata.hxx>
#include <rtl/ustrbuf.hxx>

#ifndef _SV_SALGTYPE_HXX
//#include <salgtype.hxx>
#endif

class Os2SalSystem : public SalSystem
{
public:
    Os2SalSystem() {}
    virtual ~Os2SalSystem();

    virtual unsigned int GetDisplayScreenCount();
    virtual Rectangle GetDisplayScreenPosSizePixel( unsigned int nScreen );
    //virtual bool GetSalSystemDisplayInfo( DisplayInfo& rInfo );

    virtual bool IsMultiDisplay();
    virtual unsigned int GetDefaultDisplayNumber();
    virtual Rectangle GetDisplayWorkAreaPosSizePixel( unsigned int nScreen );
    virtual rtl::OUString GetScreenName( unsigned int nScreen );

    virtual int ShowNativeMessageBox( const String& rTitle,
                                      const String& rMessage,
                                      int nButtonCombination,
                                      int nDefaultButton);
};

SalSystem* Os2SalInstance::CreateSalSystem()
{
    return new Os2SalSystem();
}

Os2SalSystem::~Os2SalSystem()
{
}

// -----------------------------------------------------------------------
#if 0
bool Os2SalSystem::GetSalSystemDisplayInfo( DisplayInfo& rInfo )
{
    HDC hDC;
    if( hDC = WinQueryWindowDC(HWND_DESKTOP) )
    {
        LONG bitCount;
        DevQueryCaps(hDC, CAPS_COLOR_BITCOUNT, CAPS_COLOR_BITCOUNT, &bitCount);
        rInfo.nWidth    = WinQuerySysValue( HWND_DESKTOP, SV_CXSCREEN );
        rInfo.nHeight   = WinQuerySysValue( HWND_DESKTOP, SV_CYSCREEN );
        rInfo.nDepth    = bitCount;
        return true;
    }
    else
        return false;
}
#endif

unsigned int Os2SalSystem::GetDisplayScreenCount()
{
    return 1;
}

Rectangle Os2SalSystem::GetDisplayScreenPosSizePixel( unsigned int nScreen )
{
    Rectangle aRet;
    aRet = Rectangle( Point(), Point( WinQuerySysValue( HWND_DESKTOP, SV_CXSCREEN ),
    WinQuerySysValue( HWND_DESKTOP, SV_CYSCREEN ) ) );
    return aRet;
}

// -----------------------------------------------------------------------
/* We have to map the button identifier to the identifier used by the Os232
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

static int COMBI_BTN_MAPPING_TABLE[] =
{
    MB_OK, MB_OKCANCEL, MB_ABORTRETRYIGNORE, MB_YESNO, MB_YESNOCANCEL, MB_RETRYCANCEL
};

int Os2SalSystem::ShowNativeMessageBox(const String& rTitle, const String& rMessage, int nButtonCombination, int nDefaultButton)
{
    DBG_ASSERT( nButtonCombination >= SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK &&
                nButtonCombination <= SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_RETRY_CANCEL &&
                nDefaultButton >= SALSYSTEM_SHOWNATIVEMSGBOX_BTN_OK &&
                nDefaultButton <= SALSYSTEM_SHOWNATIVEMSGBOX_BTN_NO, "Invalid arguments!" );

    int nFlags = MB_APPLMODAL | MB_WARNING | COMBI_BTN_MAPPING_TABLE[nButtonCombination];

    if (nButtonCombination >= SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_OK &&
        nButtonCombination <= SALSYSTEM_SHOWNATIVEMSGBOX_BTNCOMBI_RETRY_CANCEL &&
        nDefaultButton >= SALSYSTEM_SHOWNATIVEMSGBOX_BTN_OK &&
        nDefaultButton <= SALSYSTEM_SHOWNATIVEMSGBOX_BTN_NO)
        nFlags |= DEFAULT_BTN_MAPPING_TABLE[nButtonCombination][nDefaultButton];

    //#107209 hide the splash screen if active
#if 0
    ImplSVData* pSVData = ImplGetSVData();
    if (pSVData->mpIntroWindow)
        pSVData->mpIntroWindow->Hide();
#endif

    return WinMessageBox(
        HWND_DESKTOP, HWND_DESKTOP,
        (PSZ)rMessage.GetBuffer(),
        (PSZ)rTitle.GetBuffer(),
        0, nFlags);
}


unsigned int Os2SalSystem::GetDefaultDisplayNumber()
{
    return 0;
}

bool Os2SalSystem::IsMultiDisplay()
{
    return false;
}

Rectangle Os2SalSystem::GetDisplayWorkAreaPosSizePixel( unsigned int nScreen )
{
    return GetDisplayScreenPosSizePixel( nScreen );
}

rtl::OUString Os2SalSystem::GetScreenName( unsigned int nScreen )
{
   rtl::OUStringBuffer aBuf( 32 );
   aBuf.appendAscii( "VirtualScreen " );
   aBuf.append( sal_Int32(nScreen) );
   return aBuf.makeStringAndClear();
}
