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
                     sal_Bool bSpaceDelim=sal_False ) const;
    void GetColor( Color& ) const;                      // ... als Farbe

    // ... als Enum pOptEnums ist ein HTMLOptionEnum-Array
    sal_uInt16 GetEnum( const HTMLOptionEnum *pOptEnums,
                        sal_uInt16 nDflt=0 ) const;
    BOOL GetEnum( sal_uInt16 &rEnum, const HTMLOptionEnum *pOptEnums ) const;

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
    BOOL bNewDoc        : 1;        // neues Doc lesen ?
    BOOL bIsInHeader    : 1;        // scanne Header-Bereich
    BOOL bIsInBody      : 1;        // scanne Body-Bereich
    BOOL bReadListing   : 1;        // Lese Listings
    BOOL bReadXMP       : 1;        // Lese XMP
    BOOL bReadPRE       : 1;        // Lese preformatted Text
    BOOL bReadTextArea  : 1;        // Lese TEXTAREA
    BOOL bReadScript    : 1;        // Lesen von <SCRIPT>
    BOOL bReadStyle     : 1;        // Lesen von <STYLE>
    BOOL bEndTokenFound : 1;        // </SCRIPT> oder </STYLE> gefunden

    BOOL bPre_IgnoreNewPara : 1;    // Flags fuers lesen von PRE-Absaetzen
    BOOL bReadNextChar : 1;         // TRUE: NextChar nochmals lesen (JavaScript!)
    BOOL bReadComment : 1;          // TRUE: NextChar nochmals lesen (JavaScript!)

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

    void FinishHeader( BOOL bBody ) { bIsInHeader = FALSE; bIsInBody = bBody; }

public:
    HTMLParser( SvStream& rIn, int bReadNewDoc = TRUE );

    virtual SvParserState CallParser();   // Aufruf des Parsers

    BOOL IsNewDoc() const       { return bNewDoc; }
    BOOL IsInHeader() const     { return bIsInHeader; }
    BOOL IsInBody() const       { return bIsInBody; }
    BOOL IsValidSyntax() const  { return TRUE; }
    BOOL IsReadListing() const  { return bReadListing; }
    BOOL IsReadXMP() const      { return bReadXMP; }
    BOOL IsReadPRE() const      { return bReadPRE; }
    BOOL IsReadScript() const   { return bReadScript; }
    BOOL IsReadStyle() const    { return bReadStyle; }

    void SetReadNextChar()      { bReadNextChar = TRUE; }

    // PRE-/LISTING oder XMP-Modus starten/beenden oder Tags entsprechend
    // filtern
    inline void StartPRE( BOOL bRestart=FALSE );
    void FinishPRE() { bReadPRE = FALSE; }
    int FilterPRE( int nToken );

    inline void StartListing( BOOL bRestart=FALSE );
    void FinishListing() { bReadListing = FALSE; }
    int FilterListing( int nToken );

    inline void StartXMP( BOOL bRestart=FALSE );
    void FinishXMP() { bReadXMP = FALSE; }
    int FilterXMP( int nToken );

    void FinishTextArea() { bReadTextArea = FALSE; }

    // PRE-/LSITING- und XMP-Modus beenden
    void FinishPREListingXMP() { bReadPRE = bReadListing = bReadXMP = FALSE; }

    // Das aktuelle Token dem aktuellen Modus (PRE, XMP, ...) entsprechend
    // Filtern und die Flags setzen. Wird von Continue aufgerufen, bevor
    // NextToken gerufen wird. Wer eigene Schleifen implementiert bzw.
    // selbst NextToken aufruft, sollte diese Methode vorher rufen.
    int FilterToken( int nToken );

    // Scannen eines Scripts beenden (sollte nur unmittelbar nach dem
    // Lesen eines <SCRIPT> aufgerufen werden
    void EndScanScript() { bReadScript = FALSE; }

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
    // Mit Ausnahme des Falls, dass SwitchToUCS2==FALSE und
    // SVPAR_CS_DONTKNOW uebergeben wird muss der String mit zwei(!)
    // 0-Bytes an einer geraden(!) Position terminiert sein.
    static bool IsHTMLFormat( const sal_Char* pHeader,
                      BOOL bSwitchToUCS2 = FALSE,
                      rtl_TextEncoding eEnc=RTL_TEXTENCODING_DONTKNOW );

    BOOL ParseScriptOptions( String& rLangString, const String&, HTMLScriptLanguage& rLang,
                             String& rSrc, String& rLibrary, String& rModule );

    // Einen Kommentar um den Inhalt von <SCRIPT> oder <STYLE> entfernen
    // Bei 'bFull' wird ggf. die gesammte Zeile hinter einem "<!--"
    // entfernt (fuer JavaSript)
    static void RemoveSGMLComment( String &rString, BOOL bFull );

    static BOOL InternalImgToPrivateURL( String& rURL );
    static rtl_TextEncoding GetEncodingByHttpHeader( SvKeyValueIterator *pHTTPHeader );
    BOOL SetEncodingByHTTPHeader( SvKeyValueIterator *pHTTPHeader );
};

inline void HTMLParser::StartPRE( BOOL bRestart )
{
    bReadPRE = TRUE;
    bPre_IgnoreNewPara = !bRestart;
    nPre_LinePos = 0UL;
}

inline void HTMLParser::StartListing( BOOL bRestart )
{
    bReadListing = TRUE;
    bPre_IgnoreNewPara = !bRestart;
    nPre_LinePos = 0UL;
}

inline void HTMLParser::StartXMP( BOOL bRestart )
{
    bReadXMP = TRUE;
    bPre_IgnoreNewPara = !bRestart;
    nPre_LinePos = 0UL;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
