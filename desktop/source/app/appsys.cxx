/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: appsys.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 09:34:45 $
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
#include "precompiled_desktop.hxx"

#include "appsys.hxx"

#ifdef WNT

#include <rtl/ustring.hxx>
#include <tools/solar.h>
#include <tools/prewin.h>
#include <winreg.h>
#include <tools/postwin.h>
#include <tools/urlobj.hxx>


#define U2S(STRING)     ::rtl::OUStringToOString(STRING, RTL_TEXTENCODING_UTF8)

namespace desktop
{

void FATToVFat_Impl( String& aName )
{
    INetURLObject aObj( aName );
    if ( aObj.GetProtocol() == INET_PROT_FILE )
    {
        WIN32_FIND_DATA aData;
        HANDLE h = FindFirstFile( U2S(aName).getStr(), &aData );
        if ( h )
        {
            // FAT-Kurzname in VFAT-Langname wandeln
            aObj.removeSegment();
            aObj.insertName( String::CreateFromAscii( aData.cFileName ) );
            aName = aObj.PathToFileName();
            FindClose( h );
        }
    }
}

}

#endif // WNT

