/*************************************************************************
 *
 *  $RCSfile: w1class.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:57 $
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

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

// local
#ifndef _W1STRUCT_HXX
#include <w1struct.hxx>
#endif

#ifdef DUMP
#include <fstream.h>
#endif

class ostream;
class SvxFontItem;
class SvxBorderLine;
class SvxBoxItem;
class SvStream;
class SfxPoolItem;
class SwField;
class Ww1Annotation;
class Ww1AtnText;
class Ww1Chp;
class Ww1DocText;
class Ww1Dop;
class Ww1Fib;
class Ww1Fkp;
class Ww1FkpChp;
class Ww1FkpPap;
class Ww1Fonts;
class Ww1Manager;
class Ww1McrText;
class Ww1Pap;
class Ww1PlainText;
class Ww1Plc;
class Ww1PlcAnnotationRef;
class Ww1PlcAnnotationTxt;
class Ww1PlcChp;
class Ww1PlcFields;
class Ww1PlcFieldFootnote;
class Ww1PlcFieldHeaderFooter;
class Ww1PlcFieldMacro;
class Ww1PlcFieldText;
class Ww1PlcFootnoteRef;
class Ww1PlcFootnoteTxt;
class Ww1PlcGlossary;
class Ww1PlcHdd;
class Ww1PlcPap;
class Ww1PlcSep;
class Ww1Shell;
class Ww1Sprm;
class Ww1SprmPapx;
class Ww1SprmSep;
class Ww1Style;
class Ww1StyleSheet;

///////////////////////////////////////////////////////////////////////
//
// nach moeglichkeit wurden in diesem modul methoden aehnlicher
// funktionalitaet gleich benannt. Die namen wurden wenn moeglich vom
// ww-filter uebernommen.
// Where() gibt die position eines elements. dies kann sowohl eine
// seek-position im stream als auch ein relativer offset sein, da dies
// bei word durcheinander geht.  die methoden sind durch kommentare
// gekennzeichnet, ob sie sich auf positionen in der datei oder
// innerhalb des textes beziehen. vorsicht: innerhalb des textes kann
// verschiedene texte in der datei bedeuten.
// Count() gibt die anzahl der elemente zurueck. vorsicht bei
// n/n-1-feldern (word speichert strukturen gern in doppel-arrays, in
// denen das erste n elemente, das zweite jedoch n-1 elemente
// enthaelt.
// Fill() fuellt uebergebene referenzen mit daten aus den
// word-strukturen.
// GetData() gibt zeiger auf den datenbereich zurueck
// GetError() gibt zurueck, ob fehler aufgetreten ist
// Start(), Stop(), Out(), op<< siehe modul w1filter
// Dump() siehe modul w1dump
//

/////////////////////////////////////////////////////////////////// Fib
//
// file information block: wurzel des uebels: steht am beginn der
// datei (seek(0)) und enthaelt alle positionen der strukturen der
// datei
//
class Ww1Fib
{
    W1_FIB aFib;
    BOOL bOK;
    SvStream& rStream;
public:
    Ww1Fib(SvStream&);
    friend ostream& operator <<(ostream&, Ww1Fib&);
    W1_FIB& GetFIB()        { return aFib; }
    BOOL GetError()         { return !bOK; }
    SvStream& GetStream()   { return rStream; }
};

/////////////////////////////////////////////////////////////////// Dop
//
// document property: eigenschaften des gesamten dokuments
//
class Ww1Dop
{
    W1_DOP aDop;
    Ww1Fib& rFib;
    BOOL bOK;
public:
    Ww1Dop(Ww1Fib&);
    BOOL GetError() {
        return !bOK; }
    W1_DOP& GetDOP() {
        return aDop; }
    friend ostream& operator <<(ostream&, Ww1Dop&);
    void Out(Ww1Shell&);
};

///////////////////////////////////////////////////////////// PlainText
//
// ww-dateien koennen mehrere textbloecke enthalten (main-text,
// fusznoten etc). PlainText vereinigt die gemeinsamkeiten
//
class Ww1PlainText
{
protected:
    Ww1Fib& rFib;
    ULONG ulFilePos;
    ULONG ulCountBytes;
    ULONG ulSeek;
    BOOL bOK;
public:
    Ww1PlainText( Ww1Fib& rFib, ULONG ulFilePos, ULONG ulCountBytes )
        : rFib( rFib ), ulCountBytes( ulCountBytes ),
        ulFilePos( ulFilePos ), ulSeek( 0 ), bOK(TRUE)
    {}

    // innerhalb des textes
    ULONG Where() const                 { return ulSeek; }
    void Seek( ULONG ulNew )
        {
            DBG_ASSERT(ulNew < ulCountBytes, "Ww1PlainText")
            if (ulNew < ulCountBytes)
                ulSeek = ulNew;
        }

    ULONG Count() const                 { return ulCountBytes; }
    void SetCount(ULONG ulNew)
        {
            ulNew += ulSeek;
            if (ulCountBytes > ulNew)
                ulCountBytes = ulNew;
        }
    void operator++(int)
    {
        DBG_ASSERT(ulSeek+1<ulCountBytes, "Ww1PlainText");
        ulSeek++;
    }
    BOOL GetError()                     { return !bOK; }
    sal_Unicode Out( Ww1Shell&, ULONG& );
    sal_Unicode Out( String&, ULONG=0xffffffff);
    sal_Unicode Out( sal_Unicode& );
    friend ostream& operator <<(ostream&, Ww1PlainText&);
    String& Fill( String&, ULONG=0, ULONG=0xffffffff );
    sal_Unicode operator []( ULONG );
    String GetText( ULONG ulOffset, ULONG nLen ) const;

    enum Consts { MinChar = 32 };
    static BOOL IsChar( sal_Unicode c )     { return c >= MinChar; }
};

/////////////////////////////////////////////////////////////// DocText
class Ww1DocText : public Ww1PlainText
{
public:
    Ww1DocText(Ww1Fib& rFib) :
        Ww1PlainText(rFib, rFib.GetFIB().fcMinGet(),
         rFib.GetFIB().ccpTextGet()) {
        }
};

/////////////////////////////////////////////////////////////// FtnText
class Ww1FtnText : public Ww1PlainText
{
public:
    ULONG Offset(Ww1Fib& rFib) {
        return rFib.GetFIB().ccpTextGet(); }
    Ww1FtnText(Ww1Fib& rFib) :
        Ww1PlainText(rFib, rFib.GetFIB().fcMinGet() +
         Offset(rFib), rFib.GetFIB().ccpFtnGet()) {
        }
};

/////////////////////////////////////////////////////////////// HddText
class Ww1HddText : public Ww1PlainText
{
public:
    ULONG Offset(Ww1Fib& rFib) {
        return rFib.GetFIB().ccpTextGet() + rFib.GetFIB().ccpFtnGet(); }
    Ww1HddText(Ww1Fib& rFib) :
        Ww1PlainText(rFib, rFib.GetFIB().fcMinGet() +
         Offset(rFib), rFib.GetFIB().ccpHddGet()) {
        }
};

/////////////////////////////////////////////////////////////// McrText
class Ww1McrText : public Ww1PlainText
{
public:
    ULONG Offset(Ww1Fib& rFib) {
        return rFib.GetFIB().ccpTextGet() + rFib.GetFIB().ccpFtnGet()
         + rFib.GetFIB().ccpHddGet(); }
    Ww1McrText(Ww1Fib& rFib) :
        Ww1PlainText(rFib, rFib.GetFIB().fcMinGet() +
         Offset(rFib), rFib.GetFIB().ccpMcrGet()) {
        }
};

/////////////////////////////////////////////////////////////// AtnText
class Ww1AtnText : public Ww1PlainText
{
public:
    ULONG Offset(Ww1Fib& rFib) {
        return rFib.GetFIB().ccpTextGet() + rFib.GetFIB().ccpFtnGet()
         + rFib.GetFIB().ccpHddGet() + rFib.GetFIB().ccpMcrGet(); }
    Ww1AtnText(Ww1Fib& rFib) :
        Ww1PlainText(rFib, rFib.GetFIB().fcMinGet() +
         Offset(rFib), rFib.GetFIB().ccpAtnGet()) {
        }
};

///////////////////////////////////////////////////////////////// Style
//
// ein einzelner style oder vorlage
//
class Ww1Style
{
    String aName;
    W1_CHP aChpx;
    Ww1SprmPapx* pPapx;
    Ww1StyleSheet* pParent;
    BYTE stcBase;
    BYTE stcNext;
    BOOL bUsed;
public:
    Ww1Style();
    ~Ww1Style();
    BOOL IsUsed() const                 { return bUsed; }
    void SetDefaults(BYTE);
    void SetParent(Ww1StyleSheet* newParent)    { pParent = newParent; }
    void SetName(const String& rName)   { bUsed = TRUE; aName = rName; }
    const String& GetName() const       { return aName; }
//  Ww1Style& GetNext();
    Ww1Style& GetBase();
    USHORT GetnBase() const             { return stcBase; }
    USHORT GetnNext() const             { return stcNext; }
    USHORT ReadName(BYTE*&, USHORT&, USHORT stc);
    USHORT ReadChpx(BYTE*&, USHORT&);
    USHORT ReadPapx(BYTE*&, USHORT&);
    USHORT ReadEstcp(BYTE*&, USHORT&);
    friend ostream& operator <<(ostream&, Ww1Style&);
    void Out(Ww1Shell&, Ww1Manager&);
};

//////////////////////////////////////////////////////////// StyleSheet
//
// die sammlung aller vorlagen (max. 256)
//
class Ww1StyleSheet
{
    Ww1Style aStyles[256];
    USHORT cstcStd; // count style code standard
    Ww1Fib& rFib;
    BOOL bOK;
    USHORT ReadNames(BYTE*&, USHORT&);
    USHORT ReadChpx(BYTE*&, USHORT&);
    USHORT ReadPapx(BYTE*&, USHORT&);
    USHORT ReadEstcp(BYTE*&, USHORT&);

    void OutDefaults(Ww1Shell& rOut, Ww1Manager& rMan, USHORT stc);
    void OutOne(Ww1Shell& rOut, Ww1Manager& rMan, USHORT stc);
    void OutOneWithBase(Ww1Shell& rOut, Ww1Manager& rMan, USHORT stc,
                        BYTE* pbStopRecur );
public:
    Ww1StyleSheet(Ww1Fib& rFib);
    Ww1Style& GetStyle(USHORT stc) {
        return aStyles[stc]; }
    USHORT Count() {
        return 256; }
    friend ostream& operator <<(ostream&, Ww1StyleSheet&);
    void Out(Ww1Shell&, Ww1Manager&);
    friend Ww1Style;
    BOOL GetError() {
        return !bOK; }
};

///////////////////////////////////////////////////////////////// Fonts
//
// ww kennt nur font-nummern beim formatieren. nebenher gibts ein
// array von fonts, damit man aus der nummer einen konkreten font
// machen kann.
//
class Ww1Fonts
{
protected:
    W1_FFN** pFontA; // Array of Pointers to Font Description
    Ww1Fib& rFib;
    ULONG nFieldFlags;
    USHORT nMax; // Array-Groesse
    BOOL bOK;
public:
    Ww1Fonts(Ww1Fib&, ULONG nFieldFlgs);
    ~Ww1Fonts() {
        if (pFontA)
            DELETEZ(pFontA[0]);
        DELETEZ(pFontA); }
    W1_FFN* GetFFN(USHORT nNum);
    USHORT Count() {
        return nMax; }
    friend ostream& operator <<(ostream&, Ww1Fonts&);
    BOOL GetError() {
        return !bOK; }
    SvxFontItem GetFont(USHORT);
};

//////////////////////////////////////////////////////////// SingleSprm
//
// diese klassen ersetzen die aSprmTab etc des ww6-filters. die
// funktionspointer sind hier virtuale methoden, fuer die typen (byte,
// word, var-sized etc) gibt es abgeleitete klassen. diese haben
// methoden zum bestimmen der groesze, dumpen und shell-ausgeben der
// Sprms.
// die klassen werden mit new (in InitTab()) erzeugt und nach ihrem
// code in die tabelle gestellt. zum aktivieren ruft man nun nur noch
// die entsprechende methode des objektes in der tabelle auf.
// wohlgemerkt: SingleSprms sind die _beschreibung_ und _funktion_ der
// Sprms, nicht deren inhalt. dieser musz uebergeben werden an die
// einzelnen methoden wie Size, Dump und Start/Stop.
//
class Ww1SingleSprm
{
public:
#ifdef DUMP
//
// allein die virtuellen methoden stehen in der vtab, also je nachdem,
// ob fuer dumper oder filter uebersetzt wird ausblenden: das spart
// platz. ausserdem stehen die methoden fuer dumper bzw filter in
// verschiedenen modulen, die im jeweils anderen projekt nicht
// uebersetzt werden. das diese dann beim linken nicht zur verfuegung
// stehen faellt dann auch nicht auf. Der Namensstring ist nur im
// Dumper noetig: weg damit im Filter.
//
    void Start(Ww1Shell&, BYTE, BYTE*, USHORT, Ww1Manager&);
    void Stop(Ww1Shell&, BYTE, BYTE*, USHORT, Ww1Manager&);
    virtual ostream& Dump(ostream&, BYTE*, USHORT);
    const sal_Char* sName;
#else
    virtual void Start(Ww1Shell&, BYTE, BYTE*, USHORT, Ww1Manager&);
    virtual void Stop(Ww1Shell&, BYTE, BYTE*, USHORT, Ww1Manager&);
    ostream& Dump(ostream&, BYTE*, USHORT);
#endif
    virtual USHORT Size(BYTE*);
    USHORT nCountBytes;

    Ww1SingleSprm(USHORT nCountBytes, const sal_Char* pName = 0 )
        : nCountBytes(nCountBytes)
#ifdef DUMP
        , sName( pName)
#endif
        {
        }
};

class Ww1SingleSprmByteSized : public Ww1SingleSprm {
public:
//  ostream& Dump(ostream&, BYTE*, USHORT);
    USHORT Size(BYTE*);
    Ww1SingleSprmByteSized(USHORT nCountBytes, sal_Char* sName = 0) :
        Ww1SingleSprm(nCountBytes, sName) {
        }
};

class Ww1SingleSprmWordSized : public Ww1SingleSprm {
public:
//  ostream& Dump(ostream&, BYTE*, USHORT);
    USHORT Size(BYTE*);
    Ww1SingleSprmWordSized(USHORT nCountBytes, sal_Char* sName = 0) :
        Ww1SingleSprm(nCountBytes, sName) {
        }
};

class Ww1SingleSprmByte : public Ww1SingleSprm {
public:
    ostream& Dump(ostream&, BYTE*, USHORT);
//  USHORT Size(BYTE*);
    Ww1SingleSprmByte(sal_Char* sName = 0) :
        Ww1SingleSprm(1, sName) {
        }
};

class Ww1SingleSprmBool : public Ww1SingleSprmByte {
public:
    ostream& Dump(ostream&, BYTE*, USHORT);
//  USHORT Size(BYTE*);
    Ww1SingleSprmBool(sal_Char* sName = 0) :
        Ww1SingleSprmByte(sName) {
        }
};

class Ww1SingleSprm4State : public Ww1SingleSprmByte {
public:
    ostream& Dump(ostream&, BYTE*, USHORT);
//  USHORT Size(BYTE*);
    Ww1SingleSprm4State(sal_Char* sName = 0) :
        Ww1SingleSprmByte(sName) {
        }
};

class Ww1SingleSprmWord : public Ww1SingleSprm {
public:
    ostream& Dump(ostream&, BYTE*, USHORT);
//  USHORT Size(BYTE*);
    Ww1SingleSprmWord(sal_Char* sName = 0) :
        Ww1SingleSprm(2, sName) {
        }
};

class Ww1SingleSprmLong : public Ww1SingleSprm {
public:
    ostream& Dump(ostream&, BYTE*, USHORT);
//  USHORT Size(BYTE*);
    Ww1SingleSprmLong(sal_Char* sName = 0) :
        Ww1SingleSprm(4, sName) {
        }
};

class Ww1SingleSprmTab : public Ww1SingleSprm {
public:
    ostream& Dump(ostream&, BYTE*, USHORT);
    USHORT Size(BYTE*);
    Ww1SingleSprmTab(USHORT nCountBytes, sal_Char* sName = 0) :
        Ww1SingleSprm(nCountBytes, sName) {
        }
};

class Ww1SingleSprmPJc : public Ww1SingleSprmByte {
public:
    Ww1SingleSprmPJc(sal_Char* sName) :
        Ww1SingleSprmByte(sName) {
        }
    void Start(Ww1Shell&, BYTE, BYTE*, USHORT, Ww1Manager&);
    void Stop(Ww1Shell&, BYTE, BYTE*, USHORT, Ww1Manager&);
};

class Ww1SingleSprmPDxa : public Ww1SingleSprmWord {
public:
    Ww1SingleSprmPDxa(sal_Char* sName) :
        Ww1SingleSprmWord(sName) {
        }
    void Stop(Ww1Shell&, BYTE, BYTE*, USHORT, Ww1Manager&);
};

class Ww1SingleSprmPDxaRight : public Ww1SingleSprmPDxa {
public:
    Ww1SingleSprmPDxaRight(sal_Char* sName) :
        Ww1SingleSprmPDxa(sName) {
        }
    void Start(Ww1Shell&, BYTE, BYTE*, USHORT, Ww1Manager&);
};

class Ww1SingleSprmPDxaLeft : public Ww1SingleSprmPDxa {
public:
    Ww1SingleSprmPDxaLeft(sal_Char* sName) :
        Ww1SingleSprmPDxa(sName) {
        }
    void Start(Ww1Shell&, BYTE, BYTE*, USHORT, Ww1Manager&);
};

class Ww1SingleSprmPDxaLeft1 : public Ww1SingleSprmPDxa {
public:
    Ww1SingleSprmPDxaLeft1(sal_Char* sName) :
        Ww1SingleSprmPDxa(sName) {
        }
    void Start(Ww1Shell&, BYTE, BYTE*, USHORT, Ww1Manager&);
};

class Ww1SingleSprmPFKeep : public Ww1SingleSprmBool {
public:
    Ww1SingleSprmPFKeep(sal_Char* sName) :
        Ww1SingleSprmBool(sName) {
        }
    void Start(Ww1Shell&, BYTE, BYTE*, USHORT, Ww1Manager&);
    void Stop(Ww1Shell&, BYTE, BYTE*, USHORT, Ww1Manager&);
};

class Ww1SingleSprmPFKeepFollow : public Ww1SingleSprmBool {
public:
    Ww1SingleSprmPFKeepFollow(sal_Char* sName) :
        Ww1SingleSprmBool(sName) {
        }
    void Start(Ww1Shell&, BYTE, BYTE*, USHORT, Ww1Manager&);
    void Stop(Ww1Shell&, BYTE, BYTE*, USHORT, Ww1Manager&);
};

class Ww1SingleSprmPPageBreakBefore : public Ww1SingleSprmBool {
public:
    Ww1SingleSprmPPageBreakBefore(sal_Char* sName) :
        Ww1SingleSprmBool(sName) {
        }
    void Start(Ww1Shell&, BYTE, BYTE*, USHORT, Ww1Manager&);
    void Stop(Ww1Shell&, BYTE, BYTE*, USHORT, Ww1Manager&);
};

class Ww1SingleSprmPBrc : public Ww1SingleSprmWord {
protected:
//  SvxBorderLine* SetBorder(SvxBorderLine*, W1_BRC*);
    // spezielle start-routine, je nach sprm verschieden versorgt
    // mit einem BoxItem.
    void Start(Ww1Shell&, BYTE, W1_BRC10*, USHORT, Ww1Manager&, SvxBoxItem&);
    void Start(Ww1Shell&, BYTE, W1_BRC*, USHORT, Ww1Manager&, SvxBoxItem&);
public:
    Ww1SingleSprmPBrc(sal_Char* sName) :
        Ww1SingleSprmWord(sName) {
        }
    void Stop(Ww1Shell&, BYTE, BYTE*, USHORT, Ww1Manager&);
    // SetBorder() wird auch fuer Tabellen gebraucht, deshalb public
    static SvxBorderLine* SetBorder(SvxBorderLine*, W1_BRC10*);
};

#define BRC_TOP ((USHORT)0)
#define BRC_LEFT ((USHORT)1)
#define BRC_BOTTOM ((USHORT)2)
#define BRC_RIGHT ((USHORT)3)
#define BRC_ANZ ((USHORT)BRC_RIGHT-BRC_TOP+1)

// Die BRC-struktur fuer 1.0 versionen von word sind verschieden von
// denen der folgenden versionen. diese werden zum glueck aber auch
// von anderen sprms abgerufen.
// SH: Ab sofort alle 4 Umrandungen ueber nur 1 Klasse.
class Ww1SingleSprmPBrc10 : public Ww1SingleSprmPBrc {
    USHORT nLine;   // BRC_TOP, BRC_LEFT, ...
public:
    Ww1SingleSprmPBrc10(USHORT nL, sal_Char* sName) :
        nLine(nL), Ww1SingleSprmPBrc(sName) {
        }
    void Start(Ww1Shell&, BYTE, BYTE*, USHORT, Ww1Manager&);
};

class Ww1SingleSprmParaSpace : public Ww1SingleSprmWord {
public:
    Ww1SingleSprmParaSpace(sal_Char* sName) :
        Ww1SingleSprmWord(sName) {
        }
    void Stop(Ww1Shell&, BYTE, BYTE*, USHORT, Ww1Manager&);
};

class Ww1SingleSprmPDyaBefore : public Ww1SingleSprmParaSpace {
public:
    Ww1SingleSprmPDyaBefore(sal_Char* sName) :
        Ww1SingleSprmParaSpace(sName) {
        }
    void Start(Ww1Shell&, BYTE, BYTE*, USHORT, Ww1Manager&);
};

class Ww1SingleSprmPDyaAfter : public Ww1SingleSprmParaSpace {
public:
    Ww1SingleSprmPDyaAfter(sal_Char* sName) :
        Ww1SingleSprmParaSpace(sName) {
        }
    void Start(Ww1Shell&, BYTE, BYTE*, USHORT, Ww1Manager&);
};

class Ww1SingleSprmPDyaLine : public Ww1SingleSprmWord {
public:
    Ww1SingleSprmPDyaLine(sal_Char* sName) :
        Ww1SingleSprmWord(sName) {
        }
    void Start(Ww1Shell&, BYTE, BYTE*, USHORT, Ww1Manager&);
    void Stop(Ww1Shell&, BYTE, BYTE*, USHORT, Ww1Manager&);
};

class Ww1SingleSprmPChgTabsPapx : public Ww1SingleSprmByteSized {
public:
    Ww1SingleSprmPChgTabsPapx(sal_Char* sName) :
        Ww1SingleSprmByteSized(0, sName) {
        }
  // Size() ist noch nicht aktiviert !!
    void Start(Ww1Shell&, BYTE, BYTE*, USHORT, Ww1Manager&);
    void Stop(Ww1Shell&, BYTE, BYTE*, USHORT, Ww1Manager&);
};

class Ww1SingleSprmSGprfIhdt : public Ww1SingleSprmByte {
public:
    Ww1SingleSprmSGprfIhdt(sal_Char* sName) :
        Ww1SingleSprmByte(sName) {
        }
    void Start(Ww1Shell&, BYTE, BYTE*, USHORT, Ww1Manager&);
};

class Ww1SingleSprmSColumns : public Ww1SingleSprmWord {
public:
    Ww1SingleSprmSColumns(sal_Char* sName) :
        Ww1SingleSprmWord(sName) {
        }
    void Start(Ww1Shell&, BYTE, BYTE*, USHORT, Ww1Manager&);
};

class Ww1SingleSprmPFInTable : public Ww1SingleSprmBool {
public:
    Ww1SingleSprmPFInTable(sal_Char* sName) :
        Ww1SingleSprmBool(sName) {
        }
    void Start(Ww1Shell&, BYTE, BYTE*, USHORT, Ww1Manager&);
    void Stop(Ww1Shell&, BYTE, BYTE*, USHORT, Ww1Manager&);
};

class Ww1SingleSprmPTtp : public Ww1SingleSprmBool {
public:
    Ww1SingleSprmPTtp(sal_Char* sName) :
        Ww1SingleSprmBool(sName) {
        }
    void Start(Ww1Shell&, BYTE, BYTE*, USHORT, Ww1Manager&);
    void Stop(Ww1Shell&, BYTE, BYTE*, USHORT, Ww1Manager&);
};

class Ww1SingleSprmTJc : public Ww1SingleSprmWord {
public:
    Ww1SingleSprmTJc(sal_Char* sName) :
        Ww1SingleSprmWord(sName) {
        }
    void Start(Ww1Shell&, BYTE, BYTE*, USHORT, Ww1Manager&);
};

//class Ww1SingleSprmTDxaLeft : public Ww1SingleSprmWord {
//public:
//  Ww1SingleSprmTDxaLeft(sal_Char* sName) :
//      Ww1SingleSprmWord(sName) {
//      }
//  void Start(Ww1Shell&, BYTE, BYTE*, USHORT, Ww1Manager&);
//};

class Ww1SingleSprmTDxaGapHalf : public Ww1SingleSprmWord {
public:
    Ww1SingleSprmTDxaGapHalf(sal_Char* sName) :
        Ww1SingleSprmWord(sName) {
        }
    void Start(Ww1Shell&, BYTE, BYTE*, USHORT, Ww1Manager&);
};

class Ww1SingleSprmTDefTable10 : public Ww1SingleSprmWordSized {
public:
    Ww1SingleSprmTDefTable10(sal_Char* sName) :
        Ww1SingleSprmWordSized(0, sName) {
        }
    void Start(Ww1Shell&, BYTE, BYTE*, USHORT, Ww1Manager&);
};

class Ww1SingleSprmTDyaRowHeight : public Ww1SingleSprmWord {
public:
    Ww1SingleSprmTDyaRowHeight(sal_Char* sName) :
        Ww1SingleSprmWord(sName) {
        }
    void Start(Ww1Shell&, BYTE, BYTE*, USHORT, Ww1Manager&);
};

// Klassendefinitionen fuer Tabellen-Fastsave-Attribute
// Da wir kein Fastsave unterstuetzen, brauchen wir's nicht

#if 0

class Ww1SingleSprmTInsert : public Ww1SingleSprm {
public:
    Ww1SingleSprmTInsert(sal_Char* sName) :
        Ww1SingleSprm(4, sName) {
        }
    void Start(Ww1Shell&, BYTE, BYTE*, USHORT, Ww1Manager&);
};

class Ww1SingleSprmTDelete : public Ww1SingleSprmWord {
public:
    Ww1SingleSprmTDelete(sal_Char* sName) :
        Ww1SingleSprmWord(sName) {
        }
    void Start(Ww1Shell&, BYTE, BYTE*, USHORT, Ww1Manager&);
};

class Ww1SingleSprmTDxaCol : public Ww1SingleSprm {
public:
    Ww1SingleSprmTDxaCol(sal_Char* sName) :
        Ww1SingleSprm(4, sName) {
        }
    void Start(Ww1Shell&, BYTE, BYTE*, USHORT, Ww1Manager&);
};

class Ww1SingleSprmTMerge : public Ww1SingleSprmWord {
public:
    Ww1SingleSprmTMerge(sal_Char* sName) :
        Ww1SingleSprmWord(sName) {
        }
    void Start(Ww1Shell&, BYTE, BYTE*, USHORT, Ww1Manager&);
};

class Ww1SingleSprmTSplit : public Ww1SingleSprmWord {
public:
    Ww1SingleSprmTSplit(sal_Char* sName) :
        Ww1SingleSprmWord(sName) {
        }
    void Start(Ww1Shell&, BYTE, BYTE*, USHORT, Ww1Manager&);
};

class Ww1SingleSprmTSetBrc10 : public Ww1SingleSprm {
public:
    Ww1SingleSprmTSetBrc10(sal_Char* sName) :
        Ww1SingleSprm(5, sName) {
        }
    void Start(Ww1Shell&, BYTE, BYTE*, USHORT, Ww1Manager&);
};
#endif // 0

// Klassendefinitionen fuer Apos ( == Flys )

class Ww1SingleSprmPpc : public Ww1SingleSprmByte {
public:
    Ww1SingleSprmPpc(sal_Char* sName) :
        Ww1SingleSprmByte(sName) {
        }
    void Start(Ww1Shell&, BYTE, BYTE*, USHORT, Ww1Manager&);
};

class Ww1SingleSprmPDxaAbs : public Ww1SingleSprmWord {
public:
    Ww1SingleSprmPDxaAbs(sal_Char* sName) :
        Ww1SingleSprmWord(sName) {
        }
    void Start(Ww1Shell&, BYTE, BYTE*, USHORT, Ww1Manager&);
};

class Ww1SingleSprmPDyaAbs : public Ww1SingleSprmWord {
public:
    Ww1SingleSprmPDyaAbs(sal_Char* sName) :
        Ww1SingleSprmWord(sName) {
        }
    void Start(Ww1Shell&, BYTE, BYTE*, USHORT, Ww1Manager&);
};

class Ww1SingleSprmPDxaWidth : public Ww1SingleSprmWord {
public:
    Ww1SingleSprmPDxaWidth(sal_Char* sName) :
        Ww1SingleSprmWord(sName) {
        }
    void Start(Ww1Shell&, BYTE, BYTE*, USHORT, Ww1Manager&);
};

class Ww1SingleSprmPFromText : public Ww1SingleSprmWord {
public:
    Ww1SingleSprmPFromText(sal_Char* sName) :
        Ww1SingleSprmWord(sName) {
        }
    void Start(Ww1Shell&, BYTE, BYTE*, USHORT, Ww1Manager&);
};

////////////////////////////////////////////////////////////////// Sprm
//
// der tatsaechlich in der datei auftretende datentyp Sprm
//
class Ww1Sprm
{
    BOOL ReCalc();
protected:
    static Ww1SingleSprm* aTab[256];
    static void InitTab();
    Ww1SingleSprm& GetTab(USHORT nId) {
        if (aTab[0] == 0)
            InitTab();
        return *aTab[nId]; }
    BYTE* p;
    USHORT nCountBytes;
    BOOL bOK;
    USHORT* pArr;
    USHORT count;
// ohne Token, mit laengen-byte/word
    USHORT GetSize(BYTE nId, BYTE* pSprm);
// mit Token und LaengenByte
    USHORT GetSizeBrutto(BYTE* pSprm) {
        BYTE nId = *pSprm++;
        return GetSize(nId, pSprm) + 1; }
// gibt fuer nTh element id, size & zeiger auf daten:
//  BOOL Fill(USHORT, BYTE&, USHORT&, BYTE*&);
public:
// SH: brauche ich public
// gibt fuer nTh element id, size & zeiger auf daten:
    BOOL Fill(USHORT, BYTE&, USHORT&, BYTE*&);

    Ww1Sprm(BYTE*, USHORT);
    Ww1Sprm(SvStream&, ULONG);
    ~Ww1Sprm();
    friend ostream& operator <<(ostream&, Ww1Sprm&);
    void Start(Ww1Shell&, Ww1Manager&);
    void Start(Ww1Shell&, Ww1Manager&, USHORT);
    void Stop(Ww1Shell&, Ww1Manager&);
    BOOL IsUsed() {
        return nCountBytes != 255; }
    USHORT Count() {
        return count; }
    BOOL GetError() {
        return !bOK; }
    static void DeinitTab();
};

/////////////////////////////////////////////////////////////// Picture
//
// der wrapper um den datentyp PIC, eine struktur, die am beginn eines
// bild-dateinamens oder eines eingebetteten bildes steht.
//
class Ww1Picture
{
    BOOL bOK;
    W1_PIC* pPic;
public:
    Ww1Picture(SvStream&, ULONG);
    ~Ww1Picture() {
        }
    BOOL GetError() {
        return !bOK; }
    friend ostream& operator <<(ostream&, Ww1Picture&);
    void Out(Ww1Shell&, Ww1Manager&);
    void WriteBmp(SvStream&);
};

/////////////////////////////////////////////////////////////////// Plc
//
// eine der wichtigen array-strukturen der ww-dateien. sie beinhalten
// n+1 dateipositionen und n attribute, die zwischen den
// dateipositionen gelten.
//
class Ww1Plc
{
    BYTE* p;
    USHORT nCountBytes;
    USHORT iMac;
    USHORT nItemSize;
    BOOL bOK;
protected:
    Ww1Fib& rFib;
    BYTE* GetData(USHORT);
public:
    Ww1Plc(Ww1Fib&, ULONG, USHORT, USHORT);
    ~Ww1Plc();
    friend ostream& operator <<(ostream&, Ww1Plc&);
    ULONG Where(USHORT); // wie im jeweiligen plc
    void Seek(ULONG, USHORT&);
    void Fill(USHORT nIndex, ULONG& begin, ULONG& end) {
        begin = Where(nIndex);
        end = Where(nIndex+1); }
    USHORT Count() {
        return iMac; }
    BOOL GetError() {
        return !bOK; }
};

// Size Tabs from Sven:
//    CHP, PAP, SEP, HED, FNR, FNT
//Plc 2,   2,   6,   0,   2,   0
//Fkp 1,   1,   0,   0,   0,   0

/////////////////////////////////////////////////////////// PlcGlossary
class Ww1PlcGlossary : public Ww1Plc
{
public:
    Ww1PlcGlossary(Ww1Fib& rFib) :
        Ww1Plc(rFib, rFib.GetFIB().fcPlcfglsyGet(),
         rFib.GetFIB().cbPlcfglsyGet(), 0) {
        }
};

////////////////////////////////////////////////////// PlcAnnotationRef
class Ww1PlcAnnotationRef : public Ww1Plc
{
public:
    Ww1PlcAnnotationRef(Ww1Fib& rFib) :
        Ww1Plc(rFib, rFib.GetFIB().fcPlcfandRefGet(),
         rFib.GetFIB().cbPlcfandRefGet(), 0) {
        }
};

////////////////////////////////////////////////////// PlcAnnotationTxt
class Ww1PlcAnnotationTxt : public Ww1Plc
{
public:
    Ww1PlcAnnotationTxt(Ww1Fib& rFib) :
        Ww1Plc(rFib, rFib.GetFIB().fcPlcfandTxtGet(),
         rFib.GetFIB().cbPlcfandTxtGet(), 0) {
        }
};

///////////////////////////////////////////////////////// PlcAnnotation
class Ww1Annotation {
    Ww1PlcAnnotationRef aRef;
    Ww1PlcAnnotationTxt aTxt;
public:
    Ww1Annotation(Ww1Fib& rFib) :
        aRef(rFib),
        aTxt(rFib) {
        }
    friend ostream& operator <<(ostream&, Ww1Annotation&);
};

//////////////////////////////////////////////////////////////// PlcSep
class Ww1PlcSep : public Ww1Plc
{
public:
    Ww1PlcSep(Ww1Fib& rFib):
        Ww1Plc(rFib, rFib.GetFIB().fcPlcfsedGet(),
         rFib.GetFIB().cbPlcfsedGet(), 6) {
        }
    friend ostream& operator <<(ostream&, Ww1PlcSep&);
};

//////////////////////////////////////////////////////////////// PlcChp
class Ww1PlcChp : public Ww1Plc
{
public:
    Ww1PlcChp(Ww1Fib& rFib) :
        Ww1Plc(rFib, rFib.GetFIB().fcPlcfbteChpxGet(),
         rFib.GetFIB().cbPlcfbteChpxGet(), 2) {
        }
    friend ostream& operator <<(ostream&, Ww1PlcChp&);
};

//////////////////////////////////////////////////////////////// PlcPap
class Ww1PlcPap : public Ww1Plc
{
public:
    Ww1PlcPap(Ww1Fib& rFib) :
        Ww1Plc(rFib, rFib.GetFIB().fcPlcfbtePapxGet(),
         rFib.GetFIB().cbPlcfbtePapxGet(), 2) {
        }
    friend ostream& operator <<(ostream&, Ww1PlcPap&);
};

//////////////////////////////////////////////////////// PlcFootnoteRef
class Ww1PlcFootnoteRef : public Ww1Plc
{
public:
    Ww1PlcFootnoteRef(Ww1Fib& rFib) :
        Ww1Plc(rFib, rFib.GetFIB().fcPlcffndRefGet(),
         rFib.GetFIB().cbPlcffndRefGet(), 2) {
        }
    friend ostream& operator <<(ostream&, Ww1PlcFootnoteRef&);
};

//////////////////////////////////////////////////////// PlcFootnoteTxt
class Ww1PlcFootnoteTxt : public Ww1Plc
{
public:
    Ww1PlcFootnoteTxt(Ww1Fib& rFib) :
        Ww1Plc(rFib, rFib.GetFIB().fcPlcffndTxtGet(),
         rFib.GetFIB().cbPlcffndTxtGet(), 0) {
        }
    friend ostream& operator <<(ostream&, Ww1PlcFootnoteTxt&);
};

///////////////////////////////////////////////////////////// PlcFields
class Ww1PlcFields : public Ww1Plc
{
    //USHORT Fill(USHORT, BYTE&, String&, String&, String&);
public:
    Ww1PlcFields(Ww1Fib& rFib, ULONG start, USHORT nCountBytes)
        : Ww1Plc(rFib, start, nCountBytes, 2)
    {}
    W1_FLD* GetData(USHORT nIndex)
        { return (W1_FLD*)Ww1Plc::GetData(nIndex); }
    ULONG Where(USHORT nIndex)  // absolut im file
        { return Ww1Plc::Where(nIndex) + rFib.GetFIB().fcMinGet(); }
    friend ostream& operator <<(ostream&, Ww1PlcFields&);
};

///////////////////////////////////////////////////////////// PlcBookmarks
class Ww1StringList
{
    sal_Char** pIdxA;
    USHORT nMax;
public:
    Ww1StringList( SvStream& rSt, ULONG nFc, USHORT nCb );
    ~Ww1StringList()
        {   if( pIdxA ) { delete pIdxA[0]; delete pIdxA; } }
    const String GetStr( USHORT nNum ) const;
    USHORT Count() const    { return nMax; }
    BOOL GetError() const   { return (nMax != 0) && !pIdxA; }
};

class Ww1PlcBookmarkTxt: public Ww1StringList
{
public:
    Ww1PlcBookmarkTxt(Ww1Fib& rFib) :
        Ww1StringList( rFib.GetStream(), rFib.GetFIB().fcSttbfbkmkGet(),
                       rFib.GetFIB().cbSttbfbkmkGet() )
    {}
};

class Ww1PlcBookmarkPos : public Ww1Plc
{
//  USHORT Fill(USHORT, BYTE&, String&, String&, String&);
public:
    Ww1PlcBookmarkPos(Ww1Fib& rFib, ULONG start, USHORT nCountBytes, BOOL bEnd)
        : Ww1Plc(rFib, start, nCountBytes, (bEnd) ? 0 : 2)
    {}

    BYTE* GetData(USHORT nIndex)    {   return Ww1Plc::GetData(nIndex); }
    // Position als CP
    ULONG WhereCP(USHORT nIndex)    { return Ww1Plc::Where(nIndex); }
    // absolut im file
    ULONG Where(USHORT nIndex)
    {
        return ( nIndex < Count() )
               ? Ww1Plc::Where(nIndex) + rFib.GetFIB().fcMinGet()
               : 0xffffffff;
    }
//  friend ostream& operator <<(ostream&, Ww1PlcBookmarks&);
};

//////////////////////////////////////////////////////////////// PlcHdd
class Ww1PlcHdd : public Ww1Plc
{
public:
    Ww1PlcHdd(Ww1Fib& rFib)
        : Ww1Plc(rFib, rFib.GetFIB().fcPlcfhddGet(),
                     rFib.GetFIB().cbPlcfhddGet(), 0)
    {}
};

/////////////////////////////////////////////////////////////////// Fkp
//
// aehnlich den plcs aufgebaute arrays, die sich auf eine groesze von
// 512 byte beschraenken.
//
class Ww1Fkp
{
protected:
    BYTE aFkp[512];
    USHORT nItemSize;
    BOOL bOK;
    BYTE* GetData(USHORT);
public:
    Ww1Fkp(SvStream&, ULONG, USHORT);
    friend ostream& operator <<(ostream&, Ww1Fkp&);
    USHORT Count() const            { return SVBT8ToByte(aFkp+511); }
    ULONG Where(USHORT); // wie im entsprechenden fkp
};

//////////////////////////////////////////////////////////////// FkpPap
class Ww1FkpPap : public Ww1Fkp
{
public:
    Ww1FkpPap(SvStream& rStream, ULONG ulFilePos)
        : Ww1Fkp(rStream, ulFilePos, 1)
    {}
    friend ostream& operator <<(ostream&, Ww1FkpPap&);
    BOOL Fill(USHORT,  BYTE*&, USHORT&, ULONG&, ULONG&);
    BOOL Fill(USHORT,  BYTE*&, USHORT&);
};

//////////////////////////////////////////////////////////////// FkpChp
class Ww1FkpChp : public Ww1Fkp
{
#ifdef DUMP
    SvStream& rStream;
    SvStream& GetStream()   { return rStream; }
#endif
public:
    Ww1FkpChp(SvStream& rStream, ULONG ulFilePos)
        : Ww1Fkp(rStream, ulFilePos, 1)
#ifdef DUMP
        , rStream(rStream)
#endif
    {}

    friend ostream& operator <<(ostream&, Ww1FkpChp&);
    BOOL Fill(USHORT, W1_CHP&, ULONG&, ULONG&);
    BOOL Fill(USHORT, W1_CHP&);
};

////////////////////////////////////////////////////////////// SprmPapx
class Ww1SprmPapx : public Ww1Sprm
{
    W1_PAPX aPapx;
    BYTE* Sprm(BYTE* p, USHORT nSize);
    USHORT SprmSize(BYTE* p, USHORT nSize);
public:
    Ww1SprmPapx(BYTE* p, USHORT nSize);
    friend ostream& operator <<(ostream&, Ww1SprmPapx&);
    void Start(Ww1Shell&, Ww1Manager&);
    void Stop(Ww1Shell&, Ww1Manager&);
};

/////////////////////////////////////////////////////////////// SprmSep
class Ww1SprmSep : public Ww1Sprm
{
public:
    Ww1SprmSep(Ww1Fib& rFib, ULONG ulFilePos)
        : Ww1Sprm(rFib.GetStream(), ulFilePos)
    {}
    friend ostream& operator <<(ostream&, Ww1SprmSep&);
};

///////////////////////////////////////////////////////////////// Assoc
class Ww1Assoc
{
    enum fields { FileNext, Dot, Title, Subject, KeyWords, Comments,
        Author, LastRevBy, DataDoc, HeaderDoc, Criteria1, Criteria2,
        Criteria3, Criteria4, Criteria5, Criteria6, Criteria7, MaxFields };

    Ww1Fib& rFib;
    sal_Char* pBuffer;
    sal_Char* pStrTbl[ MaxFields ];
    BOOL bOK;

    String GetStr(USHORT);

public:
    Ww1Assoc(Ww1Fib&);
    ~Ww1Assoc()             { delete pBuffer; }
    BOOL GetError() const   { return !bOK; }
    friend ostream& operator <<(ostream&, Ww1Assoc&);
    void Out(Ww1Shell&);
};

////////////////////////////////////////////////////////// HeaderFooter
//
// Header/Footer/Footnoteseparators sind einer nach dem naechsten in
// einem eigenen text gespeichert. ein plc trennt diesen text in
// einzelne teile.  diese werden durchnummeriert als ihdd. nun gibt es
// 9 verschiedene funktionen fuer diese texte. wird eine davon
// angefordert, ist es der erste, beim naechstern der 2 ihdd und so
// weiter.  welcher textteil also welcher typ sein wird laeszt sich
// nur bei sequenziellem lesen der datei bestimmen. die 9 typen sind:
// fusznoten-trenner, folge-fusznoten-trenner, folge-fusznoten-notiz,
// gerade-seiten kopfzeilen, ungerade seiten kopfzeilen, dto 2*
// fuszzeilen, kopf & fuszzeilen, wenn erste seite andere zeilen hat.
// HeaderFooter merkt sich fuer jede der 9 die momentane einstellung
// (jedoch nicht die alten) und den naechstvergebbaren ihdd.  ist ein
// teil nicht vorhanden bekommt er den wert 0xffff.
//
class Ww1HeaderFooter : public Ww1PlcHdd
{
    USHORT nextIhdd; // naechster textteil im HddText
    USHORT nFtnSep; // fusznoten trenner
    USHORT nFtnFollowSep; // folge fusznoten trenner
    USHORT nFtnNote; // folgefunsznotennotiz
    USHORT nEvenHeadL; // kopfzeilen grader seiten
    USHORT nOddHeadL; // kopfzeilen ungrader seiten
    USHORT nEvenFootL; // fuszzeilen grader seiten
    USHORT nOddFootL; // fuszzeilen ungerader seiten
    USHORT nFirstHeadL; // kopfzeilen der ersten seite
    USHORT nFirstFootL; // fuszzeilen der ersten seite
    enum HeaderFooterMode {
        None, FtnSep, FtnFollowSep, FtnNote, EvenHeadL, OddHeadL,
        EvenFootL, OddFootL, FirstHeadL, MaxHeaderFooterMode
    } eHeaderFooterMode;

public:
    Ww1HeaderFooter(Ww1Fib& rFib, USHORT grpfIhdt)
        : Ww1PlcHdd(rFib),
        nextIhdd(0),
        nFtnSep(0xffff),
        nFtnFollowSep(0xffff),
        nFtnNote(0xffff),
        nEvenHeadL(0xffff),
        nOddHeadL(0xffff),
        nEvenFootL(0xffff),
        nOddFootL(0xffff),
        nFirstHeadL(0xffff),
        nFirstFootL(0xffff),
        eHeaderFooterMode(None)
    {
        if (grpfIhdt & 0x0001) nFtnSep = nextIhdd++;
        if (grpfIhdt & 0x0002) nFtnFollowSep = nextIhdd++;
        if (grpfIhdt & 0x0004) nFtnNote = nextIhdd++;
    }
//  ~Ww1HeaderFooter() {}
    void SetGrpfIhdt(USHORT grpfIhdt)
    {
        if (grpfIhdt & 0x0001) nEvenHeadL = nextIhdd++;
        if (grpfIhdt & 0x0002) nOddHeadL = nextIhdd++;
        if (grpfIhdt & 0x0004) nEvenFootL = nextIhdd++;
        if (grpfIhdt & 0x0008) nOddFootL = nextIhdd++;
        if (grpfIhdt & 0x0010) nFirstHeadL = nextIhdd++;
        if (grpfIhdt & 0x0020) nFirstFootL = nextIhdd++;
        DBG_ASSERT(nextIhdd<=Count(), "Ww1HeaderFooter");
    }
    BOOL operator++(int)
    {
        BOOL bRet = TRUE;
        eHeaderFooterMode = (HeaderFooterMode)((short)eHeaderFooterMode + 1);
        if( eHeaderFooterMode == MaxHeaderFooterMode)
        {
            eHeaderFooterMode = None;
            bRet = FALSE;
        }
        return bRet;
    }
    BOOL FillFtnSep(ULONG& begin, ULONG& end)
    {
        if (nFtnSep == 0xffff)
            return FALSE;
        Fill(nFtnSep, begin, end);
        return TRUE;
    }
    BOOL FillFtnFollowSep(ULONG& begin, ULONG& end)
    {
        if (nFtnFollowSep == 0xffff)
            return FALSE;
        Fill(nFtnFollowSep, begin, end);
        return TRUE;
    }
    BOOL FillFtnNote(ULONG& begin, ULONG& end)
    {
        if (nFtnNote == 0xffff)
            return FALSE;
        Fill(nFtnNote, begin, end);
        return TRUE;
    }
    BOOL FillEvenHeadL(ULONG& begin, ULONG& end)
    {
        if (nEvenHeadL == 0xffff)
            return FALSE;
        Fill(nEvenHeadL, begin, end);
        return TRUE;
    }
    BOOL FillOddHeadL(ULONG& begin, ULONG& end)
    {
        if (nOddHeadL == 0xffff)
            return FALSE;
        Fill(nOddHeadL, begin, end);
        return TRUE;
    }
    BOOL FillEvenFootL(ULONG& begin, ULONG& end)
    {
        if (nEvenFootL == 0xffff)
            return FALSE;
        Fill(nEvenFootL, begin, end);
        return TRUE;
    }
    BOOL FillOddFootL(ULONG& begin, ULONG& end)
    {
        if (nOddFootL == 0xffff)
            return FALSE;
        Fill(nOddFootL, begin, end);
        return TRUE;
    }
    BOOL FillFirstHeadL(ULONG& begin, ULONG& end)
    {
        if (nFirstHeadL == 0xffff)
            return FALSE;
        Fill(nFirstHeadL, begin, end);
        return TRUE;
    }
    BOOL FillFirstFootL(ULONG& begin, ULONG& end)
    {
        if (nFirstFootL == 0xffff)
            return FALSE;
        Fill(nFirstFootL, begin, end);
        return TRUE;
    }
    void Start(Ww1Shell&, Ww1Manager&);
    void Stop(Ww1Shell&, Ww1Manager&, sal_Unicode&);
};

//////////////////////////////////////////////////////////////// Fields
class Ww1Fields : public Ww1PlcFields
{
    USHORT nPlcIndex;
    String sErgebnis; // das von word errechnete ergebniss
    SwField* pField;
    ULONG Where(USHORT nIndex)  // innerhalb des textes
        { return Ww1PlcFields::Where(nIndex) - rFib.GetFIB().fcMinGet(); }

public:
    Ww1Fields(Ww1Fib& rFib, ULONG ulFilePos, USHORT nCountBytes)
        : Ww1PlcFields(rFib, ulFilePos, nCountBytes), nPlcIndex(0), pField(0)
    {}
//  ~Ww1Fields() {}
    // innerhalb des textes
    ULONG Where()       { return Where(nPlcIndex); }
    void operator++(int)
    {
        DBG_ASSERT(nPlcIndex+1 <= Count(), "Ww1Fields");
        nPlcIndex++;
    }
    void Seek(ULONG ulNew)      { Ww1PlcFields::Seek(ulNew, nPlcIndex); }
    W1_FLD* GetData()
    {
        DBG_ASSERT(nPlcIndex < Count(), "Ww1Fields");
        return Ww1PlcFields::GetData(nPlcIndex);
    }
    ULONG GetLength();
    friend ostream& operator <<(ostream&, Ww1Manager&);
    void Start(Ww1Shell&, Ww1Manager&);
    void Stop(Ww1Shell&, Ww1Manager&, sal_Unicode&);
    void Out(Ww1Shell&, Ww1Manager&, USHORT=0);
};

class Ww1TextFields : public Ww1Fields
{
public:
    Ww1TextFields(Ww1Fib& rFib)
        : Ww1Fields(rFib, rFib.GetFIB().fcPlcffldMomGet(),
                     rFib.GetFIB().cbPlcffldMomGet())
    {}
};

class Ww1FootnoteFields : public Ww1Fields
{
public:
    Ww1FootnoteFields(Ww1Fib& rFib)
        : Ww1Fields(rFib, rFib.GetFIB().fcPlcffldFtnGet(),
                     rFib.GetFIB().cbPlcffldFtnGet())
    {}
};

class Ww1HeaderFooterFields : public Ww1Fields
{
public:
    Ww1HeaderFooterFields(Ww1Fib& rFib)
        : Ww1Fields(rFib, rFib.GetFIB().fcPlcffldHdrGet(),
                     rFib.GetFIB().cbPlcffldHdrGet())
    {}
};

class Ww1MacroFields : public Ww1Fields
{
public:
    Ww1MacroFields(Ww1Fib& rFib)
        : Ww1Fields(rFib, rFib.GetFIB().fcPlcffldMcrGet(),
                     rFib.GetFIB().cbPlcffldMcrGet())
    {}
};

//////////////////////////////////////////////////////////////// Bookmarks
class Ww1Bookmarks
{
    Ww1PlcBookmarkTxt aNames;
    Ww1PlcBookmarkPos* pPos[2];
    Ww1Fib& rFib;

    USHORT nPlcIdx[2];
    USHORT nIsEnd;
    BOOL bOK;
//  ULONG Where(USHORT nIndex) { // innerhalb des textes
//      return Ww1PlcFields::Where(nIndex) - rFib.GetFIB().fcMinGet(); }
public:
    Ww1Bookmarks(Ww1Fib& rFib);
    ~Ww1Bookmarks()
    {
            delete pPos[1];
            delete pPos[0];
    }
    ULONG Where() const     { return pPos[nIsEnd]->WhereCP(nPlcIdx[nIsEnd]); }
    void operator++(int);
    BOOL GetError() const   { return !bOK; }
    long GetHandle() const;
    BOOL GetIsEnd() const   { return ( nIsEnd ) ? TRUE : FALSE; }
    const String GetName() const;
    long Len() const;
    friend ostream& operator <<(ostream&, Ww1Bookmarks&);
    void Start(Ww1Shell&, Ww1Manager&);
    void Stop(Ww1Shell&, Ww1Manager&, sal_Unicode&);
    void Out(Ww1Shell&, Ww1Manager&, USHORT=0);
};

///////////////////////////////////////////////////////////// Footnotes
class Ww1Footnotes : public Ww1PlcFootnoteRef
{
    USHORT nPlcIndex;
    Ww1PlcFootnoteTxt aText;
    BOOL bStarted;
public:
    Ww1Footnotes(Ww1Fib& rFib)
        : Ww1PlcFootnoteRef(rFib), nPlcIndex(0), aText(rFib), bStarted(FALSE)
    {}
//  ~Ww1Footnotes() {}
    // innerhalb des textes
    ULONG Where()
    {
        ULONG ulRet = 0xffffffff;
        if (Count())
            ulRet = Ww1PlcFootnoteRef::Where(nPlcIndex);
        return ulRet;
    }
    void operator++(int)
    {
        DBG_ASSERT(nPlcIndex+1 <= Count(), "Ww1Footnotes");
        nPlcIndex++;
    }
    void Start(Ww1Shell&, Ww1Manager&);
    void Stop(Ww1Shell&, Ww1Manager&, sal_Unicode&);
};

/////////////////////////////////////////////////////////////////// Sep
class Ww1Sep : public Ww1PlcSep
{
    USHORT nPlcIndex;
    Ww1HeaderFooter aHdd;
public:
    Ww1Sep(Ww1Fib& rFib, USHORT grpfIhdt)
        : Ww1PlcSep(rFib), aHdd(rFib, grpfIhdt),nPlcIndex(0)
    {}
//  ~Ww1Sep() {}

    Ww1HeaderFooter& GetHdd()   { return aHdd; }
    void operator++(int)        { nPlcIndex++; }
    BYTE* GetData()             { return Ww1PlcSep::GetData(nPlcIndex); }
    // innerhalb des textes
    ULONG Where()               { return Ww1PlcSep::Where(nPlcIndex); }
    void SetGrpfIhdt(BYTE grpfIhdt)
    {
        GetHdd().SetGrpfIhdt(grpfIhdt);
//      GetHdd().Start(rOut, rMan);
    }
    void Start(Ww1Shell&, Ww1Manager&);
    void Stop(Ww1Shell& rOut, Ww1Manager& rMan, sal_Unicode& c)
        { aHdd.Stop(rOut, rMan, c); }
};

/////////////////////////////////////////////////////////////////// Pap
class Ww1Pap : public Ww1PlcPap
{
    USHORT nPlcIndex;
    USHORT nPushedPlcIndex;
    USHORT nFkpIndex;
    USHORT nPushedFkpIndex;
    ULONG ulOffset;
    Ww1FkpPap* pPap;

    BOOL FindSprm(USHORT nId, BYTE* pStart, BYTE* pEnd);
    void UpdateIdx()
    {
        if (pPap && nFkpIndex >= pPap->Count() )
        {
            delete pPap;
            pPap = NULL;
            nPlcIndex++;
        }
        if( !pPap )
            Where();
    }
    BOOL HasId0(USHORT nId);

public:
    Ww1Pap(Ww1Fib& rFib);
    ~Ww1Pap()   { delete pPap; }
    ULONG Where( BOOL bSetIndex = TRUE ); // innerhalb des textes
    void operator++(int);
    BOOL FillStart(BYTE*& p, USHORT& nSize)
    {
        UpdateIdx();
        return pPap->Fill(nFkpIndex, p, nSize);
    }
    BOOL FillStop(BYTE*& p, USHORT& nSize)
    {
        return nFkpIndex ? pPap->Fill(nFkpIndex-1, p, nSize) : FALSE;
    }
    void Start(Ww1Shell&, Ww1Manager&);
    void Stop(Ww1Shell&, Ww1Manager&, sal_Unicode&);
    void Seek(ULONG);
    void Push(ULONG ulOffset = 0)
    {
        DBG_ASSERT(!Pushed(), "Ww1Pap");
        nPushedPlcIndex = nPlcIndex;
        nPushedFkpIndex = nFkpIndex;
        Seek(ulOffset);
        this->ulOffset = ulOffset;
        delete pPap;
        pPap = NULL;
    }
    BOOL Pushed()
    {
        return nPushedPlcIndex != 0xffff;
    }
    void Pop()
    {
        DBG_ASSERT(Pushed(), "Ww1Pap");
        ulOffset = 0;
        nPlcIndex = nPushedPlcIndex;
        nFkpIndex = nPushedFkpIndex;
        nPushedPlcIndex = 0xffff;
        nPushedFkpIndex = 0xffff;
        delete pPap;
        pPap = NULL;
        Where( FALSE );
    }
    BOOL HasId(USHORT nId);
    BOOL NextHas(USHORT nId);
};

/////////////////////////////////////////////////////////////////// Chp
class Ww1Chp : public Ww1PlcChp
{
    USHORT nPlcIndex;
    USHORT nPushedPlcIndex;
    USHORT nFkpIndex;
    USHORT nPushedFkpIndex;
    ULONG ulOffset;
    Ww1FkpChp* pChp;
    void UpdateIdx()
    {
        if (pChp && nFkpIndex >= pChp->Count() )
        {
            delete pChp;
            pChp = NULL;
            nPlcIndex++;
        }
        if( !pChp )
            Where();
    }

public:
    Ww1Chp( Ww1Fib& rFib );
    ~Ww1Chp()   { delete pChp; }
    ULONG Where( BOOL bSetIndex = TRUE ); // innerhalb des textes
    void operator++(int);
    BOOL FillStart(W1_CHP& rChp)
    {
        UpdateIdx();
        return pChp->Fill(nFkpIndex, rChp);
    }
    BOOL FillStop(W1_CHP& rChp)
    { return nFkpIndex ? pChp->Fill(nFkpIndex-1, rChp) : FALSE;  }
    void Start(Ww1Shell&, Ww1Manager&);
    void Stop(Ww1Shell&, Ww1Manager&, sal_Unicode&);
    void Seek(ULONG);
    void Push(ULONG ulOffset = 0)
    {
        DBG_ASSERT(!Pushed(), "Ww1Chp");
        nPushedPlcIndex = nPlcIndex;
        nPushedFkpIndex = nFkpIndex;
        Seek(ulOffset);
        this->ulOffset = ulOffset;
        delete pChp;
        pChp = NULL;
    }
    BOOL Pushed()               { return nPushedPlcIndex != 0xffff; }
    void Pop()
    {
        DBG_ASSERT(Pushed(), "Ww1Chp");
        ulOffset = 0;
        nPlcIndex = nPushedPlcIndex;
        nFkpIndex = nPushedFkpIndex;
        nPushedPlcIndex = 0xffff;
        nPushedFkpIndex = 0xffff;
        delete pChp;
        pChp = NULL;
        Where( FALSE );
    }
};

/////////////////////////////////////////////////////////////// Manager
//
// zentraler aufhaenger der ww-seite des filters, konstruiert aus dem
// inputstream (ww-datei) enthaelt er alles, was noetig ist, um in die
// shell (pm-seite) gepiped zu werden.
//
class Ww1Manager
{
    BOOL bOK;
    BOOL bInTtp;
    BOOL bInStyle;
    BOOL bStopAll;
    Ww1Fib aFib;
    Ww1Dop aDop;
    Ww1Fonts aFonts;
// ab jetzt alles paarig, fuer 'pushed':
    Ww1DocText aDoc;
    Ww1PlainText* pDoc;
    ULONG ulDocSeek;
    ULONG* pSeek;
    Ww1TextFields aFld;
    Ww1Fields* pFld;
// selbst 'push'bar:
    Ww1Chp aChp;
    Ww1Pap aPap;
// nicht in textbereichen vorhanden, wenn ge'pushed'
    Ww1Footnotes aFtn;
    Ww1Bookmarks aBooks;
    Ww1Sep aSep;

    void OutStop( Ww1Shell&, sal_Unicode );
    void OutStart( Ww1Shell& );
    void Out(Ww1Shell&, sal_Unicode );

public:
    Ww1Manager(SvStream& rStrm, ULONG nFieldFlgs);
    BOOL GetError() const       { return !bOK; }

// Fuer Tabellen
    void SetInTtp(BOOL bSet = TRUE)     { bInTtp = bSet; }
    BOOL IsInTtp() const                { return bInTtp; }
    void SetInStyle(BOOL bSet = TRUE)   { bInStyle = bSet; }
    BOOL IsInStyle() const              { return bInStyle; }
    void SetStopAll(BOOL bSet = TRUE)   { bStopAll = bSet; }
    BOOL IsStopAll() const              { return bStopAll; }
    BOOL HasInTable();
    BOOL HasTtp();
    BOOL NextHasTtp();
    BOOL LastHasTtp();

// Fuer Flys
    BOOL HasPPc();
    BOOL HasPDxaAbs();

    Ww1Fib& GetFib()                    { return aFib; }
    Ww1PlainText& GetText()             { return *pDoc; }
    Ww1Dop& GetDop()                    { return aDop; }
    Ww1Sep& GetSep()                    { return aSep; }
    // innerhalb des textes
    ULONG Where()                       { return pDoc->Where(); }
    void Fill( sal_Unicode& rChr )      { pDoc->Out( rChr ); }
    BYTE Fill( String& rStr, ULONG ulLen)
    {
        ulLen += pDoc->Where();
        return pDoc->Out(rStr, ulLen);
    }
    SvxFontItem GetFont(USHORT nFCode);
    static BOOL IsValidFib(const BYTE*, USHORT);
    friend Ww1Shell& operator <<(Ww1Shell&, Ww1Manager&);
    friend ostream& operator <<(ostream&, Ww1Manager&);
    BOOL Pushed()                       { return pDoc != &aDoc; }
    void Pop();
    void Push0(Ww1PlainText* pDoc, ULONG, Ww1Fields* = 0);
    void Push1(Ww1PlainText* pDoc, ULONG ulSeek, ULONG ulSeek2 = 0,
               Ww1Fields* = 0);
};

/**********************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/ww1/w1class.hxx,v 1.1.1.1 2000-09-18 17:14:57 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.19  2000/09/18 16:04:56  willem.vandorp
      OpenOffice header added.

      Revision 1.18  2000/09/09 12:53:29  willem.vandorp
      Header and footer replaced

      Revision 1.17  2000/05/11 16:06:48  jp
      Changes for Unicode

      Revision 1.16  1998/02/19 16:01:14  HJS
      fuer sh eingechecked


      Rev 1.15   19 Feb 1998 17:01:14   HJS
   fuer sh eingechecked

      Rev 1.12   14 Nov 1997 10:55:28   OS
   SH-Aenderung fuer Textmarken

      Rev 1.11   09 Oct 1997 14:13:46   JP
   Aenderungen von SH

      Rev 1.10   03 Sep 1997 14:16:28   SH
   Flys, Tabellen ok, Style-Abhaengigkeiten u.v.a.m

      Rev 1.9   11 Aug 1997 12:49:26   SH
   Flys und einfache Tabellen und Tabulatoren

      Rev 1.8   18 Jul 1997 09:31:38   MA
   Stand von SH, #41738# GPF und Tabellen etwas besser

      Rev 1.5   16 Jun 1997 13:09:04   MA
   Stand Micheal Dietrich

      Rev 1.4   12 Mar 1997 19:11:24   SH
   MDT: Progressbar, Pagedesc-Bug, Spalten, Anfang Tabellen u.a.

      Rev 1.3   10 Jan 1997 18:43:58   SH
   Stabiler und mehr von MDT

      Rev 1.2   28 Nov 1996 18:08:06   SH
   Schneller Schoener Weiter von MDT

      Rev 1.1   30 Sep 1996 23:22:18   SH
   neu von MDA


**********************************************************************/
