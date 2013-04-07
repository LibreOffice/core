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
        m_sFileName += OUString(".");
        m_sFileName += OUString::createFromAscii( _pAsciiSuffix );
    }

    //---------------------------------------------------------------------
    FILE*& LogFile::getLogFile()
    {
        FILE*& pLogFile = getStreams()[m_sFileName];
        if ( !pLogFile )
        {
            OString sByteLogName = OUStringToOString(m_sFileName,osl_getThreadTextEncoding());
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
