/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: char.cxx,v $
 * $Revision: 1.5 $
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
#include "precompiled_idl.hxx"

/****************** I N C L U D E S **************************************/
#include <ctype.h>
#include <string.h>

#ifndef _TABLE_HXX //autogen
#include <tools/table.hxx>
#endif

#include <char.hxx>

/****************** D A T E N ********************************************/
static unsigned char EqualTab[ 256 ] = {
  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,
 10,  11,  12,  13,  14,  15,  16,  17,  18,  19,
 20,  21,  22,  23,  24,  25,  26,  27,  28,  29,
 30,  31,  32,  33,  34,  35,  36,  37,  38,  39,
 40,  41,  42,  43,  44,  45,  46,  47,  48,  49,
 50,  51,  52,  53,  54,  55,  56,  57,  58,  59,
 60,  61,  62,  63,  64,  65,  66,  67,  68,  69,
 70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
 80,  81,  82,  83,  84,  85,  86,  87,  88,  89,
 90,  91,  92,  93,  94,  95,  96,  97,  98,  99,
100, 101, 102, 103, 104, 105, 106, 107, 108, 109,
110, 111, 112, 113, 114, 115, 116, 117, 118, 119,
120, 121, 122, 123, 124, 125, 126, 127, 128, 129,
130, 131, 132, 133, 134, 135, 136, 137, 138, 139,
140, 141, 142, 143, 144, 145, 146, 147, 148, 149,
150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
160, 161, 162, 163, 164, 165, 166, 167, 168, 169,
170, 171, 172, 173, 174, 175, 176, 177, 178, 179,
180, 181, 182, 183, 184, 185, 186, 187, 188, 189,
190, 191, 192, 193, 194, 195, 196, 197, 198, 199,
200, 201, 202, 203, 204, 205, 206, 207, 208, 209,
210, 211, 212, 213, 214, 215, 216, 217, 218, 219,
220, 221, 222, 223, 224, 225, 226, 227, 228, 229,
230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
240, 241, 242, 243, 244, 245, 246, 247, 248, 249,
250, 251, 252, 253, 254, 255 };


/*************************************************************************
|*
|*    RscChar::GetTable()
|*
|*    Beschreibung
|*    Ersterstellung    MM 08.08.91
|*    Letzte Aenderung  MM 08.08.91
|*
*************************************************************************/
Table *         pCharTable  = NULL;
unsigned char * pChange     = EqualTab;
char * SvChar::GetTable( CharSet nSource , CharSet nDest )
{
    if( nSource == nDest )
        return (char *)EqualTab;

    if( !pCharTable )
        pCharTable = new Table();

    BYTE * pSet;
    pSet = (BYTE *)pCharTable->Get( ((ULONG)nSource << 16) + (ULONG)nDest );

    if( !pSet )
    {
        pSet = new BYTE[ 256 ];
        memcpy( pSet, EqualTab, sizeof( EqualTab ) );
        for( USHORT i = 128; i < 256; i++ )
        {
            char c = ByteString::Convert( pSet[i], nSource, nDest );
            if( c )
                pSet[ i ] = (BYTE)c;
        }
        pCharTable->Insert( ((ULONG)nSource << 16) + (ULONG)nDest, pSet );
    }

    return (char *)pSet;
};


/*************************************************************************
|*
|*    RscChar::MakeChar()
|*
|*    Beschreibung      Der ByteString wird nach C-Konvention umgesetzt
|*    Ersterstellung    MM 20.03.91
|*    Letzte Aenderung  MM 20.03.91
|*
*************************************************************************/
BOOL SvChar::MakeChar( char * pChar, char ** ppStr )
{
    char    c;
    BOOL    bDoInc = TRUE; // Noch einmal erhoehen

    if( **ppStr == '\\' )
    {
        ++*ppStr;
        switch( **ppStr )
        {
            case 'a':
                c = '\a';
                break;
            case 'b':
                c = '\b';
                break;
            case 'f':
                c = '\f';
                break;
            case 'n':
                c = '\n';
                break;
            case 'r':
                c = '\r';
                break;
            case 't':
                c = '\t';
                break;
            case 'v':
                c = '\v';
                break;
            case '\\':
                c = '\\';
                break;
            case '?':
                c = '\?';
                break;
            case '\'':
                c = '\'';
                break;
            case '\"':
                c = '\"';
                break;
            default:
            {
                if( '0' <= **ppStr && '7' >= **ppStr )
                {
                    bDoInc = FALSE;

                    USHORT  nChar = 0;
                    USHORT  i = 0;
                    while( '0' <= **ppStr && '7' >= **ppStr && i != 3 )
                    {
                        nChar = nChar * 8 + (BYTE)**ppStr - (BYTE)'0';
                        ++*ppStr;
                        i++;
                    }
                    if( nChar > 255 )
                        // Wert zu gross, oder kein 3 Ziffern
                        return( FALSE );
                    c = (char)nChar;
                }
                else if( 'x' == **ppStr )
                {
                    bDoInc = FALSE;

                    USHORT  nChar = 0;
                    USHORT  i = 0;
                    ++*ppStr;
                    while( isxdigit( **ppStr ) && i != 2 )
                    {
                        if( isdigit( **ppStr ) )
                            nChar = nChar * 16 + (BYTE)**ppStr - (BYTE)'0';
                        else if( isupper( **ppStr ) )
                            nChar = nChar * 16 + (BYTE)**ppStr - (BYTE)'A' +10;
                        else
                            nChar = nChar * 16 + (BYTE)**ppStr - (BYTE)'a' +10;
                        ++*ppStr;
                        i++;
                    }
                    c = (char)nChar;
                }
                else
                    c = **ppStr;
                    //c = pChange [ (unsigned char)**ppStr ];
            };
        }
    }
    else
        c = **ppStr;
        //c = pChange [ (unsigned char)**ppStr ];

    if( **ppStr && bDoInc ) ++*ppStr;
    *pChar = c;
    return( TRUE );
};

