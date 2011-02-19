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
#include "precompiled_sot.hxx"

#include <tools/stream.hxx>
#include <tools/string.hxx>
#include <tools/rtti.hxx>
#include <sot/exchange.hxx>
#include <filelist.hxx>
#include <osl/thread.h>

TYPEINIT1_AUTOFACTORY( FileList, SvDataCopyStream );

/*************************************************************************
|*
|*    FileList - Ctor/Dtor
|*
\*************************************************************************/

FileList::~FileList()
{
    ClearAll();
}

void FileList::ClearAll( void )
{
    for ( size_t i = 0, n = aStrList.size(); i < n; ++i )
        delete aStrList[ i ];
    aStrList.clear();
}

/*************************************************************************
|*
|*    FileList - Zuweisungsoperator
|*
\*************************************************************************/

FileList& FileList::operator=( const FileList& rFileList )
{
    for ( size_t i = 0, n = rFileList.aStrList.size(); i < n; ++i )
        aStrList.push_back( new String( *rFileList.aStrList[ i ] ) );
    return *this;
}

/*************************************************************************
|*
|*    FileList::GetFormatName()
|*
\*************************************************************************/

ULONG FileList::GetFormat()
{
    return FORMAT_FILE_LIST;
}

/******************************************************************************
|*
|*  virtuelle SvData-Methoden
|*
\******************************************************************************/

void FileList::Load( SvStream& rIStm )
{
    rIStm >> *this;
}

void FileList::Save( SvStream& rOStm )
{
    rOStm << *this;
}

void FileList::Assign( const SvDataCopyStream& rCopyStream )
{
    *this = (const FileList&)rCopyStream;
}

/******************************************************************************
|*
|*  Stream-Operatoren
|*
\******************************************************************************/

/*
 * NOTE: to correctly handle this Protocol with Unicode, native Win32 must be called:
 * e.g. DropQueryFile
 */

SvStream& operator<<( SvStream& rOStm, const FileList& /*rFileList*/ )
{
    OSL_ENSURE(false, "Not implemented!");
    return rOStm;
}

/* #i28176#
   The Windows clipboard bridge now provides a double '\0'
   terminated list of file names for format SOT_FORMAT_FILE_LIST
   instead of the original Windows Sv_DROPFILES structure. All strings
   in this list are UTF16 strings. Shell link files will be already
   resolved by the Windows clipboard bridge.*/
SvStream& operator>>( SvStream& rIStm, FileList& rFileList )
{
    rFileList.ClearAll();

    String aStr;
    sal_uInt16 c;

    while (!rIStm.IsEof())
    {
        aStr.Erase();

        // read first character of filepath; c==0 > reach end of stream
        rIStm >> c;
        if (!c)
            break;

        // read string till c==0
        while (c && !rIStm.IsEof())
        {
            aStr += (sal_Unicode)c;
            rIStm >> c;
        }

        // append the filepath
        rFileList.AppendFile(aStr);
    }
    return rIStm;
}

/******************************************************************************
|*
|*  Liste fuellen/abfragen
|*
\******************************************************************************/

void FileList::AppendFile( const String& rStr )
{
    aStrList.push_back( new String( rStr ) );
}

String FileList::GetFile( size_t i ) const
{
    String aStr;
    if( i < aStrList.size() )
        aStr = *aStrList[ i ];
    return aStr;
}

size_t FileList::Count( void ) const
{
    return aStrList.size();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
