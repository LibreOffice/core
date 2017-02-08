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

#include <cstdio>
#include <cstring>
#include <assert.h>

#include "fontsubset.hxx"

#include <vcl/strhelper.hxx>

//#define IGNORE_HINTS

typedef unsigned char U8;
typedef unsigned short U16;
typedef long long S64;

typedef sal_Int32 GlyphWidth;

typedef float RealType;
typedef RealType ValType;
#include <vector>

static const char* pStringIds[] = {
/*0*/   ".notdef",      "space",            "exclam",           "quotedbl",
    "numbersign",       "dollar",           "percent",          "ampersand",
    "quoteright",       "parenleft",        "parenright",       "asterisk",
    "plus",             "comma",            "hyphen",           "period",
/*16*/  "slash",        "zero",             "one",              "two",
    "three",            "four",             "five",             "six",
    "seven",            "eight",            "nine",             "colon",
    "semicolon",        "less",             "equal",            "greater",
/*32*/  "question",     "at",               "A",                "B",
    "C",                "D",                "E",                "F",
    "G",                "H",                "I",                "J",
    "K",                "L",                "M",                "N",
/*48*/  "O",            "P",                "Q",                "R",
    "S",                "T",                "U",                "V",
    "W",                "X",                "Y",                "Z",
    "bracketleft",      "backslash",        "bracketright",     "asciicircum",
/*64*/  "underscore",   "quoteleft",        "a",                "b",
    "c",                "d",                "e",                "f",
    "g",                "h",                "i",                "j",
    "k",                "l",                "m",                "n",
/*80*/  "o",            "p",                "q",                "r",
    "s",                "t",                "u",                "v",
    "w",                "x",                "y",                "z",
    "braceleft",        "bar",              "braceright",       "asciitilde",
/*96*/  "exclamdown",   "cent",             "sterlin",          "fraction",
    "yen",              "florin",           "section",          "currency",
    "quotesingle",      "quotedblleft",     "guillemotleft",    "guilsinglleft",
    "guilsinglright",   "fi",               "fl",               "endash",
/*112*/ "dagger",       "daggerdbl",        "periodcentered",   "paragraph",
    "bullet",           "quotesinglbase",   "quotedblbase",     "quotedblright",
    "guillemotright",   "ellipsis",         "perthousand",      "questiondown",
    "grave",            "acute",            "circumflex",       "tilde",
/*128*/ "macron",       "breve",            "dotaccent",        "dieresis",
    "ring",             "cedilla",          "hungarumlaut",     "ogonek",
    "caron",            "emdash",           "AE",               "ordfeminine",
    "Lslash",           "Oslash",           "OE",               "ordmasculine",
/*144*/ "ae",           "dotlessi",         "lslash",           "oslash",
    "oe",               "germandbls",       "onesuperior",      "logicalnot",
    "mu",               "trademark",        "Eth",              "onehalf",
    "plusminus",        "Thorn",            "onequarter",       "divide",
/*160*/ "brokenbar",    "degree",           "thorn",            "threequarters",
    "twosuperior",      "registered",       "minus",            "eth",
    "multiply",         "threesuperior",    "copyright",        "Aacute",
    "Acircumflex",      "Adieresis",        "Agrave",           "Aring",
/*176*/ "Atilde",       "Ccedilla",         "Eacute",           "Ecircumflex",
    "Edieresis",        "Egrave",           "Iacute",           "Icircumflex",
    "Idieresis",        "Igrave",           "Ntilde",           "Oacute",
    "Ocircumflex",      "Odieresis",        "Ograve",           "Otilde",
/*192*/ "Scaron",       "Uacute",           "Ucircumflex",      "Udieresis",
    "Ugrave",           "Yacute",           "Ydieresis",        "Zcaron",
    "aacute",           "acircumflex",      "adieresis",        "agrave",
    "aring",            "atilde",           "ccedilla",         "eacute",
/*208*/ "ecircumflex",  "edieresis",        "egrave",           "iacute",
    "icircumflex",      "idieresis",        "igrave",           "ntilde",
    "oacute",           "ocircumflex",      "odieresis",        "ograve",
    "otilde",           "scaron",           "uacute",           "ucircumflex",
/*224*/ "udieresis",    "ugrave",           "yacute",           "ydieresis",
    "zcaron",           "exclamsmall",      "Hungarumlautsmall","dollaroldstyle",
    "dollarsuperior",   "ampersandsmall",   "Acutesmall",       "parenleftsuperior",
    "parenrightsuperior","twodotenleader",  "onedotenleader",   "zerooldstyle",
/*240*/ "oneoldstyle",  "twooldstyle",      "threeoldstyle",    "fouroldstyle",
    "fiveoldstyle",     "sixoldstyle",      "sevenoldstyle",    "eightoldstyle",
    "nineoldstile",     "commasuperior",    "threequartersemdash","periodsuperior",
    "questionsmall",    "asuperior",        "bsuperior",        "centsuperior",
/*256*/ "dsuperior",    "esuperior",        "isuperior",        "lsuperior",
    "msuperior",        "nsuperior",        "osuperior",        "rsuperior",
    "ssuperior",        "tsuperior",        "ff",               "ffi",
    "ffl",              "parenleftinferior","parenrightinferior","Circumflexsmall",
/*272*/ "hyphensuperior","Gravesmall",      "Asmall",           "Bsmall",
    "Csmall",           "Dsmall",           "Esmall",           "Fsmall",
    "Gsmall",           "Hsmall",           "Ismall",           "Jsmall",
    "Ksmall",           "Lsmall",           "Msmall",           "Nsmall",
/*288*/ "Osmall",       "Psmall",           "Qsmall",           "Rsmall",
    "Ssmall",           "Tsmall",           "Usmall",           "Vsmall",
    "Wsmall",           "Xsmall",           "Ysmall",           "Zsmall",
    "colonmonetary",    "onefitted",        "rupia",            "Tildesmall",
/*304*/ "exclamdownsmall","centoldstyle",   "Lslashsmall",      "Scaronsmall",
    "Zcaronsmall",      "Dieresissmall",    "Brevesmall",       "Caronsmall",
    "Dotaccentsmall",   "Macronsmall",      "figuredash",       "hypheninferior",
    "Ogoneksmall",      "Ringsmall",        "Cedillasmall",     "questiondownsmall",
/*320*/ "oneeight",     "threeeights",      "fiveeights",       "seveneights",
    "onethird",         "twothirds",        "zerosuperior",     "foursuperior",
    "fivesuperior",     "sixsuperior",      "sevensuperior",    "eightsuperior",
    "ninesuperior",     "zeroinferior",     "oneinferior",      "twoinferior",
/*336*/ "threeinferior","fourinferior",     "fiveinferior",     "sixinferior",
    "seveninferior",    "eightinferior",    "nineinferior",     "centinferior",
    "dollarinferior",   "periodinferior",   "commainferior",    "Agravesmall",
    "Aacutesmall",      "Acircumflexsmall", "Atildesmall",      "Adieresissmall",
/*352*/ "Aringsmall",   "AEsmall",          "Ccedillasmall",    "Egravesmall",
    "Eacutesmall",      "Ecircumflexsmall", "Edieresissmall",   "Igravesmall",
    "Iacutesmall",      "Icircumflexsmall", "Idieresissmall",   "Ethsmall",
    "Ntildesmall",      "Ogravesmall",      "Oacutesmall",      "Ocircumflexsmall",
/*368*/ "Otildesmall",  "Odieressissmall",  "OEsmall",          "Oslashsmall",
    "Ugravesmall",      "Uacutesmall",      "Ucircumflexsmall", "Udieresissmall",
    "Yacutesmall",      "Thornsmall",       "Ydieresissmall",   "001.000",
    "001.001",          "001.002",          "001.003",          "Black",
/*384*/ "Bold",         "Book",             "Light",            "Medium",
    "Regular",          "Roman",            "Semibold"
};

// TOP DICT keywords (also covers PRIV DICT keywords)
static const char* pDictOps[] = {
    "sVersion",         "sNotice",              "sFullName",        "sFamilyName",
    "sWeight",          "aFontBBox",            "dBlueValues",      "dOtherBlues",
    "dFamilyBlues",     "dFamilyOtherBlues",    "nStdHW",           "nStdVW",
    "xESC",             "nUniqueID",            "aXUID",            "nCharset",
    "nEncoding",        "nCharStrings",         "PPrivate",         "nSubrs",
    "nDefaultWidthX",   "nNominalWidthX",       nullptr,               nullptr,
    nullptr,               nullptr,                   nullptr,               nullptr,
    "shortint",         "longint",              "BCD",              nullptr
};

// TOP DICT escapes (also covers PRIV DICT escapes)
static const char* pDictEscs[] = {
    "sCopyright",           "bIsFixedPitch",    "nItalicAngle",     "nUnderlinePosition",
    "nUnderlineThickness",  "nPaintType",       "tCharstringType",  "aFontMatrix",
    "nStrokeWidth",         "nBlueScale",       "nBlueShift",       "nBlueFuzz",
    "dStemSnapH",           "dStemSnapV",       "bForceBold",       nullptr,
    nullptr,                   "nLanguageGroup",   "nExpansionFactor", "nInitialRandomSeed",
    "nSyntheticBase",       "sPostScript",      "sBaseFontName",    "dBaseFontBlend",
    nullptr,                   nullptr,               nullptr,               nullptr,
    nullptr,                   nullptr,               "rROS",             "nCIDFontVersion",
    "nCIDFontRevision",     "nCIDFontType",     "nCIDCount",        "nUIDBase",
    "nFDArray",             "nFDSelect",        "sFontName"
};

static const char* pType1Ops[] = {
    nullptr,               "2hstem",           nullptr,               "2vstem",
    "1vmoveto",         "Arlineto",         "1hlineto",         "1vlineto",
    "Crrcurveto",       "0closepath",       "Lcallsubr",        "0return",
    "xT1ESC",           "2hsbw",            "0endchar",         nullptr,
    nullptr,               nullptr,               nullptr,               nullptr,
    nullptr,               "2rmoveto",         "1hmoveto",         nullptr,
    nullptr,               nullptr,               nullptr,               nullptr,
    nullptr,               nullptr,               "4vhcurveto",       "4hvcurveto"
};

static const char* pT1EscOps[] = {
    "0dotsection",      "6vstem3",          "6hstem3",          nullptr,
    nullptr,               nullptr,               "5seac",            "4sbw",
    nullptr,               "1abs",             "2add",             "2sub",
    "2div",             nullptr,               nullptr,               nullptr,
    "Gcallothersubr",   "1pop",             nullptr,               nullptr,
    nullptr,               nullptr,               nullptr,               nullptr,
    nullptr,               nullptr,               nullptr,               nullptr,
    nullptr,               nullptr,               nullptr,               nullptr,
    nullptr,               "2setcurrentpoint"
};

struct TYPE1OP
{
    enum OPS
    {
        HSTEM=1,        VSTEM=3,        VMOVETO=4,      RLINETO=5,
        HLINETO=6,      VLINETO=7,      RCURVETO=8,     CLOSEPATH=9,
        CALLSUBR=10,    RETURN=11,      T1ESC=12,       HSBW=13,
        ENDCHAR=14,     RMOVETO=21,     HMOVETO=22,     VHCURVETO=30,
        HVCURVETO=31
    };

    enum ESCS
    {
        DOTSECTION=0,   VSTEM3=1,           HSTEM3=2,   SEAC=6,
        SBW=7,          ABS=9,              ADD=10,     SUB=11,
        DIV=12,         CALLOTHERSUBR=16,   POP=17,     SETCURRENTPOINT=33
    };
};

static const char* pType2Ops[] = {
    nullptr,           "hhstem",       nullptr,           "vvstem",
    "mvmoveto",     "Arlineto",     "Ehlineto",     "Evlineto",
    "Crrcurveto",   nullptr,           "Lcallsubr",    "Xreturn",
    "xT2ESC",       nullptr,           "eendchar",     nullptr,
    nullptr,           nullptr,           "Hhstemhm",     "Khintmask",
    "Kcntrmask",    "Mrmoveto",     "mhmoveto",     "Vvstemhm",
    ".rcurveline",  ".rlinecurve",  ".vvcurveto",   ".hhcurveto",
    ".shortint",    "Gcallgsubr",   ".vhcurveto",   ".hvcurveto"
};

static const char* pT2EscOps[] = {
    nullptr,       nullptr,       nullptr,       "2and",
    "2or",      "1not",     nullptr,       nullptr,
    nullptr,       "1abs",     "2add",     "2sub",
    "2div",     nullptr,       "1neg",     "2eq",
    nullptr,       nullptr,       "1drop",    nullptr,
    "1put",     "1get",     "4ifelse",  "0random",
    "2mul",     nullptr,       "1sqrt",    "1dup",
    "2exch",    "Iindex",   "Rroll",    nullptr,
    nullptr,       nullptr,       "7hflex",   "Fflex",
    "9hflex1",  "fflex1"
};

struct TYPE2OP
{
    enum OPS
    {
        HSTEM=1,        VSTEM=3,        VMOVETO=4,      RLINETO=5,
        HLINETO=6,      VLINETO=7,      RCURVETO=8,     CALLSUBR=10,
        RETURN=11,      T2ESC=12,       ENDCHAR=14,     HSTEMHM=18,
        HINTMASK=19,    CNTRMASK=20,    RMOVETO=21,     HMOVETO=22,
        VSTEMHM=23,     RCURVELINE=24,  RLINECURVE=25,  VVCURVETO=26,
        HHCURVETO=27,   SHORTINT=28,    CALLGSUBR=29,   VHCURVETO=30,
        HVCURVETO=31
    };

    enum ESCS
    {
        AND=3,      OR=4,       NOT=5,      ABS=9,
        ADD=10,     SUB=11,     DIV=12,     NEG=14,
        EQ=15,      DROP=18,    PUT=20,     GET=21,
        IFELSE=22,  RANDOM=23,  MUL=24,     SQRT=26,
        DUP=27,     EXCH=28,    INDEX=29,   ROLL=30,
        HFLEX=34,   FLEX=35,    HFLEX1=36,  FLEX1=37
    };
};

struct CffGlobal
{
    explicit CffGlobal();

    int     mnNameIdxBase;
    int     mnNameIdxCount;
    int     mnStringIdxBase;
    int     mnStringIdxCount;
    bool    mbCIDFont;
    int     mnCharStrBase;
    int     mnCharStrCount;
    int     mnEncodingBase;
    int     mnCharsetBase;
    int     mnGlobalSubrBase;
    int     mnGlobalSubrCount;
    int     mnGlobalSubrBias;
    int     mnFDSelectBase;
    int     mnFontDictBase;
    int     mnFDAryCount;

    std::vector<ValType>   maFontBBox;
    std::vector<ValType>   maFontMatrix;

    int     mnFontNameSID;
    int     mnFullNameSID;
    int     mnFamilyNameSID;
};

struct CffLocal
{
    explicit CffLocal();

    int     mnPrivDictBase;
    int     mnPrivDictSize;
    int     mnLocalSubrOffs;
    int     mnLocalSubrBase;
    int     mnLocalSubrCount;
    int     mnLocalSubrBias;

    ValType maNominalWidth;
    ValType maDefaultWidth;

    // ATM hinting related values
    ValType     maStemStdHW;
    ValType     maStemStdVW;
    std::vector<ValType>   maStemSnapH;
    std::vector<ValType>   maStemSnapV;
    std::vector<ValType>   maBlueValues;
    std::vector<ValType>   maOtherBlues;
    std::vector<ValType>   maFamilyBlues;
    std::vector<ValType>   maFamilyOtherBlues;
    RealType    mfBlueScale;
    RealType    mfBlueShift;
    RealType    mfBlueFuzz;
    RealType    mfExpFactor;
    int         mnLangGroup;
    bool        mbForceBold;
};

class CffSubsetterContext
:   private CffGlobal
{
public:
    static const int NMAXSTACK = 48;    // see CFF.appendixB
    static const int NMAXHINTS = 2*96;  // see CFF.appendixB
    static const int NMAXTRANS = 32;    // see CFF.appendixB
public:
    explicit CffSubsetterContext( const U8* pBasePtr, int nBaseLen);

    bool    initialCffRead();
    bool    emitAsType1( class Type1Emitter&,
                const sal_GlyphId* pGlyphIds, const U8* pEncoding,
                GlyphWidth* pGlyphWidths, int nGlyphCount, FontSubsetInfo& );

    // used by charstring converter
    void    setCharStringType( int);
protected:
    int     convert2Type1Ops( CffLocal*, const U8* pType2Ops, int nType2Len, U8* pType1Ops);
private:
    void    convertOneTypeOp();
    void    convertOneTypeEsc();
    void    callType2Subr( bool bGlobal, int nSubrNumber);
    sal_Int32 getReadOfs() const { return (sal_Int32)(mpReadPtr - mpBasePtr);}

    const U8* mpBasePtr;
    const U8* mpBaseEnd;

    const U8* mpReadPtr;
    const U8* mpReadEnd;

    U8*     mpWritePtr;
    bool    mbNeedClose;
    bool    mbIgnoreHints;
    sal_Int32 mnCntrMask;

private:
    int     seekIndexData( int nIndexBase, int nDataIndex);
    void    seekIndexEnd( int nIndexBase);

private:
    const char**    mpCharStringOps;
    const char**    mpCharStringEscs;

    CffLocal    maCffLocal[256];
    CffLocal*   mpCffLocal;

    void        readDictOp();
    RealType    readRealVal();
    const char* getString( int nStringID);
    int         getFDSelect( int nGlyphIndex) const;
    int         getGlyphSID( int nGlyphIndex) const;
    const char* getGlyphName( int nGlyphIndex);

    void    read2push();
    void    writeType1Val( ValType);
    void    writeTypeOp( int nTypeOp);
    void    writeTypeEsc( int nTypeOp);
    void    writeCurveTo( int nStackPos, int nIX1, int nIY1, int nIX2, int nIY2, int nIX3, int nIY3);
    void    pop2MultiWrite( int nArgsPerTypo, int nTypeOp, int nTypeXor=0);
    void    popAll2Write( int nTypeOp);

public: // TODO: is public really needed?
    // accessing the value stack
    // TODO: add more checks
    void    push( ValType nVal) { mnValStack[ mnStackIdx++] = nVal;}
    ValType popVal() { return ((mnStackIdx>0) ? mnValStack[ --mnStackIdx] : 0);}
    ValType getVal( int nIndex) const { return mnValStack[ nIndex];}
    int     popInt();
    int     size() const { return mnStackIdx;}
    void    clear() { mnStackIdx = 0;}

    // accessing the charstring hints
    void    addHints( bool bVerticalHints);

    // accessing other charstring specifics
    bool    hasCharWidth() const { return (maCharWidth > 0);}
    ValType getCharWidth() const { return maCharWidth;}
    void    setNominalWidth( ValType aWidth) { mpCffLocal->maNominalWidth = aWidth;}
    void    setDefaultWidth( ValType aWidth) { mpCffLocal->maDefaultWidth = aWidth;}
    void    updateWidth( bool bUseFirstVal);

private:
    // typeop execution context
    int mnStackIdx;
    ValType mnValStack[ NMAXSTACK+4];
    ValType mnTransVals[ NMAXTRANS];

    int mnHintSize;
    int mnHorzHintSize;
    ValType mnHintStack[ NMAXHINTS];

    ValType maCharWidth;
};

CffSubsetterContext::CffSubsetterContext( const U8* pBasePtr, int nBaseLen)
    : mpBasePtr( pBasePtr)
    , mpBaseEnd( pBasePtr+nBaseLen)
    , mpReadPtr(nullptr)
    , mpReadEnd(nullptr)
    , mpWritePtr(nullptr)
    , mbNeedClose(false)
    , mbIgnoreHints(false)
    , mnCntrMask(0)
    , mpCharStringOps(nullptr)
    , mpCharStringEscs(nullptr)
    , mnStackIdx(0)
    , mnValStack{}
    , mnTransVals{}
    , mnHintSize(0)
    , mnHorzHintSize(0)
    , mnHintStack{}
    , maCharWidth(-1)
{
//  setCharStringType( 1);
    // TODO: new CffLocal[ mnFDAryCount];
    mpCffLocal = &maCffLocal[0];
}

inline int CffSubsetterContext::popInt()
{
    const ValType aVal = popVal();
    const int nInt = static_cast<int>(aVal);
    assert( nInt == aVal);
    return nInt;
}

inline void CffSubsetterContext::updateWidth( bool bUseFirstVal)
{
#if 1 // TODO: is this still needed?
    // the first value is not a hint but the charwidth
    if( hasCharWidth())
        return;
#endif
    if( bUseFirstVal) {
        maCharWidth = mpCffLocal->maNominalWidth + mnValStack[0];
        // remove bottom stack entry
        --mnStackIdx;
        for( int i = 0; i < mnStackIdx; ++i)
            mnValStack[ i] = mnValStack[ i+1];
    } else {
        maCharWidth = mpCffLocal->maDefaultWidth;
    }
}

void CffSubsetterContext::addHints( bool bVerticalHints)
{
    // the first charstring value may a charwidth instead of a charwidth
    updateWidth( (mnStackIdx & 1) != 0);
    // return early (e.g. no implicit hints for hintmask)
    if( !mnStackIdx)
        return;

    // copy the remaining values to the hint arrays
    // assert( (mnStackIdx & 1) == 0); // depends on called subrs
    if( mnStackIdx & 1) --mnStackIdx;//#######
    // TODO: if( !bSubr) assert( mnStackIdx >= 2);

    assert( (mnHintSize + mnStackIdx) <= 2*NMAXHINTS);

#ifdef IGNORE_HINTS
    mnHintSize += mnStackIdx;
#else
    ValType nHintOfs = 0;
    for( int i = 0; i < mnStackIdx; ++i) {
        nHintOfs += mnValStack[ i ];
        mnHintStack[ mnHintSize++] = nHintOfs;
    }
#endif // IGNORE_HINTS
    if( !bVerticalHints)
        mnHorzHintSize = mnHintSize;

    // clear all values from the stack
    mnStackIdx = 0;
}

void CffSubsetterContext::setCharStringType( int nVal)
{
    switch( nVal) {
        case 1: mpCharStringOps=pType1Ops; mpCharStringEscs=pT1EscOps; break;
        case 2: mpCharStringOps=pType2Ops; mpCharStringEscs=pT2EscOps; break;
        default: fprintf( stderr, "Unknown CharstringType=%d\n",nVal); break;
    }
}

void CffSubsetterContext::readDictOp()
{
    const U8 c = *mpReadPtr;
    if( c <= 21 ) {
        int nOpId = *(mpReadPtr++);
        const char* pCmdName = nullptr;
        if( nOpId != 12)
            pCmdName = pDictOps[nOpId];
        else {
            const U8 nExtId = *(mpReadPtr++);
            if (nExtId < 39)
               pCmdName = pDictEscs[nExtId];
            nOpId = 900 + nExtId;
        }

        if (!pCmdName)  // skip reserved operators
            return;

        //TODO: if( nStackIdx > 0)
        int nInt = 0;
        switch( *pCmdName) {
        default: fprintf( stderr, "unsupported DictOp.type=\'%c\'\n", *pCmdName); break;
        case 'b':   // bool
            nInt = popInt();
            switch( nOpId) {
            case 915: mpCffLocal->mbForceBold = nInt; break;    // "ForceBold"
            default: break; // TODO: handle more boolean dictops?
            }
            break;
        case 'n': { // dict-op number
            ValType nVal = popVal();
            nInt = static_cast<int>(nVal);
            switch( nOpId) {
            case  10: mpCffLocal->maStemStdHW = nVal; break;    // "StdHW"
            case  11: mpCffLocal->maStemStdVW = nVal; break;    // "StdVW"
            case  15: mnCharsetBase = nInt; break;              // "charset"
            case  16: mnEncodingBase = nInt; break;             // "nEncoding"
            case  17: mnCharStrBase = nInt; break;              // "nCharStrings"
            case  19: mpCffLocal->mnLocalSubrOffs = nInt; break;// "nSubrs"
            case  20: setDefaultWidth( nVal ); break;           // "defaultWidthX"
            case  21: setNominalWidth( nVal ); break;           // "nominalWidthX"
            case 909: mpCffLocal->mfBlueScale = nVal; break;    // "BlueScale"
            case 910: mpCffLocal->mfBlueShift = nVal; break;    // "BlueShift"
            case 911: mpCffLocal->mfBlueFuzz = nVal; break;     // "BlueFuzz"
            case 912: mpCffLocal->mfExpFactor = nVal; break;    // "ExpansionFactor"
            case 917: mpCffLocal->mnLangGroup = nInt; break;    // "LanguageGroup"
            case 936: mnFontDictBase = nInt; break;             // "nFDArray"
            case 937: mnFDSelectBase = nInt; break;             // "nFDSelect"
            default: break; // TODO: handle more numeric dictops?
            }
            } break;
        case 'a': { // array
            switch( nOpId) {
            case   5: maFontBBox.clear(); break;     // "FontBBox"
            case 907: maFontMatrix.clear(); break; // "FontMatrix"
            default: break; // TODO: reset other arrays?
            }
            for( int i = 0; i < size(); ++i ) {
                ValType nVal = getVal(i);
                switch( nOpId) {
                case   5: maFontBBox.push_back( nVal); break;     // "FontBBox"
                case 907: maFontMatrix.push_back( nVal); break; // "FontMatrix"
                default: break; // TODO: handle more array dictops?
                }
            }
            clear();
            } break;
        case 'd': { // delta array
            ValType nVal = 0;
            for( int i = 0; i < size(); ++i ) {
                nVal += getVal(i);
                switch( nOpId) {
                case   6: mpCffLocal->maBlueValues.push_back( nVal); break;     // "BlueValues"
                case   7: mpCffLocal->maOtherBlues.push_back( nVal); break;     // "OtherBlues"
                case   8: mpCffLocal->maFamilyBlues.push_back( nVal); break;    // "FamilyBlues"
                case   9: mpCffLocal->maFamilyOtherBlues.push_back( nVal); break;// "FamilyOtherBlues"
                case 912: mpCffLocal->maStemSnapH.push_back( nVal); break;      // "StemSnapH"
                case 913: mpCffLocal->maStemSnapV.push_back( nVal); break;      // "StemSnapV"
                default: break; // TODO: handle more delta-array dictops?
                }
            }
            clear();
            } break;
        case 's':   // stringid (SID)
            nInt = popInt();
            switch( nOpId ) {
            case   2: mnFullNameSID = nInt; break;      // "FullName"
            case   3: mnFamilyNameSID = nInt; break;    // "FamilyName"
            case 938: mnFontNameSID = nInt; break;      // "FontName"
            default: break; // TODO: handle more string dictops?
            }
            break;
        case 'P':   // private dict
            mpCffLocal->mnPrivDictBase = popInt();
            mpCffLocal->mnPrivDictSize = popInt();
            break;
        case 'r': { // ROS operands
            int nSid1 = popInt();
            int nSid2 = popInt();
            (void)nSid1; // TODO: use
            (void)nSid2; // TODO: use
            popVal();
            mbCIDFont = true;
            } break;
        case 't':   // CharstringType
            nInt = popInt();
            setCharStringType( nInt );
            break;
        }
    } else if( (c >= 32) || (c == 28) ) {
//      --mpReadPtr;
        read2push();
    } else if( c == 29 ) {      // longint
        ++mpReadPtr;            // skip 29
        int nS32 = mpReadPtr[0] << 24;
        nS32 += mpReadPtr[1] << 16;
        nS32 += mpReadPtr[2] << 8;
        nS32 += mpReadPtr[3] << 0;
        if( (sizeof(nS32) != 4) && (nS32 & (1U<<31)))
            nS32 |= (~0U) << 31;    // assuming 2s complement
        mpReadPtr += 4;
        ValType nVal = static_cast<ValType>(nS32);
        push( nVal );
    } else if( c == 30) {       // real number
        ++mpReadPtr; // skip 30
        const RealType fReal = readRealVal();
        // push value onto stack
        ValType nVal = fReal;
        push( nVal);
    }
}

void CffSubsetterContext::read2push()
{
    ValType aVal = 0;

    const U8*& p = mpReadPtr;
    const U8 c = *p;
    if( c == 28 ) {
        short nS16 = (p[1] << 8) + p[2];
        if( (sizeof(nS16) != 2) && (nS16 & (1<<15)))
            nS16 |= (~0U) << 15;    // assuming 2s complement
        aVal = nS16;
        p += 3;
    } else if( c <= 246 ) {     // -107..+107
        aVal = static_cast<ValType>(p[0] - 139);
        p += 1;
    } else if( c <= 250 ) {     // +108..+1131
        aVal = static_cast<ValType>(((p[0] << 8) + p[1]) - 63124);
        p += 2;
    } else if( c <= 254 ) {     // -108..-1131
        aVal = static_cast<ValType>(64148 - ((p[0] << 8) + p[1]));
        p += 2;
    } else /*if( c == 255)*/ {  // Fixed16.16
        int nS32 = (p[1] << 24) + (p[2] << 16) + (p[3] << 8) + p[4];
        if( (sizeof(nS32) != 2) && (nS32 & (1U<<31)))
            nS32 |= (~0U) << 31;    // assuming 2s complement
        aVal = static_cast<ValType>(nS32 * (1.0 / 0x10000));
        p += 5;
    }

    push( aVal);
}

void CffSubsetterContext::writeType1Val( ValType aVal)
{
    U8* pOut = mpWritePtr;

    int nInt = static_cast<int>(aVal);
    if( (nInt >= -107) && (nInt <= +107)) {
        *(pOut++) = static_cast<U8>(nInt + 139);    // -107..+107
    } else if( (nInt >= -1131) && (nInt <= +1131)) {
        if( nInt >= 0)
            nInt += 63124;                          // +108..+1131
        else
            nInt = 64148 - nInt;                    // -108..-1131
        *(pOut++) = static_cast<U8>(nInt >> 8);
        *(pOut++) = static_cast<U8>(nInt);
    } else {
        // numtype==255 means int32 for Type1, but 16.16 for Type2 charstrings!!!
        *(pOut++) = 255;
        *(pOut++) = static_cast<U8>(nInt >> 24);
        *(pOut++) = static_cast<U8>(nInt >> 16);
        *(pOut++) = static_cast<U8>(nInt >> 8);
        *(pOut++) = static_cast<U8>(nInt);
    }

    mpWritePtr = pOut;
}

inline void CffSubsetterContext::writeTypeOp( int nTypeOp)
{
    *(mpWritePtr++) = static_cast<U8>(nTypeOp);
}

inline void CffSubsetterContext::writeTypeEsc( int nTypeEsc)
{
    *(mpWritePtr++) = TYPE1OP::T1ESC;
    *(mpWritePtr++) = static_cast<U8>(nTypeEsc);
}

void CffSubsetterContext::pop2MultiWrite( int nArgsPerTypo, int nTypeOp, int nTypeXor)
{
    for( int i = 0; i < mnStackIdx;) {
        for( int j = 0; j < nArgsPerTypo; ++j) {
            const ValType aVal = mnValStack[i+j];
            writeType1Val( aVal);
        }
        i += nArgsPerTypo;
        writeTypeOp( nTypeOp);
        nTypeOp ^= nTypeXor;    // for toggling vlineto/hlineto
    }
    clear();
}

void CffSubsetterContext::popAll2Write( int nTypeOp)
{
    // pop in reverse order, then write
    for( int i = 0; i < mnStackIdx; ++i) {
        const ValType aVal = mnValStack[i];
        writeType1Val( aVal);
    }
    clear();
    writeTypeOp( nTypeOp);
}

void CffSubsetterContext::writeCurveTo( int nStackPos,
    int nIX1, int nIY1, int nIX2, int nIY2, int nIX3, int nIY3)
{
    // get the values from the stack
    const ValType nDX1 = nIX1 ? mnValStack[ nStackPos+nIX1 ] : 0;
    const ValType nDY1 = nIY1 ? mnValStack[ nStackPos+nIY1 ] : 0;
    const ValType nDX2 = nIX2 ? mnValStack[ nStackPos+nIX2 ] : 0;
    const ValType nDY2 = nIY2 ? mnValStack[ nStackPos+nIY2 ] : 0;
    const ValType nDX3 = nIX3 ? mnValStack[ nStackPos+nIX3 ] : 0;
    const ValType nDY3 = nIY3 ? mnValStack[ nStackPos+nIY3 ] : 0;

    // emit the curveto operator and operands
    // TODO: determine the most efficient curveto operator
    // TODO: depending on type1op or type2op target
    writeType1Val( nDX1 );
    writeType1Val( nDY1 );
    writeType1Val( nDX2 );
    writeType1Val( nDY2 );
    writeType1Val( nDX3 );
    writeType1Val( nDY3 );
    writeTypeOp( TYPE1OP::RCURVETO );
}

void CffSubsetterContext::convertOneTypeOp()
{
    const int nType2Op = *(mpReadPtr++);

    int i, nInt; // prevent WAE for declarations inside switch cases
    // convert each T2op
    switch( nType2Op) {
    case TYPE2OP::T2ESC:
        convertOneTypeEsc();
        break;
    case TYPE2OP::HSTEM:
    case TYPE2OP::VSTEM:
        addHints( nType2Op == TYPE2OP::VSTEM );
#ifndef IGNORE_HINTS
        for( i = 0; i < mnHintSize; i+=2 ) {
            writeType1Val( mnHintStack[i]);
            writeType1Val( mnHintStack[i+1] - mnHintStack[i]);
            writeTypeOp( nType2Op );
        }
#endif // IGNORE_HINTS
        break;
    case TYPE2OP::HSTEMHM:
    case TYPE2OP::VSTEMHM:
        addHints( nType2Op == TYPE2OP::VSTEMHM);
        break;
    case TYPE2OP::CNTRMASK:
        // TODO: replace cntrmask with vstem3/hstem3
        addHints( true);
#ifdef IGNORE_HINTS
        mpReadPtr += (mnHintSize + 15) / 16;
        mbIgnoreHints = true;
#else
        {
        U8 nMaskBit = 0;
        U8 nMaskByte = 0;
        for( i = 0; i < mnHintSize; i+=2, nMaskBit>>=1) {
            if( !nMaskBit) {
                nMaskByte = *(mpReadPtr++);
                nMaskBit = 0x80;
            }
            if( !(nMaskByte & nMaskBit))
                continue;
            if( i >= 8*(int)sizeof(mnCntrMask))
                mbIgnoreHints = true;
            if( mbIgnoreHints)
                continue;
            mnCntrMask |= (1U << i);
        }
        }
#endif
        break;
    case TYPE2OP::HINTMASK:
        addHints( true);
#ifdef IGNORE_HINTS
        mpReadPtr += (mnHintSize + 15) / 16;
#else
        {
        sal_Int32 nHintMask = 0;
        int nCntrBits[2] = {0,0};
        U8 nMaskBit = 0;
        U8 nMaskByte = 0;
        for( i = 0; i < mnHintSize; i+=2, nMaskBit>>=1) {
            if( !nMaskBit) {
                nMaskByte = *(mpReadPtr++);
                nMaskBit = 0x80;
            }
            if( !(nMaskByte & nMaskBit))
                continue;
            if( i >= 8*(int)sizeof(nHintMask))
                mbIgnoreHints = true;
            if( mbIgnoreHints)
                continue;
            nHintMask |= (1U << i);
            nCntrBits[ i < mnHorzHintSize] += (mnCntrMask >> i) & 1;
        }

        mbIgnoreHints |= (nCntrBits[0] && (nCntrBits[0] != 3));
        mbIgnoreHints |= (nCntrBits[1] && (nCntrBits[1] != 3));
        if( mbIgnoreHints)
            break;

        for( i = 0; i < mnHintSize; i+=2) {
            if( !(nHintMask & (1U << i)))
                continue;
            writeType1Val( mnHintStack[i]);
            writeType1Val( mnHintStack[i+1] - mnHintStack[i]);
            const bool bHorz = (i < mnHorzHintSize);
            if( !nCntrBits[ bHorz])
                writeTypeOp( bHorz ? TYPE1OP::HSTEM : TYPE1OP::VSTEM);
            else if( !--nCntrBits[ bHorz])
                writeTypeEsc( bHorz ? TYPE1OP::HSTEM3 : TYPE1OP::VSTEM3);
        }
        }
#endif
        break;
    case TYPE2OP::CALLSUBR:
    case TYPE2OP::CALLGSUBR:
        {
        nInt = popInt();
        const bool bGlobal = (nType2Op == TYPE2OP::CALLGSUBR);
        callType2Subr( bGlobal, nInt);
        }
        break;
    case TYPE2OP::RETURN:
        // TODO: check that we are in a subroutine
        return;
    case TYPE2OP::VMOVETO:
    case TYPE2OP::HMOVETO:
        if( mbNeedClose)
            writeTypeOp( TYPE1OP::CLOSEPATH);
        else
            updateWidth( size() > 1);
        mbNeedClose = true;
        pop2MultiWrite( 1, nType2Op);
        break;
    case TYPE2OP::VLINETO:
    case TYPE2OP::HLINETO:
        pop2MultiWrite( 1, nType2Op,
            TYPE1OP::VLINETO ^ TYPE1OP::HLINETO);
        break;
    case TYPE2OP::RMOVETO:
        // TODO: convert rmoveto to vlineto/hlineto if possible
        if( mbNeedClose)
            writeTypeOp( TYPE1OP::CLOSEPATH);
        else
            updateWidth( size() > 2);
        mbNeedClose = true;
        pop2MultiWrite( 2, nType2Op);
        break;
    case TYPE2OP::RLINETO:
        // TODO: convert rlineto to vlineto/hlineto if possible
        pop2MultiWrite( 2, nType2Op);
        break;
    case TYPE2OP::RCURVETO:
        // TODO: convert rcurveto to vh/hv/hh/vv-curveto if possible
        pop2MultiWrite( 6, nType2Op);
        break;
    case TYPE2OP::RCURVELINE:
        i = 0;
        while( (i += 6) <= mnStackIdx)
            writeCurveTo( i, -6, -5, -4, -3, -2, -1 );
        i -= 6;
        while( (i += 2) <= mnStackIdx) {
            writeType1Val( mnValStack[i-2]);
            writeType1Val( mnValStack[i-1]);
            writeTypeOp( TYPE2OP::RLINETO);
        }
        clear();
        break;
    case TYPE2OP::RLINECURVE:
        i = 0;
        while( (i += 2) <= mnStackIdx-6) {
            writeType1Val( mnValStack[i-2]);
            writeType1Val( mnValStack[i-1]);
            writeTypeOp( TYPE2OP::RLINETO);
        }
        i -= 2;
        while( (i += 6) <= mnStackIdx)
            writeCurveTo( i, -6, -5, -4, -3, -2, -1 );
        clear();
        break;
    case TYPE2OP::VHCURVETO:
    case TYPE2OP::HVCURVETO:
        {
        bool bVert = (nType2Op == TYPE2OP::VHCURVETO);
        i = 0;
        nInt = 0;
        if( mnStackIdx & 1 )
            nInt = static_cast<int>(mnValStack[ --mnStackIdx ]);
        while( (i += 4) <= mnStackIdx) {
            // TODO: use writeCurveTo()
            if( bVert ) writeType1Val( 0 );
            writeType1Val( mnValStack[i-4] );
            if( !bVert ) writeType1Val( 0);
            writeType1Val( mnValStack[i-3] );
            writeType1Val( mnValStack[i-2] );
            if( !bVert ) writeType1Val( static_cast<ValType>((i==mnStackIdx) ? nInt : 0) );
            writeType1Val( mnValStack[i-1] );
            if( bVert ) writeType1Val( static_cast<ValType>((i==mnStackIdx) ? nInt : 0) );
            bVert = !bVert;
            writeTypeOp( TYPE2OP::RCURVETO);
        }
        }
        clear();
        break;
    case TYPE2OP::HHCURVETO:
        i = (mnStackIdx & 1);
        while( (i += 4) <= mnStackIdx) {
            if( i != 5)
                writeCurveTo( i, -4,  0, -3, -2, -1, 0);
            else
                writeCurveTo( i, -4, -5, -3, -2, -1, 0);
        }
        clear();
        break;
    case TYPE2OP::VVCURVETO:
        i = (mnStackIdx & 1);
        while( (i += 4) <= mnStackIdx) {
            if( i != 5)
                writeCurveTo( i,  0, -4, -3, -2, 0, -1);
            else
                writeCurveTo( i, -5, -4, -3, -2, 0, -1);
        }
        clear();
        break;
    case TYPE2OP::ENDCHAR:
        if( mbNeedClose)
            writeTypeOp( TYPE1OP::CLOSEPATH);
        else
            updateWidth( size() >= 1);
        // mbNeedClose = true;
        writeTypeOp( TYPE1OP::ENDCHAR);
        break;
    default:
        if( ((nType2Op >= 32) && (nType2Op <= 255)) || (nType2Op == 28)) {
            --mpReadPtr;
            read2push();
        } else {
            popAll2Write( nType2Op);
            assert(false && "TODO?");
        }
        break;
    }
}

void CffSubsetterContext::convertOneTypeEsc()
{
    const int nType2Esc = *(mpReadPtr++);
    ValType* pTop = &mnValStack[ mnStackIdx-1];
    // convert each T2op
    switch( nType2Esc) {
    case TYPE2OP::AND:
        assert( mnStackIdx >= 2 );
        pTop[0] = static_cast<ValType>(static_cast<int>(pTop[0]) & static_cast<int>(pTop[-1]));
        --mnStackIdx;
        break;
    case TYPE2OP::OR:
        assert( mnStackIdx >= 2 );
        pTop[0] = static_cast<ValType>(static_cast<int>(pTop[0]) | static_cast<int>(pTop[-1]));
        --mnStackIdx;
        break;
    case TYPE2OP::NOT:
        assert( mnStackIdx >= 1 );
        pTop[0] = ValType(pTop[0] == 0);
        break;
    case TYPE2OP::ABS:
        assert( mnStackIdx >= 1 );
        if( pTop[0] >= 0)
            break;
        SAL_FALLTHROUGH;
    case TYPE2OP::NEG:
        assert( mnStackIdx >= 1 );
        pTop[0] = -pTop[0];
        break;
    case TYPE2OP::ADD:
        assert( mnStackIdx >= 2 );
        pTop[0] += pTop[-1];
        --mnStackIdx;
        break;
    case TYPE2OP::SUB:
        assert( mnStackIdx >= 2 );
        pTop[0] -= pTop[-1];
        --mnStackIdx;
        break;
    case TYPE2OP::MUL:
        assert( mnStackIdx >= 2 );
        if( pTop[-1])
            pTop[0] *= pTop[-1];
        --mnStackIdx;
        break;
    case TYPE2OP::DIV:
        assert( mnStackIdx >= 2 );
        if( pTop[-1])
            pTop[0] /= pTop[-1];
        --mnStackIdx;
        break;
    case TYPE2OP::EQ:
        assert( mnStackIdx >= 2 );
        pTop[0] = ValType(pTop[0] == pTop[-1]);
        --mnStackIdx;
        break;
    case TYPE2OP::DROP:
        assert( mnStackIdx >= 1 );
        --mnStackIdx;
        break;
    case TYPE2OP::PUT: {
        assert( mnStackIdx >= 2 );
        const int nIdx = static_cast<int>(pTop[0]);
        assert( nIdx >= 0 );
        assert( nIdx < NMAXTRANS );
        mnTransVals[ nIdx] = pTop[-1];
        mnStackIdx -= 2;
        break;
        }
    case TYPE2OP::GET: {
        assert( mnStackIdx >= 1 );
        const int nIdx = static_cast<int>(pTop[0]);
        assert( nIdx >= 0 );
        assert( nIdx < NMAXTRANS );
        pTop[0] = mnTransVals[ nIdx ];
        break;
        }
    case TYPE2OP::IFELSE: {
        assert( mnStackIdx >= 4 );
        if( pTop[-1] > pTop[0] )
            pTop[-3] = pTop[-2];
        mnStackIdx -= 3;
        break;
        }
    case TYPE2OP::RANDOM:
        pTop[+1] = 1234; // TODO
        ++mnStackIdx;
        break;
    case TYPE2OP::SQRT:
        // TODO: implement
        break;
    case TYPE2OP::DUP:
        assert( mnStackIdx >= 1 );
        pTop[+1] = pTop[0];
        ++mnStackIdx;
        break;
    case TYPE2OP::EXCH: {
        assert( mnStackIdx >= 2 );
        const ValType nVal = pTop[0];
        pTop[0] = pTop[-1];
        pTop[-1] = nVal;
        break;
        }
    case TYPE2OP::INDEX: {
        assert( mnStackIdx >= 1 );
        const int nVal = static_cast<int>(pTop[0]);
        assert( nVal >= 0 );
        assert( nVal < mnStackIdx-1 );
        pTop[0] = pTop[-1-nVal];
        break;
        }
    case TYPE2OP::ROLL: {
        assert( mnStackIdx >= 1 );
        const int nNum = static_cast<int>(pTop[0]);
        assert( nNum >= 0);
        assert( nNum < mnStackIdx-2 );
        (void)nNum; // TODO: implement
        const int nOfs = static_cast<int>(pTop[-1]);
        mnStackIdx -= 2;
        (void)nOfs;// TODO: implement
        break;
        }
    case TYPE2OP::HFLEX1: {
            assert( mnStackIdx == 9);

            writeCurveTo( mnStackIdx, -9, -8, -7, -6, -5,  0);
            writeCurveTo( mnStackIdx, -4,  0, -3, -2, -1,  0);
        // TODO: emulate hflex1 using othersubr call

            mnStackIdx -= 9;
        }
        break;
    case TYPE2OP::HFLEX: {
            assert( mnStackIdx == 7);
            ValType* pX = &mnValStack[ mnStackIdx];

            pX[+1] = -pX[-5]; // temp: +dy5==-dy2
            writeCurveTo( mnStackIdx, -7,  0, -6, -5, -4,  0);
            writeCurveTo( mnStackIdx, -3,  0, -2, +1, -1,  0);
        // TODO: emulate hflex using othersubr call

            mnStackIdx -= 7;
        }
        break;
    case TYPE2OP::FLEX: {
            assert( mnStackIdx == 13 );
            writeCurveTo( mnStackIdx, -13, -12, -11, -10, -9, -8 );
            writeCurveTo( mnStackIdx,  -7,  -6,  -5,  -4, -3, -2 );
            const ValType nFlexDepth =  mnValStack[ mnStackIdx-1 ];
            (void)nFlexDepth; // ignoring nFlexDepth
            mnStackIdx -= 13;
        }
        break;
    case TYPE2OP::FLEX1: {
            assert( mnStackIdx == 11 );
            // write the first part of the flex1-hinted curve
            writeCurveTo( mnStackIdx, -11, -10, -9, -8, -7, -6 );

            // determine if nD6 is horizontal or vertical
            const int i = mnStackIdx;
            ValType nDeltaX = mnValStack[i-11] + mnValStack[i-9] + mnValStack[i-7] + mnValStack[i-5] + mnValStack[i-3];
            if( nDeltaX < 0 ) nDeltaX = -nDeltaX;
            ValType nDeltaY = mnValStack[i-10] + mnValStack[i-8] + mnValStack[i-6] + mnValStack[i-4] + mnValStack[i-2];
            if( nDeltaY < 0 ) nDeltaY = -nDeltaY;
            const bool bVertD6 = (nDeltaY > nDeltaX);

            // write the second part of the flex1-hinted curve
            if( !bVertD6 )
                writeCurveTo( mnStackIdx, -5, -4, -3, -2, -1, 0);
            else
                writeCurveTo( mnStackIdx, -5, -4, -3, -2, 0, -1);
            mnStackIdx -= 11;
        }
        break;
    default:
        fprintf( stderr,"unhandled type2esc %d\n", nType2Esc);
        assert( false);
        break;
    }
}

void CffSubsetterContext::callType2Subr( bool bGlobal, int nSubrNumber)
{
    const U8* const pOldReadPtr = mpReadPtr;
    const U8* const pOldReadEnd = mpReadEnd;

    if( bGlobal ) {
        nSubrNumber += mnGlobalSubrBias;
        seekIndexData( mnGlobalSubrBase, nSubrNumber);
    } else {
        nSubrNumber += mpCffLocal->mnLocalSubrBias;
        seekIndexData( mpCffLocal->mnLocalSubrBase, nSubrNumber);
    }

    while( mpReadPtr < mpReadEnd)
        convertOneTypeOp();

    mpReadPtr = pOldReadPtr;
    mpReadEnd = pOldReadEnd;
}

static const int MAX_T1OPS_SIZE = 81920; // TODO: use dynamic value

int CffSubsetterContext::convert2Type1Ops( CffLocal* pCffLocal, const U8* const pT2Ops, int nT2Len, U8* const pT1Ops)
{
    mpCffLocal = pCffLocal;

    // prepare the charstring conversion
    mpWritePtr = pT1Ops;
#if 1   // TODO: update caller
    U8 aType1Ops[ MAX_T1OPS_SIZE];
    if( !pT1Ops)
        mpWritePtr = aType1Ops;
    *const_cast<U8**>(&pT1Ops) = mpWritePtr;
#else
    assert( pT1Ops);
#endif

    // prepend random seed for T1crypt
    *(mpWritePtr++) = 0x48;
    *(mpWritePtr++) = 0x44;
    *(mpWritePtr++) = 0x55;
    *(mpWritePtr++) = ' ';
#if 1 // convert the Type2 charstring to Type1
    mpReadPtr = pT2Ops;
    mpReadEnd = pT2Ops + nT2Len;
    // prepend "hsbw" or "sbw"
    // TODO: only emit hsbw when charwidth is known
    // TODO: remove charwidth from T2 stack
    writeType1Val( 0); // TODO: aSubsetterContext.getLeftSideBearing();
    writeType1Val( 1000/*###getCharWidth()###*/);
    writeTypeOp( TYPE1OP::HSBW);
mbNeedClose = false;
mbIgnoreHints = false;
mnHintSize=mnHorzHintSize=mnStackIdx=0; maCharWidth=-1;//#######
mnCntrMask = 0;
    while( mpReadPtr < mpReadEnd)
        convertOneTypeOp();
//  if( bActivePath)
//      writeTypeOp( TYPE1OP::CLOSEPATH);
//  if( bSubRoutine)
//      writeTypeOp( TYPE1OP::RETURN);
#else // useful for manually encoding charstrings
    mpWritePtr = pT1Ops;
    mpWritePtr += sprintf( (char*)mpWritePtr, "OOo_\x8b\x8c\x0c\x10\x0b");
#endif
    const int nType1Len = mpWritePtr - pT1Ops;

    // encrypt the Type1 charstring
    int nRDCryptR = 4330; // TODO: mnRDCryptSeed;
    for( U8* p = pT1Ops; p < mpWritePtr; ++p) {
        *p ^= (nRDCryptR >> 8);
        nRDCryptR = (*p + nRDCryptR) * 52845 + 22719;
    }

    return nType1Len;
}

RealType CffSubsetterContext::readRealVal()
{
    // TODO: more thorough number validity test
    bool bComma = false;
    int nExpVal = 0;
    int nExpSign = 0;
    S64 nNumber = 0;
    RealType fReal = +1.0;
    for(;;){
        const U8 c = *(mpReadPtr++); // read nibbles
        // parse high nibble
        const U8 nH = c >> 4U;
        if( nH <= 9) {
            nNumber = nNumber * 10 + nH;
            --nExpVal;
        } else if( nH == 10) {  // comma
            nExpVal = 0;
            bComma = true;
        } else if( nH == 11) {  // +exp
            fReal *= nNumber;
            nExpSign = +1;
            nNumber = 0;
        } else if( nH == 12) {  // -exp
            fReal *= nNumber;
            nExpSign = -1;
            nNumber = 0;
        } else if( nH == 13) {  // reserved
            // TODO: ignore or error?
        } else if( nH == 14)    // minus
            fReal = -fReal;
        else if( nH == 15)  // end
            break;
        // parse low nibble
        const U8 nL = c & 0x0F;
        if( nL <= 9) {
            nNumber = nNumber * 10 + nL;
            --nExpVal;
        } else if( nL == 10) {  // comma
            nExpVal = 0;
            bComma = true;
        } else if( nL == 11) {  // +exp
            fReal *= nNumber;
            nNumber = 0;
            nExpSign = +1;
        } else if( nL == 12) {  // -exp
            fReal *= nNumber;
            nNumber = 0;
            nExpSign = -1;
        } else if( nL == 13) {  // reserved
            // TODO: ignore or error?
        } else if( nL == 14)    // minus
            fReal = -fReal;
        else if( nL == 15)  // end
            break;
    }

    // merge exponents
    if( !bComma)
        nExpVal = 0;
    if( !nExpSign) { fReal *= nNumber;}
    else if( nExpSign > 0) { nExpVal += static_cast<int>(nNumber);}
    else if( nExpSign < 0) { nExpVal -= static_cast<int>(nNumber);}

    // apply exponents
    if( !nExpVal) { /*nothing to apply*/}
    else if( nExpVal > 0) { while( --nExpVal >= 0) fReal *= 10.0;}
    else if( nExpVal < 0) { while( ++nExpVal <= 0) fReal /= 10.0;}
    return fReal;
}

// prepare to access an element inside a CFF/CID index table
int CffSubsetterContext::seekIndexData( int nIndexBase, int nDataIndex)
{
    assert( (nIndexBase > 0) && (mpBasePtr + nIndexBase + 3 <= mpBaseEnd));
    if( nDataIndex < 0)
        return -1;
    mpReadPtr = mpBasePtr + nIndexBase;
    const int nDataCount = (mpReadPtr[0]<<8) + mpReadPtr[1];
    if( nDataIndex >= nDataCount)
        return -1;
    const int nDataOfsSz = mpReadPtr[2];
    mpReadPtr += 3 + (nDataOfsSz * nDataIndex);
    int nOfs1 = 0;
    switch( nDataOfsSz) {
        default: fprintf( stderr, "\tINVALID nDataOfsSz=%d\n\n", nDataOfsSz); return -1;
        case 1: nOfs1 = mpReadPtr[0]; break;
        case 2: nOfs1 = (mpReadPtr[0]<<8) + mpReadPtr[1]; break;
        case 3: nOfs1 = (mpReadPtr[0]<<16) + (mpReadPtr[1]<<8) + mpReadPtr[2]; break;
        case 4: nOfs1 = (mpReadPtr[0]<<24) + (mpReadPtr[1]<<16) + (mpReadPtr[2]<<8) + mpReadPtr[3]; break;
    }
    mpReadPtr += nDataOfsSz;

    int nOfs2 = 0;
    switch( nDataOfsSz) {
        case 1: nOfs2 = mpReadPtr[0]; break;
        case 2: nOfs2 = (mpReadPtr[0]<<8) + mpReadPtr[1]; break;
        case 3: nOfs2 = (mpReadPtr[0]<<16) + (mpReadPtr[1]<<8) + mpReadPtr[2]; break;
        case 4: nOfs2 = (mpReadPtr[0]<<24) + (mpReadPtr[1]<<16) + (mpReadPtr[2]<<8) + mpReadPtr[3]; break;
    }

    mpReadPtr = mpBasePtr + (nIndexBase + 2) + nDataOfsSz * (nDataCount + 1) + nOfs1;
    mpReadEnd = mpReadPtr + (nOfs2 - nOfs1);
    assert( nOfs1 >= 0);
    assert( nOfs2 >= nOfs1);
    assert( mpReadPtr <= mpBaseEnd);
    assert( mpReadEnd <= mpBaseEnd);
    return (nOfs2 - nOfs1);
}

// skip over a CFF/CID index table
void CffSubsetterContext::seekIndexEnd( int nIndexBase)
{
    assert( (nIndexBase > 0) && (mpBasePtr + nIndexBase + 3 <= mpBaseEnd));
    mpReadPtr = mpBasePtr + nIndexBase;
    const int nDataCount = (mpReadPtr[0]<<8) + mpReadPtr[1];
    const int nDataOfsSz = mpReadPtr[2];
    mpReadPtr += 3 + nDataOfsSz * nDataCount;
    assert( mpReadPtr <= mpBaseEnd);
    int nEndOfs = 0;
    switch( nDataOfsSz) {
        default: fprintf( stderr, "\tINVALID nDataOfsSz=%d\n\n", nDataOfsSz); return;
        case 1: nEndOfs = mpReadPtr[0]; break;
        case 2: nEndOfs = (mpReadPtr[0]<<8) + mpReadPtr[1]; break;
        case 3: nEndOfs = (mpReadPtr[0]<<16) + (mpReadPtr[1]<<8) + mpReadPtr[2];break;
        case 4: nEndOfs = (mpReadPtr[0]<<24) + (mpReadPtr[1]<<16) + (mpReadPtr[2]<<8) + mpReadPtr[3]; break;
    }
    mpReadPtr += nDataOfsSz;
    mpReadPtr += nEndOfs - 1;
    mpReadEnd = mpBaseEnd;
    assert( nEndOfs >= 0);
    assert( mpReadEnd <= mpBaseEnd);
}

// initialize FONTDICT specific values
CffLocal::CffLocal()
:   mnPrivDictBase( 0)
,   mnPrivDictSize( 0)
,   mnLocalSubrOffs( 0)
,   mnLocalSubrBase( 0)
,   mnLocalSubrCount( 0)
,   mnLocalSubrBias( 0)
,   maNominalWidth( 0)
,   maDefaultWidth( 0)
,   maStemStdHW( 0)
,   maStemStdVW( 0)
,   mfBlueScale( 0.0)
,   mfBlueShift( 0.0)
,   mfBlueFuzz( 0.0)
,   mfExpFactor( 0.0)
,   mnLangGroup( 0)
,   mbForceBold( false)
{
}

CffGlobal::CffGlobal()
:   mnNameIdxBase( 0)
,   mnNameIdxCount( 0)
,   mnStringIdxBase( 0)
,   mnStringIdxCount( 0)
,   mbCIDFont( false)
,   mnCharStrBase( 0)
,   mnCharStrCount( 0)
,   mnEncodingBase( 0)
,   mnCharsetBase( 0)
,   mnGlobalSubrBase( 0)
,   mnGlobalSubrCount( 0)
,   mnGlobalSubrBias( 0)
,   mnFDSelectBase( 0)
,   mnFontDictBase( 0)
,   mnFDAryCount( 1)
,   mnFontNameSID( 0)
,   mnFullNameSID( 0)
,   mnFamilyNameSID( 0)
{
}

bool CffSubsetterContext::initialCffRead()
{
    // get the CFFHeader
    mpReadPtr = mpBasePtr;
    const U8 nVerMajor = *(mpReadPtr++);
    const U8 nVerMinor = *(mpReadPtr++);
    const U8 nHeaderSize = *(mpReadPtr++);
    const U8 nOffsetSize = *(mpReadPtr++);
    // TODO: is the version number useful for anything else?
    assert( (nVerMajor == 1) && (nVerMinor == 0));
    (void)(nVerMajor + nVerMinor + nOffsetSize); // avoid compiler warnings

    // prepare access to the NameIndex
    mnNameIdxBase = nHeaderSize;
    mpReadPtr = mpBasePtr + nHeaderSize;
    mnNameIdxCount = (mpReadPtr[0]<<8) + mpReadPtr[1];
    seekIndexEnd( mnNameIdxBase);

    // get the TopDict index
    const sal_Int32 nTopDictBase = getReadOfs();
    const int nTopDictCount = (mpReadPtr[0]<<8) + mpReadPtr[1];
    if( nTopDictCount) {
        for( int i = 0; i < nTopDictCount; ++i) {
            seekIndexData( nTopDictBase, i);
            while( mpReadPtr < mpReadEnd)
                readDictOp();
            assert( mpReadPtr == mpReadEnd);
        }
    }

    // prepare access to the String index
    mnStringIdxBase =  getReadOfs();
    mnStringIdxCount = (mpReadPtr[0]<<8) + mpReadPtr[1];
    seekIndexEnd( mnStringIdxBase);

    // prepare access to the GlobalSubr index
    mnGlobalSubrBase =  getReadOfs();
    mnGlobalSubrCount = (mpReadPtr[0]<<8) + mpReadPtr[1];
    mnGlobalSubrBias = (mnGlobalSubrCount<1240)?107:(mnGlobalSubrCount<33900)?1131:32768;
    // skip past the last GlobalSubr entry
//  seekIndexEnd( mnGlobalSubrBase);

    // get/skip the Encodings (we got mnEncodingBase from TOPDICT)
//  seekEncodingsEnd( mnEncodingBase);
    // get/skip the Charsets (we got mnCharsetBase from TOPDICT)
//  seekCharsetsEnd( mnCharStrBase);
    // get/skip FDSelect (CID only) data

    // prepare access to the CharStrings index (we got the base from TOPDICT)
    mpReadPtr = mpBasePtr + mnCharStrBase;
    mnCharStrCount = (mpReadPtr[0]<<8) + mpReadPtr[1];
//  seekIndexEnd( mnCharStrBase);

    // read the FDArray index (CID only)
    if( mbCIDFont) {
//      assert( mnFontDictBase == tellRel());
        mpReadPtr = mpBasePtr + mnFontDictBase;
        mnFDAryCount = (mpReadPtr[0]<<8) + mpReadPtr[1];
        if (static_cast<size_t>(mnFDAryCount) >= SAL_N_ELEMENTS(maCffLocal))
        {
            SAL_INFO("vcl.fonts", "CffSubsetterContext: too many CFF in font");
            return false;
        }

        // read FDArray details to get access to the PRIVDICTs
        for( int i = 0; i < mnFDAryCount; ++i) {
            mpCffLocal = &maCffLocal[i];
            seekIndexData( mnFontDictBase, i);
            while( mpReadPtr < mpReadEnd)
                readDictOp();
            assert( mpReadPtr == mpReadEnd);
        }
    }

    for( int i = 0; i < mnFDAryCount; ++i) {
        mpCffLocal = &maCffLocal[i];

        // get the PrivateDict index
        // (we got mnPrivDictSize and mnPrivDictBase from TOPDICT or FDArray)
        if( mpCffLocal->mnPrivDictSize != 0) {
            assert( mpCffLocal->mnPrivDictSize > 0);
            // get the PrivDict data
            mpReadPtr = mpBasePtr + mpCffLocal->mnPrivDictBase;
            mpReadEnd = mpReadPtr + mpCffLocal->mnPrivDictSize;
            assert( mpReadEnd <= mpBaseEnd);
            // read PrivDict details
            while( mpReadPtr < mpReadEnd)
                readDictOp();
        }

        // prepare access to the LocalSubrs (we got mnLocalSubrOffs from PRIVDICT)
        if( mpCffLocal->mnLocalSubrOffs) {
            // read LocalSubrs summary
            mpCffLocal->mnLocalSubrBase = mpCffLocal->mnPrivDictBase + mpCffLocal->mnLocalSubrOffs;
            mpReadPtr = mpBasePtr + mpCffLocal->mnLocalSubrBase;
            const int nSubrCount = (mpReadPtr[0] << 8) + mpReadPtr[1];
            mpCffLocal->mnLocalSubrCount = nSubrCount;
            mpCffLocal->mnLocalSubrBias = (nSubrCount<1240)?107:(nSubrCount<33900)?1131:32768;
//          seekIndexEnd( mpCffLocal->mnLocalSubrBase);
        }
    }

    // ignore the Notices info

    return true;
}

// get a cstring from a StringID
const char* CffSubsetterContext::getString( int nStringID)
{
    // get a standard string if possible
    const static int nStdStrings = sizeof(pStringIds)/sizeof(*pStringIds);
    if( (nStringID >= 0) && (nStringID < nStdStrings))
        return pStringIds[ nStringID];

    // else get the string from the StringIndex table
    const U8* pReadPtr = mpReadPtr;
    const U8* pReadEnd = mpReadEnd;
    nStringID -= nStdStrings;
    int nLen = seekIndexData( mnStringIdxBase, nStringID);
    // assert( nLen >= 0);
    // TODO: just return the undecorated name
    // TODO: get rid of static char buffer
    static char aNameBuf[ 2560];
    if( nLen < 0) {
        sprintf( aNameBuf, "name[%d].notfound!", nStringID);
    } else {
        const int nMaxLen = sizeof(aNameBuf) - 1;
        if( nLen >= nMaxLen)
            nLen = nMaxLen;
        for( int i = 0; i < nLen; ++i)
            aNameBuf[i] = *(mpReadPtr++);
        aNameBuf[ nLen] = '\0';
    }
    mpReadPtr = pReadPtr;
    mpReadEnd = pReadEnd;
    return aNameBuf;
}

// access a CID's FDSelect table
int CffSubsetterContext::getFDSelect( int nGlyphIndex) const
{
    assert( nGlyphIndex >= 0);
    assert( nGlyphIndex < mnCharStrCount);
    if( !mbCIDFont)
        return 0;

    const U8* pReadPtr = mpBasePtr + mnFDSelectBase;
    const U8 nFDSelFormat = *(pReadPtr++);
    switch( nFDSelFormat) {
        case 0: { // FDSELECT format 0
                pReadPtr += nGlyphIndex;
                const U8 nFDIdx = *(pReadPtr++);
                return nFDIdx;
            } //break;
        case 3: { // FDSELECT format 3
                const U16 nRangeCount = (pReadPtr[0]<<8) + pReadPtr[1];
                assert( nRangeCount > 0);
                assert( nRangeCount <= mnCharStrCount);
                U16 nPrev = (pReadPtr[2]<<8) + pReadPtr[3];
                assert( nPrev == 0);
                (void)nPrev;
                pReadPtr += 4;
                // TODO? binary search
                for( int i = 0; i < nRangeCount; ++i) {
                    const U8 nFDIdx = pReadPtr[0];
                    const U16 nNext = (pReadPtr[1]<<8) + pReadPtr[2];
                    assert( nPrev < nNext);
                    if( nGlyphIndex < nNext)
                        return nFDIdx;
                    pReadPtr += 3;
                    nPrev = nNext;
                }
            } break;
        default:    // invalid FDselect format
            fprintf( stderr, "invalid CFF.FdselType=%d\n", nFDSelFormat);
            break;
    }

    assert( false);
    return -1;
}

int CffSubsetterContext::getGlyphSID( int nGlyphIndex) const
{
    if( nGlyphIndex == 0)
        return 0;       // ".notdef"
    assert( nGlyphIndex >= 0);
    assert( nGlyphIndex < mnCharStrCount);
    if( (nGlyphIndex < 0) || (nGlyphIndex >= mnCharStrCount))
        return -1;

    // get the SID/CID from the Charset table
     const U8* pReadPtr = mpBasePtr + mnCharsetBase;
    const U8 nCSetFormat = *(pReadPtr++);
    int nGlyphsToSkip = nGlyphIndex - 1;
    switch( nCSetFormat) {
        case 0: // charset format 0
            pReadPtr += 2 * nGlyphsToSkip;
            nGlyphsToSkip = 0;
            break;
        case 1: // charset format 1
            while( nGlyphsToSkip >= 0) {
                const int nLeft = pReadPtr[2];
                if( nGlyphsToSkip <= nLeft)
                    break;
                nGlyphsToSkip -= nLeft + 1;
                pReadPtr += 3;
            }
            break;
        case 2: // charset format 2
            while( nGlyphsToSkip >= 0) {
                const int nLeft = (pReadPtr[2]<<8) + pReadPtr[3];
                if( nGlyphsToSkip <= nLeft)
                    break;
                nGlyphsToSkip -= nLeft + 1;
                pReadPtr += 4;
            }
            break;
        default:
            fprintf( stderr, "ILLEGAL CFF-Charset format %d\n", nCSetFormat);
            return -2;
    }

    int nSID = (pReadPtr[0]<<8) + pReadPtr[1];
    nSID += nGlyphsToSkip;
    // NOTE: for CID-fonts the resulting SID is interpreted as CID
    return nSID;
}

// NOTE: the result becomes invalid with the next call to this method
const char* CffSubsetterContext::getGlyphName( int nGlyphIndex)
{
    // the first glyph is always the .notdef glyph
    const char* pGlyphName = ".notdef";
    if( nGlyphIndex == 0)
        return pGlyphName;

    // prepare a result buffer
    // TODO: get rid of static buffer
    static char aDefaultGlyphName[64];
    pGlyphName = aDefaultGlyphName;

    // get the glyph specific name
    const int nSID = getGlyphSID( nGlyphIndex);
    if( nSID < 0)           // default glyph name
        sprintf( aDefaultGlyphName, "gly%03d", nGlyphIndex);
    else if( mbCIDFont)     // default glyph name in CIDs
         sprintf( aDefaultGlyphName, "cid%03d", nSID);
    else {                  // glyph name from string table
        const char* pSidName = getString( nSID);
        // check validity of glyph name
        if( pSidName) {
            const char* p = pSidName;
            while( (*p >= '0') && (*p <= 'z')) ++p;
            if( (p >= pSidName+1) && (*p == '\0'))
                pGlyphName = pSidName;
        }
        // if needed invent a fallback name
        if( pGlyphName != pSidName)
             sprintf( aDefaultGlyphName, "bad%03d", nSID);
    }

     return pGlyphName;
}

class Type1Emitter
{
public:
    explicit    Type1Emitter( FILE* pOutFile, bool bPfbSubset);
    ~Type1Emitter();
    void        setSubsetName( const char* );

    size_t      emitRawData( const char* pData, size_t nLength) const;
    void        emitAllRaw();
    void        emitAllHex();
    void        emitAllCrypted();
    int         tellPos() const;
    void        updateLen( int nTellPos, size_t nLength);
    void        emitValVector( const char* pLineHead, const char* pLineTail, const std::vector<ValType>&);
private:
    FILE*       mpFileOut;
    char        maBuffer[MAX_T1OPS_SIZE];   // TODO: dynamic allocation
    int         mnEECryptR;
public:
    char*       mpPtr;

    char        maSubsetName[256];
    bool        mbPfbSubset;
    int         mnHexLineCol;
};

Type1Emitter::Type1Emitter( FILE* pOutFile, bool bPfbSubset)
:   mpFileOut( pOutFile)
,   maBuffer{}
,   mnEECryptR( 55665)  // default eexec seed, TODO: mnEECryptSeed
,   mpPtr( maBuffer)
,   mbPfbSubset( bPfbSubset)
,   mnHexLineCol( 0)
{
    maSubsetName[0] = '\0';
}

Type1Emitter::~Type1Emitter()
{
    if( !mpFileOut)
        return;
    mpFileOut = nullptr;
}

void Type1Emitter::setSubsetName( const char* pSubsetName)
{
    maSubsetName[0] = '\0';
    if( pSubsetName)
        strncpy( maSubsetName, pSubsetName, sizeof(maSubsetName));
    maSubsetName[sizeof(maSubsetName)-1] = '\0';
}

int Type1Emitter::tellPos() const
{
    int nTellPos = ftell( mpFileOut);
    return nTellPos;
}

void Type1Emitter::updateLen( int nTellPos, size_t nLength)
{
    // update PFB segment header length
    U8 cData[4];
    cData[0] = static_cast<U8>(nLength >>  0);
    cData[1] = static_cast<U8>(nLength >>  8);
    cData[2] = static_cast<U8>(nLength >> 16);
    cData[3] = static_cast<U8>(nLength >> 24);
    const long nCurrPos = ftell(mpFileOut);
    if (nCurrPos < 0)
        return;
    if (fseek( mpFileOut, nTellPos, SEEK_SET) != 0)
        return;
    fwrite(cData, 1, sizeof(cData), mpFileOut);
    if( nCurrPos >= 0)
        (void)fseek(mpFileOut, nCurrPos, SEEK_SET);
}

inline size_t Type1Emitter::emitRawData(const char* pData, size_t nLength) const
{
    return fwrite( pData, 1, nLength, mpFileOut);
}

inline void Type1Emitter::emitAllRaw()
{
    // writeout raw data
    assert( (mpPtr - maBuffer) < (int)sizeof(maBuffer));
    emitRawData( maBuffer, mpPtr - maBuffer);
    // reset the raw buffer
    mpPtr = maBuffer;
}

inline void Type1Emitter::emitAllHex()
{
    assert( (mpPtr - maBuffer) < (int)sizeof(maBuffer));
    for( const char* p = maBuffer; p < mpPtr;) {
        // convert binary chunk to hex
        char aHexBuf[0x4000];
        char* pOut = aHexBuf;
        while( (p < mpPtr) && (pOut < aHexBuf+sizeof(aHexBuf)-4)) {
            // convert each byte to hex
            char cNibble = (static_cast<unsigned char>(*p) >> 4) & 0x0F;
            cNibble += (cNibble < 10) ? '0' : 'A'-10;
            *(pOut++) = cNibble;
            cNibble = *(p++) & 0x0F;
            cNibble += (cNibble < 10) ? '0' : 'A'-10;
            *(pOut++) = cNibble;
            // limit the line length
            if( (++mnHexLineCol & 0x3F) == 0)
                *(pOut++) = '\n';
        }
        // writeout hex-converted chunk
        emitRawData( aHexBuf, pOut-aHexBuf);
    }
    // reset the raw buffer
    mpPtr = maBuffer;
}

void Type1Emitter::emitAllCrypted()
{
    // apply t1crypt
    for( char* p = maBuffer; p < mpPtr; ++p) {
        *p ^= (mnEECryptR >> 8);
        mnEECryptR = (*reinterpret_cast<U8*>(p) + mnEECryptR) * 52845 + 22719;
    }

    // emit the t1crypt result
    if( mbPfbSubset)
        emitAllRaw();
    else
        emitAllHex();
}

// #i110387# quick-and-dirty double->ascii conversion
// needed because sprintf/ecvt/etc. alone are too localized (LC_NUMERIC)
// also strip off trailing zeros in fraction while we are at it
inline int dbl2str( char* pOut, double fVal)
{
    const int nLen = psp::getValueOfDouble( pOut, fVal, 6);
    return nLen;
}

void Type1Emitter::emitValVector( const char* pLineHead, const char* pLineTail,
    const std::vector<ValType>& rVector)
{
    // ignore empty vectors
    if( rVector.empty())
        return;

    // emit the line head
    mpPtr += sprintf( mpPtr, "%s", pLineHead);
    // emit the vector values
    std::vector<ValType>::value_type aVal = 0;
    for( std::vector<ValType>::const_iterator it = rVector.begin();;) {
        aVal = *it;
        if( ++it == rVector.end() )
            break;
        mpPtr += dbl2str( mpPtr, aVal);
        *(mpPtr++) = ' ';
    }
    // emit the last value
    mpPtr += dbl2str( mpPtr, aVal);
    // emit the line tail
    mpPtr += sprintf( mpPtr, "%s", pLineTail);
}

bool CffSubsetterContext::emitAsType1( Type1Emitter& rEmitter,
    const sal_GlyphId* pReqGlyphIds, const U8* pReqEncoding,
    GlyphWidth* pGlyphWidths, int nGlyphCount, FontSubsetInfo& rFSInfo)
{
    // prepare some fontdirectory details
    static const int nUniqueIdBase = 4100000; // using private-interchange UniqueIds
    static int nUniqueId = nUniqueIdBase;
    ++nUniqueId;

    char* pFontName = rEmitter.maSubsetName;
    if( !*pFontName ) {
        if( mnFontNameSID) {
            // get the fontname directly if available
            strncpy( pFontName, getString( mnFontNameSID), sizeof(rEmitter.maSubsetName) - 1);
            pFontName[sizeof(rEmitter.maSubsetName) - 1] = 0;
        } else if( mnFullNameSID) {
            // approximate fontname as fullname-whitespace
            const char* pI = getString( mnFullNameSID);
            char* pO = pFontName;
            const char* pLimit = pFontName + sizeof(rEmitter.maSubsetName) - 1;
            while( pO < pLimit) {
                const char c = *(pI++);
                if( c != ' ')
                    *(pO++) = c;
                if( !c)
                    break;
            }
            *pO = '\0';
        } else {
            // fallback name of last resort
            strncpy( pFontName, "DummyName", sizeof(rEmitter.maSubsetName));
        }
    }
    const char* pFullName = pFontName;
    const char* pFamilyName = pFontName;

    char*& pOut = rEmitter.mpPtr; // convenience reference, TODO: cleanup

    // create a PFB+Type1 header
    if( rEmitter.mbPfbSubset ) {
        static const char aPfbHeader[] = "\x80\x01\x00\x00\x00\x00";
        rEmitter.emitRawData( aPfbHeader, sizeof(aPfbHeader)-1);
    }

    pOut += sprintf( pOut, "%%!FontType1-1.0: %s 001.003\n", rEmitter.maSubsetName);
    // emit TOPDICT
    pOut += sprintf( pOut,
        "11 dict begin\n"   // TODO: dynamic entry count for TOPDICT
        "/FontType 1 def\n"
        "/PaintType 0 def\n");
    pOut += sprintf( pOut, "/FontName /%s def\n", rEmitter.maSubsetName);
    pOut += sprintf( pOut, "/UniqueID %d def\n", nUniqueId);
    // emit FontMatrix
    if( maFontMatrix.size() == 6)
        rEmitter.emitValVector( "/FontMatrix [", "]readonly def\n", maFontMatrix);
    else // emit default FontMatrix if needed
        pOut += sprintf( pOut, "/FontMatrix [0.001 0 0 0.001 0 0]readonly def\n");
    // emit FontBBox
    if( maFontBBox.size() == 4)
        rEmitter.emitValVector( "/FontBBox {", "}readonly def\n", maFontBBox);
    else // emit default FontBBox if needed
        pOut += sprintf( pOut, "/FontBBox {0 0 999 999}readonly def\n");
    // emit FONTINFO into TOPDICT
    pOut += sprintf( pOut,
        "/FontInfo 2 dict dup begin\n"  // TODO: check fontinfo entry count
        " /FullName (%s) readonly def\n"
        " /FamilyName (%s) readonly def\n"
        "end readonly def\n",
            pFullName, pFamilyName);

    pOut += sprintf( pOut,
        "/Encoding 256 array\n"
        "0 1 255 {1 index exch /.notdef put} for\n");
    for( int i = 1; (i < nGlyphCount) && (i < 256); ++i) {
        const char* pGlyphName = getGlyphName( pReqGlyphIds[i]);
        pOut += sprintf( pOut, "dup %d /%s put\n", pReqEncoding[i], pGlyphName);
    }
    pOut += sprintf( pOut, "readonly def\n");
    pOut += sprintf( pOut,
        // TODO: more topdict entries
        "currentdict end\n"
        "currentfile eexec\n");

    // emit PFB header
    rEmitter.emitAllRaw();
    if( rEmitter.mbPfbSubset) {
        // update PFB header segment
        const int nPfbHeaderLen = rEmitter.tellPos() - 6;
        rEmitter.updateLen( 2, nPfbHeaderLen);

        // prepare start of eexec segment
        rEmitter.emitRawData( "\x80\x02\x00\x00\x00\x00", 6);   // segment start
    }
    const int nEExecSegTell = rEmitter.tellPos();

    // which always starts with a privdict
    // count the privdict entries
    int nPrivEntryCount = 9;
#if !defined(IGNORE_HINTS)
    // emit blue hints only if non-default values
    nPrivEntryCount += int(!mpCffLocal->maOtherBlues.empty());
    nPrivEntryCount += int(!mpCffLocal->maFamilyBlues.empty());
    nPrivEntryCount += int(!mpCffLocal->maFamilyOtherBlues.empty());
    nPrivEntryCount += int(mpCffLocal->mfBlueScale != 0.0);
    nPrivEntryCount += int(mpCffLocal->mfBlueShift != 0.0);
    nPrivEntryCount += int(mpCffLocal->mfBlueFuzz != 0.0);
    // emit stem hints only if non-default values
    nPrivEntryCount += int(mpCffLocal->maStemStdHW != 0);
    nPrivEntryCount += int(mpCffLocal->maStemStdVW != 0);
    nPrivEntryCount += int(!mpCffLocal->maStemSnapH.empty());
    nPrivEntryCount += int(!mpCffLocal->maStemSnapV.empty());
    // emit other hints only if non-default values
    nPrivEntryCount += int(mpCffLocal->mfExpFactor != 0.0);
    nPrivEntryCount += int(mpCffLocal->mnLangGroup != 0);
    nPrivEntryCount += int(mpCffLocal->mnLangGroup == 1);
    nPrivEntryCount += int(mpCffLocal->mbForceBold);
#endif // IGNORE_HINTS
    // emit the privdict header
    pOut += sprintf( pOut,
        "\110\104\125 "
        "dup\n/Private %d dict dup begin\n"
        "/RD{string currentfile exch readstring pop}executeonly def\n"
        "/ND{noaccess def}executeonly def\n"
        "/NP{noaccess put}executeonly def\n"
        "/MinFeature{16 16}ND\n"
        "/password 5839 def\n",     // TODO: mnRDCryptSeed?
            nPrivEntryCount);

#if defined(IGNORE_HINTS)
    pOut += sprintf( pOut, "/BlueValues []ND\n");   // BlueValues are mandatory
#else
    // emit blue hint related privdict entries
    if( !mpCffLocal->maBlueValues.empty())
        rEmitter.emitValVector( "/BlueValues [", "]ND\n", mpCffLocal->maBlueValues);
    else
        pOut += sprintf( pOut, "/BlueValues []ND\n"); // default to empty BlueValues
    rEmitter.emitValVector( "/OtherBlues [", "]ND\n", mpCffLocal->maOtherBlues);
    rEmitter.emitValVector( "/FamilyBlues [", "]ND\n", mpCffLocal->maFamilyBlues);
    rEmitter.emitValVector( "/FamilyOtherBlues [", "]ND\n", mpCffLocal->maFamilyOtherBlues);

    if( mpCffLocal->mfBlueScale) {
        pOut += sprintf( pOut, "/BlueScale ");
        pOut += dbl2str( pOut, mpCffLocal->mfBlueScale);
        pOut += sprintf( pOut, " def\n");
    }
    if( mpCffLocal->mfBlueShift) {  // default BlueShift==7
        pOut += sprintf( pOut, "/BlueShift ");
        pOut += dbl2str( pOut, mpCffLocal->mfBlueShift);
        pOut += sprintf( pOut, " def\n");
    }
    if( mpCffLocal->mfBlueFuzz) {       // default BlueFuzz==1
        pOut += sprintf( pOut, "/BlueFuzz ");
        pOut += dbl2str( pOut, mpCffLocal->mfBlueFuzz);
        pOut += sprintf( pOut, " def\n");
    }

    // emit stem hint related privdict entries
    if( mpCffLocal->maStemStdHW) {
        pOut += sprintf( pOut, "/StdHW [");
        pOut += dbl2str( pOut, mpCffLocal->maStemStdHW);
        pOut += sprintf( pOut, "] def\n");
    }
    if( mpCffLocal->maStemStdVW) {
        pOut += sprintf( pOut, "/StdVW [");
        pOut += dbl2str( pOut, mpCffLocal->maStemStdVW);
        pOut += sprintf( pOut, "] def\n");
    }
    rEmitter.emitValVector( "/StemSnapH [", "]ND\n", mpCffLocal->maStemSnapH);
    rEmitter.emitValVector( "/StemSnapV [", "]ND\n", mpCffLocal->maStemSnapV);

    // emit other hints
    if( mpCffLocal->mbForceBold)
        pOut += sprintf( pOut, "/ForceBold true def\n");
    if( mpCffLocal->mnLangGroup != 0)
        pOut += sprintf( pOut, "/LanguageGroup %d def\n", mpCffLocal->mnLangGroup);
    if( mpCffLocal->mnLangGroup == 1) // compatibility with ancient printers
        pOut += sprintf( pOut, "/RndStemUp false def\n");
    if( mpCffLocal->mfExpFactor) {
        pOut += sprintf( pOut, "/ExpansionFactor ");
        pOut += dbl2str( pOut, mpCffLocal->mfExpFactor);
        pOut += sprintf( pOut, " def\n");
    }
#endif // IGNORE_HINTS

    // emit remaining privdict entries
    pOut += sprintf( pOut, "/UniqueID %d def\n", nUniqueId);
    // TODO?: more privdict entries?

    static const char aOtherSubrs[] =
        "/OtherSubrs\n"
        "% Dummy code for faking flex hints\n"
        "[ {} {} {} {systemdict /internaldict known not {pop 3}\n"
        "{1183615869 systemdict /internaldict get exec\n"
        "dup /startlock known\n"
        "{/startlock get exec}\n"
        "{dup /strtlck known\n"
        "{/strtlck get exec}\n"
        "{pop 3}\nifelse}\nifelse}\nifelse\n} executeonly\n"
        "] ND\n";
    memcpy( pOut, aOtherSubrs, sizeof(aOtherSubrs)-1);
    pOut += sizeof(aOtherSubrs)-1;

    // emit used GlobalSubr charstrings
    // these are the just the default subrs
    // TODO: do we need them as the flex hints are resolved differently?
    static const char aSubrs[] =
        "/Subrs 5 array\n"
        "dup 0 15 RD \x5F\x3D\x6B\xAC\x3C\xBD\x74\x3D\x3E\x17\xA0\x86\x58\x08\x85 NP\n"
        "dup 1 9 RD \x5F\x3D\x6B\xD8\xA6\xB5\x68\xB6\xA2 NP\n"
        "dup 2 9 RD \x5F\x3D\x6B\xAC\x39\x46\xB9\x43\xF9 NP\n"
        "dup 3 5 RD \x5F\x3D\x6B\xAC\xB9 NP\n"
        "dup 4 12 RD \x5F\x3D\x6B\xAC\x3E\x5D\x48\x54\x62\x76\x39\x03 NP\n"
        "ND\n";
    memcpy( pOut, aSubrs, sizeof(aSubrs)-1);
    pOut += sizeof(aSubrs)-1;

    // TODO: emit more GlobalSubr charstrings?
    // TODO: emit used LocalSubr charstrings?

    // emit the CharStrings for the requested glyphs
    pOut += sprintf( pOut,
        "2 index /CharStrings %d dict dup begin\n", nGlyphCount);
    rEmitter.emitAllCrypted();
    for( int i = 0; i < nGlyphCount; ++i) {
        const int nCffGlyphId = pReqGlyphIds[i];
        assert( (nCffGlyphId >= 0) && (nCffGlyphId < mnCharStrCount));
        // get privdict context matching to the glyph
        const int nFDSelect = getFDSelect( nCffGlyphId);
        if( nFDSelect < 0)
            continue;
        mpCffLocal = &maCffLocal[ nFDSelect];
        // convert the Type2op charstring to its Type1op counterpart
        const int nT2Len = seekIndexData( mnCharStrBase, nCffGlyphId);
        assert( nT2Len > 0);
        U8 aType1Ops[ MAX_T1OPS_SIZE]; // TODO: dynamic allocation
        const int nT1Len = convert2Type1Ops( mpCffLocal, mpReadPtr, nT2Len, aType1Ops);
        // get the glyph name
        const char* pGlyphName = getGlyphName( nCffGlyphId);
        // emit the encrypted Type1op charstring
        pOut += sprintf( pOut, "/%s %d RD ", pGlyphName, nT1Len);
        memcpy( pOut, aType1Ops, nT1Len);
        pOut += nT1Len;
        pOut += sprintf( pOut, " ND\n");
        rEmitter.emitAllCrypted();
        // provide individual glyphwidths if requested
        if( pGlyphWidths ) {
            ValType aCharWidth = getCharWidth();
            if( maFontMatrix.size() >= 4)
                aCharWidth *= 1000.0F * maFontMatrix[0];
            pGlyphWidths[i] = static_cast<GlyphWidth>(aCharWidth);
        }
    }
    pOut += sprintf( pOut, "end end\nreadonly put\nput\n");
    pOut += sprintf( pOut, "dup/FontName get exch definefont pop\n");
    pOut += sprintf( pOut, "mark currentfile closefile\n");
    rEmitter.emitAllCrypted();

    // mark stop of eexec encryption
     if( rEmitter.mbPfbSubset) {
        const int nEExecLen = rEmitter.tellPos() - nEExecSegTell;
        rEmitter.updateLen( nEExecSegTell-4, nEExecLen);
     }

    // create PFB footer
    static const char aPfxFooter[] = "\x80\x01\x14\x02\x00\x00\n" // TODO: check segment len
        "0000000000000000000000000000000000000000000000000000000000000000\n"
        "0000000000000000000000000000000000000000000000000000000000000000\n"
        "0000000000000000000000000000000000000000000000000000000000000000\n"
        "0000000000000000000000000000000000000000000000000000000000000000\n"
        "0000000000000000000000000000000000000000000000000000000000000000\n"
        "0000000000000000000000000000000000000000000000000000000000000000\n"
        "0000000000000000000000000000000000000000000000000000000000000000\n"
        "0000000000000000000000000000000000000000000000000000000000000000\n"
        "cleartomark\n"
        "\x80\x03";
     if( rEmitter.mbPfbSubset)
        rEmitter.emitRawData( aPfxFooter, sizeof(aPfxFooter)-1);
    else
        rEmitter.emitRawData( aPfxFooter+6, sizeof(aPfxFooter)-9);

    // provide details to the subset requesters, TODO: move into own method?
    // note: Top and Bottom are flipped between Type1 and VCL
    // note: the rest of VCL expects the details below to be scaled like for an emUnits==1000 font
    ValType fXFactor = 1.0;
    ValType fYFactor = 1.0;
    if( maFontMatrix.size() >= 4) {
        fXFactor = 1000.0F * maFontMatrix[0];
        fYFactor = 1000.0F * maFontMatrix[3];
    }
    rFSInfo.m_aFontBBox = Rectangle( Point( static_cast<sal_Int32>(maFontBBox[0] * fXFactor),
                                        static_cast<sal_Int32>(maFontBBox[1] * fYFactor) ),
                                    Point( static_cast<sal_Int32>(maFontBBox[2] * fXFactor),
                                        static_cast<sal_Int32>(maFontBBox[3] * fYFactor) ) );
    // PDF-Spec says the values below mean the ink bounds!
    // TODO: use better approximations for these ink bounds
    rFSInfo.m_nAscent  = +rFSInfo.m_aFontBBox.Bottom(); // for capital letters
    rFSInfo.m_nDescent = -rFSInfo.m_aFontBBox.Top();    // for all letters
    rFSInfo.m_nCapHeight = rFSInfo.m_nAscent;           // for top-flat capital letters

    rFSInfo.m_nFontType = rEmitter.mbPfbSubset ? FontType::TYPE1_PFB : FontType::TYPE1_PFA;
    rFSInfo.m_aPSName   = OUString( rEmitter.maSubsetName, strlen(rEmitter.maSubsetName), RTL_TEXTENCODING_UTF8 );

    return true;
}

bool FontSubsetInfo::CreateFontSubsetFromCff( GlyphWidth* pOutGlyphWidths )
{
    CffSubsetterContext aCff( mpInFontBytes, mnInByteLength);
    bool bRC = aCff.initialCffRead();
    if (!bRC)
        return bRC;

    // emit Type1 subset from the CFF input
    // TODO: also support CFF->CFF subsetting (when PDF-export and PS-printing need it)
    const bool bPfbSubset(mnReqFontTypeMask & FontType::TYPE1_PFB);
    Type1Emitter aType1Emitter( mpOutFile, bPfbSubset);
    aType1Emitter.setSubsetName( mpReqFontName);
    bRC = aCff.emitAsType1( aType1Emitter,
        mpReqGlyphIds, mpReqEncodedIds,
        pOutGlyphWidths, mnReqGlyphCount, *this);
    return bRC;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
