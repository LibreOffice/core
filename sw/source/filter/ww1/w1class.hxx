/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include <tools/string.hxx>

// local
#include <w1struct.hxx>

#ifdef DUMP
#include <fstream.h>
#endif

#include <ostream>

using std::ostream;

namespace editeng { class SvxBorderLine; }

class SvxFontItem;
class SvxBoxItem;
class SvStream;
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
    sal_Bool bOK;
    SvStream& rStream;
public:
    Ww1Fib(SvStream&);
    friend ostream& operator <<(ostream&, Ww1Fib&);
    W1_FIB& GetFIB()        { return aFib; }
    sal_Bool GetError()         { return !bOK; }
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
    sal_Bool bOK;
public:
    Ww1Dop(Ww1Fib&);
    sal_Bool GetError() {
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
    sal_uLong ulFilePos;
    sal_uLong ulCountBytes;
    sal_uLong ulSeek;
    sal_Bool bOK;
public:
    Ww1PlainText(Ww1Fib& rWwFib, sal_uLong nFilePos, sal_uLong nCountBytes);
    // innerhalb des textes
    sal_uLong Where() const                 { return ulSeek; }
    void Seek( sal_uLong ulNew )
        {
            OSL_ENSURE(ulNew < ulCountBytes, "Ww1PlainText");
            if (ulNew < ulCountBytes)
                ulSeek = ulNew;
        }

    sal_uLong Count() const                 { return ulCountBytes; }
    void SetCount(sal_uLong ulNew)
        {
            ulNew += ulSeek;
            if (ulCountBytes > ulNew)
                ulCountBytes = ulNew;
        }
    void operator++()
    {
        OSL_ENSURE(ulSeek+1<ulCountBytes, "Ww1PlainText");
        ulSeek++;
    }
    sal_Bool GetError()                     { return !bOK; }
    sal_Unicode Out( Ww1Shell&, sal_uLong& );
    sal_Unicode Out( String&, sal_uLong=0xffffffff);
    sal_Unicode Out( sal_Unicode& );
    friend ostream& operator <<(ostream&, Ww1PlainText&);
    String& Fill( String&, sal_uLong=0, sal_uLong=0xffffffff );
    sal_Unicode operator []( sal_uLong );
    String GetText( sal_uLong ulOffset, sal_uLong nLen ) const;

    enum Consts { MinChar = 32 };
    static sal_Bool IsChar( sal_Unicode c )     { return c >= MinChar; }
};

/////////////////////////////////////////////////////////////// DocText
class Ww1DocText : public Ww1PlainText
{
public:
    Ww1DocText(Ww1Fib& rFibL) :
        Ww1PlainText(rFibL, rFibL.GetFIB().fcMinGet(),
         rFibL.GetFIB().ccpTextGet()) {
        }
};

/////////////////////////////////////////////////////////////// FtnText
class Ww1FtnText : public Ww1PlainText
{
public:
    sal_uLong Offset(Ww1Fib& rFibL) {
        return rFibL.GetFIB().ccpTextGet(); }
    Ww1FtnText(Ww1Fib& rFibL) :
        Ww1PlainText(rFibL, rFibL.GetFIB().fcMinGet() +
         Offset(rFibL), rFibL.GetFIB().ccpFtnGet()) {
        }
};

/////////////////////////////////////////////////////////////// HddText
class Ww1HddText : public Ww1PlainText
{
public:
    sal_uLong Offset(Ww1Fib& rFibL) {
        return rFibL.GetFIB().ccpTextGet() + rFibL.GetFIB().ccpFtnGet(); }
    Ww1HddText(Ww1Fib& rFibL) :
        Ww1PlainText(rFibL, rFibL.GetFIB().fcMinGet() +
         Offset(rFibL), rFibL.GetFIB().ccpHddGet()) {
        }
};

/////////////////////////////////////////////////////////////// McrText
class Ww1McrText : public Ww1PlainText
{
public:
    sal_uLong Offset(Ww1Fib& rFibL) {
        return rFibL.GetFIB().ccpTextGet() + rFibL.GetFIB().ccpFtnGet()
         + rFibL.GetFIB().ccpHddGet(); }
    Ww1McrText(Ww1Fib& rFibL) :
        Ww1PlainText(rFibL, rFibL.GetFIB().fcMinGet() +
         Offset(rFibL), rFibL.GetFIB().ccpMcrGet()) {
        }
};

/////////////////////////////////////////////////////////////// AtnText
class Ww1AtnText : public Ww1PlainText
{
public:
    sal_uLong Offset(Ww1Fib& rFibL) {
        return rFibL.GetFIB().ccpTextGet() + rFibL.GetFIB().ccpFtnGet()
         + rFibL.GetFIB().ccpHddGet() + rFibL.GetFIB().ccpMcrGet(); }
    Ww1AtnText(Ww1Fib& rFibL) :
        Ww1PlainText(rFibL, rFibL.GetFIB().fcMinGet() +
         Offset(rFibL), rFibL.GetFIB().ccpAtnGet()) {
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
    sal_uInt8 stcBase;
    sal_uInt8 stcNext;
    sal_Bool bUsed;
public:
    Ww1Style();
    ~Ww1Style();
    sal_Bool IsUsed() const                 { return bUsed; }
    void SetDefaults(sal_uInt8);
    void SetParent(Ww1StyleSheet* newParent)    { pParent = newParent; }
    void SetName(const String& rName)   { bUsed = sal_True; aName = rName; }
    const String& GetName() const       { return aName; }
    Ww1Style& GetBase();
    sal_uInt16 GetnBase() const             { return stcBase; }
    sal_uInt16 GetnNext() const             { return stcNext; }
    sal_uInt16 ReadName(sal_uInt8*&, sal_uInt16&, sal_uInt16 stc);
    sal_uInt16 ReadChpx(sal_uInt8*&, sal_uInt16&);
    sal_uInt16 ReadPapx(sal_uInt8*&, sal_uInt16&);
    sal_uInt16 ReadEstcp(sal_uInt8*&, sal_uInt16&);
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
    sal_uInt16 cstcStd; // count style code standard
    Ww1Fib& rFib;
    sal_Bool bOK;
    sal_uInt16 ReadNames(sal_uInt8*&, sal_uInt16&);
    sal_uInt16 ReadChpx(sal_uInt8*&, sal_uInt16&);
    sal_uInt16 ReadPapx(sal_uInt8*&, sal_uInt16&);
    sal_uInt16 ReadEstcp(sal_uInt8*&, sal_uInt16&);

    void OutDefaults(Ww1Shell& rOut, Ww1Manager& rMan, sal_uInt16 stc);
    void OutOne(Ww1Shell& rOut, Ww1Manager& rMan, sal_uInt16 stc);
    void OutOneWithBase(Ww1Shell& rOut, Ww1Manager& rMan, sal_uInt16 stc,
                        sal_uInt8* pbStopRecur );
public:
    Ww1StyleSheet(Ww1Fib& rFib);
    Ww1Style& GetStyle(sal_uInt16 stc) {
        return aStyles[stc]; }
    sal_uInt16 Count() {
        return 256; }
    friend ostream& operator <<(ostream&, Ww1StyleSheet&);
    void Out(Ww1Shell&, Ww1Manager&);
    friend class Ww1Style;
    sal_Bool GetError() {
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
    sal_uLong nFieldFlags;
    sal_uInt16 nMax; // Array-Groesse
    sal_Bool bOK;
public:
    Ww1Fonts(Ww1Fib&, sal_uLong nFieldFlgs);
    ~Ww1Fonts() {
        if (pFontA)
            DELETEZ(pFontA[0]);
        DELETEZ(pFontA); }
    W1_FFN* GetFFN(sal_uInt16 nNum);
    sal_uInt16 Count() {
        return nMax; }
    friend ostream& operator <<(ostream&, Ww1Fonts&);
    sal_Bool GetError() {
        return !bOK; }
    SvxFontItem GetFont(sal_uInt16);
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
    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&);
    void Stop(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&);
    virtual ostream& Dump(ostream&, sal_uInt8*, sal_uInt16);
    const sal_Char* sName;
#else
    virtual void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&);
    virtual void Stop(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&);
    ostream& Dump(ostream&, sal_uInt8*, sal_uInt16);
#endif
    virtual sal_uInt16 Size(sal_uInt8*);
    sal_uInt16 nCountBytes;

    Ww1SingleSprm(sal_uInt16 nBytes, const sal_Char* /*pName*/ = 0 )
        : nCountBytes(nBytes)
#ifdef DUMP
        , sName( pName)
#endif
        {
        }
    virtual ~Ww1SingleSprm();
};

class Ww1SingleSprmByteSized : public Ww1SingleSprm {
public:
    sal_uInt16 Size(sal_uInt8*);
    Ww1SingleSprmByteSized(sal_uInt16 nBytes, sal_Char* sName = 0) :
        Ww1SingleSprm(nBytes, sName) {
        }
};

class Ww1SingleSprmWordSized : public Ww1SingleSprm {
public:
    sal_uInt16 Size(sal_uInt8*);
    Ww1SingleSprmWordSized(sal_uInt16 nBytes, sal_Char* sName = 0) :
        Ww1SingleSprm(nBytes, sName) {
        }
};

class Ww1SingleSprmByte : public Ww1SingleSprm {
public:
    ostream& Dump(ostream&, sal_uInt8*, sal_uInt16);
    Ww1SingleSprmByte(sal_Char* sName = 0) :
        Ww1SingleSprm(1, sName) {
        }
};

class Ww1SingleSprmBool : public Ww1SingleSprmByte {
public:
    ostream& Dump(ostream&, sal_uInt8*, sal_uInt16);
    Ww1SingleSprmBool(sal_Char* sName = 0) :
        Ww1SingleSprmByte(sName) {
        }
};

class Ww1SingleSprm4State : public Ww1SingleSprmByte {
public:
    ostream& Dump(ostream&, sal_uInt8*, sal_uInt16);
    Ww1SingleSprm4State(sal_Char* sName = 0) :
        Ww1SingleSprmByte(sName) {
        }
};

class Ww1SingleSprmWord : public Ww1SingleSprm {
public:
    ostream& Dump(ostream&, sal_uInt8*, sal_uInt16);
    Ww1SingleSprmWord(sal_Char* sName = 0)
    : Ww1SingleSprm(2, sName) {}
};

class Ww1SingleSprmLong : public Ww1SingleSprm {
public:
    ostream& Dump(ostream&, sal_uInt8*, sal_uInt16);
    Ww1SingleSprmLong(sal_Char* sName = 0) :
        Ww1SingleSprm(4, sName) {
        }
};

class Ww1SingleSprmTab : public Ww1SingleSprm {
public:
    ostream& Dump(ostream&, sal_uInt8*, sal_uInt16);
    sal_uInt16 Size(sal_uInt8*);
    Ww1SingleSprmTab(sal_uInt16 nBytes, sal_Char* sName = 0) :
        Ww1SingleSprm(nBytes, sName) {
        }
};

class Ww1SingleSprmPJc : public Ww1SingleSprmByte {
public:
    Ww1SingleSprmPJc(sal_Char* sName) :
        Ww1SingleSprmByte(sName) {
        }
    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&);
    void Stop(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&);
};

class Ww1SingleSprmPDxa : public Ww1SingleSprmWord {
public:
    Ww1SingleSprmPDxa(sal_Char* sName) :
        Ww1SingleSprmWord(sName) {
        }
    void Stop(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&);
};

class Ww1SingleSprmPDxaRight : public Ww1SingleSprmPDxa {
public:
    Ww1SingleSprmPDxaRight(sal_Char* sName) :
        Ww1SingleSprmPDxa(sName) {
        }
    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&);
};

class Ww1SingleSprmPDxaLeft : public Ww1SingleSprmPDxa {
public:
    Ww1SingleSprmPDxaLeft(sal_Char* sName) :
        Ww1SingleSprmPDxa(sName) {
        }
    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&);
};

class Ww1SingleSprmPDxaLeft1 : public Ww1SingleSprmPDxa {
public:
    Ww1SingleSprmPDxaLeft1(sal_Char* sName) :
        Ww1SingleSprmPDxa(sName) {
        }
    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&);
};

class Ww1SingleSprmPFKeep : public Ww1SingleSprmBool {
public:
    Ww1SingleSprmPFKeep(sal_Char* sName) :
        Ww1SingleSprmBool(sName) {
        }
    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&);
    void Stop(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&);
};

class Ww1SingleSprmPFKeepFollow : public Ww1SingleSprmBool {
public:
    Ww1SingleSprmPFKeepFollow(sal_Char* sName) :
        Ww1SingleSprmBool(sName) {
        }
    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&);
    void Stop(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&);
};

class Ww1SingleSprmPPageBreakBefore : public Ww1SingleSprmBool {
public:
    Ww1SingleSprmPPageBreakBefore(sal_Char* sName) :
        Ww1SingleSprmBool(sName) {
        }
    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&);
    void Stop(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&);
};

class Ww1SingleSprmPBrc : public Ww1SingleSprmWord {
protected:
    // spezielle start-routine, je nach sprm verschieden versorgt
    // mit einem BoxItem.
    void Start(Ww1Shell&, sal_uInt8, W1_BRC10*, sal_uInt16, Ww1Manager&, SvxBoxItem&);
    void Start(Ww1Shell&, sal_uInt8, W1_BRC*, sal_uInt16, Ww1Manager&, SvxBoxItem&);

    using Ww1SingleSprm::Start;

public:
    Ww1SingleSprmPBrc(sal_Char* sName) :
        Ww1SingleSprmWord(sName) {
        }
    void Stop(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&);
    // SetBorder() wird auch fuer Tabellen gebraucht, deshalb public
    static editeng::SvxBorderLine* SetBorder(editeng::SvxBorderLine*, W1_BRC10*);
};

#define BRC_TOP ((sal_uInt16)0)
#define BRC_LEFT ((sal_uInt16)1)
#define BRC_BOTTOM ((sal_uInt16)2)
#define BRC_RIGHT ((sal_uInt16)3)
#define BRC_ANZ ((sal_uInt16)BRC_RIGHT-BRC_TOP+1)

// Die BRC-struktur fuer 1.0 versionen von word sind verschieden von
// denen der folgenden versionen. diese werden zum glueck aber auch
// von anderen sprms abgerufen.
// SH: Ab sofort alle 4 Umrandungen ueber nur 1 Klasse.
class Ww1SingleSprmPBrc10 : public Ww1SingleSprmPBrc
{
    sal_uInt16 nLine;   // BRC_TOP, BRC_LEFT, ...

    using Ww1SingleSprmPBrc::Start;

public:
    Ww1SingleSprmPBrc10(sal_uInt16 nL, sal_Char* sName)
    : Ww1SingleSprmPBrc(sName), nLine(nL) {}

    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&);
};

class Ww1SingleSprmParaSpace : public Ww1SingleSprmWord {
public:
    Ww1SingleSprmParaSpace(sal_Char* sName)
    : Ww1SingleSprmWord(sName) {}
    void Stop(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&);
};

class Ww1SingleSprmPDyaBefore : public Ww1SingleSprmParaSpace {
public:
    Ww1SingleSprmPDyaBefore(sal_Char* sName)
    : Ww1SingleSprmParaSpace(sName) {}
    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&);
};

class Ww1SingleSprmPDyaAfter : public Ww1SingleSprmParaSpace {
public:
    Ww1SingleSprmPDyaAfter(sal_Char* sName) :
        Ww1SingleSprmParaSpace(sName) {
        }
    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&);
};

class Ww1SingleSprmPDyaLine : public Ww1SingleSprmWord {
public:
    Ww1SingleSprmPDyaLine(sal_Char* sName) :
        Ww1SingleSprmWord(sName) {
        }
    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&);
    void Stop(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&);
};

class Ww1SingleSprmPChgTabsPapx : public Ww1SingleSprmByteSized {
public:
    Ww1SingleSprmPChgTabsPapx(sal_Char* sName) :
        Ww1SingleSprmByteSized(0, sName) {
        }
  // Size() ist noch nicht aktiviert !!
    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&);
    void Stop(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&);
};

class Ww1SingleSprmSGprfIhdt : public Ww1SingleSprmByte {
public:
    Ww1SingleSprmSGprfIhdt(sal_Char* sName) :
        Ww1SingleSprmByte(sName) {
        }
    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&);
};

class Ww1SingleSprmSColumns : public Ww1SingleSprmWord {
public:
    Ww1SingleSprmSColumns(sal_Char* sName) :
        Ww1SingleSprmWord(sName) {
        }
    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&);
};

class Ww1SingleSprmPFInTable : public Ww1SingleSprmBool {
public:
    Ww1SingleSprmPFInTable(sal_Char* sName) :
        Ww1SingleSprmBool(sName) {
        }
    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&);
    void Stop(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&);
};

class Ww1SingleSprmPTtp : public Ww1SingleSprmBool {
public:
    Ww1SingleSprmPTtp(sal_Char* sName) :
        Ww1SingleSprmBool(sName) {
        }
    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&);
    void Stop(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&);
};

class Ww1SingleSprmTJc : public Ww1SingleSprmWord {
public:
    Ww1SingleSprmTJc(sal_Char* sName)
    : Ww1SingleSprmWord(sName) {}
};

class Ww1SingleSprmTDxaGapHalf : public Ww1SingleSprmWord {
public:
    Ww1SingleSprmTDxaGapHalf(sal_Char* sName) :
        Ww1SingleSprmWord(sName) {
        }
    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&);
};

class Ww1SingleSprmTDefTable10 : public Ww1SingleSprmWordSized {
public:
    Ww1SingleSprmTDefTable10(sal_Char* sName) :
        Ww1SingleSprmWordSized(0, sName) {
        }
    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&);
};

class Ww1SingleSprmTDyaRowHeight : public Ww1SingleSprmWord {
public:
    Ww1SingleSprmTDyaRowHeight(sal_Char* sName) :
        Ww1SingleSprmWord(sName) {
        }
    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&);
};

// Klassendefinitionen fuer Tabellen-Fastsave-Attribute
// Da wir kein Fastsave unterstuetzen, brauchen wir's nicht

// Klassendefinitionen fuer Apos ( == Flys )

class Ww1SingleSprmPpc : public Ww1SingleSprmByte {
public:
    Ww1SingleSprmPpc(sal_Char* sName) :
        Ww1SingleSprmByte(sName) {
        }
    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&);
};

class Ww1SingleSprmPDxaAbs : public Ww1SingleSprmWord {
public:
    Ww1SingleSprmPDxaAbs(sal_Char* sName) :
        Ww1SingleSprmWord(sName) {
        }
    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&);
};

class Ww1SingleSprmPDyaAbs : public Ww1SingleSprmWord {
public:
    Ww1SingleSprmPDyaAbs(sal_Char* sName) :
        Ww1SingleSprmWord(sName) {
        }
    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&);
};

class Ww1SingleSprmPDxaWidth : public Ww1SingleSprmWord {
public:
    Ww1SingleSprmPDxaWidth(sal_Char* sName) :
        Ww1SingleSprmWord(sName) {
        }
    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&);
};

class Ww1SingleSprmPFromText : public Ww1SingleSprmWord {
public:
    Ww1SingleSprmPFromText(sal_Char* sName) :
        Ww1SingleSprmWord(sName) {
        }
    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&);
};

////////////////////////////////////////////////////////////////// Sprm
//
// der tatsaechlich in der datei auftretende datentyp Sprm
//
class Ww1Sprm
{
    sal_Bool ReCalc();
    static Ww1SingleSprm* aTab[256];
    static Ww1SingleSprm* pSingleSprm;
protected:
    static void InitTab();
    Ww1SingleSprm& GetTab(sal_uInt16 nId)
    {
        if( !pSingleSprm )
            InitTab();
        return aTab[ nId ] ? *aTab[nId] : *pSingleSprm;
    }

    sal_uInt8* p;
    sal_uInt16 nCountBytes;
    sal_Bool bOK;
    sal_uInt16* pArr;
    sal_uInt16 count;
// ohne Token, mit laengen-byte/word
    sal_uInt16 GetSize(sal_uInt8 nId, sal_uInt8* pSprm);
// mit Token und LaengenByte
    sal_uInt16 GetSizeBrutto(sal_uInt8* pSprm) {
        sal_uInt8 nId = *pSprm++;
        return GetSize(nId, pSprm) + 1; }
// gibt fuer nTh element id, size & zeiger auf daten:
//  sal_Bool Fill(sal_uInt16, sal_uInt8&, sal_uInt16&, sal_uInt8*&);
public:
// SH: brauche ich public
// gibt fuer nTh element id, size & zeiger auf daten:
    sal_Bool Fill(sal_uInt16, sal_uInt8&, sal_uInt16&, sal_uInt8*&);

    Ww1Sprm(sal_uInt8*, sal_uInt16);
    Ww1Sprm(SvStream&, sal_uLong);
    ~Ww1Sprm();
    friend ostream& operator <<(ostream&, Ww1Sprm&);
    void Start(Ww1Shell&, Ww1Manager&);
    void Start(Ww1Shell&, Ww1Manager&, sal_uInt16);
    void Stop(Ww1Shell&, Ww1Manager&);
    sal_Bool IsUsed() {
        return nCountBytes != 255; }
    sal_uInt16 Count() {
        return count; }
    sal_Bool GetError() {
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
    sal_Bool bOK;
    W1_PIC* pPic;
public:
    Ww1Picture(SvStream&, sal_uLong);
    ~Ww1Picture() {
        }
    sal_Bool GetError() {
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
    sal_uInt8* p;
    sal_uInt16 nCountBytes;
    sal_uInt16 iMac;
    sal_uInt16 nItemSize;
    sal_Bool bOK;
protected:
    Ww1Fib& rFib;
    sal_uInt8* GetData(sal_uInt16);
public:
    Ww1Plc(Ww1Fib&, sal_uLong, sal_uInt16, sal_uInt16);
    ~Ww1Plc();
    friend ostream& operator <<(ostream&, Ww1Plc&);
    sal_uLong Where(sal_uInt16); // wie im jeweiligen plc
    void Seek(sal_uLong, sal_uInt16&);
    void Fill(sal_uInt16 nIndex, sal_uLong& begin, sal_uLong& end) {
        begin = Where(nIndex);
        end = Where(nIndex+1); }
    sal_uInt16 Count() {
        return iMac; }
    sal_Bool GetError() {
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
    Ww1PlcGlossary(Ww1Fib& rFibL) :
        Ww1Plc(rFibL, rFibL.GetFIB().fcPlcfglsyGet(),
         rFibL.GetFIB().cbPlcfglsyGet(), 0) {
        }
};

////////////////////////////////////////////////////// PlcAnnotationRef
class Ww1PlcAnnotationRef : public Ww1Plc
{
public:
    Ww1PlcAnnotationRef(Ww1Fib& rFibL) :
        Ww1Plc(rFibL, rFibL.GetFIB().fcPlcfandRefGet(),
         rFibL.GetFIB().cbPlcfandRefGet(), 0) {
        }
};

////////////////////////////////////////////////////// PlcAnnotationTxt
class Ww1PlcAnnotationTxt : public Ww1Plc
{
public:
    Ww1PlcAnnotationTxt(Ww1Fib& rFibL) :
        Ww1Plc(rFibL, rFibL.GetFIB().fcPlcfandTxtGet(),
         rFibL.GetFIB().cbPlcfandTxtGet(), 0) {
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
    Ww1PlcSep(Ww1Fib& rFibL):
        Ww1Plc(rFibL, rFibL.GetFIB().fcPlcfsedGet(),
         rFibL.GetFIB().cbPlcfsedGet(), 6) {
        }
    friend ostream& operator <<(ostream&, Ww1PlcSep&);
};

//////////////////////////////////////////////////////////////// PlcChp
class Ww1PlcChp : public Ww1Plc
{
public:
    Ww1PlcChp(Ww1Fib& rFibL) :
        Ww1Plc(rFibL, rFibL.GetFIB().fcPlcfbteChpxGet(),
         rFibL.GetFIB().cbPlcfbteChpxGet(), 2) {
        }
    friend ostream& operator <<(ostream&, Ww1PlcChp&);
};

//////////////////////////////////////////////////////////////// PlcPap
class Ww1PlcPap : public Ww1Plc
{
public:
    Ww1PlcPap(Ww1Fib& rFibL) :
        Ww1Plc(rFibL, rFibL.GetFIB().fcPlcfbtePapxGet(),
         rFibL.GetFIB().cbPlcfbtePapxGet(), 2) {
        }
    friend ostream& operator <<(ostream&, Ww1PlcPap&);
};

//////////////////////////////////////////////////////// PlcFootnoteRef
class Ww1PlcFootnoteRef : public Ww1Plc
{
public:
    Ww1PlcFootnoteRef(Ww1Fib& rFibL) :
        Ww1Plc(rFibL, rFibL.GetFIB().fcPlcffndRefGet(),
         rFibL.GetFIB().cbPlcffndRefGet(), 2) {
        }
    friend ostream& operator <<(ostream&, Ww1PlcFootnoteRef&);
};

//////////////////////////////////////////////////////// PlcFootnoteTxt
class Ww1PlcFootnoteTxt : public Ww1Plc
{
public:
    Ww1PlcFootnoteTxt(Ww1Fib& rFibL) :
        Ww1Plc(rFibL, rFibL.GetFIB().fcPlcffndTxtGet(),
         rFibL.GetFIB().cbPlcffndTxtGet(), 0) {
        }
    friend ostream& operator <<(ostream&, Ww1PlcFootnoteTxt&);
};

///////////////////////////////////////////////////////////// PlcFields
class Ww1PlcFields : public Ww1Plc
{
public:
    Ww1PlcFields(Ww1Fib& rFibL, sal_uLong start, sal_uInt16 nBytes)
        : Ww1Plc(rFibL, start, nBytes, 2)
    {}
    W1_FLD* GetData(sal_uInt16 nIndex)
        { return (W1_FLD*)Ww1Plc::GetData(nIndex); }
    sal_uLong Where(sal_uInt16 nIndex)  // absolut im file
        { return Ww1Plc::Where(nIndex) + rFib.GetFIB().fcMinGet(); }
    friend ostream& operator <<(ostream&, Ww1PlcFields&);
};

///////////////////////////////////////////////////////////// PlcBookmarks
class Ww1StringList
{
    sal_Char** pIdxA;
    sal_uInt16 nMax;
public:
    Ww1StringList( SvStream& rSt, sal_uLong nFc, sal_uInt16 nCb );
    ~Ww1StringList()
        {   if( pIdxA ) { delete pIdxA[0]; delete pIdxA; } }
    const String GetStr( sal_uInt16 nNum ) const;
    sal_uInt16 Count() const    { return nMax; }
    sal_Bool GetError() const   { return (nMax != 0) && !pIdxA; }
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
public:
    Ww1PlcBookmarkPos(Ww1Fib& _rFib, sal_uLong start, sal_uInt16 nBytes, sal_Bool bEnd)
        : Ww1Plc(_rFib, start, nBytes, (bEnd) ? 0 : 2)
    {}

    sal_uInt8* GetData(sal_uInt16 nIndex)   {   return Ww1Plc::GetData(nIndex); }
    // Position als CP
    sal_uLong WhereCP(sal_uInt16 nIndex)    { return Ww1Plc::Where(nIndex); }
    // absolut im file
    sal_uLong Where(sal_uInt16 nIndex)
    {
        return ( nIndex < Count() )
               ? Ww1Plc::Where(nIndex) + rFib.GetFIB().fcMinGet()
               : 0xffffffff;
    }
};

//////////////////////////////////////////////////////////////// PlcHdd
class Ww1PlcHdd : public Ww1Plc
{
public:
    Ww1PlcHdd(Ww1Fib& rFibL)
        : Ww1Plc(rFibL, rFibL.GetFIB().fcPlcfhddGet(),
                    rFibL.GetFIB().cbPlcfhddGet(), 0)
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
    sal_uInt8 aFkp[512];
    sal_uInt16 nItemSize;
    sal_Bool bOK;
    sal_uInt8* GetData(sal_uInt16);
public:
    Ww1Fkp(SvStream&, sal_uLong, sal_uInt16);
    friend ostream& operator <<(ostream&, Ww1Fkp&);
    sal_uInt16 Count() const            { return SVBT8ToByte(aFkp+511); }
    sal_uLong Where(sal_uInt16); // wie im entsprechenden fkp
};

//////////////////////////////////////////////////////////////// FkpPap
class Ww1FkpPap : public Ww1Fkp
{
public:
    Ww1FkpPap(SvStream& rStream, sal_uLong ulFilePos)
        : Ww1Fkp(rStream, ulFilePos, 1)
    {}
    friend ostream& operator <<(ostream&, Ww1FkpPap&);
    sal_Bool Fill(sal_uInt16,  sal_uInt8*&, sal_uInt16&);
};

//////////////////////////////////////////////////////////////// FkpChp
class Ww1FkpChp : public Ww1Fkp
{
#ifdef DUMP
    SvStream& rStream;
    SvStream& GetStream()   { return rStream; }
#endif
public:
    Ww1FkpChp(SvStream& rStream, sal_uLong ulFilePos)
        : Ww1Fkp(rStream, ulFilePos, 1)
#ifdef DUMP
        , rStream(rStream)
#endif
    {}

    friend ostream& operator <<(ostream&, Ww1FkpChp&);
    sal_Bool Fill(sal_uInt16, W1_CHP&);
};

////////////////////////////////////////////////////////////// SprmPapx
class Ww1SprmPapx : public Ww1Sprm
{
    W1_PAPX aPapx;
    sal_uInt8* Sprm(sal_uInt8* p, sal_uInt16 nSize);
    sal_uInt16 SprmSize(sal_uInt8* p, sal_uInt16 nSize);
public:
    Ww1SprmPapx(sal_uInt8* p, sal_uInt16 nSize);
    friend ostream& operator <<(ostream&, Ww1SprmPapx&);
    void Start(Ww1Shell&, Ww1Manager&);
    void Stop(Ww1Shell&, Ww1Manager&);
};

/////////////////////////////////////////////////////////////// SprmSep
class Ww1SprmSep : public Ww1Sprm
{
public:
    Ww1SprmSep(Ww1Fib& rFib, sal_uLong ulFilePos)
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
    sal_Bool bOK;

    String GetStr(sal_uInt16);

public:
    Ww1Assoc(Ww1Fib&);
    ~Ww1Assoc()             { delete pBuffer; }
    sal_Bool GetError() const   { return !bOK; }
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
    sal_uInt16 nextIhdd; // naechster textteil im HddText
    sal_uInt16 nFtnSep; // fusznoten trenner
    sal_uInt16 nFtnFollowSep; // folge fusznoten trenner
    sal_uInt16 nFtnNote; // folgefunsznotennotiz
    sal_uInt16 nEvenHeadL; // kopfzeilen grader seiten
    sal_uInt16 nOddHeadL; // kopfzeilen ungrader seiten
    sal_uInt16 nEvenFootL; // fuszzeilen grader seiten
    sal_uInt16 nOddFootL; // fuszzeilen ungerader seiten
    sal_uInt16 nFirstHeadL; // kopfzeilen der ersten seite
    sal_uInt16 nFirstFootL; // fuszzeilen der ersten seite
    enum HeaderFooterMode {
        None, FtnSep, FtnFollowSep, FtnNote, EvenHeadL, OddHeadL,
        EvenFootL, OddFootL, FirstHeadL, MaxHeaderFooterMode
    } eHeaderFooterMode;

public:
    Ww1HeaderFooter(Ww1Fib& rFibL, sal_uInt16 grpfIhdt)
        : Ww1PlcHdd(rFibL),
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
    void SetGrpfIhdt(sal_uInt16 grpfIhdt)
    {
        if (grpfIhdt & 0x0001) nEvenHeadL = nextIhdd++;
        if (grpfIhdt & 0x0002) nOddHeadL = nextIhdd++;
        if (grpfIhdt & 0x0004) nEvenFootL = nextIhdd++;
        if (grpfIhdt & 0x0008) nOddFootL = nextIhdd++;
        if (grpfIhdt & 0x0010) nFirstHeadL = nextIhdd++;
        if (grpfIhdt & 0x0020) nFirstFootL = nextIhdd++;
        OSL_ENSURE(nextIhdd<=Count(), "Ww1HeaderFooter");
    }
    sal_Bool operator++()
    {
        sal_Bool bRet = sal_True;
        eHeaderFooterMode = (HeaderFooterMode)((short)eHeaderFooterMode + 1);
        if( eHeaderFooterMode == MaxHeaderFooterMode)
        {
            eHeaderFooterMode = None;
            bRet = sal_False;
        }
        return bRet;
    }
    sal_Bool FillFtnSep(sal_uLong& begin, sal_uLong& end)
    {
        if (nFtnSep == 0xffff)
            return sal_False;
        Fill(nFtnSep, begin, end);
        return sal_True;
    }
    sal_Bool FillFtnFollowSep(sal_uLong& begin, sal_uLong& end)
    {
        if (nFtnFollowSep == 0xffff)
            return sal_False;
        Fill(nFtnFollowSep, begin, end);
        return sal_True;
    }
    sal_Bool FillFtnNote(sal_uLong& begin, sal_uLong& end)
    {
        if (nFtnNote == 0xffff)
            return sal_False;
        Fill(nFtnNote, begin, end);
        return sal_True;
    }
    sal_Bool FillEvenHeadL(sal_uLong& begin, sal_uLong& end)
    {
        if (nEvenHeadL == 0xffff)
            return sal_False;
        Fill(nEvenHeadL, begin, end);
        return sal_True;
    }
    sal_Bool FillOddHeadL(sal_uLong& begin, sal_uLong& end)
    {
        if (nOddHeadL == 0xffff)
            return sal_False;
        Fill(nOddHeadL, begin, end);
        return sal_True;
    }
    sal_Bool FillEvenFootL(sal_uLong& begin, sal_uLong& end)
    {
        if (nEvenFootL == 0xffff)
            return sal_False;
        Fill(nEvenFootL, begin, end);
        return sal_True;
    }
    sal_Bool FillOddFootL(sal_uLong& begin, sal_uLong& end)
    {
        if (nOddFootL == 0xffff)
            return sal_False;
        Fill(nOddFootL, begin, end);
        return sal_True;
    }
    sal_Bool FillFirstHeadL(sal_uLong& begin, sal_uLong& end)
    {
        if (nFirstHeadL == 0xffff)
            return sal_False;
        Fill(nFirstHeadL, begin, end);
        return sal_True;
    }
    sal_Bool FillFirstFootL(sal_uLong& begin, sal_uLong& end)
    {
        if (nFirstFootL == 0xffff)
            return sal_False;
        Fill(nFirstFootL, begin, end);
        return sal_True;
    }
    void Start(Ww1Shell&, Ww1Manager&);
    void Stop(Ww1Shell&, Ww1Manager&, sal_Unicode&);
};

//////////////////////////////////////////////////////////////// Fields
class Ww1Fields : public Ww1PlcFields
{
    sal_uInt16 nPlcIndex;
    String sErgebnis; // das von word errechnete ergebniss
    SwField* pField;
    sal_uLong Where(sal_uInt16 nIndex)  // innerhalb des textes
        { return Ww1PlcFields::Where(nIndex) - rFib.GetFIB().fcMinGet(); }

public:
    Ww1Fields(Ww1Fib& rFibL, sal_uLong ulFilePos, sal_uInt16 nBytes)
        : Ww1PlcFields(rFibL, ulFilePos, nBytes), nPlcIndex(0), pField(0)
    {}
    // innerhalb des textes
    sal_uLong Where()       { return Where(nPlcIndex); }
    void operator++()
    {
        OSL_ENSURE(nPlcIndex+1 <= Count(), "Ww1Fields");
        nPlcIndex++;
    }
    void Seek(sal_uLong ulNew)      { Ww1PlcFields::Seek(ulNew, nPlcIndex); }
    W1_FLD* GetData()
    {
        OSL_ENSURE(nPlcIndex < Count(), "Ww1Fields");
        return Ww1PlcFields::GetData(nPlcIndex);
    }
    sal_uLong GetLength();
    friend ostream& operator <<(ostream&, Ww1Manager&);
    void Start(Ww1Shell&, Ww1Manager&);
    void Stop(Ww1Shell&, Ww1Manager&, sal_Unicode&);
    void Out(Ww1Shell&, Ww1Manager&, sal_uInt16=0);
};

class Ww1TextFields : public Ww1Fields
{
public:
    Ww1TextFields(Ww1Fib& rFibL)
        : Ww1Fields(rFibL, rFibL.GetFIB().fcPlcffldMomGet(),
                    rFibL.GetFIB().cbPlcffldMomGet())
    {}
};

class Ww1FootnoteFields : public Ww1Fields
{
public:
    Ww1FootnoteFields(Ww1Fib& rFibL)
        : Ww1Fields(rFibL, rFibL.GetFIB().fcPlcffldFtnGet(),
                    rFibL.GetFIB().cbPlcffldFtnGet())
    {}
};

class Ww1HeaderFooterFields : public Ww1Fields
{
public:
    Ww1HeaderFooterFields(Ww1Fib& rFibL)
        : Ww1Fields(rFibL, rFibL.GetFIB().fcPlcffldHdrGet(),
                    rFibL.GetFIB().cbPlcffldHdrGet())
    {}
};

class Ww1MacroFields : public Ww1Fields
{
public:
    Ww1MacroFields(Ww1Fib& rFibL)
        : Ww1Fields(rFibL, rFibL.GetFIB().fcPlcffldMcrGet(),
                    rFibL.GetFIB().cbPlcffldMcrGet())
    {}
};

//////////////////////////////////////////////////////////////// Bookmarks
class Ww1Bookmarks
{
    Ww1PlcBookmarkTxt aNames;
    Ww1PlcBookmarkPos* pPos[2];
    Ww1Fib& rFib;

    sal_uInt16 nPlcIdx[2];
    sal_uInt16 nIsEnd;
    sal_Bool bOK;
public:
    Ww1Bookmarks(Ww1Fib& rFib);
    ~Ww1Bookmarks()
    {
            delete pPos[1];
            delete pPos[0];
    }
    sal_uLong Where() const     { return pPos[nIsEnd]->WhereCP(nPlcIdx[nIsEnd]); }
    void operator++();
    sal_Bool GetError() const   { return !bOK; }
    long GetHandle() const;
    sal_Bool GetIsEnd() const   { return ( nIsEnd ) ? sal_True : sal_False; }
    const String GetName() const;
    long Len() const;
    friend ostream& operator <<(ostream&, Ww1Bookmarks&);
    void Start(Ww1Shell&, Ww1Manager&);
    void Stop(Ww1Shell&, Ww1Manager&, sal_Unicode&);
    void Out(Ww1Shell&, Ww1Manager&, sal_uInt16=0);
};

///////////////////////////////////////////////////////////// Footnotes
class Ww1Footnotes : public Ww1PlcFootnoteRef
{
    sal_uInt16 nPlcIndex;
    Ww1PlcFootnoteTxt aText;
    sal_Bool bStarted;
public:
    Ww1Footnotes(Ww1Fib& rFibL)
        : Ww1PlcFootnoteRef(rFibL), nPlcIndex(0), aText(rFibL), bStarted(sal_False)
    {}
    // innerhalb des textes
    sal_uLong Where()
    {
        sal_uLong ulRet = 0xffffffff;
        if (Count())
            ulRet = Ww1PlcFootnoteRef::Where(nPlcIndex);
        return ulRet;
    }
    void operator++()
    {
        OSL_ENSURE(nPlcIndex+1 <= Count(), "Ww1Footnotes");
        nPlcIndex++;
    }
    void Start(Ww1Shell&, Ww1Manager&);
    void Stop(Ww1Shell&, Ww1Manager&, sal_Unicode&);
};

/////////////////////////////////////////////////////////////////// Sep
class Ww1Sep : public Ww1PlcSep
{
    Ww1HeaderFooter aHdd;
    sal_uInt16 nPlcIndex;
public:
    Ww1Sep(Ww1Fib& rFibL, sal_uInt16 grpfIhdt)
    : Ww1PlcSep(rFibL), aHdd(rFibL, grpfIhdt), nPlcIndex(0) {}

    Ww1HeaderFooter& GetHdd()   { return aHdd; }
    void operator++()        { nPlcIndex++; }
    sal_uInt8* GetData()            { return Ww1PlcSep::GetData(nPlcIndex); }
    // innerhalb des textes
    sal_uLong Where()               { return Ww1PlcSep::Where(nPlcIndex); }
    void SetGrpfIhdt(sal_uInt8 grpfIhdt)
    {
        GetHdd().SetGrpfIhdt(grpfIhdt);
    }
    void Start(Ww1Shell&, Ww1Manager&);
    void Stop(Ww1Shell& rOut, Ww1Manager& rMan, sal_Unicode& c)
        { aHdd.Stop(rOut, rMan, c); }
};

/////////////////////////////////////////////////////////////////// Pap
class Ww1Pap : public Ww1PlcPap
{
    sal_uInt16 nPlcIndex;
    sal_uInt16 nPushedPlcIndex;
    sal_uInt16 nFkpIndex;
    sal_uInt16 nPushedFkpIndex;
    sal_uLong ulOffset;
    Ww1FkpPap* pPap;

    sal_Bool FindSprm(sal_uInt16 nId, sal_uInt8* pStart, sal_uInt8* pEnd);
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
    sal_Bool HasId0(sal_uInt16 nId);

public:
    Ww1Pap(Ww1Fib& rFib);
    ~Ww1Pap()   { delete pPap; }
    sal_uLong Where( sal_Bool bSetIndex = sal_True ); // innerhalb des textes
    void operator++();
    sal_Bool FillStart(sal_uInt8*& pB, sal_uInt16& nSize)
    {
        UpdateIdx();
        return pPap->Fill(nFkpIndex, pB, nSize);
    }
    sal_Bool FillStop(sal_uInt8*& pB, sal_uInt16& nSize)
    {
        return nFkpIndex ? pPap->Fill(nFkpIndex-1, pB, nSize) : sal_False;
    }
    void Start(Ww1Shell&, Ww1Manager&);
    void Stop(Ww1Shell&, Ww1Manager&, sal_Unicode&);
    void Seek(sal_uLong);
    void Push(sal_uLong ulOffsetTmp = 0)
    {
        OSL_ENSURE(!Pushed(), "Ww1Pap");
        nPushedPlcIndex = nPlcIndex;
        nPushedFkpIndex = nFkpIndex;
        Seek(ulOffsetTmp);
        ulOffset = ulOffsetTmp;
        delete pPap;
        pPap = NULL;
    }
    sal_Bool Pushed()
    {
        return nPushedPlcIndex != 0xffff;
    }
    void Pop()
    {
        OSL_ENSURE(Pushed(), "Ww1Pap");
        ulOffset = 0;
        nPlcIndex = nPushedPlcIndex;
        nFkpIndex = nPushedFkpIndex;
        nPushedPlcIndex = 0xffff;
        nPushedFkpIndex = 0xffff;
        delete pPap;
        pPap = NULL;
        Where( sal_False );
    }
    sal_Bool HasId(sal_uInt16 nId);
};

/////////////////////////////////////////////////////////////////// Chp
class Ww1Chp : public Ww1PlcChp
{
    sal_uInt16 nPlcIndex;
    sal_uInt16 nPushedPlcIndex;
    sal_uInt16 nFkpIndex;
    sal_uInt16 nPushedFkpIndex;
    sal_uLong ulOffset;
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
    sal_uLong Where( sal_Bool bSetIndex = sal_True ); // innerhalb des textes
    void operator++();
    sal_Bool FillStart(W1_CHP& rChp)
    {
        UpdateIdx();
        return pChp->Fill(nFkpIndex, rChp);
    }
    sal_Bool FillStop(W1_CHP& rChp)
    { return nFkpIndex ? pChp->Fill(nFkpIndex-1, rChp) : sal_False;  }
    void Start(Ww1Shell&, Ww1Manager&);
    void Stop(Ww1Shell&, Ww1Manager&, sal_Unicode&);
    void Seek(sal_uLong);
    void Push(sal_uLong ulOffsetTmp = 0)
    {
        OSL_ENSURE(!Pushed(), "Ww1Chp");
        nPushedPlcIndex = nPlcIndex;
        nPushedFkpIndex = nFkpIndex;
        Seek(ulOffsetTmp);
        ulOffset = ulOffsetTmp;
        delete pChp;
        pChp = NULL;
    }
    sal_Bool Pushed()               { return nPushedPlcIndex != 0xffff; }
    void Pop()
    {
        OSL_ENSURE(Pushed(), "Ww1Chp");
        ulOffset = 0;
        nPlcIndex = nPushedPlcIndex;
        nFkpIndex = nPushedFkpIndex;
        nPushedPlcIndex = 0xffff;
        nPushedFkpIndex = 0xffff;
        delete pChp;
        pChp = NULL;
        Where( sal_False );
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
    sal_Bool bOK;
    sal_Bool bInTtp;
    sal_Bool bInStyle;
    sal_Bool bStopAll;
    Ww1Fib aFib;
    Ww1Dop aDop;
    Ww1Fonts aFonts;
// ab jetzt alles paarig, fuer 'pushed':
    Ww1DocText aDoc;
    Ww1PlainText* pDoc;
    sal_uLong ulDocSeek;
    sal_uLong* pSeek;
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
    Ww1Manager(SvStream& rStrm, sal_uLong nFieldFlgs);
    sal_Bool GetError() const       { return !bOK; }

// Fuer Tabellen
    void SetInTtp(sal_Bool bSet = sal_True)     { bInTtp = bSet; }
    sal_Bool IsInTtp() const                { return bInTtp; }
    void SetInStyle(sal_Bool bSet = sal_True)   { bInStyle = bSet; }
    sal_Bool IsInStyle() const              { return bInStyle; }
    void SetStopAll(sal_Bool bSet = sal_True)   { bStopAll = bSet; }
    sal_Bool IsStopAll() const              { return bStopAll; }
    sal_Bool HasInTable();
    sal_Bool HasTtp();
    sal_Bool LastHasTtp();

// Fuer Flys
    sal_Bool HasPPc();
    sal_Bool HasPDxaAbs();

    Ww1Fib& GetFib()                    { return aFib; }
    Ww1PlainText& GetText()             { return *pDoc; }
    Ww1Dop& GetDop()                    { return aDop; }
    Ww1Sep& GetSep()                    { return aSep; }
    // innerhalb des textes
    sal_uLong Where()                       { return pDoc->Where(); }
    void Fill( sal_Unicode& rChr )      { pDoc->Out( rChr ); }
    sal_uInt8 Fill( String& rStr, sal_uLong ulLen)
    {
        ulLen += pDoc->Where();
        return sal::static_int_cast< sal_uInt8 >(pDoc->Out(rStr, ulLen));
    }
    SvxFontItem GetFont(sal_uInt16 nFCode);
    friend Ww1Shell& operator <<(Ww1Shell&, Ww1Manager&);
    friend ostream& operator <<(ostream&, Ww1Manager&);
    sal_Bool Pushed()                       { return pDoc != &aDoc; }
    void Pop();
    void Push0(Ww1PlainText* pDoc, sal_uLong, Ww1Fields* = 0);
    void Push1(Ww1PlainText* pDoc, sal_uLong ulSeek, sal_uLong ulSeek2 = 0,
               Ww1Fields* = 0);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
