/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: filetest.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 04:49:08 $
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
#include <iostream>

#ifndef _OSL_FILE_HXX_
#include<osl/file.hxx>
#endif

#include <rtl/ustring.hxx>

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

// -----------------------------------------------------------------------------
// --------------------------------- namespaces ---------------------------------
// -----------------------------------------------------------------------------
using namespace com::sun::star::uno;

using ::rtl::OUString;
using ::osl::File;
// -----------------------------------------------------------------------------
// ---------------------------------- defines ----------------------------------
// -----------------------------------------------------------------------------
#define ASCII(x) OUString::createFromAscii(x)


// -----------------------------------------------------------------------------
// ------------------------------------ main ------------------------------------
// -----------------------------------------------------------------------------

/*
OUString operator+(const OUString &a, const OUString &b)
{
    OUString c = a;
    c += b;
    return c;
}
*/

#if (defined UNX) || (defined OS2)
int main( int argc, char * argv[] )
#else
int _cdecl main( int argc, char * argv[] )
#endif
{
    OUString aPath = ASCII("f:/local/SRC598/configmgr/workben/local_io");
    OUString aFilename = ASCII("com.sun.star.office.Setup");
    OUString aExtension = ASCII("xml");

    OUString aFullname = aPath + ASCII("/") + aFilename + ASCII(".") + aExtension;

    // Filename convertieren
    OUString aURL;
    File aConvert(ASCII(""));
    aConvert.normalizePath(aFullname, aURL);

    // File oeffnen
    File aFile(aURL);
    aFile.open(osl_File_OpenFlag_Read);

    sal_uInt64 nBytesRead;
    Sequence< sal_Int8 > aBufferSeq(2000);
    sal_Int8 *pBuff = aBufferSeq.getArray();
    aFile.read(pBuff, 2000, nBytesRead);

    aFile.close();
    return 0;
}
