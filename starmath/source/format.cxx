/*************************************************************************
 *
 *  $RCSfile: format.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:57:26 $
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

#pragma hdrstop


#ifndef FORMAT_HXX
#include "format.hxx"
#endif

SmFormat::SmFormat()
:   aBaseSize(0, SmPtsTo100th_mm(12))
{
    nVersion    = SM_FMT_VERSION_NOW;

    eHorAlign   = AlignCenter;
    bIsTextmode = bScaleNormalBrackets = FALSE;

    vSize[SIZ_TEXT]     = 100;
    vSize[SIZ_INDEX]    = 60;
    vSize[SIZ_FUNCTION] =
    vSize[SIZ_OPERATOR] = 100;
    vSize[SIZ_LIMITS]   = 60;

    vDist[DIS_HORIZONTAL]           = 10;
    vDist[DIS_VERTICAL]             = 5;
    vDist[DIS_ROOT]                 = 0;
    vDist[DIS_SUPERSCRIPT]          =
    vDist[DIS_SUBSCRIPT]            = 20;
    vDist[DIS_NUMERATOR]            =
    vDist[DIS_DENOMINATOR]          = 0;
    vDist[DIS_FRACTION]             = 10;
    vDist[DIS_STROKEWIDTH]          = 5;
    vDist[DIS_UPPERLIMIT]           =
    vDist[DIS_LOWERLIMIT]           = 0;
    vDist[DIS_BRACKETSIZE]          =
    vDist[DIS_BRACKETSPACE]         = 5;
    vDist[DIS_MATRIXROW]            = 3;
    vDist[DIS_MATRIXCOL]            = 30;
    vDist[DIS_ORNAMENTSIZE]         =
    vDist[DIS_ORNAMENTSPACE]        = 0;
    vDist[DIS_OPERATORSIZE]         = 50;
    vDist[DIS_OPERATORSPACE]        = 20;
    vDist[DIS_LEFTSPACE]            =
    vDist[DIS_RIGHTSPACE]           = 100;
    vDist[DIS_TOPSPACE]             =
    vDist[DIS_BOTTOMSPACE]          =
    vDist[DIS_NORMALBRACKETSIZE]    = 0;

    vFont[FNT_VARIABLE] =
    vFont[FNT_FUNCTION] =
    vFont[FNT_NUMBER]   =
    vFont[FNT_TEXT]     =
    vFont[FNT_SERIF]    = SmFace(C2S(FNTNAME_TIMES), aBaseSize);
    vFont[FNT_SANS]     = SmFace(C2S(FNTNAME_HELV),  aBaseSize);
    vFont[FNT_FIXED]    = SmFace(C2S(FNTNAME_COUR),  aBaseSize);
    vFont[FNT_MATH]     = SmFace(C2S(FNTNAME_MATH),  aBaseSize);

    vFont[FNT_VARIABLE].SetItalic(ITALIC_NORMAL);
    vFont[FNT_FUNCTION].SetItalic(ITALIC_NONE);
    vFont[FNT_TEXT].SetItalic(ITALIC_NONE);

    for ( USHORT i = FNT_BEGIN;  i <= FNT_END;  i++ )
    {
        vFont[i].SetTransparent(TRUE);
        vFont[i].SetAlign(ALIGN_BASELINE);
    }
}


SmFormat & SmFormat::operator = (const SmFormat &rFormat)
{
    SetBaseSize(rFormat.GetBaseSize());
    SetVersion (rFormat.GetVersion());
    SetHorAlign(rFormat.GetHorAlign());
    SetTextmode(rFormat.IsTextmode());
    SetScaleNormalBrackets(rFormat.IsScaleNormalBrackets());

    USHORT  i;
    for (i = FNT_BEGIN;  i <= FNT_END;  i++)
        SetFont(i, rFormat.GetFont(i));
    for (i = SIZ_BEGIN;  i <= SIZ_END;  i++)
        SetRelSize(i, rFormat.GetRelSize(i));
    for (i = DIS_BEGIN;  i <= DIS_END;  i++)
        SetDistance(i, rFormat.GetDistance(i));

    return *this;
}


SvStream & operator << (SvStream &rStream, const SmFormat &rFormat)
{
    //Da hier keinerlei Kompatibilit„t vorgesehen ist muessen wir leider
    //heftig tricksen. Gluecklicherweise sind offenbar einige Informationen
    //ueberfluessig geworden. In diese quetschen wir jetzt vier neue
    //Einstellungen fuer die Rander.
    //Bei Gelegenheit wird hier ein Im- Und Export gebraucht. Dann muessen
    //die Stream-Operatoren dieser Klassen dringend mit Versionsverwaltung
    //versehen werden!

    UINT16  n;

    // convert the heigth (in 100th of mm) to Pt and round the result to the
    // nearest integer
    n = (UINT16) SmRoundFraction(Sm100th_mmToPts(rFormat.aBaseSize.Height()));
    DBG_ASSERT((n & 0xFF00) == 0, "Sm : higher Byte nicht leer");

    // to be compatible with the old format (size and order) we put the info
    // about textmode in the higher byte (height is already restricted to a
    // maximum of 127!)
    n |=   (rFormat.bIsTextmode != 0)          << 8
         | (rFormat.bScaleNormalBrackets != 0) << 9;
    rStream << n;

    rStream << rFormat.vDist[DIS_LEFTSPACE];    //Wir nutzen den Platz
    rStream << rFormat.vDist[DIS_RIGHTSPACE];   //Wir nutzen den Platz

    for ( n = SIZ_BEGIN; n <= SIZ_END; ++n )
        rStream << rFormat.vSize[n];

    rStream << rFormat.vDist[DIS_TOPSPACE];     //Wir nutzen den Platz

    for ( n = 0; n <= FNT_FIXED; ++n )
        rStream << rFormat.vFont[n];

    // Den zweiten Wert noch im HigherByte unterbringen
    USHORT uTmp =   rFormat.vDist[DIS_BRACKETSIZE]
                  | rFormat.vDist[DIS_NORMALBRACKETSIZE] << 8;
    // und dann dieses rausstreamen
    for ( n = 0; n <= DIS_OPERATORSPACE; ++n )
        rStream << (USHORT)(n != DIS_BRACKETSIZE ? rFormat.vDist[(USHORT) n] : uTmp);

    // higher byte is version number, lower byte is horizontal alignment
    n = rFormat.eHorAlign | SM_FMT_VERSION_NOW << 8;
    rStream << n;

    rStream << rFormat.vDist[DIS_BOTTOMSPACE];  //Wir nutzen den Platz

    return rStream;
}


SvStream & operator >> (SvStream &rStream, SmFormat &rFormat)
{
    UINT16  n;

    rStream >> n;
    long nBaseHeight    = n & 0x00FF;
    rFormat.bIsTextmode          = ((n >> 8) & 0x01) != 0;
    rFormat.bScaleNormalBrackets = ((n >> 9) & 0x01) != 0;
    rFormat.aBaseSize   = Size(0, SmPtsTo100th_mm(nBaseHeight));

    rStream >> rFormat.vDist[DIS_LEFTSPACE];    //Wir nutzen den Platz
    rStream >> rFormat.vDist[DIS_RIGHTSPACE];   //Wir nutzen den Platz

    for ( n = SIZ_BEGIN; n <= SIZ_END; ++n )
        rStream >> rFormat.vSize[n];

    rStream >> rFormat.vDist[DIS_TOPSPACE];     //Wir nutzen den Platz

    for ( n = 0; n <= FNT_FIXED; ++n )
        rStream >> rFormat.vFont[n];

    for ( n = 0; n <= DIS_OPERATORSPACE; ++n )
        rStream >> rFormat.vDist[n];
    // den zweiten Wert aus dem HigherByte holen
    rFormat.vDist[DIS_NORMALBRACKETSIZE] = rFormat.vDist[DIS_BRACKETSIZE] >> 8;
    // und dieses dann ausblenden
    rFormat.vDist[DIS_BRACKETSIZE] &= 0x00FF;

    // higher byte is version number, lower byte is horizontal alignment
    rStream >> n;
    rFormat.nVersion  = n >> 8;
    rFormat.eHorAlign = (SmHorAlign) (n & 0x00FF);

    rStream >> rFormat.vDist[DIS_BOTTOMSPACE];  //Wir nutzen den Platz

    const Size aTmp( rFormat.GetBaseSize() );
    for ( USHORT i = 0; i <= FNT_FIXED; ++i )
    {
        rFormat.vFont[i].SetSize(aTmp);
        rFormat.vFont[i].SetTransparent(TRUE);
        rFormat.vFont[i].SetAlign(ALIGN_BASELINE);
    }
    rFormat.vFont[FNT_MATH].SetSize(aTmp);

    // Für Version 4.0 (und älter) sollen auch die normalen Klammern skalierbar
    // sein und wachsen (so wie es der Fall war), in der 5.0 Version jedoch nicht.
    // In späteren Versionen (>= 5.1) ist das Verhalten nun durch den Anwender
    // festzulegen (bleibt also wie aus dem Stream gelesen).
    if (rFormat.nVersion < SM_FMT_VERSION_51)
    {
        BOOL    bIs50Stream = rStream.GetVersion() == SOFFICE_FILEFORMAT_50;
        BOOL    bVal        = bIs50Stream ? FALSE : TRUE;
        USHORT  nExcHeight  = bIs50Stream ? 0 : rFormat.vDist[DIS_BRACKETSIZE];

        rFormat.SetScaleNormalBrackets(bVal);
        rFormat.SetDistance(DIS_NORMALBRACKETSIZE, nExcHeight);
    }

    return rStream;
}

void SmFormat::ReadSM20Format(SvStream &rStream)
{
    UINT16  n;
    USHORT  i;

    rStream >> n;
    SetBaseSize( Size(0, SmPtsTo100th_mm(n)) );

    rStream >> n >> n;

    for (i = SIZ_BEGIN;  i <= SIZ_LIMITS;  i++)
    {   rStream >> n;
        SetRelSize(i, n);
    }

    rStream >> n;

    for (i = FNT_BEGIN;  i <= FNT_FIXED;  i++)
        ReadSM20Font(rStream, Font(i));

    for (i = DIS_BEGIN;  i <= DIS_OPERATORSPACE;  i++)
    {   rStream >> n;
        SetDistance(i, n);
    }

    rStream >> n;
    SetHorAlign((SmHorAlign) n);
    rStream >> n;

    const Size  aTmp (GetBaseSize());
    for (i = FNT_BEGIN;  i <= FNT_FIXED;  i++)
    {
        Font(i).SetSize(aTmp);
        Font(i).SetTransparent(TRUE);
        Font(i).SetAlign(ALIGN_BASELINE);
    }
    Font(FNT_MATH).SetSize(aTmp);
}


void SmFormat::From300To304a()
{
    long nBaseSize = SmRoundFraction(Sm100th_mmToPts(aBaseSize.Height()))
                     * 2540l / 72l;
    for (USHORT i = DIS_BEGIN;  i < DIS_OPERATORSPACE;  i++)
        SetDistance(i, USHORT(GetDistance(i) * 254000L / 72L / nBaseSize));
}



