/*************************************************************************
 *
 *  $RCSfile: strhelper.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: pl $ $Date: 2001-05-08 11:45:33 $
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
