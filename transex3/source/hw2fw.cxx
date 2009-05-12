/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: hw2fw.cxx,v $
 * $Revision: 1.4 $
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
#include "precompiled_transex3.hxx"
#include <tools/string.hxx>

struct hw_pair
{
    sal_Unicode         nFrom;
    sal_Unicode         nTo;
};

#define MAKE_PAIR(a,b) { a, b }

static struct hw_pair aHWPairs[] =
{
  MAKE_PAIR( 0xFF65, 0x30FB ),  // HALFWIDTH KATAKANA MIDDLE DOT --> KATAKANA MIDDLE DOT
  MAKE_PAIR( 0xFF66, 0x30F2 ),  // HALFWIDTH KATAKANA LETTER WO --> KATAKANA LETTER WO
  MAKE_PAIR( 0xFF67, 0x30A1 ),  // HALFWIDTH KATAKANA LETTER SMALL A --> KATAKANA LETTER SMALL A
  MAKE_PAIR( 0xFF68, 0x30A3 ),  // HALFWIDTH KATAKANA LETTER SMALL I --> KATAKANA LETTER SMALL I
  MAKE_PAIR( 0xFF69, 0x30A5 ),  // HALFWIDTH KATAKANA LETTER SMALL U --> KATAKANA LETTER SMALL U
  MAKE_PAIR( 0xFF6A, 0x30A7 ),  // HALFWIDTH KATAKANA LETTER SMALL E --> KATAKANA LETTER SMALL E
  MAKE_PAIR( 0xFF6B, 0x30A9 ),  // HALFWIDTH KATAKANA LETTER SMALL O --> KATAKANA LETTER SMALL O
  MAKE_PAIR( 0xFF6C, 0x30E3 ),  // HALFWIDTH KATAKANA LETTER SMALL YA --> KATAKANA LETTER SMALL YA
  MAKE_PAIR( 0xFF6D, 0x30E5 ),  // HALFWIDTH KATAKANA LETTER SMALL YU --> KATAKANA LETTER SMALL YU
  MAKE_PAIR( 0xFF6E, 0x30E7 ),  // HALFWIDTH KATAKANA LETTER SMALL YO --> KATAKANA LETTER SMALL YO
  MAKE_PAIR( 0xFF6F, 0x30C3 ),  // HALFWIDTH KATAKANA LETTER SMALL TU --> KATAKANA LETTER SMALL TU
  MAKE_PAIR( 0xFF70, 0x30FC ),  // HALFWIDTH KATAKANA-HIRAGANA PROLONGED SOUND MARK --> KATAKANA-HIRAGANA PROLONGED SOUND MARK
  MAKE_PAIR( 0xFF71, 0x30A2 ),  // HALFWIDTH KATAKANA LETTER A --> KATAKANA LETTER A
  MAKE_PAIR( 0xFF72, 0x30A4 ),  // HALFWIDTH KATAKANA LETTER I --> KATAKANA LETTER I
  MAKE_PAIR( 0xFF73, 0x30A6 ),  // HALFWIDTH KATAKANA LETTER U --> KATAKANA LETTER U
  MAKE_PAIR( 0xFF74, 0x30A8 ),  // HALFWIDTH KATAKANA LETTER E --> KATAKANA LETTER E
  MAKE_PAIR( 0xFF75, 0x30AA ),  // HALFWIDTH KATAKANA LETTER O --> KATAKANA LETTER O
  MAKE_PAIR( 0xFF76, 0x30AB ),  // HALFWIDTH KATAKANA LETTER KA --> KATAKANA LETTER KA
  MAKE_PAIR( 0xFF77, 0x30AD ),  // HALFWIDTH KATAKANA LETTER KI --> KATAKANA LETTER KI
  MAKE_PAIR( 0xFF78, 0x30AF ),  // HALFWIDTH KATAKANA LETTER KU --> KATAKANA LETTER KU
  MAKE_PAIR( 0xFF79, 0x30B1 ),  // HALFWIDTH KATAKANA LETTER KE --> KATAKANA LETTER KE
  MAKE_PAIR( 0xFF7A, 0x30B3 ),  // HALFWIDTH KATAKANA LETTER KO --> KATAKANA LETTER KO
  MAKE_PAIR( 0xFF7B, 0x30B5 ),  // HALFWIDTH KATAKANA LETTER SA --> KATAKANA LETTER SA
  MAKE_PAIR( 0xFF7C, 0x30B7 ),  // HALFWIDTH KATAKANA LETTER SI --> KATAKANA LETTER SI
  MAKE_PAIR( 0xFF7D, 0x30B9 ),  // HALFWIDTH KATAKANA LETTER SU --> KATAKANA LETTER SU
  MAKE_PAIR( 0xFF7E, 0x30BB ),  // HALFWIDTH KATAKANA LETTER SE --> KATAKANA LETTER SE
  MAKE_PAIR( 0xFF7F, 0x30BD ),  // HALFWIDTH KATAKANA LETTER SO --> KATAKANA LETTER SO
  MAKE_PAIR( 0xFF80, 0x30BF ),  // HALFWIDTH KATAKANA LETTER TA --> KATAKANA LETTER TA
  MAKE_PAIR( 0xFF81, 0x30C1 ),  // HALFWIDTH KATAKANA LETTER TI --> KATAKANA LETTER TI
  MAKE_PAIR( 0xFF82, 0x30C4 ),  // HALFWIDTH KATAKANA LETTER TU --> KATAKANA LETTER TU
  MAKE_PAIR( 0xFF83, 0x30C6 ),  // HALFWIDTH KATAKANA LETTER TE --> KATAKANA LETTER TE
  MAKE_PAIR( 0xFF84, 0x30C8 ),  // HALFWIDTH KATAKANA LETTER TO --> KATAKANA LETTER TO
  MAKE_PAIR( 0xFF85, 0x30CA ),  // HALFWIDTH KATAKANA LETTER NA --> KATAKANA LETTER NA
  MAKE_PAIR( 0xFF86, 0x30CB ),  // HALFWIDTH KATAKANA LETTER NI --> KATAKANA LETTER NI
  MAKE_PAIR( 0xFF87, 0x30CC ),  // HALFWIDTH KATAKANA LETTER NU --> KATAKANA LETTER NU
  MAKE_PAIR( 0xFF88, 0x30CD ),  // HALFWIDTH KATAKANA LETTER NE --> KATAKANA LETTER NE
  MAKE_PAIR( 0xFF89, 0x30CE ),  // HALFWIDTH KATAKANA LETTER NO --> KATAKANA LETTER NO
  MAKE_PAIR( 0xFF8A, 0x30CF ),  // HALFWIDTH KATAKANA LETTER HA --> KATAKANA LETTER HA
  MAKE_PAIR( 0xFF8B, 0x30D2 ),  // HALFWIDTH KATAKANA LETTER HI --> KATAKANA LETTER HI
  MAKE_PAIR( 0xFF8C, 0x30D5 ),  // HALFWIDTH KATAKANA LETTER HU --> KATAKANA LETTER HU
  MAKE_PAIR( 0xFF8D, 0x30D8 ),  // HALFWIDTH KATAKANA LETTER HE --> KATAKANA LETTER HE
  MAKE_PAIR( 0xFF8E, 0x30DB ),  // HALFWIDTH KATAKANA LETTER HO --> KATAKANA LETTER HO
  MAKE_PAIR( 0xFF8F, 0x30DE ),  // HALFWIDTH KATAKANA LETTER MA --> KATAKANA LETTER MA
  MAKE_PAIR( 0xFF90, 0x30DF ),  // HALFWIDTH KATAKANA LETTER MI --> KATAKANA LETTER MI
  MAKE_PAIR( 0xFF91, 0x30E0 ),  // HALFWIDTH KATAKANA LETTER MU --> KATAKANA LETTER MU
  MAKE_PAIR( 0xFF92, 0x30E1 ),  // HALFWIDTH KATAKANA LETTER ME --> KATAKANA LETTER ME
  MAKE_PAIR( 0xFF93, 0x30E2 ),  // HALFWIDTH KATAKANA LETTER MO --> KATAKANA LETTER MO
  MAKE_PAIR( 0xFF94, 0x30E4 ),  // HALFWIDTH KATAKANA LETTER YA --> KATAKANA LETTER YA
  MAKE_PAIR( 0xFF95, 0x30E6 ),  // HALFWIDTH KATAKANA LETTER YU --> KATAKANA LETTER YU
  MAKE_PAIR( 0xFF96, 0x30E8 ),  // HALFWIDTH KATAKANA LETTER YO --> KATAKANA LETTER YO
  MAKE_PAIR( 0xFF97, 0x30E9 ),  // HALFWIDTH KATAKANA LETTER RA --> KATAKANA LETTER RA
  MAKE_PAIR( 0xFF98, 0x30EA ),  // HALFWIDTH KATAKANA LETTER RI --> KATAKANA LETTER RI
  MAKE_PAIR( 0xFF99, 0x30EB ),  // HALFWIDTH KATAKANA LETTER RU --> KATAKANA LETTER RU
  MAKE_PAIR( 0xFF9A, 0x30EC ),  // HALFWIDTH KATAKANA LETTER RE --> KATAKANA LETTER RE
  MAKE_PAIR( 0xFF9B, 0x30ED ),  // HALFWIDTH KATAKANA LETTER RO --> KATAKANA LETTER RO
  MAKE_PAIR( 0xFF9C, 0x30EF ),  // HALFWIDTH KATAKANA LETTER WA --> KATAKANA LETTER WA
  MAKE_PAIR( 0xFF9D, 0x30F3 ),  // HALFWIDTH KATAKANA LETTER N --> KATAKANA LETTER N
  MAKE_PAIR( 0xFF9E, 0x3099 ),  // HALFWIDTH KATAKANA VOICED SOUND MARK --> COMBINING KATAKANA-HIRAGANA VOICED SOUND MARK
  MAKE_PAIR( 0xFF9F, 0x309A )   // HALFWIDTH KATAKANA SEMI-VOICED SOUND MARK --> COMBINING KATAKANA-
};


static struct hw_pair aCombine3099[] =
{
    { 0x30a6, 0x30f4 },
    { 0x30ab, 0x30ac },
    { 0x30ad, 0x30ae },
    { 0x30af, 0x30b0 },
    { 0x30b1, 0x30b2 },
    { 0x30b3, 0x30b4 },
    { 0x30b5, 0x30b6 },
    { 0x30b7, 0x30b8 },
    { 0x30b9, 0x30ba },
    { 0x30bb, 0x30bc },
    { 0x30bd, 0x30be },
    { 0x30bf, 0x30c0 },
    { 0x30c1, 0x30c2 },
    { 0x30c4, 0x30c5 },
    { 0x30c6, 0x30c7 },
    { 0x30c8, 0x30c9 },
    { 0x30cf, 0x30d0 },
    { 0x30d2, 0x30d3 },
    { 0x30d5, 0x30d6 },
    { 0x30d8, 0x30d9 },
    { 0x30db, 0x30dc },
    { 0x30ef, 0x30f7 },
    { 0x30f0, 0x30f8 },
    { 0x30f1, 0x30f9 },
    { 0x30f2, 0x30fa },
    { 0x30fd, 0x30fe }
};

static struct hw_pair aCombine309A[] =
{
    { 0x30cf, 0x30d1 },
    { 0x30d2, 0x30d4 },
    { 0x30d5, 0x30d7 },
    { 0x30d8, 0x30da },
    { 0x30db, 0x30dd }
};

USHORT ImplReplaceFullWidth( sal_Unicode* pString, USHORT nLen )
{
    sal_Unicode* pRead  = pString;
    sal_Unicode* pWrite = pRead;
    USHORT nNewLen = nLen;

    while( (pRead - pString) < nLen )
    {
        if( pWrite != pRead )
            *pWrite = *pRead;

        if( *pRead >= 0xff65 && *pRead <= 0xff9f )
        {
            *pWrite = aHWPairs[ *pRead - 0xff65 ].nTo;

            struct hw_pair* pTable = NULL;
            int nTableEntries = 0;
            if( *pWrite == 0x3099 )
            {
                // replace 0x3099 combinations
                pTable = aCombine3099;
                nTableEntries = sizeof(aCombine3099)/sizeof(aCombine3099[0]);
            }
            else if( *pWrite == 0x309a )
            {
                // replace 0x309a combinations
                pTable = aCombine309A;
                nTableEntries = sizeof(aCombine309A)/sizeof(aCombine309A[0]);
            }
            if( pTable )
            {
                sal_Unicode c = pWrite[-1];
                for( int i = 0; i < nTableEntries; i++ )
                    if( c == pTable[i].nFrom )
                    {
                        pWrite--;
                        *pWrite = pTable[i].nTo;
                        nNewLen--;
                        break;
                    }
            }
        }
        pRead++;
        pWrite++;
    }
    if( pWrite < pRead )
        *pWrite = 0;

    return nNewLen;
}

void ConvertHalfwitdhToFullwidth( String& rString )
{
    USHORT nNewLen = ImplReplaceFullWidth( rString.GetBufferAccess(), rString.Len() );
    rString.ReleaseBufferAccess( nNewLen );
}
