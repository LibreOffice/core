/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: pdfunzip.cxx,v $
 *
 * $Revision: 1.2 $
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
#include "precompiled_sdext.hxx"

#include <stdio.h>
#include <sal/main.h>
#include <osl/file.h>
#include <osl/thread.h>
#include <rtl/alloc.h>
#include <rtl/ustring.hxx>
#include <rtl/strbuf.hxx>

#include "pdfparse.hxx"

using namespace rtl;
using namespace pdfparse;

void printHelp( const char* pExe )
{
    fprintf( stdout,
    "USAGE: %s [-h,--help] [-a, --extract-add-streams] [-pw, --password <password>] <inputfile> [<outputfile>]\n"
    "  -h, --help: show help\n"
    "  -a, --extract-add-streams: extracts additional streams to outputfile_object\n"
    "      and prints the mimetype found to stdout\n"
    "  -pw, --password: use password for decryption\n"
    , pExe );
}

class FileEmitContext : public EmitContext
{
    oslFileHandle m_aHandle;
    oslFileHandle m_aReadHandle;
    unsigned int  m_nReadLen;

    void openReadFile( const char* pOrigName );

    public:
    FileEmitContext( const char* pFileName, const char* pOrigName, const PDFContainer* pTop );
    virtual ~FileEmitContext();

    virtual bool write( const void* pBuf, unsigned int nLen ) throw();
    virtual unsigned int getCurPos() throw();
    virtual bool copyOrigBytes( unsigned int nOrigOffset, unsigned int nLen ) throw();
    virtual unsigned int readOrigBytes( unsigned int nOrigOffset, unsigned int nLen, void* pBuf ) throw();
};

FileEmitContext::FileEmitContext( const char* pFileName, const char* pOrigName, const PDFContainer* pTop )
    : EmitContext( pTop ),
      m_aHandle( NULL ),
      m_aReadHandle( NULL ),
      m_nReadLen( 0 )
{
    OUString aSysFile( OStringToOUString( OString( pFileName ), osl_getThreadTextEncoding() ) );
    OUString aURL;
    if( osl_getFileURLFromSystemPath( aSysFile.pData, &aURL.pData ) != osl_File_E_None )
    {
        fprintf( stderr, "filename conversion \"%s\" failed\n", pFileName );
        return;
    }

    if( osl_openFile( aURL.pData, &m_aHandle, osl_File_OpenFlag_Write ) == osl_File_E_None )
    {
        if( osl_setFileSize( m_aHandle, 0 ) != osl_File_E_None )
        {
            fprintf( stderr, "could not truncate %s\n", pFileName );
            osl_closeFile( m_aHandle );
            m_aHandle = NULL;
        }
    }
    else if( osl_openFile( aURL.pData, &m_aHandle,
            osl_File_OpenFlag_Write |osl_File_OpenFlag_Create ) != osl_File_E_None )
    {
        fprintf( stderr, "could not open %s\n", pFileName );
        return;
    }
    m_bDeflate = true;

    openReadFile( pOrigName );
}

FileEmitContext::~FileEmitContext()
{
    if( m_aHandle )
        osl_closeFile( m_aHandle );
    if( m_aReadHandle )
        osl_closeFile( m_aReadHandle );
}

void FileEmitContext::openReadFile( const char* pInFile )
{
    OUString aSysFile( OStringToOUString( OString( pInFile ), osl_getThreadTextEncoding() ) );
    OUString aURL;
    if( osl_getFileURLFromSystemPath( aSysFile.pData, &aURL.pData ) != osl_File_E_None )
    {
        fprintf( stderr, "filename conversion \"%s\" failed\n", pInFile );
        return;
    }

    if( osl_openFile( aURL.pData, &m_aReadHandle, osl_File_OpenFlag_Read ) != osl_File_E_None )
    {
        fprintf( stderr, "could not open %s\n", pInFile );
        return;
    }

    if( osl_setFilePos( m_aReadHandle, osl_Pos_End, 0 ) != osl_File_E_None )
    {
        fprintf( stderr, "could not seek to end of %s\n", pInFile );
        osl_closeFile( m_aReadHandle );
        return;
    }

    sal_uInt64 nFileSize = 0;
    if( osl_getFilePos( m_aReadHandle, &nFileSize ) != osl_File_E_None )
    {
        fprintf( stderr, "could not get end pos of %s\n", pInFile );
        osl_closeFile( m_aReadHandle );
        return;
    }

    m_nReadLen = static_cast<unsigned int>(nFileSize);
}

bool FileEmitContext::write( const void* pBuf, unsigned int nLen ) throw()
{
    if( ! m_aHandle )
        return false;

    sal_uInt64 nWrite = static_cast<sal_uInt64>(nLen);
    sal_uInt64 nWritten = 0;
    return (osl_writeFile( m_aHandle, pBuf, nWrite, &nWritten ) == osl_File_E_None)
           && nWrite == nWritten;
}

unsigned int FileEmitContext::getCurPos() throw()
{
    sal_uInt64 nFileSize = 0;
    if( m_aHandle )
    {
        if( osl_getFilePos( m_aHandle, &nFileSize ) != osl_File_E_None )
            nFileSize = 0;
    }
    return static_cast<unsigned int>(nFileSize);
}

bool FileEmitContext::copyOrigBytes( unsigned int nOrigOffset, unsigned int nLen ) throw()
{
    if( nOrigOffset + nLen > m_nReadLen )
        return false;

    if( osl_setFilePos( m_aReadHandle, osl_Pos_Absolut, nOrigOffset ) != osl_File_E_None )
    {
        fprintf( stderr, "could not seek to offset %u\n", nOrigOffset );
        return false;
    }
    void* pBuf = rtl_allocateMemory( nLen );
    if( ! pBuf )
        return false;
    sal_uInt64 nBytesRead = 0;
    if( osl_readFile( m_aReadHandle, pBuf, nLen, &nBytesRead ) != osl_File_E_None
        || nBytesRead != static_cast<sal_uInt64>(nLen) )
    {
        fprintf( stderr, "could not read %u bytes\n", nLen );
        rtl_freeMemory( pBuf );
        return false;
    }
    bool bRet = write( pBuf, nLen );
    rtl_freeMemory( pBuf );
    return bRet;
}

unsigned int FileEmitContext::readOrigBytes( unsigned int nOrigOffset, unsigned int nLen, void* pBuf ) throw()
{
    if( nOrigOffset + nLen > m_nReadLen )
        return 0;

    if( osl_setFilePos( m_aReadHandle, osl_Pos_Absolut, nOrigOffset ) != osl_File_E_None )
    {
        fprintf( stderr, "could not seek to offset %u\n", nOrigOffset );
        return 0;
    }
    sal_uInt64 nBytesRead = 0;
    if( osl_readFile( m_aReadHandle, pBuf, nLen, &nBytesRead ) != osl_File_E_None )
        return 0;
    return static_cast<unsigned int>(nBytesRead);
}

typedef int(*PDFFileHdl)(const char*, const char*, PDFFile*);

int handleFile( const char* pInFile, const char* pOutFile, const char* pPassword, PDFFileHdl pHdl )
{

    PDFReader aParser;
    int nRet = 0;
    PDFEntry* pEntry = aParser.read( pInFile );
    if( pEntry )
    {
        PDFFile* pPDFFile = dynamic_cast<PDFFile*>(pEntry);
        if( pPDFFile )
        {
            fprintf( stdout, "have a %s PDF file\n", pPDFFile->isEncrypted() ? "encrypted" : "unencrypted" );
            if( pPassword )
                fprintf( stdout, "password %s\n",
                         pPDFFile->setupDecryptionData( pPassword ) ? "matches" : "does not match" );
            nRet = pHdl( pInFile, pOutFile, pPDFFile );
        }
        else
            nRet = 20;
        delete pEntry;
    }
    return nRet;
}

int write_unzipFile( const char* pInFile, const char* pOutFile, PDFFile* pPDFFile )
{
    FileEmitContext aContext( pOutFile, pInFile, pPDFFile );
    aContext.m_bDecrypt = pPDFFile->isEncrypted();
    pPDFFile->emit(aContext);
    return 0;
}

int write_addStreamArray( const char* pOutFile, PDFArray* pStreams, PDFFile* pPDFFile, const char* pInFile )
{
    int nRet = 0;
    unsigned int nArrayElements = pStreams->m_aSubElements.size();
    for( unsigned int i = 0; i < nArrayElements-1 && nRet == 0; i++ )
    {
        PDFName* pMimeType = dynamic_cast<PDFName*>(pStreams->m_aSubElements[i]);
        PDFObjectRef* pStreamRef = dynamic_cast<PDFObjectRef*>(pStreams->m_aSubElements[i+1]);
        if( ! pMimeType )
            fprintf( stderr, "error: no mimetype element\n" );
        if( ! pStreamRef )
            fprintf( stderr, "error: no stream ref element\n" );
        if( pMimeType && pStreamRef )
        {
            fprintf( stdout, "found stream %d %d with mimetype %s\n",
                     pStreamRef->m_nNumber, pStreamRef->m_nGeneration,
                     pMimeType->m_aName.getStr() );
            PDFObject* pObject = pPDFFile->findObject( pStreamRef->m_nNumber, pStreamRef->m_nGeneration );
            if( pObject )
            {
                rtl::OStringBuffer aOutStream( pOutFile );
                aOutStream.append( "_stream_" );
                aOutStream.append( sal_Int32(pStreamRef->m_nNumber) );
                aOutStream.append( "_" );
                aOutStream.append( sal_Int32(pStreamRef->m_nGeneration) );
                FileEmitContext aContext( aOutStream.getStr(), pInFile, pPDFFile );
                aContext.m_bDecrypt = pPDFFile->isEncrypted();
                pObject->writeStream( aContext, pPDFFile );
            }
            else
            {
                fprintf( stderr, "object not found\n" );
                nRet = 121;
            }
        }
        else
            nRet = 120;
    }
    return nRet;
}

int write_addStreams( const char* pInFile, const char* pOutFile, PDFFile* pPDFFile )
{
    // find all trailers
    int nRet = 0;
    unsigned int nElements = pPDFFile->m_aSubElements.size();
    for( unsigned i = 0; i < nElements && nRet == 0; i++ )
    {
        PDFTrailer* pTrailer = dynamic_cast<PDFTrailer*>(pPDFFile->m_aSubElements[i]);
        if( pTrailer && pTrailer->m_pDict )
        {
            // search for AdditionalStreams entry
            std::hash_map<rtl::OString,PDFEntry*,rtl::OStringHash>::iterator add_stream;
            add_stream = pTrailer->m_pDict->m_aMap.find( "AdditionalStreams" );
            if( add_stream != pTrailer->m_pDict->m_aMap.end() )
            {
                PDFArray* pStreams = dynamic_cast<PDFArray*>(add_stream->second);
                if( pStreams )
                    nRet = write_addStreamArray( pOutFile, pStreams, pPDFFile, pInFile );
            }
        }
    }
    return nRet;
}

SAL_IMPLEMENT_MAIN_WITH_ARGS( argc, argv )
{
    const char* pInFile = NULL;
    const char* pOutFile = NULL;
    const char* pPassword = NULL;
    OStringBuffer aOutFile( 256 );
    PDFFileHdl aHdl = write_unzipFile;

    for( int nArg = 1; nArg < argc; nArg++ )
    {
        if( argv[nArg][0] == '-' )
        {
            if( ! rtl_str_compare( "-pw", argv[nArg] ) ||
                ! rtl_str_compare( "--password" , argv[nArg] ) )
            {
                if( nArg == argc-1 )
                {
                    fprintf( stderr, "no password given\n" );
                    return 1;
                }
                nArg++;
                pPassword = argv[nArg];
            }
            else if( ! rtl_str_compare( "-h", argv[nArg] ) ||
                ! rtl_str_compare( "--help", argv[nArg] ) )
            {
                printHelp( argv[0] );
                return 0;
            }
            else if( ! rtl_str_compare( "-a", argv[nArg] ) ||
                ! rtl_str_compare( "--extract-add-streams", argv[nArg] ) )
            {
                aHdl = write_addStreams;
            }
            else
            {
                fprintf( stderr, "unrecognized option \"%s\"\n",
                         argv[nArg] );
                printHelp( argv[0] );
                return 1;
            }
        }
        else if( pInFile == NULL )
            pInFile = argv[nArg];
        else if( pOutFile == NULL )
            pOutFile = argv[nArg];
    }
    if( ! pInFile )
    {
        fprintf( stderr, "no input file given\n" );
        return 10;
    }
    if( ! pOutFile )
    {
        OString aFile( pInFile );
        if( aFile.getLength() > 0 )
        {
            if( aFile.getLength() > 4 )
            {
                if( aFile.matchIgnoreAsciiCase( OString( ".pdf" ), aFile.getLength()-4 ) )
                    aOutFile.append( pInFile, aFile.getLength() - 4 );
                else
                    aOutFile.append( aFile );
            }
            aOutFile.append( "_unzip.pdf" );
            pOutFile = aOutFile.getStr();
        }
        else
        {
            fprintf( stderr, "no output file given\n" );
            return 11;
        }
    }

    return handleFile( pInFile, pOutFile, pPassword, aHdl );
}

