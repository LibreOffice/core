/*************************************************************************
 *
 *  $RCSfile: tutil.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: sz $ $Date: 2001-04-12 10:54:54 $
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
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "tutil.hxx"

// <namespace_tstutl>
namespace tstutl {

// _getcwd hack is deprecated as soon as normalizePath works as intend
#include <direct.h>                         // _getcwd

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
            OString curPth(_getcwd(buffer,256));
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
    if ( inFile.open( OpenFlag_Read ) == ::osl::FileBase::E_None) {
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

