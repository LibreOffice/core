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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_filter.hxx"

/*
#include <tools/datetime.hxx>
*/
#include <osl/diagnose.h>
#include <rtl/crc.h>

#include "zip.hxx"
#include "zipfile.hxx"

using namespace rtl;

/** this struct describes one entry in a zip file */
struct ZipEntry
{
    OString name;			/* the name we used */
    sal_Int32 offset;		/* where the header starts */
    sal_Int32 endOffset;	/* where the file data ends */
    sal_Int32 crc;
    sal_Int32 modTime;		/* dos mod time & date */
    sal_Int32 fileLen;		/* file size, in bytes */
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

    e->modTime = ((aDate.Year - 1980) << 25) | (aDate.Month << 21) |	(aDate.Day << 16) |
    (aDate.Hours << 11) | (aDate.Minutes << 5) | (aDate.Seconds >> 1);

    e->fileLen = e->endOffset - e->offset - zf_lfhSIZE - e->name.getLength();

    if(!isError())
    {
        mnRC = mrFile.setPos( Pos_Absolut, e->offset );

        writeLong(zf_LFHSIGValue);								// magic number
        writeShort(zf_Vers(1, 0));								// extract version
        writeShort(0);											// flags
        writeShort(zf_compNone);								// compression method
        writeLong(e->modTime);									// file mod date & time
        writeLong(e->crc);										// file crc
        writeLong(e->fileLen);									// compressed size
        writeLong(e->fileLen);									// uncompressed size
        writeShort((sal_Int16) e->name.getLength());					// name length
        writeShort(0);											// extra length field

        if( !isError() )
        {
            sal_uInt64 nWritten;
            mnRC = mrFile.write( e->name.getStr(), e->name.getLength(), nWritten );	// file name
            OSL_ASSERT( nWritten == (sal_uInt64)e->name.getLength() );
            if( !isError() )
            {
                mnRC = mrFile.setPos( Pos_Absolut, e->endOffset );
            }
        }
    }
}

/* write a zipentry in the central dir to the zipfile */
void ZipFile::writeCentralDir(ZipEntry *e)
{
    writeLong(zf_CDHSIGValue);				// magic number
    writeShort(zf_Vers(1, 0));				// version made by
    writeShort(zf_Vers(1, 0));				// vers to extract
    writeShort(0);							// flags
    writeShort(zf_compNone);				// compression method
    writeLong(e->modTime);					// file mod time & date
    writeLong(e->crc);
    writeLong(e->fileLen);					// compressed file size
    writeLong(e->fileLen);					// uncompressed file size
    writeShort((sal_Int16) e->name.getLength());	// name length
    writeShort(0);							// extra field length 
    writeShort(0);							// file comment length
    writeShort(0);							// disk number start
    writeShort(0);							// internal file attributes
    writeLong(0);							// external file attributes
    writeLong(e->offset);					// offset w.r.t disk
    if( !isError() )
    {
        sal_uInt64 nWritten;
        mrFile.write( e->name.getStr(), e->name.getLength(), nWritten );	// file name
        OSL_ASSERT( nWritten == (sal_uInt64)e->name.getLength() );
    }
}

/* write the end of the central dir to the zipfile */
void ZipFile::writeEndCentralDir(sal_Int32 nCdOffset, sal_Int32 nCdSize)
{
    writeLong(zf_ECDSIGValue);		// magic number
    writeShort(0);					// disk num
    writeShort(0);					// disk with central dir 
    writeShort( static_cast< sal_Int16 >( maEntries.size() ) );	// number of file entries
    writeShort( static_cast< sal_Int16 >( maEntries.size() ) );	// number of file entries
    writeLong(nCdSize);				// central dir size
    writeLong(nCdOffset);
    writeShort(0);					// comment len
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

    OSL_ASSERT( 0 != rName.getLength() );

    if(0 == rName.getLength())
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
