/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include "appsys.hxx"

#ifdef WNT

#include <rtl/ustring.hxx>
#include <tools/solar.h>
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
            // Change FAT short filename into VFAT long filename
            aObj.removeSegment();
            aObj.insertName( String::CreateFromAscii( aData.cFileName ) );
            aName = aObj.PathToFileName();
            FindClose( h );
        }
    }
}

}

#endif // WNT

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
