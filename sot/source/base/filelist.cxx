/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <rtl/ustrbuf.hxx>
#include <tools/stream.hxx>
#include <sot/exchange.hxx>
#include <sot/filelist.hxx>
#include <osl/diagnose.h>
#include <osl/thread.h>

/*************************************************************************
|*
|*    FileList - Ctor/Dtor
|*
\*************************************************************************/

FileList::~FileList()
{
    ClearAll();
}

void FileList::ClearAll()
{
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
        aStrList.push_back( rFileList.aStrList[ i ] );
    return *this;
}

/******************************************************************************
|*
|*  Stream-Operatoren
|*
\******************************************************************************/

/* #i28176#
   The Windows clipboard bridge now provides a double '\0'
   terminated list of file names for format SotClipboardFormatId::FILE_LIST
   instead of the original Windows Sv_DROPFILES structure. All strings
   in this list are UTF16 strings. Shell link files will be already
   resolved by the Windows clipboard bridge.*/
SvStream& ReadFileList( SvStream& rIStm, FileList& rFileList )
{
    rFileList.ClearAll();

    OUStringBuffer sBuf(512);
    sal_uInt16 c;

    while (!rIStm.IsEof())
    {
        // read first character of filepath; c==0 > reach end of stream
        rIStm.ReadUInt16( c );
        if (!c)
            break;

        // read string till c==0
        while (c && !rIStm.IsEof())
        {
            sBuf.append((sal_Unicode)c);
            rIStm.ReadUInt16( c );
        }

        // append the filepath
        rFileList.AppendFile(sBuf.toString());
        sBuf.truncate();
    }
    return rIStm;
}

/******************************************************************************
|*
|*  Liste fuellen/abfragen
|*
\******************************************************************************/

void FileList::AppendFile( const OUString& rStr )
{
    aStrList.push_back( rStr );
}

OUString FileList::GetFile( size_t i ) const
{
    OUString aStr;
    if( i < aStrList.size() )
        aStr = aStrList[ i ];
    return aStr;
}

size_t FileList::Count() const
{
    return aStrList.size();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
