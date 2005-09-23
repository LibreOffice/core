/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accesslog.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2005-09-23 11:40:53 $
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

#ifdef HSQLDB_DBG

#ifndef CONNECTIVITY_HSQLDB_ACCESSLOG_HXX
#include "accesslog.hxx"
#endif

#ifndef CONNECTIVI_HSQLDB_HSTORAGEMAP_HXX
#include "hsqldb/HStorageMap.hxx"
#endif

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
