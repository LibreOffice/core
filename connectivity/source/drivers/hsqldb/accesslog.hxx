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
        OUString     m_sFileName;

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
            :LogFile( env, streamName, ( OString( _pAsciiSuffix ) += ".op" ).getStr() )
        {
        }

        void logOperation( const sal_Char* _pOp )
        {
            writeString( _pOp, true );
        }

        void logOperation( const sal_Char* _pOp, jlong _nLongArg )
        {
            OString sLine( _pOp );
            sLine += "( ";
            sLine += OString::number( _nLongArg );
            sLine += " )";
            writeString( sLine.getStr(), true );
        }

        void logReturn( jlong _nRetVal )
        {
            OString sLine( " -> " );
            sLine += OString::number( _nRetVal );
            writeString( sLine.getStr(), true );
        }

        void logReturn( jint _nRetVal )
        {
            OString sLine( " -> " );
            sLine += OString::number( _nRetVal );
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
