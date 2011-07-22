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

#ifndef _PARHTML_HXX
#define _PARHTML_HXX

#include "svtools/svtdllapi.h"
#include <tools/solar.h>
#include <tools/string.hxx>
#include <svl/svarray.hxx>
#include <svtools/svparser.hxx>


namespace com { namespace sun { namespace star {
    namespace document {
        class XDocumentProperties;
    }
} } }

class Color;
class SvNumberFormatter;
class SvULongs;
class SvKeyValueIterator;

#define HTMLFONTSZ1_DFLT 7
#define HTMLFONTSZ2_DFLT 10
#define HTMLFONTSZ3_DFLT 12
#define HTMLFONTSZ4_DFLT 14
#define HTMLFONTSZ5_DFLT 18
#define HTMLFONTSZ6_DFLT 24
#define HTMLFONTSZ7_DFLT 36

enum HTMLTableFrame { HTML_TF_VOID, HTML_TF_ABOVE, HTML_TF_BELOW,
    HTML_TF_HSIDES, HTML_TF_LHS, HTML_TF_RHS, HTML_TF_VSIDES, HTML_TF_BOX };

enum HTMLTableRules { HTML_TR_NONE, HTML_TR_GROUPS, HTML_TR_ROWS,
    HTML_TR_COLS, HTML_TR_ALL };

enum HTMLInputType
{
    HTML_IT_TEXT =      0x01,
    HTML_IT_PASSWORD =  0x02,
    HTML_IT_CHECKBOX =  0x03,
    HTML_IT_RADIO =     0x04,
    HTML_IT_RANGE =     0x05,
    HTML_IT_SCRIBBLE =  0x06,
    HTML_IT_FILE =      0x07,
    HTML_IT_HIDDEN =    0x08,
    HTML_IT_SUBMIT =    0x09,
    HTML_IT_IMAGE =     0x0a,
    HTML_IT_RESET =     0x0b,
    HTML_IT_BUTTON =    0x0c
};

enum HTMLScriptLanguage
{
    HTML_SL_STARBASIC,
    HTML_SL_JAVASCRIPT,
    HTML_SL_UNKNOWN
};

struct HTMLOptionEnum
{
    const sal_Char *pName;  // Wert einer HTML-Option
    sal_uInt16 nValue;      // und der dazugehoerige Wert eines Enums
};

// Repraesentation einer HTML-Option (=Atrribut in einem Start-Tag)
// Die Werte der Optionen werden immer als String gespeichert.
// Die Methoden GetNumber, ... duerfen nur aufgerufen werden, wenn
// die Option auch numerisch, ... ist.

class SVT_DLLPUBLIC HTMLOption
{
    String aValue;          // der Wert der Option (immer als String)
    String aToken;          // der Name der Option als String
    sal_uInt16 nToken;          // und das entsprechende Token

public:

    HTMLOption( sal_uInt16 nTyp, const String& rToken, const String& rValue );

    // der Name der Option ...
    sal_uInt16 GetToken() const { return nToken; }  // ... als Enum
    const String& GetTokenString() const { return aToken; } // ... als String

    // der Wert der Option ...
    const String& GetString() const { return aValue; }  // ... als String

    sal_uInt32 GetNumber() const;                           // ... als Zahl
    sal_Int32 GetSNumber() const;                           // ... als Zahl
    void GetNumbers( SvULongs &rLongs,                  // ... als Zahlen
                     bool bSpaceDelim=false ) const;
    void GetColor( Color& ) const;                      // ... als Farbe

    // ... als Enum pOptEnums ist ein HTMLOptionEnum-Array
    sal_uInt16 GetEnum( const HTMLOptionEnum *pOptEnums,
                        sal_uInt16 nDflt=0 ) const;
    bool GetEnum( sal_uInt16 &rEnum, const HTMLOptionEnum *pOptEnums ) const;

    // ... und als ein par spezielle Enums
    HTMLInputType GetInputType() const;                 // <INPUT TYPE=...>
    HTMLTableFrame GetTableFrame() const;               // <TABLE FRAME=...>
    HTMLTableRules GetTableRules() const;               // <TABLE RULES=...>
    //SvxAdjust GetAdjust() const;                      // <P,TH,TD ALIGN=>
};

typedef HTMLOption* HTMLOptionPtr;
SV_DECL_PTRARR(HTMLOptions,HTMLOptionPtr,16,16)

class SVT_DLLPUBLIC HTMLParser : public SvParser
{
    bool bNewDoc        : 1;        // neues Doc lesen ?
    bool bIsInHeader    : 1;        // scanne Header-Bereich
    bool bIsInBody      : 1;        // scanne Body-Bereich
    bool bReadListing   : 1;        // Lese Listings
    bool bReadXMP       : 1;        // Lese XMP
    bool bReadPRE       : 1;        // Lese preformatted Text
    bool bReadTextArea  : 1;        // Lese TEXTAREA
    bool bReadScript    : 1;        // Lesen von <SCRIPT>
    bool bReadStyle     : 1;        // Lesen von <STYLE>
    bool bEndTokenFound : 1;        // </SCRIPT> oder </STYLE> gefunden

    bool bPre_IgnoreNewPara : 1;    // Flags fuers lesen von PRE-Absaetzen
    bool bReadNextChar : 1;         // true: NextChar nochmals lesen (JavaScript!)
    bool bReadComment : 1;          // true: NextChar nochmals lesen (JavaScript!)

    sal_uInt32 nPre_LinePos;            // Pos in der Line im PRE-Tag

    HTMLOptions *pOptions;          // die Optionen des Start-Tags
    String aEndToken;

protected:
    String sSaveToken;              // das gelesene Tag als String

    int ScanText( const sal_Unicode cBreak = 0U );

    int _GetNextRawToken();

    // scanne das naechste Token,
    virtual int _GetNextToken();

    virtual ~HTMLParser();

    void FinishHeader( bool bBody ) { bIsInHeader = false; bIsInBody = bBody; }

public:
    HTMLParser( SvStream& rIn, bool bReadNewDoc = true );

    virtual SvParserState CallParser();   // Aufruf des Parsers

    bool IsNewDoc() const       { return bNewDoc; }
    bool IsInHeader() const     { return bIsInHeader; }
    bool IsInBody() const       { return bIsInBody; }
    bool IsValidSyntax() const  { return true; }
    bool IsReadListing() const  { return bReadListing; }
    bool IsReadXMP() const      { return bReadXMP; }
    bool IsReadPRE() const      { return bReadPRE; }
    bool IsReadScript() const   { return bReadScript; }
    bool IsReadStyle() const    { return bReadStyle; }

    void SetReadNextChar()      { bReadNextChar = true; }

    // PRE-/LISTING oder XMP-Modus starten/beenden oder Tags entsprechend
    // filtern
    inline void StartPRE( bool bRestart=false );
    void FinishPRE() { bReadPRE = false; }
    int FilterPRE( int nToken );

    inline void StartListing( bool bRestart=false );
    void FinishListing() { bReadListing = false; }
    int FilterListing( int nToken );

    inline void StartXMP( bool bRestart=false );
    void FinishXMP() { bReadXMP = false; }
    int FilterXMP( int nToken );

    void FinishTextArea() { bReadTextArea = false; }

    // PRE-/LSITING- und XMP-Modus beenden
    void FinishPREListingXMP() { bReadPRE = bReadListing = bReadXMP = false; }

    // Das aktuelle Token dem aktuellen Modus (PRE, XMP, ...) entsprechend
    // Filtern und die Flags setzen. Wird von Continue aufgerufen, bevor
    // NextToken gerufen wird. Wer eigene Schleifen implementiert bzw.
    // selbst NextToken aufruft, sollte diese Methode vorher rufen.
    int FilterToken( int nToken );

    // Scannen eines Scripts beenden (sollte nur unmittelbar nach dem
    // Lesen eines <SCRIPT> aufgerufen werden
    void EndScanScript() { bReadScript = false; }

    void ReadRawData( const sal_Char *pEndToken ) { aEndToken.AssignAscii(pEndToken); }

    // Token ohne \-Sequenzen
    void UnescapeToken();

    // Ermitteln der Optionen. pNoConvertToken ist das optionale Token
    // einer Option, fuer die CR/LFs nicht aus dem Wert der Option
    // geloescht werden.
    const HTMLOptions *GetOptions( sal_uInt16 *pNoConvertToken=0 ) const;

    // fuers asynchrone lesen aus dem SvStream
//  virtual void SaveState( int nToken );
//  virtual void RestoreState();
    virtual void Continue( int nToken );


protected:

    static rtl_TextEncoding GetEncodingByMIME( const String& rMime );

    /// template method: called when ParseMetaOptions adds a user-defined meta
    virtual void AddMetaUserDefined( ::rtl::OUString const & i_rMetaName );

private:
    /// parse meta options into XDocumentProperties and encoding
    bool ParseMetaOptionsImpl( const ::com::sun::star::uno::Reference<
                ::com::sun::star::document::XDocumentProperties>&,
            SvKeyValueIterator*,
            const HTMLOptions*,
            rtl_TextEncoding& rEnc );

public:
    /// overriding method must call this implementation!
    virtual bool ParseMetaOptions( const ::com::sun::star::uno::Reference<
                ::com::sun::star::document::XDocumentProperties>&,
            SvKeyValueIterator* );

    // Ist der uebergebene 0-terminierte String (vermutlich) der Anfang
    // eines HTML-Files? Er sollte mind. 80 Zeichen lang sein.
    // Mit Ausnahme des Falls, dass SwitchToUCS2==false und
    // SVPAR_CS_DONTKNOW uebergeben wird muss der String mit zwei(!)
    // 0-Bytes an einer geraden(!) Position terminiert sein.
    static bool IsHTMLFormat( const sal_Char* pHeader,
                      bool bSwitchToUCS2 = false,
                      rtl_TextEncoding eEnc=RTL_TEXTENCODING_DONTKNOW );

    sal_Bool ParseScriptOptions( String& rLangString, const String&, HTMLScriptLanguage& rLang,
                             String& rSrc, String& rLibrary, String& rModule );

    // Einen Kommentar um den Inhalt von <SCRIPT> oder <STYLE> entfernen
    // Bei 'bFull' wird ggf. die gesammte Zeile hinter einem "<!--"
    // entfernt (fuer JavaSript)
    static void RemoveSGMLComment( String &rString, sal_Bool bFull );

    static bool InternalImgToPrivateURL( String& rURL );
    static rtl_TextEncoding GetEncodingByHttpHeader( SvKeyValueIterator *pHTTPHeader );
    bool SetEncodingByHTTPHeader( SvKeyValueIterator *pHTTPHeader );
};

inline void HTMLParser::StartPRE( bool bRestart )
{
    bReadPRE = true;
    bPre_IgnoreNewPara = !bRestart;
    nPre_LinePos = 0UL;
}

inline void HTMLParser::StartListing( bool bRestart )
{
    bReadListing = true;
    bPre_IgnoreNewPara = !bRestart;
    nPre_LinePos = 0UL;
}

inline void HTMLParser::StartXMP( bool bRestart )
{
    bReadXMP = true;
    bPre_IgnoreNewPara = !bRestart;
    nPre_LinePos = 0UL;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
