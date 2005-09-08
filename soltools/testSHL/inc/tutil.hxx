/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tutil.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 07:31:37 $
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
#ifndef _SOLTOOLS_TESTSHL_TUTIL_HXX_
#define _SOLTOOLS_TESTSHL_TUTIL_HXX__

#ifndef _OSL_FILE_HXX_
#include    <osl/file.hxx>
#endif

using namespace std;

#include <vector>

// <namespace_tstutl>
namespace tstutl {

sal_uInt32 getEntriesFromFile( sal_Char* fName, vector< sal_Char* >& entries );
::rtl::OUString cnvrtPth( ::rtl::OString sysPth );

// string copy, cat, len methods
sal_Char* cpy( sal_Char** dest, const sal_Char* src );
sal_Char* cat( const sal_Char* str1, const sal_Char* str2 );
sal_uInt32 ln( const sal_Char* str );

} // </namespace_tstutl>

#endif
