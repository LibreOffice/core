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
