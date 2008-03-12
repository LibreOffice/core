/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: platform_vista.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 07:35:02 $
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

#ifndef _PLATFORM_VISTA_H_
#define _PLATFORM_VISTA_H_

#pragma once

// Change these values to use different versions
#undef WINVER
#undef _WIN32_WINNT
#undef _WIN32_IE
#undef _WTL_NO_CSTRING

#define WINVER          0x0600
#define _WIN32_WINNT    0x0600
#define _WIN32_IE       0x0700
#define _WTL_NO_CSTRING

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <comip.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

// ATL/WTL
//#include <atlbase.h>
//#include <atlstr.h>
//#include <atlapp.h>
//extern CAppModule _Module;
//#include <atlcom.h>
//#include <atlwin.h>
//#include <atlframe.h>
//#include <atlcrack.h>
//#include <atlctrls.h>
//#include <atlctrlx.h>
//#include <atldlgs.h>
//#include <atlmisc.h>

/*
// STL
#include <vector>

// Global functions
LPCTSTR PrepFilterString ( CString& sFilters );
bool    PathFromShellItem ( IShellItem* pItem, CString& sPath );
bool    BuildFilterSpecList ( _U_STRINGorID szFilterList,
                              std::vector<CString>& vecsFilterParts,
                              std::vector<COMDLG_FILTERSPEC>& vecFilters );
*/

#if defined _M_IX86
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif

#endif

