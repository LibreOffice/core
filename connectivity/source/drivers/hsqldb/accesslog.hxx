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

#ifndef CONNECTIVITY_HSQLDB_ACCESSLOG_HXX
#define CONNECTIVITY_HSQLDB_ACCESSLOG_HXX

#ifdef HSQLDB_DBG

#include <stdio.h>
#include <jni.h>
#include <rtl/ustring.hxx>
#include <rtl/string.hxx>

namespace connectivity { namespace hsqldb
{
    class LogFile
    {
    private:
        ::rtl::OUString     m_sFileName;

    public:
        LogFile( JNIEnv* env, jstring streamName, const sal_Char* _pAsciiSuffix );

    public:
                void    writeString( const sal_Char* _pString, bool _bEndLine = true );
                void    create() { getLogFile(); }
        virtual void    close();

    protected:
        FILE*&  getLogFile();
    };

    class OperationLogFile : public LogFile
    {
    public:
        OperationLogFile( JNIEnv* env, jstring streamName, const sal_Char* _pAsciiSuffix )
            :LogFile( env, streamName, ( ::rtl::OString( _pAsciiSuffix ) += ".op" ).getStr() )
        {
        }

        void logOperation( const sal_Char* _pOp )
        {
            writeString( _pOp, true );
        }

        void logOperation( const sal_Char* _pOp, jlong _nLongArg )
        {
            ::rtl::OString sLine( _pOp );
            sLine += "( ";
            sLine += ::rtl::OString::valueOf( _nLongArg );
            sLine += " )";
            writeString( sLine.getStr(), true );
        }

        void logReturn( jlong _nRetVal )
        {
            ::rtl::OString sLine( " -> " );
            sLine += ::rtl::OString::valueOf( _nRetVal );
            writeString( sLine.getStr(), true );
        }

        void logReturn( jint _nRetVal )
        {
            ::rtl::OString sLine( " -> " );
            sLine += ::rtl::OString::valueOf( _nRetVal );
            writeString( sLine.getStr(), true );
        }

        virtual void close()
        {
            writeString( "-------------------------------", true );
            writeString( "", true );
            LogFile::close();
        }
    };

    class DataLogFile : public LogFile
    {
    public:
        DataLogFile( JNIEnv* env, jstring streamName, const sal_Char* _pAsciiSuffix )
            :LogFile( env, streamName, _pAsciiSuffix )
        {
        }

        void write( jint value )
        {
            fputc( value, getLogFile() );
            fflush( getLogFile() );
        }

        void write( const sal_Int8* buffer, sal_Int32 bytesRead )
        {
            fwrite( buffer, sizeof(sal_Int8), bytesRead, getLogFile() );
            fflush( getLogFile() );
        }

        sal_Int64 seek( sal_Int64 pos )
        {
            FILE* pFile = getLogFile();
            fseek( pFile, 0, SEEK_END );
            if ( ftell( pFile ) < pos )
            {
                sal_Int8 filler( 0 );
                while ( ftell( pFile ) < pos )
                    fwrite( &filler, sizeof( sal_Int8 ), 1, pFile );
                fflush( pFile );
            }
            fseek( pFile, pos, SEEK_SET );
            return ftell( pFile );
        }

        sal_Int64 tell()
        {
            return ftell( getLogFile() );
        }
    };

} }
#endif

#endif // CONNECTIVITY_HSQLDB_ACCESSLOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
