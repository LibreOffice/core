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
#ifndef _SVX_PAPERINF_HXX
#define _SVX_PAPERINF_HXX

// INCLUDE ---------------------------------------------------------------

#ifndef _PRINT_HXX //autogen
#include <vcl/print.hxx>
#endif
#ifndef _MAPMOD_HXX //autogen
#include <vcl/mapmod.hxx>
#endif
#ifndef _SVX_SVXENUM_HXX
#include <bf_svx/svxenum.hxx>
#endif
class Printer;
class Size;
class String;
namespace binfilter {

// forward ---------------------------------------------------------------



// class SvxPaperInfo -----------------------------------------------------

class SvxPaperInfo
{
public:
    static Size			GetPaperSize( const Printer* );
    static Size			GetPaperSize( SvxPaper ePaper,
                                      MapUnit eUnit = MAP_TWIP );
};

// INLINE -----------------------------------------------------------------

inline Size &Swap(Size &rSize)
{
    const long lVal = rSize.Width();
    rSize.Width() = rSize.Height();
    rSize.Height() = lVal;
    return rSize;
}

inline Size &LandscapeSwap(Size &rSize)
{
    if ( rSize.Height() > rSize.Width() )
        Swap( rSize );
    return rSize;
}

// Masze f"ur Standard- Papiergr"ossen:
// Berechnungsformel: X twips = Y mm * 14400 / 254 (ungerundet!)

const long lA0Width			= 47678;			// A0	841mm
const long lA0Height		= 67407;			// A0	1189mm
const long lA1Width			= 33675;			// A1	594mm
const long lA1Height		= lA0Width;			// A1
const long lA2Width			= 23811;			// A2	420mm
const long lA2Height		= lA1Width;			// A2
const long lA3Width			= 16837;			// A3	297mm
const long lA3Height		= lA2Width;			// A3	420mm
const long lA4Width			= 11905;			// A4	210mm
const long lA4Height		= lA3Width;			// A4
const long lA5Width			= 8390;				// A5	148mm
const long lA5Height		= lA4Width;			// A5

const long lB4Width			= 14173;			// B4	250mm
const long lB4Height		= 20012;			// B4	353mm
const long lB5Width			=  9977;			// B5	176mm
const long lB5Height		= lB4Width;			// B5
const long lB6Width			= 7086;
const long lB6Height		= lB5Width;

const long lC4Width			= 12983;
const long lC4Height		= 18368;
const long lC5Width			= 9184;
const long lC5Height		= lC4Width;
const long lC6Width			= 6463;
const long lC6Height		= lC5Width;
const long lC65Width		= 6463;
const long lC65Height		= 12870;
const long lDLWidth			= 6236;
const long lDLHeight		= 12472;

const long lJISB4Width		= 14570;
const long lJISB4Height		= 20636;
const long lJISB5Width		= 10318;
const long lJISB5Height		= lJISB4Width;
const long lJISB6Width		= 7256;
const long lJISB6Height		= lJISB5Width;

// Die folgenden Daten stammen vom WinWord und sind noch
// nicht nach der obigen Formel,
// sondern per 1cm = 567 twips oder 1 Zoll = 1440 twips berechnet worden:

const long lLetterWidth		= 12240;			// LETTER
const long lLetterHeight	= 15840;			// LETTER
const long lLegalWidth		= lLetterWidth;		// LEGAL
const long lLegalHeight		= 20163;			// LEGAL
const long lTabloidWidth	= 15850;			// TABLOID
const long lTabloidHeight	= 24450;			// TABLOID

const long lDiaWidth		= 10206;			// DIA
const long lDiaHeight		= 15309;			// DIA
const long lScreenWidth		= 11905;			// SCREEN
const long lScreenHeight	= 15874;			// SCREEN

const long lAWidth			= 12240;			// A	8,5 Zoll
const long lAHeight			= 15840;			// A	11 Zoll
const long lBWidth			= lAHeight;			// B
const long lBHeight			= 24480;			// B	17 Zoll
const long lCWidth			= lBHeight;			// C
const long lCHeight			= 31680;			// C	22 Zoll
const long lDWidth			= lCHeight;			// D
const long lDHeight			= 48960;			// D	34 Zoll
const long lEWidth			= lDHeight;			// E
const long lEHeight			= 63360;			// E	44 Zoll

const long lExeWidth		= 10440;			// Executive 7,25 Zoll
const long lExeHeight		= 15120;			// Executive 10,5 Zoll
const long lLegal2Width		= 12240;			// Legal2 8,5 Zoll
const long lLegal2Height 	= 18720;			// Legal2 13 Zoll
const long lCom675Width		= 5220;				// COM-6 3/4 3,625 Zoll
const long lCom675Height	= 9360;				// COM-6 3/4 6,5 Zoll
const long lCom9Width		= 5580;				// COM-9 3,875 Zoll
const long lCom9Height		= 12780;			// COM-9 8,875 Zoll
const long lCom10Width		= 5940;				// COM-10 4,125 Zoll
const long lCom10Height		= 13680;			// COM-10 9,5 Zoll
const long lCom11Width		= 6480;				// COM-11 4,5 Zoll
const long lCom11Height		= 14940;			// COM-11 10,375 Zoll
const long lCom12Width		= 6840;				// COM-12 4,75 Zoll
const long lCom12Height		= lAHeight;			// COM-12 11 Zoll
const long lMonarchWidth 	= lCom9Width;		// Monarch 3,875 Zoll
const long lMonarchHeight	= 10800;			// Monarch 7,5 Zoll

const long lKai16Width		= 10433;			// 16 kai 18,4 cm
const long lKai16Height		= 14742;			// 16 kai 26 cm
const long lKai32Width		= 7371;				// 32 kai 13 cm
const long lKai32Height		= lKai16Width;		// 32 kai 18,4 cm
const long lKai32BigWidth	= 7938;				// 32 kai gross 14 cm
const long lKai32BigHeight	= 11510;			// 32 kai gross 20,3 cm

}//end of namespace binfilter
#endif

