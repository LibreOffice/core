/*************************************************************************
 *
 *  $RCSfile: sgvtext.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:59 $
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

#ifdef MAC
#include <mac_start.h>
#include <math.h>
#include <mac_end.h>
#else
#include <math.h>
#endif

#include <vcl/config.hxx>
#include <filter.hxx>
#include "sgffilt.hxx"
#include "sgfbram.hxx"
#include "sgvmain.hxx"
// #include "Debug.c"

extern SgfFontLst* pSgfFonts;

#if defined( WIN ) && defined( MSC )
#pragma code_seg( "SVTOOLS_FILTER1", "SVTOOLS_CODE" )
#endif

#ifndef abs
#define abs(x) ((x)<0 ? -(x) : (x))
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Einschränkungen: Schatten nur grau, 2D und mit fixem Abstand.
//
//
//
//
////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
// AbsBase.Pas

// die folgenden Werte sind in % vom maximalen Schriftgrad der Zeile   */
#define  UndlSpace  5 /* Untersteichungsabstand von der Baseline       */
#define  UndlWidth  6 /* Untersteichungsdicke                          */
#define  UndlSpac2  7 /* Zwischenraum bei doppelter Unterstreichung    */
#define  StrkSpace 25 /* Abstand der Durchstreichlinie von der Baseline*/
#define  StrkWidth  5 /* Durchstreichungsliniendicke                   */
#define  StrkSpac2  7 /* Zwischenraum bei doppelter Durchstreichung    */
#define  OutlWidth  2 /* Strichstärke ist 2% vom Schriftgrad           */

// vvv Sonderzeichen im TextBuffer vvv
#define  TextEnd        0 /* ^@ Ende der Zeichenkette                           */
#define  HardSpace      6 /* ^F Hartspace (wird nicht umbrochen) ,' '           */
#define  GrafText       7 /* ^G Im Text eingebundene Grafik (future)            */
#define  Tabulator      9 /* ^I Tabulatorzeichen, Pfeil                         */
#define  LineFeed      10 /* ^J Neue Zeile                                      */
#define  SoftTrennK    11 /* ^K Zeichen für k-c-Austausch bei Trennung, 'k'     */
#define  AbsatzEnd     13 /* ^M Neuer Absatz =CR                                */
#define  HardTrenn     16 /* ^P Hartes Trennzeichen (wird nicht umbrochen), '-' */
#define  SoftTrennAdd  19 /* ^S Zusatz-Zeichen Trennung von z.b."Schiff-fahrt"  */
#define  Paragraf      21 /* ^U Zeichen welches für Paragraf-Zeichen            */
#define  Escape        27 /* ^[ Escapesequenz einleiten                         */
#define  SoftTrenn     31 /* ^_ Weiches Trennzeichen, '-' nur Zeilenende        */
#define  MaxEscValLen  8
#define  MaxEscLen     (MaxEscValLen+3)

//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
// Escapesequenzen: [Esc]<Ident><Value>[Esc]  also mind. 4 Char
// Max. Länge von Value soll sein: 8 Char (7+Vorzeichen). Demnach max. Länge
// einer Escapesequenz: 11 Char.
// Identifer:

#define  EscFont   'F' /* FontID, z.B. 92500 für CG Times                           */
#define  EscGrad   'G' /* Schriftgrad 1..255 für «Pt-127«Pt                         */
#define  EscBreit  'B' /* Breite 1..255% des Schriftgrades                          */
#define  EscKaptS  'K' /* Kapitälchengröße 1..255% des Schriftgrades                */
#define  EscLFeed  'L' /* Zeilenabstand 1..32767% vom max. Schriftgrad der Zeile    */
                        // oder 1..32767 für 1..16383«Pt absolut (Wenn Bit 15=1)
#define  EscSlant  'S' /* Kursiv(Winkel) 1..8999 für 0.01ø..89.99ø                  */
#define  EscVPos   'V' /* Zeichen Vertikal-Position  1..255 für «Pt..127«Pt         */
#define  EscZAbst  'Z' /* Zeichenabstand -128..127%                                 */
#define  EscHJust  'A' /* H-Justify    Absatz: Links, Zentr, Rechts, Block, Austreibend, Gesperrt (0..5)*/

#define  EscFarbe  'C' /* Farbe 0..7                                                */
#define  EscBFarb  'U' /* BackFarbe 0..7                                            */
#define  EscInts   'I' /* Farbintensität 0..100%                                    */
#define  EscMustr  'M' /* Muster 0..? inkl. Transp...                               */
#define  EscMFarb  'O' /* Musterfarbe 0..7                                          */
#define  EscMBFrb  'P' /* 2. Musterfarbe 0..7                                       */
#define  EscMInts  'W' /* Musterintensität 0..7                                     */

#define  EscSMstr  'E' /* Schattenmuster 0..? inkl. Transp...                       */
#define  EscSFarb  'R' /* Schattenfarbe 0..7                                        */
#define  EscSBFrb  'T' /* 2. Schattenfarbe 0..7                                     */
#define  EscSInts  'Q' /* Schattenintensität 0..7                                   */

#define  EscSXDst  'X' /* Schattenversatz X 0..100%                                 */
#define  EscSYDst  'Y' /* Schattenversatz Y 0..100%                                 */
#define  EscSDist  'D' /* Schattenversatz X-Y 0..100%                               */

#define  EscBold   'f' /* Fett                       //                          */
#define  EscLSlnt  'l' /* LKursiv                    //                          */
#define  EscRSlnt  'r' /* RKursiv                    //                          */
#define  EscUndln  'u' /* Unterstrichen              //                          */
#define  EscDbUnd  'p' /* doppelt Unterstrichen      //                          */
#define  EscKaptF  'k' /* Kapitälchenflag            //                          */
#define  EscStrik  'd' /* Durchgestrichen            //                          */
#define  EscDbStk  'e' /* doppelt Durchgestrichen    //                          */
#define  EscSupSc  'h' /* Hochgestellt               //                          */
#define  EscSubSc  't' /* Tiefgestellt               //                          */
#define  Esc2DShd  's' /* 2D-Schatten                //                          */
#define  Esc3DShd  'j' /* 3D-Schatten                //                          */
#define  Esc4DShd  'i' /* 4D-Schatten                //                          */
#define  EscEbShd  'b' /* Embossed                   //                          */

//  AllEscIdent =[EscFont, EscGrad, EscBreit,EscKaptS,EscLFeed,EscSlant,EscVPos, EscZAbst,EscHJust,
//                EscFarbe,EscBFarb,EscInts, EscMustr,EscMFarb,EscMBFrb,EscMInts,
//                EscSMstr,EscSFarb,EscSBFrb,EscSInts,EscSXDst,EscSYDst,EscSDist,
//                EscBold, EscLSlnt,EscRSlnt,EscUndln,EscDbUnd,EscKaptF,EscStrik,EscDbStk,
//                EscSupSc,EscSubSc,Esc2DShd,Esc3DShd,Esc4DShd];
// Justify muß spätestens am Anfang des Absatzes stehen
#define  EscSet    '' /* Flag setzen                                               */
#define  EscReset  '' /* Flag löschen                                              */
#define  EscDeflt  '' /* Flag auf default setzen                                   */
#define  EscToggl  '' /* Flag Toggeln                                              */
#define  EscRelat  '%'
#define  EscNoFlg  0
#define  EscNoVal  -2147483647 /* -MaxLongInt */
//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
#define  NoTrenn 0xFFFF   /* Wert für Parameter 'Rest' von GetTextChar(), wenn auf keinen Fall getrennt werden soll */
#define  DoTrenn 0xFFFE   /* Wert für Parameter 'Rest' von GetTextChar(), wenn getrennt werden soll                 */

#define  MaxLineChars 1024

#define  ChrXPosArrSize (MaxLineChars+1+1)           /* 2k - Beginnt mit 0 im gegensatz zu StarDraw */
#define  CharLineSize   (MaxLineChars+1+1)
#define  EscStr         (UCHAR[MaxEscLen+1]);

#define  MinChar 32
#define  MaxChar 255


//ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

#define  DefaultCharWidth 4800
#define  GradDiv             2
#define  CharTopToBase     100 /* wegen Apostrophe und Umlaute mehr als 75% */
#define  CharTopToBtm      120 /* Zeilenhöhe ist größer als Schriftgrad     */
                               // bei Avanti-Bold '' eigentlich sogar 130%

// end of AbsBase.Pas
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
// DefBase.Pas

#define  TextBoldBit  0x0001   /* Fett                    */
#define  TextRSlnBit  0x0002   /* Kursiv                  */
#define  TextUndlBit  0x0004   /* Unterstrichen           */
#define  TextStrkBit  0x0008   /* Durchgesteichen         */
#define  TextSupSBit  0x0010   /* Hocgestellt             */
#define  TextSubSBit  0x0020   /* Tiefgestellt            */
#define  TextKaptBit  0x0040   /* Kapitälchen             */
#define  TextLSlnBit  0x0080   /* Linkskursiv             */
#define  TextDbUnBit  0x0100   /* Doppelt unterstrichen   */
#define  TextDbStBit  0x0200   /* Doppelt durchgestrichen */
#define  TextSh2DBit  0x0400   /* 2D-Schatten         2.0 */
#define  TextSh3DBit  0x0800   /* 3D-Schatten         2.0 */
#define  TextSh4DBit  0x1000   /* 4D-Schatten         2.0 */
#define  TextShEbBit  0x2000   /* Embossed-Schatten   2.0 */
#define  FontAtrBits  (TextBoldBit | TextRSlnBit)

#define  THJustLeft    0x00
#define  THJustCenter  0x01
#define  THJustRight   0x02
#define  THJustBlock   0x03
#define  THJustDrvOut  0x04 /* Austreibend          Formatiert */
#define  THJustLocked  0x05 /* A l s   S p e r r s c h r i f t */
#define  TVJustTop     0x00 /* Future */
#define  TVJustCenter  0x10 /* Future */
#define  TVJustBottom  0x20 /* Future */
#define  TVJustBlock   0x30 /* Future */

#define  MaxCharSlant  4200 /* Maximal 42ø kursiv ! */

// end of DefBase.Pas
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


BOOL CheckTextOutl(ObjAreaType& F, ObjLineType& L);

BOOL CheckTextOutl(ObjAreaType& F, ObjLineType& L)
{
    return (F.FIntens!=L.LIntens) ||
           ((F.FFarbe!=L.LFarbe)   && (F.FIntens>0)) ||
           ((F.FBFarbe!=L.LBFarbe) && (F.FIntens<100));
}


/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
// Misc.Pas

short hPoint2Sgf(short a)
{
    long b;
    b=long(a)*127*SgfDpmm/(144*5);
    return short(b);
}

short Sgf2hPoint(short a)
{
    long b;
    b=long(a)*5*144/(127*SgfDpmm);
    return short(b);
}

// End of Misc.Pas
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
// AbsRead.Pas

// ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
// ³ Function GetTopToBaseLine()  Function GetBaseLineToBtm()           ³
// ³                                                                    ³
// ³ Abstand von Zeilenoberkante bis BaseLine bzw. von BaseLine bis     ³
// ³ Unterkante berechnen. Alles in SGF-Units.                          ³
// ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ

USHORT GetTopToBaseLine(USHORT MaxGrad)
{
    long ret;
    ret=long(MaxGrad)*long(CharTopToBase) /long(100);
    return USHORT(ret);
}

// ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
// ³ Function GetTextChar()   Function GetTextCharConv()                ³
// ³                                                                    ³
// ³ Liest ein Zeichen aus dem Textbuffer, wertet dabei eventuell       ³
// ³ auftretende Escapesequenzen aus und setzt dementsprechend den      ³
// ³ Ein-/Ausgabeparameter AktAtr. Index wird entsprechend erhöht.      ³
// ³ Der Parameter Rest muß immer die Anzahl der Zeichen beinhalten,    ³
// ³ den angeforderten Zeichen in der aktuellen Zeile noch folgen.      ³
// ³ Ansonsten funktioniert die Silbentrennung nicht richtig. Gibt man  ³
// ³ stattdessen die Konstante NoTrenn an, wird in keinem Fall          ³
// ³ getrennt, die Konstante DoTrenn bewirkt dagegen, daß überall dort  ³
// ³ getrennt wird, wo ein SoftTrenner vorkommt.                        ³
// ³                                                                    ³
// ³ SoftTrenner werden immer in ein Minuszeichen konvertiert.          ³
// ³ GetTextCharConv() konvertiert zusätzlich HardSpace und AbsatzEnde  ³
// ³ in Spaces sowie HardTrenner in Minuszeichen. TextEnde wird immer   ³
// ³ als Char(0) geliefert.                                             ³
// ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ



UCHAR ConvertTextChar(UCHAR c)
{
    if (c<32) {
        switch (c) {
            case HardSpace   : c=' '; break;
            case AbsatzEnd   : c=' '; break;
            case SoftTrenn   : c='-'; break;
            case HardTrenn   : c='-'; break;
            case SoftTrennK  : c='-'; break;
            case SoftTrennAdd: c='-';
        }
    }
    return c;
}



USHORT GetSchnittBit(UCHAR c)
{
    USHORT r=0;
    switch (c) {
        case EscBold : r=TextBoldBit; break;
        case EscRSlnt: r=TextRSlnBit; break;
        case EscUndln: r=TextUndlBit; break;
        case EscStrik: r=TextStrkBit; break;
        case EscDbUnd: r=TextDbUnBit; break;
        case EscDbStk: r=TextDbStBit; break;
        case EscSupSc: r=TextSupSBit; break;
        case EscSubSc: r=TextSubSBit; break;
        case EscKaptF: r=TextKaptBit; break;
        case EscLSlnt: r=TextLSlnBit; break;
        case Esc2DShd: r=TextSh2DBit; break;
        case Esc3DShd: r=TextSh3DBit; break;
        case Esc4DShd: r=TextSh4DBit; break;
        case EscEbShd: r=TextShEbBit;
    }
    return r;
}



long ChgValue(long Def, long Min, long Max, UCHAR FlgVal, long NumVal)
{
    long r=0;

    if (FlgVal==EscDeflt) {
        r=Def;                          // zurück auf Default
    } else {
        if (NumVal!=EscNoVal) r=NumVal; // Hart setzen
    }

    if (Min!=0 || Max!=0) {
        if (r>Max) r=Max;
        if (r<Min) r=Min;
    }
    return r;
}



void ChgSchnittBit(USHORT Bit, USHORT Radio1, USHORT Radio2, USHORT Radio3,
                   UCHAR FlgVal, USHORT Schnitt0, USHORT& Schnitt)
{
    USHORT All,Rad;

    Rad=Radio1 | Radio2 | Radio3;
    All=Bit | Rad;

    switch (FlgVal) {
        case EscSet  : Schnitt=(Schnitt & ~All) | Bit;              break;
        case EscReset: Schnitt=(Schnitt & ~All);                    break;
        case EscDeflt: Schnitt=(Schnitt & ~All) | (Schnitt0 & All); break;
        case EscToggl: Schnitt=(Schnitt & ~Rad) ^ Bit;
    }
}



UCHAR GetNextChar(UCHAR* TBuf, USHORT Index)
{
    USHORT Cnt;
    while (TBuf[Index]==Escape) {
        Index++;
        Cnt=0;
        while (TBuf[Index]!=Escape && Cnt<=MaxEscLen) {
            Index++; Cnt++; }
        Index++;
    }
    return TBuf[Index];
}



UCHAR ProcessOne(UCHAR* TBuf, USHORT& Index,
                 ObjTextType& Atr0, ObjTextType& AktAtr,
                 BOOL ScanEsc)
{
    UCHAR c;
    UCHAR Ident;
    BOOL  Ende;
    BOOL  q;
    UCHAR FlgVal;
    long  NumVal;
    long  Sgn;
    short i;
    BOOL  EoVal;

    do {
        c=TBuf[Index]; Index++;
        Ende=(c!=Escape);
        if (Ende==FALSE) {
            c=TBuf[Index]; Index++;
            Ident=c;                          // Identifer merken
            FlgVal=EscNoFlg;
            NumVal=EscNoVal;
            c=TBuf[Index]; Index++;            // Hier fängt der Wert an
            if (c==EscSet || c==EscReset || c==EscDeflt || c==EscToggl) FlgVal=c; else {
                if (c=='-') Sgn=-1; else Sgn=1;
                if (c=='+' || c=='-') { c=TBuf[Index]; Index++; }
                i=MaxEscValLen;
                NumVal=0;
                do {
                    NumVal=10*NumVal+c-'0';
                    EoVal=(TBuf[Index]<'0' || TBuf[Index]>'9');
                    if (EoVal==FALSE) { c=TBuf[Index]; Index++; }
                    i--;
                } while (i>0 && EoVal==FALSE);
                NumVal=Sgn*NumVal;
            }
            q=!CheckTextOutl(AktAtr.F,AktAtr.L);

            switch (Ident) {
                case EscFont : AktAtr.SetFont(ULONG (ChgValue(Atr0.GetFont(),0,0          ,FlgVal,NumVal)));break;
                case EscGrad : AktAtr.Grad   =USHORT(ChgValue(Atr0.Grad,   2,2000         ,FlgVal,NumVal)); break;
                case EscBreit: AktAtr.Breite =USHORT(ChgValue(Atr0.Breite, 1,1000         ,FlgVal,NumVal)); break;
                case EscKaptS: AktAtr.Kapit  =(BYTE)(ChgValue(Atr0.Kapit,  1,255          ,FlgVal,NumVal)); break;
                case EscLFeed: AktAtr.LnFeed =USHORT(ChgValue(Atr0.LnFeed, 1,65535        ,FlgVal,NumVal)); break;
                case EscSlant: AktAtr.Slant  =USHORT(ChgValue(Atr0.Slant,  1,MaxCharSlant ,FlgVal,NumVal)); break;
                case EscVPos : AktAtr.ChrVPos=char  (ChgValue(Atr0.ChrVPos,-128,127       ,FlgVal,NumVal)); break;
                case EscZAbst: AktAtr.ZAbst  =(BYTE)(ChgValue(Atr0.ZAbst,  1,255          ,FlgVal,NumVal)); break;
                case EscHJust: AktAtr.Justify=(BYTE)(ChgValue(Atr0.Justify & 0x0F,0,5     ,FlgVal,NumVal)); break;
                case EscFarbe: { AktAtr.L.LFarbe =(BYTE)(ChgValue(Atr0.L.LFarbe,0,7   ,FlgVal,NumVal)); if (q) AktAtr.F.FFarbe =AktAtr.L.LFarbe;  } break;
                case EscBFarb: { AktAtr.L.LBFarbe=(BYTE)(ChgValue(Atr0.L.LBFarbe,0,255,FlgVal,NumVal)); if (q) AktAtr.F.FBFarbe=AktAtr.L.LBFarbe; } break;
                case EscInts : { AktAtr.L.LIntens=(BYTE)(ChgValue(Atr0.L.LIntens,0,100,FlgVal,NumVal)); if (q) AktAtr.F.FIntens=AktAtr.L.LIntens; } break;

                case EscMustr: { AktAtr.F.FMuster=USHORT(ChgValue(Atr0.F.FMuster,0,65535,FlgVal,NumVal)); } break;
                case EscMFarb: { AktAtr.F.FFarbe =(BYTE)(ChgValue(Atr0.F.FFarbe,0,7   ,FlgVal,NumVal));   } break;
                case EscMBFrb: { AktAtr.F.FBFarbe=(BYTE)(ChgValue(Atr0.F.FBFarbe,0,255,FlgVal,NumVal));   } break;
                case EscMInts: { AktAtr.F.FIntens=(BYTE)(ChgValue(Atr0.F.FIntens,0,100,FlgVal,NumVal));   } break;

                case EscSMstr: { AktAtr.ShdF.FMuster=USHORT(ChgValue(Atr0.ShdF.FMuster,0,65535,FlgVal,NumVal)); } break;
                case EscSFarb: { AktAtr.ShdL.LFarbe =(BYTE)(ChgValue(Atr0.ShdL.LFarbe,0,7   ,FlgVal,NumVal)); AktAtr.ShdF.FFarbe =AktAtr.ShdL.LFarbe;  } break;
                case EscSBFrb: { AktAtr.ShdL.LBFarbe=(BYTE)(ChgValue(Atr0.ShdL.LBFarbe,0,255,FlgVal,NumVal)); AktAtr.ShdF.FBFarbe=AktAtr.ShdL.LBFarbe; } break;
                case EscSInts: { AktAtr.ShdL.LIntens=(BYTE)(ChgValue(Atr0.ShdL.LIntens,0,100,FlgVal,NumVal)); AktAtr.ShdF.FIntens=AktAtr.ShdL.LIntens; } break;
                case EscSDist: { AktAtr.ShdVers.x=(short)ChgValue(Atr0.ShdVers.x,0,30000,FlgVal,NumVal); AktAtr.ShdVers.y=AktAtr.ShdVers.x; }            break;
                case EscSXDst: { AktAtr.ShdVers.x=(short)ChgValue(Atr0.ShdVers.x,0,30000,FlgVal,NumVal); }  break;
                case EscSYDst: { AktAtr.ShdVers.y=(short)ChgValue(Atr0.ShdVers.y,0,30000,FlgVal,NumVal); }  break;

                case EscBold : ChgSchnittBit(TextBoldBit,0,0,0                              ,FlgVal,Atr0.Schnitt,AktAtr.Schnitt); break;
                case EscRSlnt: ChgSchnittBit(TextRSlnBit,TextLSlnBit,0,0                    ,FlgVal,Atr0.Schnitt,AktAtr.Schnitt); break;
                case EscUndln: ChgSchnittBit(TextUndlBit,TextDbUnBit,0,0                    ,FlgVal,Atr0.Schnitt,AktAtr.Schnitt); break;
                case EscStrik: ChgSchnittBit(TextStrkBit,TextDbStBit,0,0                    ,FlgVal,Atr0.Schnitt,AktAtr.Schnitt); break;
                case EscDbUnd: ChgSchnittBit(TextDbUnBit,TextUndlBit,0,0                    ,FlgVal,Atr0.Schnitt,AktAtr.Schnitt); break;
                case EscDbStk: ChgSchnittBit(TextDbStBit,TextStrkBit,0,0                    ,FlgVal,Atr0.Schnitt,AktAtr.Schnitt); break;
                case EscSupSc: ChgSchnittBit(TextSupSBit,TextSubSBit,0,0                    ,FlgVal,Atr0.Schnitt,AktAtr.Schnitt); break;
                case EscSubSc: ChgSchnittBit(TextSubSBit,TextSupSBit,0,0                    ,FlgVal,Atr0.Schnitt,AktAtr.Schnitt); break;
                case EscKaptF: ChgSchnittBit(TextKaptBit,0,0,0                              ,FlgVal,Atr0.Schnitt,AktAtr.Schnitt); break;
                case EscLSlnt: ChgSchnittBit(TextLSlnBit,TextRSlnBit,0,0                    ,FlgVal,Atr0.Schnitt,AktAtr.Schnitt); break;
                case Esc2DShd: ChgSchnittBit(TextSh2DBit,TextSh3DBit,TextSh4DBit,TextShEbBit,FlgVal,Atr0.Schnitt,AktAtr.Schnitt); break;
                case Esc3DShd: ChgSchnittBit(TextSh3DBit,TextSh2DBit,TextSh4DBit,TextShEbBit,FlgVal,Atr0.Schnitt,AktAtr.Schnitt); break;
                case Esc4DShd: ChgSchnittBit(TextSh4DBit,TextSh2DBit,TextSh3DBit,TextShEbBit,FlgVal,Atr0.Schnitt,AktAtr.Schnitt); break;
                case EscEbShd: ChgSchnittBit(TextShEbBit,TextSh2DBit,TextSh3DBit,TextSh4DBit,FlgVal,Atr0.Schnitt,AktAtr.Schnitt); break;
            } //endcase
            if (TBuf[Index]==Escape) Index++;         // zweites Esc weglesen }
        } // if Ende==FALSE
    } while (Ende==FALSE && ScanEsc==FALSE);
    if (Ende==FALSE) c=Escape;
    return c;
} // end of ProcessOne


UCHAR GetTextChar(UCHAR* TBuf, USHORT& Index,
                  ObjTextType& Atr0, ObjTextType& AktAtr,
                  USHORT Rest, BOOL ScanEsc)
{
    UCHAR c,c0,nc;

    c=ProcessOne(TBuf,Index,Atr0,AktAtr,ScanEsc);
    if (ScanEsc==FALSE) {
        if (c==SoftTrennAdd || c==SoftTrennK || c==SoftTrenn) {
            nc=GetNextChar(TBuf,Index);
            c0=c;
            if (Rest==0 || Rest==DoTrenn ||
                nc==' ' || nc==AbsatzEnd || nc==TextEnd) c='-';
            else {
                c=ProcessOne(TBuf,Index,Atr0,AktAtr,ScanEsc); // den Trenner überspringen
                if (c0==SoftTrennAdd) {
                    if (c>=32) c=ProcessOne(TBuf,Index,Atr0,AktAtr,ScanEsc); // und hier noch 'nen Buchstaben überspringen
                }
            }
        }
        if ((Rest==1 || Rest==DoTrenn) && GetNextChar(TBuf,Index)==SoftTrennK) {
            if (c=='c') c='k';
            else if (c=='C') c='K';
        }
    }
    return c;
}

  // HardSpace und HardTrenn müssen explizit konvertiert werden ! }
  // if AktAtr.Schnitt and TextKaptBit =TextKaptBit then c:=UpCase(c);(explizit) }

  // Bei der Trennmethode SoftTrennAdd wird davon ausgegangen, daß der zu }
  // trennende Konsonant bereits 3x mal im TextBuf vorhanden ist, z.b.:   }
  // "Schiff-fahrt". Wenn nicht getrennt, dann wird "-f" entfernt.        }



UCHAR GetTextCharConv(UCHAR* TBuf, USHORT& Index,
                      ObjTextType& Atr0, ObjTextType& AktAtr,
                      USHORT Rest, BOOL ScanEsc)
{
    UCHAR c;

    c=GetTextChar(TBuf,Index,Atr0,AktAtr,Rest,ScanEsc);
    if (c<32) {
        switch (c) {
            case HardSpace   : c=' '; break;
            case AbsatzEnd   : c=' '; break;
            case HardTrenn   : c='-';
        }
    }
    return c;
}


// ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿
// ³ Function GetLineFeed()                                             ³
// ³                                                                    ³
// ³ Benötigter Zeilenabstand in SGF-Units. ChrVPos wird berücksichtigt.³
// ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
USHORT GetLineFeed(UCHAR* TBuf, USHORT Index, ObjTextType Atr0, ObjTextType AktAtr,
                   USHORT nChar, USHORT& LF, USHORT& MaxGrad)
{
    UCHAR  c=0;
    BOOL   AbsEnd=FALSE;
    ULONG  LF100=0;
    ULONG  MaxLF100=0;
    BOOL   LFauto=0;
    BOOL   First=TRUE;
    USHORT Grad;
    USHORT i=0;
    USHORT r=1;

    MaxGrad=0;
    while (!AbsEnd && nChar>0) {
        nChar--;
        c=GetTextChar(TBuf,Index,Atr0,AktAtr,nChar,FALSE);
        i++;
        AbsEnd=(c==TextEnd || c==AbsatzEnd);
        if (First || (!AbsEnd && c!=' ' && c!=HardTrenn)) {
            LFauto=(AktAtr.LnFeed & 0x8000)==0;
            LF100=AktAtr.LnFeed & 0x7FFF;
            if (LFauto) LF100=LF100*AktAtr.Grad; else LF100*=LF100;
            if (AktAtr.ChrVPos>0) LF100-=AktAtr.ChrVPos*100;
            if (LF100>MaxLF100) MaxLF100=LF100;
            Grad=AktAtr.Grad;
            if (AktAtr.ChrVPos>0) Grad-=AktAtr.ChrVPos;
            if (Grad>MaxGrad) MaxGrad=Grad;
            First=FALSE;
        }
        if (!AbsEnd && c!=' ') r=i;
    }
    MaxGrad=hPoint2Sgf(MaxGrad);
    if (MaxLF100<=4000) {  // sonst Overflowgefahr
        LF=USHORT(hPoint2Sgf(short(MaxLF100)) /100);
    } else {
        LF=USHORT(hPoint2Sgf(short(MaxLF100) /100));
    }

    return r;
}

// End of AbsRead.Pas
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
// iFont.Pas

#define DefaultSlant 1500   /* Default: Italic ist 15ø                    */
#define SuperSubFact 60     /* SuperScript/SubScript: 60% vom Schriftgrad */
#define DefaultSpace 40     /* Default: Space ist 40% vom SchriftGrad     */

USHORT SetTextContext(OutputDevice& rOut, ObjTextType& Atr, BOOL Kapt, USHORT Dreh,
                      USHORT FitXMul, USHORT FitXDiv, USHORT FitYMul, USHORT FitYDiv)
{
    SgfFontOne* pSgfFont; // Font aus dem IniFile
    Font   aFont;
    Color  aColor;
    ULONG  Grad;
    ULONG  Brei;
    String FNam;
    USHORT StdBrei=50;    // Durchschnittliche Zeichenbreite in % von Schriftgrad
    BOOL   bFit=(FitXMul!=1 || FitXDiv!=1 || FitYMul!=1 || FitYDiv!=1);

    pSgfFont = pSgfFonts->GetFontDesc(Atr.GetFont());

    if ( pSgfFont!=NULL )
    {
        FNam   =pSgfFont->SVFName;
        StdBrei=pSgfFont->SVWidth;
        if (pSgfFont->Fixd) aFont.SetPitch(PITCH_FIXED); else aFont.SetPitch(PITCH_VARIABLE);
        aFont.SetFamily(pSgfFont->SVFamil);
        aFont.SetCharSet(pSgfFont->SVChSet);
        aFont.SetName(FNam);
    }
    else
    {  // Falls nich im Inifile, sind hier einige Fonts hart kodiert
        aFont.SetPitch(PITCH_VARIABLE);
        switch (Atr.GetFont()) {
          case 92500: case 92501: case 92504: case 92505:
          {
#if defined(WIN) || defined(WNT) || defined(PM2)
              FNam=String::CreateFromAscii( "Times New Roman" );  // CG Times ist unter Windows und OS/2 Times New Roman
#else
              FNam=String::CreateFromAscii( "Times" );            // ansonsten ist das einfach Times
#endif
              StdBrei=40;
              aFont.SetFamily(FAMILY_ROMAN);
          } break;
          case 94021: case 94022: case 94023: case 94024: {
#if defined(WIN) || defined(WNT)
              FNam=String::CreateFromAscii( "Arial", 5 );            // Univers ist unter Windows Arial
#else
              FNam=String::CreateFromAscii( "Helvetica" );        // und ansonsten Helvetica
#endif
              aFont.SetFamily(FAMILY_SWISS);
              StdBrei=47;
          } break;
          case 93950: case 93951: case 93952: case 93953: {
#if defined(WIN) || defined(WNT)
              FNam=String::CreateFromAscii( "Courier New" );      // Der Vector-Courierfont unter Windows heißt Courier New
#else
              FNam=String::CreateFromAscii( "Courier" );          // ansonsten ist und bleibt Courier immer Courier
#endif
              aFont.SetFamily(FAMILY_ROMAN);
              aFont.SetPitch(PITCH_FIXED);
          } break;
          default: FNam=String::CreateFromAscii( "Helvetica", 9 );
        }
        aFont.SetName(FNam);
        //aFont.SetCharSet(CHARSET_SYSTEM);
    }

    Grad=ULONG(Atr.Grad);
    if ((Atr.Schnitt & TextKaptBit) !=0 && Kapt) Grad=Grad*ULONG(Atr.Kapit)/100;
    if ((Atr.Schnitt & TextSupSBit) !=0 || (Atr.Schnitt & TextSubSBit) !=0) Grad=Grad*SuperSubFact/100;
    Brei=Grad;
    if (Atr.Breite!=100 || bFit) {
        if (bFit) {
            Grad=Grad*ULONG(FitYMul)/ULONG(FitYDiv);
            Brei=Brei*ULONG(FitXMul)/ULONG(FitXDiv);
        }
        Brei=Brei*ULONG(Atr.Breite)/100;
        Brei=Brei*ULONG(StdBrei)/100;
        aFont.SetSize(Size(hPoint2Sgf(USHORT(Brei)),hPoint2Sgf(USHORT(Grad))));
    } else {
        aFont.SetSize(Size(0,hPoint2Sgf(USHORT(Grad))));
    }

    aColor=Sgv2SvFarbe(Atr.L.LFarbe,Atr.L.LBFarbe,Atr.L.LIntens); aFont.SetColor(aColor);
    aColor=Sgv2SvFarbe(Atr.F.FFarbe,Atr.F.FBFarbe,Atr.F.FIntens); aFont.SetFillColor(aColor);
    aFont.SetTransparent(TRUE);
    aFont.SetAlign(ALIGN_BASELINE);

    Dreh/=10; Dreh=3600-Dreh; if (Dreh==3600) Dreh=0;
    aFont.SetOrientation(Dreh);

    if ((Atr.Schnitt & TextBoldBit) !=0) aFont.SetWeight(WEIGHT_BOLD);
    if ((Atr.Schnitt & TextRSlnBit) !=0) aFont.SetItalic(ITALIC_NORMAL);
    if ((Atr.Schnitt & TextUndlBit) !=0) aFont.SetUnderline(UNDERLINE_SINGLE);
    if ((Atr.Schnitt & TextDbUnBit) !=0) aFont.SetUnderline(UNDERLINE_DOUBLE);
    if ((Atr.Schnitt & TextStrkBit) !=0) aFont.SetStrikeout(STRIKEOUT_SINGLE);
    if ((Atr.Schnitt & TextDbStBit) !=0) aFont.SetStrikeout(STRIKEOUT_DOUBLE);
    if ((Atr.Schnitt & TextSh2DBit) !=0) aFont.SetShadow(TRUE);
    if ((Atr.Schnitt & TextSh3DBit) !=0) aFont.SetShadow(TRUE);
    if ((Atr.Schnitt & TextSh4DBit) !=0) aFont.SetShadow(TRUE);
    if ((Atr.Schnitt & TextShEbBit) !=0) aFont.SetShadow(TRUE);
    if (CheckTextOutl(Atr.F,Atr.L)) aFont.SetOutline(TRUE);

    if (aFont!=rOut.GetFont()) rOut.SetFont(aFont);

    return 0;
}

// iFont.Pas
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
// Absatz.Pas

struct ProcChrSta {
    USHORT Index;
    USHORT ChrXP;
    UCHAR  OutCh;
    BOOL   Kapt;
    ObjTextType Attrib;
};

void InitProcessCharState(ProcChrSta& State, ObjTextType& AktAtr, USHORT IndexA)
{
    State.Attrib=AktAtr;
    State.OutCh=0;
    State.Index=IndexA;
    State.ChrXP=0;
    State.Kapt=FALSE;
}

BOOL UpcasePossible(UCHAR c)
{
    if ((c>='a' && c<='z') || c=='ä' || c=='ö' || c=='ü') return TRUE;
    else return FALSE;
}

UCHAR Upcase(UCHAR c)
{
    if ((c>=(UCHAR)'a' && c<=(UCHAR)'z')) c=(c-(UCHAR)'a')+(UCHAR)'A';
    else if (c==(UCHAR)'ä') c=(UCHAR)'Ä';
    else if (c==(UCHAR)'ö') c=(UCHAR)'Ö';
    else if (c==(UCHAR)'ü') c=(UCHAR)'Ü';
    return c;
}

USHORT GetCharWidth(OutputDevice& rOut, UCHAR c)
{
    UCHAR  c1;
    USHORT ChrWidth;

    c1 = ByteString::Convert((char)c,RTL_TEXTENCODING_IBM_437, gsl_getSystemTextEncoding() );
    if (c==' ')
    {
        ChrWidth=(USHORT)rOut.GetTextWidth( String('A') );
        if (rOut.GetFont().GetPitch()!=PITCH_FIXED) {
            ChrWidth=MulDiv(ChrWidth,DefaultSpace,100);
        }
    } else {
        if (c>=MinChar && c<=MaxChar) {
            ChrWidth=(USHORT)rOut.GetTextWidth(String((char)c1));
        } else {
            ChrWidth=(USHORT)rOut.GetTextWidth(String('A'));
        }
    }
    return ChrWidth;
}

UCHAR ProcessChar(OutputDevice& rOut, UCHAR* TBuf, ProcChrSta& R, ObjTextType& Atr0,
                  USHORT& nChars, USHORT Rest,
                  short* Line, UCHAR* cLine)
{
    USHORT       KernDist=0;       // Wert für Kerning
    USHORT       ChrWidth;
    UCHAR        c;
    UCHAR        c1;
    BOOL         AbsEnd;

    c=GetTextChar(TBuf,R.Index,Atr0,R.Attrib,Rest,FALSE); // versucht evtl. zu trennen, wenn Rest entsprechenden Wert besitzt

    AbsEnd=(c==AbsatzEnd || c==TextEnd);
    if (AbsEnd==FALSE) {
        R.OutCh=ConvertTextChar(c); // von HardTrenn nach '-', ...
        R.Kapt=(R.Attrib.Schnitt & TextKaptBit) !=0 && UpcasePossible(R.OutCh);
        if (R.Kapt) R.OutCh=Upcase(R.OutCh);
        SetTextContext(rOut,R.Attrib,R.Kapt,0,1,1,1,1);

        if (R.Kapt) c1=Upcase(c); else c1=c;
        ChrWidth=GetCharWidth(rOut,c1);

        if (R.Attrib.ZAbst!=100) { // Spezial-Zeichenabstand ?
            ULONG Temp;
            Temp=ULONG(ChrWidth)*ULONG(R.Attrib.ZAbst)/100;
            ChrWidth=USHORT(Temp);
        }
        nChars++;
        if (R.ChrXP>32000) R.ChrXP=32000;
        Line[nChars]=R.ChrXP-KernDist;
        cLine[nChars]=c;
        R.ChrXP+=ChrWidth-KernDist; // Position für den nächsten Character
    }
    return c;
}

void FormatLine(UCHAR* TBuf, USHORT& Index, ObjTextType& Atr0, ObjTextType& AktAtr,
                USHORT UmbWdt, USHORT AdjWdt,
                short* Line, USHORT& nChars,
                double rSn, double rCs,
                UCHAR* cLine, BOOL TextFit)
{
    VirtualDevice vOut;
    UCHAR        c,c0;
    UCHAR        ct;
    BOOL         First;               // erster Char ?
    BYTE         Just;                // Absatzformatierung
    BOOL         Border;              // Rand der Box erreicht ?
    BOOL         Border0;
    BOOL         AbsEnd;              // Ende des Absatzes erreicht ?
    ProcChrSta*  R=new ProcChrSta;
    ProcChrSta*  R0=new ProcChrSta;
    ProcChrSta*  WErec=new ProcChrSta;
    USHORT       WEnChar;
    ProcChrSta*  WErec0=new ProcChrSta;
    USHORT       WEnChar0;
    ProcChrSta*  TRrec=new ProcChrSta;
    USHORT       TRnChar;

    USHORT       WordEndCnt;          // Justieren und Trennen
    BOOL         WordEnd;
    BOOL         Trenn;

    short        BoxRest;             // zum Quetschen und formatieren
    USHORT       i,j,k,h;
    USHORT       re,li;

    vOut.SetMapMode(MapMode(MAP_10TH_MM,Point(),Fraction(1,4),Fraction(1,4)));

    nChars=0;
    SetTextContext(vOut,AktAtr,FALSE,0,1,1,1,1);
    InitProcessCharState(*R,AktAtr,Index);
    (*R0)=(*R); (*WErec)=(*R); WEnChar=0; c0=0; Border0=FALSE;
    Border=FALSE; First=TRUE;
    WordEndCnt=0;

    do {               // mal schauen, wieviele Worte so in die Zeile passen
        if (Border) c=ProcessChar(vOut,TBuf,*R,Atr0,nChars,DoTrenn,Line,cLine);
        else        c=ProcessChar(vOut,TBuf,*R,Atr0,nChars,NoTrenn,Line,cLine);
        AbsEnd=(c==AbsatzEnd || c==TextEnd);
        //if not AbsEnd then
        {
            if (First) {
                Just=R->Attrib.Justify & 0x0F; // Absatzformat steht wenn, dann am Anfang
            }
            Border=R->ChrXP>UmbWdt;
            WordEnd=(AbsEnd || (c==' ')) && (c0!=' ') && (c0!=0);
            Trenn=c=='-';
            if (WordEnd && !Border0) {
                WordEndCnt++;
                (*WErec)=(*R0);
                WEnChar=nChars-1;
            }
            if (Trenn && !Border) {
                WordEndCnt++;
                (*WErec)=(*R);
                WEnChar=nChars;
            }
        }
        (*R0)=(*R); c0=c;
        Border0=Border;
        First=FALSE;
        AbsEnd=AbsEnd || (nChars>=MaxLineChars);
    } while (!(AbsEnd || (Border && ((WordEndCnt>0) || WordEnd || Trenn))));

    if (Border) { // Trennen und Quetschen
        (*WErec0)=(*WErec); WEnChar0=WEnChar;
        AbsEnd=FALSE; c0=0;
        (*R)=(*WErec); nChars=WEnChar;
        (*TRrec)=(*R); TRnChar=nChars;
        Border0=FALSE; Border=FALSE;
        do {                // erst mal gucken wieviele Silben noch reinpassen
            ct=ProcessChar(vOut,TBuf,*TRrec,Atr0,TRnChar,DoTrenn,Line,cLine);
            c=ProcessChar(vOut,TBuf,*R,Atr0,nChars,NoTrenn,Line,cLine);
            AbsEnd=(ct==AbsatzEnd) || (ct==TextEnd) || (nChars>=MaxLineChars);

            Border=TRrec->ChrXP>UmbWdt;
            WordEnd=AbsEnd || ((AbsEnd || (c==' ')) && (c0!=' ') && (c0!=0));
            Trenn=ct=='-';
            if (WordEnd && (!Border0 || (WordEndCnt==0))) {
                WordEndCnt++;
                (*WErec)=(*R0);
                if (AbsEnd) WEnChar=nChars; else WEnChar=nChars-1;
                (*TRrec)=(*R); TRnChar=nChars;                       // zum weitersuchen
            }
            if (Trenn && (!Border || (WordEndCnt==0))) {
                WordEndCnt++;                 // merken, daß man hier trennen kann
                (*WErec)=(*TRrec);
                WEnChar=TRnChar;
                (*TRrec)=(*R); TRnChar=nChars;                       // zum weitersuchen
            }
            (*R0)=(*R); c0=c;
            Border0=Border;
            Border=R->ChrXP>UmbWdt;
        } while (!(AbsEnd || (Border && ((WordEndCnt>0) || WordEnd || Trenn))));

        while (WErec0->Index<WErec->Index) { // damit Line[] auch garantiert stimmt }
            c=ProcessChar(vOut,TBuf,*WErec0,Atr0,WEnChar0,WEnChar-WEnChar0-1,Line,cLine);
        }

        (*R)=(*WErec); nChars=WEnChar;

        if (UmbWdt>=R->ChrXP) {
            BoxRest=UmbWdt-R->ChrXP;
        } else {                                       // Zusammenquetschen
            BoxRest=R->ChrXP-UmbWdt;                     // um soviel muß gequetscht werden
            for (i=2;i<=nChars;i++) {                  // 1. CharPosition bleibt !
                Line[i]-=(i-1)*(BoxRest) /(nChars-1);
            }
            R->ChrXP=UmbWdt;
            Line[nChars+1]=UmbWdt;
        }
    }

    if (!AbsEnd) {
        do {                                         // Leerzeichen weglesen
            (*WErec)=(*R);
            c=GetTextChar(TBuf,R->Index,Atr0,R->Attrib,NoTrenn,FALSE);
            nChars++;
            Line[nChars]=R->ChrXP;
            cLine[nChars]=c;
        } while (c==' ');
        if (c!=' ' && c!=AbsatzEnd && c!=TextEnd) {
            nChars--;
            (*R)=(*WErec);
        }
    }

    if (AbsEnd && nChars<MaxLineChars) { // Ausrichten, statt Blocksatz aber linksbündig
        if (Just==3) Just=0;
        nChars++; Line[nChars]=R->ChrXP; // Damit AbsatzEnde auch weggelesen wird
        Line[nChars+1]=R->ChrXP;         // denn die Breite von CR oder #0 ist nun mal sehr klein
        if (TBuf[R->Index-1]!=AbsatzEnd &&  TBuf[R->Index-1]!=TextEnd) {
            c=GetTextChar(TBuf,R->Index,Atr0,R->Attrib,NoTrenn,FALSE); // Kleine Korrektur. Notig, wenn nur 1 Wort in
        }
    }

    BoxRest=AdjWdt-R->ChrXP;
    if (TextFit) Just=THJustLeft;

    switch (Just) {
        case THJustLeft: break;                                // Links
        case THJustCenter: {
            BoxRest=BoxRest /2;                                // Mitte
            for (i=1;i<=nChars;i++) Line[i]+=BoxRest;
        } break;
        case THJustRight: {                                    // Rechts
            for (i=1;i<=nChars;i++) Line[i]+=BoxRest;
        } break;
        case THJustDrvOut:
        case THJustBlock: {                                    // Block und Austreibend
            re=nChars;
            if (Just==THJustDrvOut) re--;
            while (re>=1 && (cLine[re]==' ' || cLine[re]==TextEnd || cLine[re]==AbsatzEnd)) re--;
            li=1;
            while (li<=re && (cLine[li]==' ' || cLine[li]==TextEnd || cLine[li]==AbsatzEnd)) li++;
            if (Just==THJustDrvOut) BoxRest=AdjWdt-Line[re+1];

            j=0;                        // Anzahl der Spaces ermitteln
            for (i=li;i<=re;i++) {
                if (cLine[i]==' ') {
                    j++;
                }
            }

            if (j==0) {                        // nur 1 Wort ?  -> Strecken !
                for (i=li+1;i<=re;i++) {       // von links nach rechts
                  Line[i]+=MulDiv(i-li,BoxRest,re-li+1-1);
                }
            } else {
                k=0; h=0;
                for (i=li;i<=re;i++) {          // j Spaces aufbohren !
                    if (cLine[i]==' ') {        // Space gefunden !
                        k++;
                        h=MulDiv(k,BoxRest,j);
                    }
                    Line[i]+=h;
                }
            }
            for (i=re+1;i<=nChars;i++) Line[i]+=BoxRest; // und den Rest anpassen
            Line[nChars+1]=AdjWdt;
        } break;
        case THJustLocked: {                                    //Gesperrt
            re=nChars-1;
            while (re>=1 && (cLine[re]==' ' || cLine[re]==TextEnd || cLine[re]==AbsatzEnd)) re--;
            li=1;
            while (li<=re && (cLine[li]==' ' || cLine[li]==TextEnd || cLine[li]==AbsatzEnd)) li++;
            BoxRest=AdjWdt-Line[re+1];
            for (i=li+1;i<=re;i++) {         // Strecken von links nach rechts
                Line[i]+=MulDiv(i-li,BoxRest,re-li+1-1);
            }
            for (i=re+1;i<=nChars;i++) Line[i]+=BoxRest; // und den Rest anpassen
            Line[nChars+1]=AdjWdt;
        } break;
    }
    Index=R->Index;
    AktAtr=R->Attrib;
    delete R;
    delete R0;
    delete WErec;
    delete WErec0;
    delete TRrec;
}



// End of Absatz.Pas
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
// DrawText.Pas

void DrawChar(OutputDevice& rOut, UCHAR c, ObjTextType T, PointType Pos, USHORT DrehWink,
              USHORT FitXMul, USHORT FitXDiv, USHORT FitYMul, USHORT FitYDiv)
{
    SetTextContext(rOut,T,UpcasePossible(c),DrehWink,FitXMul,FitXDiv,FitYMul,FitYDiv);
    if ((T.Schnitt & TextKaptBit)!=0 && UpcasePossible(c)) c=Upcase(c);
    String s( (char)c, RTL_TEXTENCODING_IBM_437 );
    rOut.DrawText( Point( Pos.x, Pos.y ), s );
}

/*************************************************************************
|*
|*    TextType::Draw()
|*
|*    Beschreibung
|*    Ersterstellung    JOE 09.08.93
|*    Letzte Aenderung  JOE 09.08.93
|*
*************************************************************************/
void TextType::Draw(OutputDevice& rOut)
{
    if ((Flags & TextOutlBit)!=0) return;   // Sourcetext für Outliner !!

    ObjTextType T1,T2;
    USHORT Index1;
    USHORT Index2;
    UCHAR  c;
    USHORT l;                // Anzahl der Zeichen in der Zeile
    USHORT i;
    short  yPos0;
    short  xPos;
    short  yPos;
    USHORT LF;
    USHORT MaxGrad;
    short  xSize;
    short  xSAdj;
    short  ySize;
    double sn,cs;
    USHORT TopToBase;
    BOOL   Ende;
    USHORT lc;
    BOOL   LineFit; // FitSize.x=0? oder Flags -> jede Zeile stretchen
    BOOL   TextFit;
    short* xLine;
    UCHAR* cLine;   // Buffer für FormatLine
    USHORT FitXMul;
    USHORT FitXDiv;
    USHORT FitYMul;
    USHORT FitYDiv;
    BOOL   Fehler;
    UCHAR* Buf=Buffer; // Zeiger auf die Buchstaben

    pSgfFonts->ReadList();
    xLine=new short[ChrXPosArrSize];
    cLine=new UCHAR[CharLineSize];

    TextFit=(Flags & TextFitBits)!=0;
    LineFit=FALSE;
    LineFit=((Flags & TextFitZBit)!=0);
    if (TextFit && FitSize.x==0) LineFit=TRUE;

    if (DrehWink==0) {
        sn=0.0;
        cs=1.0;
    } else {
        sn=sin(double(DrehWink)*3.14159265359/18000);
        cs=cos(double(DrehWink)*3.14159265359/18000);
    }

    T1=T; Index1=0; yPos=0; xPos=0;
    if (TextFit) {
        ySize=Pos2.y-Pos1.y;
        xSize=32000 /2;      // Umbruch
        xSAdj=Pos2.x-Pos1.x; // zum Ausrichten bei Zentriert/Blocksatz
        //if (xSize<=0) { xSize=32000 /2; LineFit=TRUE; }
        FitXMul=abs(Pos2.x-Pos1.x); FitXDiv=FitSize.x; if (FitXDiv==0) FitXDiv=1;
        FitYMul=abs(Pos2.y-Pos1.y); FitYDiv=FitSize.y; if (FitYDiv==0) FitYDiv=1;
    } else {
        xSize=Pos2.x-Pos1.x;
        xSAdj=xSize;
        ySize=Pos2.y-Pos1.y;
        FitXMul=1; FitXDiv=1;
        FitYMul=1; FitYDiv=1;
    }
    if (xSize<0) xSize=0;
    if (xSAdj<0) xSAdj=0;

    do {
        T2=T1; Index2=Index1;
        FormatLine(Buf,Index2,T,T2,xSize,xSAdj,xLine,l,sn,cs,cLine,LineFit);
        Fehler=(Index2==Index1);
        if (!Fehler) {
            lc=GetLineFeed(Buf,Index1,T,T1,l,LF,MaxGrad);
            if (TextFit) {
                if (LineFit) FitXDiv=xLine[lc+1];
                if (FitXDiv>0) {
                    long Temp;
                    for (i=1;i<=l+1;i++) {
                        Temp=long(xLine[i])*long(FitXMul) /long(FitXDiv);
                        xLine[i]=short(Temp);
                    }
                    LF=MulDiv(LF,FitYMul,FitYDiv);
                    MaxGrad=MulDiv(MaxGrad,FitYMul,FitYDiv);
                } else {
                    FitXDiv=1; // 0 gibts nicht
                }
            }
            yPos0=yPos;
            TopToBase=GetTopToBaseLine(MaxGrad);
            yPos+=TopToBase;
            Ende=(yPos0+short(MulDiv(MaxGrad,CharTopToBtm,100))>ySize) && !TextFit;
            if (!Ende) {
                T2=T1; Index2=Index1;
                i=1;
                while (i<=l) {
                    c=GetTextCharConv(Buf,Index2,T,T2,l-i,FALSE);
                    long xp1,yp1;       // wegen Overflowgefahr
                    PointType Pos;
                    xp1=long(Pos1.x)+xPos+long(xLine[i]);
                    yp1=long(Pos1.y)+yPos;
                    if (xp1>32000) xp1=32000; if (xp1<-12000) xp1=-12000;
                    if (yp1>32000) yp1=32000; if (yp1<-12000) yp1=-12000;
                    Pos.x=short(xp1);
                    Pos.y=short(yp1);

                    if (DrehWink!=0) RotatePoint(Pos,Pos1.x,Pos1.y,sn,cs);
                    DrawChar(rOut,c,T2,Pos,DrehWink,FitXMul,FitXDiv,FitYMul,FitYDiv);
                    i++;
                } // while i<=l
                yPos=yPos0+LF;
                T1=T2; Index1=Index2; // Für die nächste Zeile
            } // if ObjMin.y+yPos<=Obj_Max.y
        } // if !Fehler
    } while (c!=TextEnd && !Ende && !Fehler);
    delete cLine;
    delete xLine;
}

// End of DrawText.Pas
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

// nicht mehr benötigt, da der Pointer nun extra geführt wird
// (DEC Alpha hat nämlich 64Bit-Pointer!)
//UCHAR* TextType::GetBufPtr()
//{
//    ULONG Temp;
//    Temp=ULONG(BufLo)+0x00010000*ULONG(BufHi);
//    return (UCHAR*)Temp;
//}
//
//void TextType::SetBufPtr(UCHAR* Ptr)
//{
//    ULONG Temp=(ULONG)Ptr;
//    BufLo=USHORT(Temp & 0x0000FFFF);
//    BufHi=USHORT((Temp & 0xFFFF0000)>>16);
//}

UINT32 ObjTextType::GetFont()
{
    return ULONG(FontLo)+0x00010000*ULONG(FontHi);
}

void ObjTextType::SetFont(UINT32 FontID)
{
    FontLo=USHORT(FontID & 0x0000FFFF);
    FontHi=USHORT((FontID & 0xFFFF0000)>>16);
}


/////////////////////////////////////////////////////////////////////////////////
// SGF.Ini lesen ////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
SgfFontOne::SgfFontOne()
{
    Next=NULL;
    IFID=0;
    Bold=FALSE;
    Ital=FALSE;
    Sans=FALSE;
    Serf=FALSE;
    Fixd=FALSE;
    SVFamil=FAMILY_DONTKNOW;
    SVChSet=RTL_TEXTENCODING_DONTKNOW;
    SVWidth=40;
}

void SgfFontOne::ReadOne( ByteString& ID, ByteString& Dsc )
{
    USHORT i,j,n;
    ByteString s;

    if ( Dsc.Len() < 4 || ( Dsc.GetChar( 0 ) != '(' ) )
        return;
    i=1;   // Erster Buchstabe des IF-Fontnamen. Davor ist eine '('
    while ( i < Dsc.Len() && ( Dsc.GetChar( i ) !=')' ) )
        i++;
    Dsc.Erase(0,i+1);                                // IF-Fontname löschen inkl. ()

    if ( Dsc.Len() < 2 || ( Dsc.GetChar( Dsc.Len() - 1 ) !=')' ) )
        return;
    i=Dsc.Len()-2;                                   // hier ist die ')' des SV-Fontnames
    j=0;
    while ( i > 0 && ( Dsc.GetChar( i ) != '(' ) )
    {
        i--;
        j++;
    }
    SVFName=String(Dsc,i+1,j);                       // SV-Fontname rausholen
    Dsc.Erase(i,j);

    IFID = (UINT32)ID.ToInt32();
    n=Dsc.GetTokenCount(' ');
    for (i=0;i<n;i++)
    {
        s = Dsc.GetToken( i,' ' );
        if ( s.Len() )
        {
            s.ToUpperAscii();
            if      ( s.CompareTo( "BOLD", 4 ) == COMPARE_EQUAL ) Bold=TRUE;
            else if ( s.CompareTo( "ITAL", 4 ) == COMPARE_EQUAL ) Ital=TRUE;
            else if ( s.CompareTo( "SERF", 4 ) == COMPARE_EQUAL ) Serf=TRUE;
            else if ( s.CompareTo( "SANS", 4 ) == COMPARE_EQUAL ) Sans=TRUE;
            else if ( s.CompareTo( "FIXD", 4 ) == COMPARE_EQUAL ) Fixd=TRUE;
            else if ( s.CompareTo( "ROMAN", 5 ) == COMPARE_EQUAL ) SVFamil=FAMILY_ROMAN;
            else if ( s.CompareTo( "SWISS", 5 ) == COMPARE_EQUAL ) SVFamil=FAMILY_SWISS;
            else if ( s.CompareTo( "MODERN", 6 ) == COMPARE_EQUAL ) SVFamil=FAMILY_MODERN;
            else if ( s.CompareTo( "SCRIPT", 6 ) == COMPARE_EQUAL ) SVFamil=FAMILY_SCRIPT;
            else if ( s.CompareTo( "DECORA", 6 ) == COMPARE_EQUAL ) SVFamil=FAMILY_DECORATIVE;
            else if ( s.CompareTo( "ANSI", 4 ) == COMPARE_EQUAL ) SVChSet=RTL_TEXTENCODING_MS_1252;
            else if ( s.CompareTo( "IBMPC", 5 ) == COMPARE_EQUAL ) SVChSet=RTL_TEXTENCODING_IBM_850;
            else if ( s.CompareTo( "MAC", 3 ) == COMPARE_EQUAL ) SVChSet=RTL_TEXTENCODING_APPLE_ROMAN;
            else if ( s.CompareTo( "SYMBOL", 6 ) == COMPARE_EQUAL ) SVChSet=RTL_TEXTENCODING_SYMBOL;
            else if ( s.CompareTo( "SYSTEM", 6 ) == COMPARE_EQUAL ) SVChSet = gsl_getSystemTextEncoding();
            else if ( s.IsNumericAscii() ) SVWidth=s.ToInt32();
        }
    }
}

/////////////////////////////////////////////////////////////////////////////////

SgfFontLst::SgfFontLst()
{
    pList=NULL;
    Last=NULL;
    LastID=0;
    LastLn=NULL;
    Tried=FALSE;
}

SgfFontLst::~SgfFontLst()
{
    RausList();
}

void SgfFontLst::RausList()
{
    SgfFontOne* P;
    SgfFontOne* P1;
    P=pList;
    while (P!=NULL) {
        P1=P->Next;
        delete P;
        P=P1;
    }
    pList=NULL;
    Last=NULL;
    Tried=FALSE;
    LastID=0;
    LastLn=NULL;
}

void SgfFontLst::AssignFN(const String& rFName)
{   FNam=rFName;   }

void SgfFontLst::ReadList()
{
    if (!Tried) {
        Tried=TRUE;
        LastID=0;
        LastLn=NULL;
        SgfFontOne* P,P1;
        Config aCfg(FNam);
        aCfg.SetGroup("SGV Fonts fuer StarView");
        USHORT Anz=aCfg.GetKeyCount();
        USHORT i;
        ByteString FID,Dsc;

        for (i=0;i<Anz;i++)
        {
            FID = aCfg.GetKeyName( i );
            FID = FID.EraseAllChars(); // Leerzeichen weg
            Dsc = aCfg.ReadKey( i );
            if ( FID.IsNumericAscii() )
            {
                P=new SgfFontOne;                                   // neuer Eintrag
                if (Last!=NULL) Last->Next=P; else pList=P; Last=P; // einklinken
                P->ReadOne(FID,Dsc);                                // und Zeile interpretieren
            }
        }
    }
}

SgfFontOne* SgfFontLst::GetFontDesc(UINT32 ID)
{
    if (ID!=LastID) {
        SgfFontOne* P;
        P=pList;
        while (P!=NULL && P->IFID!=ID) P=P->Next;
        LastID=ID;
        LastLn=P;
    }
    return LastLn;
}
