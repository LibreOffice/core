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
#include "precompiled_configmgr.hxx"
#include <iostream>
#include<osl/file.hxx>

#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Sequence.hxx>

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
