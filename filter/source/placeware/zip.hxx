/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: zip.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2006-12-01 14:29:51 $
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

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

#include <vector>

struct ZipEntry;

class ZipFile
{
public:
    ZipFile( osl::File& rFile );
    ~ZipFile();

    bool addFile( osl::File& rFile, const rtl::OString& rName );
    bool close();

private:
    void writeShort( sal_Int16 s);
    void writeLong( sal_Int32 l );

    void copyAndCRC( ZipEntry *e, osl::File& rFile );
    void writeDummyLocalHeader(ZipEntry *e);
    void writeLocalHeader(ZipEntry *e);
    void writeCentralDir(ZipEntry *e);
    void writeEndCentralDir(sal_Int32 nCdOffset, sal_Int32 nCdSize);

private:
    bool isError() const { return osl::File::E_None != mnRC; }

    osl::File& mrFile;              /* file we're writing to */
    bool mbOpen;
    osl::File::RC mnRC;
    std::vector<ZipEntry*> maEntries;
};
