/*************************************************************************
 *
 *  $RCSfile: textconv.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:03 $
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

#ifndef ENABLEUNICODE       // Keine Konvertierung bei Unicode

#ifndef _TEXTCONV_HXX
#define _TEXTCONV_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _SVARRAY_HXX //autogen
#include <svtools/svarray.hxx>
#endif

class OutputDevice;
class Font;

#define INVPOS  0xFFFF

// nStart und nEnd sind normale Char-Positionen, nicht wie Index vom PaM.

struct AttribRange
{
    USHORT nStart;
    USHORT nEnd;

    AttribRange()                       { nStart = 0; nEnd = 0; }
    AttribRange( USHORT nS, USHORT nE ) { nStart = nS; nEnd = nE; }

    USHORT  GetLen() const              { return nEnd-nStart+1; }
};

SV_DECL_VARARR( NonConvChars, AttribRange, 0, 4 );

class TextConverter
{
private:
    CharSet             eSourceCharSet;
    CharSet             eDestCharSet;
    NonConvChars        aNonConvblChars;
    NonConvChars        aNotConvertedChars;

#ifdef _TEXTCONV_CXX
    inline void         GetNonConvblRange( USHORT nRange, USHORT& nStart, USHORT& nEnd );
#endif

public:
                        TextConverter();
                        TextConverter( CharSet eSource, CharSet eDest );

    void                SetSourceCharSet( CharSet eSource ) { eSourceCharSet = eSource; }
    CharSet             GetSourceCharSet() const            { return eSourceCharSet; }

    void                SetDestCharSet( CharSet eDest )     { eDestCharSet = eDest; }
    CharSet             GetDestCharSet() const              { return eDestCharSet; }

    void                SetNonConvblChars( const NonConvChars& rNonConvbl )
                            {
                                aNonConvblChars.Remove( 0, aNonConvblChars.Count() );
                                aNonConvblChars.Insert( &rNonConvbl, 0 );
                            }
    NonConvChars&       GetNonConvblChars()
                            { return aNonConvblChars; }
    void                ResetNonConvblChars()
                            { aNonConvblChars.Remove( 0, aNonConvblChars.Count() ); }

    const NonConvChars& GetNotConvertedChars() const
                            { return aNotConvertedChars; }

    void                Convert( String& rText, BOOL bSmartReplace = TRUE );

    static char         GetReplaceChar( OutputDevice* pDev, char c, CharSet eCharSet, Font& rFont )
                            { return GetReplaceChar( pDev, String::ConvertToUnicode( c, eCharSet ), rFont ); }
    static char         GetReplaceChar( OutputDevice* pDev, sal_Unicode cUnicode, Font& rFont );
};

#ifdef _TEXTCONV_CXX

inline void TextConverter::GetNonConvblRange( USHORT nRange, USHORT& nStart, USHORT& nEnd )
{
    if ( aNonConvblChars.Count() > nRange )
    {
        nStart = aNonConvblChars[nRange].nStart;
        nEnd = aNonConvblChars[nRange].nEnd;
    }
    else
    {
        nStart = INVPOS;
        nEnd = 0;
    }
}

#endif // _TEXTCONV_CXX

#endif // _TEXTCONV_HXX

#endif // ENABLEUNICODE

