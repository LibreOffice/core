/*************************************************************************
 *
 *  $RCSfile: paperinf.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:21 $
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

// include ---------------------------------------------------------------

#include <limits.h>

#ifndef _SHL_HXX
#include <tools/shl.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#include <vcl/svapp.hxx>
#pragma hdrstop

#include "dialogs.hrc"

#include "paperinf.hxx"
#include "dialmgr.hxx"

#define SVX_PAPER_OFFSET    3   // Anfang: enum Paper A3 - SvxPaper A0; Diff=3

// STATIC DATA -----------------------------------------------------------

static Size __FAR_DATA aDinTab[] =
{
    Size(lA0Width,lA0Height),           // A0
    Size(lA1Width,lA1Height),           // A1
    Size(lA2Width,lA2Height),           // A2
    Size(lA3Width,lA3Height),           // A3
    Size(lA4Width,lA4Height),           // A4
    Size(lA5Width,lA5Height),           // A5
    Size(lB4Width, lB4Height),          // B4
    Size(lB5Width,lB5Height),           // B5
    Size(lLetterWidth,lLetterHeight),   // LETTER
    Size(lLegalWidth,lLegalHeight),     // LEGAL
    Size(lTabloidWidth,lTabloidHeight), // TABLOID
    Size(0, 0),                         // USER
    Size(lB6Width, lB6Height),          // B6
    Size(lC4Width, lC4Height),          // C4
    Size(lC5Width, lC5Height),          // C5
    Size(lC6Width, lC6Height),          // C6
    Size(lC65Width, lC65Height),        // C65
    Size(lDLWidth, lDLHeight),          // DL
    Size(lDiaWidth,lDiaHeight ),        // DIA
    Size(lScreenWidth, lScreenHeight),  // SCREEN
    Size(lAWidth, lAHeight),            // A
    Size(lBWidth, lBHeight),            // B
    Size(lCWidth, lCHeight),            // C
    Size(lDWidth, lDHeight),            // D
    Size(lEWidth, lEHeight),            // E
    Size(lExeWidth, lExeHeight),        // Executive
    Size(lLegal2Width, lLegal2Height),  // Legal2
    Size(lMonarchWidth, lMonarchHeight),// Monarch
    Size(lCom675Width, lCom675Height),  // COM-6 3/4
    Size(lCom9Width, lCom9Height),      // COM-9
    Size(lCom10Width, lCom10Height),    // COM-10
    Size(lCom11Width, lCom11Height),    // COM-11
    Size(lCom12Width, lCom12Height),    // COM-12
    Size(lKai16Width, lKai16Height),    // 16 kai
    Size(lKai32Width, lKai32Height),    // 32 kai
    Size(lKai32BigWidth, lKai32BigHeight) // 32 kai gross
};

static const int nTabSize = sizeof(aDinTab) / sizeof(aDinTab[0]);

// -----------------------------------------------------------------------

long TwipsTo100thMM( long nIn )
{
    long nRet = OutputDevice::LogicToLogic( nIn, MAP_TWIP, MAP_100TH_MM );
    long nTmp = nRet % 10;

    if ( nTmp )
        nRet += 10 - nTmp;
    return nRet;
}

// -----------------------------------------------------------------------

Size ConvertTo100thMM( Size& rSize )
{
    // Convert form TWIPS to 100TH_MM
    long nW = TwipsTo100thMM( rSize.Width() );
    long nH = TwipsTo100thMM( rSize.Height() );

    rSize.Width() = nW;
    rSize.Height() = nH;
    return rSize;
}

// -----------------------------------------------------------------------

long HundMMToTwips( long nIn )
{
    long nRet = OutputDevice::LogicToLogic( nIn, MAP_100TH_MM, MAP_TWIP );
    return nRet;
}

// -----------------------------------------------------------------------

Size ConvertToTwips( Size& rSize )
{
    // Convert form TWIPS to 100TH_MM
    long nW = HundMMToTwips( rSize.Width() );
    long nH = HundMMToTwips( rSize.Height() );

    rSize.Width() = nW;
    rSize.Height() = nH;
    return rSize;
}

// -----------------------------------------------------------------------

SvxPaper GetPaper_Impl( const Size &rSize, MapUnit eUnit, BOOL bSloppy )
{
    DBG_ASSERT( eUnit == MAP_TWIP || eUnit == MAP_100TH_MM,
                "map unit not supported" );
    Size aSize = rSize;

    if ( eUnit == MAP_100TH_MM )
        ConvertToTwips( aSize );

    for ( USHORT i = 0; i < nTabSize; i++ )
    {
        if ( aDinTab[i] == aSize )
            return (SvxPaper)i;
        else if ( bSloppy )
        {
            long lDiffW = Abs(aDinTab[i].Width () - aSize.Width ()),
                 lDiffH = Abs(aDinTab[i].Height() - aSize.Height());

            if ( lDiffW < 6 && lDiffH < 6 )
                return (SvxPaper)i;
        }
    }
    return SVX_PAPER_USER;
}


/*--------------------------------------------------------------------
    Beschreibung:   Ist der Printer gueltig
 --------------------------------------------------------------------*/

inline BOOL IsValidPrinter(const Printer *pPtr)
{
    return pPtr->GetName().Len() ? TRUE : FALSE;
}

/*------------------------------------------------------------------------
 Beschreibung:  Konvertierung eines SV-Defines fuer Papiergroesse in
                Twips.
                Funktioniert logischerweise nicht fuer User Groessen
                (ASSERT).
------------------------------------------------------------------------*/

Size SvxPaperInfo::GetPaperSize( SvxPaper ePaper, MapUnit eUnit )
{
    DBG_ASSERT( ePaper < nTabSize, "Tabelle der Papiergroessen ueberindiziert" );
    DBG_ASSERT( eUnit == MAP_TWIP || eUnit == MAP_100TH_MM, "this MapUnit not supported" );
    Size aSize = aDinTab[ePaper];   // in Twips

    if ( eUnit == MAP_100TH_MM )
        ConvertTo100thMM( aSize );
    return aSize;
}

/*------------------------------------------------------------------------
 Beschreibung:  Papiergroesse der Druckers liefern, aligned auf
                die eigenen Groessen.
                Falls kein Printer im System eingestellt ist,
                wird DIN A4 Portrait als Defaultpapiergroesse geliefert.
------------------------------------------------------------------------*/

Size SvxPaperInfo::GetPaperSize( const Printer* pPrinter )
{
    if ( !IsValidPrinter(pPrinter) )
        return GetPaperSize( SVX_PAPER_A4 );
    const SvxPaper ePaper = (SvxPaper)(pPrinter->GetPaper() + SVX_PAPER_OFFSET);

    if ( ePaper == SVX_PAPER_USER )
    {
            // Orientation nicht beruecksichtigen, da durch SV bereits
            // die richtigen Masze eingestellt worden sind.
        Size aPaperSize = pPrinter->GetPaperSize();
        const Size aInvalidSize;

        if ( aPaperSize == aInvalidSize )
        {
#ifdef OS2
            // Workaround fuer OS/2, da class Printer unter OS/2 Macken hat
            LanguageType eLType =
                GetpApp()->GetAppInternational().GetLanguage();

            if ( eLType == LANGUAGE_ENGLISH_US ||
                 eLType == LANGUAGE_ENGLISH_UK )
                return GetPaperSize(SVX_PAPER_LETTER);
            else
                return GetPaperSize(SVX_PAPER_A4);
#else
            return GetPaperSize(SVX_PAPER_A4);
#endif
        }
        MapMode aMap1 = pPrinter->GetMapMode();
        MapMode aMap2;

        if ( aMap1 == aMap2 )
            aPaperSize =
                pPrinter->PixelToLogic( aPaperSize, MapMode( MAP_TWIP ) );
        return aPaperSize;
    }

    const Orientation eOrient = pPrinter->GetOrientation();
    Size aSize( GetPaperSize( ePaper ) );
        // bei Landscape die Seiten tauschen, ist bei SV schon geschehen
    if ( eOrient == ORIENTATION_LANDSCAPE )
        Swap( aSize );
    return aSize;
}

/*------------------------------------------------------------------------
 Beschreibung:  Konvertierung einer Papiergroesse in Twips in das
                SV-Define. Ist bSloppy TRUE, so wird nur auf 1/10 mm genau
                verglichen.
------------------------------------------------------------------------*/

SvxPaper SvxPaperInfo::GetPaper( const Size &rSize, MapUnit eUnit,
                                 BOOL bSloppy )
{
    return GetPaper_Impl( rSize, eUnit, bSloppy );
}

// -----------------------------------------------------------------------

SvxPaper SvxPaperInfo::GetSvxPaper( const Size &rSize, MapUnit eUnit,
                                    BOOL bSloppy )
{
    return GetPaper_Impl( rSize, eUnit, bSloppy );
}

// -----------------------------------------------------------------------

Paper SvxPaperInfo::GetSvPaper( const Size &rSize, MapUnit eUnit,
                                BOOL bSloppy )
{
    SvxPaper ePaper = GetPaper_Impl( rSize, eUnit, bSloppy );

    switch ( ePaper )
    {
        case SVX_PAPER_A3:      return PAPER_A3;
        case SVX_PAPER_A4:      return PAPER_A4;
        case SVX_PAPER_A5:      return PAPER_A5;
        case SVX_PAPER_B4:      return PAPER_B4;
        case SVX_PAPER_B5:      return PAPER_B5;
        case SVX_PAPER_LETTER:  return PAPER_LETTER;
        case SVX_PAPER_LEGAL:   return PAPER_LEGAL;
        case SVX_PAPER_TABLOID: return PAPER_TABLOID;
        default:                return PAPER_USER;
    }
}

/*------------------------------------------------------------------------
 Beschreibung:  String Repr"asentation f"ur die SV-Defines f"ur
                Papiergroessen.
------------------------------------------------------------------------*/

String SvxPaperInfo::GetName( SvxPaper ePaper )
{
    USHORT  nResId = 0;

    switch ( ePaper )
    {
        case SVX_PAPER_A0:          nResId = RID_SVXSTR_PAPER_A0;       break;
        case SVX_PAPER_A1:          nResId = RID_SVXSTR_PAPER_A1;       break;
        case SVX_PAPER_A2:          nResId = RID_SVXSTR_PAPER_A2;       break;
        case SVX_PAPER_A3:          nResId = RID_SVXSTR_PAPER_A3;       break;
        case SVX_PAPER_A4:          nResId = RID_SVXSTR_PAPER_A4;       break;
        case SVX_PAPER_A5:          nResId = RID_SVXSTR_PAPER_A5;       break;
        case SVX_PAPER_B4:          nResId = RID_SVXSTR_PAPER_B4;       break;
        case SVX_PAPER_B5:          nResId = RID_SVXSTR_PAPER_B5;       break;
        case SVX_PAPER_LETTER:      nResId = RID_SVXSTR_PAPER_LETTER;   break;
        case SVX_PAPER_LEGAL:       nResId = RID_SVXSTR_PAPER_LEGAL;    break;
        case SVX_PAPER_TABLOID:     nResId = RID_SVXSTR_PAPER_TABLOID;  break;
        case SVX_PAPER_USER:        nResId = RID_SVXSTR_PAPER_USER;     break;
        case SVX_PAPER_B6:          nResId = RID_SVXSTR_PAPER_B6;       break;
        case SVX_PAPER_C4:          nResId = RID_SVXSTR_PAPER_C4;       break;
        case SVX_PAPER_C5:          nResId = RID_SVXSTR_PAPER_C5;       break;
        case SVX_PAPER_C6:          nResId = RID_SVXSTR_PAPER_C6;       break;
        case SVX_PAPER_C65:         nResId = RID_SVXSTR_PAPER_C65;      break;
        case SVX_PAPER_DL:          nResId = RID_SVXSTR_PAPER_DL;       break;
        case SVX_PAPER_DIA:         nResId = RID_SVXSTR_PAPER_DIA;      break;
        case SVX_PAPER_SCREEN:      nResId = RID_SVXSTR_PAPER_SCREEN;   break;
        case SVX_PAPER_A:           nResId = RID_SVXSTR_PAPER_A;        break;
        case SVX_PAPER_B:           nResId = RID_SVXSTR_PAPER_B;        break;
        case SVX_PAPER_C:           nResId = RID_SVXSTR_PAPER_C;        break;
        case SVX_PAPER_D:           nResId = RID_SVXSTR_PAPER_D;        break;
        case SVX_PAPER_E:           nResId = RID_SVXSTR_PAPER_E;        break;
        case SVX_PAPER_EXECUTIVE:   nResId = RID_SVXSTR_PAPER_EXECUTIVE;break;
        case SVX_PAPER_LEGAL2:      nResId = RID_SVXSTR_PAPER_LEGAL2;   break;
        case SVX_PAPER_MONARCH:     nResId = RID_SVXSTR_PAPER_MONARCH;  break;
        case SVX_PAPER_COM675:      nResId = RID_SVXSTR_PAPER_COM675;   break;
        case SVX_PAPER_COM9:        nResId = RID_SVXSTR_PAPER_COM9;     break;
        case SVX_PAPER_COM10:       nResId = RID_SVXSTR_PAPER_COM10;    break;
        case SVX_PAPER_COM11:       nResId = RID_SVXSTR_PAPER_COM11;    break;
        case SVX_PAPER_COM12:       nResId = RID_SVXSTR_PAPER_COM12;    break;
        case SVX_PAPER_KAI16:       nResId = RID_SVXSTR_PAPER_KAI16;    break;
        case SVX_PAPER_KAI32:       nResId = RID_SVXSTR_PAPER_KAI32;    break;
        case SVX_PAPER_KAI32BIG:    nResId = RID_SVXSTR_PAPER_KAI32BIG; break;

        default: DBG_ERROR( "unknown papersize" );
    }

    if ( nResId )
        return String( SVX_RES( nResId ) );
    return String();
}


