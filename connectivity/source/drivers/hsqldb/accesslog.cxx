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
#include "precompiled_connectivity.hxx"

#ifdef HSQLDB_DBG
#include "accesslog.hxx"
#include "hsqldb/HStorageMap.hxx"

#include <osl/thread.h>

namespace connectivity { namespace hsqldb
{
    DECLARE_STL_USTRINGACCESS_MAP(FILE *,TDebugStreamMap);
    TDebugStreamMap& getStreams()
    {
        static TDebugStreamMap streams;
        return streams;
    }

    //---------------------------------------------------------------------
    LogFile::LogFile( JNIEnv* env, jstring streamName, const sal_Char* _pAsciiSuffix )
    {
        m_sFileName = StorageContainer::jstring2ustring(env,streamName);
        m_sFileName += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("."));
        m_sFileName += ::rtl::OUString::createFromAscii( _pAsciiSuffix );
    }

    //---------------------------------------------------------------------
    FILE*& LogFile::getLogFile()
    {
        FILE*& pLogFile = getStreams()[m_sFileName];
        if ( !pLogFile )
        {
            ::rtl::OString sByteLogName = ::rtl::OUStringToOString(m_sFileName,osl_getThreadTextEncoding());
            pLogFile = fopen( sByteLogName.getStr(), "a+" );
        }
        return pLogFile;
    }

    //---------------------------------------------------------------------
    void LogFile::writeString( const sal_Char* _pString, bool _bEndLine )
    {
        FILE* pLogFile = getLogFile();
        fwrite( _pString, sizeof( *_pString ), strlen( _pString ), pLogFile );
        if ( _bEndLine )
            fwrite( "\n", sizeof( *_pString ), strlen( "\n" ), pLogFile );
        fflush( pLogFile );
    }

    //---------------------------------------------------------------------
    void LogFile::close()
    {
        fclose( getLogFile() );
        getLogFile() = NULL;
    }
} }
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
