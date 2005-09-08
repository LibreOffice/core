/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: strhelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 16:35:58 $
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
#ifndef _PSPRINT_STRHELPER_HXX_
#define _PSPRINT_STRHELPER_HXX_

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

namespace psp {

String GetCommandLineToken( int, const String& );
ByteString GetCommandLineToken( int, const ByteString& );
// gets one token of a unix command line style string
// doublequote, singlequote and singleleftquote protect their respective
// contents

int GetCommandLineTokenCount( const String& );
int GetCommandLineTokenCount( const ByteString& );
// returns number of tokens (zero if empty or whitespace only)

String WhitespaceToSpace( const String&, BOOL bProtect = TRUE );
ByteString WhitespaceToSpace( const ByteString&, BOOL bProtect = TRUE );
// returns a string with multiple adjacent occurences of whitespace
// converted to a single space. if bProtect is TRUE (nonzero), then
// doublequote, singlequote and singleleftquote protect their respective
// contents

double StringToDouble( const String& rStr );
double StringToDouble( const ByteString& rStr );
// parses the first double in the string; decimal is '.' only

// fills a character buffer with the string representation of a double
// the buffer has to be long enough (e.g. 128 bytes)
// returns the string len
int getValueOfDouble( char* pBuffer, double f, int nPrecision = 0 );
// corresponding convenience functions
ByteString DoubleToByteString( double f, int nPrecision = 0 );
String DoubleToString( double f, int nPrecision = 0 );

} // namespace

#endif // _PSPRINT_STRHELPER_HXX_
