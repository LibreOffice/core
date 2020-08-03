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

#pragma once

#ifdef HSQLDB_DBG

#include <jni.h>
#include <rtl/ustring.hxx>
#include <rtl/string.hxx>

namespace connectivity::hsqldb
{
    class LogFile
    {
    private:
        OUString     m_sFileName;

    public:
        LogFile( JNIEnv* env, jstring streamName, const char* _pAsciiSuffix );

    public:
                void    writeString( const char* _pString, bool _bEndLine = true );
                void    create() { getLogFile(); }
        virtual void    close();

    protected:
        FILE*&  getLogFile();
    };

    class OperationLogFile : public LogFile
    {
    public:
        OperationLogFile( JNIEnv* env, jstring streamName, const char* _pAsciiSuffix )
            :LogFile( env, streamName, ( OString( _pAsciiSuffix ) += ".op" ).getStr() )
        {
        }

        void logOperation( const char* _pOp )
        {
            writeString( _pOp, true );
        }

        void logOperation( const char* _pOp, jlong _nLongArg )
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
        DataLogFile( JNIEnv* env, jstring streamName, const char* _pAsciiSuffix )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
