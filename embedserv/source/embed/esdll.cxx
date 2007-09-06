/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: esdll.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: kz $ $Date: 2007-09-06 14:15:58 $
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

#define STRICT
#define _WIN32_WINNT 0x0400
#define _WIN32_DCOM
#if defined(_MSC_VER) && (_MSC_VER >= 1300)
#undef _DEBUG
#endif
#if defined(_MSC_VER) && (_MSC_VER > 1310)
#pragma warning(disable : 4917 4555)
#endif

#ifdef __MINGW32__
#define _INIT_ATL_COMMON_VARS
#endif

#include "stdafx.h"

#include <atlbase.h>
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable: 4710 )
#endif
CComModule _Module;
#ifdef _MSC_VER
#pragma warning( pop )
#endif
#include <atlcom.h>

BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()

/////////////////////////////////////////////////////////////////////////////
// DLL Entry Point


#include "syswinwrapper.hxx"
#include "docholder.hxx"


HINSTANCE DocumentHolder::m_hInstance;

extern "C"
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID /*lpReserved*/)
{
    DocumentHolder::m_hInstance = hInstance;
    if (!winwrap::HatchWindowRegister(hInstance))
        return FALSE;

    if (dwReason == DLL_PROCESS_ATTACH)
    {
        _Module.Init(ObjectMap, hInstance, NULL);
        DisableThreadLibraryCalls(hInstance);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        _Module.Term();
    }
    return TRUE;    // ok
}

// Fix strange warnings about some
// ATL::CAxHostWindow::QueryInterface|AddRef|Releae functions.
// warning C4505: 'xxx' : unreferenced local function has been removed
#if defined(_MSC_VER)
#pragma warning(disable: 4505)
#endif