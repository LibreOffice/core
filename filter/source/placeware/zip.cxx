/*************************************************************************
 *
 *  $RCSfile: zip.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: cl $ $Date: 2002-10-02 15:43:44 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): Christian Lippka (christian.lippka@sun.com)
 *
 *
 ************************************************************************/

#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _DATETIME_HXX
#include <tools/datetime.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _RTL_CRC_H_
#include <rtl/crc.h>
#endif

#include "zip.hxx"
#include "zipfile.hxx"

/** this struct describes one entry in a zip file */
struct ZipEntry
{
    ByteString name;        /* the name we used */
    sal_Int32 offset;       /* where the header starts */
    sal_Int32 endOffset;    /* where the file data ends */
    sal_Int32 crc;
    sal_Int32 modTime;      /* dos mod time & date */
    sal_Int32 fileLen;      /* file size, in bytes */
};

/** put one byte inside this stream */
static void putc( unsigned char c, SvStream& rStream )
{
    rStream << c;
}

/** write a short to the ZipFile */
void ZipFile::writeShort( sal_Int16 s)
{
    putc( s & 0xff, mrStream );
    putc( (s >> 8) & 0xff, mrStream );
}

/** write a long to the ZipFile */

void ZipFile::writeLong( sal_Int32 l )
{
    putc(l & 0xff, mrStream);
    putc((l >> 8) & 0xff, mrStream);
    putc((l >> 16) & 0xff, mrStream);
    putc((l >> 24) & 0xff, mrStream);
}

/** copy the zipentries file to the zipfile and updates the crc of that zipentry */
void ZipFile::copyAndCRC(ZipEntry *e, SvStream& rFile)
{
    char buf[2048];
    sal_uInt32 n;

    e->crc = rtl_crc32( 0, 0L, 0 );
    while((0 == rFile.GetError()) && (0 == mrStream.GetError()))
    {
        n = rFile.Read( buf, sizeof(buf) );
        if (n <= 0)
            break;
        e->crc = rtl_crc32( e->crc, (const void *) buf, n );
        mrStream.Write( buf, n );
    }

    if( 0 == mrStream.GetError() )
        e->endOffset = mrStream.Tell();
}

/** write a yet empty local header for a zipentry to the zipfile */
void ZipFile::writeDummyLocalHeader(ZipEntry *e)
{
    int len = zf_lfhSIZE + e->name.Len();
    int i;

    e->offset = mrStream.Tell();
    for (i = 0; i < len; ++i)
        putc(0, mrStream);
}

/** write the local header for a zipentry to the zipfile */
void ZipFile::writeLocalHeader(ZipEntry *e)
{
    DateTime aDate;

    e->modTime = ((aDate.GetYear() - 1980) << 25) | (aDate.GetMonth() << 21) |  (aDate.GetDay() << 16) |
    (aDate.GetHour() << 11) | (aDate.GetMin() << 5) | (aDate.GetSec() >> 1);

    e->fileLen = e->endOffset - e->offset - zf_lfhSIZE - e->name.Len();
    mrStream.Seek( e->offset );
    writeLong(zf_LFHSIGValue);                              // magic number
    writeShort(zf_Vers(1, 0));                              // extract version
    writeShort(0);                                          // flags
    writeShort(zf_compNone);                                // compression method
    writeLong(e->modTime);                                  // file mod date & time
    writeLong(e->crc);                                      // file crc
    writeLong(e->fileLen);                                  // compressed size
    writeLong(e->fileLen);                                  // uncompressed size
    writeShort((sal_Int16) e->name.Len());                  // name length
    writeShort(0);                                          // extra length field
    mrStream.Write( e->name.GetBuffer(), e->name.Len() );   // file name
    mrStream.Seek( e->endOffset );
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
    writeShort((sal_Int16) e->name.Len());  // name length
    writeShort(0);                          // extra field length
    writeShort(0);                          // file comment length
    writeShort(0);                          // disk number start
    writeShort(0);                          // internal file attributes
    writeLong(0);                           // external file attributes
    writeLong(e->offset);                   // offset w.r.t disk
    mrStream.Write( e->name.GetBuffer(), e->name.Len() );   // file name
}

/* write the end of the central dir to the zipfile */
void ZipFile::writeEndCentralDir(sal_Int32 nCdOffset, sal_Int32 nCdSize)
{
    writeLong(zf_ECDSIGValue);      // magic number
    writeShort(0);                  // disk num
    writeShort(0);                  // disk with central dir
    writeShort(maEntries.size());   // number of file entries
    writeShort(maEntries.size());   // number of file entries
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
ZipFile::ZipFile(SvStream& rFile )
: mrStream( rFile ), mbOpen( true )
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
bool ZipFile::addFile( SvStream& rFile, const ByteString& rName )
{
    DBG_ASSERT( mbOpen, "zipfile must be open for addFile()" );

    if( !mbOpen )
        return false;

    DBG_ASSERT( 0 != rName.Len(), "can't zip files without a name" );

    if(0 == rName.Len())
        return false;

    ZipEntry *e = new ZipEntry;
    e->name = rName;
    maEntries.push_back(e);

    writeDummyLocalHeader(e);
    if( 0 == mrStream.GetError() )
    {
        copyAndCRC(e, rFile);
        if((0 == mrStream.GetError() && (0 == rFile.GetError())))
        {
            writeLocalHeader(e);
        }
    }

    return (0 == mrStream.GetError() && (0 == rFile.GetError()));
}

/* Finish up the zip file, close it, and deallocate the zip file object.
 * RETURNS: true if successful, else false.
*/
bool ZipFile::close()
{
    DBG_ASSERT( mbOpen, "zipfile must be open for close()" );

    if( !mbOpen )
        return false;

    if( 0 == mrStream.GetError() )
    {
        sal_Int32 nCdOffset = mrStream.Tell();

        std::vector< ZipEntry* >::iterator aIter( maEntries.begin() );
        while((aIter != maEntries.end()) && (0 == mrStream.GetError()))
        {
            writeCentralDir( (*aIter++) );
        }

        if( 0 == mrStream.GetError() )
        {
            sal_Int32 nCdSize = mrStream.Tell() - nCdOffset;

            if( 0 == mrStream.GetError() )
            {
                writeEndCentralDir(nCdOffset, nCdSize);
            }
        }
    }

    std::vector< ZipEntry* >::iterator aIter( maEntries.begin() );
    while( aIter != maEntries.end() )
    {
        delete (*aIter++);
    }

    mbOpen = false;

    return 0 == mrStream.GetError();
}
