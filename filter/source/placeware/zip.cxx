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

#include <osl/diagnose.h>
#include <rtl/crc.h>

#include "zip.hxx"
#include "zipfile.hxx"

using ::rtl::OString;
/** this struct describes one entry in a zip file */
struct ZipEntry
{
    OString name;           /* the name we used */
    sal_Int32 offset;       /* where the header starts */
    sal_Int32 endOffset;    /* where the file data ends */
    sal_Int32 crc;
    sal_Int32 modTime;      /* dos mod time & date */
    sal_Int32 fileLen;      /* file size, in bytes */
};

/** put one byte inside this stream */
static osl::File::RC putC( unsigned char c, osl::File& rFile )
{
    sal_uInt64 nBytesWritten;
    osl::File::RC nRC = rFile.write( &c, 1, nBytesWritten );

    OSL_ASSERT( nBytesWritten == 1 );
    return nRC;
}

/** write a short to the ZipFile */
void ZipFile::writeShort( sal_Int16 s)
{
    if( !isError() )
    {
        mnRC = putC( static_cast< unsigned char >( s & 0xff ), mrFile );
        if( !isError() )
            mnRC = putC( static_cast< unsigned char >( (s >> 8) & 0xff ), mrFile );
    }
}

/** write a long to the ZipFile */

void ZipFile::writeLong( sal_Int32 l )
{
    if( !isError() )
    {
        mnRC = putC( static_cast< unsigned char >( l & 0xff ), mrFile);
        if( !isError() )
        {
            mnRC = putC( static_cast< unsigned char >( (l >> 8) & 0xff ), mrFile);
            if( !isError() )
            {
                mnRC = putC( static_cast< unsigned char >( (l >> 16) & 0xff ), mrFile);
                if( !isError() )
                {
                    mnRC = putC( static_cast< unsigned char >( (l >> 24) & 0xff ), mrFile);
                }
            }
        }
    }
}

/** copy the zipentries file to the zipfile and updates the crc of that zipentry */
void ZipFile::copyAndCRC(ZipEntry *e, osl::File& rFile)
{
    char buf[2048];
    sal_uInt64 n, nWritten;

    e->crc = rtl_crc32( 0, 0L, 0 );

    while( !isError() )
    {
        mnRC = rFile.read( buf, sizeof(buf), n );
        if(n == 0)
            break;

        if( !isError() )
        {
            sal_uInt32 nTemp = static_cast<sal_uInt32>(n);
            e->crc = rtl_crc32( e->crc, (const void *) buf, nTemp );
            mnRC = mrFile.write( buf, n, nWritten );
            OSL_ASSERT( n == nWritten );
        }
    }

    if( !isError() )
    {
        sal_uInt64 nPosition = 0;
        mnRC = mrFile.getPos( nPosition );
        if( !isError() )
        {
            e->endOffset = static_cast< sal_Int32 >( nPosition );
        }
    }
}

/** write a yet empty local header for a zipentry to the zipfile */
void ZipFile::writeDummyLocalHeader(ZipEntry *e)
{
    sal_Int32 len = zf_lfhSIZE + e->name.getLength();
    sal_Int32 i;

    sal_uInt64 nPosition = 0;
    mnRC = mrFile.getPos( nPosition );
    if( !isError() )
    {
        e->offset = static_cast< sal_Int32 >( nPosition );

        for (i = 0; (i < len) && !isError(); ++i)
            mnRC = putC(0, mrFile);
    }
}

/** write the local header for a zipentry to the zipfile */
void ZipFile::writeLocalHeader(ZipEntry *e)
{
    TimeValue aTime;
    osl_getSystemTime( &aTime );

    oslDateTime aDate;
    osl_getDateTimeFromTimeValue( &aTime, &aDate );

    e->modTime = ((aDate.Year - 1980) << 25) | (aDate.Month << 21) |    (aDate.Day << 16) |
    (aDate.Hours << 11) | (aDate.Minutes << 5) | (aDate.Seconds >> 1);

    e->fileLen = e->endOffset - e->offset - zf_lfhSIZE - e->name.getLength();

    if(!isError())
    {
        mnRC = mrFile.setPos( osl_Pos_Absolut, e->offset );

        writeLong(zf_LFHSIGValue);                              // magic number
        writeShort(zf_Vers(1, 0));                              // extract version
        writeShort(0);                                          // flags
        writeShort(zf_compNone);                                // compression method
        writeLong(e->modTime);                                  // file mod date & time
        writeLong(e->crc);                                      // file crc
        writeLong(e->fileLen);                                  // compressed size
        writeLong(e->fileLen);                                  // uncompressed size
        writeShort((sal_Int16) e->name.getLength());                    // name length
        writeShort(0);                                          // extra length field

        if( !isError() )
        {
            sal_uInt64 nWritten;
            mnRC = mrFile.write( e->name.getStr(), e->name.getLength(), nWritten ); // file name
            OSL_ASSERT( nWritten == (sal_uInt64)e->name.getLength() );
            if( !isError() )
            {
                mnRC = mrFile.setPos( osl_Pos_Absolut, e->endOffset );
            }
        }
    }
}

/* write a zipentry in the central dir to the zipfile */
void ZipFile::writeCentralDir(ZipEntry *e)
{
    writeLong(zf_CDHSIGValue);              // magic number
    writeShort(zf_Vers(1, 0));              // version made by
    writeShort(zf_Vers(1, 0));              // vers to extract
    writeShort(0);                          // flags
    writeShort(zf_compNone);                // compression method
    writeLong(e->modTime);                  // file mod time & date
    writeLong(e->crc);
    writeLong(e->fileLen);                  // compressed file size
    writeLong(e->fileLen);                  // uncompressed file size
    writeShort((sal_Int16) e->name.getLength());    // name length
    writeShort(0);                          // extra field length
    writeShort(0);                          // file comment length
    writeShort(0);                          // disk number start
    writeShort(0);                          // internal file attributes
    writeLong(0);                           // external file attributes
    writeLong(e->offset);                   // offset w.r.t disk
    if( !isError() )
    {
        sal_uInt64 nWritten;
        mrFile.write( e->name.getStr(), e->name.getLength(), nWritten );    // file name
        OSL_ASSERT( nWritten == (sal_uInt64)e->name.getLength() );
    }
}

/* write the end of the central dir to the zipfile */
void ZipFile::writeEndCentralDir(sal_Int32 nCdOffset, sal_Int32 nCdSize)
{
    writeLong(zf_ECDSIGValue);      // magic number
    writeShort(0);                  // disk num
    writeShort(0);                  // disk with central dir
    writeShort( static_cast< sal_Int16 >( maEntries.size() ) ); // number of file entries
    writeShort( static_cast< sal_Int16 >( maEntries.size() ) ); // number of file entries
    writeLong(nCdSize);             // central dir size
    writeLong(nCdOffset);
    writeShort(0);                  // comment len
}


/****************************************************************
 * The exported functions
 ****************************************************************/

/* Create a zip file for writing, return a handle for it.
 * RETURNS: A new zip-file output object, or NULL if it failed, in
 *   which case *errMsgBuffer will contain an error message string. */
ZipFile::ZipFile(osl::File& rFile )
: mrFile( rFile ), mbOpen( true ), mnRC( osl::File::E_None )
{
}

ZipFile::~ZipFile()
{
    if( mbOpen )
        close();
}

/* Add a file to this zip with the given name.
 * RETURNS: true if successful, else false. If false, the caller should
 *   call zip_Close() and delete the bum zip file.
*/
bool ZipFile::addFile( osl::File& rFile, const OString& rName )
{
    OSL_ASSERT( mbOpen );

    if( !mbOpen )
        return false;

    OSL_ASSERT( !rName.isEmpty() );

    if(rName.isEmpty())
        return false;

    mnRC = rFile.open( osl_File_OpenFlag_Read );

    if( !isError() )
    {
        ZipEntry *e = new ZipEntry;
        e->name = rName;
        maEntries.push_back(e);

        writeDummyLocalHeader(e);
        if( !isError() )
        {
            copyAndCRC(e, rFile);
            if(!isError())
            {
                writeLocalHeader(e);
            }
        }

        rFile.close();
    }

    return !isError();
}

/* Finish up the zip file, close it, and deallocate the zip file object.
 * RETURNS: true if successful, else false.
*/
bool ZipFile::close()
{
    OSL_ASSERT( mbOpen );

    if( !mbOpen )
        return false;

    if( !isError() )
    {
        sal_uInt64 nCdOffset;
        mrFile.getPos( nCdOffset );

        std::vector< ZipEntry* >::iterator aIter( maEntries.begin() );
        while((aIter != maEntries.end()) && !isError())
        {
            writeCentralDir( (*aIter++) );
        }

        if( !isError() )
        {
            sal_uInt64 nCdSize;
            mrFile.getPos( nCdSize );

            nCdSize -= nCdOffset;

            if( !isError() )
            {
                writeEndCentralDir(static_cast<sal_Int32>(nCdOffset), static_cast<sal_Int32>(nCdSize));
            }
        }
    }

    std::vector< ZipEntry* >::iterator aIter( maEntries.begin() );
    while( aIter != maEntries.end() )
    {
        delete (*aIter++);
    }

    mbOpen = false;

    return !isError();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
