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
#include <rtl/ustring.hxx>

// local
#include <w1struct.hxx>

#ifdef DUMP
#include <fstream.h>
#endif

#include <ostream>

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

// If possible, methods with similar functionality were given similar
// names in this module. The names were taken from ww filter, if
// possible.
// Where() returns an element's position. This can be either a seek
// position in the stream or a relative offset inside the text, since
// these are mixed up in Word. The methods' comments indicate which of
// these options they apply to.
// Count() returns the number of elements. Be careful with n/n-1
// fields (Word likes to store structures in double arrays, with the
// first one containing n elements, and the second one n-1.
// Fill() fills passed references with data from Word structures.
// GetData() returns pointers to the data range
// Start(), Stop(), Out(), op<< see module w1filter
// Dump() see module w1dump

// file information block: root of the evil: it's at the start of the
// file (seek(0)) and contains all positions of the structures of the
// file.
class Ww1Fib
{
    W1_FIB aFib;
    sal_Bool bOK;
    SvStream& rStream;
public:
    Ww1Fib(SvStream&);
    friend std::ostream& operator <<(std::ostream&, Ww1Fib&);
    W1_FIB& GetFIB()        { return aFib; }
    sal_Bool GetError()         { return !bOK; }
    SvStream& GetStream()   { return rStream; }
};

// document property: properties of the entire document
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
    friend std::ostream& operator <<(std::ostream&, Ww1Dop&);
    void Out(Ww1Shell&);
};

// ww-files can contain several blocks of text (main-text,
// footnotes etc). PlainText unifies the overlaps
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
    // within the text
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
    sal_Unicode Out( OUString&, sal_uLong=0xffffffff);
    sal_Unicode Out( sal_Unicode& );
    friend std::ostream& operator <<(std::ostream&, Ww1PlainText&);
    OUString& Fill( OUString&, sal_uLong=0, sal_uLong=0xffffffff );
    sal_Unicode operator []( sal_uLong );
    OUString GetText( sal_uLong ulOffset, sal_uLong nLen ) const;

    enum Consts { MinChar = 32 };
    static bool IsChar( sal_Unicode c )     { return c >= MinChar; }
};

class Ww1DocText : public Ww1PlainText
{
public:
    Ww1DocText(Ww1Fib& rFibL) :
        Ww1PlainText(rFibL, rFibL.GetFIB().fcMinGet(),
         rFibL.GetFIB().ccpTextGet()) {
        }
};

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

// a single style or template
class Ww1Style
{
    OUString aName;
    W1_CHP aChpx;
    Ww1SprmPapx* pPapx;
    Ww1StyleSheet* pParent;
    sal_uInt8 stcBase;
    sal_uInt8 stcNext;
    sal_Bool bUsed;
public:
    Ww1Style();
    ~Ww1Style();
    bool IsUsed() const                 { return bUsed; }
    void SetDefaults(sal_uInt8);
    void SetParent(Ww1StyleSheet* newParent)    { pParent = newParent; }
    void SetName(const OUString& rName)   { bUsed = sal_True; aName = rName; }
    const OUString& GetName() const       { return aName; }
    Ww1Style& GetBase();
    sal_uInt16 GetnBase() const             { return stcBase; }
    sal_uInt16 GetnNext() const             { return stcNext; }
    sal_uInt16 ReadName(sal_uInt8*&, sal_uInt16&, sal_uInt16 stc);
    sal_uInt16 ReadChpx(sal_uInt8*&, sal_uInt16&);
    sal_uInt16 ReadPapx(sal_uInt8*&, sal_uInt16&);
    sal_uInt16 ReadEstcp(sal_uInt8*&, sal_uInt16&);
    friend std::ostream& operator <<(std::ostream&, Ww1Style&);
    void Out(Ww1Shell&, Ww1Manager&);
};

// collection of all templates (max. 256)
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
    friend std::ostream& operator <<(std::ostream&, Ww1StyleSheet&);
    void Out(Ww1Shell&, Ww1Manager&);
    friend class Ww1Style;
    sal_Bool GetError() {
        return !bOK; }
};

// ww only knows font numbers during formatting. Independently, there
// is an array of fonts, so that the number can be converted to a
// specific font.
class Ww1Fonts
{
protected:
    W1_FFN** pFontA; // Array of Pointers to Font Description
    Ww1Fib& rFib;
    sal_uLong nFieldFlags;
    sal_uInt16 nMax; // Array size
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
    friend std::ostream& operator <<(std::ostream&, Ww1Fonts&);
    sal_Bool GetError() {
        return !bOK; }
    SvxFontItem GetFont(sal_uInt16);
};

// SingleSprm
// These classes replace aSprmTab etc. from ww6 filter. The function
// pointers are virtual methods. There exist derived classes for
// specific types (byte, word, var-sized etc.). They have methods for
// determining their sizes, for dumping and outputting the Sprms to
// the shell.
// The classes get created with new (in InitTab()) and added to the
// table according to their code. For activating them the respective
// method has to be called on an object in the table.
// Take note: SingleSprms only describe Sprms, they don't carry their
// content, which must be passed to the individual methods such as
// Size, Dump and Start/Stop.
class Ww1SingleSprm
{
public:
#ifdef DUMP
    // The vtab only contains the virtual methods; we hide them
    // depending on what we compile for (dumper or filter). This saves
    // space. Furthermore, these method's implementations live in
    // different modules which only get compiled and linked when
    // required by the respective project. The name string is only
    // required by the dumper -- let's not include it in the filter.
    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&);
    void Stop(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&);
    virtual std::ostream& Dump(std::ostream&, sal_uInt8*, sal_uInt16);
    const sal_Char* sName;
#else
    virtual void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&);
    virtual void Stop(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&);
    std::ostream& Dump(std::ostream&, sal_uInt8*, sal_uInt16);
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
    sal_uInt16 Size(sal_uInt8*) SAL_OVERRIDE;
    Ww1SingleSprmByteSized(sal_uInt16 nBytes, sal_Char* sName = 0) :
        Ww1SingleSprm(nBytes, sName) {
        }
};

class Ww1SingleSprmWordSized : public Ww1SingleSprm {
public:
    sal_uInt16 Size(sal_uInt8*) SAL_OVERRIDE;
    Ww1SingleSprmWordSized(sal_uInt16 nBytes, sal_Char* sName = 0) :
        Ww1SingleSprm(nBytes, sName) {
        }
};

class Ww1SingleSprmByte : public Ww1SingleSprm {
public:
    std::ostream& Dump(std::ostream&, sal_uInt8*, sal_uInt16);
    Ww1SingleSprmByte(sal_Char* sName = 0) :
        Ww1SingleSprm(1, sName) {
        }
};

class Ww1SingleSprmBool : public Ww1SingleSprmByte {
public:
    std::ostream& Dump(std::ostream&, sal_uInt8*, sal_uInt16);
    Ww1SingleSprmBool(sal_Char* sName = 0) :
        Ww1SingleSprmByte(sName) {
        }
};

class Ww1SingleSprm4State : public Ww1SingleSprmByte {
public:
    std::ostream& Dump(std::ostream&, sal_uInt8*, sal_uInt16);
    Ww1SingleSprm4State(sal_Char* sName = 0) :
        Ww1SingleSprmByte(sName) {
        }
};

class Ww1SingleSprmWord : public Ww1SingleSprm {
public:
    std::ostream& Dump(std::ostream&, sal_uInt8*, sal_uInt16);
    Ww1SingleSprmWord(sal_Char* sName = 0)
    : Ww1SingleSprm(2, sName) {}
};

class Ww1SingleSprmLong : public Ww1SingleSprm {
public:
    std::ostream& Dump(std::ostream&, sal_uInt8*, sal_uInt16);
    Ww1SingleSprmLong(sal_Char* sName = 0) :
        Ww1SingleSprm(4, sName) {
        }
};

class Ww1SingleSprmTab : public Ww1SingleSprm {
public:
    std::ostream& Dump(std::ostream&, sal_uInt8*, sal_uInt16);
    sal_uInt16 Size(sal_uInt8*) SAL_OVERRIDE;
    Ww1SingleSprmTab(sal_uInt16 nBytes, sal_Char* sName = 0) :
        Ww1SingleSprm(nBytes, sName) {
        }
};

class Ww1SingleSprmPJc : public Ww1SingleSprmByte {
public:
    Ww1SingleSprmPJc(sal_Char* sName) :
        Ww1SingleSprmByte(sName) {
        }
    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&) SAL_OVERRIDE;
    void Stop(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&) SAL_OVERRIDE;
};

class Ww1SingleSprmPDxa : public Ww1SingleSprmWord {
public:
    Ww1SingleSprmPDxa(sal_Char* sName) :
        Ww1SingleSprmWord(sName) {
        }
    void Stop(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&) SAL_OVERRIDE;
};

class Ww1SingleSprmPDxaRight : public Ww1SingleSprmPDxa {
public:
    Ww1SingleSprmPDxaRight(sal_Char* sName) :
        Ww1SingleSprmPDxa(sName) {
        }
    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&) SAL_OVERRIDE;
};

class Ww1SingleSprmPDxaLeft : public Ww1SingleSprmPDxa {
public:
    Ww1SingleSprmPDxaLeft(sal_Char* sName) :
        Ww1SingleSprmPDxa(sName) {
        }
    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&) SAL_OVERRIDE;
};

class Ww1SingleSprmPDxaLeft1 : public Ww1SingleSprmPDxa {
public:
    Ww1SingleSprmPDxaLeft1(sal_Char* sName) :
        Ww1SingleSprmPDxa(sName) {
        }
    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&) SAL_OVERRIDE;
};

class Ww1SingleSprmPFKeep : public Ww1SingleSprmBool {
public:
    Ww1SingleSprmPFKeep(sal_Char* sName) :
        Ww1SingleSprmBool(sName) {
        }
    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&) SAL_OVERRIDE;
    void Stop(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&) SAL_OVERRIDE;
};

class Ww1SingleSprmPFKeepFollow : public Ww1SingleSprmBool {
public:
    Ww1SingleSprmPFKeepFollow(sal_Char* sName) :
        Ww1SingleSprmBool(sName) {
        }
    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&) SAL_OVERRIDE;
    void Stop(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&) SAL_OVERRIDE;
};

class Ww1SingleSprmPPageBreakBefore : public Ww1SingleSprmBool {
public:
    Ww1SingleSprmPPageBreakBefore(sal_Char* sName) :
        Ww1SingleSprmBool(sName) {
        }
    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&) SAL_OVERRIDE;
    void Stop(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&) SAL_OVERRIDE;
};

class Ww1SingleSprmPBrc : public Ww1SingleSprmWord {
protected:
    // specific start routines, supplied with different BoxItems
    // depending on sprm
    void Start(Ww1Shell&, sal_uInt8, W1_BRC10*, sal_uInt16, Ww1Manager&, SvxBoxItem&);
    void Start(Ww1Shell&, sal_uInt8, W1_BRC*, sal_uInt16, Ww1Manager&, SvxBoxItem&);

    using Ww1SingleSprm::Start;

public:
    Ww1SingleSprmPBrc(sal_Char* sName) :
        Ww1SingleSprmWord(sName) {
        }
    void Stop(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&) SAL_OVERRIDE;
    // SetBorder() is needed for tables, too. That's why it's public
    static editeng::SvxBorderLine* SetBorder(editeng::SvxBorderLine*, W1_BRC10*);
};

#define BRC_TOP ((sal_uInt16)0)
#define BRC_LEFT ((sal_uInt16)1)
#define BRC_BOTTOM ((sal_uInt16)2)
#define BRC_RIGHT ((sal_uInt16)3)
#define BRC_ANZ ((sal_uInt16)BRC_RIGHT-BRC_TOP+1)

// The BRC structure for 1.0 versions of Word differ from the ones in
// later versions. Luckily, they will be queried by other sprms.
// SH: From now on, all 4 borders handled by a single class.
class Ww1SingleSprmPBrc10 : public Ww1SingleSprmPBrc
{
    sal_uInt16 nLine;   // BRC_TOP, BRC_LEFT, ...

    using Ww1SingleSprmPBrc::Start;

public:
    Ww1SingleSprmPBrc10(sal_uInt16 nL, sal_Char* sName)
    : Ww1SingleSprmPBrc(sName), nLine(nL) {}

    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&) SAL_OVERRIDE;
};

class Ww1SingleSprmParaSpace : public Ww1SingleSprmWord {
public:
    Ww1SingleSprmParaSpace(sal_Char* sName)
    : Ww1SingleSprmWord(sName) {}
    void Stop(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&) SAL_OVERRIDE;
};

class Ww1SingleSprmPDyaBefore : public Ww1SingleSprmParaSpace {
public:
    Ww1SingleSprmPDyaBefore(sal_Char* sName)
    : Ww1SingleSprmParaSpace(sName) {}
    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&) SAL_OVERRIDE;
};

class Ww1SingleSprmPDyaAfter : public Ww1SingleSprmParaSpace {
public:
    Ww1SingleSprmPDyaAfter(sal_Char* sName) :
        Ww1SingleSprmParaSpace(sName) {
        }
    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&) SAL_OVERRIDE;
};

class Ww1SingleSprmPDyaLine : public Ww1SingleSprmWord {
public:
    Ww1SingleSprmPDyaLine(sal_Char* sName) :
        Ww1SingleSprmWord(sName) {
        }
    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&) SAL_OVERRIDE;
    void Stop(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&) SAL_OVERRIDE;
};

class Ww1SingleSprmPChgTabsPapx : public Ww1SingleSprmByteSized {
public:
    Ww1SingleSprmPChgTabsPapx(sal_Char* sName) :
        Ww1SingleSprmByteSized(0, sName) {
        }
    // Size() is not yet activated!
    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&) SAL_OVERRIDE;
    void Stop(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&) SAL_OVERRIDE;
};

class Ww1SingleSprmSGprfIhdt : public Ww1SingleSprmByte {
public:
    Ww1SingleSprmSGprfIhdt(sal_Char* sName) :
        Ww1SingleSprmByte(sName) {
        }
    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&) SAL_OVERRIDE;
};

class Ww1SingleSprmSColumns : public Ww1SingleSprmWord {
public:
    Ww1SingleSprmSColumns(sal_Char* sName) :
        Ww1SingleSprmWord(sName) {
        }
    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&) SAL_OVERRIDE;
};

class Ww1SingleSprmPFInTable : public Ww1SingleSprmBool {
public:
    Ww1SingleSprmPFInTable(sal_Char* sName) :
        Ww1SingleSprmBool(sName) {
        }
    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&) SAL_OVERRIDE;
    void Stop(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&) SAL_OVERRIDE;
};

class Ww1SingleSprmPTtp : public Ww1SingleSprmBool {
public:
    Ww1SingleSprmPTtp(sal_Char* sName) :
        Ww1SingleSprmBool(sName) {
        }
    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&) SAL_OVERRIDE;
    void Stop(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&) SAL_OVERRIDE;
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
    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&) SAL_OVERRIDE;
};

class Ww1SingleSprmTDefTable10 : public Ww1SingleSprmWordSized {
public:
    Ww1SingleSprmTDefTable10(sal_Char* sName) :
        Ww1SingleSprmWordSized(0, sName) {
        }
    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&) SAL_OVERRIDE;
};

class Ww1SingleSprmTDyaRowHeight : public Ww1SingleSprmWord {
public:
    Ww1SingleSprmTDyaRowHeight(sal_Char* sName) :
        Ww1SingleSprmWord(sName) {
        }
    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&) SAL_OVERRIDE;
};

// Class definitions for table fastsave attributes
// Because we don't support fastsave, we don't need it

// Class definitions for Apos ( == Flys )

class Ww1SingleSprmPpc : public Ww1SingleSprmByte {
public:
    Ww1SingleSprmPpc(sal_Char* sName) :
        Ww1SingleSprmByte(sName) {
        }
    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&) SAL_OVERRIDE;
};

class Ww1SingleSprmPDxaAbs : public Ww1SingleSprmWord {
public:
    Ww1SingleSprmPDxaAbs(sal_Char* sName) :
        Ww1SingleSprmWord(sName) {
        }
    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&) SAL_OVERRIDE;
};

class Ww1SingleSprmPDyaAbs : public Ww1SingleSprmWord {
public:
    Ww1SingleSprmPDyaAbs(sal_Char* sName) :
        Ww1SingleSprmWord(sName) {
        }
    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&) SAL_OVERRIDE;
};

class Ww1SingleSprmPDxaWidth : public Ww1SingleSprmWord {
public:
    Ww1SingleSprmPDxaWidth(sal_Char* sName) :
        Ww1SingleSprmWord(sName) {
        }
    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&) SAL_OVERRIDE;
};

class Ww1SingleSprmPFromText : public Ww1SingleSprmWord {
public:
    Ww1SingleSprmPFromText(sal_Char* sName) :
        Ww1SingleSprmWord(sName) {
        }
    void Start(Ww1Shell&, sal_uInt8, sal_uInt8*, sal_uInt16, Ww1Manager&) SAL_OVERRIDE;
};

// The data type Sprm actually appearing in the file
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
    // without token, with length byte/word
    sal_uInt16 GetSize(sal_uInt8 nId, sal_uInt8* pSprm);
    // with token and length byte
    sal_uInt16 GetSizeBrutto(sal_uInt8* pSprm) {
        sal_uInt8 nId = *pSprm++;
        return GetSize(nId, pSprm) + 1; }
    // returns for the n-th element id, size & pointer to data:
    // sal_Bool Fill(sal_uInt16, sal_uInt8&, sal_uInt16&, sal_uInt8*&);
public:
    // SH: I need it to be public
    // returns for the n-th element id, size & pointer to data:
    sal_Bool Fill(sal_uInt16, sal_uInt8&, sal_uInt16&, sal_uInt8*&);

    Ww1Sprm(sal_uInt8*, sal_uInt16);
    Ww1Sprm(SvStream&, sal_uLong);
    ~Ww1Sprm();
    friend std::ostream& operator <<(std::ostream&, Ww1Sprm&);
    void Start(Ww1Shell&, Ww1Manager&);
    void Start(Ww1Shell&, Ww1Manager&, sal_uInt16);
    void Stop(Ww1Shell&, Ww1Manager&);
    bool IsUsed() {
        return nCountBytes != 255; }
    sal_uInt16 Count() {
        return count; }
    sal_Bool GetError() {
        return !bOK; }
    static void DeinitTab();
};

// the wrapper around the type PIC, a structure, which stands at the beginning
// of a picture filename or an embedded picture
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
    friend std::ostream& operator <<(std::ostream&, Ww1Picture&);
    void Out(Ww1Shell&, Ww1Manager&);
    void WriteBmp(SvStream&);
};

// One of the important array structures in ww files. They contain n+1
// file positions and n attributes applying in between them.
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
    friend std::ostream& operator <<(std::ostream&, Ww1Plc&);
    sal_uLong Where(sal_uInt16); // like in each plc
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

class Ww1PlcGlossary : public Ww1Plc
{
public:
    Ww1PlcGlossary(Ww1Fib& rFibL) :
        Ww1Plc(rFibL, rFibL.GetFIB().fcPlcfglsyGet(),
         rFibL.GetFIB().cbPlcfglsyGet(), 0) {
        }
};

class Ww1PlcAnnotationRef : public Ww1Plc
{
public:
    Ww1PlcAnnotationRef(Ww1Fib& rFibL) :
        Ww1Plc(rFibL, rFibL.GetFIB().fcPlcfandRefGet(),
         rFibL.GetFIB().cbPlcfandRefGet(), 0) {
        }
};

class Ww1PlcAnnotationTxt : public Ww1Plc
{
public:
    Ww1PlcAnnotationTxt(Ww1Fib& rFibL) :
        Ww1Plc(rFibL, rFibL.GetFIB().fcPlcfandTxtGet(),
         rFibL.GetFIB().cbPlcfandTxtGet(), 0) {
        }
};

// PlcAnnotation
class Ww1Annotation {
    Ww1PlcAnnotationRef aRef;
    Ww1PlcAnnotationTxt aTxt;
public:
    Ww1Annotation(Ww1Fib& rFib) :
        aRef(rFib),
        aTxt(rFib) {
        }
    friend std::ostream& operator <<(std::ostream&, Ww1Annotation&);
};

class Ww1PlcSep : public Ww1Plc
{
public:
    Ww1PlcSep(Ww1Fib& rFibL):
        Ww1Plc(rFibL, rFibL.GetFIB().fcPlcfsedGet(),
         rFibL.GetFIB().cbPlcfsedGet(), 6) {
        }
    friend std::ostream& operator <<(std::ostream&, Ww1PlcSep&);
};

class Ww1PlcChp : public Ww1Plc
{
public:
    Ww1PlcChp(Ww1Fib& rFibL) :
        Ww1Plc(rFibL, rFibL.GetFIB().fcPlcfbteChpxGet(),
         rFibL.GetFIB().cbPlcfbteChpxGet(), 2) {
        }
    friend std::ostream& operator <<(std::ostream&, Ww1PlcChp&);
};

class Ww1PlcPap : public Ww1Plc
{
public:
    Ww1PlcPap(Ww1Fib& rFibL) :
        Ww1Plc(rFibL, rFibL.GetFIB().fcPlcfbtePapxGet(),
         rFibL.GetFIB().cbPlcfbtePapxGet(), 2) {
        }
    friend std::ostream& operator <<(std::ostream&, Ww1PlcPap&);
};

class Ww1PlcFootnoteRef : public Ww1Plc
{
public:
    Ww1PlcFootnoteRef(Ww1Fib& rFibL) :
        Ww1Plc(rFibL, rFibL.GetFIB().fcPlcffndRefGet(),
         rFibL.GetFIB().cbPlcffndRefGet(), 2) {
        }
    friend std::ostream& operator <<(std::ostream&, Ww1PlcFootnoteRef&);
};

class Ww1PlcFootnoteTxt : public Ww1Plc
{
public:
    Ww1PlcFootnoteTxt(Ww1Fib& rFibL) :
        Ww1Plc(rFibL, rFibL.GetFIB().fcPlcffndTxtGet(),
         rFibL.GetFIB().cbPlcffndTxtGet(), 0) {
        }
    friend std::ostream& operator <<(std::ostream&, Ww1PlcFootnoteTxt&);
};

class Ww1PlcFields : public Ww1Plc
{
public:
    Ww1PlcFields(Ww1Fib& rFibL, sal_uLong start, sal_uInt16 nBytes)
        : Ww1Plc(rFibL, start, nBytes, 2)
    {}
    W1_FLD* GetData(sal_uInt16 nIndex)
        { return (W1_FLD*)Ww1Plc::GetData(nIndex); }
    sal_uLong Where(sal_uInt16 nIndex)  // absolute within the file
        { return Ww1Plc::Where(nIndex) + rFib.GetFIB().fcMinGet(); }
    friend std::ostream& operator <<(std::ostream&, Ww1PlcFields&);
};

// PlcBookmarks
class Ww1StringList
{
    sal_Char** pIdxA;
    sal_uInt16 nMax;
public:
    Ww1StringList( SvStream& rSt, sal_uLong nFc, sal_uInt16 nCb );
    ~Ww1StringList()
        {   if( pIdxA ) { delete pIdxA[0]; delete pIdxA; } }
    const OUString GetStr( sal_uInt16 nNum ) const;
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
    // Position as CP
    sal_uLong WhereCP(sal_uInt16 nIndex)    { return Ww1Plc::Where(nIndex); }
    // absolute within the file
    sal_uLong Where(sal_uInt16 nIndex)
    {
        return ( nIndex < Count() )
               ? Ww1Plc::Where(nIndex) + rFib.GetFIB().fcMinGet()
               : 0xffffffff;
    }
};

class Ww1PlcHdd : public Ww1Plc
{
public:
    Ww1PlcHdd(Ww1Fib& rFibL)
        : Ww1Plc(rFibL, rFibL.GetFIB().fcPlcfhddGet(),
                    rFibL.GetFIB().cbPlcfhddGet(), 0)
    {}
};

// Arrays very similar to the plcs; limited to a size of 512 bytes
class Ww1Fkp
{
protected:
    sal_uInt8 aFkp[512];
    sal_uInt16 nItemSize;
    sal_Bool bOK;
    sal_uInt8* GetData(sal_uInt16);
public:
    Ww1Fkp(SvStream&, sal_uLong, sal_uInt16);
    friend std::ostream& operator <<(std::ostream&, Ww1Fkp&);
    sal_uInt16 Count() const            { return *(aFkp+511); }
    sal_uLong Where(sal_uInt16); // like in the corresponding fkp
};

class Ww1FkpPap : public Ww1Fkp
{
public:
    Ww1FkpPap(SvStream& rStream, sal_uLong ulFilePos)
        : Ww1Fkp(rStream, ulFilePos, 1)
    {}
    friend std::ostream& operator <<(std::ostream&, Ww1FkpPap&);
    sal_Bool Fill(sal_uInt16,  sal_uInt8*&, sal_uInt16&);
};

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

    friend std::ostream& operator <<(std::ostream&, Ww1FkpChp&);
    sal_Bool Fill(sal_uInt16, W1_CHP&);
};

class Ww1SprmPapx : public Ww1Sprm
{
    W1_PAPX aPapx;
    sal_uInt8* Sprm(sal_uInt8* p, sal_uInt16 nSize);
    sal_uInt16 SprmSize(sal_uInt8* p, sal_uInt16 nSize);
public:
    Ww1SprmPapx(sal_uInt8* p, sal_uInt16 nSize);
    friend std::ostream& operator <<(std::ostream&, Ww1SprmPapx&);
    void Start(Ww1Shell&, Ww1Manager&);
    void Stop(Ww1Shell&, Ww1Manager&);
};

class Ww1SprmSep : public Ww1Sprm
{
public:
    Ww1SprmSep(Ww1Fib& rFib, sal_uLong ulFilePos)
        : Ww1Sprm(rFib.GetStream(), ulFilePos)
    {}
    friend std::ostream& operator <<(std::ostream&, Ww1SprmSep&);
};

class Ww1Assoc
{
    enum fields { FileNext, Dot, Title, Subject, KeyWords, Comments,
        Author, LastRevBy, DataDoc, HeaderDoc, Criteria1, Criteria2,
        Criteria3, Criteria4, Criteria5, Criteria6, Criteria7, MaxFields };

    Ww1Fib& rFib;
    sal_Char* pBuffer;
    sal_Char* pStrTbl[ MaxFields ];
    sal_Bool bOK;

    OUString GetStr(sal_uInt16);

public:
    Ww1Assoc(Ww1Fib&);
    ~Ww1Assoc()             { delete pBuffer; }
    sal_Bool GetError() const   { return !bOK; }
    friend std::ostream& operator <<(std::ostream&, Ww1Assoc&);
    void Out(Ww1Shell&);
};

// Header/footer/footnote separators are stored in a separate text one
// after the other. A plc partitions these texts into several
// parts. They are numbered as ihdd. There are 9 different functions
// for these texts. When one of them is requested, it applies to the
// first ihdd, the next time to the second and so on. Which type a
// given text part will be treated as can only be determined by
// reading the file sequentially. The 9 types are: footnote separator,
// footnote continuation separator, footnote continuation note, even
// page header, odd page header, even page footer, odd page footer,
// header and footer for the first page. HeaderFooter remembers the
// current setting for each of these (but not the previous one) and
// the following ihdd. A part of type 0xffff denotes a non-existing
// part.
class Ww1HeaderFooter : public Ww1PlcHdd
{
    sal_uInt16 nextIhdd;        // next text part in HddText
    sal_uInt16 nFtnSep;         // footnote separator
    sal_uInt16 nFtnFollowSep;   // footnote continuation separator
    sal_uInt16 nFtnNote;        // footnote continuation note
    sal_uInt16 nEvenHeadL;
    sal_uInt16 nOddHeadL;
    sal_uInt16 nEvenFootL;
    sal_uInt16 nOddFootL;
    sal_uInt16 nFirstHeadL;
    sal_uInt16 nFirstFootL;
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

class Ww1Fields : public Ww1PlcFields
{
    sal_uInt16 nPlcIndex;
    OUString sResult; // Calculated by Word
    SwField* pField;
    sal_uLong Where(sal_uInt16 nIndex)  // within the text
        { return Ww1PlcFields::Where(nIndex) - rFib.GetFIB().fcMinGet(); }

public:
    Ww1Fields(Ww1Fib& rFibL, sal_uLong ulFilePos, sal_uInt16 nBytes)
        : Ww1PlcFields(rFibL, ulFilePos, nBytes), nPlcIndex(0), pField(0)
    {}
    // within the text
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
    friend std::ostream& operator <<(std::ostream&, Ww1Manager&);
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
    const OUString GetName() const;
    long Len() const;
    friend std::ostream& operator <<(std::ostream&, Ww1Bookmarks&);
    void Start(Ww1Shell&, Ww1Manager&);
    void Stop(Ww1Shell&, Ww1Manager&, sal_Unicode&);
    void Out(Ww1Shell&, Ww1Manager&, sal_uInt16=0);
};

class Ww1Footnotes : public Ww1PlcFootnoteRef
{
    sal_uInt16 nPlcIndex;
    Ww1PlcFootnoteTxt aText;
    sal_Bool bStarted;
public:
    Ww1Footnotes(Ww1Fib& rFibL)
        : Ww1PlcFootnoteRef(rFibL), nPlcIndex(0), aText(rFibL), bStarted(sal_False)
    {}
    // within the text
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
    // within the text
    sal_uLong Where()               { return Ww1PlcSep::Where(nPlcIndex); }
    void SetGrpfIhdt(sal_uInt8 grpfIhdt)
    {
        GetHdd().SetGrpfIhdt(grpfIhdt);
    }
    void Start(Ww1Shell&, Ww1Manager&);
    void Stop(Ww1Shell& rOut, Ww1Manager& rMan, sal_Unicode& c)
        { aHdd.Stop(rOut, rMan, c); }
};

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
    sal_uLong Where( sal_Bool bSetIndex = sal_True ); // within the text
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
    sal_uLong Where( sal_Bool bSetIndex = sal_True ); // within the text
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

// Central point of access for the ww side of the filter, constructed
// from the input stream (ww file). It contains everything necessary
// for being piped into the shell (pm side).
class Ww1Manager
{
    sal_Bool bOK;
    bool bInTtp;
    bool bInStyle;
    bool bStopAll;
    Ww1Fib aFib;
    Ww1Dop aDop;
    Ww1Fonts aFonts;
    // from now on, everything in pairs, for 'pushed':
    Ww1DocText aDoc;
    Ww1PlainText* pDoc;
    sal_uLong ulDocSeek;
    sal_uLong* pSeek;
    Ww1TextFields aFld;
    Ww1Fields* pFld;
    // 'push'able itself:
    Ww1Chp aChp;
    Ww1Pap aPap;
    // not present in text ranges if 'pushed'
    Ww1Footnotes aFtn;
    Ww1Bookmarks aBooks;
    Ww1Sep aSep;

    void OutStop( Ww1Shell&, sal_Unicode );
    void OutStart( Ww1Shell& );
    void Out(Ww1Shell&, sal_Unicode );

public:
    Ww1Manager(SvStream& rStrm, sal_uLong nFieldFlgs);
    sal_Bool GetError() const       { return !bOK; }

    // for tables
    void SetInTtp(bool bSet = true)     { bInTtp = bSet; }
    bool IsInTtp() const                { return bInTtp; }
    void SetInStyle(bool bSet = true)   { bInStyle = bSet; }
    bool IsInStyle() const              { return bInStyle; }
    void SetStopAll(bool bSet = true)   { bStopAll = bSet; }
    bool IsStopAll() const              { return bStopAll; }
    sal_Bool HasInTable();
    sal_Bool HasTtp();
    sal_Bool LastHasTtp();

    // for flys
    sal_Bool HasPPc();
    sal_Bool HasPDxaAbs();

    Ww1Fib& GetFib()                    { return aFib; }
    Ww1PlainText& GetText()             { return *pDoc; }
    Ww1Dop& GetDop()                    { return aDop; }
    Ww1Sep& GetSep()                    { return aSep; }
    // within the text
    sal_uLong Where()                       { return pDoc->Where(); }
    void Fill( sal_Unicode& rChr )      { pDoc->Out( rChr ); }
    sal_uInt8 Fill( OUString& rStr, sal_uLong ulLen)
    {
        ulLen += pDoc->Where();
        return sal::static_int_cast< sal_uInt8 >(pDoc->Out(rStr, ulLen));
    }
    SvxFontItem GetFont(sal_uInt16 nFCode);
    friend Ww1Shell& operator <<(Ww1Shell&, Ww1Manager&);
    friend std::ostream& operator <<(std::ostream&, Ww1Manager&);
    sal_Bool Pushed()                       { return pDoc != &aDoc; }
    void Pop();
    void Push0(Ww1PlainText* pDoc, sal_uLong, Ww1Fields* = 0);
    void Push1(Ww1PlainText* pDoc, sal_uLong ulSeek, sal_uLong ulSeek2 = 0,
               Ww1Fields* = 0);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
