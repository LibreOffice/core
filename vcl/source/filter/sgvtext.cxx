/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include "sal/config.h"

#include <osl/thread.h>
#include <rtl/math.h>
#include <comphelper/string.hxx>
#include <tools/config.hxx>
#include <tools/fract.hxx>
#include <vcl/graphicfilter.hxx>
#include "sgffilt.hxx"
#include "sgfbram.hxx"
#include "sgvmain.hxx"
#include <memory>
#include <cstdlib>

extern SgfFontLst* pSgfFonts;

//  Limitations:  only grey shadows, 2D and with fixed distance.
// Start of AbsBase.Pas

// vvv special characters in text buffer vvv
#define  TextEnd        0 /* ^@ end of character sequence                       */
#define  HardSpace      6 /* ^F non-breaking space,' '                          */
#define  SoftTrennK    11 /* ^K character for k-c-switch for separation, 'k'    */
#define  AbsatzEnd     13 /* ^M new paragraph=CR                                */
#define  HardTrenn     16 /* ^P hard separator (not wrapped), '-'               */
#define  SoftTrennAdd  19 /* ^S extra-character, separates e.g. "schiff-fahrt"  */
#define  Escape        27 /* ^[ start of escape sequence                        */
#define  SoftTrenn     31 /* ^_ soft separator, '-' only end of line            */
#define  MaxEscValLen  8
#define  MaxEscLen     (MaxEscValLen+3)

// Escape sequences: [Esc]<Ident><Value>[Esc]  at least 4 characters
// Max length of value should be: 8 chars (7+sign). Therefore max. length
// of a escape sequence: 11 chars.
// Identifer:

#define  EscFont   'F' /* fontID, e.g. 92500 for CG Times                           */
#define  EscGrad   'G' /* font angle 1..255 for <<Pt-127<<Pt                        */
#define  EscBreit  'B' /* width 1..255% of the font angle                           */
#define  EscKaptS  'K' /* uppercase size 1..255% of the text angle                  */
#define  EscLFeed  'L' /* character spacing 1..32767% of max. text angle of the line*/
                        // or 1..32767 for 1..16383<<Pt absolute (if bit 15=1)
#define  EscSlant  'S' /* italic (angle) 1..8999 for 0.01deg..89.99deg              */
#define  EscVPos   'V' /* character vertical position 1..255 for <<Pt..127<<Pt       */
#define  EscZAbst  'Z' /* character spacing -128..127%                              */
#define  EscHJust  'A' /* H-justify  paragraph: left, cent, right, block, aligned, fixed (0..5)*/

#define  EscFarbe  'C' /* colour 0..7                                               */
#define  EscBFarb  'U' /* background colour 0..7                                    */
#define  EscInts   'I' /* colour intensity 0..100%                                  */
#define  EscMustr  'M' /* pattern 0..? incl. transp...                              */
#define  EscMFarb  'O' /* pattern colour 0..7                                       */
#define  EscMBFrb  'P' /* 2nd pattern colour 0..7                                   */
#define  EscMInts  'W' /* pattern intensity 0..7                                    */

#define  EscSMstr  'E' /* shadow pattern 0..? inkl. Transp...                       */
#define  EscSFarb  'R' /* shadow colour 0..7                                        */
#define  EscSBFrb  'T' /* 2nd shadow colour 0..7                                    */
#define  EscSInts  'Q' /* shadow intensity 0..7                                     */

#define  EscSXDst  'X' /* shadow offset X 0..100%                                   */
#define  EscSYDst  'Y' /* shadow offset Y 0..100%                                   */
#define  EscSDist  'D' /* shadow offset X-Y 0..100%                                 */

#define  EscBold   'f' /* bold                                                      */
#define  EscLSlnt  'l' /* L italic                                                  */
#define  EscRSlnt  'r' /* R italic                                                  */
#define  EscUndln  'u' /* underlined                                                */
#define  EscDbUnd  'p' /* double underlined                                         */
#define  EscKaptF  'k' /* uppercase flag                                            */
#define  EscStrik  'd' /* strikethrough                                             */
#define  EscDbStk  'e' /* double strikethrough                                      */
#define  EscSupSc  'h' /* superscript                                               */
#define  EscSubSc  't' /* subscript                                                 */
#define  Esc2DShd  's' /* 2D-shadow                                                 */
#define  Esc3DShd  'j' /* 3D-shadow                                                 */
#define  Esc4DShd  'i' /* 4D-shadow                                                 */
#define  EscEbShd  'b' /* embossed                                                  */

//  AllEscIdent =[EscFont, EscGrad, EscBreit,EscKaptS,EscLFeed,EscSlant,EscVPos, EscZAbst,EscHJust,
//                EscFarbe,EscBFarb,EscInts, EscMustr,EscMFarb,EscMBFrb,EscMInts,
//                EscSMstr,EscSFarb,EscSBFrb,EscSInts,EscSXDst,EscSYDst,EscSDist,
//                EscBold, EscLSlnt,EscRSlnt,EscUndln,EscDbUnd,EscKaptF,EscStrik,EscDbStk,
//                EscSupSc,EscSubSc,Esc2DShd,Esc3DShd,Esc4DShd];
// Justify muss spaetestens am Anfang des Absatzes stehen
#define  EscSet    '\x1e' /* set flag                                               */
#define  EscReset  '\x1f' /* reset flag                                             */
#define  EscDeflt  '\x11' /* set flag to default                                    */
#define  EscToggl  '\x1d' /* toggle flag                                            */
#define  EscNoFlg  0
#define  EscNoVal  -2147483647 /* -MaxLongInt */

#define  NoTrenn 0xFFFF   /* parameter value for 'Rest' of GetTextChar(), if separation should not occur */
#define  DoTrenn 0xFFFE   /* parameter value for 'Rest' of GetTextChar(), if separtion should occur      */

#define  MaxLineChars 1024

#define  ChrXPosArrSize (MaxLineChars+1+1)           /* 2k - start with 0 opposite to StarDraw */
#define  CharLineSize   (MaxLineChars+1+1)

#define  MinChar 32
#define  MaxChar 255

#define  CharTopToBase     100 /* due to quotes more as 75%         */
#define  CharTopToBtm      120 /* line height larger as text angle  */
                               // for Avanti-Bold 'ue' actually even 130%

// end of AbsBase.Pas

// Start of DefBase.Pas

#define  TextBoldBit  0x0001   /* bold                   */
#define  TextRSlnBit  0x0002   /* italic                 */
#define  TextUndlBit  0x0004   /* underline              */
#define  TextStrkBit  0x0008   /* strikethrough          */
#define  TextSupSBit  0x0010   /* superscript            */
#define  TextSubSBit  0x0020   /* subscript              */
#define  TextKaptBit  0x0040   /* capital                */
#define  TextLSlnBit  0x0080   /* left italic            */
#define  TextDbUnBit  0x0100   /* double underline       */
#define  TextDbStBit  0x0200   /* double strikethrough   */
#define  TextSh2DBit  0x0400   /* 2D-shadow         2.0  */
#define  TextSh3DBit  0x0800   /* 3D-shadow         2.0  */
#define  TextSh4DBit  0x1000   /* 4D-shadow         2.0  */
#define  TextShEbBit  0x2000   /* embossed-shadow   2.0  */

#define  THJustLeft     0x00
#define  THJustCenter   0x01
#define  THJustRight    0x02
#define  THJustBlock    0x03
#define  THJustDrvOut   0x04   /* Justified         formatted */
#define  THJustLocked   0x05   /* J u s t   l  o  c  k  e  d */

#define  MaxCharSlant   4200   /* maximum 42deg italic ! */

// end of DefBase.Pas

bool CheckTextOutl(ObjAreaType& F, ObjLineType& L)
{
    return (F.FIntens!=L.LIntens) ||
           ((F.FFarbe!=L.LFarbe)   && (F.FIntens>0)) ||
           ((F.FBFarbe!=L.LBFarbe) && (F.FIntens<100));
}

// Start of Misc.Pas

short hPoint2Sgf(short a)
{
    long b;
    b=long(a)*127*SgfDpmm/(144*5);
    return short(b);
}

// End of Misc.Pas

// Start of AbsRead.Pas

// Function GetTopToBaseLine()  Function GetBaseLineToBtm()
//
// Calculate distance from ascender of line to baseline or from baseline to
// descender. All in SGF-units.

sal_uInt16 GetTopToBaseLine(sal_uInt16 MaxGrad)
{
    long ret;
    ret=long(MaxGrad)*long(CharTopToBase) /long(100);
    return sal_uInt16(ret);
}

// Function GetTextChar()   Function GetTextCharConv()
//
// Reads a character from textbuffer, in doing so escape sequences
// are evaluated and accordingly the input/output parameter AktAtr is set.
// Index is incremented accordingly.
// Parameter Rest should always contain the number of characters,
// the number of remaining characters in that line.
// Otherwise the hyphenation does not work. If the constant NoTrenn
// is used instead, no hyphenation is done. To the contrary then
// constant DoTrenn triggers hyphenation where a soft-break is present.
//
// Soft separators are converted to a minus sign.
// On top GetTextCharConv() converts HardSpace and AbsatzEnde
// in spaces, including HardTrenner in minus signs. TextEnde is
// always returned as Char(0).

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

long ChgValue(long Def, long Min, long Max, UCHAR FlgVal, long NumVal)
{
    long r=0;

    if (FlgVal==EscDeflt) {
        r=Def;                          // return to default
    } else {
        if (NumVal!=EscNoVal) r=NumVal; // set non-breaking
    }

    if (Min!=0 || Max!=0) {
        if (r>Max) r=Max;
        if (r<Min) r=Min;
    }
    return r;
}

void ChgSchnittBit(sal_uInt16 Bit, sal_uInt16 Radio1, sal_uInt16 Radio2, sal_uInt16 Radio3,
                   UCHAR FlgVal, sal_uInt16 Schnitt0, sal_uInt16& Schnitt)
{
    sal_uInt16 All,Rad;

    Rad=Radio1 | Radio2 | Radio3;
    All=Bit | Rad;

    switch (FlgVal) {
        case EscSet  : Schnitt=(Schnitt & ~All) | Bit;              break;
        case EscReset: Schnitt=(Schnitt & ~All);                    break;
        case EscDeflt: Schnitt=(Schnitt & ~All) | (Schnitt0 & All); break;
        case EscToggl: Schnitt=(Schnitt & ~Rad) ^ Bit;
    }
}

UCHAR GetNextChar(UCHAR* TBuf, sal_uInt16 Index)
{
    sal_uInt16 Cnt;
    while (TBuf[Index]==Escape) {
        Index++;
        Cnt=0;
        while (TBuf[Index]!=Escape && Cnt<=MaxEscLen) {
            Index++; Cnt++; }
        Index++;
    }
    return TBuf[Index];
}

UCHAR ProcessOne(UCHAR* TBuf, sal_uInt16& Index,
                 ObjTextType& Atr0, ObjTextType& AktAtr,
                 bool ScanEsc)
{
    UCHAR c;
    UCHAR Ident;
    bool  Ende;
    bool  q;
    UCHAR FlgVal;
    long  NumVal;
    long  Sgn;
    short i;
    bool  EoVal;

    do {
        c=TBuf[Index]; Index++;
        Ende=(c!=Escape);
        if (!Ende) {
            c=TBuf[Index]; Index++;
            Ident=c;                          // remember identifier
            FlgVal=EscNoFlg;
            NumVal=EscNoVal;
            c=TBuf[Index]; Index++;            // value starts here
            if (c==EscSet || c==EscReset || c==EscDeflt || c==EscToggl) FlgVal=c; else {
                if (c=='-') Sgn=-1; else Sgn=1;
                if (c=='+' || c=='-') { c=TBuf[Index]; Index++; }
                i=MaxEscValLen;
                NumVal=0;
                do {
                    NumVal=10*NumVal+c-'0';
                    EoVal=(TBuf[Index]<'0' || TBuf[Index]>'9');
                    if (!EoVal) { c=TBuf[Index]; Index++; }
                    i--;
                } while (i>0 && !EoVal);
                NumVal=Sgn*NumVal;
            }
            q=!CheckTextOutl(AktAtr.F,AktAtr.L);

            switch (Ident) {
                case EscFont : AktAtr.SetFont(sal_uLong (ChgValue(Atr0.GetFont(),0,0          ,FlgVal,NumVal)));break;
                case EscGrad : AktAtr.Grad   =sal_uInt16(ChgValue(Atr0.Grad,   2,2000         ,FlgVal,NumVal)); break;
                case EscBreit: AktAtr.Breite =sal_uInt16(ChgValue(Atr0.Breite, 1,1000         ,FlgVal,NumVal)); break;
                case EscKaptS: AktAtr.Kapit  =(sal_uInt8)(ChgValue(Atr0.Kapit,  1,255          ,FlgVal,NumVal)); break;
                case EscLFeed: AktAtr.LnFeed =sal_uInt16(ChgValue(Atr0.LnFeed, 1,65535        ,FlgVal,NumVal)); break;
                case EscSlant: AktAtr.Slant  =sal_uInt16(ChgValue(Atr0.Slant,  1,MaxCharSlant ,FlgVal,NumVal)); break;
                case EscVPos : AktAtr.ChrVPos=char  (ChgValue(Atr0.ChrVPos,-128,127       ,FlgVal,NumVal)); break;
                case EscZAbst: AktAtr.ZAbst  =(sal_uInt8)(ChgValue(Atr0.ZAbst,  1,255          ,FlgVal,NumVal)); break;
                case EscHJust: AktAtr.Justify=(sal_uInt8)(ChgValue(Atr0.Justify & 0x0F,0,5     ,FlgVal,NumVal)); break;
                case EscFarbe: { AktAtr.L.LFarbe =(sal_uInt8)(ChgValue(Atr0.L.LFarbe,0,7   ,FlgVal,NumVal)); if (q) AktAtr.F.FFarbe =AktAtr.L.LFarbe;  } break;
                case EscBFarb: { AktAtr.L.LBFarbe=(sal_uInt8)(ChgValue(Atr0.L.LBFarbe,0,255,FlgVal,NumVal)); if (q) AktAtr.F.FBFarbe=AktAtr.L.LBFarbe; } break;
                case EscInts : { AktAtr.L.LIntens=(sal_uInt8)(ChgValue(Atr0.L.LIntens,0,100,FlgVal,NumVal)); if (q) AktAtr.F.FIntens=AktAtr.L.LIntens; } break;

                case EscMustr: { AktAtr.F.FMuster=sal_uInt16(ChgValue(Atr0.F.FMuster,0,65535,FlgVal,NumVal)); } break;
                case EscMFarb: { AktAtr.F.FFarbe =(sal_uInt8)(ChgValue(Atr0.F.FFarbe,0,7   ,FlgVal,NumVal));   } break;
                case EscMBFrb: { AktAtr.F.FBFarbe=(sal_uInt8)(ChgValue(Atr0.F.FBFarbe,0,255,FlgVal,NumVal));   } break;
                case EscMInts: { AktAtr.F.FIntens=(sal_uInt8)(ChgValue(Atr0.F.FIntens,0,100,FlgVal,NumVal));   } break;

                case EscSMstr: { AktAtr.ShdF.FMuster=sal_uInt16(ChgValue(Atr0.ShdF.FMuster,0,65535,FlgVal,NumVal)); } break;
                case EscSFarb: { AktAtr.ShdL.LFarbe =(sal_uInt8)(ChgValue(Atr0.ShdL.LFarbe,0,7   ,FlgVal,NumVal)); AktAtr.ShdF.FFarbe =AktAtr.ShdL.LFarbe;  } break;
                case EscSBFrb: { AktAtr.ShdL.LBFarbe=(sal_uInt8)(ChgValue(Atr0.ShdL.LBFarbe,0,255,FlgVal,NumVal)); AktAtr.ShdF.FBFarbe=AktAtr.ShdL.LBFarbe; } break;
                case EscSInts: { AktAtr.ShdL.LIntens=(sal_uInt8)(ChgValue(Atr0.ShdL.LIntens,0,100,FlgVal,NumVal)); AktAtr.ShdF.FIntens=AktAtr.ShdL.LIntens; } break;
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
            if (TBuf[Index]==Escape) Index++;         // read 2nd escape }
        }
    } while (!Ende && !ScanEsc);
    if (!Ende) c=Escape;
    return c;
} // end of ProcessOne

UCHAR GetTextChar(UCHAR* TBuf, sal_uInt16& Index,
                  ObjTextType& Atr0, ObjTextType& AktAtr,
                  sal_uInt16 Rest, bool ScanEsc)
{
    UCHAR c=ProcessOne(TBuf,Index,Atr0,AktAtr,ScanEsc);
    if (!ScanEsc) {
        if (c==SoftTrennAdd || c==SoftTrennK || c==SoftTrenn) {
            UCHAR nc=GetNextChar(TBuf,Index);
            UCHAR c0=c;
            if (Rest==0 || Rest==DoTrenn ||
                nc==' ' || nc==AbsatzEnd || nc==TextEnd) c='-';
            else {
                c=ProcessOne(TBuf,Index,Atr0,AktAtr,ScanEsc); // skip separator
                if (c0==SoftTrennAdd) {
                    if (c>=32) c=ProcessOne(TBuf,Index,Atr0,AktAtr,ScanEsc); // skip another letter
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

  // HardSpace and HardTrenn should be converted explicitely ! }
  // if AktAtr.Schnitt and TextKaptBit =TextKaptBit then c:=UpCase(c);(explizit) }

  // The separationmethod SoftTrennAdd assumes, the separating consonant }
  // is 3x present in TextBuf, e.g.: "schiff-fahrt".   }
  // If not separated then, "-f" is removed.        }

UCHAR GetTextCharConv(UCHAR* TBuf, sal_uInt16& Index,
                      ObjTextType& Atr0, ObjTextType& AktAtr,
                      sal_uInt16 Rest, bool ScanEsc)
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

// Function GetLineFeed()
//
// Required line spacing in SGF-Units. ChrVPos is taken into account.
// ======================================================================
sal_uInt16 GetLineFeed(UCHAR* TBuf, sal_uInt16 Index, ObjTextType Atr0, ObjTextType AktAtr,
                   sal_uInt16 nChar, sal_uInt16& LF, sal_uInt16& MaxGrad)
{
    bool   AbsEnd=false;
    sal_uLong  LF100=0;
    sal_uLong  MaxLF100=0;
    bool   LFauto=false;
    bool   First=true;
    sal_uInt16 Grad;
    sal_uInt16 i=0;
    sal_uInt16 r=1;

    MaxGrad=0;
    while (!AbsEnd && nChar>0) {
        nChar--;
        UCHAR c=GetTextChar(TBuf,Index,Atr0,AktAtr,nChar,false);
        i++;
        AbsEnd=(c==TextEnd || c==AbsatzEnd);
        if (First || (!AbsEnd && c!=' ' && c!=HardTrenn)) {
            LFauto=(AktAtr.LnFeed & 0x8000)==0;
            LF100=AktAtr.LnFeed & 0x7FFF;
            if (LFauto) LF100=LF100*AktAtr.Grad; else LF100*=LF100;
            if (AktAtr.ChrVPos>0) LF100-=AktAtr.ChrVPos*100;
            if (LF100>MaxLF100) MaxLF100=LF100;
            Grad=AktAtr.Grad;
            if (AktAtr.ChrVPos>0) Grad=Grad-AktAtr.ChrVPos;
            if (Grad>MaxGrad) MaxGrad=Grad;
            First=false;
        }
        if (!AbsEnd && c!=' ') r=i;
    }
    MaxGrad=hPoint2Sgf(MaxGrad);
    if (MaxLF100<=4000) {  // otherwise overflow could occur
        LF=sal_uInt16(hPoint2Sgf(short(MaxLF100)) /100);
    } else {
        LF=sal_uInt16(hPoint2Sgf(short(MaxLF100) /100));
    }

    return r;
}

// End of AbsRead.Pas

// Start of iFont.Pas

#define SuperSubFact 60     /* superscript/subscript: 60% of text angle */
#define DefaultSpace 40     /* Default: space is 40% of text angle      */

sal_uInt16 SetTextContext(OutputDevice& rOut, ObjTextType& Atr, bool Kapt, sal_uInt16 nRotation,
                      sal_uInt16 FitXMul, sal_uInt16 FitXDiv, sal_uInt16 FitYMul, sal_uInt16 FitYDiv)
{
    SgfFontOne* pSgfFont; // Font from the IniFile
    vcl::Font   aFont;
    Color  aColor;
    sal_uLong  Grad;
    sal_uLong  Brei;
    OUString FNam;
    sal_uInt16 StdBrei=50;    // average line width in % of text angle
    bool   bFit=(FitXMul!=1 || FitXDiv!=1 || FitYMul!=1 || FitYDiv!=1);

    pSgfFont = pSgfFonts->GetFontDesc(Atr.GetFont());

    if ( pSgfFont!=nullptr )
    {
        FNam   =pSgfFont->SVFName;
        StdBrei=pSgfFont->SVWidth;
        if (pSgfFont->Fixd) aFont.SetPitch(PITCH_FIXED); else aFont.SetPitch(PITCH_VARIABLE);
        aFont.SetFamily(pSgfFont->SVFamil);
        aFont.SetCharSet(pSgfFont->SVChSet);
        aFont.SetName(FNam);
    }
    else
    {  // if not in Inifile, some fonts are hard coded here
        aFont.SetPitch(PITCH_VARIABLE);
        switch (Atr.GetFont()) {
          case 92500: case 92501: case 92504: case 92505:
          {
#if defined(WNT)
              FNam = "Times New Roman";  // CG Times is Times New Roman in Windows
#else
              FNam = "Times";            // otherwise just Times
#endif
              StdBrei=40;
              aFont.SetFamily(FAMILY_ROMAN);
          } break;
          case 94021: case 94022: case 94023: case 94024: {
#if defined(WNT)
              FNam = "Arial";            // Univers is Arial in Windows
#else
              FNam = "Helvetica";        // otherwise Helvetica
#endif
              aFont.SetFamily(FAMILY_SWISS);
              StdBrei=47;
          } break;
          case 93950: case 93951: case 93952: case 93953: {
#if defined(WNT)
              FNam = "Courier New";      // The vector-Courierfont is called Courier New in Windows
#else
              FNam = "Courier";          // otherwise Courier remains Courier
#endif
              aFont.SetFamily(FAMILY_ROMAN);
              aFont.SetPitch(PITCH_FIXED);
          } break;
          default: FNam = "Helvetica";
        }
        aFont.SetName(FNam);
        //aFont.SetCharSet(CHARSET_SYSTEM);
    }

    Grad=sal_uLong(Atr.Grad);
    if ((Atr.Schnitt & TextKaptBit) !=0 && Kapt) Grad=Grad*sal_uLong(Atr.Kapit)/100;
    if ((Atr.Schnitt & TextSupSBit) !=0 || (Atr.Schnitt & TextSubSBit) !=0) Grad=Grad*SuperSubFact/100;
    Brei=Grad;
    if (Atr.Breite!=100 || bFit) {
        if (bFit) {
            Grad=Grad*sal_uLong(FitYMul)/sal_uLong(FitYDiv);
            Brei=Brei*sal_uLong(FitXMul)/sal_uLong(FitXDiv);
        }
        Brei=Brei*sal_uLong(Atr.Breite)/100;
        Brei=Brei*sal_uLong(StdBrei)/100;
        aFont.SetSize(Size(hPoint2Sgf(sal_uInt16(Brei)),hPoint2Sgf(sal_uInt16(Grad))));
    } else {
        aFont.SetSize(Size(0,hPoint2Sgf(sal_uInt16(Grad))));
    }

    aColor=Sgv2SvFarbe(Atr.L.LFarbe,Atr.L.LBFarbe,Atr.L.LIntens); aFont.SetColor(aColor);
    aColor=Sgv2SvFarbe(Atr.F.FFarbe,Atr.F.FBFarbe,Atr.F.FIntens); aFont.SetFillColor(aColor);
    aFont.SetTransparent(true);
    aFont.SetAlign(ALIGN_BASELINE);

    nRotation/=10; nRotation=3600-nRotation; if (nRotation==3600) nRotation=0;
    aFont.SetOrientation(nRotation);

    if ((Atr.Schnitt & TextBoldBit) !=0) aFont.SetWeight(WEIGHT_BOLD);
    if ((Atr.Schnitt & TextRSlnBit) !=0) aFont.SetItalic(ITALIC_NORMAL);
    if ((Atr.Schnitt & TextUndlBit) !=0) aFont.SetUnderline(UNDERLINE_SINGLE);
    if ((Atr.Schnitt & TextDbUnBit) !=0) aFont.SetUnderline(UNDERLINE_DOUBLE);
    if ((Atr.Schnitt & TextStrkBit) !=0) aFont.SetStrikeout(STRIKEOUT_SINGLE);
    if ((Atr.Schnitt & TextDbStBit) !=0) aFont.SetStrikeout(STRIKEOUT_DOUBLE);
    if ((Atr.Schnitt & TextSh2DBit) !=0) aFont.SetShadow(true);
    if ((Atr.Schnitt & TextSh3DBit) !=0) aFont.SetShadow(true);
    if ((Atr.Schnitt & TextSh4DBit) !=0) aFont.SetShadow(true);
    if ((Atr.Schnitt & TextShEbBit) !=0) aFont.SetShadow(true);
    if (CheckTextOutl(Atr.F,Atr.L)) aFont.SetOutline(true);

    if (aFont!=rOut.GetFont()) rOut.SetFont(aFont);

    return 0;
}

// iFont.Pas

// Start of Absatz.Pas

struct ProcChrSta {
    sal_uInt16 Index;
    sal_uInt16 ChrXP;
    UCHAR  OutCh;
    bool   Kapt;
    ObjTextType Attrib;
};

void InitProcessCharState(ProcChrSta& State, ObjTextType& AktAtr, sal_uInt16 IndexA)
{
    State.Attrib=AktAtr;
    State.OutCh=0;
    State.Index=IndexA;
    State.ChrXP=0;
    State.Kapt=false;
}

bool UpcasePossible(UCHAR c)
{
    return ((c>='a' && c<='z') || c == 0xe4 || c == 0xf6 || c == 0xfc );
}

UCHAR Upcase(UCHAR c)
{
    if ((c>=(UCHAR)'a' && c<=(UCHAR)'z')) c=(c-(UCHAR)'a')+(UCHAR)'A';
    else if ( c == 0xe4 ) c = 0xc4;
    else if ( c == 0xf6 ) c = 0xd6;
    else if ( c == 0xfc ) c = 0xdc;
    return c;
}

sal_uInt16 GetCharWidth(OutputDevice& rOut, UCHAR c)
{
    sal_uInt16 ChrWidth;

    if (c==' ')
    {
        ChrWidth=(sal_uInt16)rOut.GetTextWidth( OUString('A') );
        if (rOut.GetFont().GetPitch()!=PITCH_FIXED) {
            ChrWidth=MulDiv(ChrWidth,DefaultSpace,100);
        }
    }
    else
    {
        static_assert(MaxChar == 255, "with MaxChar == 255 c cannot be greater than MaxChar");
        static_assert(sizeof(UCHAR) == 1, "assert if MaxChar type is ever changed");
        if (c>=MinChar /*&& c<=MaxChar*/)
        {
            ChrWidth=(sal_uInt16)rOut.GetTextWidth(OUString(reinterpret_cast<sal_Char*>(&c), 1, RTL_TEXTENCODING_IBM_437));
        }
        else
        {
            ChrWidth=(sal_uInt16)rOut.GetTextWidth(OUString(static_cast<sal_Unicode>('A')));
        }
    }
    return ChrWidth;
}

UCHAR ProcessChar(OutputDevice& rOut, UCHAR* TBuf, ProcChrSta& R, ObjTextType& Atr0,
                  sal_uInt16& nChars, sal_uInt16 Rest,
                  short* Line, UCHAR* cLine)
{
    sal_uInt16       KernDist=0;       // value for kerning
    sal_uInt16       ChrWidth;
    UCHAR        c;
    bool         AbsEnd;

    c=GetTextChar(TBuf,R.Index,Atr0,R.Attrib,Rest,false); // tries to separate, if Rest contains appropriate value

    AbsEnd=(c==AbsatzEnd || c==TextEnd);
    if (!AbsEnd) {
        R.OutCh=ConvertTextChar(c); // from HardTrenn to '-', ...
        R.Kapt=(R.Attrib.Schnitt & TextKaptBit) !=0 && UpcasePossible(R.OutCh);
        if (R.Kapt) R.OutCh=Upcase(R.OutCh);
        SetTextContext(rOut,R.Attrib,R.Kapt,0,1,1,1,1);

        UCHAR c1 = (R.Kapt)?Upcase(c):c;
        ChrWidth=GetCharWidth(rOut,c1);

        if (R.Attrib.ZAbst!=100) { // special line distance ?
            sal_uLong Temp;
            Temp=sal_uLong(ChrWidth)*sal_uLong(R.Attrib.ZAbst)/100;
            ChrWidth=sal_uInt16(Temp);
        }
        nChars++;
        if (R.ChrXP>32000) R.ChrXP=32000;
        Line[nChars]=R.ChrXP-KernDist;
        cLine[nChars]=c;
        R.ChrXP+=ChrWidth-KernDist; // position of next character
    }
    return c;
}

void FormatLine(UCHAR* TBuf, sal_uInt16& Index, ObjTextType& Atr0, ObjTextType& AktAtr,
                sal_uInt16 UmbWdt, sal_uInt16 AdjWdt,
                short* Line, sal_uInt16& nChars,
                double, double,
                UCHAR* cLine, bool TextFit)
{
    ScopedVclPtrInstance< VirtualDevice > vOut;
    UCHAR        c,c0;
    bool         First;               // first char ?
    sal_uInt8    Just = 0;            // paragraph format
    bool         Border;              // border of box reached ?
    bool         Border0;
    bool         AbsEnd;              // end of paragraph reached ?
    std::unique_ptr<ProcChrSta> R(new ProcChrSta);
    std::unique_ptr<ProcChrSta> R0(new ProcChrSta);
    std::unique_ptr<ProcChrSta> WErec(new ProcChrSta);
    sal_uInt16       WEnChar;
    std::unique_ptr<ProcChrSta> WErec0(new ProcChrSta);
    sal_uInt16       WEnChar0;
    std::unique_ptr<ProcChrSta> TRrec(new ProcChrSta);
    sal_uInt16       TRnChar;

    sal_uInt16       WordEndCnt;          // justify and separate
    bool         WordEnd;
    bool         Trenn;

    short        BoxRest;             // to crush and format
    sal_uInt16       i,j,k,h;
    sal_uInt16       re,li;

    vOut->SetMapMode(MapMode(MAP_10TH_MM,Point(),Fraction(1,4),Fraction(1,4)));

    nChars=0;
    SetTextContext(*vOut.get(),AktAtr,false,0,1,1,1,1);
    InitProcessCharState(*R,AktAtr,Index);
    (*R0)=(*R); (*WErec)=(*R); WEnChar=0; c0=0; Border0=false;
    Border=false; First=true;
    WordEndCnt=0;

    do {               // check how many words to on that line
        if (Border) c=ProcessChar(*vOut.get(),TBuf,*R,Atr0,nChars,DoTrenn,Line,cLine);
        else        c=ProcessChar(*vOut.get(),TBuf,*R,Atr0,nChars,NoTrenn,Line,cLine);
        AbsEnd=(c==AbsatzEnd || c==TextEnd);
        //if not AbsEnd then
        {
            if (First) {
                Just=R->Attrib.Justify & 0x0F; // paragraph format remains, then at start
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
        First=false;
        AbsEnd=AbsEnd || (nChars>=MaxLineChars);
    } while (!(AbsEnd || (Border && ((WordEndCnt>0) || WordEnd || Trenn))));

    if (Border) { // separate and crush
        (*WErec0)=(*WErec); WEnChar0=WEnChar;
        c0=0;
        (*R)=(*WErec); nChars=WEnChar;
        (*TRrec)=(*R); TRnChar=nChars;
        Border0=false;
        do {                // first check how many syllables fit
            UCHAR ct=ProcessChar(*vOut.get(),TBuf,*TRrec,Atr0,TRnChar,DoTrenn,Line,cLine);
            c=ProcessChar(*vOut.get(),TBuf,*R,Atr0,nChars,NoTrenn,Line,cLine);
            AbsEnd=(ct==AbsatzEnd) || (ct==TextEnd) || (nChars>=MaxLineChars);

            Border=TRrec->ChrXP>UmbWdt;
            WordEnd=AbsEnd || ((AbsEnd || (c==' ')) && (c0!=' ') && (c0!=0));
            Trenn=ct=='-';
            if (WordEnd && (!Border0 || (WordEndCnt==0))) {
                WordEndCnt++;
                (*WErec)=(*R0);
                if (AbsEnd) WEnChar=nChars; else WEnChar=nChars-1;
                (*TRrec)=(*R); TRnChar=nChars;                       // to continue searching
            }
            if (Trenn && (!Border || (WordEndCnt==0))) {
                WordEndCnt++;                       // remember we can separate here
                (*WErec)=(*TRrec);
                WEnChar=TRnChar;
                (*TRrec)=(*R); TRnChar=nChars;      // continue searching
            }
            (*R0)=(*R); c0=c;
            Border0=Border;
            Border=R->ChrXP>UmbWdt;
        } while (!(AbsEnd || (Border && ((WordEndCnt>0) || WordEnd || Trenn))));

        while (WErec0->Index<WErec->Index) { // to assure Line[] matches }
            (void)ProcessChar(*vOut.get(),TBuf,*WErec0,Atr0,WEnChar0,WEnChar-WEnChar0-1,Line,cLine);
        }

        (*R)=(*WErec); nChars=WEnChar;

        if (UmbWdt<R->ChrXP) {
            BoxRest=R->ChrXP-UmbWdt;                   // so much should be crushed
            for (i=2;i<=nChars;i++) {                  // first character position remains!
                Line[i]-=(i-1)*(BoxRest) /(nChars-1);
            }
            R->ChrXP=UmbWdt;
            Line[nChars+1]=UmbWdt;
        }
    }

    if (!AbsEnd) {
        do {                                         // read empty characters
            (*WErec)=(*R);
            c=GetTextChar(TBuf,R->Index,Atr0,R->Attrib,NoTrenn,false);
            nChars++;
            Line[nChars]=R->ChrXP;
            cLine[nChars]=c;
        } while (c==' ');
        if (c!=' ' && c!=AbsatzEnd && c!=TextEnd) {
            nChars--;
            (*R)=(*WErec);
        }
    }

    if (AbsEnd && nChars<MaxLineChars) { // align, left aligned instead of block
        if (Just==3) Just=0;
        nChars++; Line[nChars]=R->ChrXP; // to assure AbsatzEnde is read
        Line[nChars+1]=R->ChrXP;         // as the width of CR or #0 is very small
        if (TBuf[R->Index-1]!=AbsatzEnd &&  TBuf[R->Index-1]!=TextEnd) {
            GetTextChar(TBuf,R->Index,Atr0,R->Attrib,NoTrenn,false); // small correction needed, if 1st word read
        }
    }

    BoxRest=AdjWdt-R->ChrXP;
    if (TextFit) Just=THJustLeft;

    switch (Just) {
        case THJustLeft: break;                                // left
        case THJustCenter: {
            BoxRest=BoxRest /2;                                // middle
            for (i=1;i<=nChars;i++) Line[i]=Line[i]+BoxRest;
        } break;
        case THJustRight: {                                    // right
            for (i=1;i<=nChars;i++) Line[i]=Line[i]+BoxRest;
        } break;
        case THJustDrvOut:
        case THJustBlock: {                                    // block and justified
            re=nChars;
            if (Just==THJustDrvOut) re--;
            while (re>=1 && (cLine[re]==' ' || cLine[re]==TextEnd || cLine[re]==AbsatzEnd)) re--;
            li=1;
            while (li<=re && (cLine[li]==' ' || cLine[li]==TextEnd || cLine[li]==AbsatzEnd)) li++;
            if (Just==THJustDrvOut) BoxRest=AdjWdt-Line[re+1];

            j=0;                        // get number of spaces
            for (i=li;i<=re;i++) {
                if (cLine[i]==' ') {
                    j++;
                }
            }

            if (j==0) {                        // only one word ?  -> stretch !
                for (i=li+1;i<=re;i++) {       // from left to right
                  Line[i]=Line[i]+MulDiv(i-li,BoxRest,re-li+1-1);
                }
            } else {
                k=0; h=0;
                for (i=li;i<=re;i++) {          // j drill spaces !
                    if (cLine[i]==' ') {        // space found !
                        k++;
                        h=MulDiv(k,BoxRest,j);
                    }
                    Line[i]=Line[i]+h;
                }
            }
            for (i=re+1;i<=nChars;i++) Line[i]=Line[i]+BoxRest; // adapt the rest
            Line[nChars+1]=AdjWdt;
        } break;
        case THJustLocked: {                                    // locked out
            re=nChars-1;
            while (re>=1 && (cLine[re]==' ' || cLine[re]==TextEnd || cLine[re]==AbsatzEnd)) re--;
            li=1;
            while (li<=re && (cLine[li]==' ' || cLine[li]==TextEnd || cLine[li]==AbsatzEnd)) li++;
            BoxRest=AdjWdt-Line[re+1];
            for (i=li+1;i<=re;i++) {         // stretch from left to right
                Line[i]=Line[i]+MulDiv(i-li,BoxRest,re-li+1-1);
            }
            for (i=re+1;i<=nChars;i++) Line[i]=Line[i]+BoxRest; // adapt the rest
            Line[nChars+1]=AdjWdt;
        } break;
    }
    Index=R->Index;
    AktAtr=R->Attrib;
}

// End of Absatz.Pas

// Start of DrawText.Pas

void DrawChar(OutputDevice& rOut, UCHAR c, ObjTextType T, PointType Pos, sal_uInt16 RotationAngle,
              sal_uInt16 FitXMul, sal_uInt16 FitXDiv, sal_uInt16 FitYMul, sal_uInt16 FitYDiv)
{
    SetTextContext(rOut,T,UpcasePossible(c),RotationAngle,FitXMul,FitXDiv,FitYMul,FitYDiv);
    if ((T.Schnitt & TextKaptBit)!=0 && UpcasePossible(c)) c=Upcase(c);
    OUString s(reinterpret_cast<const sal_Char*>(&c), 1,
        RTL_TEXTENCODING_IBM_437);
    rOut.DrawText( Point( Pos.x, Pos.y ), s );
}

void TextType::Draw(OutputDevice& rOut)
{
    if ((Flags & TextOutlBit)!=0) return;   // source text for Outliner !!

    ObjTextType T1,T2;
    sal_uInt16 Index1;
    sal_uInt16 Index2;
    UCHAR  c = TextEnd;
    sal_uInt16 l;                // number of characters on the line
    sal_uInt16 i;
    short  yPos0;
    short  xPos;
    short  yPos;
    sal_uInt16 LF;
    sal_uInt16 MaxGrad;
    short  xSize;
    short  xSAdj;
    short  ySize;
    double sn,cs;
    sal_uInt16 TopToBase;
    bool   Ende = false;
    sal_uInt16 lc;
    bool   TextFit;
    std::unique_ptr<short[]> xLine;
    std::unique_ptr<UCHAR[]> cLine;   // Buffer for FormatLine
    sal_uInt16 FitXMul;
    sal_uInt16 FitXDiv;
    sal_uInt16 FitYMul;
    sal_uInt16 FitYDiv;
    bool   Fehler;
    UCHAR* Buf=Buffer; // pointer to the letters

    pSgfFonts->ReadList();
    xLine.reset(new short[ChrXPosArrSize]);
    cLine.reset(new UCHAR[CharLineSize]);

    TextFit=(Flags & TextFitBits)!=0;
    bool LineFit=((Flags & TextFitZBit)!=0);  // FitSize.x=0? or flags -> stretch each line
    if (TextFit && FitSize.x==0) LineFit=true;

    if (RotationAngle==0) {
        sn=0.0;
        cs=1.0;
    } else {
        sn=sin(double(RotationAngle)*3.14159265359/18000);
        cs=cos(double(RotationAngle)*3.14159265359/18000);
    }

    T1=T; Index1=0; yPos=0; xPos=0;
    if (TextFit) {
        ySize=Pos2.y-Pos1.y;
        xSize=32000 /2;      // break
        xSAdj=Pos2.x-Pos1.x; // to align for center/block
        //if (xSize<=0) { xSize=32000 /2; LineFit=true; }
        FitXMul=sal::static_int_cast< sal_uInt16 >(std::abs(Pos2.x-Pos1.x)); FitXDiv=FitSize.x; if (FitXDiv==0) FitXDiv=1;
        FitYMul=sal::static_int_cast< sal_uInt16 >(std::abs(Pos2.y-Pos1.y)); FitYDiv=FitSize.y; if (FitYDiv==0) FitYDiv=1;
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
        FormatLine(Buf,Index2,T,T2,xSize,xSAdj,xLine.get(),l,sn,cs,cLine.get(),LineFit);
        Fehler=(Index2==Index1);
        if (!Fehler) {
            lc=GetLineFeed(Buf,Index1,T,T1,l,LF,MaxGrad);
            if (TextFit) {
                if (LineFit) FitXDiv=xLine[lc+1];
                if (FitXDiv>0) {
                    for (i=1;i<=l+1;i++) {
                        const long Temp=long(xLine[i])*long(FitXMul) /long(FitXDiv);
                        xLine[i]=short(Temp);
                    }
                    LF=MulDiv(LF,FitYMul,FitYDiv);
                    MaxGrad=MulDiv(MaxGrad,FitYMul,FitYDiv);
                } else {
                    FitXDiv=1; // 0 does not exist
                }
            }
            yPos0=yPos;
            TopToBase=GetTopToBaseLine(MaxGrad);
            yPos=yPos+TopToBase;
            Ende=(yPos0+short(MulDiv(MaxGrad,CharTopToBtm,100))>ySize) && !TextFit;
            if (!Ende) {
                T2=T1; Index2=Index1;
                i=1;
                while (i<=l) {
                    c=GetTextCharConv(Buf,Index2,T,T2,l-i,false);
                    long xp1,yp1;       // due to overflow danger
                    PointType Pos;
                    xp1=long(Pos1.x)+xPos+long(xLine[i]);
                    yp1=long(Pos1.y)+yPos;
                    if (xp1>32000) xp1=32000; if (xp1<-12000) xp1=-12000;
                    if (yp1>32000) yp1=32000; if (yp1<-12000) yp1=-12000;
                    Pos.x=short(xp1);
                    Pos.y=short(yp1);

                    if (RotationAngle!=0) RotatePoint(Pos,Pos1.x,Pos1.y,sn,cs);
                    DrawChar(rOut,c,T2,Pos,RotationAngle,FitXMul,FitXDiv,FitYMul,FitYDiv);
                    i++;
                } // while i<=l
                yPos=yPos0+LF;
                T1=T2; Index1=Index2; // for next line
            } // if ObjMin.y+yPos<=Obj_Max.y
        } // if !Fehler
    } while (c!=TextEnd && !Ende && !Fehler);
}

// End of DrawText.Pas

sal_uInt32 ObjTextType::GetFont()
{
    return sal_uLong(FontLo)+0x00010000*sal_uLong(FontHi);
}

void ObjTextType::SetFont(sal_uInt32 FontID)
{
    FontLo=sal_uInt16(FontID & 0x0000FFFF);
    FontHi=sal_uInt16((FontID & 0xFFFF0000)>>16);
}

// SGF.Ini lesen
SgfFontOne::SgfFontOne()
{
    Next=nullptr;
    IFID=0;
    Bold=false;
    Ital=false;
    Sans=false;
    Serf=false;
    Fixd=false;
    SVFamil=FAMILY_DONTKNOW;
    SVChSet=RTL_TEXTENCODING_DONTKNOW;
    SVWidth=40;
}

void SgfFontOne::ReadOne( const OString& rID, OString& Dsc )
{
    if ( Dsc.getLength() < 4 || ( Dsc[0] != '(' ) )
        return;
    sal_Int32 i=1;   // first letter of  IF fontname. In front is a '('
    while ( i < Dsc.getLength() && ( Dsc[i] !=')' ) )
        i++;
    Dsc = Dsc.copy(i+1);                                // delete IF fontname incl. ()

    if ( Dsc.getLength() < 2 || ( Dsc[Dsc.getLength() - 1] !=')' ) )
        return;
    i = Dsc.getLength()-2;                                // here is the ')' of the SV fontname
    sal_Int32 j=0;
    while ( i > 0 && ( Dsc[i] != '(' ) )
    {
        i--;
        j++;
    }
    SVFName = OStringToOUString(Dsc.copy(i+1,j), RTL_TEXTENCODING_IBM_437); // retrieve SV fontname
    Dsc = OStringBuffer(Dsc).remove(i,j).makeStringAndClear();

    IFID = (sal_uInt32)rID.toInt32();
    sal_Int32 nTokenCount = comphelper::string::getTokenCount(Dsc, ' ');
    for (sal_Int32 nIdx = 0; nIdx < nTokenCount; ++nIdx)
    {
        OString s(Dsc.getToken(nIdx, ' '));
        if (!s.isEmpty())
        {
            s = s.toAsciiUpperCase();
            if      (s.match("BOLD")) Bold=true;
            else if (s.match("ITAL")) Ital=true;
            else if (s.match("SERF")) Serf=true;
            else if (s.match("SANS")) Sans=true;
            else if (s.match("FIXD")) Fixd=true;
            else if (s.match("ROMAN")) SVFamil=FAMILY_ROMAN;
            else if (s.match("SWISS")) SVFamil=FAMILY_SWISS;
            else if (s.match("MODERN")) SVFamil=FAMILY_MODERN;
            else if (s.match("SCRIPT")) SVFamil=FAMILY_SCRIPT;
            else if (s.match("DECORA")) SVFamil=FAMILY_DECORATIVE;
            else if (s.match("ANSI")) SVChSet=RTL_TEXTENCODING_MS_1252;
            else if (s.match("IBMPC")) SVChSet=RTL_TEXTENCODING_IBM_850;
            else if (s.match("MAC")) SVChSet=RTL_TEXTENCODING_APPLE_ROMAN;
            else if (s.match("SYMBOL")) SVChSet=RTL_TEXTENCODING_SYMBOL;
            else if (s.match("SYSTEM")) SVChSet = osl_getThreadTextEncoding();
            else if (comphelper::string::isdigitAsciiString(s) ) SVWidth=sal::static_int_cast< sal_uInt16 >(s.toInt32());
        }
    }
}

SgfFontLst::SgfFontLst()
{
    pList=nullptr;
    Last=nullptr;
    LastID=0;
    LastLn=nullptr;
    Tried=false;
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
    while (P!=nullptr) {
        P1=P->Next;
        delete P;
        P=P1;
    }
    pList=nullptr;
    Last=nullptr;
    Tried=false;
    LastID=0;
    LastLn=nullptr;
}

void SgfFontLst::AssignFN(const OUString& rFName)
{   FNam=rFName;   }

void SgfFontLst::ReadList()
{
    if (!Tried)
    {
        Tried=true;
        LastID=0;
        LastLn=nullptr;
        SgfFontOne* P,P1;
        Config aCfg(FNam);
        aCfg.SetGroup("SGV Fonts fuer StarView");
        sal_uInt16 Anz=aCfg.GetKeyCount();
        sal_uInt16 i;
        OString FID;
        OString Dsc;

        for (i=0;i<Anz;i++)
        {
            FID = comphelper::string::remove(aCfg.GetKeyName(i), ' ');
            Dsc = aCfg.ReadKey( i );
            if (comphelper::string::isdigitAsciiString(FID))
            {
                P=new SgfFontOne;                                   // new entry
                if (Last!=nullptr) Last->Next=P; else pList=P; Last=P; // link it
                P->ReadOne(FID,Dsc);                                // interpret line
            }
        }
    }
}

SgfFontOne* SgfFontLst::GetFontDesc(sal_uInt32 ID)
{
    if (ID!=LastID) {
        SgfFontOne* P;
        P=pList;
        while (P!=nullptr && P->IFID!=ID) P=P->Next;
        LastID=ID;
        LastLn=P;
    }
    return LastLn;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
