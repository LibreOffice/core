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


#include <stdio.h>
#include <sal/main.h>
#include <osl/file.h>
#include <osl/thread.h>
#include <rtl/alloc.h>
#include <rtl/ustring.hxx>
#include <rtl/strbuf.hxx>

#include "pdfparse.hxx"

using namespace pdfparse;

using ::rtl::OUString;
using ::rtl::OString;
using ::rtl::OStringBuffer;
using ::rtl::OStringToOUString;

void printHelp( const char* pExe )
{
    fprintf( stdout,
    "USAGE: %s [-h,--help]\n"
    "       %s [-pw, --password <password>] <inputfile> [<outputfile>]\n"
    "       %s <-a, --extract-add-streams> [-pw, --password <password>] <inputfile> [<outputfile>]\n"
    "       %s <-f, --extract-fonts> [-pw, --password <password>] <inputfile> [<outputfile>]\n"
    "       %s <-o, --extract-objects> <o0>[:<g0>][,<o1>[:g1][,...]] [-pw, --password <password>] <inputfile> [<outputfile>]\n"
    "  -h, --help: show help\n"
    "  -a, --extract-add-streams: extracts additional streams to outputfile_object\n"
    "      and prints the mimetype found to stdout\n"
    "  -f, --extract-fonts: extracts fonts (currently only type1 and truetype are supported\n"
    "  -o, --extract-objects: extracts object streams, the syntax of the argument is comma separated\n"
    "      object numbers, where object number and generation number are separated by \':\'\n"
    "      an omitted generation number defaults to 0\n"
    "  -pw, --password: use password for decryption\n"
    "\n"
    "note: -f, -a, -o and normal unzip operation are mutually exclusive\n"
    , pExe, pExe, pExe, pExe, pExe );
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
            boost::unordered_map<rtl::OString,PDFEntry*,rtl::OStringHash>::iterator add_stream;
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

int write_fonts( const char* i_pInFile, const char* i_pOutFile, PDFFile* i_pPDFFile )
{
    int nRet = 0;
    unsigned int nElements = i_pPDFFile->m_aSubElements.size();
    for( unsigned i = 0; i < nElements && nRet == 0; i++ )
    {
        // search FontDescriptors
        PDFObject* pObj = dynamic_cast<PDFObject*>(i_pPDFFile->m_aSubElements[i]);
        if( ! pObj )
            continue;
        PDFDict* pDict = dynamic_cast<PDFDict*>(pObj->m_pObject);
        if( ! pDict )
            continue;

        boost::unordered_map<rtl::OString,PDFEntry*,rtl::OStringHash>::iterator map_it =
                pDict->m_aMap.find( "Type" );
        if( map_it == pDict->m_aMap.end() )
            continue;

        PDFName* pName = dynamic_cast<PDFName*>(map_it->second);
        if( ! pName )
            continue;
        if( ! pName->m_aName.equals( "FontDescriptor" ) )
            continue;

        // the font name will be helpful, also there must be one in
        // a font descriptor
        map_it = pDict->m_aMap.find( "FontName" );
        if( map_it == pDict->m_aMap.end() )
            continue;
        pName = dynamic_cast<PDFName*>(map_it->second);
        if( ! pName )
            continue;
        rtl::OString aFontName( pName->m_aName );

        PDFObjectRef* pStreamRef = 0;
        const char* pFileType = NULL;
        // we have a font descriptor, try for a type 1 font
        map_it = pDict->m_aMap.find( "FontFile" );
        if( map_it != pDict->m_aMap.end() )
        {
            pStreamRef = dynamic_cast<PDFObjectRef*>(map_it->second);
            if( pStreamRef )
                pFileType = "pfa";
        }

        // perhaps it's a truetype file ?
        if( ! pStreamRef )
        {
            map_it  = pDict->m_aMap.find( "FontFile2" );
            if( map_it != pDict->m_aMap.end() )
            {
                pStreamRef = dynamic_cast<PDFObjectRef*>(map_it->second);
                if( pStreamRef )
                    pFileType = "ttf";
            }
        }

        if( ! pStreamRef )
            continue;

        PDFObject* pStream = i_pPDFFile->findObject( pStreamRef );
        if( ! pStream )
            continue;

        rtl::OStringBuffer aOutStream( i_pOutFile );
        aOutStream.append( "_font_" );
        aOutStream.append( sal_Int32(pStreamRef->m_nNumber) );
        aOutStream.append( "_" );
        aOutStream.append( sal_Int32(pStreamRef->m_nGeneration) );
        aOutStream.append( "_" );
        aOutStream.append( aFontName );
        if( pFileType )
        {
            aOutStream.append( "." );
            aOutStream.append( pFileType );
        }
        FileEmitContext aContext( aOutStream.getStr(), i_pInFile, i_pPDFFile );
        aContext.m_bDecrypt = i_pPDFFile->isEncrypted();
        pStream->writeStream( aContext, i_pPDFFile );
    }
    return nRet;
}

std::vector< std::pair< sal_Int32, sal_Int32 > > s_aEmitObjects;

int write_objects( const char* i_pInFile, const char* i_pOutFile, PDFFile* i_pPDFFile )
{
    int nRet = 0;
    unsigned int nElements = s_aEmitObjects.size();
    for( unsigned i = 0; i < nElements && nRet == 0; i++ )
    {
        sal_Int32 nObject     = s_aEmitObjects[i].first;
        sal_Int32 nGeneration = s_aEmitObjects[i].second;
        PDFObject* pStream = i_pPDFFile->findObject( nObject, nGeneration );
        if( ! pStream )
        {
            fprintf( stderr, "object %d %d not found !\n", (int)nObject, (int)nGeneration );
            continue;
        }

        rtl::OStringBuffer aOutStream( i_pOutFile );
        aOutStream.append( "_stream_" );
        aOutStream.append( nObject );
        aOutStream.append( "_" );
        aOutStream.append( nGeneration );
        FileEmitContext aContext( aOutStream.getStr(), i_pInFile, i_pPDFFile );
        aContext.m_bDecrypt = i_pPDFFile->isEncrypted();
        pStream->writeStream( aContext, i_pPDFFile );
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
            else if( ! rtl_str_compare( "-f", argv[nArg] ) ||
                ! rtl_str_compare( "--extract-fonts", argv[nArg] ) )
            {
                aHdl = write_fonts;
            }
            else if( ! rtl_str_compare( "-o", argv[nArg] ) ||
                ! rtl_str_compare( "--extract-objects", argv[nArg] ) )
            {
                aHdl = write_objects;
                nArg++;
                if( nArg < argc )
                {
                    rtl::OString aObjs( argv[nArg] );
                    sal_Int32 nIndex = 0;
                    while( nIndex != -1 )
                    {
                        rtl::OString aToken( aObjs.getToken( 0, ',', nIndex ) );
                        sal_Int32 nObject = 0;
                        sal_Int32 nGeneration = 0;
                        sal_Int32 nGenIndex = 0;
                        nObject = aToken.getToken( 0, ':', nGenIndex ).toInt32();
                        if( nGenIndex != -1 )
                            nGeneration = aToken.getToken( 0, ':', nGenIndex ).toInt32();
                        s_aEmitObjects.push_back( std::pair<sal_Int32,sal_Int32>(nObject,nGeneration) );
                    }
                }
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
