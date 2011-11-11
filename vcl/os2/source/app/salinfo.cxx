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



#define INCL_PM
#define INCL_DOS
#define INCL_GPI
#include <svpm.h>

#include "svsys.h"
#include "rtl/ustrbuf.hxx"

#include "tools/debug.hxx"
#include "tools/string.hxx"

#include "vcl/window.hxx"

#include "os2/salsys.h"
#include "os2/salframe.h"
#include "os2/salinst.h"
#include "os2/saldata.hxx"

#include "svdata.hxx"

#define CHAR_POINTER(THE_OUSTRING) ::rtl::OUStringToOString (THE_OUSTRING, RTL_TEXTENCODING_UTF8).pData->buffer

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
    ImplSVData* pSVData = ImplGetSVData();
    if (pSVData->mpIntroWindow)
        pSVData->mpIntroWindow->Hide();

    return WinMessageBox(
        HWND_DESKTOP, HWND_DESKTOP,
        (PSZ)CHAR_POINTER(rMessage),
        (PSZ)CHAR_POINTER(rTitle),
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
