/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accesslog.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2005-09-23 11:41:13 $
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

#ifndef CONNECTIVITY_HSQLDB_ACCESSLOG_HXX
#define CONNECTIVITY_HSQLDB_ACCESSLOG_HXX

#ifdef HSQLDB_DBG

#include <stdio.h>
#include <jni.h>

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif

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
