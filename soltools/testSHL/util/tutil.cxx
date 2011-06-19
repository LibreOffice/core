/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_soltools.hxx"
#include "tutil.hxx"

// <namespace_tstutl>
namespace tstutl {

// getcwd hack is deprecated as soon as normalizePath works as intend
#ifdef WNT
#define _getcwd getcwd
#include <direct.h>                         // _getcwd
#else
#include <unistd.h>                         // getcwd
#endif

// <function_cnvrtPth>
::rtl::OUString cnvrtPth( ::rtl::OString sysPth ) {

    using ::osl::FileBase;
    using ::rtl::OUString;
    using ::rtl::OString;

    ::rtl::OUString ret;
    sysPth = sysPth.replace( '\\','/' );
    OUString pth( OUString::createFromAscii( sysPth.getStr() ) );

    if ( sysPth.indexOf("..") != -1 ) {

        // <hack> for osl_normalizePath() can't handle relatives
            char buffer[256];
            OString curPth(getcwd(buffer,256));
        // </hack>
        OUString nrmCurPth;
        FileBase::normalizePath( OUString::createFromAscii( curPth ) ,
                                                                nrmCurPth );
        FileBase::getAbsolutePath( nrmCurPth, pth, ret );
    }
    else  {
        FileBase::normalizePath( pth, ret );
    }
    return ret;

} // </function_cnvrtPth>

// <function_getEntriesFromFile>
sal_uInt32 getEntriesFromFile( sal_Char* fName,
                                            vector< sal_Char* >& entries ) {

    ::osl::File inFile( cnvrtPth( fName ) );
    if ( inFile.open( osl_File_OpenFlag_Read ) == ::osl::FileBase::E_None) {
        ::rtl::ByteSequence byteSeq;
        inFile.readLine( byteSeq );
        while ( byteSeq.getLength() ) {
            sal_uInt32 len = byteSeq.getLength();
            sal_uInt32 i;
            sal_Char* pEnt = new sal_Char[ len+1 ];
            sal_Char* bsPtr = (sal_Char*)byteSeq.getArray();
            for ( i=0; i<len; i++ ) {
                pEnt[i] = bsPtr[i];
            }
            pEnt[len] = '\0';
            entries.push_back( pEnt );

            inFile.readLine( byteSeq );
        }
    }
    return ( entries.size() );

} // </function_getEntriesFromFile>

// <function_cpy>
sal_Char* cpy( sal_Char** dest, const sal_Char* src ) {

    *dest = new sal_Char[ ln(src)+1 ];
    // set pointer
    sal_Char* pdest = *dest;
    const sal_Char* psrc = src;

    // copy string by char
    while( *pdest++ = *psrc++ );

    return ( *dest );

} // </function_cpy>

// <function_cat>
sal_Char* cat( const sal_Char* str1, const sal_Char* str2 ) {

    // allocate memory for destination string
    sal_Char* dest = new sal_Char[ ln(str1)+ln(str2)+1 ];

    // set pointers
    sal_Char* pdest = dest;
    const sal_Char* psrc = str1;

    // copy string1 by char to dest
    while( *pdest++ = *psrc++ );
    pdest--;
    psrc = str2;
    while( *pdest++ = *psrc++ );

    return ( dest );

} // </function_cat>

// <function_ln>
sal_uInt32 ln( const sal_Char* str ) {

    sal_uInt32 len = 0;
    const sal_Char* ptr = str;

    if( ptr ) {
        while( *ptr++ ) len++;
    }

    return(len);
} // <function_ln>

} // </namespace_tstutl>

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
