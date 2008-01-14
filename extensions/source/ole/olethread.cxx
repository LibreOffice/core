/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: olethread.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 14:47:24 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"
#if defined _MSC_VER && ( _MSC_VER >= 1200 )
#define _WIN32_WINNT  0x0400
#endif

#include "ole2uno.hxx"

#include <tools/presys.h>
#include <tools/postsys.h>

#include <vos/thread.hxx>

using namespace vos;
using namespace std;

namespace ole_adapter
{
// CoInitializeEx *
typedef DECLSPEC_IMPORT HRESULT (STDAPICALLTYPE *ptrCoInitEx)( LPVOID, DWORD);
// CoInitialize *
typedef DECLSPEC_IMPORT HRESULT (STDAPICALLTYPE *ptrCoInit)( LPVOID);

void o2u_attachCurrentThread()
{
    static OThreadData oleThreadData;

    if ((sal_Bool)(sal_IntPtr)oleThreadData.getData() != sal_True)
    {
        HINSTANCE inst= LoadLibrary( _T("ole32.dll"));
        if( inst )
        {
            HRESULT hr;
            // Try DCOM
            ptrCoInitEx initFuncEx= (ptrCoInitEx)GetProcAddress( inst, _T("CoInitializeEx"));
            if( initFuncEx)
                hr= initFuncEx( NULL, COINIT_MULTITHREADED);
            // No DCOM, try COM
            else
            {
                ptrCoInit initFunc= (ptrCoInit)GetProcAddress( inst,_T("CoInitialize"));
                if( initFunc)
                    hr= initFunc( NULL);
            }
        }
        oleThreadData.setData((void*)sal_True);
    }
}

} // end namespace
